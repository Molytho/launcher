#ifndef LAUNCHER_MAIN_WINDOW_H
#define LAUNCHER_MAIN_WINDOW_H

#include <gtkmm.h>

#include <ranges>

#include "model/module_interfaces.h"
#include "ui/list_item.h"

namespace launcher::ui {
    class ListModelEntry : public Glib::Object {
        std::shared_ptr<interfaces::Entry> m_entry;

        ListModelEntry(std::shared_ptr<interfaces::Entry> e);

    public:
        std::shared_ptr<interfaces::Entry> get_entry() { return m_entry; }

        using Glib::Object::get_base_type;
        using Glib::Object::get_type;

        static Glib::RefPtr<ListModelEntry> create(std::shared_ptr<interfaces::Entry> e);
    };

    class MainWindow : public Gtk::Window {
        Glib::RefPtr<Gtk::Entry> m_entry;
        Glib::RefPtr<Gtk::ScrolledWindow> m_scroll;
        Glib::RefPtr<Gtk::ListBox> m_listbox;
        Glib::RefPtr<Gio::ListStore<ListModelEntry>> m_model;
        sigc::signal<void(const std::shared_ptr<interfaces::Entry> &)> m_signal_entry_selected {};
        sigc::signal<void(std::string_view)> m_signal_query_changed {};

        std::vector<std::shared_ptr<ListItem>> m_list_item_cache {};
        size_t m_cache_pos {0};

        void setup_controllers();
        bool on_key_pressed(guint keyval, guint, Gdk::ModifierType);

        void emit_entry_selected(Gtk::ListBoxRow *row);
        void emit_query_changed() const;

        void on_realize() override;

        bool entry_has_focus() const noexcept;

        Gtk::Widget *create_widget(const Glib::RefPtr<Glib::Object> &obj);

    public:
        MainWindow(GtkWindow *base_object, const Glib::RefPtr<Gtk::Builder> &builder);

        template<std::ranges::input_range Range>
        void set_entries(Range &&range) {
            m_cache_pos = 0;
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

        sigc::signal<void(const std::shared_ptr<interfaces::Entry> &)> signal_entry_selected() const noexcept {
            return m_signal_entry_selected;
        }

        sigc::signal<void(std::string_view)> signal_query_changed() const noexcept {
            return m_signal_query_changed;
        }
    };
} // namespace launcher::ui

#endif