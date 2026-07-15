#include "utils/fuzzy_matcher.h"

#include <rapidfuzz/fuzz.hpp>

#include <cctype>
#include <climits>
#include <string_view>

namespace {
    std::string tolower(std::string_view str) {
        std::string res;
        res.resize(str.size());
        std::ranges::transform(str, res.begin(), ::tolower);
        return res;
    }

    launcher::utils::fuzzy_match_result fuzzy_match_impl(std::string_view query, std::string_view stringToSearchIn) {
        if (query.empty() || stringToSearchIn.empty()) {
            return {INT_MIN};
        }

        launcher::utils::fuzzy_match_result result {
            .score = rapidfuzz::fuzz::partial_ratio(query, stringToSearchIn)};
        return result;
    }
} // namespace

namespace launcher::utils {
    fuzzy_match_result fuzzy_match(std::string_view query, std::string_view stringToSearchIn, bool caseInsensitive) {
        if (query.empty() || stringToSearchIn.empty()) {
            return {INT_MIN};
        }

        if (caseInsensitive) {
            auto lower_query  = tolower(query);
            auto lower_string = tolower(stringToSearchIn);
            return fuzzy_match_impl(lower_query, lower_string);
        } else {
            return fuzzy_match_impl(query, stringToSearchIn);
        }
    }

} // namespace launcher::utils
