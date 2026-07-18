#ifndef LAUNCHER_CONSOLE_PROVIDER_H
#define LAUNCHER_CONSOLE_PROVIDER_H

#include "model/module_interfaces.h"

namespace launcher::provider::console {
    class console_entry;
}

namespace launcher::providers {
    class console_provider : public interfaces::provider {
        std::shared_ptr<launcher::provider::console::console_entry> m_entry;

    public:
        console_provider();

        std::vector<std::shared_ptr<interfaces::entry>> query(const interfaces::query &query) const final;
    };
} // namespace launcher::providers

#endif