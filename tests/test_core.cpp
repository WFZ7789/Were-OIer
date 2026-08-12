#include "were_oier/core/ai.hpp"
#include "were_oier/core/game.hpp"

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

namespace {

using were_oier::Action;
using were_oier::ActionKind;
using were_oier::AiController;
using were_oier::DecisionProvider;
using were_oier::DecisionRequest;
using were_oier::EliminationNotice;
using were_oier::EventType;
using were_oier::GameConfig;
using were_oier::GameEngine;
using were_oier::GameEvent;
using were_oier::MatchResult;
using were_oier::Phase;
using were_oier::PlayerId;
using were_oier::Role;
using were_oier::RoleCounts;

[[noreturn]] void fail(const std::string& message) {
  std::cerr << "test failure: " << message << '\n';
  std::exit(1);
}

void expect(bool condition, const std::string& message) {
  if (!condition) {
    fail(message);
  }
}

[[nodiscard]] bool contains_event(const std::vector<GameEvent>& events, EventType type,
                                  PlayerId target = were_oier::kNoPlayer) {
  return std::any_of(events.begin(), events.end(), [type, target](const GameEvent& event) {
    return event.type == type && (target == were_oier::kNoPlayer || event.target == target);
  });
}

class AiProvider final : public DecisionProvider {
 public:
  explicit AiProvider(std::uint64_t seed) : ai_(seed) {}

  [[nodiscard]] bool controls(PlayerId) const override { return false; }
  Action choose_action(const DecisionRequest& request) override {
    requests.push_back(request);
    return ai_.decide(request);
  }
  void on_event(const GameEvent& event) override { events.push_back(event); }

  std::vector<GameEvent> events;
  std::vector<DecisionRequest> requests;

 private:
  AiController ai_;
};

class ScriptedProvider final : public DecisionProvider {
 public:
  [[nodiscard]] bool controls(PlayerId) const override { return false; }

  Action choose_action(const DecisionRequest& request) override {
    requests.push_back(request);
    if (request.phase == Phase::NightMagician && swap_roles) {
      Action action;
      action.kind = ActionKind::SwapRoles;
      action.target = 0;
      action.secondary_target = 1;
      return action;
    }
    if (request.phase == Phase::NightWolves && wolf_target != were_oier::kNoPlayer) {
      Action action;
      action.kind = ActionKind::WerewolfAttack;
      action.target = wolf_target;
      return action;
    }
    if (request.phase == Phase::NightWitch && save_target != were_oier::kNoPlayer) {
      Action action;
      action.kind = ActionKind::WitchSave;
      action.target = save_target;
      return action;
    }
    if (request.phase == Phase::DeathTrigger && death_shot_target != were_oier::kNoPlayer) {
      Action action;
      action.kind = ActionKind::DeathShot;
      action.target = death_shot_target;
      return action;
    }
    if (request.phase == Phase::DayVote && force_tie_votes) {
      Action action;
      action.kind = ActionKind::Vote;
      action.target = request.actor < 3 ? 0 : 1;
      return action;
    }
    return Action::skip();
  }

  void on_event(const GameEvent& event) override { events.push_back(event); }
  void on_player_eliminated(const EliminationNotice& notice) override { eliminations.push_back(notice.player); }

  bool swap_roles{false};
  PlayerId wolf_target{were_oier::kNoPlayer};
  PlayerId save_target{were_oier::kNoPlayer};
  PlayerId death_shot_target{were_oier::kNoPlayer};
  bool force_tie_votes{false};
  std::vector<GameEvent> events;
  std::vector<DecisionRequest> requests;
  std::vector<PlayerId> eliminations;
};

[[nodiscard]] GameConfig config_for(RoleCounts counts, std::size_t players) {
  GameConfig config;
  config.player_count = players;
  config.roles = counts;
  config.seed = 424242;
  config.max_nights = 12;
  return config;
}

[[nodiscard]] std::string signature(const std::vector<GameEvent>& events) {
  std::string result;
  for (const GameEvent& event : events) {
    result += std::to_string(static_cast<int>(event.type));
    result += ':';
    result += std::to_string(event.actor);
    result += ':';
    result += std::to_string(event.target);
    result += ':';
    result += std::to_string(event.secondary_target);
    result += ':';
    result += event.role.has_value() ? std::to_string(static_cast<int>(*event.role)) : std::string{"-"};
    result += ':';
    result += event.death_cause.has_value() ? std::to_string(static_cast<int>(*event.death_cause))
                                             : std::string{"-"};
    result += ':';
    result += event.speech.has_value() ? std::to_string(static_cast<int>(*event.speech)) : std::string{"-"};
    result += ':';
    for (const auto& score : event.suspicion_snapshot) {
      result += std::to_string(score.player);
      result += '=';
      result += std::to_string(score.score);
      result += ',';
    }
    result += ':';
    result += event.detail;
    result += '\n';
  }
  return result;
}

void test_configuration_validation() {
  const GameConfig standard = were_oier::standard_config();
  expect(standard.validate().ok(), "standard ten-player configuration must be valid");

  GameConfig invalid = standard;
  invalid.roles.werewolves = 0;
  expect(!invalid.validate().ok(), "a deck without the wolf team must be rejected");

  invalid = standard;
  invalid.roles.seers = 2;
  expect(!invalid.validate().ok(), "unique special roles must be rejected when duplicated");

  const auto presets = were_oier::preset_configs();
  expect(!presets.empty(), "preset configurations must be present");
  for (const auto& preset : presets) {
    expect(preset.config.validate().ok(), "every built-in preset must be valid");
  }
}

void test_immediate_wolf_majority_finishes_at_start() {
  const GameConfig config = config_for(RoleCounts{3, 3, 0, 0, 0, 0, 0}, 6);
  const std::vector<Role> deal{
      Role::Werewolf, Role::Werewolf, Role::Werewolf,
      Role::Villager, Role::Villager, Role::Villager,
  };
  ScriptedProvider provider;
  GameEngine game(config, deal);
  game.start(provider);
  expect(game.finished(), "an initial wolf majority must end the match immediately");
  expect(game.result() == MatchResult::WolvesWin, "initial wolf majority must award wolves the win");
}

void test_witch_save_prevents_the_wolf_kill() {
  const GameConfig config = config_for(RoleCounts{2, 1, 0, 1, 1, 1, 0}, 6);
  const std::vector<Role> deal{
      Role::Werewolf, Role::Witch, Role::Seer, Role::Hunter, Role::Villager, Role::Villager,
  };
  ScriptedProvider provider;
  provider.wolf_target = 4;
  provider.save_target = 4;
  GameEngine game(config, deal);
  game.start(provider);
  game.play_night(provider);

  expect(contains_event(provider.events, EventType::WitchSaved, 4), "witch save must be logged");
  expect(!contains_event(provider.events, EventType::PlayerDied, 4),
         "a saved normal-wolf target must survive dawn");
}

void test_witch_save_is_a_single_use_resource() {
  const GameConfig config = config_for(RoleCounts{2, 1, 0, 1, 1, 1, 0}, 6);
  const std::vector<Role> deal{
      Role::Werewolf, Role::Witch, Role::Seer, Role::Hunter, Role::Villager, Role::Villager,
  };
  ScriptedProvider provider;
  provider.wolf_target = 4;
  provider.save_target = 4;
  provider.force_tie_votes = true;
  GameEngine game(config, deal);
  game.start(provider);
  game.play_night(provider);
  game.play_day(provider);
  game.play_night(provider);

  const auto saves = static_cast<int>(std::count_if(provider.events.begin(), provider.events.end(),
                                                     [](const GameEvent& event) {
                                                       return event.type == EventType::WitchSaved;
                                                     }));
  expect(saves == 1, "witch save potion must not be reusable");
  expect(contains_event(provider.events, EventType::ActionRejected),
         "a repeated save attempt must be rejected by the rules engine");
  expect(contains_event(provider.events, EventType::PlayerDied, 4),
         "the second unprotected wolf attack must resolve normally");
}

void test_death_trigger_chains() {
  const GameConfig config = config_for(RoleCounts{1, 1, 1, 1, 1, 1, 0}, 6);
  const std::vector<Role> deal{
      Role::Werewolf, Role::WhiteWolfKing, Role::Hunter,
      Role::Seer, Role::Witch, Role::Villager,
  };
  ScriptedProvider provider;
  provider.wolf_target = 2;
  provider.death_shot_target = 1;
  GameEngine game(config, deal);
  game.start(provider);
  game.play_night(provider);

  expect(contains_event(provider.events, EventType::DeathShot, 1),
         "hunter death must permit a death-triggered shot");
  expect(contains_event(provider.events, EventType::PlayerDied, 1),
         "the death-triggered shot target must be eliminated");
  expect(std::find(provider.eliminations.begin(), provider.eliminations.end(), 2) !=
             provider.eliminations.end(),
         "hunter elimination must notify the provider");
}

void test_magician_swaps_cards_before_other_night_roles() {
  const GameConfig config = config_for(RoleCounts{1, 1, 0, 1, 1, 1, 1}, 6);
  const std::vector<Role> deal{
      Role::Werewolf, Role::Magician, Role::Seer,
      Role::Witch, Role::Hunter, Role::Villager,
  };
  ScriptedProvider provider;
  provider.swap_roles = true;
  GameEngine game(config, deal);
  game.start(provider);
  game.play_night(provider);
  const auto report = game.report();

  expect(contains_event(provider.events, EventType::RolesSwapped), "magic swap must be logged");
  expect(report.players.at(0).final_role == Role::Magician,
         "first swapped player must hold the magician card afterwards");
  expect(report.players.at(1).final_role == Role::Werewolf,
         "second swapped player must hold the werewolf card afterwards");
}

void test_ai_is_deterministic_and_has_no_hidden_roles() {
  const GameConfig config = config_for(RoleCounts{2, 1, 0, 1, 1, 1, 0}, 6);
  const std::vector<Role> deal{
      Role::Werewolf, Role::Witch, Role::Seer, Role::Hunter, Role::Villager, Role::Villager,
  };

  AiProvider first_provider(99);
  GameEngine first_game(config, deal);
  first_game.start(first_provider);
  for (int turns = 0; turns < 20 && !first_game.finished(); ++turns) {
    first_game.play_night(first_provider);
    if (!first_game.finished()) {
      first_game.play_day(first_provider);
    }
  }
  expect(first_game.finished(), "AI match must terminate under the night cap");

  AiProvider second_provider(99);
  GameEngine second_game(config, deal);
  second_game.start(second_provider);
  for (int turns = 0; turns < 20 && !second_game.finished(); ++turns) {
    second_game.play_night(second_provider);
    if (!second_game.finished()) {
      second_game.play_day(second_provider);
    }
  }
  expect(second_game.finished(), "replayed AI match must terminate");
  expect(signature(first_provider.events) == signature(second_provider.events),
         "same deal and AI seed must produce the same event timeline");
  const bool has_suspicion_trace = std::any_of(first_provider.events.begin(), first_provider.events.end(),
                                                [](const GameEvent& event) {
                                                  return event.type == EventType::DecisionReason &&
                                                         !event.suspicion_snapshot.empty();
                                                });
  expect(has_suspicion_trace, "AI decisions must retain a post-match suspicion snapshot");

  const auto villager_request = std::find_if(first_provider.requests.begin(), first_provider.requests.end(),
                                             [](const DecisionRequest& request) {
                                               return request.private_knowledge.role == Role::Villager;
                                             });
  expect(villager_request != first_provider.requests.end(), "villager AI must receive a decision request");
  expect(villager_request->private_knowledge.facts.empty(),
         "villager AI must not receive hidden role facts");
  const bool leaks_alive_role = std::any_of(villager_request->players.begin(), villager_request->players.end(),
                                            [](const auto& player) {
                                              return player.alive && player.revealed_role.has_value();
                                            });
  expect(!leaks_alive_role, "public AI view must not reveal roles of alive players");
}

}  // namespace

int main() {
  test_configuration_validation();
  test_immediate_wolf_majority_finishes_at_start();
  test_witch_save_prevents_the_wolf_kill();
  test_witch_save_is_a_single_use_resource();
  test_death_trigger_chains();
  test_magician_swaps_cards_before_other_night_roles();
  test_ai_is_deterministic_and_has_no_hidden_roles();
  std::cout << "all core tests passed\n";
  return 0;
}
