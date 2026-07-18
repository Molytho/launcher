#ifndef LAUNCHER_DESKTOP_ENTRIES_PROVIDER_H
#define LAUNCHER_DESKTOP_ENTRIES_PROVIDER_H

#include "model/module_interfaces.h"

namespace launcher::provider::desktop_entries {
    class desktop_file_entry;
}

namespace launcher::providers {
    class desktop_entry_provider : public interfaces::provider {
        std::vector<std::shared_ptr<launcher::provider::desktop_entries::desktop_file_entry>> m_available_entries;

    public:
        desktop_entry_provider();

        std::vector<std::shared_ptr<interfaces::entry>> query(const interfaces::query &query) const override;
    };
} // namespace launcher::providers

#endif