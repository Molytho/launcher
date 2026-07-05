#ifndef LAUNCHER_CONFIG_H
#define LAUNCHER_CONFIG_H

#include <filesystem>
#include <iostream>

#include <boost/program_options.hpp>

#include "model/module_interfaces.h"
#include "utils/singleton.h"

namespace launcher {
    std::filesystem::path get_config_dir();
    std::filesystem::path get_state_dir();

    struct provider_config {
        std::string name;
        char activation_token;
    };

    std::istream &operator>>(std::istream &is, provider_config &out);

    class options : public singleton_impl<options> {
        boost::program_options::variables_map m_results;
        boost::program_options::options_description m_config_options;

        static boost::program_options::options_description make_options();

    public:
        options(int argc, char *argv[]);

        int get_height() const noexcept;
        int get_width() const noexcept;
        int get_icon_size() const noexcept;

        size_t get_history_max_size() const noexcept;
        interfaces::Score get_history_boost() const noexcept;
        double get_history_decay() const noexcept;

        bool should_spawn_as_service() const noexcept;

        const std::vector<provider_config> &get_provider_config() const noexcept;

        const std::string &get_terminal_cmd() const noexcept;
    };
} // namespace launcher

#endif