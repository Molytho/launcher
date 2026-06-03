#ifndef LAUNCHER_XDG_BASE_DIRECTORY_H
#define LAUNCHER_XDG_BASE_DIRECTORY_H

#include <filesystem>
#include <stdexcept>

namespace xdg::base_directory {
    class mandatory_envvar_missing : public std::runtime_error {
        using std::runtime_error::runtime_error;
    };

    class mandatory_envvar_relative : public std::runtime_error {
        using std::runtime_error::runtime_error;
    };

    std::filesystem::path get_data_home();
    std::filesystem::path get_config_home();
    std::filesystem::path get_state_home();
} // namespace xdg::base_directory

#endif