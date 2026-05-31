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
    public:
        virtual ~Entry() = default;

        virtual void execute() const = 0;

        [[nodiscard]] virtual std::string_view get_title() const noexcept = 0;

        [[nodiscard]] virtual std::string_view get_subtitle() const noexcept = 0;

        // Rework this interface
        [[nodiscard]] virtual std::string_view get_icon() const noexcept = 0;

        // The score interface is bad as well
        [[nodiscard]] virtual Score get_score() const noexcept = 0;

        virtual void boost_score(Score score) noexcept = 0;

        [[nodiscard]] virtual std::string_view get_id() const noexcept = 0;
    };

    class Provider {
    public:
        virtual ~Provider() = default;

        [[nodiscard]] virtual std::vector<std::shared_ptr<Entry>> query(const Query &query) const = 0;
    };
} // namespace launcher::interfaces

#endif