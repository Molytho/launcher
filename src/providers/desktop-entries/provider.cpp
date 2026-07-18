#include "providers/desktop-entries/provider.h"

#include <format>
#include <ranges>

#include <boost/regex.hpp>
#include <desktop-entry.h>

#include "config.h"
#include "macros.h"
#include "utils/fuzzy_matcher.h"
#include "utils/ranges_helper.h"
#include "utils/spawn_helper.h"

namespace {
    [[nodiscard]] std::string make_id(xdg::desktop_entry_spec::desktop_entry &entry) {
        auto id = entry.get_id();
        r_assert(id.ends_with(".desktop"));
        id.resize(id.size() - 8);
        id = launcher::escape_systemd_string(id, false);
        return id;
    }

    void launch(launcher::interfaces::execute_context &e_context, std::string id,
        xdg::desktop_entry_spec::launch_parameters &params) {
        launcher::spawn_context context {};
        context.executable = "/bin/sh";
        if (params.terminal) {
            context.arguments = {"-c",
                std::vformat(launcher::options::get_instance().get_terminal_cmd(),
                    std::make_format_args(params.command_list))};
        } else {
            context.arguments = {"-c", std::move(params.command_list)};
        }
        context.environ           = {e_context.get_startup_notify_environment()};
        context.working_directory = params.working_directory;
        context.unit_name         = "app-" + id + "-" + launcher::make_unique_identifier();
        context.slice             = "app-" + id + ".slice";
        spawn_as_service(context);
    }
} // namespace

namespace launcher::provider::desktop_entries {
    class desktop_action : public interfaces::action {
        std::shared_ptr<xdg::desktop_entry_spec::application_action> m_action;

    public:
        desktop_action(std::shared_ptr<xdg::desktop_entry_spec::application_action> action) :
                m_action(std::move(action)) {}

        [[nodiscard]] std::string_view get_title() const noexcept final {
            return m_action->get_name().get();
        }

        [[nodiscard]] interfaces::icon_variant get_icon() const noexcept final {
            if (auto icon = m_action->get_icon(); icon) {
                return icon->get();
            } else {
                return "";
            }
        }

        void execute(interfaces::execute_context &e_context) const final {
            auto entry = m_action->try_get_entry();
            if (!entry) {
                throw std::logic_error("desktop entry already destroyed");
            }
            m_action->launch([&](xdg::desktop_entry_spec::launch_parameters &params) {
                launch(e_context, make_id(*entry), params);
            });
        }
    };

    class desktop_file_entry : public interfaces::entry {
        std::shared_ptr<xdg::desktop_entry_spec::desktop_entry> m_desktop_entry;
        std::vector<std::shared_ptr<launcher::interfaces::action>> m_actions;

        static std::vector<std::shared_ptr<launcher::interfaces::action>> make_actions(
            xdg::desktop_entry_spec::desktop_entry *entry) {
            r_assert(entry);
            return std::views::transform(entry->get_actions(),
                       [](const std::shared_ptr<xdg::desktop_entry_spec::application_action> &action)
                           -> std::shared_ptr<launcher::interfaces::action> {
                           return std::make_shared<desktop_action>(action);
                       })
                   | view_helper::to_vector;
        }

    public:
        desktop_file_entry(std::shared_ptr<xdg::desktop_entry_spec::desktop_entry> desktop_entry) :
                m_desktop_entry(std::move(desktop_entry)),
                m_actions(make_actions(m_desktop_entry.get())) {}

        [[nodiscard]] virtual std::string_view get_title() const noexcept final {
            return m_desktop_entry->get_name().get();
        }

        [[nodiscard]] virtual std::string_view get_subtitle() const noexcept final {
            auto desc = m_desktop_entry->get_comment();
            return desc ? std::string_view(desc->get()) : "";
        }

        [[nodiscard]] virtual interfaces::icon_variant get_icon() const noexcept final {
            auto icon = m_desktop_entry->get_icon();
            return icon ? std::string_view(icon->get()) : "";
        }

        [[nodiscard]] std::string get_id() const noexcept final {
            return m_desktop_entry->get_id();
        }

        [[nodiscard]] std::string_view get_exec() const noexcept {
            return m_desktop_entry->get_exec();
        }

        const std::vector<std::shared_ptr<launcher::interfaces::action>> &get_actions() const override {
            return m_actions;
        }

        using interfaces::entry::set_score;

        void execute(interfaces::execute_context &e_context) const final {
            m_desktop_entry->launch([&](xdg::desktop_entry_spec::launch_parameters &params) {
                launch(e_context, make_id(*m_desktop_entry), params);
            });
        }
    };
} // namespace launcher::provider::desktop_entries

namespace {
    std::vector<std::shared_ptr<launcher::provider::desktop_entries::desktop_file_entry>> make_available_entries() {
        return std::views::filter(xdg::desktop_entry_spec::get_all_desktop_entries(), [](const auto &entry) {
            return entry->should_show();
        }) | std::views::transform([](auto &app_info) {
            return std::make_shared<launcher::provider::desktop_entries::desktop_file_entry>(std::move(app_info));
        }) | view_helper::to_vector;
    }
} // namespace

namespace launcher::providers {
    desktop_entry_provider::desktop_entry_provider() :
            interfaces::provider(), m_available_entries(make_available_entries()) {}

    std::vector<std::shared_ptr<interfaces::entry>> desktop_entry_provider::query(
        const interfaces::query &query) const {
        return std::views::transform(m_available_entries, [&query](const auto &entry) -> std::shared_ptr<interfaces::entry> {
            auto match_result = utils::fuzzy_match_multiple(query.get_query(),
                std::initializer_list<std::string_view> {entry->get_title(), entry->get_exec()});
            entry->set_score(match_result.score);
            return entry;
        }) | view_helper::to_vector;
    }
} // namespace launcher::providers