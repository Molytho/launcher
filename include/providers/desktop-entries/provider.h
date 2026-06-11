#ifndef LAUNCHER_DESKTOP_ENTRIES_PROVIDER_H
#define LAUNCHER_DESKTOP_ENTRIES_PROVIDER_H

#include "model/module_interfaces.h"

namespace launcher::provider::desktop_entries {
    class DesktopFileEntry;
}

namespace launcher::providers {
    class DesktopEntryProvider : public interfaces::Provider {
        std::vector<std::shared_ptr<provider::desktop_entries::DesktopFileEntry>> m_available_entries;

    public:
        DesktopEntryProvider();

        std::vector<std::shared_ptr<interfaces::Entry>> query(const interfaces::Query &query) const override;
    };
} // namespace launcher::providers

#endif