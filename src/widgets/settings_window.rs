use crate::resources;
use crate::theme::{SettingsManager, ThemeMode};
use gtk4::prelude::*;
use gtk4::{
    Align, ApplicationWindow, Box as GtkBox, CheckButton, HeaderBar, Image, Label, ListBox,
    ListBoxRow, Orientation, Separator, Stack, StackSidebar, Switch,
};
use std::cell::RefCell;
use std::rc::Rc;

/// Open the settings as a separate window with a sidebar.
pub fn open_settings_window(app: &gtk4::Application) {
    let win = ApplicationWindow::builder()
        .application(app)
        .title("Settings")
        .default_width(700)
        .default_height(500)
        .modal(true)
        .resizable(true)
        .build();

    let header = HeaderBar::new();
    win.set_titlebar(Some(&header));

    let main_box = GtkBox::new(Orientation::Horizontal, 0);
    main_box.set_vexpand(true);

    let stack = Stack::new();
    stack.set_transition_type(gtk4::StackTransitionType::Crossfade);
    stack.set_vexpand(true);
    stack.set_hexpand(true);
    stack.set_margin_start(24);
    stack.set_margin_end(24);
    stack.set_margin_top(24);
    stack.set_margin_bottom(24);

    // ── Appearance page ──
    let appearance_page = build_appearance_page();
    stack.add_titled(&appearance_page, Some("appearance"), "Appearance");

    // ── Behavior page ──
    let behavior_page = build_behavior_page();
    stack.add_titled(&behavior_page, Some("behavior"), "Behavior");

    // ── About page ──
    let about_page = build_about_page();
    stack.add_titled(&about_page, Some("about"), "About");

    let sidebar = StackSidebar::new();
    sidebar.set_stack(&stack);
    sidebar.set_width_request(180);

    let sep = Separator::new(Orientation::Vertical);

    main_box.append(&sidebar);
    main_box.append(&sep);
    main_box.append(&stack);

    win.set_child(Some(&main_box));
    win.present();
}

// ──────────────────────────────────────────────
// Appearance page
// ──────────────────────────────────────────────

fn build_appearance_page() -> gtk4::Box {
    let page = GtkBox::new(Orientation::Vertical, 0);

    let title = Label::new(Some("Appearance"));
    title.set_widget_name("pageTitle");
    title.set_halign(Align::Start);
    page.append(&title);

    let sub = Label::new(Some("Choose how Oreon System Manager looks."));
    sub.set_widget_name("pageSubtitle");
    sub.set_halign(Align::Start);
    sub.set_wrap(true);
    page.append(&sub);
    page.append(&GtkBox::new(Orientation::Vertical, 18));

    let group = ListBox::new();
    group.add_css_class("boxed-list");
    group.set_selection_mode(gtk4::SelectionMode::None);

    let modes = [ThemeMode::System, ThemeMode::Light, ThemeMode::Dark];
    let current = SettingsManager::instance().lock().unwrap().theme_mode();
    let group_leader: Rc<RefCell<Option<CheckButton>>> = Rc::new(RefCell::new(None));

    for mode in modes.iter() {
        let row = ListBoxRow::new();

        let content = GtkBox::new(Orientation::Horizontal, 12);
        content.set_margin_start(12);
        content.set_margin_end(12);
        content.set_margin_top(12);
        content.set_margin_bottom(12);

        let icon = Image::from_icon_name(mode.icon());
        icon.set_icon_size(gtk4::IconSize::Large);

        let text_box = GtkBox::new(Orientation::Vertical, 2);
        text_box.set_halign(Align::Start);
        text_box.set_hexpand(true);

        let name_label = Label::new(Some(mode.label()));
        name_label.set_halign(Align::Start);

        let desc = match mode {
            ThemeMode::System => "Follow the system color scheme",
            ThemeMode::Light => "Always use light theme",
            ThemeMode::Dark => "Always use dark theme",
        };
        let desc_label = Label::new(Some(desc));
        desc_label.set_halign(Align::Start);
        desc_label.add_css_class("dim-label");

        text_box.append(&name_label);
        text_box.append(&desc_label);

        let check = CheckButton::new();
        check.set_halign(Align::End);

        let leader = group_leader.borrow().clone();
        if let Some(ref prev) = leader {
            check.set_group(Some(prev));
        } else {
            *group_leader.borrow_mut() = Some(check.clone());
        }

        if *mode == current {
            check.set_active(true);
        }

        let mode_val = *mode;
        let check_for_toggle = check.clone();
        check.connect_toggled(move |_| {
            if check_for_toggle.is_active() {
                SettingsManager::instance()
                    .lock()
                    .unwrap()
                    .apply_theme(mode_val);
            }
        });

        let check_for_row = check.clone();
        row.connect_activate(move |_| {
            check_for_row.set_active(true);
        });

        content.append(&icon);
        content.append(&text_box);
        content.append(&check);
        row.set_child(Some(&content));
        group.append(&row);
    }

    page.append(&group);
    page
}

// ──────────────────────────────────────────────
// Behavior page
// ──────────────────────────────────────────────

fn build_behavior_page() -> gtk4::Box {
    let page = GtkBox::new(Orientation::Vertical, 0);

    let title = Label::new(Some("Behavior"));
    title.set_widget_name("pageTitle");
    title.set_halign(Align::Start);
    page.append(&title);

    let sub = Label::new(Some("Configure how the application behaves."));
    sub.set_widget_name("pageSubtitle");
    sub.set_halign(Align::Start);
    sub.set_wrap(true);
    page.append(&sub);
    page.append(&GtkBox::new(Orientation::Vertical, 18));

    let group = ListBox::new();
    group.add_css_class("boxed-list");
    group.set_selection_mode(gtk4::SelectionMode::None);

    let (confirm, auto_expand) = {
        let sm = SettingsManager::instance().lock().unwrap();
        (sm.get().confirm_remove, sm.get().auto_expand_output)
    };

    let row_confirm = make_switch_row(
        "dialog-warning-symbolic",
        "Confirm before removing",
        "Show a confirmation dialog before removing packages or containers",
        confirm,
    );
    let row_auto = make_switch_row(
        "utilities-terminal-symbolic",
        "Auto-expand output",
        "Automatically expand the output panel when a command runs",
        auto_expand,
    );

    if let Some(switch) = get_switch_from_row(&row_confirm) {
        switch.connect_state_notify(|sw| {
            SettingsManager::instance()
                .lock()
                .unwrap()
                .set_confirm_remove(sw.is_active());
        });
    }

    if let Some(switch) = get_switch_from_row(&row_auto) {
        switch.connect_state_notify(|sw| {
            SettingsManager::instance()
                .lock()
                .unwrap()
                .set_auto_expand_output(sw.is_active());
        });
    }

    group.append(&row_confirm);
    group.append(&row_auto);

    page.append(&group);
    page
}

// ──────────────────────────────────────────────
// About page
// ──────────────────────────────────────────────

fn build_about_page() -> gtk4::Box {
    let page = GtkBox::new(Orientation::Vertical, 0);

    let title = Label::new(Some("About"));
    title.set_widget_name("pageTitle");
    title.set_halign(Align::Start);
    page.append(&title);

    let sub = Label::new(Some("Information about Oreon System Manager."));
    sub.set_widget_name("pageSubtitle");
    sub.set_halign(Align::Start);
    sub.set_wrap(true);
    page.append(&sub);
    page.append(&GtkBox::new(Orientation::Vertical, 18));

    let group = ListBox::new();
    group.add_css_class("boxed-list");
    group.set_selection_mode(gtk4::SelectionMode::None);

    let about_row = ListBoxRow::new();
    about_row.set_activatable(false);
    let about_content = GtkBox::new(Orientation::Horizontal, 12);
    about_content.set_margin_start(12);
    about_content.set_margin_end(12);
    about_content.set_margin_top(12);
    about_content.set_margin_bottom(12);

    let app_icon = load_logo_image(48);

    let about_text = GtkBox::new(Orientation::Vertical, 2);
    about_text.set_halign(Align::Start);
    about_text.set_hexpand(true);

    let name_label = Label::new(Some("Oreon System Manager"));
    name_label.set_halign(Align::Start);

    let version_label = Label::new(Some("Version 0.1.0"));
    version_label.set_halign(Align::Start);
    version_label.add_css_class("dim-label");

    let desc_label = Label::new(Some(
        "All-in-one system management GUI for Fedora-based Linux distributions.",
    ));
    desc_label.set_halign(Align::Start);
    desc_label.set_wrap(true);
    desc_label.add_css_class("dim-label");

    let license_label = Label::new(Some("Licensed under GPL-3.0"));
    license_label.set_halign(Align::Start);
    license_label.add_css_class("dim-label");

    about_text.append(&name_label);
    about_text.append(&version_label);
    about_text.append(&desc_label);
    about_text.append(&license_label);

    about_content.append(&app_icon);
    about_content.append(&about_text);
    about_row.set_child(Some(&about_content));
    group.append(&about_row);

    page.append(&group);
    page
}

// ──────────────────────────────────────────────
// Helpers
// ──────────────────────────────────────────────

fn make_switch_row(icon_name: &str, title: &str, subtitle: &str, active: bool) -> ListBoxRow {
    let row = ListBoxRow::new();
    row.set_activatable(false);

    let content = GtkBox::new(Orientation::Horizontal, 12);
    content.set_margin_start(12);
    content.set_margin_end(12);
    content.set_margin_top(10);
    content.set_margin_bottom(10);

    let icon = Image::from_icon_name(icon_name);
    icon.set_icon_size(gtk4::IconSize::Large);

    let text_box = GtkBox::new(Orientation::Vertical, 2);
    text_box.set_halign(Align::Start);
    text_box.set_hexpand(true);

    let title_label = Label::new(Some(title));
    title_label.set_halign(Align::Start);

    let subtitle_label = Label::new(Some(subtitle));
    subtitle_label.set_halign(Align::Start);
    subtitle_label.add_css_class("dim-label");

    text_box.append(&title_label);
    text_box.append(&subtitle_label);

    let switch = Switch::new();
    switch.set_active(active);
    switch.set_halign(Align::End);

    content.append(&icon);
    content.append(&text_box);
    content.append(&switch);
    row.set_child(Some(&content));
    row
}

fn get_switch_from_row(row: &ListBoxRow) -> Option<Switch> {
    let content = row.child()?;
    let content = content.downcast::<GtkBox>().ok()?;
    let mut child = content.first_child();
    while let Some(c) = child {
        if let Some(sw) = c.downcast_ref::<Switch>() {
            return Some(sw.clone());
        }
        child = c.next_sibling();
    }
    None
}

/// Load the logo from the embedded GResource as an `Image` widget.
fn load_logo_image(size: i32) -> Image {
    let texture = gtk4::gdk::Texture::from_resource(resources::LOGO_RESOURCE_PATH);
    let img = Image::from_paintable(Some(&texture));
    img.set_pixel_size(size);
    img
}
