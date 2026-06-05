#include "provider_repository.h"

#include "macros.h"
#include "model/module_interfaces.h"

#include "providers/console/provider.h"
#include "providers/desktop-entries/provider.h"

#include <memory>
#include <optional>
#include <ranges>

namespace I = launcher::interfaces;

namespace {
    std::optional<launcher::ProviderRepository> instance;

    std::shared_ptr<launcher::interfaces::Provider> create_provider_by_name(std::string_view str) {
        if (str == "DesktopEntry") {
            return std::make_shared<launcher::providers::DesktopEntryProvider>();
        } else if (str == "Console") {
            return std::make_shared<launcher::providers::ConsoleProvider>();
        } else {
            // TODO: Make specific exception
            throw std::runtime_error("Invalid provider name");
        }
    }
} // namespace

// TODO: Map implementation

namespace launcher {
    ProviderRepository &ProviderRepository::get_instance() {
        r_assert(instance);
        return *instance;
    }

    void ProviderRepository::init(const options &options) {
        instance.emplace(options);
    }

    ProviderRepository::ProviderRepository(const options &options) :
            m_available_providers(make_available_providers(options)) {}

    std::vector<std::pair<char, std::shared_ptr<interfaces::Provider>>>
        ProviderRepository::make_available_providers(const options &options) {
        auto &provider_config = options.get_provider_config();
        std::vector<std::pair<char, std::shared_ptr<interfaces::Provider>>> results {};
        for (const auto &provider : provider_config) {
            results.emplace_back(provider.activation_token, create_provider_by_name(provider.name));
        }
        return results;
    }

    std::vector<std::shared_ptr<I::Provider>> ProviderRepository::get_active_providers(char activation_char) const {
        std::vector<std::shared_ptr<I::Provider>> result;
        auto res = std::views::filter(m_available_providers, [activation_char](const auto &provider) {
            return provider.first == activation_char;
        }) | std::views::transform([](const auto &provider) { return provider.second; });
        result.insert(result.cend(), res.begin(), res.end());
        return result;
    }
} // namespace launcher