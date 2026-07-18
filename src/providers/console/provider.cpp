#include "providers/console/provider.h"

#include <format>
#include <spawn.h>
#include <unistd.h>

#include "config.h"
#include "utils/spawn_helper.h"

namespace {
    constexpr launcher::interfaces::score EntryScore = 1000;

    std::string_view get_icon() {
        return launcher::options::get_instance().get_console_provider_icon();
    }

    constexpr std::vector<std::shared_ptr<launcher::interfaces::action>> EmptyActions;
} // namespace

namespace launcher::provider::console {
    class console_entry : public interfaces::entry {
        static std::string_view Icon;

        std::string m_command;

    public:
        console_entry() = default;

        void execute(interfaces::execute_context &e_context) const final {
            spawn_context context {};
            context.executable = "/bin/sh";
            context.arguments  = {"-c",
                std::vformat(options::get_instance().get_terminal_cmd(), std::make_format_args(m_command))};
            context.environ    = {e_context.get_startup_notify_environment()};
            context.unit_name  = "command-" + make_unique_identifier();
            context.slice      = "app.slice";
            spawn_as_service(context);
        }

        [[nodiscard]] virtual std::string_view get_title() const noexcept override {
            // TODO: Localization?
            return "Run command in console:";
        }

        [[nodiscard]] virtual std::string_view get_subtitle() const noexcept override {
            return m_command;
        }

        [[nodiscard]] virtual interfaces::icon_variant get_icon() const noexcept override {
            if (Icon.empty()) {
                Icon = ::get_icon();
            }
            return Icon;
        }

        [[nodiscard]] std::string get_id() const noexcept override {
            return "command_" + m_command;
        }

        const std::vector<std::shared_ptr<launcher::interfaces::action>> &get_actions() const override {
            return EmptyActions;
        }

        void reset(std::string_view str) {
            m_command = str;
            set_score(EntryScore);
        }
    };

    std::string_view console_entry::Icon {};
} // namespace launcher::provider::console

namespace launcher::providers {
    console_provider::console_provider() :
            interfaces::provider(), m_entry(std::make_shared<launcher::provider::console::console_entry>()) {}

    std::vector<std::shared_ptr<interfaces::entry>> console_provider::query(const interfaces::query &query) const {
        if (query.get_query().empty()) {
            return {};
        }
        m_entry->reset(query.get_query());
        return {m_entry};
    }
} // namespace launcher::providers