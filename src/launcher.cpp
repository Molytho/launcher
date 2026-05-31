#include <algorithm>
#include <iostream>

#include <gtkmm.h>

#include "macros.h"
#include "provider_repository.h"
#include "ui/list_item.h"
#include "ui/main_window.h"

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

template<class T>
std::ostream &operator<<(std::ostream &os, const std::shared_ptr<T> &ptr) {
    if (ptr) {
        return os << *ptr;
    } else {
        return os << "nullptr";
    }
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

std::vector<std::shared_ptr<interfaces::Entry>> query_plugins(std::string_view p_query) {
    return query_plugins(std::string(p_query));
}

void setup_css_providers() {
    {
        auto css_provider = Gtk::CssProvider::create();
        css_provider->load_from_resource("/Launcher/style.css");
        Gtk::StyleContext::add_provider_for_display(Gdk::Display::get_default(), css_provider, GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    }
    /*{
        auto css_provider = Gtk::CssProvider::create();
        css_provider->load_from_path("/home/robin/.config/sirula/style.css");
        Gtk::StyleContext::add_provider_for_display(Gdk::Display::get_default(), css_provider, GTK_STYLE_PROVIDER_PRIORITY_USER);
    }*/
}

int main(int argc, [[maybe_unused]] char **argv) {
    if (argc != 1) {
        std::cout << "Usage: launcher" << std::endl;
        return 1;
    }

    auto app = Gtk::Application::create("de.molytho.launcher");
    app->signal_activate().connect([&]() {
        setup_css_providers();

        auto builder = Gtk::Builder::create_from_resource("/Launcher/Launcher.ui");
        auto window  = Gtk::Builder::get_widget_derived<ui::MainWindow>(builder, "launcher");
        r_assert(window);
        window->signal_entry_selected().connect([](auto entry_ptr) {
            entry_ptr->execute();
        });
        window->signal_query_changed().connect([window](std::string_view str) {
            auto results = query_plugins(str);
            window->set_entries(results);
        });
        {
            auto results = query_plugins(std::string(""));
            window->set_entries(results);
        }

        app->signal_window_removed().connect([](Gtk::Window *window) { delete window; });
        app->add_window(*window);
        window->set_visible();
    });

    return app->run(argc, argv);
}
