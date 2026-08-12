#pragma once

#include <array>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace were_oier {

inline constexpr int kSeatCount = 10;

enum class Role {
    Villager,
    Werewolf,
    Seer,
    Witch,
    Hunter,
};

enum class Faction {
    Village,
    Wolves,
};

enum class Phase {
    Night,
    Discussion,
    Voting,
    Finished,
};

enum class NightActionKind {
    Kill,
    Inspect,
    Heal,
    Poison,
    Pass,
};

enum class TalkKind {
    Accuse,
    Defend,
    ClaimRole,
    Question,
    Pass,
};

struct PlayerState {
    int seat{};
    Role role{Role::Villager};
    bool alive{true};
    bool human{false};
    int cleverness{5};
    int boldness{5};
};

struct NightAction {
    int actor{};
    NightActionKind kind{NightActionKind::Pass};
    int target{};
};

struct DiscussionAction {
    int speaker{};
    TalkKind kind{TalkKind::Pass};
    int target{};
    std::optional<Role> claimed_role;
};

struct Event {
    int day{};
    Phase phase{Phase::Night};
    int visible_to{}; // 0 means public; otherwise only this seat can see it.
    std::string text;
};

struct BeliefState {
    std::array<int, kSeatCount + 1> suspicion{};
    std::array<int, kSeatCount + 1> credibility{};
    std::array<std::optional<Role>, kSeatCount + 1> claims{};
    std::array<std::optional<Faction>, kSeatCount + 1> known_faction{};
};

struct GameState {
    std::uint64_t seed{};
    int day{1};
    int human_seat{10};
    Phase phase{Phase::Night};
    std::optional<Faction> winner;
    std::array<PlayerState, kSeatCount + 1> players{};
    bool witch_heal_available{true};
    bool witch_poison_available{true};
};

[[nodiscard]] std::string to_string(Role role);
[[nodiscard]] std::string to_string(Faction faction);
[[nodiscard]] std::string to_string(Phase phase);
[[nodiscard]] Faction faction_of(Role role);

} // namespace were_oier

