#include "providers/desktop-entries/provider.h"

#include <boost/regex.hpp>
#include <ranges>

#include "macros.h"
#include "utils/fuzzy_matcher.h"
#include "utils/spawn_helper.h"

namespace {
    std::string prepare_command_line(const Gio::AppInfo &app_info) {
        static const boost::regex ignored_keys {"(?<!%)((?:%%)*)%[fFuU]"};
        static const boost::regex percent_re {"%%"};
        static const boost::regex uneven_percents_re {"(?<!%)(?:%%)*%(?!%)"};
        // TODO: Some are unsupported
        static const boost::regex unsupported_re {"(?<!%)((?:%%)*)%[ik]"};
        static const boost::regex name_re {"(?<!%)((?:%%)*)%c"};
        auto cmdline = app_info.get_commandline();
        cmdline      = boost::regex_replace(std::move(cmdline), ignored_keys, "$1");
        cmdline      = boost::regex_replace(std::move(cmdline), unsupported_re, "$1");
        cmdline = boost::regex_replace(std::move(cmdline), name_re, app_info.get_display_name());
        if (boost::regex_search(cmdline, uneven_percents_re)) {
            throw std::runtime_error("Invalid desktop file Exec entry");
        }
        cmdline = boost::regex_replace(std::move(cmdline), percent_re, "%");
        return cmdline;
    }
} // namespace

namespace launcher::provider::desktop_entries {
    class DesktopFileEntry : public interfaces::Entry {
        Glib::RefPtr<Gio::AppInfo> m_app_info;

    public:
        DesktopFileEntry(Glib::RefPtr<Gio::AppInfo> app_info) : m_app_info(std::move(app_info)) {}

        void execute() const final {
            auto id = m_app_info->get_id();
            r_assert(id.ends_with(".desktop"));
            id.resize(id.size() - 8);
            id = escape_systemd_string(id, false);

            spawn_context context {};
            context.executable = "/bin/sh";
            context.arguments  = {"-c", prepare_command_line(*m_app_info)};
            context.unit_name  = "app-" + id + "-" + make_unique_identifier();
            context.slice      = "app-" + id + ".slice";
            spawn_as_service(context);
        }

        [[nodiscard]] virtual std::string_view get_title() const noexcept final {
            return g_app_info_get_display_name(m_app_info->gobj());
        }

        [[nodiscard]] virtual std::string_view get_subtitle() const noexcept final {
            auto desc = g_app_info_get_description(m_app_info->gobj());
            return desc ? desc : "";
        }

        [[nodiscard]] virtual interfaces::IconVariant get_icon() const noexcept final {
            return m_app_info->get_icon();
        }

        [[nodiscard]] std::string get_id() const noexcept final { return m_app_info->get_id(); }

        using interfaces::Entry::set_score;
    };
} // namespace launcher::provider::desktop_entries

namespace {
    std::vector<std::shared_ptr<launcher::provider::desktop_entries::DesktopFileEntry>> make_available_entries() {
        Gio::init();
        auto view = std::views::filter(Gio::AppInfo::get_all(), [](const auto &app_info) {
            return app_info->should_show();
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