#pragma once

#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace were_oier {

using PlayerId = std::size_t;
inline constexpr PlayerId kNoPlayer = std::numeric_limits<PlayerId>::max();

enum class Role : std::uint8_t {
  Villager,
  Werewolf,
  WhiteWolfKing,
  Seer,
  Witch,
  Hunter,
  Magician,
};

enum class Team : std::uint8_t {
  Village,
  Wolves,
};

enum class Phase : std::uint8_t {
  Setup,
  NightMagician,
  NightSeer,
  NightWolves,
  NightWhiteWolfKing,
  NightWitch,
  Dawn,
  DayDiscussion,
  DayVote,
  DeathTrigger,
  Finished,
};

enum class MatchResult : std::uint8_t {
  Ongoing,
  VillageWin,
  WolvesWin,
  Draw,
};

enum class DeathCause : std::uint8_t {
  WerewolfAttack,
  WhiteWolfKingAttack,
  WitchPoison,
  Vote,
  HunterShot,
  WhiteWolfKingShot,
};

enum class SpeechKind : std::uint8_t {
  ClaimGood,
  ClaimRole,
  Accuse,
  Defend,
  Silence,
};

enum class ActionKind : std::uint8_t {
  Skip,
  SwapRoles,
  Investigate,
  WerewolfAttack,
  WhiteWolfKingAttack,
  WitchSave,
  WitchPoison,
  Speak,
  Vote,
  DeathShot,
};

enum class EventVisibility : std::uint8_t {
  Public,
  Private,
};

enum class EventType : std::uint8_t {
  MatchStarted,
  PhaseChanged,
  RoleAssigned,
  RoleChanged,
  RolesSwapped,
  InvestigationResult,
  WerewolfTargetSelected,
  WhiteWolfKingTargetSelected,
  WitchSaved,
  WitchPoisoned,
  PlayerDied,
  RoleRevealed,
  Speech,
  VoteCast,
  VoteResolved,
  DeathShot,
  ActionRejected,
  DecisionReason,
  MatchFinished,
};

[[nodiscard]] std::string_view role_name(Role role);
[[nodiscard]] std::string_view team_name(Team team);
[[nodiscard]] std::string_view phase_name(Phase phase);
[[nodiscard]] std::string_view death_cause_name(DeathCause cause);
[[nodiscard]] std::string_view speech_name(SpeechKind speech);
[[nodiscard]] std::string_view result_name(MatchResult result);
[[nodiscard]] Team team_of(Role role);
[[nodiscard]] bool is_wolf_team(Role role);
[[nodiscard]] bool is_unique_role(Role role);

struct RoleCounts {
  int villagers{3};
  int werewolves{2};
  int white_wolf_kings{1};
  int seers{1};
  int witches{1};
  int hunters{1};
  int magicians{1};

  [[nodiscard]] int count(Role role) const;
  [[nodiscard]] int total() const;
  [[nodiscard]] int wolf_team_total() const;
  [[nodiscard]] int village_team_total() const;
};

struct ConfigValidation {
  std::vector<std::string> errors;
  std::vector<std::string> warnings;

  [[nodiscard]] bool ok() const { return errors.empty(); }
};

struct GameConfig {
  std::size_t player_count{10};
  RoleCounts roles{};
  PlayerId human_seat{0};
  std::uint64_t seed{1};
  unsigned int max_nights{30};

  [[nodiscard]] ConfigValidation validate() const;
};

struct MatchPreset {
  std::string name;
  std::string description;
  GameConfig config;
};

[[nodiscard]] GameConfig standard_config();
[[nodiscard]] std::vector<MatchPreset> preset_configs();

struct SuspicionScore {
  PlayerId player{kNoPlayer};
  int score{0};
};

struct Action {
  ActionKind kind{ActionKind::Skip};
  PlayerId target{kNoPlayer};
  PlayerId secondary_target{kNoPlayer};
  SpeechKind speech{SpeechKind::Silence};
  std::optional<Role> claimed_role;
  std::string rationale;
  std::vector<SuspicionScore> suspicion_snapshot;

  [[nodiscard]] static Action skip(std::string rationale = {});
};

struct GameEvent {
  EventType type{EventType::MatchStarted};
  EventVisibility visibility{EventVisibility::Public};
  unsigned int night{0};
  Phase phase{Phase::Setup};
  PlayerId actor{kNoPlayer};
  PlayerId target{kNoPlayer};
  PlayerId secondary_target{kNoPlayer};
  std::optional<Role> role;
  std::optional<DeathCause> death_cause;
  std::optional<SpeechKind> speech;
  std::vector<SuspicionScore> suspicion_snapshot;
  std::vector<PlayerId> recipients;
  std::string detail;

  [[nodiscard]] bool visible_to(PlayerId player) const;
};

struct PlayerPublicView {
  PlayerId id{kNoPlayer};
  bool alive{false};
  std::optional<Role> revealed_role;
};

enum class KnowledgeSource : std::uint8_t {
  WolfPack,
  Investigation,
};

struct KnownFact {
  PlayerId player{kNoPlayer};
  Role role{Role::Villager};
  KnowledgeSource source{KnowledgeSource::Investigation};
};

struct PrivateKnowledge {
  Role role{Role::Villager};
  std::vector<KnownFact> facts;
  bool magician_swap_available{false};
  bool witch_save_available{false};
  bool witch_poison_available{false};
  std::optional<PlayerId> known_werewolf_target;
};

struct DecisionRequest {
  unsigned int night{0};
  Phase phase{Phase::Setup};
  PlayerId actor{kNoPlayer};
  std::vector<PlayerPublicView> players;
  std::vector<PlayerId> available_targets;
  PrivateKnowledge private_knowledge;
  std::vector<GameEvent> public_events;
};

struct MatchSnapshot {
  unsigned int night{0};
  Phase phase{Phase::Setup};
  MatchResult result{MatchResult::Ongoing};
  std::vector<PlayerPublicView> players;
};

struct EliminationNotice {
  PlayerId player{kNoPlayer};
  DeathCause cause{DeathCause::Vote};
  MatchSnapshot snapshot;
};

struct PlayerReport {
  PlayerId id{kNoPlayer};
  Role initial_role{Role::Villager};
  Role final_role{Role::Villager};
  bool alive{false};
};

struct MatchReport {
  GameConfig config;
  MatchResult result{MatchResult::Ongoing};
  unsigned int nights_played{0};
  std::vector<PlayerReport> players;
  std::vector<GameEvent> events;
};

}  // namespace were_oier
