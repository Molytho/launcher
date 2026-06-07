#include "utils/spawn_helper.h"

#include <array>
#include <cctype>
#include <charconv>
#include <spawn.h>
#include <sstream>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <system_error>
#include <unistd.h>

#include "config.h"
#include "macros.h"
#include "utils/owned_fd.h"

namespace {
    constexpr char SliceFormatString[] = "--property=Slice=%s";
} // namespace

namespace launcher {
    std::string escape_systemd_string(std::string_view str, bool start_of_string) {
        if (str.empty()) {
            return {};
        }

        std::string result;
        result.reserve(str.size());
        // Dot at first position needs to be escaped
        if (start_of_string && str.at(0) == '.') {
            result.append("\\x2e");
            str = str.substr(1);
        }
        for (char c : str) {
            if (std::isalnum(c) || c == ':' || c == '_' || c == '.') {
                result.push_back(c);
            } else if (c == '/') {
                result.push_back('-');
            } else {
                result.append("\\x");
                std::array<char, 3> buffer {0, 0, 0};
                std::to_chars(buffer.data(), buffer.data() + 2, (unsigned char)(c), 16);
                result.append(buffer.data());
            }
        }
        return result;
    }

    void spawn_as_service(spawn_context &context) {
        r_assert(!context.executable.empty());
        r_assert(!context.unit_name.empty());

        char run_executable[] = "systemd-run";
        char arg1[]           = "--user";
        char arg_exit_type[]  = "--property=ExitType=cgroup";
        char arg_type[]       = "--property=Type=exec";
        char arg_restart[]    = "--property=Restart=no";
        char arg_unit[]       = "-u";
        char arg_scope[]      = "--scope";
        char arg_end[]        = "--";
        std::string slice {}; // Here because of lifetime

        std::vector<char *> args {run_executable, arg1, arg_exit_type, arg_type, arg_restart, arg_unit};
        args.push_back(context.unit_name.data());
        if (!options::get_instance().should_spawn_as_service()) {
            args.push_back(arg_scope);
        }
        if (!context.slice.empty()) {
            slice.resize(sizeof(SliceFormatString) + context.slice.size() - 1);
            int res = snprintf(slice.data(), slice.size(), SliceFormatString, context.slice.c_str());
            args.push_back(slice.data());
            r_assert(res >= 0 && size_t(res) < slice.size());
        }
        args.push_back(arg_end);
        args.push_back(context.executable.data());
        for (auto &arg : context.arguments) {
            args.push_back(arg.data());
        }
        args.push_back(nullptr);

        pid_t pid {0};
        int res = posix_spawnp(&pid, run_executable, nullptr, nullptr, args.data(), environ);
        if (res < 0) {
            throw std::system_error(errno, std::system_category());
        }
    }

    std::string make_unique_identifier() {
        pid_t pid = getpid();
        auto fd   = make_checked_owned_fd(syscall(SYS_pidfd_open, pid, 0));

        struct stat buffer {};
        if (fstat(fd, &buffer) < 0) {
            throw std::system_error(errno, std::system_category());
        }

        std::stringstream sstr {};
        sstr << 'p' << std::to_string(pid) << "-i" << std::to_string(buffer.st_ino);
        return std::move(sstr).str();
    }
} // namespace launcher