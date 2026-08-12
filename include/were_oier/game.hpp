#pragma once

#include "were_oier/types.hpp"

#include <array>
#include <cstdint>
#include <optional>
#include <random>
#include <string>
#include <vector>

namespace were_oier {

class Game {
public:
    explicit Game(std::uint64_t seed, int human_seat = 10);

    [[nodiscard]] const GameState& state() const noexcept { return state_; }
    [[nodiscard]] const std::vector<Event>& events() const noexcept { return events_; }
    [[nodiscard]] bool can_view(const Event& event, int viewer) const noexcept;
    [[nodiscard]] int suspicion(int observer, int target) const;
    [[nodiscard]] int credibility(int observer, int speaker) const;
    [[nodiscard]] std::optional<int> vote_of(int voter) const;
    [[nodiscard]] std::optional<int> preview_wolf_target() const;
    [[nodiscard]] std::string digest() const;

    bool submit_night_action(const NightAction& action, std::string* error = nullptr);
    void run_ai_night();
    void resolve_night();

    bool apply_discussion(const DiscussionAction& action, std::string* error = nullptr);
    void run_ai_discussion_round();

    void begin_voting();
    bool cast_vote(int voter, int target, std::string* error = nullptr);
    void run_ai_votes();
    [[nodiscard]] std::optional<int> resolve_vote();

private:
    [[nodiscard]] bool valid_alive_seat(int seat) const noexcept;
    [[nodiscard]] bool has_night_action(int actor) const noexcept;
    [[nodiscard]] int most_suspicious_target(int observer, bool avoid_wolves = false) const;
    [[nodiscard]] int least_suspicious_target(int observer) const;
    [[nodiscard]] int choose_wolf_target() const;
    [[nodiscard]] std::optional<int> choose_hunter_target(int hunter) const;
    void eliminate(int seat, const std::string& reason);
    void evaluate_winner();
    void update_beliefs_after_reveal(int revealed_seat);
    void add_event(int visible_to, const std::string& text);
    void set_error(std::string* error, const std::string& text) const;

    GameState state_;
    std::array<BeliefState, kSeatCount + 1> beliefs_{};
    std::vector<NightAction> night_actions_;
    std::array<int, kSeatCount + 1> votes_{};
    std::vector<Event> events_;
    std::mt19937_64 rng_;
};

} // namespace were_oier

