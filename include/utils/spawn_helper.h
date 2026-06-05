#ifndef LAUNCHER_SPAWN_HELPER_H

# include <string>
# include <vector>

namespace launcher {
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