#ifndef LAUNCHER_FUZZY_MATCHER_HPP
#define LAUNCHER_FUZZY_MATCHER_HPP

#include <string_view>

namespace launcher::utils {
    using fuzzy_match_score = double;

    struct fuzzy_match_result {
        fuzzy_match_score score;
    };

    [[nodiscard]] fuzzy_match_result fuzzy_match(
        std::string_view query, std::string_view stringToSearchIn, bool caseInsensitive = true);
} // namespace launcher::utils

#endif // LAUNCHER_FUZZY_MATCHER_HPP
