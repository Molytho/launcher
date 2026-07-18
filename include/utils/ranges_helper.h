#ifndef RANGES_HELPER_HPP
#define RANGES_HELPER_HPP

#include <concepts>
#include <ranges>
#include <vector>

namespace view_helper {
    template<class R>
    concept simple_view
        = std::ranges::view<R>
          && std::ranges::range<const R>
          && std::same_as<std::ranges::iterator_t<R>, std::ranges::iterator_t<const R>>
          && std::same_as<std::ranges::sentinel_t<R>, std::ranges::sentinel_t<const R>>;

    template<std::ranges::input_range R>
    class as_rvalue_view : public std::ranges::view_interface<as_rvalue_view<R>> {
        R m_view = R();

    public:
        as_rvalue_view()
            requires std::default_initializable<R>
        = default;

        constexpr explicit as_rvalue_view(R __base) : m_view(std::move(__base)) {}

        constexpr R base() const &
            requires std::copy_constructible<R>
        {
            return m_view;
        }

        constexpr R base() && { return std::move(m_view); }

        constexpr auto begin()
            requires(!simple_view<R>)
        {
            return move_iterator(std::ranges::begin(m_view));
        }

        constexpr auto begin() const
            requires std::ranges::range<const R>
        {
            return move_iterator(std::ranges::begin(m_view));
        }

        constexpr auto end()
            requires(!simple_view<R>)
        {
            if constexpr (std::ranges::common_range<R>) {
                return move_iterator(std::ranges::end(m_view));
            } else {
                return move_sentinel(std::ranges::end(m_view));
            }
        }

        constexpr auto end() const
            requires std::ranges::range<const R>
        {
            if constexpr (std::ranges::common_range<const R>) {
                return move_iterator(std::ranges::end(m_view));
            } else {
                return move_sentinel(std::ranges::end(m_view));
            }
        }

        constexpr auto size()
            requires std::ranges::sized_range<R>
        {
            return std::ranges::size(m_view);
        }

        constexpr auto size() const
            requires std::ranges::sized_range<const R>
        {
            return std::ranges::size(m_view);
        }
    };

    template<typename _Range>
    as_rvalue_view(_Range &&) -> as_rvalue_view<std::views::all_t<_Range>>;

    namespace detail {
        struct as_rvalue_impl {
            template<std::ranges::viewable_range R>
            auto operator()(R &&r) const {
                if constexpr (std::same_as<std::ranges::range_rvalue_reference_t<R>, std::ranges::range_reference_t<R>>) {
                    return std::views::all(std::forward<R>(r));
                } else {
                    return as_rvalue_view(std::forward<R>(r));
                }
            }
        };

        template<std::ranges::input_range R>
        auto operator|(R &&r, as_rvalue_impl as_rvalue) {
            return as_rvalue(std::forward<R>(r));
        }
    } // namespace detail

    constexpr inline detail::as_rvalue_impl as_rvalue;

    namespace detail {
        struct to_vector_imp {
            template<std::ranges::viewable_range R>
            std::vector<std::ranges::range_value_t<R>> operator()(R &&r) const {
                return {std::ranges::begin(r), std::ranges::end(r)};
            }
        };

        template<std::ranges::range R>
        auto operator|(R &&r, to_vector_imp to_vector) {
            return to_vector(std::forward<R>(r));
        }
    } // namespace detail

    constexpr inline detail::to_vector_imp to_vector;
} // namespace view_helper

#endif