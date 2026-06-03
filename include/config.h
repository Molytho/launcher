#ifndef LAUNCHER_CONFIG_H
#define LAUNCHER_CONFIG_H

#include <filesystem>

#include <boost/program_options.hpp>

#include "model/module_interfaces.h"

namespace launcher {
    std::filesystem::path get_config_dir();
    std::filesystem::path get_state_dir();

    class options {
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
    };
} // namespace launcher

#endif