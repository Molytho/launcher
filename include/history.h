#ifndef LAUNCHER_HISTORY_H
#define LAUNCHER_HISTORY_H

#include "model/module_interfaces.h"

#include <string>
#include <vector>

#include "config.h"

namespace launcher {
    class history_provider {
        bool m_changed {false};
        std::vector<std::string> m_history_entries;
        const options &m_options;

    public:
        history_provider(const options &options);
        ~history_provider();

        void boost_history_entries(std::vector<std::shared_ptr<interfaces::Entry>> &entries) const;

        void add_to_history(const interfaces::Entry &entry);
    };
} // namespace launcher

#endif