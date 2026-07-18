#include "ui/main_window.h"

#include <ranges>

#include "config.h"
#include "macros.h"
#include "ui/list_item.h"
#include "utils/ranges_helper.h"

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

using namespace launcher;
using namespace launcher::ui;

namespace {
    void platform_setup(main_window &window) {
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

        if (auto model_entry = std::dynamic_pointer_cast<list_model::entry>(item); model_entry) {
            return Gtk::make_managed<entry_list_item>(model_entry->get_entry(),
                options::get_instance().get_icon_size(),
                row);
        } else if (auto model_action = std::dynamic_pointer_cast<list_model::action>(item); model_action) {
            return Gtk::make_managed<action_list_item>(model_action->get_action(),
                options::get_instance().get_action_icon_size());
        } else {
            std::abort();
        }
    }

    Glib::RefPtr<Gio::ListModel> create_model(const Glib::RefPtr<Glib::ObjectBase> &obj) {
        return std::dynamic_pointer_cast<Gio::ListModel>(obj);
    }
} // namespace

namespace launcher::ui {
    namespace list_model {
        action::action(std::shared_ptr<interfaces::action> e) :
                ObjectBase("ListModelAction"), m_action(std::move(e)) {}

        Glib::RefPtr<action> action::create(std::shared_ptr<interfaces::action> e) {
            return Glib::make_refptr_for_instance(new action(std::move(e)));
        }

        entry::entry(std::shared_ptr<interfaces::entry> e) :
                ObjectBase("ListModelEntry"), m_entry(std::move(e)) {}

        Glib::RefPtr<entry> entry::create(std::shared_ptr<interfaces::entry> e) {
            return Glib::make_refptr_for_instance(new entry(std::move(e)));
        }

        GType entry::get_item_type_vfunc() {
            return action::get_type();
        }

        guint entry::get_n_items_vfunc() {
            return m_entry->get_actions().size();
        }

        gpointer entry::get_item_vfunc(guint position) {
            auto action        = m_entry->get_actions().at(position);
            auto action_object = action::create(action);
            return action_object->gobj_copy();
        }
    } // namespace list_model

    void main_window::setup_controllers() {
        add_controller([&]() {
            auto key_controller = Gtk::EventControllerKey::create();
            key_controller->set_propagation_phase(Gtk::PropagationPhase::CAPTURE);
            key_controller->signal_key_pressed().connect(sigc::mem_fun(*this, &main_window::on_key_pressed), false);
            return key_controller;
        }());
    }

    void main_window::on_realize() {
        Gtk::Window::on_realize();
        platform_setup(*this);
    }

    void main_window::move_entry_focus(Gtk::DirectionType direction) {
        auto index = [&]() {
            auto selected_row = m_listbox->get_selected_row();
            r_assert(selected_row);
            return selected_row->get_index();
        }();

        switch (direction) {
        case Gtk::DirectionType::UP:
            index--;
            break;
        case Gtk::DirectionType::DOWN:
            index++;
            break;
        default:
            std::abort();
        }

        move_entry_focus(index);
    }

    void main_window::move_entry_focus(size_t index) {
        if (auto row = m_listbox->get_row_at_index(index); row) {
            m_listbox->select_row(*row);
            m_listbox_viewport->scroll_to(*row);
        }
    }

    void main_window::expand_focused_entry(bool expand) {
        if (auto entry_row = dynamic_cast<entry_list_item *>(m_listbox->get_selected_row()); entry_row) {
            entry_row->expand(expand);
        }
    }

    bool main_window::on_key_pressed(guint keyval, guint, Gdk::ModifierType) {
        switch (keyval) {
        case GDK_KEY_Escape:
            close();
            return true;
        case GDK_KEY_Down:
            move_entry_focus(Gtk::DirectionType::DOWN);
            return true;
        case GDK_KEY_Up:
            move_entry_focus(Gtk::DirectionType::UP);
            return true;
        case GDK_KEY_Right:
            expand_focused_entry();
            return true;
        case GDK_KEY_Left:
            expand_focused_entry(false);
            return true;
        case GDK_KEY_Return:
            if (auto row = m_listbox->get_selected_row(); row) {
                row->activate();
                return true;
            }
        }

        return false;
    }

    void main_window::emit_entry_selected(Gtk::ListBoxRow *row) {
        auto list_item = dynamic_cast<action_list_item *>(row);
        r_assert(list_item);
        m_signal_entry_selected.emit(list_item->get_action());
        close();
    }

    void main_window::emit_query_changed() const {
        // TODO: Implement delay
        std::string_view str = gtk_editable_get_text(GTK_EDITABLE(m_entry->gobj()));
        m_signal_query_changed.emit(str);
    }

    main_window::main_window(GtkWindow *base_object, const Glib::RefPtr<Gtk::Builder> &builder) :
            Gtk::Window(base_object), m_entry(builder->get_widget<Gtk::Entry>("search")),
            m_listbox_viewport(builder->get_widget<Gtk::Viewport>("app-list-viewport")),
            m_listbox(builder->get_widget<Gtk::ListBox>("app-list")),
            m_model(Gio::ListStore<list_model::entry>::create()),
            m_tree_model(Gtk::TreeListModel::create(m_model, sigc::ptr_fun(create_model), false, false)) {
        m_entry->property_text().signal_changed().connect(sigc::mem_fun(*this, &main_window::emit_query_changed));
        m_listbox->signal_row_activated().connect(sigc::mem_fun(*this, &main_window::emit_entry_selected));
        setup_controllers();

        m_listbox->bind_model(m_tree_model, sigc::ptr_fun(create_widget));
    }

    void main_window::set_entries(std::vector<std::shared_ptr<interfaces::entry>> entries) {
        auto additions = std::views::transform(entries, [](auto &entry) {
            return list_model::entry::create(std::move(entry));
        }) | view_helper::to_vector;
        m_model->splice(0, m_model->get_n_items(), std::move(additions));
        move_entry_focus(0);
    }

    sigc::signal<void(std::shared_ptr<interfaces::action>)> main_window::signal_entry_selected() const noexcept {
        return m_signal_entry_selected;
    }

    sigc::signal<void(std::string_view)> main_window::signal_query_changed() const noexcept {
        return m_signal_query_changed;
    }
} // namespace launcher::ui