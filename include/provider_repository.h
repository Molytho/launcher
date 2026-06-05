#ifndef LAUNCHER_PROVIDER_REPOSITORY_H
#define LAUNCHER_PROVIDER_REPOSITORY_H

#include <memory>

#include "config.h"
#include "model/module_interfaces.h"

namespace launcher {
    class ProviderRepository {
        std::vector<std::pair<char, std::shared_ptr<interfaces::Provider>>> m_available_providers;

        static std::vector<std::pair<char, std::shared_ptr<interfaces::Provider>>> make_available_providers(
            const options &options);


    public:
        static void init(const options &options);
        static ProviderRepository &get_instance();

        ProviderRepository(const options &options);

        std::vector<std::shared_ptr<interfaces::Provider>> get_active_providers(char activation_char = 0) const;
    };
} // namespace launcher

#endif
