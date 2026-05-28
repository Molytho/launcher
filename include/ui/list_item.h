#ifndef LAUNCHER_LIST_ITEM_H
#define LAUNCHER_LIST_ITEM_H

#include <glibmm/extraclassinit.h>
#include <gtkmm.h>

namespace launcher::ui {
    class ListItemExtraInit : public Glib::ExtraClassInit {
    public:
        ListItemExtraInit();
    };

    class ListItem : public ListItemExtraInit, public Gtk::ListBoxRow {
    public:
        ListItem();
        ~ListItem() override;
    };
} // namespace launcher::ui

#endif