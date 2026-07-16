#ifndef LAUNCHER_LIST_ITEM_H
#define LAUNCHER_LIST_ITEM_H

#include <gtkmm.h>

#include "model/module_interfaces.h"

namespace launcher::ui {
    class ActionListItem : public Gtk::ListBoxRow {
    protected:
        std::shared_ptr<interfaces::Action> m_action;

        Gtk::Label m_title;
        Gtk::Image m_icon;

        void build_layout(int icon_size);

        ActionListItem(const char *type_name, std::shared_ptr<interfaces::Action> action);

    public:
        ActionListItem(std::shared_ptr<interfaces::Action> action, int icon_size);
        ~ActionListItem() override;

        void set_title(std::string str);
        void set_title(std::string_view str);

        void set_icon(interfaces::IconVariant str);

        std::shared_ptr<interfaces::Action> get_action() const noexcept { return m_action; }
    };

    class EntryListItem : public ActionListItem {
        Gtk::Label m_subtitle;

        Gtk::TreeExpander m_tree_expander;

        void build_layout(int icon_size);

    public:
        EntryListItem(std::shared_ptr<interfaces::Entry> entry, int icon_size,
            const Glib::RefPtr<Gtk::TreeListRow> &tree_list_row);
        ~EntryListItem() override;

        void set_subtitle(std::string str);
        void set_subtitle(std::string_view str);

        std::shared_ptr<interfaces::Entry> get_entry() const noexcept {
            return std::static_pointer_cast<interfaces::Entry>(ActionListItem::m_action);
        }
    };
} // namespace launcher::ui

#endif