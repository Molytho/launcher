#ifndef LAUNCHER_PROVIDER_REPOSITORY_H
#define LAUNCHER_PROVIDER_REPOSITORY_H

#include <memory>

#include "config.h"
#include "model/module_interfaces.h"
#include "utils/singleton.h"

namespace launcher {
    class provider_repository : public singleton_impl<provider_repository> {
        std::vector<std::pair<char, std::shared_ptr<interfaces::provider>>> m_available_providers;

        static std::vector<std::pair<char, std::shared_ptr<interfaces::provider>>> make_available_providers(
            const options &options);

    public:
        provider_repository(const options &options = options::get_instance());

        std::vector<std::shared_ptr<interfaces::provider>> get_active_providers(char activation_char = 0) const;
    };
} // namespace launcher

#endif
