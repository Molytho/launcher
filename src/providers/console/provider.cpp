#include "providers/console/provider.h"

#include <spawn.h>
#include <unistd.h>

#include "utils/spawn_helper.h"

namespace {
    constexpr launcher::interfaces::Score EntryScore = 1000;

    constexpr std::string_view Icon = "Alacritty";
} // namespace

namespace launcher::provider::console {
    class ConsoleEntry : public interfaces::Entry {
        std::string m_command;

    public:
        ConsoleEntry() = default;

        void execute() const final {
            spawn_context context {};
            context.executable = "/usr/bin/alacritty";
            context.arguments  = {"-e", "sh", "-c", m_command};
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

        [[nodiscard]] virtual std::string_view get_icon() const noexcept override { return Icon; }

        [[nodiscard]] std::string get_id() const noexcept override {
            return "command_" + m_command;
        }

        void reset(std::string_view str) {
            m_command = str;
            set_score(EntryScore);
        }
    };
} // namespace launcher::provider::console

namespace launcher::providers {
    ConsoleProvider::ConsoleProvider() :
            interfaces::Provider(), m_entry(std::make_shared<provider::console::ConsoleEntry>()) {}

    std::vector<std::shared_ptr<interfaces::Entry>> ConsoleProvider::query(const interfaces::Query &query) const {
        if (query.get_query().empty()) {
            return {};
        }
        m_entry->reset(query.get_query());
        return {m_entry};
    }
} // namespace launcher::providers