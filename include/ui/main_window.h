#ifndef LAUNCHER_MAIN_WINDOW_H
#define LAUNCHER_MAIN_WINDOW_H

#include <gtkmm.h>

#include <ranges>

#include "model/module_interfaces.h"

namespace launcher::ui {
    class ListModelAction : public Glib::Object {
        std::shared_ptr<interfaces::Action> m_action;

        ListModelAction(std::shared_ptr<interfaces::Action> e);

    public:
        std::shared_ptr<interfaces::Action> get_action() const noexcept {
            return m_action;
        }
        
        using Glib::Object::get_base_type;
        using Glib::Object::get_type;

        static Glib::RefPtr<ListModelAction> create(std::shared_ptr<interfaces::Action> e);
    };

    class ListModelEntry : public Gio::ListModel, public Glib::Object {
        std::shared_ptr<interfaces::Entry> m_entry;

        ListModelEntry(std::shared_ptr<interfaces::Entry> e);

    public:
        std::shared_ptr<interfaces::Entry> get_entry() { return m_entry; }

        using Glib::Object::get_base_type;
        using Glib::Object::get_type;

        static Glib::RefPtr<ListModelEntry> create(std::shared_ptr<interfaces::Entry> e);

        GType get_item_type_vfunc() override;
        guint get_n_items_vfunc() override;
        gpointer get_item_vfunc(guint position) override;
    };

    class MainWindow : public Gtk::Window {
        Glib::RefPtr<Gtk::Entry> m_entry;
        Glib::RefPtr<Gtk::ScrolledWindow> m_scroll;
        Glib::RefPtr<Gtk::ListBox> m_listbox;
        Glib::RefPtr<Gio::ListStore<ListModelEntry>> m_model;
        Glib::RefPtr<Gtk::TreeListModel> m_tree_model;
        sigc::signal<void(std::shared_ptr<interfaces::Action>)> m_signal_entry_selected {};
        sigc::signal<void(std::string_view)> m_signal_query_changed {};

        void setup_controllers();
        bool on_key_pressed(guint keyval, guint, Gdk::ModifierType);

        void emit_entry_selected(Gtk::ListBoxRow *row);
        void emit_query_changed() const;

        void on_realize() override;

        bool entry_has_focus() const noexcept;

    public:
        MainWindow(GtkWindow *base_object, const Glib::RefPtr<Gtk::Builder> &builder);

        template<std::ranges::input_range Range>
        void set_entries(Range &&range) {
            std::vector<Glib::RefPtr<ListModelEntry>> additions = [&]() {
                auto view = std::views::transform(range,
                    [](auto &entry) { return ListModelEntry::create(std::move(entry)); });
                return std::vector<Glib::RefPtr<ListModelEntry>> {std::move_iterator(view.begin()),
                    std::move_iterator(view.end())};
            }();
            m_model->splice(0, m_model->get_n_items(), additions);
            if (auto first_row = m_listbox->get_row_at_index(0); first_row) {
                m_scroll->get_vadjustment()->set_value(0);
                m_listbox->select_row(*first_row);
            }
        }

        sigc::signal<void(std::shared_ptr<interfaces::Action>)> signal_entry_selected() const noexcept {
            return m_signal_entry_selected;
        }

        sigc::signal<void(std::string_view)> signal_query_changed() const noexcept {
            return m_signal_query_changed;
        }
    };
} // namespace launcher::ui

#endif