#include "were_oier/game.hpp"

#include <algorithm>
#include <iomanip>
#include <limits>
#include <sstream>
#include <stdexcept>

namespace were_oier {
namespace {

int clamp_suspicion(int value) {
    return std::clamp(value, 0, 100);
}

int clamp_credibility(int value) {
    return std::clamp(value, 1, 20);
}

bool is_unique_role(Role role) {
    return role == Role::Seer || role == Role::Witch || role == Role::Hunter;
}

} // namespace

std::string to_string(Role role) {
    switch (role) {
    case Role::Villager:
        return "平民";
    case Role::Werewolf:
        return "狼人";
    case Role::Seer:
        return "预言家";
    case Role::Witch:
        return "女巫";
    case Role::Hunter:
        return "猎人";
    }
    return "未知角色";
}

std::string to_string(Faction faction) {
    return faction == Faction::Village ? "村民阵营" : "狼人阵营";
}

std::string to_string(Phase phase) {
    switch (phase) {
    case Phase::Night:
        return "夜晚";
    case Phase::Discussion:
        return "讨论";
    case Phase::Voting:
        return "投票";
    case Phase::Finished:
        return "结束";
    }
    return "未知阶段";
}

Faction faction_of(Role role) {
    return role == Role::Werewolf ? Faction::Wolves : Faction::Village;
}

Game::Game(std::uint64_t seed, int human_seat)
    : rng_(seed) {
    if (human_seat < 0 || human_seat > kSeatCount) {
        throw std::invalid_argument("human seat must be 0 or a seat from 1 to 10");
    }

    state_.seed = seed;
    state_.human_seat = human_seat;
    state_.day = 1;
    state_.phase = Phase::Night;

    std::array<Role, kSeatCount> deck{
        Role::Werewolf,
        Role::Werewolf,
        Role::Seer,
        Role::Witch,
        Role::Hunter,
        Role::Villager,
        Role::Villager,
        Role::Villager,
        Role::Villager,
        Role::Villager,
    };
    std::shuffle(deck.begin(), deck.end(), rng_);

    std::uniform_int_distribution<int> trait(3, 9);
    std::uniform_int_distribution<int> initial_suspicion(0, 2);
    for (int seat = 1; seat <= kSeatCount; ++seat) {
        state_.players[seat] = PlayerState{
            seat,
            deck[static_cast<std::size_t>(seat - 1)],
            true,
            seat == human_seat,
            trait(rng_),
            trait(rng_),
        };
    }

    for (int observer = 1; observer <= kSeatCount; ++observer) {
        for (int target = 1; target <= kSeatCount; ++target) {
            beliefs_[observer].credibility[target] = 10;
            beliefs_[observer].suspicion[target] =
                observer == target ? 0 : initial_suspicion(rng_);
        }
        beliefs_[observer].known_faction[observer] =
            faction_of(state_.players[observer].role);
        if (state_.players[observer].role == Role::Werewolf) {
            for (int target = 1; target <= kSeatCount; ++target) {
                if (state_.players[target].role == Role::Werewolf) {
                    beliefs_[observer].known_faction[target] = Faction::Wolves;
                }
            }
        }
    }

    add_event(0, "第 1 天夜晚开始。");
    if (human_seat != 0) {
        add_event(human_seat,
                  "你的身份是" + to_string(state_.players[human_seat].role) + "。");
    }
}

bool Game::can_view(const Event& event, int viewer) const noexcept {
    return event.visible_to == 0 || event.visible_to == viewer;
}

int Game::suspicion(int observer, int target) const {
    if (observer < 1 || observer > kSeatCount || target < 1 || target > kSeatCount) {
        throw std::out_of_range("invalid seat");
    }
    return beliefs_[observer].suspicion[target];
}

int Game::credibility(int observer, int speaker) const {
    if (observer < 1 || observer > kSeatCount || speaker < 1 || speaker > kSeatCount) {
        throw std::out_of_range("invalid seat");
    }
    return beliefs_[observer].credibility[speaker];
}

std::optional<int> Game::vote_of(int voter) const {
    if (voter < 1 || voter > kSeatCount || votes_[voter] == 0) {
        return std::nullopt;
    }
    return votes_[voter];
}

bool Game::valid_alive_seat(int seat) const noexcept {
    return seat >= 1 && seat <= kSeatCount && state_.players[seat].alive;
}

bool Game::has_night_action(int actor) const noexcept {
    return std::any_of(night_actions_.begin(), night_actions_.end(),
                       [actor](const NightAction& action) { return action.actor == actor; });
}

void Game::set_error(std::string* error, const std::string& text) const {
    if (error != nullptr) {
        *error = text;
    }
}

bool Game::submit_night_action(const NightAction& action, std::string* error) {
    if (state_.phase != Phase::Night) {
        set_error(error, "当前不是夜晚阶段");
        return false;
    }
    if (!valid_alive_seat(action.actor)) {
        set_error(error, "行动者不存在或已经死亡");
        return false;
    }
    if (has_night_action(action.actor)) {
        set_error(error, "该玩家今晚已经行动");
        return false;
    }

    const Role role = state_.players[action.actor].role;
    auto require_target = [&]() {
        return valid_alive_seat(action.target) && action.target != action.actor;
    };

    bool valid = false;
    switch (action.kind) {
    case NightActionKind::Pass:
        valid = true;
        break;
    case NightActionKind::Kill:
        valid = role == Role::Werewolf && require_target() &&
                state_.players[action.target].role != Role::Werewolf;
        break;
    case NightActionKind::Inspect:
        valid = role == Role::Seer && require_target();
        break;
    case NightActionKind::Heal:
        valid = role == Role::Witch && state_.witch_heal_available &&
                preview_wolf_target() == action.target;
        break;
    case NightActionKind::Poison:
        valid = role == Role::Witch && state_.witch_poison_available && require_target();
        break;
    }

    if (!valid) {
        set_error(error, "角色能力、目标或药剂状态不允许该行动");
        return false;
    }
    night_actions_.push_back(action);
    return true;
}

int Game::most_suspicious_target(int observer, bool avoid_wolves) const {
    int chosen = 0;
    int best = std::numeric_limits<int>::min();
    for (int target = 1; target <= kSeatCount; ++target) {
        if (!valid_alive_seat(target) || target == observer) {
            continue;
        }
        if (avoid_wolves && state_.players[target].role == Role::Werewolf) {
            continue;
        }
        int score = beliefs_[observer].suspicion[target];
        if (beliefs_[observer].known_faction[target] == Faction::Wolves) {
            score += 1000;
        } else if (beliefs_[observer].known_faction[target] == Faction::Village) {
            score -= 1000;
        }
        if (score > best) {
            best = score;
            chosen = target;
        }
    }
    return chosen;
}

int Game::least_suspicious_target(int observer) const {
    int chosen = 0;
    int best = std::numeric_limits<int>::max();
    for (int target = 1; target <= kSeatCount; ++target) {
        if (!valid_alive_seat(target) || target == observer) {
            continue;
        }
        int score = beliefs_[observer].suspicion[target];
        if (beliefs_[observer].known_faction[target] == Faction::Village) {
            score -= 1000;
        }
        if (score < best) {
            best = score;
            chosen = target;
        }
    }
    return chosen;
}

int Game::choose_wolf_target() const {
    int observer = 0;
    for (int seat = 1; seat <= kSeatCount; ++seat) {
        if (valid_alive_seat(seat) && state_.players[seat].role == Role::Werewolf) {
            observer = seat;
            break;
        }
    }
    if (observer == 0) {
        return 0;
    }

    int chosen = 0;
    int best = std::numeric_limits<int>::min();
    for (int target = 1; target <= kSeatCount; ++target) {
        if (!valid_alive_seat(target) || state_.players[target].role == Role::Werewolf) {
            continue;
        }
        const int score = beliefs_[observer].credibility[target] * 3 -
                          beliefs_[observer].suspicion[target] +
                          state_.players[target].boldness;
        if (score > best) {
            best = score;
            chosen = target;
        }
    }
    return chosen;
}

std::optional<int> Game::preview_wolf_target() const {
    std::array<int, kSeatCount + 1> counts{};
    for (const NightAction& action : night_actions_) {
        if (action.kind == NightActionKind::Kill && valid_alive_seat(action.target)) {
            ++counts[action.target];
        }
    }
    int target = 0;
    int best = 0;
    for (int seat = 1; seat <= kSeatCount; ++seat) {
        if (counts[seat] > best) {
            best = counts[seat];
            target = seat;
        }
    }
    return target == 0 ? std::nullopt : std::optional<int>{target};
}

void Game::run_ai_night() {
    if (state_.phase != Phase::Night) {
        throw std::logic_error("AI night actions require the night phase");
    }
    const std::array<Role, 5> action_order{
        Role::Werewolf, Role::Seer, Role::Witch, Role::Hunter, Role::Villager};
    for (const Role active_role : action_order) {
        for (int seat = 1; seat <= kSeatCount; ++seat) {
            if (!valid_alive_seat(seat) || state_.players[seat].human ||
                has_night_action(seat) || state_.players[seat].role != active_role) {
                continue;
            }
            NightAction action{seat, NightActionKind::Pass, 0};
            if (active_role == Role::Werewolf) {
                action = {seat, NightActionKind::Kill, choose_wolf_target()};
            } else if (active_role == Role::Seer) {
                action = {seat, NightActionKind::Inspect, most_suspicious_target(seat)};
            } else if (active_role == Role::Witch) {
                const std::optional<int> victim = preview_wolf_target();
                if (state_.witch_heal_available && victim.has_value() &&
                    beliefs_[seat].suspicion[*victim] <= 2) {
                    action = {seat, NightActionKind::Heal, *victim};
                } else {
                    const int target = most_suspicious_target(seat);
                    if (state_.witch_poison_available && target != 0 &&
                        beliefs_[seat].suspicion[target] >= 5) {
                        action = {seat, NightActionKind::Poison, target};
                    }
                }
            }
            std::string error;
            if (!submit_night_action(action, &error)) {
                throw std::logic_error("AI generated invalid night action: " + error);
            }
        }
    }
}

std::optional<int> Game::choose_hunter_target(int hunter) const {
    const int target = most_suspicious_target(hunter);
    return target == 0 ? std::nullopt : std::optional<int>{target};
}

void Game::eliminate(int seat, const std::string& reason) {
    if (!valid_alive_seat(seat)) {
        return;
    }
    state_.players[seat].alive = false;
    add_event(0, std::to_string(seat) + " 号因" + reason + "死亡，身份是" +
                     to_string(state_.players[seat].role) + "。");
    update_beliefs_after_reveal(seat);

    if (state_.players[seat].role == Role::Hunter) {
        const std::optional<int> target = choose_hunter_target(seat);
        if (target.has_value()) {
            add_event(0, std::to_string(seat) + " 号猎人发动遗言枪，带走 " +
                             std::to_string(*target) + " 号。");
            eliminate(*target, "猎人的遗言枪");
        }
    }
}

void Game::resolve_night() {
    if (state_.phase != Phase::Night) {
        throw std::logic_error("night resolution requires the night phase");
    }

    std::array<bool, kSeatCount + 1> deaths{};
    const std::optional<int> wolf_target = preview_wolf_target();
    if (wolf_target.has_value()) {
        deaths[*wolf_target] = true;
    }

    for (const NightAction& action : night_actions_) {
        if (action.kind == NightActionKind::Inspect) {
            const Faction result = faction_of(state_.players[action.target].role);
            beliefs_[action.actor].known_faction[action.target] = result;
            add_event(action.actor, "你查验了 " + std::to_string(action.target) +
                                        " 号：属于" + to_string(result) + "。");
        } else if (action.kind == NightActionKind::Heal &&
                   wolf_target == action.target && deaths[action.target]) {
            deaths[action.target] = false;
            state_.witch_heal_available = false;
            add_event(action.actor, "你使用了解药，救下 " +
                                        std::to_string(action.target) + " 号。");
        } else if (action.kind == NightActionKind::Poison) {
            deaths[action.target] = true;
            state_.witch_poison_available = false;
            add_event(action.actor, "你使用毒药选择了 " +
                                        std::to_string(action.target) + " 号。");
        }
    }

    bool any_death = false;
    for (int seat = 1; seat <= kSeatCount; ++seat) {
        if (deaths[seat]) {
            any_death = true;
            eliminate(seat, "夜间袭击");
        }
    }
    if (!any_death) {
        add_event(0, "昨夜平安无事。");
    }

    night_actions_.clear();
    votes_.fill(0);
    evaluate_winner();
    if (state_.phase != Phase::Finished) {
        state_.phase = Phase::Discussion;
        add_event(0, "第 " + std::to_string(state_.day) + " 天进入公开讨论。");
    }
}

bool Game::apply_discussion(const DiscussionAction& action, std::string* error) {
    if (state_.phase != Phase::Discussion) {
        set_error(error, "当前不是讨论阶段");
        return false;
    }
    if (!valid_alive_seat(action.speaker)) {
        set_error(error, "发言者不存在或已经死亡");
        return false;
    }
    const bool needs_target = action.kind == TalkKind::Accuse ||
                              action.kind == TalkKind::Defend ||
                              action.kind == TalkKind::Question;
    if (needs_target && (!valid_alive_seat(action.target) || action.target == action.speaker)) {
        set_error(error, "该发言需要一名仍存活的其他玩家作为目标");
        return false;
    }
    if (action.kind == TalkKind::ClaimRole && !action.claimed_role.has_value()) {
        set_error(error, "身份声明缺少角色");
        return false;
    }

    switch (action.kind) {
    case TalkKind::Accuse:
        add_event(0, std::to_string(action.speaker) + " 号指控 " +
                         std::to_string(action.target) + " 号具有狼人嫌疑。");
        break;
    case TalkKind::Defend:
        add_event(0, std::to_string(action.speaker) + " 号为 " +
                         std::to_string(action.target) + " 号辩护。");
        break;
    case TalkKind::ClaimRole:
        add_event(0, std::to_string(action.speaker) + " 号声明自己是" +
                         to_string(*action.claimed_role) + "。");
        break;
    case TalkKind::Question:
        add_event(0, std::to_string(action.speaker) + " 号要求 " +
                         std::to_string(action.target) + " 号解释自己的行为。");
        break;
    case TalkKind::Pass:
        add_event(0, std::to_string(action.speaker) + " 号选择暂不表态。");
        break;
    }

    for (int observer = 1; observer <= kSeatCount; ++observer) {
        if (!valid_alive_seat(observer) || observer == action.speaker) {
            continue;
        }
        const int weight = std::clamp(beliefs_[observer].credibility[action.speaker] / 5,
                                      1, 4);
        if (action.kind == TalkKind::Accuse) {
            beliefs_[observer].suspicion[action.target] = clamp_suspicion(
                beliefs_[observer].suspicion[action.target] + weight);
        } else if (action.kind == TalkKind::Defend) {
            beliefs_[observer].suspicion[action.target] = clamp_suspicion(
                beliefs_[observer].suspicion[action.target] - std::max(1, weight - 1));
        } else if (action.kind == TalkKind::Question) {
            beliefs_[observer].suspicion[action.target] = clamp_suspicion(
                beliefs_[observer].suspicion[action.target] + 1);
        } else if (action.kind == TalkKind::ClaimRole) {
            beliefs_[observer].claims[action.speaker] = action.claimed_role;
            if (is_unique_role(*action.claimed_role)) {
                for (int other = 1; other <= kSeatCount; ++other) {
                    if (other == action.speaker || !valid_alive_seat(other)) {
                        continue;
                    }
                    if (beliefs_[observer].claims[other] == action.claimed_role) {
                        beliefs_[observer].suspicion[action.speaker] = clamp_suspicion(
                            beliefs_[observer].suspicion[action.speaker] + 2);
                        beliefs_[observer].suspicion[other] = clamp_suspicion(
                            beliefs_[observer].suspicion[other] + 2);
                    }
                }
            }
        }
    }
    return true;
}

void Game::run_ai_discussion_round() {
    if (state_.phase != Phase::Discussion) {
        throw std::logic_error("AI discussion requires the discussion phase");
    }
    for (int speaker = 1; speaker <= kSeatCount; ++speaker) {
        if (!valid_alive_seat(speaker) || state_.players[speaker].human) {
            continue;
        }

        const Role role = state_.players[speaker].role;
        int known_wolf = 0;
        for (int target = 1; target <= kSeatCount; ++target) {
            if (valid_alive_seat(target) && target != speaker &&
                beliefs_[speaker].known_faction[target] == Faction::Wolves) {
                known_wolf = target;
                break;
            }
        }

        DiscussionAction action{speaker, TalkKind::Pass, 0, std::nullopt};
        if (role == Role::Seer && known_wolf != 0) {
            action = {speaker, TalkKind::Accuse, known_wolf, std::nullopt};
        } else if (role == Role::Seer && state_.day >= 2 &&
                   state_.players[speaker].boldness >= 6) {
            action = {speaker, TalkKind::ClaimRole, 0, Role::Seer};
        } else {
            const bool wolf = role == Role::Werewolf;
            const int target = most_suspicious_target(speaker, wolf);
            if (target != 0 && beliefs_[speaker].suspicion[target] >= 4) {
                action = {speaker, TalkKind::Accuse, target, std::nullopt};
            } else if (target != 0 && state_.players[speaker].boldness >= 5) {
                action = {speaker, TalkKind::Question, target, std::nullopt};
            } else {
                const int defended = least_suspicious_target(speaker);
                if (defended != 0) {
                    action = {speaker, TalkKind::Defend, defended, std::nullopt};
                }
            }
        }
        std::string error;
        if (!apply_discussion(action, &error)) {
            throw std::logic_error("AI generated invalid discussion action: " + error);
        }
    }
}

void Game::begin_voting() {
    if (state_.phase != Phase::Discussion) {
        throw std::logic_error("voting can only begin after discussion");
    }
    votes_.fill(0);
    state_.phase = Phase::Voting;
    add_event(0, "讨论结束，开始投票。");
}

bool Game::cast_vote(int voter, int target, std::string* error) {
    if (state_.phase != Phase::Voting) {
        set_error(error, "当前不是投票阶段");
        return false;
    }
    if (!valid_alive_seat(voter) || !valid_alive_seat(target) || voter == target) {
        set_error(error, "投票者或目标无效");
        return false;
    }
    if (votes_[voter] != 0) {
        set_error(error, "该玩家已经投票");
        return false;
    }
    votes_[voter] = target;
    return true;
}

void Game::run_ai_votes() {
    if (state_.phase != Phase::Voting) {
        throw std::logic_error("AI voting requires the voting phase");
    }
    for (int voter = 1; voter <= kSeatCount; ++voter) {
        if (!valid_alive_seat(voter) || state_.players[voter].human || votes_[voter] != 0) {
            continue;
        }
        const bool avoid_wolves = state_.players[voter].role == Role::Werewolf;
        const int target = most_suspicious_target(voter, avoid_wolves);
        if (target != 0) {
            std::string error;
            if (!cast_vote(voter, target, &error)) {
                throw std::logic_error("AI generated invalid vote: " + error);
            }
        }
    }
}

void Game::update_beliefs_after_reveal(int revealed_seat) {
    const bool wolf = state_.players[revealed_seat].role == Role::Werewolf;
    for (int observer = 1; observer <= kSeatCount; ++observer) {
        beliefs_[observer].known_faction[revealed_seat] =
            wolf ? Faction::Wolves : Faction::Village;
        for (int voter = 1; voter <= kSeatCount; ++voter) {
            if (votes_[voter] != revealed_seat) {
                continue;
            }
            const int credibility_delta = wolf ? 2 : -2;
            const int suspicion_delta = wolf ? -2 : 2;
            beliefs_[observer].credibility[voter] = clamp_credibility(
                beliefs_[observer].credibility[voter] + credibility_delta);
            beliefs_[observer].suspicion[voter] = clamp_suspicion(
                beliefs_[observer].suspicion[voter] + suspicion_delta);
        }
    }
}

void Game::evaluate_winner() {
    int wolves = 0;
    int village = 0;
    for (int seat = 1; seat <= kSeatCount; ++seat) {
        if (!state_.players[seat].alive) {
            continue;
        }
        if (state_.players[seat].role == Role::Werewolf) {
            ++wolves;
        } else {
            ++village;
        }
    }
    if (wolves == 0) {
        state_.winner = Faction::Village;
    } else if (wolves >= village) {
        state_.winner = Faction::Wolves;
    }
    if (state_.winner.has_value()) {
        state_.phase = Phase::Finished;
        add_event(0, to_string(*state_.winner) + "获胜。");
    }
}

std::optional<int> Game::resolve_vote() {
    if (state_.phase != Phase::Voting) {
        throw std::logic_error("vote resolution requires the voting phase");
    }

    std::array<int, kSeatCount + 1> counts{};
    for (int voter = 1; voter <= kSeatCount; ++voter) {
        if (!valid_alive_seat(voter) || votes_[voter] == 0) {
            continue;
        }
        ++counts[votes_[voter]];
        add_event(0, std::to_string(voter) + " 号投给了 " +
                         std::to_string(votes_[voter]) + " 号。理由：在其视角中该目标嫌疑最高。");
    }

    int best = 0;
    int exiled = 0;
    bool tie = false;
    for (int seat = 1; seat <= kSeatCount; ++seat) {
        if (!valid_alive_seat(seat) || counts[seat] == 0) {
            continue;
        }
        if (counts[seat] > best) {
            best = counts[seat];
            exiled = seat;
            tie = false;
        } else if (counts[seat] == best) {
            tie = true;
        }
    }

    if (exiled == 0 || tie) {
        add_event(0, "最高票出现平票，本轮无人被放逐。");
        exiled = 0;
    } else {
        eliminate(exiled, "公开投票放逐");
    }

    evaluate_winner();
    if (state_.phase != Phase::Finished) {
        ++state_.day;
        state_.phase = Phase::Night;
        night_actions_.clear();
        votes_.fill(0);
        add_event(0, "第 " + std::to_string(state_.day) + " 天夜晚开始。");
    }
    return exiled == 0 ? std::nullopt : std::optional<int>{exiled};
}

void Game::add_event(int visible_to, const std::string& text) {
    events_.push_back(Event{state_.day, state_.phase, visible_to, text});
}

std::string Game::digest() const {
    std::ostringstream raw;
    raw << state_.seed << '|' << state_.day << '|' << static_cast<int>(state_.phase) << '|';
    raw << (state_.winner.has_value() ? static_cast<int>(*state_.winner) + 1 : 0) << '|';
    raw << state_.witch_heal_available << state_.witch_poison_available << '|';
    for (int seat = 1; seat <= kSeatCount; ++seat) {
        const PlayerState& player = state_.players[seat];
        raw << static_cast<int>(player.role) << player.alive << player.cleverness
            << player.boldness << ';';
    }
    for (int observer = 1; observer <= kSeatCount; ++observer) {
        for (int target = 1; target <= kSeatCount; ++target) {
            raw << beliefs_[observer].suspicion[target] << ','
                << beliefs_[observer].credibility[target] << ','
                << (beliefs_[observer].claims[target].has_value()
                        ? static_cast<int>(*beliefs_[observer].claims[target]) + 1
                        : 0)
                << ','
                << (beliefs_[observer].known_faction[target].has_value()
                        ? static_cast<int>(*beliefs_[observer].known_faction[target]) + 1
                        : 0)
                << ';';
        }
    }
    for (const Event& event : events_) {
        raw << event.day << static_cast<int>(event.phase) << event.visible_to << event.text << '|';
    }
    raw << rng_;

    std::uint64_t hash = 1469598103934665603ULL;
    for (const unsigned char byte : raw.str()) {
        hash ^= byte;
        hash *= 1099511628211ULL;
    }
    std::ostringstream result;
    result << std::hex << std::setw(16) << std::setfill('0') << hash;
    return result.str();
}

} // namespace were_oier
