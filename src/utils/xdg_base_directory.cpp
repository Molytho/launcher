#include "utils/xdg_base_directory.h"

#include <cstdlib>

namespace {
    std::string get_home_var() {
        auto var = getenv("HOME");
        if (!var) {
            throw xdg::base_directory::mandatory_envvar_missing("HOME");
        }
        return var;
    }

    std::string get_string_var(const char *name) {
        auto var = getenv(name);
        return var ? std::string(var) : std::string();
    }

    std::filesystem::path get_var_or_default(const char *name, const char *default_value) {
        std::filesystem::path path = [&]() {
            std::string res = get_string_var(name);
            if (res.empty()) {
                res = get_home_var().append(default_value);
            }
            return res;
        }();
        if (path.is_relative()) {
            std::stringstream strstr;
            strstr << name << " or HOME";
            throw xdg::base_directory::mandatory_envvar_relative(std::move(strstr.str()));
        }
        return path;
    }
} // namespace

namespace xdg::base_directory {
    std::filesystem::path get_data_home() {
        return get_var_or_default("XDG_DATA_HOME", "/.local/share");
    }

    std::filesystem::path get_config_home() {
        return get_var_or_default("XDG_CONFIG_HOME", "/.config");
    }

    std::filesystem::path get_state_home() {
        return get_var_or_default("XDG_STATE_HOME", "/.local/state");
    }
} // namespace xdg::base_directory