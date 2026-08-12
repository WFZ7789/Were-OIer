#include "were_oier/game.hpp"
#include "were_oier/input.hpp"

#include <cstdint>
#include <exception>
#include <iostream>
#include <limits>
#include <string>

namespace {

using were_oier::DiscussionAction;
using were_oier::Game;
using were_oier::NightAction;
using were_oier::NightActionKind;
using were_oier::Phase;
using were_oier::Role;
using were_oier::TalkKind;

class EndOfInput final : public std::exception {};

int prompt_number(const std::string& prompt, int minimum, int maximum) {
    while (true) {
        std::cout << prompt;
        std::string line;
        if (!std::getline(std::cin, line)) {
            throw EndOfInput{};
        }
        if (const auto value = were_oier::parse_integer(line, minimum, maximum)) {
            return *value;
        }
        std::cout << "输入无效，请输入 " << minimum << " 到 " << maximum
                  << " 之间的整数。\n";
    }
}

int prompt_alive_target(const Game& game, int actor, const std::string& purpose,
                        bool avoid_wolves = false) {
    while (true) {
        std::cout << "存活玩家：";
        for (int seat = 1; seat <= were_oier::kSeatCount; ++seat) {
            const auto& player = game.state().players[seat];
            if (player.alive) {
                std::cout << ' ' << seat;
            }
        }
        std::cout << "\n";
        const int target = prompt_number(purpose, 1, were_oier::kSeatCount);
        const auto& player = game.state().players[target];
        if (!player.alive || target == actor) {
            std::cout << "目标必须是仍存活的其他玩家。\n";
            continue;
        }
        if (avoid_wolves && player.role == Role::Werewolf) {
            std::cout << "你不能袭击狼人队友。\n";
            continue;
        }
        return target;
    }
}

void render_new_events(const Game& game, std::size_t& cursor, int viewer) {
    const auto& events = game.events();
    while (cursor < events.size()) {
        const auto& event = events[cursor++];
        if (game.can_view(event, viewer)) {
            std::cout << "  " << event.text << '\n';
        }
    }
}

void prompt_early_night_action(Game& game, int human) {
    if (!game.state().players[human].alive) {
        return;
    }
    const Role role = game.state().players[human].role;
    if (role == Role::Werewolf) {
        std::cout << "狼人同伴：";
        for (int seat = 1; seat <= were_oier::kSeatCount; ++seat) {
            if (seat != human && game.state().players[seat].alive &&
                game.state().players[seat].role == Role::Werewolf) {
                std::cout << ' ' << seat << " 号";
            }
        }
        std::cout << '\n';
        const int target = prompt_alive_target(game, human, "选择今晚袭击的目标：", true);
        game.submit_night_action({human, NightActionKind::Kill, target});
    } else if (role == Role::Seer) {
        const int target = prompt_alive_target(game, human, "选择今晚查验的目标：");
        game.submit_night_action({human, NightActionKind::Inspect, target});
    }
}

void prompt_witch_action(Game& game, int human) {
    if (!game.state().players[human].alive ||
        game.state().players[human].role != Role::Witch) {
        return;
    }

    const auto victim = game.preview_wolf_target();
    if (victim.has_value()) {
        std::cout << "今晚的袭击目标是 " << *victim << " 号。\n";
    } else {
        std::cout << "今晚暂未发现狼人袭击目标。\n";
    }
    std::cout << "0. 不使用药剂";
    if (game.state().witch_heal_available && victim.has_value()) {
        std::cout << "  1. 使用解药";
    }
    if (game.state().witch_poison_available) {
        std::cout << "  2. 使用毒药";
    }
    std::cout << '\n';

    while (true) {
        const int choice = prompt_number("选择行动：", 0, 2);
        NightAction action{human, NightActionKind::Pass, 0};
        if (choice == 1 && game.state().witch_heal_available && victim.has_value()) {
            action = {human, NightActionKind::Heal, *victim};
        } else if (choice == 2 && game.state().witch_poison_available) {
            const int target = prompt_alive_target(game, human, "选择毒药目标：");
            action = {human, NightActionKind::Poison, target};
        } else if (choice != 0) {
            std::cout << "该药剂目前不可用。\n";
            continue;
        }
        std::string error;
        if (game.submit_night_action(action, &error)) {
            return;
        }
        std::cout << error << '\n';
    }
}

Role prompt_claimed_role() {
    std::cout << "1. 平民  2. 狼人  3. 预言家  4. 女巫  5. 猎人\n";
    switch (prompt_number("声明身份：", 1, 5)) {
    case 1:
        return Role::Villager;
    case 2:
        return Role::Werewolf;
    case 3:
        return Role::Seer;
    case 4:
        return Role::Witch;
    default:
        return Role::Hunter;
    }
}

void prompt_discussion_action(Game& game, int human) {
    if (!game.state().players[human].alive) {
        return;
    }
    std::cout << "你的发言行动：\n"
              << "1. 指控  2. 辩护  3. 声明身份  4. 追问  5. 暂不表态\n";
    const int choice = prompt_number("选择行动：", 1, 5);
    DiscussionAction action{human, TalkKind::Pass, 0, std::nullopt};
    if (choice == 1) {
        action = {human, TalkKind::Accuse,
                  prompt_alive_target(game, human, "指控几号："), std::nullopt};
    } else if (choice == 2) {
        action = {human, TalkKind::Defend,
                  prompt_alive_target(game, human, "为几号辩护："), std::nullopt};
    } else if (choice == 3) {
        action = {human, TalkKind::ClaimRole, 0, prompt_claimed_role()};
    } else if (choice == 4) {
        action = {human, TalkKind::Question,
                  prompt_alive_target(game, human, "追问几号："), std::nullopt};
    }
    std::string error;
    if (!game.apply_discussion(action, &error)) {
        throw std::logic_error("terminal created invalid discussion action: " + error);
    }
}

std::uint64_t parse_seed(int argc, char** argv) {
    if (argc < 2) {
        return 20260812ULL;
    }
    try {
        std::size_t used = 0;
        const std::uint64_t seed = std::stoull(argv[1], &used);
        if (argv[1][used] != '\0') {
            throw std::invalid_argument("trailing input");
        }
        return seed;
    } catch (const std::exception&) {
        throw std::invalid_argument("种子必须是无符号整数");
    }
}

} // namespace

int main(int argc, char** argv) {
    try {
        const std::uint64_t seed = parse_seed(argc, argv);
        constexpr int human = 10;
        Game game(seed, human);
        std::size_t event_cursor = 0;

        std::cout << "Were-OIer v0.3 vertical slice\n"
                  << "种子：" << seed << "，你是 10 号玩家。\n";
        render_new_events(game, event_cursor, human);

        while (game.state().phase != Phase::Finished) {
            if (game.state().phase == Phase::Night) {
                std::cout << "\n== 第 " << game.state().day << " 天夜晚 ==\n";
                prompt_early_night_action(game, human);
                game.run_ai_night();
                prompt_witch_action(game, human);
                game.resolve_night();
                render_new_events(game, event_cursor, human);
            }
            if (game.state().phase == Phase::Discussion) {
                std::cout << "\n== 公开讨论 ==\n";
                game.run_ai_discussion_round();
                render_new_events(game, event_cursor, human);
                prompt_discussion_action(game, human);
                render_new_events(game, event_cursor, human);
                game.begin_voting();
            }
            if (game.state().phase == Phase::Voting) {
                game.run_ai_votes();
                if (game.state().players[human].alive) {
                    const int target = prompt_alive_target(game, human, "你要投给几号：");
                    std::string error;
                    if (!game.cast_vote(human, target, &error)) {
                        throw std::logic_error(error);
                    }
                }
                static_cast<void>(game.resolve_vote());
                render_new_events(game, event_cursor, human);
            }
        }

        std::cout << "\n对局结束。状态摘要：" << game.digest() << '\n';
        return 0;
    } catch (const EndOfInput&) {
        std::cout << "\n输入结束，退出当前对局。\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "错误：" << error.what() << '\n';
        return 1;
    }
}
