#ifndef LAUNCHER_HISTORY_H
#define LAUNCHER_HISTORY_H

# include "model/module_interfaces.h"

# include <vector>
#include <string>

namespace launcher {
    class history_provider {
        std::vector<std::string> m_history_entries;

    public:
        history_provider();
        ~history_provider();

        void boost_history_entries(std::vector<std::shared_ptr<interfaces::Entry>> &entries) const;

        void add_to_history(const interfaces::Entry &entry);
    };
} // namespace launcher

#endif