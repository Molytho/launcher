#include "providers/desktop-entries/provider.h"

#include "utils/fuzzy_matcher.h"

namespace {
    std::vector<Glib::RefPtr<Gio::AppInfo>> get_available_apps() {
        Gio::init();
        std::vector<Glib::RefPtr<Gio::AppInfo>> result = Gio::AppInfo::get_all();
        std::erase_if(result, [](const auto &app_info) { return !app_info->should_show(); });
        return result;
    }
} // namespace

namespace launcher::provider::desktop_entries {
    class DesktopFileEntry : public interfaces::Entry {
        Glib::RefPtr<Gio::AppInfo> m_app_info;

    public:
        DesktopFileEntry(Glib::RefPtr<Gio::AppInfo> app_info) :
                interfaces::Entry(app_info->get_display_name(),
                    app_info->get_icon() ? app_info->get_icon()->to_string() : std::string()),
                m_app_info(std::move(app_info)) {}

        using interfaces::Entry::set_score;

        void execute() const final { m_app_info->launch(std::vector<Glib::RefPtr<Gio::File>> {}); }
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