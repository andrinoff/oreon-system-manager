mod pages;
mod process;
mod resources;
mod theme;
mod widgets;

use gtk4::prelude::*;
use gtk4::{Application, ApplicationWindow, HeaderBar, Orientation};
use theme::SettingsManager;

use pages::container_page::ContainerPage;
use pages::drivers_page::DriversPage;
use pages::package_page::PackagePage;
use pages::repo_page::RepoPage;
use widgets::settings_window::open_settings_window;
use widgets::sidebar::Sidebar;

fn main() {
    let app = Application::builder()
        .application_id("org.oreon.SystemManager")
        .resource_base_path("/org/oreon/SystemManager")
        .build();

    app.connect_startup(|_| {
        resources::init();
        SettingsManager::instance().lock().unwrap().restore();
    });

    app.connect_activate(build_ui);

    let args: Vec<String> = std::env::args().collect();
    app.run_with_args(&args);
}

fn build_ui(app: &Application) {
    // ── Actions ──
    let settings_action = gtk4::gio::SimpleAction::new("settings", None);
    {
        let app_clone = app.clone();
        settings_action.connect_activate(move |_, _| {
            open_settings_window(&app_clone);
        });
    }
    app.add_action(&settings_action);

    let about_action = gtk4::gio::SimpleAction::new("about", None);
    {
        about_action.connect_activate(move |_, _| {
            show_about_dialog();
        });
    }
    app.add_action(&about_action);

    let quit_action = gtk4::gio::SimpleAction::new("quit", None);
    {
        let app_clone = app.clone();
        quit_action.connect_activate(move |_, _| {
            app_clone.quit();
        });
    }
    app.add_action(&quit_action);

    // ── Menu ──
    let menu = gtk4::gio::Menu::new();
    menu.append(Some("Settings"), Some("app.settings"));
    menu.append(Some("About"), Some("app.about"));
    menu.append(Some("Quit"), Some("app.quit"));

    // ── Window ──
    let window = ApplicationWindow::builder()
        .application(app)
        .title("Oreon System Manager")
        .default_width(1100)
        .default_height(700)
        .icon_name("oreon-system-manager")
        .build();

    let header = HeaderBar::new();

    // Hamburger menu button
    let menu_btn = gtk4::MenuButton::new();
    menu_btn.set_icon_name("open-menu-symbolic");
    menu_btn.set_tooltip_text(Some("Menu"));
    menu_btn.set_menu_model(Some(&menu));

    header.pack_end(&menu_btn);

    // ── Main layout: sidebar + page stack ──
    let main_box = gtk4::Box::new(Orientation::Horizontal, 0);
    main_box.set_vexpand(true);

    let sidebar = Sidebar::new();

    let stack = gtk4::Stack::new();
    stack.set_transition_type(gtk4::StackTransitionType::Crossfade);

    let package_page = PackagePage::new();
    let repo_page = RepoPage::new();
    let container_page = ContainerPage::new();
    let drivers_page = DriversPage::new();

    stack.add_named(&package_page, Some("packages"));
    stack.add_named(&repo_page, Some("repos"));
    stack.add_named(&container_page, Some("containers"));
    stack.add_named(&drivers_page, Some("drivers"));

    stack.set_visible_child_name("packages");

    let stack_clone = stack.clone();
    sidebar.connect_page_requested(move |idx| {
        let name = match idx {
            0 => "packages",
            1 => "repos",
            2 => "containers",
            3 => "drivers",
            _ => "packages",
        };
        stack_clone.set_visible_child_name(name);
    });

    let sep = gtk4::Separator::new(Orientation::Vertical);
    sep.add_css_class("sidebar");

    main_box.append(&sidebar);
    main_box.append(&sep);
    main_box.append(&stack);

    let outer = gtk4::Box::new(Orientation::Vertical, 0);
    outer.append(&main_box);

    window.set_titlebar(Some(&header));
    window.set_child(Some(&outer));
    window.present();
}

fn show_about_dialog() {
    let logo = gtk4::gdk::Texture::from_resource(resources::LOGO_RESOURCE_PATH);
    let paintable = logo.upcast::<gtk4::gdk::Paintable>();

    let about = gtk4::AboutDialog::builder()
        .program_name("Oreon System Manager")
        .version("0.1.0")
        .comments("All-in-one system management GUI for Fedora-based Linux distributions")
        .license_type(gtk4::License::Gpl30)
        .website("https://github.com/oreon/oreon-system-manager")
        .logo(&paintable)
        .build();
    about.present();
}
