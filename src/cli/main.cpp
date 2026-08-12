#include "were_oier/core/ai.hpp"
#include "were_oier/core/game.hpp"

#include <algorithm>
#include <array>
#include <charconv>
#include <cstdint>
#include <iostream>
#include <optional>
#include <random>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>
#include <utility>
#include <vector>

namespace {

using were_oier::Action;
using were_oier::ActionKind;
using were_oier::AiController;
using were_oier::ConfigValidation;
using were_oier::DecisionProvider;
using were_oier::DecisionRequest;
using were_oier::EliminationNotice;
using were_oier::EventType;
using were_oier::EventVisibility;
using were_oier::GameConfig;
using were_oier::GameEngine;
using were_oier::GameEvent;
using were_oier::KnownFact;
using were_oier::KnowledgeSource;
using were_oier::MatchPreset;
using were_oier::MatchReport;
using were_oier::PlayerId;
using were_oier::PlayerPublicView;
using were_oier::Role;
using were_oier::RoleCounts;
using were_oier::SpeechKind;

constexpr std::array<Role, 7> kRoles{
    Role::Villager, Role::Werewolf, Role::WhiteWolfKing, Role::Seer,
    Role::Witch, Role::Hunter, Role::Magician,
};

[[nodiscard]] std::string player_name(PlayerId player) {
  return std::to_string(player + 1) + "号";
}

[[nodiscard]] std::string_view trim(std::string_view value) {
  const auto first = value.find_first_not_of(" \t\r\n");
  if (first == std::string_view::npos) {
    return {};
  }
  const auto last = value.find_last_not_of(" \t\r\n");
  return value.substr(first, last - first + 1);
}

[[nodiscard]] std::optional<long long> parse_integer(std::string_view text) {
  text = trim(text);
  if (text.empty()) {
    return std::nullopt;
  }
  long long value = 0;
  const char* const first = text.data();
  const char* const last = first + text.size();
  const auto [end, error] = std::from_chars(first, last, value);
  if (error != std::errc{} || end != last) {
    return std::nullopt;
  }
  return value;
}

[[nodiscard]] std::optional<std::uint64_t> parse_seed(std::string_view text) {
  text = trim(text);
  if (text.empty()) {
    return std::nullopt;
  }
  std::uint64_t value = 0;
  const char* const first = text.data();
  const char* const last = first + text.size();
  const auto [end, error] = std::from_chars(first, last, value);
  if (error != std::errc{} || end != last) {
    return std::nullopt;
  }
  return value;
}

[[nodiscard]] std::string read_line(std::string_view prompt) {
  std::cout << prompt;
  std::string line;
  if (!std::getline(std::cin, line)) {
    throw std::runtime_error("输入已结束。");
  }
  return line;
}

[[nodiscard]] int read_int(std::string_view prompt, int minimum, int maximum) {
  while (true) {
    const std::optional<long long> parsed = parse_integer(read_line(prompt));
    if (parsed.has_value() && *parsed >= minimum && *parsed <= maximum) {
      return static_cast<int>(*parsed);
    }
    std::cout << "请输入 " << minimum << " 到 " << maximum << " 之间的整数。\n";
  }
}

[[nodiscard]] bool ask_yes_no(std::string_view prompt) {
  while (true) {
    const std::string answer = read_line(std::string(prompt) + " [y/n]: ");
    if (answer == "y" || answer == "Y" || answer == "是") {
      return true;
    }
    if (answer == "n" || answer == "N" || answer == "否") {
      return false;
    }
    std::cout << "请输入 y 或 n。\n";
  }
}

void print_separator() { std::cout << "------------------------------------------------------------\n"; }

void print_players(const std::vector<PlayerPublicView>& players) {
  std::cout << "存活：";
  bool first = true;
  for (const PlayerPublicView& player : players) {
    if (!player.alive) {
      continue;
    }
    if (!first) {
      std::cout << "、";
    }
    std::cout << player_name(player.id);
    first = false;
  }
  if (first) {
    std::cout << "无";
  }
  std::cout << "\n出局：";
  first = true;
  for (const PlayerPublicView& player : players) {
    if (player.alive) {
      continue;
    }
    if (!first) {
      std::cout << "、";
    }
    std::cout << player_name(player.id);
    if (player.revealed_role.has_value()) {
      std::cout << "(" << were_oier::role_name(*player.revealed_role) << ")";
    }
    first = false;
  }
  if (first) {
    std::cout << "无";
  }
  std::cout << "\n";
}

[[nodiscard]] std::string targets_text(const std::vector<PlayerId>& targets) {
  std::ostringstream stream;
  for (std::size_t index = 0; index < targets.size(); ++index) {
    if (index != 0) {
      stream << "、";
    }
    stream << player_name(targets[index]);
  }
  return stream.str();
}

[[nodiscard]] PlayerId choose_target(const std::vector<PlayerId>& targets, std::string_view prompt,
                                     bool allow_skip = false) {
  if (targets.empty()) {
    return were_oier::kNoPlayer;
  }
  while (true) {
    std::cout << "可选目标：" << targets_text(targets);
    if (allow_skip) {
      std::cout << "（输入 0 跳过）";
    }
    std::cout << "\n";
    const int lower = allow_skip ? 0 : 1;
    const int choice = read_int(prompt, lower, 16);
    if (allow_skip && choice == 0) {
      return were_oier::kNoPlayer;
    }
    const PlayerId player = static_cast<PlayerId>(choice - 1);
    for (const PlayerId candidate : targets) {
      if (candidate == player) {
        return player;
      }
    }
    std::cout << "该编号不是当前合法目标。\n";
  }
}

[[nodiscard]] Role choose_role() {
  std::cout << "选择声明身份：\n";
  for (std::size_t index = 0; index < kRoles.size(); ++index) {
    std::cout << "  " << index + 1 << ". " << were_oier::role_name(kRoles[index]) << "\n";
  }
  const int selected = read_int("编号: ", 1, static_cast<int>(kRoles.size()));
  return kRoles[static_cast<std::size_t>(selected - 1)];
}

[[nodiscard]] std::string describe_speech(const GameEvent& event) {
  if (!event.speech.has_value()) {
    return player_name(event.actor) + "发言。";
  }
  switch (*event.speech) {
    case SpeechKind::ClaimGood:
      return player_name(event.actor) + "：我是好人。";
    case SpeechKind::ClaimRole:
      return player_name(event.actor) + "：我是" +
             std::string(event.role.has_value() ? were_oier::role_name(*event.role) : "未知身份") + "。";
    case SpeechKind::Accuse:
      return player_name(event.actor) + "：我怀疑" + player_name(event.target) + "。";
    case SpeechKind::Defend:
      return player_name(event.actor) + "：我认为" + player_name(event.target) + "可信。";
    case SpeechKind::Silence:
      return player_name(event.actor) + "选择沉默。";
  }
  return player_name(event.actor) + "发言。";
}

[[nodiscard]] std::string describe_event(const GameEvent& event, bool reveal_secrets) {
  std::ostringstream stream;
  switch (event.type) {
    case EventType::MatchStarted:
      return "新对局开始。";
    case EventType::PhaseChanged:
      return "【" + event.detail + "】";
    case EventType::RoleAssigned:
      return player_name(event.target) + "初始身份为" +
             std::string(event.role.has_value() ? were_oier::role_name(*event.role) : "未知") + "。";
    case EventType::RoleChanged:
      return player_name(event.target) + "的身份牌变为" +
             std::string(event.role.has_value() ? were_oier::role_name(*event.role) : "未知") + "。";
    case EventType::RolesSwapped:
      return "魔术师交换了" + player_name(event.target) + "与" + player_name(event.secondary_target) + "的身份牌。";
    case EventType::InvestigationResult:
      return "查验结果：" + player_name(event.target) + "是" +
             std::string(event.role.has_value() ? were_oier::role_name(*event.role) : "未知") + "。";
    case EventType::WerewolfTargetSelected:
      return "狼人队选择袭击" + player_name(event.target) + "。";
    case EventType::WhiteWolfKingTargetSelected:
      return "白狼王选择袭击" + player_name(event.target) + "。";
    case EventType::WitchSaved:
      return "女巫使用解药救下" + player_name(event.target) + "。";
    case EventType::WitchPoisoned:
      return "女巫选择毒杀" + player_name(event.target) + "。";
    case EventType::PlayerDied:
      stream << player_name(event.target) << "出局";
      if (reveal_secrets && event.death_cause.has_value()) {
        stream << "（" << were_oier::death_cause_name(*event.death_cause) << "）";
      }
      stream << "。";
      return stream.str();
    case EventType::RoleRevealed:
      return player_name(event.target) + "的身份是" +
             std::string(event.role.has_value() ? were_oier::role_name(*event.role) : "未知") + "。";
    case EventType::Speech:
      return describe_speech(event);
    case EventType::VoteCast:
      return player_name(event.actor) + "投给了" + player_name(event.target) + "。";
    case EventType::VoteResolved:
      return event.detail == "tie" ? "本轮平票，无人被放逐。"
                                   : player_name(event.target) + "被放逐。";
    case EventType::DeathShot:
      return player_name(event.actor) + "发动死亡技能，带走" + player_name(event.target) + "。";
    case EventType::ActionRejected:
      return "操作无效：" + event.detail;
    case EventType::DecisionReason:
      if (!reveal_secrets) {
        return {};
      }
      return player_name(event.actor) + "的 AI 决策理由：" + event.detail;
    case EventType::MatchFinished:
      return "本局结束：" + event.detail;
  }
  return {};
}

void print_knowledge(const DecisionRequest& request) {
  const auto& knowledge = request.private_knowledge;
  std::cout << "你的当前身份：" << were_oier::role_name(knowledge.role) << "\n";
  if (knowledge.magician_swap_available) {
    std::cout << "魔术师换牌：可用\n";
  }
  if (knowledge.role == Role::Witch) {
    std::cout << "女巫资源：解药" << (knowledge.witch_save_available ? "可用" : "已用")
              << "，毒药" << (knowledge.witch_poison_available ? "可用" : "已用") << "\n";
    if (knowledge.known_werewolf_target.has_value()) {
      std::cout << "今晚狼人袭击了" << player_name(*knowledge.known_werewolf_target) << "。\n";
    }
  }
  if (!knowledge.facts.empty()) {
    std::cout << "私密情报：\n";
    for (const KnownFact& fact : knowledge.facts) {
      const std::string_view source = fact.source == KnowledgeSource::WolfPack ? "狼人同伴" : "查验结果";
      std::cout << "  - " << player_name(fact.player) << "是" << were_oier::role_name(fact.role)
                << "（" << source << "）\n";
    }
  }
}

class TerminalProvider final : public DecisionProvider {
 public:
  TerminalProvider(PlayerId controlled, std::uint64_t seed)
      : controlled_(controlled), ai_(seed ^ 0x9e3779b97f4a7c15ULL) {}

  [[nodiscard]] bool controls(PlayerId player) const override {
    return controlled_.has_value() && *controlled_ == player;
  }

  Action choose_action(const DecisionRequest& request) override {
    if (!controls(request.actor)) {
      return ai_.decide(request);
    }

    print_separator();
    std::cout << "轮到你操控的" << player_name(request.actor) << "行动（"
              << were_oier::phase_name(request.phase) << "）。\n";
    print_players(request.players);
    print_knowledge(request);
    return choose_human_action(request);
  }

  void on_event(const GameEvent& event) override {
    if (event.visibility == EventVisibility::Private &&
        (!controlled_.has_value() || !event.visible_to(*controlled_))) {
      return;
    }
    if (event.type == EventType::DecisionReason) {
      return;
    }
    const std::string text = describe_event(event, false);
    if (text.empty()) {
      return;
    }
    if (event.visibility == EventVisibility::Private) {
      std::cout << "[私密] ";
    }
    std::cout << text << "\n";
  }

  void on_player_eliminated(const EliminationNotice& notice) override {
    if (!controls(notice.player)) {
      return;
    }
    print_separator();
    std::cout << "你操控的" << player_name(notice.player) << "已出局。";
    std::vector<PlayerId> living;
    for (const PlayerPublicView& player : notice.snapshot.players) {
      if (player.alive) {
        living.push_back(player.id);
      }
    }
    if (living.empty()) {
      controlled_.reset();
      std::cout << "没有可接管的玩家，转为观战。\n";
      return;
    }

    std::cout << "\n1. 继续观战\n2. 接管一名存活 AI（完整继承其私密信息）\n";
    const int choice = read_int("选择: ", 1, 2);
    if (choice == 1) {
      controlled_.reset();
      std::cout << "已转为观战；局后可查看完整复盘。\n";
      return;
    }
    const PlayerId successor = choose_target(living, "接管哪一名玩家: ");
    controlled_ = successor;
    std::cout << "你已接管" << player_name(successor) << "；下一次行动会展示其完整私密信息。\n";
  }

 private:
  std::optional<PlayerId> controlled_;
  AiController ai_;

  [[nodiscard]] Action choose_human_action(const DecisionRequest& request) {
    Action action;
    switch (request.phase) {
      case were_oier::Phase::NightMagician: {
        if (!request.private_knowledge.magician_swap_available) {
          return Action::skip();
        }
        const int choice = read_int("1. 交换两张身份牌  2. 本夜跳过\n选择: ", 1, 2);
        if (choice == 2) {
          return Action::skip();
        }
        action.kind = ActionKind::SwapRoles;
        action.target = choose_target(request.available_targets, "第一名玩家: ");
        std::vector<PlayerId> other_targets = request.available_targets;
        other_targets.erase(std::remove(other_targets.begin(), other_targets.end(), action.target),
                            other_targets.end());
        action.secondary_target = choose_target(other_targets, "第二名玩家: ");
        return action;
      }
      case were_oier::Phase::NightSeer:
        if (request.available_targets.empty()) {
          return Action::skip();
        }
        action.kind = ActionKind::Investigate;
        action.target = choose_target(request.available_targets, "查验哪一名玩家: ", true);
        return action.target == were_oier::kNoPlayer ? Action::skip() : action;
      case were_oier::Phase::NightWolves:
        if (request.available_targets.empty()) {
          return Action::skip();
        }
        action.kind = ActionKind::WerewolfAttack;
        action.target = choose_target(request.available_targets, "狼人队今晚袭击: ", true);
        return action.target == were_oier::kNoPlayer ? Action::skip() : action;
      case were_oier::Phase::NightWhiteWolfKing:
        if (request.available_targets.empty()) {
          return Action::skip();
        }
        action.kind = ActionKind::WhiteWolfKingAttack;
        action.target = choose_target(request.available_targets, "白狼王今晚袭击: ", true);
        return action.target == were_oier::kNoPlayer ? Action::skip() : action;
      case were_oier::Phase::NightWitch:
        return choose_witch_action(request);
      case were_oier::Phase::DayDiscussion:
        return choose_speech_action(request);
      case were_oier::Phase::DayVote:
        action.kind = ActionKind::Vote;
        action.target = choose_target(request.available_targets, "投票给哪一名玩家: ");
        return action;
      case were_oier::Phase::DeathTrigger:
        if (request.available_targets.empty()) {
          return Action::skip();
        }
        action.kind = ActionKind::DeathShot;
        action.target = choose_target(request.available_targets, "发动死亡技能带走谁（0 跳过）: ", true);
        return action.target == were_oier::kNoPlayer ? Action::skip() : action;
      case were_oier::Phase::Setup:
      case were_oier::Phase::Dawn:
      case were_oier::Phase::Finished:
        return Action::skip();
    }
    return Action::skip();
  }

  [[nodiscard]] Action choose_witch_action(const DecisionRequest& request) {
    const auto& knowledge = request.private_knowledge;
    std::cout << "1. 跳过\n";
    int save_option = 0;
    int poison_option = 0;
    int next_option = 2;
    if (knowledge.witch_save_available && knowledge.known_werewolf_target.has_value()) {
      save_option = next_option++;
      std::cout << save_option << ". 使用解药救" << player_name(*knowledge.known_werewolf_target) << "\n";
    }
    if (knowledge.witch_poison_available) {
      poison_option = next_option++;
      std::cout << poison_option << ". 使用毒药\n";
    }
    const int choice = read_int("选择: ", 1, next_option - 1);
    if (choice == 1) {
      return Action::skip();
    }
    Action action;
    if (choice == save_option) {
      action.kind = ActionKind::WitchSave;
      action.target = *knowledge.known_werewolf_target;
      return action;
    }
    std::vector<PlayerId> poison_targets = request.available_targets;
    poison_targets.erase(std::remove(poison_targets.begin(), poison_targets.end(), request.actor),
                         poison_targets.end());
    action.kind = ActionKind::WitchPoison;
    action.target = choose_target(poison_targets, "毒杀哪一名玩家: ");
    return action;
  }

  [[nodiscard]] Action choose_speech_action(const DecisionRequest& request) {
    std::cout << "1. 声明自己是好人\n2. 声明一个身份\n3. 质疑一名玩家\n4. 为一名玩家辩护\n5. 沉默\n";
    const int choice = read_int("选择: ", 1, 5);
    Action action;
    action.kind = ActionKind::Speak;
    if (choice == 1) {
      action.speech = SpeechKind::ClaimGood;
    } else if (choice == 2) {
      action.speech = SpeechKind::ClaimRole;
      action.claimed_role = choose_role();
    } else if (choice == 3) {
      action.speech = SpeechKind::Accuse;
      action.target = choose_target(request.available_targets, "质疑哪一名玩家: ");
    } else if (choice == 4) {
      action.speech = SpeechKind::Defend;
      action.target = choose_target(request.available_targets, "为哪一名玩家辩护: ");
    } else {
      action.speech = SpeechKind::Silence;
    }
    return action;
  }
};

void print_config(const GameConfig& config) {
  std::cout << "人数：" << config.player_count << "，真人座位：" << player_name(config.human_seat) << "\n";
  std::cout << "牌组：平民 " << config.roles.villagers << "，狼人 " << config.roles.werewolves
            << "，白狼王 " << config.roles.white_wolf_kings << "，预言家 " << config.roles.seers
            << "，女巫 " << config.roles.witches << "，猎人 " << config.roles.hunters
            << "，魔术师 " << config.roles.magicians << "\n";
}

[[nodiscard]] bool confirm_config(const GameConfig& config) {
  const ConfigValidation validation = config.validate();
  if (!validation.ok()) {
    std::cout << "配置无效：\n";
    for (const std::string& error : validation.errors) {
      std::cout << "  - " << error << "\n";
    }
    return false;
  }
  print_config(config);
  if (!validation.warnings.empty()) {
    std::cout << "平衡提示：\n";
    for (const std::string& warning : validation.warnings) {
      std::cout << "  - " << warning << "\n";
    }
  }
  return ask_yes_no("使用此配置开始对局？");
}

[[nodiscard]] std::uint64_t choose_seed() {
  while (true) {
    const std::optional<std::uint64_t> parsed = parse_seed(read_line("输入种子（0 使用随机种子）: "));
    if (!parsed.has_value()) {
      std::cout << "请输入非负整数。\n";
      continue;
    }
    if (*parsed != 0) {
      return *parsed;
    }
    std::random_device device;
    return (static_cast<std::uint64_t>(device()) << 32U) ^ static_cast<std::uint64_t>(device());
  }
}

void show_report(const MatchReport& report) {
  print_separator();
  std::cout << "对局结算：" << were_oier::result_name(report.result) << "，共经历 "
            << report.nights_played << " 个夜晚。\n";
  std::cout << "最终身份：\n";
  for (const auto& player : report.players) {
    std::cout << "  " << player_name(player.id) << "：初始 " << were_oier::role_name(player.initial_role)
              << "，最终 " << were_oier::role_name(player.final_role)
              << "，" << (player.alive ? "存活" : "出局") << "\n";
  }

  std::cout << "\n完整事件时间线：\n";
  for (const GameEvent& event : report.events) {
    if (event.type == EventType::DecisionReason) {
      continue;
    }
    const std::string text = describe_event(event, true);
    if (!text.empty()) {
      std::cout << "  第" << event.night << "夜 · " << text << "\n";
    }
  }

  std::cout << "\nAI 决策理由：\n";
  bool has_reason = false;
  for (const GameEvent& event : report.events) {
    if (event.type != EventType::DecisionReason) {
      continue;
    }
    has_reason = true;
    std::cout << "  第" << event.night << "夜 · " << describe_event(event, true) << "\n";
    if (!event.suspicion_snapshot.empty()) {
      std::cout << "    嫌疑快照：";
      for (std::size_t index = 0; index < event.suspicion_snapshot.size(); ++index) {
        const auto& score = event.suspicion_snapshot[index];
        if (index != 0) {
          std::cout << "，";
        }
        std::cout << player_name(score.player) << '=' << score.score;
      }
      std::cout << "\n";
    }
  }
  if (!has_reason) {
    std::cout << "  本局没有 AI 行动。\n";
  }
}

void play_match(GameConfig config) {
  config.seed = choose_seed();
  std::cout << "本局种子：" << config.seed << "\n";
  TerminalProvider provider(config.human_seat, config.seed);
  GameEngine game(config);
  game.start(provider);
  while (!game.finished()) {
    game.play_night(provider);
    if (!game.finished()) {
      game.play_day(provider);
    }
  }
  show_report(game.report());
  (void)read_line("按 Enter 返回主菜单。");
}

void start_preset_match() {
  const std::vector<MatchPreset> presets = were_oier::preset_configs();
  print_separator();
  std::cout << "选择预设局：\n";
  for (std::size_t index = 0; index < presets.size(); ++index) {
    std::cout << "  " << index + 1 << ". " << presets[index].name << " — " << presets[index].description
              << "\n";
  }
  std::cout << "  0. 返回\n";
  const int choice = read_int("选择: ", 0, static_cast<int>(presets.size()));
  if (choice == 0) {
    return;
  }
  GameConfig config = presets[static_cast<std::size_t>(choice - 1)].config;
  config.human_seat = static_cast<PlayerId>(
      read_int("你要坐几号位: ", 1, static_cast<int>(config.player_count)) - 1);
  if (confirm_config(config)) {
    play_match(config);
  }
}

[[nodiscard]] RoleCounts read_role_counts(int player_count) {
  RoleCounts counts;
  std::cout << "依次输入各身份数量；总和必须等于 " << player_count << "。\n";
  counts.villagers = read_int("平民: ", 0, player_count);
  counts.werewolves = read_int("狼人: ", 0, player_count);
  counts.white_wolf_kings = read_int("白狼王（最多 1）: ", 0, 1);
  counts.seers = read_int("预言家（最多 1）: ", 0, 1);
  counts.witches = read_int("女巫（最多 1）: ", 0, 1);
  counts.hunters = read_int("猎人（最多 1）: ", 0, 1);
  counts.magicians = read_int("魔术师（最多 1）: ", 0, 1);
  return counts;
}

void start_custom_match() {
  while (true) {
    print_separator();
    std::cout << "高级自定义局（6–16 人）\n";
    GameConfig config = were_oier::standard_config();
    config.player_count = static_cast<std::size_t>(read_int("玩家人数: ", 6, 16));
    config.roles = read_role_counts(static_cast<int>(config.player_count));
    config.human_seat = static_cast<PlayerId>(
        read_int("你要坐几号位: ", 1, static_cast<int>(config.player_count)) - 1);

    if (confirm_config(config)) {
      play_match(config);
      return;
    }
    if (!ask_yes_no("重新配置？")) {
      return;
    }
  }
}

void show_rules() {
  print_separator();
  std::cout << R"(局规摘要

- 牌组支持 6–16 人；至少一名狼人阵营、两名好人。预设十人局为 3 平民、2 狼人、白狼王、预言家、女巫、猎人、魔术师。
- 夜晚顺序：魔术师换牌（全局一次）→ 预言家查验 → 普通狼人共同击杀 → 白狼王独立击杀 → 女巫行动。
- 女巫有一瓶解药与一瓶毒药，同一夜最多使用其中一种；解药只阻止普通狼人的击杀。
- 猎人和白狼王死亡时都可以带走一名存活玩家。身份牌被换走后，能力与阵营随新身份牌变化。
- 狼人阵营存活人数不少于好人时获胜；狼人阵营全灭时好人获胜。为避免异常无限局，最多 30 个夜晚后平局。
- 白天使用结构化发言（声明、质疑、辩护、沉默）和公开投票；不再解析自然语言关键词。
- AI 只基于公共事件和自身合法私密信息决策。对局结束后可查看所有隐藏行动、身份、AI 理由与嫌疑评分。
- 你出局后可继续观战，或接管一名存活 AI 并完整继承其私密信息。无音效、无礼包、无商店、无持久化。
)";
  (void)read_line("按 Enter 返回主菜单。");
}

}  // namespace

int main() {
  try {
    std::cout << "\x1b[1;36mWere-OIer · C++23 现代终端狼人杀\x1b[0m\n";
    std::cout << "推理、行动与复盘全部可追溯；请输入编号操作。\n";
    while (true) {
      print_separator();
      std::cout << "1. 开始预设对局\n2. 高级自定义对局\n3. 查看局规\n0. 退出\n";
      const int choice = read_int("选择: ", 0, 3);
      if (choice == 0) {
        std::cout << "再见。\n";
        return 0;
      }
      if (choice == 1) {
        start_preset_match();
      } else if (choice == 2) {
        start_custom_match();
      } else {
        show_rules();
      }
    }
  } catch (const std::exception& error) {
    std::cerr << "程序结束：" << error.what() << "\n";
    return 1;
  }
}
