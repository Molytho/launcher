#include "ui/list_item.h"

#include "macros.h"

namespace {
    extern "C" {
        static void class_dispose(GObject *gobject) {
            auto klass = G_TYPE_INSTANCE_GET_CLASS(gobject, GTK_TYPE_WIDGET, GtkWidgetClass);

            gtk_widget_dispose_template(GTK_WIDGET(gobject), G_TYPE_FROM_CLASS(klass));

            auto base_class = G_OBJECT_CLASS(g_type_class_peek_parent(klass));
            r_assert(base_class);
            base_class->dispose(gobject);
        }

        static void class_init_function(void *g_class, void *) {
            G_OBJECT_CLASS(g_class)->dispose = class_dispose;

            const auto klass = GTK_WIDGET_CLASS(g_class);
            gtk_widget_class_set_template_from_resource(klass, "/Launcher/ListBoxItem.ui");
            gtk_widget_class_bind_template_child_full(klass, "Icon", true, 0);
            gtk_widget_class_bind_template_child_full(klass, "Title", true, 0);
            gtk_widget_class_bind_template_child_full(klass, "Subtitle", true, 0);
        }

        static void class_instances_init_function(GTypeInstance *instance, void *) {
            gtk_widget_init_template(GTK_WIDGET(instance));
        }
    }
} // namespace

namespace launcher::ui {
    ListItemExtraInit::ListItemExtraInit() :
            Glib::ExtraClassInit(class_init_function, nullptr, class_instances_init_function) {}

    ListItem::ListItem(std::shared_ptr<interfaces::Entry> entry) :
            Glib::ObjectBase("MyListBoxItem"), ListItemExtraInit(), Gtk::ListBoxRow(),
            m_entry(std::move(entry)) {
        r_assert(m_entry);
        set_title(m_entry->get_title());
        set_subtitle(m_entry->get_subtitle());
        set_icon(m_entry->get_icon());
    }

    ListItem::~ListItem() = default;

    void ListItem::set_title(std::string str) {
        auto label = GTK_LABEL(gtk_widget_get_template_child(GTK_WIDGET(gobj()),
            G_TYPE_FROM_INSTANCE(gobj()),
            "Title"));
        r_assert(label);
        gtk_label_set_label(label, str.c_str());
    }

    void ListItem::set_subtitle(std::string str) {
        auto label = GTK_LABEL(gtk_widget_get_template_child(GTK_WIDGET(gobj()),
            G_TYPE_FROM_INSTANCE(gobj()),
            "Subtitle"));
        r_assert(label);
        gtk_label_set_label(label, str.c_str());
    }

    void ListItem::set_icon(std::string str) {
        auto image = GTK_IMAGE(
            gtk_widget_get_template_child(GTK_WIDGET(gobj()), G_TYPE_FROM_INSTANCE(gobj()), "Icon"));
        r_assert(image);
        auto icon = Gio::Icon::create(str);
        gtk_image_set_from_gicon(image, icon->gobj());
        gtk_image_set_pixel_size(image, 64);
    }

    void ListItem::set_title(std::string_view str) {
        return set_title(std::string(str));
    }

    void ListItem::set_subtitle(std::string_view str) {
        return set_subtitle(std::string(str));
    }

    void ListItem::set_icon(std::string_view str) {
        return set_icon(std::string(str));
    }
} // namespace launcher::ui