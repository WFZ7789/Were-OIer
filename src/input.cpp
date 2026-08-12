#include "were_oier/input.hpp"

#include <charconv>

namespace were_oier {

std::optional<int> parse_integer(std::string_view text, int minimum, int maximum) {
    while (!text.empty() && (text.front() == ' ' || text.front() == '\t')) {
        text.remove_prefix(1);
    }
    while (!text.empty() && (text.back() == ' ' || text.back() == '\t')) {
        text.remove_suffix(1);
    }
    if (text.empty()) {
        return std::nullopt;
    }

    int value = 0;
    const char* begin = text.data();
    const char* end = text.data() + text.size();
    const auto [next, error] = std::from_chars(begin, end, value);
    if (error != std::errc{} || next != end || value < minimum || value > maximum) {
        return std::nullopt;
    }
    return value;
}

} // namespace were_oier

