#include "config.h"

#include <filesystem>
#include <iostream>

#include "macros.h"
#include "utils/xdg_base_directory.h"

namespace po = boost::program_options;

namespace {
    constexpr char OptionHeight[]   = "height";
    constexpr char OptionWidth[]    = "width";
    constexpr char OptionIconSize[] = "icon-size";

    std::filesystem::path get_config_file_path() {
        auto path = launcher::get_config_dir();
        path.append("config");
        return path;
    }
} // namespace

namespace launcher {
    std::filesystem::path get_config_dir() {
        return xdg::base_directory::get_config_home().append(PROJECT_NAME);
    }

    std::filesystem::path get_state_dir() {
        return xdg::base_directory::get_state_home().append(PROJECT_NAME);
    }

    po::options_description options::make_options() {
        po::options_description options {};
        // clang-format off
        options.add_options()
            (OptionHeight, po::value<int>()->default_value(600))
            (OptionWidth, po::value<int>()->default_value(1300))
            (OptionIconSize, po::value<int>()->default_value(64));
        // clang-format on
        return options;
    }

    options::options([[maybe_unused]] int argc, [[maybe_unused]] char *argv[]) :
            m_results(), m_config_options(make_options()) {
        auto config_path  = get_config_file_path();
        auto parse_result = [&]() {
            try {
                return po::parse_config_file(config_path.c_str(), m_config_options, false);
            } catch (const po::reading_file &ex) {
                std::cerr << "Could not read config file:\n" << ex.what() << '\n';
            }
            return po::basic_parsed_options<char>(&m_config_options);
        }();
        po::store(parse_result, m_results);
        po::notify(m_results);
    }

    int options::get_height() const noexcept {
        return m_results[OptionHeight].as<int>();
    }

    int options::get_width() const noexcept {
        return m_results[OptionWidth].as<int>();
    }

    int options::get_icon_size() const noexcept {
        return m_results[OptionIconSize].as<int>();
    }
} // namespace launcher