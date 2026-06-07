#ifndef LAUNCHER_LIST_ITEM_H
#define LAUNCHER_LIST_ITEM_H

#include <glibmm/extraclassinit.h>
#include <gtkmm.h>

#include "model/module_interfaces.h"

namespace launcher::ui {
    class ListItemExtraInit : public Glib::ExtraClassInit {
    public:
        ListItemExtraInit();
    };

    class ListItem : public ListItemExtraInit, public Gtk::ListBoxRow {
        std::shared_ptr<interfaces::Entry> m_entry;

        void reset();

    public:
        ListItem(std::shared_ptr<interfaces::Entry> entry, int icon_size);
        ~ListItem() override;

        static std::shared_ptr<ListItem> create(std::shared_ptr<interfaces::Entry> entry, int icon_size);

        void reset(std::shared_ptr<interfaces::Entry> entry);

        void set_title(std::string str);
        void set_subtitle(std::string str);

        void set_title(std::string_view str);
        void set_subtitle(std::string_view str);

        void set_icon(interfaces::IconVariant str);

        std::shared_ptr<interfaces::Entry> get_entry() const { return m_entry; }
    };
} // namespace launcher::ui

#endif