#include <algorithm>
#include <iostream>

#include <gtkmm.h>

#include "config.h"
#include "history.h"
#include "macros.h"
#include "provider_repository.h"
#include "ui/main_window.h"

using namespace launcher;

std::vector<std::pair<char, interfaces::Query>> make_queries(std::string_view query) {
    if (query.empty()) {
        return {
            {0, {query}}
        };
    } else {
        return {
            {0,           {query}          },
            {query.at(0), {query.substr(1)}}
        };
    }
}

std::vector<std::shared_ptr<interfaces::Entry>> query_plugins(
    std::string query, const history_provider &history_provider = history_provider::get_instance()) {
    const provider_repository &repo = provider_repository::get_instance();

    std::vector<std::shared_ptr<interfaces::Entry>> result {};
    for (const auto &[activation_char, query] : make_queries(query)) {
        for (const auto &provider : repo.get_active_providers(activation_char)) {
            auto provider_result = provider->query(query);
            result.insert(result.cend(),
                std::move_iterator(provider_result.begin()),
                std::move_iterator(provider_result.end()));
        }
    }

    history_provider.boost_history_entries(result);

    std::ranges::stable_sort(result,
        [](const auto &lhs, const auto &rhs) { return lhs->get_score() > rhs->get_score(); });

    return result;
}

std::vector<std::shared_ptr<interfaces::Entry>> query_plugins(std::string_view p_query,
    const history_provider &history_provider = history_provider::get_instance()) {
    return query_plugins(std::string(p_query), history_provider);
}

interfaces::execute_context make_execute_context() {
    interfaces::execute_context res {};
    {
        auto app_launch_context = Gdk::Display::get_default()->get_app_launch_context();
        auto token              = app_launch_context->get_startup_notify_id(nullptr, {});
        if (!token.empty()) {
            res.set_startup_notify_token(std::move(token));
        }
    }
    return res;
}

void setup_css_providers() {
    {
        auto css_provider = Gtk::CssProvider::create();
        css_provider->load_from_resource("/Launcher/style.css");
        Gtk::StyleContext::add_provider_for_display(Gdk::Display::get_default(), css_provider, GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    }
    {
        auto path = get_config_dir().append("style.css");
        if (std::filesystem::is_regular_file(path)) {
            auto css_provider = Gtk::CssProvider::create();
            css_provider->load_from_path(path);
            Gtk::StyleContext::add_provider_for_display(Gdk::Display::get_default(), css_provider, GTK_STYLE_PROVIDER_PRIORITY_USER);
        }
    }
}

int main(int argc, [[maybe_unused]] char **argv) {
    if (argc != 1) {
        std::cout << "Usage: launcher" << std::endl;
        return 1;
    }

    options::init(argc, argv);
    provider_repository::init();
    history_provider::init();

    auto app = Gtk::Application::create(PROJECT_NAME);
    app->signal_activate().connect([&]() {
        setup_css_providers();

        auto builder = Gtk::Builder::create_from_resource("/Launcher/Launcher.ui");
        r_assert(builder);
        auto window = Gtk::Builder::get_widget_derived<ui::MainWindow>(builder, "launcher");
        r_assert(window);
        window->signal_entry_selected().connect([](auto entry_ptr) {
            r_assert(entry_ptr);
            auto context = make_execute_context();
            if (auto entry = dynamic_cast<interfaces::Entry *>(entry_ptr.get()); entry) {
                history_provider::get_instance().add_to_history(*entry);
            }
            entry_ptr->execute(context);
        });
        window->signal_query_changed().connect([window](std::string_view str) {
            auto results = query_plugins(str);
            window->set_entries(std::move(results));
        });
        {
            auto results = query_plugins(std::string(""));
            window->set_entries(std::move(results));
        }
        app->signal_window_removed().connect([](Gtk::Window *window) { delete window; });

        const auto &options = options::get_instance();
        window->set_default_size(options.get_width(), options.get_height());

        app->add_window(*window);
        window->set_visible();
    });

    return app->run(argc, argv);
}
