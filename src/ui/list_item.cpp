#include "ui/list_item.h"

#include "macros.h"

namespace {
    void init_label(Gtk::Label &label) {
        label.add_css_class("app-label");
        label.set_ellipsize(Pango::EllipsizeMode::END);
        label.set_halign(Gtk::Align::START);
        label.set_hexpand();
        label.set_lines(1);
        label.set_single_line_mode();
    }

    void init_icon(Gtk::Image &icon, int icon_size) {
        icon.add_css_class("app-icon");
        icon.set_halign(Gtk::Align::CENTER);
        icon.set_valign(Gtk::Align::CENTER);
        icon.set_margin(10);
        icon.set_pixel_size(icon_size);
    }
} // namespace

namespace launcher::ui {
    void ActionListItem::build_layout(int icon_size) {
        init_label(m_title);
        m_title.add_css_class("app-title");

        init_icon(m_icon, icon_size);

        std::unique_ptr<Gtk::Box> box {Gtk::make_managed<Gtk::Box>()};
        box->set_overflow(Gtk::Overflow::HIDDEN);
        box->append(m_icon);
        box->append(m_title);

        add_css_class("app-row");
        set_focusable(false);
        set_margin_bottom(2);
        set_margin_start(20);
        set_child(*box.release());
    }

    ActionListItem::ActionListItem(const char *type_name, std::shared_ptr<interfaces::Action> action) :
            Glib::ObjectBase(type_name), Gtk::ListBoxRow(), m_action(std::move(action)) {
        r_assert(m_action);

        set_title(m_action->get_title());
        set_icon(m_action->get_icon());
    }

    ActionListItem::ActionListItem(std::shared_ptr<interfaces::Action> action, int icon_size) :
            ActionListItem("ActionListItem", std::move(action)) {
        build_layout(icon_size);
    }

    ActionListItem::~ActionListItem() = default;

    void ActionListItem::set_title(std::string str) {
        m_title.set_label(std::move(str));
    }

    void ActionListItem::set_title(std::string_view str) {
        return set_title(std::string(str));
    }

    void ActionListItem::set_icon(interfaces::IconVariant str) {
        auto icon = std::visit(
            [](auto &&arg) -> Glib::RefPtr<Gio::Icon> {
                using T = std::decay_t<decltype(arg)>;
                if constexpr (std::is_same_v<T, std::string_view>) {
                    return arg.empty() ? nullptr : Gio::Icon::create(std::string(arg));
                } else if constexpr (std::is_same_v<T, Glib::RefPtr<Gio::Icon>>) {
                    return std::move(arg);
                } else {
                    static_assert(false, "non-exhaustive visitor!");
                }
            },
            std::move(str));
        if (icon) {
            m_icon.property_gicon().set_value(icon);
        }
    }

    void EntryListItem::build_layout(int icon_size) {
        init_label(m_title);
        m_title.add_css_class("app-title");

        init_label(m_subtitle);
        m_subtitle.add_css_class("app-subtitle");

        std::unique_ptr<Gtk::Box> label_box {Gtk::make_managed<Gtk::Box>()};
        label_box->set_halign(Gtk::Align::START);
        label_box->set_hexpand();
        label_box->set_margin_bottom(10);
        label_box->set_margin_end(10);
        label_box->set_margin_top(10);
        label_box->set_orientation(Gtk::Orientation::VERTICAL);
        label_box->set_overflow(Gtk::Overflow::HIDDEN);
        label_box->set_spacing(15);
        label_box->set_valign(Gtk::Align::CENTER);
        label_box->append(m_title);
        label_box->append(m_subtitle);

        init_icon(m_icon, icon_size);

        std::unique_ptr<Gtk::Box> box {Gtk::make_managed<Gtk::Box>()};
        box->set_overflow(Gtk::Overflow::HIDDEN);
        box->append(m_icon);
        box->append(*label_box.release());

        m_tree_expander.set_child(*box.release());

        add_css_class("app-row");
        set_focusable(false);
        set_margin_bottom(2);
        set_child(m_tree_expander);
    }

    EntryListItem::EntryListItem(std::shared_ptr<interfaces::Entry> p_entry, int icon_size,
        const Glib::RefPtr<Gtk::TreeListRow> &tree_list_row) :
            ActionListItem("EntryListItem", std::move(p_entry)) {
        build_layout(icon_size);

        auto entry = get_entry();
        set_subtitle(entry->get_subtitle());

        m_tree_expander.set_list_row(tree_list_row);
        if (entry->get_actions().empty()) {
            m_tree_expander.set_hide_expander();
        }
    }

    EntryListItem::~EntryListItem() = default;

    void EntryListItem::set_subtitle(std::string str) {
        m_subtitle.set_label(std::move(str));
    }

    void EntryListItem::set_subtitle(std::string_view str) {
        return set_subtitle(std::string(str));
    }
} // namespace launcher::ui