#ifndef LAUNCHER_FUZZY_MATCHER_HPP
#define LAUNCHER_FUZZY_MATCHER_HPP

#include <optional>
#include <string_view>

#include <rapidfuzz/fuzz.hpp>

namespace launcher::utils {
    using fuzzy_match_score = double;

    struct fuzzy_match_result {
        fuzzy_match_score score;
    };

    [[nodiscard]] fuzzy_match_result fuzzy_match(
        std::string_view query, std::string_view stringToSearchIn, bool caseInsensitive = true);

    namespace detail {
        std::string tolower(std::string_view str);

        template<typename Sentence1, typename Iterable, typename Sentence2 = typename Iterable::value_type>
        std::optional<std::pair<Sentence2, double>> extractOne(
            const Sentence1 &query, const Iterable &choices, const double score_cutoff = 0.0) {
            bool match_found  = false;
            double best_score = score_cutoff;
            Sentence2 best_match;

            rapidfuzz::fuzz::CachedRatio<typename Sentence1::value_type> scorer(query);

            for (const auto &choice : choices) {
                double score = scorer.similarity(choice, best_score);

                if (score >= best_score) {
                    match_found = true;
                    best_score  = score;
                    best_match  = choice;
                }
            }

            if (!match_found) {
                return std::nullopt;
            }

            return std::make_pair(best_match, best_score);
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
            return fuzzy_match_multiple(detail::tolower(query), std::move(lower_strings), false);
        }

        auto match = detail::extractOne(query, strings);
        fuzzy_match_result res {
            .score = match ? match->second : std::numeric_limits<fuzzy_match_score>::min()};
        return res;
    }
} // namespace launcher::utils

#endif // LAUNCHER_FUZZY_MATCHER_HPP
