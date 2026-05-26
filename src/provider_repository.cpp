#include "provider_repository.h"

#include "model/module_interfaces.h"

#include "providers/desktop-entries/provider.h"

#include <memory>
#include <optional>

namespace I = launcher::interfaces;

namespace {
    std::optional<launcher::ProviderRepository> instance;
}

namespace launcher {
    ProviderRepository &ProviderRepository::get_instance() {
        if (!instance) {
            instance.emplace();
        }
        return *instance;
    }

    ProviderRepository::ProviderRepository() :
            m_available_providers({std::make_shared<providers::DesktopEntryProvider>()}) {}

    std::vector<std::shared_ptr<I::Provider>> ProviderRepository::get_active_providers() const {
        return m_available_providers;
    }
} // namespace launcher