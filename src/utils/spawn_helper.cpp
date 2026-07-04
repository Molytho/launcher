#include "utils/spawn_helper.h"

#include <array>
#include <cctype>
#include <charconv>
#include <format>
#include <spawn.h>
#include <sstream>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <system_error>
#include <unistd.h>

#include "config.h"
#include "macros.h"
#include "utils/owned_fd.h"
#include "utils/zstring_view.h"

namespace {
    using namespace string_utils;

    constexpr zstring_view SliceFormatString            = "--property=Slice={}";
    constexpr zstring_view WorkingDirectoryFormatString = "--property=WorkingDirectory={}";

    constexpr zstring_view EnvironmentPrefixString = "--property=Environment=";

    constexpr zstring_view RunExecutable = "systemd-run";
    constexpr zstring_view ArgUser       = "--user";
    constexpr zstring_view ArgExitType   = "--property=ExitType=cgroup";
    constexpr zstring_view ArgType       = "--property=Type=exec";
    constexpr zstring_view ArgRestart    = "--property=Restart=no";
    constexpr zstring_view ArgUnit       = "-u";
    constexpr zstring_view ArgScope      = "--scope";
    constexpr zstring_view ArgEnd        = "--";

    void score_or_service_make_args(std::vector<const char *> &args, launcher::spawn_context &context) {
        if (launcher::options::get_instance().should_spawn_as_service()) {
            for (auto &str : context.environ) {
                str.insert(str.begin(), EnvironmentPrefixString.begin(), EnvironmentPrefixString.end());
                args.push_back(str.c_str());
            }

            // TODO: Support for scope
            if (!context.working_directory.empty()) {
                context.working_directory
                    = std::format(WorkingDirectoryFormatString, std::move(context.working_directory));
                args.push_back(context.working_directory.c_str());
            }
        }
    }

    void scope_or_service_make_environ(std::vector<const char *> &environ, launcher::spawn_context &context) {
        if (!launcher::options::get_instance().should_spawn_as_service()) {
            environ.reserve(environ.size() + context.environ.size());
            for (const auto &str : context.environ) {
                environ.push_back(str.c_str());
            }
        }
    }

    std::vector<const char *> make_args(launcher::spawn_context &context) {
        std::vector<const char *> res {RunExecutable.c_str(),
            ArgUser.c_str(),
            ArgExitType.c_str(),
            ArgType.c_str(),
            ArgRestart.c_str(),
            ArgUnit.c_str(),
            context.unit_name.c_str()};

        if (!launcher::options::get_instance().should_spawn_as_service()) {
            res.push_back(ArgScope.c_str());
        }

        if (!context.slice.empty()) {
            context.slice = std::format(SliceFormatString, std::move(context.slice));
            res.push_back(context.slice.c_str());
        }

        score_or_service_make_args(res, context);

        res.push_back(ArgEnd.c_str());

        // Application args
        res.push_back(context.executable.c_str());
        for (const auto &arg : context.arguments) {
            res.push_back(arg.c_str());
        }
        res.push_back(nullptr);

        return res;
    }

    std::vector<const char *> make_environ(launcher::spawn_context &context) {
        char **begin = environ;
        char **end   = [&]() {
            char **it = environ;
            r_assert(it);
            while (*it != nullptr) {
                it++;
            }
            return it;
        }();
        std::vector<const char *> res {begin, end};
        scope_or_service_make_environ(res, context);
        res.push_back(nullptr);
        return res;
    }

    void posix_spawnp(pid_t *pid, const char *file, const posix_spawn_file_actions_t *file_actions,
        const posix_spawnattr_t *attrp, const char *const argv[], const char *const envp[]) {
        int res
            = posix_spawnp(pid, file, file_actions, attrp, const_cast<char *const *>(argv), const_cast<char *const *>(envp));
        if (res < 0) {
            throw std::system_error(errno, std::system_category());
        }
    }

    pid_t spawn(const char *const argv[], const char *const envp[]) {
        pid_t pid {};
        posix_spawnp(&pid, RunExecutable.c_str(), nullptr, nullptr, argv, envp);
        return pid;
    }
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

        auto args    = make_args(context);
        auto environ = make_environ(context);
        spawn(args.data(), environ.data());
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