#ifndef LAUNCHER_PROVIDER_REPOSITORY_H
#define LAUNCHER_PROVIDER_REPOSITORY_H

#include "model/module_interfaces.h"

#include <memory>

namespace launcher {
    class ProviderRepository {
        std::vector<std::shared_ptr<interfaces::Provider>> m_available_providers;

    public:
        static ProviderRepository &get_instance();

        ProviderRepository();

        std::vector<std::shared_ptr<interfaces::Provider>> get_active_providers() const;
    };
} // namespace launcher

#endif
