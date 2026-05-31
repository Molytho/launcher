#include "history.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>

#include "config.h"

namespace {
    constexpr size_t max_history_size                 = 10;
    constexpr launcher::interfaces::Score boost_value = 20;
    constexpr launcher::interfaces::Score boost_decay = 2;

    std::filesystem::path get_history_file_path() {
        return launcher::get_state_dir().append("history");
    }

    std::vector<std::string> read_history() {
        std::ifstream istream {get_history_file_path()};
        if (!istream.is_open()) {
            return {};
        }
        std::vector<std::string> result;
        result.reserve(max_history_size);
        for (size_t i = max_history_size; i > 0 && !istream.eof(); i--) {
            std::string line;
            if (std::getline(istream, line)) {
                result.push_back(std::move(line));
            }
        }
        return result;
    }

    void write_history(const std::vector<std::string> &history) {
        std::ofstream ostream {get_history_file_path(), std::ios_base::out | std::ios_base::trunc};
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
    history_provider::history_provider() : m_history_entries(read_history()) {}

    history_provider::~history_provider() {
        if (m_changed) {
            write_history(m_history_entries);
        }
    }

    void history_provider::boost_history_entries(std::vector<std::shared_ptr<interfaces::Entry>> &entries) const {
        interfaces::Score boost = boost_value;
        for (const std::string &history_entry : m_history_entries) {
            auto it = std::ranges::find_if(entries,
                [&](const auto &entry) { return entry->get_id() == history_entry; });
            if (it != entries.end()) {
                (*it)->boost_score(boost);
            }
            boost -= boost_decay;
        }
    }

    void history_provider::add_to_history(const interfaces::Entry &entry) {
        if (m_history_entries.size() < max_history_size) {
            m_history_entries.resize(m_history_entries.size() + 1);
        }
        auto it = std::ranges::find_if(m_history_entries,
            [&](const auto &history_entry) { return entry.get_id() == history_entry; });
        std::shift_right(m_history_entries.begin(), it, 1);
        m_history_entries.front() = entry.get_id();
        m_changed = true;
    }
} // namespace launcher