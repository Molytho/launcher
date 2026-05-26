#ifndef LAUNCHER_DESKTOP_ENTRIES_PROVIDER_H
#define LAUNCHER_DESKTOP_ENTRIES_PROVIDER_H

#include <giomm.h>

#include "model/module_interfaces.h"

namespace launcher::providers {
    class DesktopEntryProvider : public interfaces::Provider {
        std::vector<Glib::RefPtr<Gio::AppInfo>> m_available_applications;

    public:
        DesktopEntryProvider();

        std::vector<std::shared_ptr<interfaces::Entry>> query(const interfaces::Query &query) const override;
    };
} // namespace launcher::providers

#endif