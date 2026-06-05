#include "utils/spawn_helper.h"

#include <spawn.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <system_error>
#include <unistd.h>
#include <sstream>

#include "macros.h"
#include "utils/owned_fd.h"

namespace {
    constexpr char SliceFormatString[] = "--property=Slice=%s";
}

namespace launcher {
    void spawn_as_service(spawn_context &context) {
        r_assert(!context.executable.empty());
        r_assert(!context.unit_name.empty());

        char run_executable[] = "systemd-run";
        char arg1[]           = "--user";
        char arg_unit[]       = "-u";
        char arg_end[]        = "--";
        std::string slice {}; // Here because of lifetime

        std::vector<char *> args {run_executable, arg1, arg_unit};
        args.push_back(context.unit_name.data());
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
        sstr << 'p'<< std::to_string(pid) << "-i" << std::to_string(buffer.st_ino);
        return std::move(sstr).str();
    }
} // namespace launcher