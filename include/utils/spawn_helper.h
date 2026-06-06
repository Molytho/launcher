#ifndef LAUNCHER_SPAWN_HELPER_H

# include <string>
# include <vector>

namespace launcher {
    std::string escape_systemd_string(std::string_view str, bool start_of_string = true);

    struct spawn_context {
        std::string executable;
        std::vector<std::string> arguments;
        std::string unit_name;
        std::string slice;
    };

    void spawn_as_service(spawn_context &context);

    std::string make_unique_identifier();
} // namespace launcher

#endif