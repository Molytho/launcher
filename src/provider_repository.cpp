#include "provider_repository.h"

#include "model/module_interfaces.h"

#include "providers/console/provider.h"
#include "providers/desktop-entries/provider.h"

#include <memory>
#include <optional>
#include <ranges>

namespace I = launcher::interfaces;

namespace {
    std::optional<launcher::provider_repository> instance;

    std::shared_ptr<launcher::interfaces::provider> create_provider_by_name(std::string_view str) {
        if (str == "DesktopEntry") {
            return std::make_shared<launcher::providers::desktop_entry_provider>();
        } else if (str == "Console") {
            return std::make_shared<launcher::providers::console_provider>();
        } else {
            // TODO: Make specific exception
            throw std::runtime_error("Invalid provider name");
        }
    }
} // namespace

// TODO: Map implementation

namespace launcher {
    provider_repository::provider_repository(const options &options) :
            m_available_providers(make_available_providers(options)) {}

    std::vector<std::pair<char, std::shared_ptr<interfaces::provider>>>
        provider_repository::make_available_providers(const options &options) {
        auto &provider_config = options.get_provider_config();
        std::vector<std::pair<char, std::shared_ptr<interfaces::provider>>> results {};
        for (const auto &provider : provider_config) {
            results.emplace_back(provider.activation_token, create_provider_by_name(provider.name));
        }
        return results;
    }

    std::vector<std::shared_ptr<I::provider>> provider_repository::get_active_providers(char activation_char) const {
        std::vector<std::shared_ptr<I::provider>> result;
        auto res = std::views::filter(m_available_providers, [activation_char](const auto &provider) {
            return provider.first == activation_char;
        }) | std::views::transform([](const auto &provider) { return provider.second; });
        result.insert(result.cend(), res.begin(), res.end());
        return result;
    }
} // namespace launcher