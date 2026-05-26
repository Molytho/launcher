#ifndef LAUNCHER_ENTRY_H
#define LAUNCHER_ENTRY_H

#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace launcher::interfaces {
    using Score = int64_t;

    class Query {
        std::string m_query;

    public:
        constexpr Query(std::string query) noexcept : m_query(std::move(query)) {}

        [[nodiscard]] constexpr std::string_view get_query() const noexcept { return m_query; }
    };

    class Entry {
        std::string m_title;
        std::string m_subtitle;
        std::string m_icon;
        Score m_score;

    protected:
        constexpr Entry(std::string title, std::string subtitle, std::string icon, Score score = {}) noexcept :
                m_title(std::move(title)), m_subtitle(std::move(subtitle)), m_icon(std::move(icon)),
                m_score(score) {}

        constexpr Entry(std::string title, std::string icon, Score score = {}) noexcept :
                Entry(std::move(title), "", std::move(icon), score) {}

        void set_score(Score score) noexcept { m_score = score; }

    public:
        virtual ~Entry() = default;

        virtual void execute() const = 0;

        [[nodiscard]] constexpr std::string_view get_title() const noexcept { return m_title; }

        [[nodiscard]] constexpr std::string_view get_subtitle() const noexcept {
            return m_subtitle;
        }

        [[nodiscard]] constexpr std::string_view get_icon() const noexcept { return m_icon; }

        [[nodiscard]] constexpr Score get_score() const noexcept { return m_score; }
    };

    class Provider {
    public:
        virtual ~Provider() = default;
        virtual std::vector<std::shared_ptr<Entry>> query(const Query &query) const = 0;
    };
} // namespace launcher::interfaces

#endif