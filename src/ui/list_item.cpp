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

    ListItem::ListItem(std::shared_ptr<interfaces::Entry> entry, int icon_size) :
            Glib::ObjectBase("MyListBoxItem"), ListItemExtraInit(), Gtk::ListBoxRow(),
            m_entry(std::move(entry)) {
        r_assert(m_entry);

        auto image = GTK_IMAGE(
            gtk_widget_get_template_child(GTK_WIDGET(gobj()), G_TYPE_FROM_INSTANCE(gobj()), "Icon"));
        r_assert(image);
        gtk_image_set_pixel_size(image, icon_size);

        reset();
    }

    ListItem::~ListItem() = default;

    std::shared_ptr<ListItem> ListItem::create(std::shared_ptr<interfaces::Entry> entry, int icon_size) {
        return std::make_shared<ListItem>(std::move(entry), icon_size);
    }

    void ListItem::reset(std::shared_ptr<interfaces::Entry> entry) {
        m_entry = std::move(entry);
        reset();
    }

    void ListItem::reset() {
        set_title(m_entry->get_title());
        set_subtitle(m_entry->get_subtitle());
        set_icon(m_entry->get_icon());
    }

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

    void ListItem::set_icon(interfaces::IconVariant str) {
        auto image = GTK_IMAGE(
            gtk_widget_get_template_child(GTK_WIDGET(gobj()), G_TYPE_FROM_INSTANCE(gobj()), "Icon"));
        r_assert(image);
        auto icon = std::visit(
            [](auto &&arg) -> Glib::RefPtr<Gio::Icon> {
                using T = std::decay_t<decltype(arg)>;
                if constexpr (std::is_same_v<T, std::string_view>) {
                    return Gio::Icon::create(std::string(arg));
                } else if constexpr (std::is_same_v<T, Glib::RefPtr<Gio::Icon>>) {
                    return std::move(arg);
                } else {
                    static_assert(false, "non-exhaustive visitor!");
                }
            },
            std::move(str));
        if (icon) {
            gtk_image_set_from_gicon(image, icon->gobj());
        }
    }

    void ListItem::set_title(std::string_view str) {
        return set_title(std::string(str));
    }

    void ListItem::set_subtitle(std::string_view str) {
        return set_subtitle(std::string(str));
    }
} // namespace launcher::ui