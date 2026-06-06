#ifndef LAUNCHER_ENTRY_H
#define LAUNCHER_ENTRY_H

#include <giomm.h>
#include <memory>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

namespace launcher {
    class history_provider;

    namespace interfaces {
        using Score = int64_t;

        class Query {
            std::string_view m_query;

        public:
            constexpr Query(std::string_view query) noexcept : m_query(query) {}

            [[nodiscard]] constexpr std::string_view get_query() const noexcept { return m_query; }
        };

        using IconVariant = std::variant<std::string_view, Glib::RefPtr<Gio::Icon>>;

        class Entry {
        private:
            friend launcher::history_provider;

            Score m_score {};

        protected:
            void set_score(Score score) noexcept { m_score = score; }

            void boost_score(Score score) noexcept { m_score += score; }

        public:
            virtual ~Entry() = default;

            virtual void execute() const = 0;

            [[nodiscard]] virtual std::string_view get_title() const noexcept = 0;

            [[nodiscard]] virtual std::string_view get_subtitle() const noexcept = 0;

            [[nodiscard]] virtual IconVariant get_icon() const noexcept = 0;

            [[nodiscard]] virtual std::string get_id() const noexcept = 0;

            [[nodiscard]] Score get_score() const noexcept { return m_score; }
        };

        class Provider {
        public:
            virtual ~Provider() = default;

            [[nodiscard]] virtual std::vector<std::shared_ptr<Entry>> query(const Query &query) const = 0;
        };
    } // namespace interfaces
} // namespace launcher

#endif