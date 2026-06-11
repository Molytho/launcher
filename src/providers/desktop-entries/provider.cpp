#include "providers/desktop-entries/provider.h"

#include <ranges>

#include <boost/regex.hpp>
#include <desktop-entry.h>

#include "macros.h"
#include "utils/fuzzy_matcher.h"
#include "utils/spawn_helper.h"

namespace {
    std::string prepare_command_line(const xdg::desktop_entry_spec::desktop_entry &entry) {
        static const boost::regex ignored_keys {"(?<!%)((?:%%)*)%[fFuU]"};
        static const boost::regex percent_re {"%%"};
        static const boost::regex uneven_percents_re {"(?<!%)(?:%%)*%(?!%)"};
        // TODO: Some are unsupported
        static const boost::regex unsupported_re {"(?<!%)((?:%%)*)%[ik]"};
        static const boost::regex name_re {"(?<!%)((?:%%)*)%c"};
        std::string cmdline {entry.get_exec()};
        cmdline = boost::regex_replace(std::move(cmdline), ignored_keys, "$1");
        cmdline = boost::regex_replace(std::move(cmdline), unsupported_re, "$1");
        cmdline = boost::regex_replace(std::move(cmdline), name_re, entry.get_name().get());
        if (boost::regex_search(cmdline, uneven_percents_re)) {
            throw std::runtime_error("Invalid desktop file Exec entry");
        }
        cmdline = boost::regex_replace(std::move(cmdline), percent_re, "%");
        return cmdline;
    }
} // namespace

namespace launcher::provider::desktop_entries {
    class DesktopFileEntry : public interfaces::Entry {
        std::unique_ptr<xdg::desktop_entry_spec::desktop_entry> m_desktop_entry;

    public:
        DesktopFileEntry(std::unique_ptr<xdg::desktop_entry_spec::desktop_entry> desktop_entry) :
                m_desktop_entry(std::move(desktop_entry)) {}

        void execute() const final {
            auto id = m_desktop_entry->get_id();
            r_assert(id.ends_with(".desktop"));
            id.resize(id.size() - 8);
            id = escape_systemd_string(id, false);

            spawn_context context {};
            context.executable = "/bin/sh";
            context.arguments  = {"-c", prepare_command_line(*m_desktop_entry)};
            context.unit_name  = "app-" + id + "-" + make_unique_identifier();
            context.slice      = "app-" + id + ".slice";
            spawn_as_service(context);
        }

        [[nodiscard]] virtual std::string_view get_title() const noexcept final {
            return m_desktop_entry->get_name().get();
        }

        [[nodiscard]] virtual std::string_view get_subtitle() const noexcept final {
            auto desc = m_desktop_entry->get_comment();
            return desc ? std::string_view(desc->get()) : "";
        }

        [[nodiscard]] virtual interfaces::IconVariant get_icon() const noexcept final {
            auto icon = m_desktop_entry->get_icon();
            return icon ? std::string_view(icon->get()) : "";
        }

        [[nodiscard]] std::string get_id() const noexcept final {
            return m_desktop_entry->get_id();
        }

        using interfaces::Entry::set_score;
    };
} // namespace launcher::provider::desktop_entries

namespace {
    std::vector<std::shared_ptr<launcher::provider::desktop_entries::DesktopFileEntry>> make_available_entries() {
        auto view = std::views::filter(xdg::desktop_entry_spec::get_all_desktop_entries(), [](const auto &entry) {
            return entry->should_show();
        }) | std::views::transform([](auto &app_info) {
            return std::make_shared<launcher::provider::desktop_entries::DesktopFileEntry>(std::move(app_info));
        });
        return {std::move_iterator(view.begin()), std::move_iterator(view.end())};
    }
} // namespace

namespace launcher::providers {
    DesktopEntryProvider::DesktopEntryProvider() :
            interfaces::Provider(), m_available_entries(make_available_entries()) {}

    std::vector<std::shared_ptr<interfaces::Entry>> DesktopEntryProvider::query(
        const interfaces::Query &query) const {
        auto view = std::views::transform(m_available_entries, [&query](const auto &entry) {
            auto match_result = utils::fuzzy_match(query.get_query(), entry->get_title());
            entry->set_score(match_result.score);
            return std::shared_ptr<interfaces::Entry>(entry);
        });
        return {std::move_iterator(view.begin()), std::move_iterator(view.end())};
    }
} // namespace launcher::providers