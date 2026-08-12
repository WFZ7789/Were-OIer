#include "were_oier/core/game.hpp"

#include <algorithm>
#include <array>
#include <random>
#include <stdexcept>
#include <string>
#include <utility>

namespace were_oier {
namespace {

constexpr std::array<Role, 7> kRoles{
    Role::Villager, Role::Werewolf, Role::WhiteWolfKing, Role::Seer,
    Role::Witch, Role::Hunter, Role::Magician,
};

[[nodiscard]] bool contains(const std::vector<PlayerId>& players, PlayerId player) {
  return std::find(players.begin(), players.end(), player) != players.end();
}

[[nodiscard]] std::vector<PlayerId> unique_recipients(std::vector<PlayerId> recipients) {
  std::sort(recipients.begin(), recipients.end());
  recipients.erase(std::unique(recipients.begin(), recipients.end()), recipients.end());
  return recipients;
}

[[nodiscard]] bool valid_player(PlayerId player, std::size_t player_count) {
  return player != kNoPlayer && player < player_count;
}

}  // namespace

std::string_view role_name(Role role) {
  switch (role) {
    case Role::Villager:
      return "平民";
    case Role::Werewolf:
      return "狼人";
    case Role::WhiteWolfKing:
      return "白狼王";
    case Role::Seer:
      return "预言家";
    case Role::Witch:
      return "女巫";
    case Role::Hunter:
      return "猎人";
    case Role::Magician:
      return "魔术师";
  }
  return "未知身份";
}

std::string_view team_name(Team team) {
  return team == Team::Wolves ? "狼人阵营" : "好人阵营";
}

std::string_view phase_name(Phase phase) {
  switch (phase) {
    case Phase::Setup:
      return "准备阶段";
    case Phase::NightMagician:
      return "夜晚：魔术师";
    case Phase::NightSeer:
      return "夜晚：预言家";
    case Phase::NightWolves:
      return "夜晚：狼人";
    case Phase::NightWhiteWolfKing:
      return "夜晚：白狼王";
    case Phase::NightWitch:
      return "夜晚：女巫";
    case Phase::Dawn:
      return "黎明";
    case Phase::DayDiscussion:
      return "白天：讨论";
    case Phase::DayVote:
      return "白天：投票";
    case Phase::DeathTrigger:
      return "死亡技能";
    case Phase::Finished:
      return "对局结束";
  }
  return "未知阶段";
}

std::string_view death_cause_name(DeathCause cause) {
  switch (cause) {
    case DeathCause::WerewolfAttack:
      return "狼人袭击";
    case DeathCause::WhiteWolfKingAttack:
      return "白狼王袭击";
    case DeathCause::WitchPoison:
      return "女巫毒药";
    case DeathCause::Vote:
      return "放逐";
    case DeathCause::HunterShot:
      return "猎人开枪";
    case DeathCause::WhiteWolfKingShot:
      return "白狼王带人";
  }
  return "未知原因";
}

std::string_view speech_name(SpeechKind speech) {
  switch (speech) {
    case SpeechKind::ClaimGood:
      return "声明好人";
    case SpeechKind::ClaimRole:
      return "声明身份";
    case SpeechKind::Accuse:
      return "质疑";
    case SpeechKind::Defend:
      return "辩护";
    case SpeechKind::Silence:
      return "沉默";
  }
  return "未知发言";
}

std::string_view result_name(MatchResult result) {
  switch (result) {
    case MatchResult::Ongoing:
      return "进行中";
    case MatchResult::VillageWin:
      return "好人阵营获胜";
    case MatchResult::WolvesWin:
      return "狼人阵营获胜";
    case MatchResult::Draw:
      return "平局";
  }
  return "未知结果";
}

Team team_of(Role role) {
  return is_wolf_team(role) ? Team::Wolves : Team::Village;
}

bool is_wolf_team(Role role) {
  return role == Role::Werewolf || role == Role::WhiteWolfKing;
}

bool is_unique_role(Role role) {
  return role != Role::Villager && role != Role::Werewolf;
}

int RoleCounts::count(Role role) const {
  switch (role) {
    case Role::Villager:
      return villagers;
    case Role::Werewolf:
      return werewolves;
    case Role::WhiteWolfKing:
      return white_wolf_kings;
    case Role::Seer:
      return seers;
    case Role::Witch:
      return witches;
    case Role::Hunter:
      return hunters;
    case Role::Magician:
      return magicians;
  }
  return 0;
}

int RoleCounts::total() const {
  return villagers + werewolves + white_wolf_kings + seers + witches + hunters + magicians;
}

int RoleCounts::wolf_team_total() const { return werewolves + white_wolf_kings; }

int RoleCounts::village_team_total() const { return total() - wolf_team_total(); }

ConfigValidation GameConfig::validate() const {
  ConfigValidation validation;
  if (player_count < 6 || player_count > 16) {
    validation.errors.emplace_back("人数必须在 6 到 16 人之间。");
  }
  if (human_seat >= player_count) {
    validation.errors.emplace_back("真人座位必须位于玩家人数范围内。");
  }
  if (max_nights == 0 || max_nights > 100) {
    validation.errors.emplace_back("最大夜晚数必须在 1 到 100 之间。");
  }

  for (const Role role : kRoles) {
    const int value = roles.count(role);
    if (value < 0) {
      validation.errors.emplace_back(std::string(role_name(role)) + "数量不能为负数。");
    }
    if (is_unique_role(role) && value > 1) {
      validation.errors.emplace_back(std::string(role_name(role)) + "最多只能有一名。");
    }
  }

  if (roles.total() != static_cast<int>(player_count)) {
    validation.errors.emplace_back("所有身份数量之和必须等于玩家人数。");
  }
  if (roles.wolf_team_total() < 1) {
    validation.errors.emplace_back("牌组至少需要一名狼人阵营玩家。");
  }
  if (roles.village_team_total() < 2) {
    validation.errors.emplace_back("牌组至少需要两名好人阵营玩家。");
  }

  if (validation.ok()) {
    const int wolves = roles.wolf_team_total();
    const int village = roles.village_team_total();
    if (wolves >= village) {
      validation.warnings.emplace_back("狼人阵营人数不少于好人；对局会在开始时立刻结束。");
    } else if (wolves * 3 > static_cast<int>(player_count)) {
      validation.warnings.emplace_back("狼人阵营比例较高，对局可能明显偏向狼人。");
    }
    if (roles.seers == 0 || roles.witches == 0) {
      validation.warnings.emplace_back("缺少信息或救援角色，推理空间会减少。");
    }
  }
  return validation;
}

GameConfig standard_config() {
  GameConfig config;
  config.player_count = 10;
  config.roles = RoleCounts{};
  config.human_seat = 0;
  config.max_nights = 30;
  return config;
}

std::vector<MatchPreset> preset_configs() {
  auto make = [](std::string name, std::string description, std::size_t players,
                 RoleCounts roles) {
    GameConfig config;
    config.player_count = players;
    config.roles = roles;
    return MatchPreset{std::move(name), std::move(description), config};
  };

  return {
      make("新手六人局", "一狼、预言家、女巫、猎人和两名平民。", 6,
           RoleCounts{2, 1, 0, 1, 1, 1, 0}),
      make("进阶八人局", "双狼加入白狼王，保留四个好人身份。", 8,
           RoleCounts{2, 2, 1, 1, 1, 1, 0}),
      make("标准十人局", "原作七种身份的平衡化现代规则。", 10,
           RoleCounts{3, 2, 1, 1, 1, 1, 1}),
      make("扩展十二人局", "三狼、白狼王与完整好人阵营。", 12,
           RoleCounts{4, 3, 1, 1, 1, 1, 1}),
      make("大型十六人局", "四狼、白狼王与七名平民。", 16,
           RoleCounts{7, 4, 1, 1, 1, 1, 1}),
  };
}

Action Action::skip(std::string rationale) {
  Action action;
  action.kind = ActionKind::Skip;
  action.rationale = std::move(rationale);
  return action;
}

bool GameEvent::visible_to(PlayerId player) const {
  return visibility == EventVisibility::Public || contains(recipients, player);
}

struct GameEngine::PlayerState {
  struct Investigation {
    PlayerId target{kNoPlayer};
    Role role{Role::Villager};
  };

  Role role{Role::Villager};
  bool alive{true};
  std::vector<Investigation> investigations;
};

struct GameEngine::PendingDeath {
  PlayerId target{kNoPlayer};
  DeathCause cause{DeathCause::Vote};
  PlayerId source{kNoPlayer};
};

class GameEngine::Random {
 public:
  explicit Random(std::uint64_t seed) : engine_(seed) {}

  template <typename Iterator>
  void shuffle(Iterator first, Iterator last) {
    std::shuffle(first, last, engine_);
  }

 private:
  std::mt19937_64 engine_;
};

GameEngine::GameEngine(GameConfig config, std::vector<Role> scripted_deal)
    : config_(std::move(config)),
      scripted_deal_(std::move(scripted_deal)),
      random_(std::make_unique<Random>(config_.seed)) {
  const ConfigValidation validation = config_.validate();
  if (!validation.ok()) {
    throw std::invalid_argument(validation.errors.front());
  }

  if (!scripted_deal_.empty()) {
    if (scripted_deal_.size() != config_.player_count) {
      throw std::invalid_argument("脚本化发牌的长度必须等于玩家人数。");
    }
    for (const Role role : kRoles) {
      const auto expected = static_cast<std::size_t>(config_.roles.count(role));
      const auto actual = static_cast<std::size_t>(
          std::count(scripted_deal_.begin(), scripted_deal_.end(), role));
      if (expected != actual) {
        throw std::invalid_argument("脚本化发牌必须与牌组配置一致。");
      }
    }
  }
}

GameEngine::~GameEngine() = default;
GameEngine::GameEngine(GameEngine&&) noexcept = default;
GameEngine& GameEngine::operator=(GameEngine&&) noexcept = default;

void GameEngine::start(DecisionProvider& provider) {
  if (started_) {
    throw std::logic_error("同一局游戏不能重复开始。");
  }
  started_ = true;
  emit(provider, GameEvent{.type = EventType::MatchStarted,
                           .visibility = EventVisibility::Public,
                           .detail = "match-started"});
  deal_roles(provider);
  check_winner(provider);
}

void GameEngine::play_night(DecisionProvider& provider) {
  if (!started_) {
    throw std::logic_error("必须先开始游戏。");
  }
  if (finished()) {
    return;
  }
  if (phase_ != Phase::Setup && phase_ != Phase::DayVote) {
    throw std::logic_error("必须在准备阶段或上一轮白天结束后进入夜晚。");
  }
  if (night_ >= config_.max_nights) {
    finish(provider, MatchResult::Draw, "达到最大夜晚数，避免无限对局。");
    return;
  }

  ++night_;
  pending_werewolf_target_ = kNoPlayer;
  saved_werewolf_target_ = kNoPlayer;
  pending_white_wolf_target_ = kNoPlayer;
  pending_witch_poison_target_ = kNoPlayer;

  change_phase(provider, Phase::NightMagician);
  run_magician_phase(provider);
  change_phase(provider, Phase::NightSeer);
  run_seer_phase(provider);
  change_phase(provider, Phase::NightWolves);
  run_werewolf_phase(provider);
  change_phase(provider, Phase::NightWhiteWolfKing);
  run_white_wolf_king_phase(provider);
  change_phase(provider, Phase::NightWitch);
  run_witch_phase(provider);
  resolve_dawn(provider);
}

void GameEngine::play_day(DecisionProvider& provider) {
  if (!started_) {
    throw std::logic_error("必须先开始游戏。");
  }
  if (finished()) {
    return;
  }
  if (phase_ != Phase::Dawn) {
    throw std::logic_error("必须先完成夜晚结算才能进入白天。");
  }
  run_discussion(provider);
  if (!finished()) {
    run_vote(provider);
  }
  if (!finished() && night_ >= config_.max_nights) {
    finish(provider, MatchResult::Draw, "达到最大夜晚数，避免无限对局。");
  }
}

bool GameEngine::started() const { return started_; }
bool GameEngine::finished() const { return result_ != MatchResult::Ongoing; }
MatchResult GameEngine::result() const { return result_; }
unsigned int GameEngine::nights_played() const { return night_; }
Phase GameEngine::phase() const { return phase_; }
const std::vector<GameEvent>& GameEngine::events() const { return events_; }

MatchSnapshot GameEngine::snapshot() const {
  MatchSnapshot result;
  result.night = night_;
  result.phase = phase_;
  result.result = result_;
  result.players.reserve(players_.size());
  for (PlayerId id = 0; id < players_.size(); ++id) {
    const PlayerState& player = *players_[id];
    result.players.push_back(PlayerPublicView{
        .id = id,
        .alive = player.alive,
        .revealed_role = player.alive ? std::nullopt : std::optional<Role>(player.role),
    });
  }
  return result;
}

MatchReport GameEngine::report() const {
  MatchReport report;
  report.config = config_;
  report.result = result_;
  report.nights_played = night_;
  report.events = events_;
  report.players.reserve(players_.size());
  for (PlayerId id = 0; id < players_.size(); ++id) {
    report.players.push_back(PlayerReport{
        .id = id,
        .initial_role = initial_roles_.at(id),
        .final_role = players_[id]->role,
        .alive = players_[id]->alive,
    });
  }
  return report;
}

void GameEngine::emit(DecisionProvider& provider, GameEvent event) {
  event.night = night_;
  event.phase = phase_;
  if (event.visibility == EventVisibility::Private) {
    event.recipients = unique_recipients(std::move(event.recipients));
  }
  events_.push_back(std::move(event));
  provider.on_event(events_.back());
}

void GameEngine::change_phase(DecisionProvider& provider, Phase phase) {
  phase_ = phase;
  emit(provider, GameEvent{.type = EventType::PhaseChanged,
                           .visibility = EventVisibility::Public,
                           .detail = std::string(phase_name(phase))});
}

void GameEngine::deal_roles(DecisionProvider& provider) {
  std::vector<Role> deal;
  deal.reserve(config_.player_count);
  for (const Role role : kRoles) {
    const int count = config_.roles.count(role);
    deal.insert(deal.end(), static_cast<std::size_t>(count), role);
  }
  if (scripted_deal_.empty()) {
    random_->shuffle(deal.begin(), deal.end());
  } else {
    deal = scripted_deal_;
  }

  players_.reserve(deal.size());
  initial_roles_ = deal;
  for (PlayerId id = 0; id < deal.size(); ++id) {
    players_.push_back(std::make_unique<PlayerState>(PlayerState{.role = deal[id]}));
    emit(provider, GameEvent{.type = EventType::RoleAssigned,
                             .visibility = EventVisibility::Private,
                             .target = id,
                             .role = deal[id],
                             .recipients = {id}});
  }
}

void GameEngine::run_magician_phase(DecisionProvider& provider) {
  if (magician_swap_used_) {
    return;
  }
  const PlayerId actor = actor_with_role(Role::Magician, provider);
  if (actor == kNoPlayer) {
    return;
  }
  const DecisionRequest request = request_for(actor, Phase::NightMagician);
  const Action action = request_action(provider, request);
  if (action.kind != ActionKind::SwapRoles) {
    return;
  }

  std::swap(players_[action.target]->role, players_[action.secondary_target]->role);
  magician_swap_used_ = true;
  emit(provider, GameEvent{.type = EventType::RolesSwapped,
                           .visibility = EventVisibility::Private,
                           .actor = actor,
                           .target = action.target,
                           .secondary_target = action.secondary_target,
                           .recipients = {actor, action.target, action.secondary_target}});
  for (const PlayerId player : {action.target, action.secondary_target}) {
    emit(provider, GameEvent{.type = EventType::RoleChanged,
                             .visibility = EventVisibility::Private,
                             .target = player,
                             .role = players_[player]->role,
                             .recipients = {player}});
  }
}

void GameEngine::run_seer_phase(DecisionProvider& provider) {
  const PlayerId actor = actor_with_role(Role::Seer, provider);
  if (actor == kNoPlayer) {
    return;
  }
  const DecisionRequest request = request_for(actor, Phase::NightSeer);
  const Action action = request_action(provider, request);
  if (action.kind != ActionKind::Investigate) {
    return;
  }

  players_[actor]->investigations.push_back(
      PlayerState::Investigation{.target = action.target, .role = role_of(action.target)});
  emit(provider, GameEvent{.type = EventType::InvestigationResult,
                           .visibility = EventVisibility::Private,
                           .actor = actor,
                           .target = action.target,
                           .role = role_of(action.target),
                           .recipients = {actor}});
}

void GameEngine::run_werewolf_phase(DecisionProvider& provider) {
  const PlayerId actor = actor_with_role(Role::Werewolf, provider);
  if (actor == kNoPlayer) {
    return;
  }
  const DecisionRequest request = request_for(actor, Phase::NightWolves);
  const Action action = request_action(provider, request);
  if (action.kind != ActionKind::WerewolfAttack) {
    return;
  }
  pending_werewolf_target_ = action.target;

  std::vector<PlayerId> wolf_team;
  for (PlayerId player = 0; player < players_.size(); ++player) {
    if (alive(player) && is_wolf_team(role_of(player))) {
      wolf_team.push_back(player);
    }
  }
  emit(provider, GameEvent{.type = EventType::WerewolfTargetSelected,
                           .visibility = EventVisibility::Private,
                           .actor = actor,
                           .target = action.target,
                           .recipients = std::move(wolf_team)});
}

void GameEngine::run_white_wolf_king_phase(DecisionProvider& provider) {
  const PlayerId actor = actor_with_role(Role::WhiteWolfKing, provider);
  if (actor == kNoPlayer) {
    return;
  }
  const DecisionRequest request = request_for(actor, Phase::NightWhiteWolfKing);
  const Action action = request_action(provider, request);
  if (action.kind != ActionKind::WhiteWolfKingAttack) {
    return;
  }
  pending_white_wolf_target_ = action.target;
  emit(provider, GameEvent{.type = EventType::WhiteWolfKingTargetSelected,
                           .visibility = EventVisibility::Private,
                           .actor = actor,
                           .target = action.target,
                           .recipients = {actor}});
}

void GameEngine::run_witch_phase(DecisionProvider& provider) {
  const PlayerId actor = actor_with_role(Role::Witch, provider);
  if (actor == kNoPlayer) {
    return;
  }
  const DecisionRequest request = request_for(actor, Phase::NightWitch);
  const Action action = request_action(provider, request);
  if (action.kind == ActionKind::WitchSave) {
    saved_werewolf_target_ = action.target;
    witch_save_used_ = true;
    emit(provider, GameEvent{.type = EventType::WitchSaved,
                             .visibility = EventVisibility::Private,
                             .actor = actor,
                             .target = action.target,
                             .recipients = {actor}});
  } else if (action.kind == ActionKind::WitchPoison) {
    pending_witch_poison_target_ = action.target;
    witch_poison_used_ = true;
    emit(provider, GameEvent{.type = EventType::WitchPoisoned,
                             .visibility = EventVisibility::Private,
                             .actor = actor,
                             .target = action.target,
                             .recipients = {actor}});
  }
}

void GameEngine::resolve_dawn(DecisionProvider& provider) {
  change_phase(provider, Phase::Dawn);
  std::vector<PendingDeath> deaths;
  if (pending_werewolf_target_ != kNoPlayer && pending_werewolf_target_ != saved_werewolf_target_) {
    deaths.push_back(PendingDeath{.target = pending_werewolf_target_,
                                  .cause = DeathCause::WerewolfAttack});
  }
  if (pending_white_wolf_target_ != kNoPlayer) {
    deaths.push_back(PendingDeath{.target = pending_white_wolf_target_,
                                  .cause = DeathCause::WhiteWolfKingAttack});
  }
  if (pending_witch_poison_target_ != kNoPlayer) {
    deaths.push_back(PendingDeath{.target = pending_witch_poison_target_,
                                  .cause = DeathCause::WitchPoison});
  }
  process_deaths(provider, std::move(deaths));
  check_winner(provider);
}

void GameEngine::run_discussion(DecisionProvider& provider) {
  change_phase(provider, Phase::DayDiscussion);
  std::vector<PlayerId> speakers = alive_players();
  random_->shuffle(speakers.begin(), speakers.end());
  for (const PlayerId actor : speakers) {
    if (!alive(actor)) {
      continue;
    }
    const DecisionRequest request = request_for(actor, Phase::DayDiscussion);
    const Action action = request_action(provider, request);
    if (action.kind != ActionKind::Speak) {
      continue;
    }
    emit(provider, GameEvent{.type = EventType::Speech,
                             .visibility = EventVisibility::Public,
                             .actor = actor,
                             .target = action.target,
                             .role = action.claimed_role,
                             .speech = action.speech});
  }
}

void GameEngine::run_vote(DecisionProvider& provider) {
  change_phase(provider, Phase::DayVote);
  const std::vector<PlayerId> voters = alive_players();
  std::vector<int> totals(players_.size(), 0);
  for (const PlayerId actor : voters) {
    if (!alive(actor)) {
      continue;
    }
    const DecisionRequest request = request_for(actor, Phase::DayVote);
    const Action action = request_action(provider, request);
    if (action.kind != ActionKind::Vote) {
      continue;
    }
    ++totals[action.target];
    emit(provider, GameEvent{.type = EventType::VoteCast,
                             .visibility = EventVisibility::Public,
                             .actor = actor,
                             .target = action.target});
  }

  int highest_votes = 0;
  std::vector<PlayerId> candidates;
  for (const PlayerId player : alive_players()) {
    if (totals[player] > highest_votes) {
      highest_votes = totals[player];
      candidates.assign(1, player);
    } else if (totals[player] == highest_votes && highest_votes > 0) {
      candidates.push_back(player);
    }
  }

  if (candidates.size() != 1) {
    emit(provider, GameEvent{.type = EventType::VoteResolved,
                             .visibility = EventVisibility::Public,
                             .detail = "tie"});
    return;
  }

  const PlayerId eliminated = candidates.front();
  emit(provider, GameEvent{.type = EventType::VoteResolved,
                           .visibility = EventVisibility::Public,
                           .target = eliminated});
  process_deaths(provider, {PendingDeath{.target = eliminated, .cause = DeathCause::Vote}});
  check_winner(provider);
}

DecisionRequest GameEngine::request_for(PlayerId actor, Phase phase) const {
  DecisionRequest request;
  request.night = night_;
  request.phase = phase;
  request.actor = actor;
  request.players = snapshot().players;
  request.available_targets = targets_for(actor, phase);
  request.private_knowledge = knowledge_for(actor, phase);
  request.public_events = public_events();
  return request;
}

Action GameEngine::request_action(DecisionProvider& provider, const DecisionRequest& request) {
  Action action = provider.choose_action(request);
  if (!legal_action(action, request)) {
    emit(provider, GameEvent{.type = EventType::ActionRejected,
                             .visibility = EventVisibility::Private,
                             .actor = request.actor,
                             .recipients = {request.actor},
                             .detail = "非法操作已替换为跳过。"});
    action = default_action(request);
  }
  record_reason(provider, request, action);
  return action;
}

bool GameEngine::legal_action(const Action& action, const DecisionRequest& request) const {
  if (action.kind == ActionKind::Skip) {
    return true;
  }
  const auto target_allowed = [&request, &action] { return contains(request.available_targets, action.target); };
  switch (request.phase) {
    case Phase::NightMagician:
      return action.kind == ActionKind::SwapRoles && !magician_swap_used_ && target_allowed() &&
             contains(request.available_targets, action.secondary_target) &&
             action.target != action.secondary_target;
    case Phase::NightSeer:
      return action.kind == ActionKind::Investigate && target_allowed();
    case Phase::NightWolves:
      return action.kind == ActionKind::WerewolfAttack && target_allowed();
    case Phase::NightWhiteWolfKing:
      return action.kind == ActionKind::WhiteWolfKingAttack && target_allowed();
    case Phase::NightWitch:
      if (action.kind == ActionKind::WitchSave) {
        return !witch_save_used_ && request.private_knowledge.known_werewolf_target.has_value() &&
               action.target == *request.private_knowledge.known_werewolf_target;
      }
      return action.kind == ActionKind::WitchPoison && !witch_poison_used_ && target_allowed() &&
             action.target != request.actor;
    case Phase::DayDiscussion:
      if (action.kind != ActionKind::Speak) {
        return false;
      }
      if (action.speech == SpeechKind::ClaimRole) {
        return action.claimed_role.has_value();
      }
      if (action.speech == SpeechKind::Accuse || action.speech == SpeechKind::Defend) {
        return target_allowed();
      }
      return action.speech == SpeechKind::ClaimGood || action.speech == SpeechKind::Silence;
    case Phase::DayVote:
      return action.kind == ActionKind::Vote && target_allowed();
    case Phase::DeathTrigger:
      return action.kind == ActionKind::DeathShot && target_allowed() &&
             action.target != request.actor;
    case Phase::Dawn:
    case Phase::Setup:
    case Phase::Finished:
      return false;
  }

  if (request.phase == Phase::Finished) {
    return false;
  }
  return false;
}

Action GameEngine::default_action(const DecisionRequest& request) const {
  if (request.phase == Phase::DayDiscussion) {
    Action action;
    action.kind = ActionKind::Speak;
    action.speech = SpeechKind::Silence;
    return action;
  }
  if (request.phase == Phase::DayVote && !request.available_targets.empty()) {
    Action action;
    action.kind = ActionKind::Vote;
    action.target = request.available_targets.front();
    return action;
  }
  return Action::skip();
}

void GameEngine::record_reason(DecisionProvider& provider, const DecisionRequest& request,
                               const Action& action) {
  if (action.rationale.empty() && action.suspicion_snapshot.empty()) {
    return;
  }
  emit(provider, GameEvent{.type = EventType::DecisionReason,
                           .visibility = EventVisibility::Private,
                           .actor = request.actor,
                           .target = action.target,
                           .suspicion_snapshot = action.suspicion_snapshot,
                           .recipients = {request.actor},
                           .detail = action.rationale});
}

std::vector<PlayerId> GameEngine::alive_players() const {
  std::vector<PlayerId> result;
  result.reserve(players_.size());
  for (PlayerId id = 0; id < players_.size(); ++id) {
    if (players_[id]->alive) {
      result.push_back(id);
    }
  }
  return result;
}

std::vector<PlayerId> GameEngine::targets_for(PlayerId actor, Phase phase) const {
  std::vector<PlayerId> targets;
  if (!valid_player(actor, players_.size())) {
    return targets;
  }
  for (const PlayerId player : alive_players()) {
    switch (phase) {
      case Phase::NightMagician:
        targets.push_back(player);
        break;
      case Phase::NightSeer:
      case Phase::DayDiscussion:
      case Phase::DeathTrigger:
        if (player != actor) {
          targets.push_back(player);
        }
        break;
      case Phase::NightWolves:
      case Phase::NightWhiteWolfKing:
        if (!is_wolf_team(role_of(player))) {
          targets.push_back(player);
        }
        break;
      case Phase::NightWitch:
        targets.push_back(player);
        break;
      case Phase::DayVote:
        targets.push_back(player);
        break;
      case Phase::Dawn:
      case Phase::Setup:
      case Phase::Finished:
        break;
    }
  }
  return targets;
}

PlayerId GameEngine::actor_with_role(Role role, const DecisionProvider& provider) const {
  const PlayerId controlled = [&] {
    for (const PlayerId player : alive_players()) {
      if (role_of(player) == role && provider.controls(player)) {
        return player;
      }
    }
    return kNoPlayer;
  }();
  return controlled == kNoPlayer ? first_alive_with_role(role) : controlled;
}

PlayerId GameEngine::first_alive_with_role(Role role) const {
  for (const PlayerId player : alive_players()) {
    if (role_of(player) == role) {
      return player;
    }
  }
  return kNoPlayer;
}

bool GameEngine::alive(PlayerId player) const {
  return valid_player(player, players_.size()) && players_[player]->alive;
}

Role GameEngine::role_of(PlayerId player) const {
  if (!valid_player(player, players_.size())) {
    throw std::out_of_range("玩家编号越界。");
  }
  return players_[player]->role;
}

PrivateKnowledge GameEngine::knowledge_for(PlayerId actor, Phase phase) const {
  PrivateKnowledge knowledge;
  knowledge.role = role_of(actor);
  if (is_wolf_team(knowledge.role)) {
    for (PlayerId player = 0; player < players_.size(); ++player) {
      if (player != actor && is_wolf_team(role_of(player))) {
        knowledge.facts.push_back(
            KnownFact{.player = player, .role = role_of(player), .source = KnowledgeSource::WolfPack});
      }
    }
  }
  for (const PlayerState::Investigation& investigation : players_[actor]->investigations) {
    knowledge.facts.push_back(KnownFact{.player = investigation.target,
                                        .role = investigation.role,
                                        .source = KnowledgeSource::Investigation});
  }
  std::sort(knowledge.facts.begin(), knowledge.facts.end(), [](const KnownFact& lhs, const KnownFact& rhs) {
    if (lhs.player != rhs.player) {
      return lhs.player < rhs.player;
    }
    return static_cast<int>(lhs.source) < static_cast<int>(rhs.source);
  });
  knowledge.magician_swap_available = knowledge.role == Role::Magician && !magician_swap_used_;
  knowledge.witch_save_available = knowledge.role == Role::Witch && !witch_save_used_;
  knowledge.witch_poison_available = knowledge.role == Role::Witch && !witch_poison_used_;
  if (phase == Phase::NightWitch && knowledge.role == Role::Witch &&
      pending_werewolf_target_ != kNoPlayer) {
    knowledge.known_werewolf_target = pending_werewolf_target_;
  }
  return knowledge;
}

std::vector<GameEvent> GameEngine::public_events() const {
  std::vector<GameEvent> public_log;
  for (const GameEvent& event : events_) {
    if (event.visibility == EventVisibility::Public) {
      public_log.push_back(event);
    }
  }
  return public_log;
}

bool GameEngine::has_winner() const { return result_ != MatchResult::Ongoing; }

void GameEngine::check_winner(DecisionProvider& provider) {
  if (has_winner()) {
    return;
  }
  int wolves = 0;
  int village = 0;
  for (const PlayerId player : alive_players()) {
    if (is_wolf_team(role_of(player))) {
      ++wolves;
    } else {
      ++village;
    }
  }
  if (wolves == 0) {
    finish(provider, MatchResult::VillageWin);
  } else if (wolves >= village) {
    finish(provider, MatchResult::WolvesWin);
  }
}

void GameEngine::finish(DecisionProvider& provider, MatchResult result, std::string detail) {
  if (finished()) {
    return;
  }
  result_ = result;
  phase_ = Phase::Finished;
  emit(provider, GameEvent{.type = EventType::PhaseChanged,
                           .visibility = EventVisibility::Public,
                           .detail = std::string(phase_name(Phase::Finished))});
  std::string result_detail = std::string(result_name(result));
  if (!detail.empty()) {
    result_detail += "：" + detail;
  }
  emit(provider, GameEvent{.type = EventType::MatchFinished,
                           .visibility = EventVisibility::Public,
                           .detail = std::move(result_detail)});
}

void GameEngine::process_deaths(DecisionProvider& provider, std::vector<PendingDeath> pending) {
  std::size_t next = 0;
  while (next < pending.size()) {
    const PendingDeath death = pending[next];
    ++next;
    eliminate(provider, death.target, death.cause, death.source, pending);
  }
}

void GameEngine::eliminate(DecisionProvider& provider, PlayerId player, DeathCause cause,
                           PlayerId source, std::vector<PendingDeath>& queue) {
  if (!alive(player)) {
    return;
  }
  PlayerState& state = *players_[player];
  state.alive = false;
  emit(provider, GameEvent{.type = EventType::PlayerDied,
                           .visibility = EventVisibility::Public,
                           .actor = source,
                           .target = player,
                           .death_cause = cause});
  emit(provider, GameEvent{.type = EventType::RoleRevealed,
                           .visibility = EventVisibility::Public,
                           .target = player,
                           .role = state.role});

  if (state.role == Role::Hunter || state.role == Role::WhiteWolfKing) {
    const DecisionRequest request = request_for(player, Phase::DeathTrigger);
    const Action action = request_action(provider, request);
    if (action.kind == ActionKind::DeathShot) {
      const DeathCause shot_cause =
          state.role == Role::Hunter ? DeathCause::HunterShot : DeathCause::WhiteWolfKingShot;
      emit(provider, GameEvent{.type = EventType::DeathShot,
                               .visibility = EventVisibility::Public,
                               .actor = player,
                               .target = action.target,
                               .death_cause = shot_cause});
      queue.push_back(PendingDeath{.target = action.target, .cause = shot_cause, .source = player});
    }
  }

  provider.on_player_eliminated(
      EliminationNotice{.player = player, .cause = cause, .snapshot = snapshot()});
}

}  // namespace were_oier
