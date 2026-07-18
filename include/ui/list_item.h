#ifndef LAUNCHER_LIST_ITEM_H
#define LAUNCHER_LIST_ITEM_H

#include <gtkmm.h>

#include "model/module_interfaces.h"

namespace launcher::ui {
    class action_list_item : public Gtk::ListBoxRow {
    protected:
        std::shared_ptr<interfaces::action> m_action;

        Gtk::Label m_title;
        Gtk::Image m_icon;

        void build_layout(int icon_size);

        action_list_item(const char *type_name, std::shared_ptr<interfaces::action> action);

    public:
        action_list_item(std::shared_ptr<interfaces::action> action, int icon_size);
        ~action_list_item() override;

        void set_title(std::string str);
        void set_title(std::string_view str);

        void set_icon(interfaces::icon_variant str);

        std::shared_ptr<interfaces::action> get_action() const noexcept { return m_action; }
    };

    class entry_list_item : public action_list_item {
        Gtk::Label m_subtitle;

        Gtk::TreeExpander m_tree_expander;

        void build_layout(int icon_size);

    public:
        entry_list_item(std::shared_ptr<interfaces::entry> entry, int icon_size,
            const Glib::RefPtr<Gtk::TreeListRow> &tree_list_row);
        ~entry_list_item() override;

        void set_subtitle(std::string str);
        void set_subtitle(std::string_view str);

        void expand(bool value = true);
        void colapse();

        std::shared_ptr<interfaces::entry> get_entry() const noexcept {
            return std::static_pointer_cast<interfaces::entry>(action_list_item::m_action);
        }
    };
} // namespace launcher::ui

#endif