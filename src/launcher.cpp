#include <algorithm>
#include <iostream>

#include "provider_repository.h"

using namespace launcher;

std::ostream &operator<<(std::ostream &os, const interfaces::Entry &entry) {
    return os
           << entry.get_title()
           << ", "
           << entry.get_subtitle()
           << ", "
           << entry.get_icon()
           << ", "
           << entry.get_score();
}

std::vector<std::shared_ptr<interfaces::Entry>> query_plugins(std::string p_query) {
    const ProviderRepository &repo = ProviderRepository::get_instance();

    interfaces::Query query {p_query};

    std::vector<std::shared_ptr<interfaces::Entry>> result {};
    for (const auto &provider : repo.get_active_providers()) {
        auto provider_result = provider->query(query);
        result.insert(result.cend(),
            std::make_move_iterator(provider_result.begin()),
            std::make_move_iterator(provider_result.end()));
    }

    std::ranges::sort(result,
        [](const auto &lhs, const auto &rhs) { return lhs->get_score() > rhs->get_score(); });

    return result;
}

int main(int argc, [[maybe_unused]] char **argv) {
    if (argc != 1) {
        std::cout << "Usage: launcher" << std::endl;
        return 1;
    }

    std::string query;
    std::cout << "Insert query: " << std::endl;
    std::cin >> query;

    auto results = query_plugins(std::move(query));

    std::cout << "Got results:\n";
    std::ranges::for_each(results, [](const auto &entry_ptr) { std::cout << *entry_ptr << '\n'; });

    std::cout << "Running highest" << std::endl;
    results.front()->execute();

    return 0;
}
