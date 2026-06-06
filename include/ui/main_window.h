#ifndef LAUNCHER_MAIN_WINDOW_H
#define LAUNCHER_MAIN_WINDOW_H

#include <gtkmm.h>

#include <ranges>

#include "config.h"
#include "model/module_interfaces.h"
#include "ui/list_item.h"

namespace launcher::ui {
    class MainWindow : public Gtk::Window {
        Glib::RefPtr<Gtk::Entry> m_entry;
        Glib::RefPtr<Gtk::ScrolledWindow> m_scroll;
        Glib::RefPtr<Gtk::ListBox> m_listbox;
        sigc::signal<void(const std::shared_ptr<interfaces::Entry> &)> m_signal_entry_selected {};
        sigc::signal<void(std::string_view)> m_signal_query_changed {};
        const options &m_options;

        void setup_controllers();
        bool on_key_pressed(guint keyval, guint, Gdk::ModifierType);

        void emit_entry_selected(Gtk::ListBoxRow *row);
        void emit_query_changed() const;

        void on_realize() override;

        bool entry_has_focus() const noexcept;

    public:
        MainWindow(GtkWindow *base_object, const Glib::RefPtr<Gtk::Builder> &builder, const options &options = options::get_instance());

        template<class It>
        void set_entries(It begin, It end) {
            m_listbox->remove_all();
            for (auto entry : std::ranges::subrange(begin, end)) {
                auto list_item = Gtk::make_managed<ListItem>(std::move(entry), m_options.get_icon_size());
                r_assert(list_item);
                m_listbox->append(*list_item);
            }
            if (auto first_row = m_listbox->get_row_at_index(0); first_row) {
                m_scroll->get_vadjustment()->set_value(0);
                m_listbox->select_row(*first_row);
            }
        }

        template<std::ranges::input_range Range>
        void set_entries(Range &&range) {
            set_entries(std::ranges::begin(range), std::ranges::end(range));
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