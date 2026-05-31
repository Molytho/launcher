#include "config.h"

#include "macros.h"
#include "utils/xdg_base_directory.h"

namespace launcher {
    std::filesystem::path get_config_dir() {
        return xdg::base_directory::get_config_home().append(PROJECT_NAME);
    }

    std::filesystem::path get_state_dir() {
        return xdg::base_directory::get_state_home().append(PROJECT_NAME);
    }
} // namespace launcher