#include "ui/main_window.h"

#include "config.h"
#include "macros.h"
#include "ui/list_item.h"

#ifdef GDK_WINDOWING_WAYLAND
# include <gdk/wayland/gdkwayland.h>

void platform_setup_wayland(Gtk::Window &window);

bool is_wayland_display(Gdk::Display *display) {
    return GDK_IS_WAYLAND_DISPLAY(display->gobj());
}
#else
[[maybe_unused]] void platform_setup_wayland(Gtk::Window &) {}

bool is_wayland_display(Gdk::Display *) {
    return false;
}
#endif

#ifdef GDK_WINDOWING_X11
# include <gdk/x11/gdkx.h>

void platform_setup_x11(Gtk::Window &window);

bool is_x11_display(Gdk::Display *display) {
    return GDK_IS_X11_DISPLAY(display->gobj());
}
#else
[[maybe_unused]] void platform_setup_x11(Gtk::Window &) {}

bool is_x11_display(Gdk::Display *) {
    return false;
}
#endif

namespace {
    void platform_setup(launcher::ui::MainWindow &window) {
        if (getenv("LAUNCHER_NO_PLATFORM_SETUP") != nullptr) {
            return;
        }
        if (is_wayland_display(window.get_display().get())) {
            platform_setup_wayland(window);
        } else if (is_x11_display(window.get_display().get())) {
            platform_setup_x11(window);
        } else {
            std::cerr << "Invalid gdk platform\n";
            exit(254);
        }
    }

    Gtk::Widget *create_widget(const Glib::RefPtr<Glib::Object> &obj) {
        r_assert(obj);
        auto row  = std::dynamic_pointer_cast<Gtk::TreeListRow>(obj);
        auto item = row->get_item();

        if (auto model_entry = std::dynamic_pointer_cast<launcher::ui::ListModelEntry>(item); model_entry) {
            return Gtk::make_managed<launcher::ui::EntryListItem>(model_entry->get_entry(),
                launcher::options::get_instance().get_icon_size(),
                row);
        } else if (auto model_action = std::dynamic_pointer_cast<launcher::ui::ListModelAction>(item); model_action) {
            return Gtk::make_managed<launcher::ui::ActionListItem>(model_action->get_action(),
                launcher::options::get_instance().get_action_icon_size());
        } else {
            std::abort();
        }
    }

    Glib::RefPtr<Gio::ListModel> create_model(const Glib::RefPtr<Glib::ObjectBase> &obj) {
        return std::dynamic_pointer_cast<Gio::ListModel>(obj);
    }
} // namespace

namespace launcher::ui {
    ListModelAction::ListModelAction(std::shared_ptr<interfaces::Action> e) :
            ObjectBase("ListModelAction"), m_action(std::move(e)) {}

    Glib::RefPtr<ListModelAction> ListModelAction::create(std::shared_ptr<interfaces::Action> e) {
        return Glib::make_refptr_for_instance(new ListModelAction(std::move(e)));
    }

    ListModelEntry::ListModelEntry(std::shared_ptr<interfaces::Entry> e) :
            ObjectBase("ListModelEntry"), m_entry(std::move(e)) {}

    Glib::RefPtr<ListModelEntry> ListModelEntry::create(std::shared_ptr<interfaces::Entry> e) {
        return Glib::make_refptr_for_instance(new ListModelEntry(std::move(e)));
    }

    GType ListModelEntry::get_item_type_vfunc() {
        return ListModelAction::get_type();
    }

    guint ListModelEntry::get_n_items_vfunc() {
        return m_entry->get_actions().size();
    }

    gpointer ListModelEntry::get_item_vfunc(guint position) {
        auto action        = m_entry->get_actions().at(position);
        auto action_object = ListModelAction::create(action);
        return action_object->gobj_copy();
    }

    MainWindow::MainWindow(GtkWindow *base_object, const Glib::RefPtr<Gtk::Builder> &builder) :
            Gtk::Window(base_object), m_entry(builder->get_widget<Gtk::Entry>("search")),
            m_scroll(builder->get_widget<Gtk::ScrolledWindow>("scroll")),
            m_listbox(builder->get_widget<Gtk::ListBox>("app-list")),
            m_model(Gio::ListStore<ListModelEntry>::create()),
            m_tree_model(Gtk::TreeListModel::create(m_model, sigc::ptr_fun(create_model), false, false)) {
        m_entry->property_text().signal_changed().connect(sigc::mem_fun(*this, &MainWindow::emit_query_changed));
        m_listbox->signal_row_activated().connect(sigc::mem_fun(*this, &MainWindow::emit_entry_selected));
        setup_controllers();

        m_listbox->bind_model(m_tree_model, sigc::ptr_fun(create_widget));
    }

    void MainWindow::setup_controllers() {
        add_controller([&]() {
            auto key_controller = Gtk::EventControllerKey::create();
            key_controller->set_propagation_phase(Gtk::PropagationPhase::CAPTURE);
            key_controller->signal_key_pressed().connect(sigc::mem_fun(*this, &MainWindow::on_key_pressed), false);
            return key_controller;
        }());
    }

    void MainWindow::on_realize() {
        Gtk::Window::on_realize();
        platform_setup(*this);
    }

    bool MainWindow::entry_has_focus() const noexcept {
        // GtkEntry's implementation is broken as fuck...
        auto focus = get_focus();
        if (!focus) {
            return false;
        }
        auto parent = focus->get_parent();
        if (!parent) {
            return false;
        }
        return parent == m_entry.get();
    }

    bool MainWindow::on_key_pressed(guint keyval, guint, Gdk::ModifierType) {
        switch (keyval) {
        case GDK_KEY_Escape:
            close();
            return true;
        case GDK_KEY_Down: {
            if (entry_has_focus()) {
                if (auto row = m_listbox->get_row_at_index(1); row) {
                    m_listbox->select_row(*row);
                    row->grab_focus();
                    return true;
                }
            }
            break;
        }
        case GDK_KEY_Up:
            if (auto row = m_listbox->get_row_at_index(0); row && row->is_focus()) {
                m_entry->grab_focus_without_selecting();
                return true;
            }
            break;
        case GDK_KEY_Return: {
            if (auto row = m_listbox->get_selected_row(); row) {
                row->activate();
                return true;
            }
        }
        }

        return false;
    }

    void MainWindow::emit_entry_selected(Gtk::ListBoxRow *row) {
        auto list_item = dynamic_cast<ActionListItem *>(row);
        r_assert(list_item);
        m_signal_entry_selected.emit(list_item->get_action());
        close();
    }

    void MainWindow::emit_query_changed() const {
        // TODO: Implement delay
        std::string_view str = gtk_editable_get_text(GTK_EDITABLE(m_entry->gobj()));
        m_signal_query_changed.emit(str);
    }
} // namespace launcher::ui