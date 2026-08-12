#include "were_oier/game.hpp"
#include "were_oier/input.hpp"

#include <functional>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

using were_oier::DiscussionAction;
using were_oier::Faction;
using were_oier::Game;
using were_oier::NightActionKind;
using were_oier::Phase;
using were_oier::Role;
using were_oier::TalkKind;

void require(bool condition, const std::string& message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

void run_ai_cycle(Game& game) {
    if (game.state().phase == Phase::Night) {
        game.run_ai_night();
        game.resolve_night();
    }
    if (game.state().phase == Phase::Discussion) {
        game.run_ai_discussion_round();
        game.begin_voting();
    }
    if (game.state().phase == Phase::Voting) {
        game.run_ai_votes();
        static_cast<void>(game.resolve_vote());
    }
}

void test_standard_deck() {
    Game game(7, 0);
    int wolves = 0;
    int seers = 0;
    int witches = 0;
    int hunters = 0;
    int villagers = 0;
    for (int seat = 1; seat <= were_oier::kSeatCount; ++seat) {
        switch (game.state().players[seat].role) {
        case Role::Werewolf:
            ++wolves;
            break;
        case Role::Seer:
            ++seers;
            break;
        case Role::Witch:
            ++witches;
            break;
        case Role::Hunter:
            ++hunters;
            break;
        case Role::Villager:
            ++villagers;
            break;
        }
    }
    require(wolves == 2 && seers == 1 && witches == 1 && hunters == 1 && villagers == 5,
            "standard deck composition changed");
}

void test_deterministic_full_game() {
    Game first(123456, 0);
    Game second(123456, 0);
    require(first.digest() == second.digest(), "same seed differs at creation");
    for (int cycle = 0; cycle < 20 && first.state().phase != Phase::Finished; ++cycle) {
        run_ai_cycle(first);
        run_ai_cycle(second);
        require(first.digest() == second.digest(), "same seed diverged during simulation");
    }
    require(first.state().phase == Phase::Finished, "AI-only game did not terminate");
    require(first.state().winner.has_value(), "finished game has no winner");
}

void test_many_seeds_terminate() {
    for (std::uint64_t seed = 1; seed <= 250; ++seed) {
        Game game(seed, 0);
        int cycles = 0;
        while (game.state().phase != Phase::Finished && cycles < 20) {
            run_ai_cycle(game);
            ++cycles;
        }
        require(game.state().phase == Phase::Finished,
                "seed " + std::to_string(seed) + " did not terminate");
        require(game.state().day <= 20, "game exceeded the bounded day count");
    }
}

void test_discussion_changes_beliefs_and_vote() {
    Game game(42, 0);
    game.resolve_night(); // An empty night is legal and useful for deterministic setup.

    int target = 0;
    int observer = 0;
    for (int seat = 1; seat <= were_oier::kSeatCount; ++seat) {
        if (game.state().players[seat].role != Role::Werewolf && target == 0) {
            target = seat;
        } else if (game.state().players[seat].role != Role::Werewolf && observer == 0) {
            observer = seat;
        }
    }
    require(target != 0 && observer != 0 && target != observer, "test seats unavailable");
    const int before = game.suspicion(observer, target);

    int accusations = 0;
    for (int speaker = 1; speaker <= were_oier::kSeatCount && accusations < 5; ++speaker) {
        if (speaker == target || speaker == observer) {
            continue;
        }
        require(game.apply_discussion({speaker, TalkKind::Accuse, target, std::nullopt}),
                "valid accusation rejected");
        ++accusations;
    }
    require(game.suspicion(observer, target) > before,
            "accusation did not change the observer's belief");

    game.begin_voting();
    game.run_ai_votes();
    require(game.vote_of(observer) == target,
            "AI vote was not derived from the strengthened suspicion");
}

void test_private_seer_information() {
    Game game(99, 0);
    int seer = 0;
    int wolf = 0;
    for (int seat = 1; seat <= were_oier::kSeatCount; ++seat) {
        if (game.state().players[seat].role == Role::Seer) {
            seer = seat;
        } else if (game.state().players[seat].role == Role::Werewolf) {
            wolf = seat;
        }
    }
    require(game.submit_night_action({seer, NightActionKind::Inspect, wolf}),
            "seer inspection rejected");
    game.resolve_night();

    bool found_private_result = false;
    for (const auto& event : game.events()) {
        if (event.visible_to == seer && event.text.find("查验") != std::string::npos) {
            found_private_result = true;
            require(game.can_view(event, seer), "seer cannot view own result");
            const int other = seer == 1 ? 2 : 1;
            require(!game.can_view(event, other), "seer result leaked to another player");
        }
    }
    require(found_private_result, "seer inspection result was not emitted");
}

void test_witch_exile_regression() {
    Game game(2024, 0);
    game.resolve_night();
    int witch = 0;
    int fallback = 0;
    for (int seat = 1; seat <= were_oier::kSeatCount; ++seat) {
        if (game.state().players[seat].role == Role::Witch) {
            witch = seat;
        } else {
            fallback = seat;
        }
    }
    game.begin_voting();
    for (int voter = 1; voter <= were_oier::kSeatCount; ++voter) {
        const int target = voter == witch ? fallback : witch;
        require(game.cast_vote(voter, target), "setup vote rejected");
    }
    const auto exiled = game.resolve_vote();
    require(exiled == witch, "witch was not exiled in regression setup");
    require(!game.state().players[witch].alive, "exiled witch remained alive");
    require(game.state().phase == Phase::Night || game.state().phase == Phase::Finished,
            "witch exile left the game in an invalid phase");
}

void test_invalid_input_and_actions() {
    require(!were_oier::parse_integer("English discussion", 1, 10).has_value(),
            "English input was parsed as a seat");
    require(!were_oier::parse_integer("2x", 1, 10).has_value(),
            "trailing input was accepted");
    require(were_oier::parse_integer(" 10 ", 1, 10) == 10,
            "trimmed valid input was rejected");

    Game game(88, 0);
    std::vector<int> wolves;
    for (int seat = 1; seat <= were_oier::kSeatCount; ++seat) {
        if (game.state().players[seat].role == Role::Werewolf) {
            wolves.push_back(seat);
        }
    }
    require(wolves.size() == 2, "wolf setup failed");
    std::string error;
    require(!game.submit_night_action({wolves[0], NightActionKind::Kill, wolves[1]}, &error),
            "wolf was allowed to attack a teammate");
    require(!error.empty(), "invalid action did not explain rejection");
}

} // namespace

int main() {
    const std::vector<std::pair<std::string, std::function<void()>>> tests{
        {"standard deck", test_standard_deck},
        {"deterministic full game", test_deterministic_full_game},
        {"250 seeds terminate", test_many_seeds_terminate},
        {"discussion drives vote", test_discussion_changes_beliefs_and_vote},
        {"private seer information", test_private_seer_information},
        {"witch exile regression", test_witch_exile_regression},
        {"invalid input and actions", test_invalid_input_and_actions},
    };

    int failures = 0;
    for (const auto& [name, test] : tests) {
        try {
            test();
            std::cout << "[PASS] " << name << '\n';
        } catch (const std::exception& error) {
            ++failures;
            std::cerr << "[FAIL] " << name << ": " << error.what() << '\n';
        }
    }
    std::cout << tests.size() - static_cast<std::size_t>(failures) << '/' << tests.size()
              << " tests passed\n";
    return failures == 0 ? 0 : 1;
}
