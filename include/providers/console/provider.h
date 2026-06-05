#ifndef LAUNCHER_CONSOLE_PROVIDER_H
#define LAUNCHER_CONSOLE_PROVIDER_H

#include "model/module_interfaces.h"

namespace launcher::provider::console {
    class ConsoleEntry;
}

namespace launcher::providers {
    class ConsoleProvider : public interfaces::Provider {
        std::shared_ptr<provider::console::ConsoleEntry> m_entry;

    public:
        ConsoleProvider();

        std::vector<std::shared_ptr<interfaces::Entry>> query(const interfaces::Query &query) const override;
    };
} // namespace launcher::providers

#endif