use crate::theme::SettingsManager;
use glib::subclass::prelude::ObjectSubclassIsExt;
use gtk4::glib;
use gtk4::prelude::*;
use gtk4::{Button, PolicyType, ScrolledWindow, TextBuffer, TextView};

glib::wrapper! {
    pub struct CollapsibleOutput(ObjectSubclass<imp::Imp>)
        @extends gtk4::Box, gtk4::Widget, @implements gtk4::Orientable, gtk4::Accessible, gtk4::Buildable, gtk4::ConstraintTarget;
}

mod imp {
    use super::*;
    use gtk4::subclass::prelude::*;
    use std::cell::{Cell, RefCell};

    pub struct Imp {
        pub toggle: RefCell<Option<Button>>,
        pub buffer: TextBuffer,
        pub view: RefCell<Option<TextView>>,
        pub scrolled: RefCell<Option<ScrolledWindow>>,
        pub expanded: Cell<bool>,
    }

    impl Default for Imp {
        fn default() -> Self {
            let buffer = TextBuffer::new(None::<&gtk4::TextTagTable>);
            Self {
                toggle: RefCell::new(None),
                buffer,
                view: RefCell::new(None),
                scrolled: RefCell::new(None),
                expanded: Cell::new(false),
            }
        }
    }

    #[glib::object_subclass]
    impl ObjectSubclass for Imp {
        const NAME: &'static str = "OreonCollapsibleOutput";
        type Type = super::CollapsibleOutput;
        type ParentType = gtk4::Box;
    }

    impl ObjectImpl for Imp {
        fn constructed(&self) {
            let obj = self.obj();
            obj.set_orientation(gtk4::Orientation::Vertical);
            obj.set_spacing(6);

            let view = TextView::with_buffer(&self.buffer);
            view.set_widget_name("terminal");
            view.set_editable(false);
            view.set_monospace(true);
            view.set_top_margin(12);
            view.set_bottom_margin(12);
            view.set_left_margin(12);
            view.set_right_margin(12);

            let scrolled = ScrolledWindow::new();
            scrolled.set_child(Some(&view));
            scrolled.set_policy(PolicyType::Automatic, PolicyType::Automatic);
            scrolled.set_min_content_height(130);
            scrolled.set_max_content_height(220);
            scrolled.set_visible(false);

            let toggle = Button::with_label("Output");
            toggle.set_has_frame(false);
            toggle.set_halign(gtk4::Align::Start);

            obj.upcast_ref::<gtk4::Box>().append(&toggle);
            obj.upcast_ref::<gtk4::Box>().append(&scrolled);

            let obj_clone = obj.clone();
            toggle.connect_clicked(move |_| {
                obj_clone.toggle();
            });

            *self.toggle.borrow_mut() = Some(toggle);
            *self.view.borrow_mut() = Some(view);
            *self.scrolled.borrow_mut() = Some(scrolled);
        }
    }

    impl BoxImpl for Imp {}
    impl WidgetImpl for Imp {}
}

impl CollapsibleOutput {
    pub fn new() -> Self {
        glib::Object::new()
    }

    pub fn append(&self, text: &str) {
        let imp = self.imp();
        let buffer = &imp.buffer;
        let mut end_iter = buffer.end_iter();
        buffer.insert(&mut end_iter, text);
        if !text.ends_with('\n') {
            let mut end_iter = buffer.end_iter();
            buffer.insert(&mut end_iter, "\n");
        }
    }

    pub fn clear(&self) {
        let imp = self.imp();
        let buffer = &imp.buffer;
        let mut start = buffer.start_iter();
        let mut end = buffer.end_iter();
        buffer.delete(&mut start, &mut end);
    }

    pub fn expand(&self) {
        let imp = self.imp();
        let auto = SettingsManager::instance()
            .lock()
            .map(|s| s.get().auto_expand_output)
            .unwrap_or(true);
        if !auto {
            return;
        }
        if !imp.expanded.get() {
            imp.expanded.set(true);
            if let Some(ref scrolled) = *imp.scrolled.borrow() {
                scrolled.set_visible(true);
            }
            self.sync_label();
        }
    }

    pub fn toggle(&self) {
        let imp = self.imp();
        let new_state = !imp.expanded.get();
        imp.expanded.set(new_state);
        if let Some(ref scrolled) = *imp.scrolled.borrow() {
            scrolled.set_visible(new_state);
        }
        self.sync_label();
    }

    fn sync_label(&self) {
        let imp = self.imp();
        let label = if imp.expanded.get() {
            "Hide Output"
        } else {
            "Show Output"
        };
        if let Some(ref toggle) = *imp.toggle.borrow() {
            toggle.set_label(label);
        }
    }
}

impl Default for CollapsibleOutput {
    fn default() -> Self {
        Self::new()
    }
}
