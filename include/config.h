#ifndef LAUNCHER_CONFIG_H
#define LAUNCHER_CONFIG_H

#include <filesystem>

namespace launcher {
    std::filesystem::path get_config_dir();
    std::filesystem::path get_state_dir();
}

#endif