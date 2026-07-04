#include "model/api.h"

namespace launcher::interfaces {
    void execute_context::set_startup_notify_token(std::string str) {
        startup_notify_token = std::move(str);
    }

    std::vector<std::string> execute_context::get_startup_notify_environment() const {
        if (startup_notify_token.empty()) {
            return {};
        }
        return {"XDG_ACTIVATION_TOKEN=" + startup_notify_token, "DESKTOP_STARTUP_ID=" + startup_notify_token};
    }
} // namespace launcher::interfaces