#include "providers/desktop-entries/provider.h"

#include <format>
#include <ranges>

#include <boost/regex.hpp>
#include <desktop-entry.h>

#include "config.h"
#include "macros.h"
#include "utils/fuzzy_matcher.h"
#include "utils/spawn_helper.h"

namespace launcher::provider::desktop_entries {
    class DesktopFileEntry : public interfaces::Entry {
        std::unique_ptr<xdg::desktop_entry_spec::desktop_entry> m_desktop_entry;

    public:
        DesktopFileEntry(std::unique_ptr<xdg::desktop_entry_spec::desktop_entry> desktop_entry) :
                m_desktop_entry(std::move(desktop_entry)) {}

        void execute(interfaces::execute_context &e_context) const final {
            auto id = m_desktop_entry->get_id();
            r_assert(id.ends_with(".desktop"));
            id.resize(id.size() - 8);
            id = escape_systemd_string(id, false);

            m_desktop_entry->launch([&](xdg::desktop_entry_spec::launch_parameters &params) {
                spawn_context context {};
                context.executable = "/bin/sh";
                if (params.terminal) {
                    context.arguments = {"-c",
                        std::vformat(options::get_instance().get_terminal_cmd(),
                            std::make_format_args(params.command_list))};
                } else {
                    context.arguments = {"-c", std::move(params.command_list)};
                }
                context.environ           = {e_context.get_startup_notify_environment()};
                context.working_directory = params.working_directory;
                context.unit_name         = "app-" + id + "-" + make_unique_identifier();
                context.slice             = "app-" + id + ".slice";
                spawn_as_service(context);
            });
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

        [[nodiscard]] std::string_view get_exec() const noexcept {
            return m_desktop_entry->get_exec();
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
            auto match_result = utils::fuzzy_match_multiple(query.get_query(),
                std::initializer_list<std::string_view> {entry->get_title(), entry->get_exec()});
            entry->set_score(match_result.score);
            return std::shared_ptr<interfaces::Entry>(entry);
        });
        return {std::move_iterator(view.begin()), std::move_iterator(view.end())};
    }
} // namespace launcher::providers