#ifndef LAUNCHER_MAIN_WINDOW_H
#define LAUNCHER_MAIN_WINDOW_H

#include <gtkmm.h>

#include <span>

#include "model/module_interfaces.h"

namespace launcher::ui {
    class MainWindow : public Gtk::Window {
        Glib::RefPtr<Gtk::Entry> m_entry;
        Glib::RefPtr<Gtk::ScrolledWindow> m_scroll;
        Glib::RefPtr<Gtk::ListBox> m_listbox;
        sigc::signal<void(const std::shared_ptr<interfaces::Entry> &)> m_signal_entry_selected {};
        sigc::signal<void(std::string_view)> m_signal_query_changed {};

        void setup_controllers();
        bool on_key_pressed(guint keyval, guint, Gdk::ModifierType);

        void emit_entry_selected(Gtk::ListBoxRow *row);
        void emit_query_changed() const;

    public:
        MainWindow(GtkWindow *base_object, const Glib::RefPtr<Gtk::Builder> &builder);

        // TODO: Technically this should be a view
        void set_entries(std::span<std::shared_ptr<interfaces::Entry>> entries);

        sigc::signal<void(const std::shared_ptr<interfaces::Entry> &)> signal_entry_selected() const noexcept {
            return m_signal_entry_selected;
        }

        sigc::signal<void(std::string_view)> signal_query_changed() const noexcept {
            return m_signal_query_changed;
        }
    };
} // namespace launcher::ui

#endif