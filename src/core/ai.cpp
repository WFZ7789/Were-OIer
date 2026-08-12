#include "were_oier/core/ai.hpp"

#include <algorithm>
#include <limits>
#include <random>
#include <string>
#include <utility>
#include <vector>

namespace were_oier {
namespace {

[[nodiscard]] const KnownFact* known_fact(const DecisionRequest& request, PlayerId player) {
  const auto found = std::find_if(request.private_knowledge.facts.begin(),
                                  request.private_knowledge.facts.end(),
                                  [player](const KnownFact& fact) { return fact.player == player; });
  return found == request.private_knowledge.facts.end() ? nullptr : &*found;
}

[[nodiscard]] bool known_wolf_teammate(const DecisionRequest& request, PlayerId player) {
  const KnownFact* fact = known_fact(request, player);
  return fact != nullptr && fact->source == KnowledgeSource::WolfPack && is_wolf_team(fact->role);
}

[[nodiscard]] std::vector<PlayerId> unknown_to_seer(const DecisionRequest& request) {
  std::vector<PlayerId> candidates;
  for (const PlayerId player : request.available_targets) {
    const KnownFact* fact = known_fact(request, player);
    if (fact == nullptr || fact->source != KnowledgeSource::Investigation) {
      candidates.push_back(player);
    }
  }
  return candidates;
}

[[nodiscard]] std::optional<PlayerId> known_enemy(const DecisionRequest& request) {
  for (const KnownFact& fact : request.private_knowledge.facts) {
    if (fact.source == KnowledgeSource::Investigation && is_wolf_team(fact.role)) {
      const auto player = std::find_if(request.players.begin(), request.players.end(),
                                       [&fact](const PlayerPublicView& view) {
                                         return view.id == fact.player && view.alive;
                                       });
      if (player != request.players.end()) {
        return fact.player;
      }
    }
  }
  return std::nullopt;
}

}  // namespace

class AiController::Random {
 public:
  explicit Random(std::uint64_t seed) : engine_(seed) {}

  [[nodiscard]] std::size_t index(std::size_t size) {
    std::uniform_int_distribution<std::size_t> distribution(0, size - 1);
    return distribution(engine_);
  }

 private:
  std::mt19937_64 engine_;
};

AiController::AiController(std::uint64_t seed) : random_(std::make_unique<Random>(seed)) {}
AiController::~AiController() = default;
AiController::AiController(AiController&&) noexcept = default;
AiController& AiController::operator=(AiController&&) noexcept = default;

int AiController::suspicion(const DecisionRequest& request, PlayerId candidate) const {
  int score = 0;
  if (candidate == request.actor) {
    return std::numeric_limits<int>::min() / 4;
  }

  if (const KnownFact* fact = known_fact(request, candidate); fact != nullptr) {
    score += is_wolf_team(fact->role) ? 100 : -100;
  }

  for (const GameEvent& event : request.public_events) {
    if (event.type == EventType::Speech && event.actor == candidate && event.speech.has_value()) {
      if (*event.speech == SpeechKind::ClaimRole) {
        const int matching_claims = static_cast<int>(std::count_if(
            request.public_events.begin(), request.public_events.end(), [&event](const GameEvent& other) {
              return other.type == EventType::Speech && other.speech == SpeechKind::ClaimRole &&
                     other.role == event.role;
            }));
        if (matching_claims > 1) {
          score += (matching_claims - 1) * 15;
        }
      } else if (*event.speech == SpeechKind::Accuse && event.target != kNoPlayer) {
        if (const KnownFact* target_fact = known_fact(request, event.target); target_fact != nullptr) {
          score += is_wolf_team(target_fact->role) ? -30 : 30;
        } else {
          score += 1;
        }
      }
    }
    if (event.type == EventType::VoteCast && event.actor == candidate && event.target != kNoPlayer) {
      if (const KnownFact* target_fact = known_fact(request, event.target); target_fact != nullptr) {
        score += is_wolf_team(target_fact->role) ? -25 : 25;
      }
    }
    if (event.type == EventType::Speech && event.target == candidate && event.speech.has_value()) {
      if (*event.speech == SpeechKind::Accuse) {
        score += 2;
      } else if (*event.speech == SpeechKind::Defend) {
        score -= 2;
      }
    }
  }
  return score;
}

Action AiController::annotate(Action action, const DecisionRequest& request) const {
  for (const PlayerPublicView& player : request.players) {
    if (player.alive && player.id != request.actor) {
      action.suspicion_snapshot.push_back(
          SuspicionScore{.player = player.id, .score = suspicion(request, player.id)});
    }
  }
  return action;
}

PlayerId AiController::highest_suspicion(const DecisionRequest& request, bool prefer_high) {
  if (request.available_targets.empty()) {
    return kNoPlayer;
  }

  int best_score = prefer_high ? std::numeric_limits<int>::min() : std::numeric_limits<int>::max();
  std::vector<PlayerId> candidates;
  for (const PlayerId player : request.available_targets) {
    const int score = suspicion(request, player);
    const bool better = prefer_high ? score > best_score : score < best_score;
    if (better) {
      best_score = score;
      candidates.assign(1, player);
    } else if (score == best_score) {
      candidates.push_back(player);
    }
  }
  return random_target(candidates);
}

PlayerId AiController::random_target(const std::vector<PlayerId>& candidates) {
  if (candidates.empty()) {
    return kNoPlayer;
  }
  return candidates[random_->index(candidates.size())];
}

Action AiController::decide(const DecisionRequest& request) {
  Action action;
  const auto choose_high = [&] { return highest_suspicion(request, true); };
  const auto choose_low = [&] { return highest_suspicion(request, false); };

  switch (request.phase) {
    case Phase::NightMagician: {
      if (!request.private_knowledge.magician_swap_available || request.available_targets.size() < 2) {
        return annotate(Action::skip("魔术师能力已经用尽或没有合法目标。"), request);
      }
      action.kind = ActionKind::SwapRoles;
      action.target = random_target(request.available_targets);
      std::vector<PlayerId> remaining = request.available_targets;
      remaining.erase(std::remove(remaining.begin(), remaining.end(), action.target), remaining.end());
      action.secondary_target = random_target(remaining);
      action.rationale = "首夜制造身份错位，迫使公开信息重新洗牌。";
      return annotate(std::move(action), request);
    }
    case Phase::NightSeer: {
      const std::vector<PlayerId> unknown = unknown_to_seer(request);
      if (unknown.empty()) {
        return annotate(Action::skip("已查验所有存活目标。"), request);
      }
      action.kind = ActionKind::Investigate;
      action.target = random_target(unknown);
      action.rationale = "优先补充未查验玩家的确定身份信息。";
      return annotate(std::move(action), request);
    }
    case Phase::NightWolves:
      if (request.available_targets.empty()) {
        return annotate(Action::skip("没有可袭击的好人阵营目标。"), request);
      }
      action.kind = ActionKind::WerewolfAttack;
      action.target = choose_low();
      action.rationale = "优先袭击公开记录中较受信任的目标，削弱好人信息网络。";
      return annotate(std::move(action), request);
    case Phase::NightWhiteWolfKing:
      if (request.available_targets.empty()) {
        return annotate(Action::skip("没有可袭击的好人阵营目标。"), request);
      }
      action.kind = ActionKind::WhiteWolfKingAttack;
      action.target = choose_low();
      action.rationale = "独立击杀更受信任的好人，扩大夜晚收益。";
      return annotate(std::move(action), request);
    case Phase::NightWitch: {
      if (request.private_knowledge.witch_save_available &&
          request.private_knowledge.known_werewolf_target.has_value()) {
        const PlayerId target = *request.private_knowledge.known_werewolf_target;
        if (suspicion(request, target) < 40) {
          action.kind = ActionKind::WitchSave;
          action.target = target;
          action.rationale = "被袭击者当前嫌疑较低，解药保留好人信息。";
          return annotate(std::move(action), request);
        }
      }
      if (request.private_knowledge.witch_poison_available && !request.available_targets.empty()) {
        const PlayerId target = choose_high();
        if (target != kNoPlayer && target != request.actor && suspicion(request, target) >= 25) {
          action.kind = ActionKind::WitchPoison;
          action.target = target;
          action.rationale = "毒杀公开行为与私有线索共同指向的高嫌疑目标。";
          return annotate(std::move(action), request);
        }
      }
      return annotate(Action::skip("现有信息不足，保留女巫资源。"), request);
    }
    case Phase::DayDiscussion: {
      if (const std::optional<PlayerId> enemy = known_enemy(request); enemy.has_value()) {
        action.kind = ActionKind::Speak;
        action.speech = SpeechKind::Accuse;
        action.target = *enemy;
        action.rationale = "私有查验确认该玩家属于狼人阵营。";
        return annotate(std::move(action), request);
      }
      if (is_wolf_team(request.private_knowledge.role)) {
        std::vector<PlayerId> teammates;
        for (const KnownFact& fact : request.private_knowledge.facts) {
          if (fact.source == KnowledgeSource::WolfPack && is_wolf_team(fact.role)) {
            teammates.push_back(fact.player);
          }
        }
        if (!teammates.empty()) {
          action.kind = ActionKind::Speak;
          action.speech = SpeechKind::Defend;
          action.target = random_target(teammates);
          action.rationale = "为已知队友建立公开可信度。";
          return annotate(std::move(action), request);
        }
      }
      const PlayerId target = choose_high();
      if (target != kNoPlayer && suspicion(request, target) > 0) {
        action.kind = ActionKind::Speak;
        action.speech = SpeechKind::Accuse;
        action.target = target;
        action.rationale = "质疑其公开发言与投票中的不一致。";
        return annotate(std::move(action), request);
      }
      action.kind = ActionKind::Speak;
      action.speech = request.private_knowledge.role == Role::Seer ? SpeechKind::ClaimRole
                                                                    : SpeechKind::ClaimGood;
      if (action.speech == SpeechKind::ClaimRole) {
        action.claimed_role = Role::Seer;
      }
      action.rationale = "当前缺少可验证的矛盾，先维持公开信息空间。";
      return annotate(std::move(action), request);
    }
    case Phase::DayVote: {
      DecisionRequest vote_request = request;
      if (is_wolf_team(request.private_knowledge.role)) {
        vote_request.available_targets.erase(
            std::remove_if(vote_request.available_targets.begin(), vote_request.available_targets.end(),
                           [&request](PlayerId player) { return known_wolf_teammate(request, player); }),
            vote_request.available_targets.end());
      }
      if (vote_request.available_targets.empty()) {
        return annotate(Action::skip("没有可投放的非队友目标。"), request);
      }
      action.kind = ActionKind::Vote;
      action.target = highest_suspicion(vote_request, true);
      action.rationale = "将票投向当前证据链中嫌疑最高的存活玩家。";
      return annotate(std::move(action), request);
    }
    case Phase::DeathTrigger:
      if (request.available_targets.empty()) {
        return annotate(Action::skip("没有可带走的存活玩家。"), request);
      }
      action.kind = ActionKind::DeathShot;
      action.target = is_wolf_team(request.private_knowledge.role) ? choose_low() : choose_high();
      action.rationale = is_wolf_team(request.private_knowledge.role)
                             ? "带走最受信任的好人，争取阵营人数优势。"
                             : "带走嫌疑最高的玩家，减少狼人获胜路径。";
      return annotate(std::move(action), request);
    case Phase::Setup:
    case Phase::Dawn:
    case Phase::Finished:
      return annotate(Action::skip("当前阶段没有可执行操作。"), request);
  }
  return annotate(Action::skip("未识别的决策阶段。"), request);
}

}  // namespace were_oier
