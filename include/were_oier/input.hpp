#pragma once

#include <optional>
#include <string_view>

namespace were_oier {

[[nodiscard]] std::optional<int> parse_integer(std::string_view text, int minimum, int maximum);

} // namespace were_oier

