#pragma once

#include "were_oier/core/types.hpp"

#include <cstdint>
#include <memory>

namespace were_oier {

/// AI policy that only receives a DecisionRequest, never the hidden game state.
class AiController {
 public:
  explicit AiController(std::uint64_t seed);
  ~AiController();
  AiController(AiController&&) noexcept;
  AiController& operator=(AiController&&) noexcept;
  AiController(const AiController&) = delete;
  AiController& operator=(const AiController&) = delete;

  [[nodiscard]] Action decide(const DecisionRequest& request);

 private:
  class Random;
  std::unique_ptr<Random> random_;

  [[nodiscard]] int suspicion(const DecisionRequest& request, PlayerId candidate) const;
  [[nodiscard]] Action annotate(Action action, const DecisionRequest& request) const;
  [[nodiscard]] PlayerId highest_suspicion(const DecisionRequest& request,
                                           bool prefer_high);
  [[nodiscard]] PlayerId random_target(const std::vector<PlayerId>& candidates);
};

}  // namespace were_oier
