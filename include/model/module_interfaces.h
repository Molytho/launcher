#ifndef LAUNCHER_ENTRY_H
#define LAUNCHER_ENTRY_H

#include <iostream>
#include <memory>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

#include "api.h"

namespace launcher {
    class history_provider;

    namespace interfaces {
        using score = int64_t;

        class query {
            std::string_view m_query;

        public:
            constexpr query(std::string_view query) noexcept : m_query(query) {}

            [[nodiscard]] constexpr std::string_view get_query() const noexcept { return m_query; }
        };

        using icon_variant = std::variant<std::string_view>;

        class historyable {
        public:
            virtual ~historyable() = default;

            [[nodiscard]] virtual std::string get_history_id() const = 0;

            virtual void boost_score(score score) noexcept = 0;
        };

        class action {
        public:
            virtual ~action() = default;

            virtual void execute(execute_context &context) const = 0;

            [[nodiscard]] virtual std::string_view get_title() const noexcept = 0;

            [[nodiscard]] virtual icon_variant get_icon() const noexcept = 0;
        };

        class entry : public action {
        private:
            score m_score {};

        protected:
            void set_score(score score) noexcept { m_score = score; }

            void boost_score(score score) noexcept { m_score += score; }

        public:
            virtual ~entry() = default;

            [[nodiscard]] virtual std::string_view get_subtitle() const noexcept = 0;

            [[nodiscard]] virtual const std::vector<std::shared_ptr<action>> &get_actions() const = 0;

            [[nodiscard]] score get_score() const noexcept { return m_score; }
        };

        class historyable_entry : public entry, public historyable {
            void boost_score(score score) noexcept final {
                std::cout << score << "\n";
                entry::boost_score(score);
            }
        };

        class provider {
        public:
            virtual ~provider() = default;

            [[nodiscard]] virtual std::vector<std::shared_ptr<entry>> query(const query &query) const = 0;
        };
    } // namespace interfaces
} // namespace launcher

#endif