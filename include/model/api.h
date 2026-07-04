#ifndef LAUNCHER_API_H
#define LAUNCHER_API_H

#include <string>
#include <vector>

namespace launcher::interfaces {
    class execute_context {
        std::string startup_notify_token;

    public:
        void set_startup_notify_token(std::string str);
        std::vector<std::string> get_startup_notify_environment() const;
    };
} // namespace launcher::interfaces

#endif