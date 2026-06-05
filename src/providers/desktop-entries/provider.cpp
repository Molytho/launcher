#include "providers/desktop-entries/provider.h"

#include <boost/regex.hpp>

#include "macros.h"
#include "utils/fuzzy_matcher.h"
#include "utils/spawn_helper.h"

namespace {
    std::vector<Glib::RefPtr<Gio::AppInfo>> get_available_apps() {
        Gio::init();
        std::vector<Glib::RefPtr<Gio::AppInfo>> result = Gio::AppInfo::get_all();
        std::erase_if(result, [](const auto &app_info) { return !app_info->should_show(); });
        return result;
    }

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
        cmdline      = boost::regex_replace(std::move(cmdline), name_re, app_info.get_display_name());
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
        std::string m_icon_string;

    public:
        DesktopFileEntry(Glib::RefPtr<Gio::AppInfo> app_info) :
                m_app_info(std::move(app_info)),
                m_icon_string(m_app_info->get_icon() ? m_app_info->get_icon()->to_string() : std::string()) {}

        void execute() const final {
            auto id = m_app_info->get_id();
            r_assert(id.ends_with(".desktop"));
            id.resize(id.size() - 8);

            spawn_context context {};
            context.executable = "/bin/sh";
            context.arguments  = {"-c", prepare_command_line(*m_app_info)};
            context.unit_name  = "app-" + id + "-" + make_unique_identifier();
            context.slice      = "app-" + id + ".slice";
            spawn_as_service(context);
        }

        [[nodiscard]] virtual std::string_view get_title() const noexcept override {
            return g_app_info_get_display_name(m_app_info->gobj());
        }

        [[nodiscard]] virtual std::string_view get_subtitle() const noexcept override {
            auto desc = g_app_info_get_description(m_app_info->gobj());
            return desc ? desc : "";
        }

        [[nodiscard]] virtual std::string_view get_icon() const noexcept override {
            return m_icon_string;
        }

        [[nodiscard]] std::string get_id() const noexcept override { return m_app_info->get_id(); }

        using interfaces::Entry::set_score;
    };
} // namespace launcher::provider::desktop_entries

namespace launcher::providers {
    DesktopEntryProvider::DesktopEntryProvider() :
            interfaces::Provider(), m_available_applications(get_available_apps()) {}

    std::vector<std::shared_ptr<interfaces::Entry>> DesktopEntryProvider::query(
        const interfaces::Query &query) const {
        std::vector<std::shared_ptr<interfaces::Entry>> result {};
        result.reserve(m_available_applications.size());
        std::ranges::transform(m_available_applications,
            std::back_inserter(result),
            [&](const auto &app_info) -> std::shared_ptr<interfaces::Entry> {
                return std::make_shared<provider::desktop_entries::DesktopFileEntry>(app_info);
            });

        for (auto &result_entry : result) {
            auto &desktop_file_entry = static_cast<provider::desktop_entries::DesktopFileEntry &>(*result_entry);
            auto match_result = utils::fuzzy_match(query.get_query(), desktop_file_entry.get_title());
            desktop_file_entry.set_score(match_result.score);
        }
        return result;
    }
} // namespace launcher::providers