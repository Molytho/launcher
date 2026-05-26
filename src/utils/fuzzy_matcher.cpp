#include "utils/fuzzy_matcher.h"

#include <cctype>
#include <climits>
#include <string_view>

namespace {
    using score_t               = launcher::utils::fuzzy_match_score;
    constexpr score_t SCORE_MIN = std::numeric_limits<score_t>::min();

    constexpr score_t score_bonus_per_correct_character = 20;
    constexpr score_t score_penalty_per_index           = 1;

    bool compare_char(char a, char b, bool caseInsensitive) {
        if (caseInsensitive) {
            return std::tolower(static_cast<unsigned char>(a)) == std::tolower(static_cast<unsigned char>(b));
        } else {
            return a == b;
        }
    }

    score_t match(std::string_view query, std::string_view stringToSearchIn, bool caseInsensitive) {
        if (query.size() > stringToSearchIn.size()) {
            query = query.substr(0, stringToSearchIn.size());
        }

        score_t score               = 0;
        const char *search_position = stringToSearchIn.data();
        for (char c : query) {
            if (compare_char(c, *search_position, caseInsensitive)) {
                score += score_bonus_per_correct_character;
            }
            ++search_position;
        }
        return score;
    }
} // namespace

namespace launcher::utils {
    fuzzy_match_result fuzzy_match(std::string_view query, std::string_view stringToSearchIn, bool caseInsensitive) {
        if (query.empty() || stringToSearchIn.empty()) {
            return {INT_MIN};
        }

        score_t max_score = SCORE_MIN;
        score_t penalty   = 0;
        for (size_t i = 0; i < stringToSearchIn.size(); ++i, penalty += score_penalty_per_index) {
            int64_t score = match(query, stringToSearchIn.substr(i), caseInsensitive);
            score -= penalty;
            if (score > max_score) {
                max_score = score;
            }
        }

        return {max_score};
    }

} // namespace launcher::utils
