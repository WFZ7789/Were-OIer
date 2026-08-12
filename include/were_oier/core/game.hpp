#pragma once

#include "were_oier/core/types.hpp"

#include <memory>
#include <vector>

namespace were_oier {

class DecisionProvider {
 public:
  virtual ~DecisionProvider() = default;

  /// Returns whether this player is controlled interactively instead of by AI.
  [[nodiscard]] virtual bool controls(PlayerId player) const = 0;

  /// Supplies one structured action for the actor. Invalid actions safely become a skip.
  virtual Action choose_action(const DecisionRequest& request) = 0;

  /// Receives every event. The provider must respect EventVisibility before displaying it.
  virtual void on_event(const GameEvent& event) { (void)event; }

  /// Called after a death and any death-triggered skill. Providers may transfer control.
  virtual void on_player_eliminated(const EliminationNotice& notice) { (void)notice; }
};

/// Deterministic, UI-independent werewolf rules engine.
class GameEngine {
 public:
  /// A non-empty scripted deal assigns roles by zero-based seat and is useful for tests/replays.
  explicit GameEngine(GameConfig config, std::vector<Role> scripted_deal = {});
  ~GameEngine();
  GameEngine(GameEngine&&) noexcept;
  GameEngine& operator=(GameEngine&&) noexcept;
  GameEngine(const GameEngine&) = delete;
  GameEngine& operator=(const GameEngine&) = delete;

  void start(DecisionProvider& provider);
  void play_night(DecisionProvider& provider);
  void play_day(DecisionProvider& provider);

  [[nodiscard]] bool started() const;
  [[nodiscard]] bool finished() const;
  [[nodiscard]] MatchResult result() const;
  [[nodiscard]] unsigned int nights_played() const;
  [[nodiscard]] Phase phase() const;
  [[nodiscard]] MatchSnapshot snapshot() const;
  [[nodiscard]] MatchReport report() const;
  [[nodiscard]] const std::vector<GameEvent>& events() const;

 private:
  struct PlayerState;
  struct PendingDeath;

  GameConfig config_;
  std::vector<Role> scripted_deal_;
  std::vector<std::unique_ptr<PlayerState>> players_;
  std::vector<GameEvent> events_;
  std::vector<Role> initial_roles_;
  Phase phase_{Phase::Setup};
  MatchResult result_{MatchResult::Ongoing};
  unsigned int night_{0};
  bool started_{false};
  bool magician_swap_used_{false};
  bool witch_save_used_{false};
  bool witch_poison_used_{false};
  PlayerId pending_werewolf_target_{kNoPlayer};
  PlayerId saved_werewolf_target_{kNoPlayer};
  PlayerId pending_white_wolf_target_{kNoPlayer};
  PlayerId pending_witch_poison_target_{kNoPlayer};

  class Random;
  std::unique_ptr<Random> random_;

  void emit(DecisionProvider& provider, GameEvent event);
  void change_phase(DecisionProvider& provider, Phase phase);
  void deal_roles(DecisionProvider& provider);
  void run_magician_phase(DecisionProvider& provider);
  void run_seer_phase(DecisionProvider& provider);
  void run_werewolf_phase(DecisionProvider& provider);
  void run_white_wolf_king_phase(DecisionProvider& provider);
  void run_witch_phase(DecisionProvider& provider);
  void resolve_dawn(DecisionProvider& provider);
  void run_discussion(DecisionProvider& provider);
  void run_vote(DecisionProvider& provider);

  [[nodiscard]] DecisionRequest request_for(PlayerId actor, Phase phase) const;
  [[nodiscard]] Action request_action(DecisionProvider& provider, const DecisionRequest& request);
  [[nodiscard]] bool legal_action(const Action& action, const DecisionRequest& request) const;
  [[nodiscard]] Action default_action(const DecisionRequest& request) const;
  void record_reason(DecisionProvider& provider, const DecisionRequest& request, const Action& action);

  [[nodiscard]] std::vector<PlayerId> alive_players() const;
  [[nodiscard]] std::vector<PlayerId> targets_for(PlayerId actor, Phase phase) const;
  [[nodiscard]] PlayerId actor_with_role(Role role, const DecisionProvider& provider) const;
  [[nodiscard]] PlayerId first_alive_with_role(Role role) const;
  [[nodiscard]] bool alive(PlayerId player) const;
  [[nodiscard]] Role role_of(PlayerId player) const;
  [[nodiscard]] PrivateKnowledge knowledge_for(PlayerId actor, Phase phase) const;
  [[nodiscard]] std::vector<GameEvent> public_events() const;
  [[nodiscard]] bool has_winner() const;
  void check_winner(DecisionProvider& provider);
  void finish(DecisionProvider& provider, MatchResult result, std::string detail = {});
  void process_deaths(DecisionProvider& provider, std::vector<PendingDeath> pending);
  void eliminate(DecisionProvider& provider, PlayerId player, DeathCause cause,
                 PlayerId source, std::vector<PendingDeath>& queue);
};

}  // namespace were_oier
