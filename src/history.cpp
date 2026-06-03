#include "history.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>

#include "config.h"

using namespace launcher;
using namespace launcher::interfaces;

namespace fs = std::filesystem;

namespace {
    fs::path get_history_file_path(bool mkdir = false) {
        if (mkdir) {
            fs::create_directories(launcher::get_state_dir());
        }
        return launcher::get_state_dir().append("history");
    }

    // TODO: History locking
    std::vector<std::string> read_history(size_t max_history_size) {
        std::ifstream istream {get_history_file_path()};
        if (!istream.is_open()) {
            return {};
        }
        std::vector<std::string> result;
        for (size_t i = max_history_size; i > 0 && !istream.eof(); i--) {
            std::string line;
            if (std::getline(istream, line)) {
                result.push_back(std::move(line));
            }
        }
        return result;
    }

    void write_history(const std::vector<std::string> &history) {
        std::ofstream ostream {get_history_file_path(true), std::ios_base::out | std::ios_base::trunc};
        for (const auto &entry : history) {
            if (entry.find('\n') != std::string::npos) {
                std::cerr << "History entry id contains newline. Skipping.\n";
                continue;
            }
            ostream << entry << '\n';
        }
    }
} // namespace

namespace launcher {
    history_provider::history_provider(const options &options) :
            m_history_entries(read_history(options.get_history_max_size())), m_options(options) {}

    history_provider::~history_provider() {
        if (m_changed) {
            write_history(m_history_entries);
        }
    }

    void history_provider::boost_history_entries(std::vector<std::shared_ptr<Entry>> &entries) const {
        double boost             = m_options.get_history_boost();
        const double boost_decay = m_options.get_history_decay();
        for (const std::string &history_entry : m_history_entries) {
            auto it = std::ranges::find_if(entries,
                [&](const auto &entry) { return entry->get_id() == history_entry; });
            if (it != entries.end()) {
                (*it)->boost_score(Score(boost));
            }
            boost -= boost_decay;
        }
    }

    void history_provider::add_to_history(const Entry &entry) {
        auto it = std::ranges::find_if(m_history_entries,
            [&](const auto &history_entry) { return entry.get_id() == history_entry; });
        if (it != m_history_entries.end()) {
            it++;
        } else if (m_history_entries.size() < m_options.get_history_max_size()) {
            m_history_entries.resize(m_history_entries.size() + 1);
            it = m_history_entries.end();
        }
        std::shift_right(m_history_entries.begin(), it, 1);
        m_history_entries.front() = entry.get_id();
        m_changed                 = true;
    }
} // namespace launcher