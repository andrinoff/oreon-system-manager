use gtk4::gio;

/// Load the compiled GResource bundle.
/// Call this once at startup (before any UI is built).
pub fn init() {
    let gresource_bytes = include_bytes!(concat!(env!("OUT_DIR"), "/resources.gresource"));
    let bytes = gtk4::glib::Bytes::from(&gresource_bytes[..]);
    let resource =
        gio::Resource::from_data(&bytes).expect("Failed to load embedded GResource bundle");
    gio::resources_register(&resource);
}

/// Get the resource path for the logo.
pub const LOGO_RESOURCE_PATH: &str = "/org/oreon/SystemManager/logo.png";
