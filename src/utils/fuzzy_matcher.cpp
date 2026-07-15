#include "utils/fuzzy_matcher.h"

#include <rapidfuzz/fuzz.hpp>

#include <cctype>
#include <climits>
#include <string_view>

namespace launcher::utils {
    namespace detail {
        std::string tolower(std::string_view str) {
            std::string res;
            res.resize(str.size());
            std::ranges::transform(str, res.begin(), ::tolower);
            return res;
        }
    } // namespace detail

    fuzzy_match_result fuzzy_match(std::string_view query, std::string_view stringToSearchIn, bool caseInsensitive) {
        if (query.empty() || stringToSearchIn.empty()) {
            return {std::numeric_limits<fuzzy_match_result>::min()};
        }

        if (caseInsensitive) {
            return fuzzy_match(detail::tolower(query), detail::tolower(stringToSearchIn), false);
        }

        launcher::utils::fuzzy_match_result result {
            .score = rapidfuzz::fuzz::partial_ratio(query, stringToSearchIn)};
        return result;
    }

} // namespace launcher::utils
