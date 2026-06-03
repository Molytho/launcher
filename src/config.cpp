#include "config.h"

#include <filesystem>
#include <iostream>

#include "macros.h"
#include "model/module_interfaces.h"
#include "utils/xdg_base_directory.h"

namespace po = boost::program_options;

namespace {
    constexpr char OptionHeight[]   = "height";
    constexpr char OptionWidth[]    = "width";
    constexpr char OptionIconSize[] = "icon-size";

    constexpr char OptionHistoryMaxSize[] = "history.max-size";
    constexpr char OptionHistoryBoost[]   = "history.boost";
    constexpr char OptionHistoryDecay[]   = "history.decay";

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
            (OptionIconSize, po::value<int>()->default_value(64))
            (OptionHistoryMaxSize, po::value<size_t>()->default_value(64))
            (OptionHistoryBoost, po::value<interfaces::Score>()->default_value(10))
            (OptionHistoryDecay, po::value<double>());
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

    size_t options::get_history_max_size() const noexcept {
        return m_results[OptionHistoryMaxSize].as<size_t>();
    }

    interfaces::Score options::get_history_boost() const noexcept {
        return m_results[OptionHistoryBoost].as<interfaces::Score>();
    }

    double options::get_history_decay() const noexcept {
        auto it = m_results.find(OptionHistoryDecay);
        if (it != m_results.end()) {
            return it->second.as<double>();
        } else {
            return static_cast<double>(get_history_boost()) / get_history_max_size();
        }
    }
} // namespace launcher