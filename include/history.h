#ifndef LAUNCHER_HISTORY_H
#define LAUNCHER_HISTORY_H

#include "model/module_interfaces.h"

#include <string>
#include <vector>

#include "config.h"
#include "utils/singleton.h"

namespace launcher {
    class history_provider : public singleton_impl<history_provider> {
        bool m_changed {false};
        std::vector<std::string> m_history_entries;
        const options &m_options;

    public:
        history_provider(const options &options = options::get_instance());
        ~history_provider();

        void boost_history_entries(std::vector<std::shared_ptr<interfaces::entry>> &entries) const;

        void add_to_history(const interfaces::entry &entry);
    };
} // namespace launcher

#endif