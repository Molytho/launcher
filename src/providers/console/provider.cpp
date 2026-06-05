#include "providers/console/provider.h"

#include <array>
#include <cstdio>

#include <spawn.h>
#include <unistd.h>

#include "macros.h"

namespace {
    constexpr launcher::interfaces::Score EntryScore = 1000;

    constexpr size_t MaxCommandSize   = 100;
    constexpr char ExecFormatString[] = "/usr/bin/alacritty -e sh -c \"%s\"";
    constexpr std::string_view Icon   = "Alacritty";
} // namespace

namespace launcher::provider::console {
    class ConsoleEntry : public interfaces::Entry {
        std::string m_command;

    public:
        ConsoleEntry() = default;

        void execute() const final {
            std::array<char, MaxCommandSize + sizeof(ExecFormatString)> arg_buffer {};
            int res = snprintf(arg_buffer.data(), arg_buffer.size(), ExecFormatString, m_command.c_str());
            if (res < 0 || size_t(res) >= arg_buffer.size()) {
                throw std::runtime_error("Failed to make exec string");
            }
            pid_t pid {0};
            char path[]                = "/bin/sh";
            char first_arg[]           = "-c";
            std::array<char *, 4> argv = {path, first_arg, arg_buffer.data(), nullptr};
            res = posix_spawn(&pid, "/bin/sh", nullptr, nullptr, argv.data(), environ);
            r_assert(res >= 0);
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