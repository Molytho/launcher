#ifndef LAUNCHER_FUZZY_MATCHER_HPP
#define LAUNCHER_FUZZY_MATCHER_HPP

#include <string_view>

#include <rapidfuzz/fuzz.hpp>

#include "macros.h"

namespace launcher::utils {
    using fuzzy_match_score = double;

    struct fuzzy_match_result {
        fuzzy_match_score score = std::numeric_limits<fuzzy_match_score>::min();
        std::string match       = "";
        size_t index            = 0;
    };

    [[nodiscard]] fuzzy_match_result fuzzy_match(
        std::string_view query, std::string_view stringToSearchIn, bool caseInsensitive = true);

    namespace detail {
        std::string tolower(std::string_view str);

        template<typename Sentence1, typename Iterable, typename Sentence2 = typename Iterable::value_type>
        fuzzy_match_result extractOne(
            const Sentence1 &query, const Iterable &choices, const double score_cutoff = 0.0) {
            bool match_found  = false;
            double best_score = score_cutoff;
            Sentence2 best_match;
            size_t best_index;

            rapidfuzz::fuzz::CachedPartialRatio<typename Sentence1::value_type> scorer(query);

            size_t i = 0;
            for (const auto &choice : choices) {
                double score = scorer.similarity(choice, best_score);

                if (score > best_score || (score == best_score && !match_found)) {
                    match_found = true;
                    best_score  = score;
                    best_match  = choice;
                    best_index  = i;
                }

                i++;
            }

            if (!match_found) {
                return {};
            }

            return {.score = best_score, .match = std::string(best_match), .index = best_index};
        }
    } // namespace detail

    template<std::ranges::range R>
        requires std::convertible_to<std::ranges::range_value_t<R>, std::string_view>
    [[nodiscard]] fuzzy_match_result fuzzy_match_multiple(
        std::string_view query, R &&strings, bool caseInsensitive = true) {
        if (caseInsensitive) {
            std::vector<std::string> lower_strings;
            if constexpr (std::ranges::sized_range<R>) {
                lower_strings.reserve(std::ranges::size(strings));
            }
            std::ranges::transform(strings, std::back_inserter(lower_strings), detail::tolower);

            auto match = fuzzy_match_multiple(detail::tolower(query), std::move(lower_strings), false);
            r_assert(match.index < lower_strings.size());
            auto it = strings.begin();
            std::advance(it, match.index);
            match.match = *it;
            return match;
        }

        return detail::extractOne(query, strings);
    }
} // namespace launcher::utils

#endif // LAUNCHER_FUZZY_MATCHER_HPP
