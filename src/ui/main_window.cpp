#include "ui/main_window.h"

#include "ui/list_item.h"

#include "macros.h"

namespace launcher::ui {
    MainWindow::MainWindow(
        GtkWindow *base_object, const Glib::RefPtr<Gtk::Builder> &builder, const options &options) :
            Gtk::Window(base_object), m_entry(builder->get_widget<Gtk::Entry>("search")),
            m_scroll(builder->get_widget<Gtk::ScrolledWindow>("scroll")),
            m_listbox(builder->get_widget<Gtk::ListBox>("app-list")), m_options(options) {
        m_entry->property_text().signal_changed().connect(sigc::mem_fun(*this, &MainWindow::emit_query_changed));
        m_listbox->signal_row_activated().connect(sigc::mem_fun(*this, &MainWindow::emit_entry_selected));
        setup_controllers();
    }

    void MainWindow::setup_controllers() {
        add_controller([&]() {
            auto key_controller = Gtk::EventControllerKey::create();
            key_controller->set_propagation_phase(Gtk::PropagationPhase::CAPTURE);
            key_controller->signal_key_pressed().connect(sigc::mem_fun(*this, &MainWindow::on_key_pressed), false);
            return key_controller;
        }());
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
        auto list_item = dynamic_cast<ListItem *>(row);
        r_assert(list_item);
        m_signal_entry_selected.emit(list_item->get_entry());
        close();
    }

    void MainWindow::emit_query_changed() const {
        // TODO: Implement delay
        std::string_view str = gtk_editable_get_text(GTK_EDITABLE(m_entry->gobj()));
        m_signal_query_changed.emit(str);
    }

    void MainWindow::set_entries(std::span<std::shared_ptr<interfaces::Entry>> entries) {
        m_listbox->remove_all();
        // TODO: Using a view would allow this to move items
        for (const auto &entry : entries) {
            auto list_item = Gtk::make_managed<ListItem>(entry, m_options.get_icon_size());
            r_assert(list_item);
            m_listbox->append(*list_item);
        }
        if (auto first_row = m_listbox->get_row_at_index(0); first_row) {
            m_scroll->get_vadjustment()->set_value(0);
            m_listbox->select_row(*first_row);
        }
    }

} // namespace launcher::ui