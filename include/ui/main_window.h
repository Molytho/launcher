#ifndef LAUNCHER_MAIN_WINDOW_H
#define LAUNCHER_MAIN_WINDOW_H

#include <gtkmm.h>

#include "model/module_interfaces.h"

namespace launcher::ui {
    namespace list_model {
        class action : public Glib::Object {
            std::shared_ptr<interfaces::action> m_action;

            action(std::shared_ptr<interfaces::action> e);

        public:
            std::shared_ptr<interfaces::action> get_action() const noexcept { return m_action; }

            using Glib::Object::get_base_type;
            using Glib::Object::get_type;

            static Glib::RefPtr<action> create(std::shared_ptr<interfaces::action> e);
        };

        class entry : public Gio::ListModel, public Glib::Object {
            std::shared_ptr<interfaces::entry> m_entry;

            entry(std::shared_ptr<interfaces::entry> e);

        public:
            std::shared_ptr<interfaces::entry> get_entry() { return m_entry; }

            using Glib::Object::get_base_type;
            using Glib::Object::get_type;

            static Glib::RefPtr<entry> create(std::shared_ptr<interfaces::entry> e);

            GType get_item_type_vfunc() override;
            guint get_n_items_vfunc() override;
            gpointer get_item_vfunc(guint position) override;
        };

    } // namespace list_model

    class main_window : public Gtk::Window {
        Glib::RefPtr<Gtk::Entry> m_entry;
        Glib::RefPtr<Gtk::Viewport> m_listbox_viewport;
        Glib::RefPtr<Gtk::ListBox> m_listbox;
        Glib::RefPtr<Gio::ListStore<list_model::entry>> m_model;
        Glib::RefPtr<Gtk::TreeListModel> m_tree_model;

        sigc::signal<void(std::shared_ptr<interfaces::action>)> m_signal_entry_selected {};
        sigc::signal<void(std::string_view)> m_signal_query_changed {};

        void setup_controllers();
        void move_entry_focus(Gtk::DirectionType direction);
        void move_entry_focus(size_t index);
        void expand_focused_entry(bool expand = true);
        bool on_key_pressed(guint keyval, guint, Gdk::ModifierType);

        void emit_entry_selected(Gtk::ListBoxRow *row);
        void emit_query_changed() const;

        void on_realize() override;

    public:
        main_window(GtkWindow *base_object, const Glib::RefPtr<Gtk::Builder> &builder);

        void set_entries(std::vector<std::shared_ptr<interfaces::entry>> entries);

        sigc::signal<void(std::shared_ptr<interfaces::action>)> signal_entry_selected() const noexcept;
        sigc::signal<void(std::string_view)> signal_query_changed() const noexcept;
    };
} // namespace launcher::ui

#endif