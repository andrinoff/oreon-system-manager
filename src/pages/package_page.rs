use crate::process::{self, extract_package_name, parse_dnf_search_output, ProcessRequest};
use crate::widgets::collapsible_output::CollapsibleOutput;
use glib::subclass::prelude::ObjectSubclassIsExt;
use gtk4::glib;
use gtk4::prelude::*;
use gtk4::ListView;
use gtk4::{
    Button, Entry, Label, Orientation, PolicyType, ScrolledWindow, SignalListItemFactory,
    StringList, StringObject,
};

glib::wrapper! {
    pub struct PackagePage(ObjectSubclass<imp::Imp>)
        @extends gtk4::Box, gtk4::Widget, @implements gtk4::Orientable, gtk4::Accessible, gtk4::Buildable, gtk4::ConstraintTarget;
}

mod imp {
    use super::*;
    use gtk4::subclass::prelude::*;
    use std::cell::RefCell;

    pub struct Imp {
        pub search_bar: RefCell<Option<Entry>>,
        pub package_list: RefCell<Option<StringList>>,
        pub install_btn: RefCell<Option<Button>>,
        pub remove_btn: RefCell<Option<Button>>,
        pub output: RefCell<Option<CollapsibleOutput>>,
        pub selected_index: RefCell<Option<u32>>,
    }

    impl Default for Imp {
        fn default() -> Self {
            Self {
                search_bar: RefCell::new(None),
                package_list: RefCell::new(None),
                install_btn: RefCell::new(None),
                remove_btn: RefCell::new(None),
                output: RefCell::new(None),
                selected_index: RefCell::new(None),
            }
        }
    }

    #[glib::object_subclass]
    impl ObjectSubclass for Imp {
        const NAME: &'static str = "OreonPackagePage";
        type Type = super::PackagePage;
        type ParentType = gtk4::Box;
    }

    impl ObjectImpl for Imp {
        fn constructed(&self) {
            let obj = self.obj();
            obj.set_orientation(Orientation::Vertical);
            obj.set_margin_start(24);
            obj.set_margin_end(24);
            obj.set_margin_top(24);
            obj.set_margin_bottom(24);
            obj.set_spacing(0);
        }
    }

    impl BoxImpl for Imp {}
    impl WidgetImpl for Imp {}
}

fn make_list_factory() -> SignalListItemFactory {
    let factory = SignalListItemFactory::new();
    factory.connect_setup(move |_f, item| {
        let label = Label::new(None);
        label.set_halign(gtk4::Align::Start);
        label.set_margin_start(16);
        label.set_margin_end(16);
        label.set_margin_top(7);
        label.set_margin_bottom(7);
        item.set_child(Some(&label));
    });
    factory.connect_bind(move |_f, item| {
        let label = item
            .child()
            .and_then(|w| w.downcast::<Label>().ok())
            .unwrap();
        if let Some(string_obj) = item.item().and_then(|i| i.downcast::<StringObject>().ok()) {
            label.set_label(&string_obj.string());
        }
    });
    factory
}

impl PackagePage {
    pub fn new() -> Self {
        let obj: Self = glib::Object::new();
        let box_ref: &gtk4::Box = obj.upcast_ref();

        let title = Label::new(Some("Packages"));
        title.set_widget_name("pageTitle");
        title.set_halign(gtk4::Align::Start);
        box_ref.append(&title);

        let sub = Label::new(Some("Search, install, and remove DNF packages."));
        sub.set_widget_name("pageSubtitle");
        sub.set_halign(gtk4::Align::Start);
        sub.set_wrap(true);
        box_ref.append(&sub);
        box_ref.append(&gtk4::Box::new(Orientation::Vertical, 18));

        let search_row = gtk4::Box::new(Orientation::Horizontal, 8);
        let search_bar = Entry::new();
        search_bar.set_placeholder_text(Some("Search packages\u{2026}"));
        search_bar.set_hexpand(true);
        search_bar.set_icon_from_icon_name(
            gtk4::EntryIconPosition::Primary,
            Some("system-search-symbolic"),
        );
        let search_btn = Button::with_label("Search");
        search_btn.add_css_class("suggested-action");
        search_row.append(&search_bar);
        search_row.append(&search_btn);
        box_ref.append(&search_row);
        box_ref.append(&gtk4::Box::new(Orientation::Vertical, 12));

        let card = gtk4::Frame::new(None);
        card.set_vexpand(true);
        let model = StringList::new(&[]);
        let selection_model = gtk4::SingleSelection::new(Some(model.clone()));
        let factory = make_list_factory();
        let list_view = ListView::new(Some(selection_model.clone()), Some(factory));
        let scrolled = ScrolledWindow::new();
        scrolled.set_child(Some(&list_view));
        scrolled.set_policy(PolicyType::Automatic, PolicyType::Automatic);
        card.set_child(Some(&scrolled));
        box_ref.append(&card);
        box_ref.append(&gtk4::Box::new(Orientation::Vertical, 10));

        let action_row = gtk4::Box::new(Orientation::Horizontal, 8);
        let install_btn = Button::with_label("Install");
        let remove_btn = Button::with_label("Remove");
        install_btn.add_css_class("suggested-action");
        remove_btn.add_css_class("destructive-action");
        install_btn.set_sensitive(false);
        remove_btn.set_sensitive(false);
        action_row.append(&install_btn);
        action_row.append(&remove_btn);
        let stretch = gtk4::Box::new(Orientation::Horizontal, 0);
        stretch.set_hexpand(true);
        action_row.append(&stretch);
        box_ref.append(&action_row);
        box_ref.append(&gtk4::Box::new(Orientation::Vertical, 14));

        let output = CollapsibleOutput::new();
        box_ref.append(&output);

        *obj.imp().search_bar.borrow_mut() = Some(search_bar.clone());
        *obj.imp().package_list.borrow_mut() = Some(model.clone());
        *obj.imp().install_btn.borrow_mut() = Some(install_btn.clone());
        *obj.imp().remove_btn.borrow_mut() = Some(remove_btn.clone());
        *obj.imp().output.borrow_mut() = Some(output.clone());

        let obj_c = obj.clone();
        search_btn.connect_clicked(move |_| obj_c.on_search());
        let obj_c = obj.clone();
        search_bar.connect_activate(move |_| obj_c.on_search());
        let obj_c = obj.clone();
        install_btn.connect_clicked(move |_| obj_c.on_install());
        let obj_c = obj.clone();
        remove_btn.connect_clicked(move |_| obj_c.on_remove());

        let obj_c = obj.clone();
        selection_model.connect_selection_changed(move |sel, _, _| {
            *obj_c.imp().selected_index.borrow_mut() = Some(sel.selected());
            let has = sel.selected_item().is_some();
            if let Some(ref b) = *obj_c.imp().install_btn.borrow() {
                b.set_sensitive(has);
            }
            if let Some(ref b) = *obj_c.imp().remove_btn.borrow() {
                b.set_sensitive(has);
            }
        });

        obj
    }

    fn selected_text(&self) -> Option<String> {
        let imp = self.imp();
        let model = imp.package_list.borrow();
        let idx = imp.selected_index.borrow();
        if let (Some(model), Some(idx)) = (model.as_ref(), *idx) {
            if (idx as usize) < model.n_items() as usize {
                return Some(model.string(idx).map(|s| s.to_string()).unwrap_or_default());
            }
        }
        None
    }

    fn on_search(&self) {
        let imp = self.imp();
        let query = imp
            .search_bar
            .borrow()
            .as_ref()
            .map(|e| e.text().trim().to_string())
            .unwrap_or_default();
        if query.is_empty() {
            return;
        }
        if let Some(m) = imp.package_list.borrow().as_ref() {
            m.splice(0, m.n_items(), &[]);
        }
        if let Some(ref o) = *imp.output.borrow() {
            o.clear();
        }

        let request = ProcessRequest::new("dnf", &["search", "--quiet", &query]);
        let output = imp.output.borrow().clone();
        let model = imp.package_list.borrow().clone();
        let output_c = output.clone();
        let model_c = model.clone();
        process::run_process(
            request,
            true,
            move |is_list, text| {
                if let Some(ref o) = output_c {
                    o.append(text);
                }
                if is_list {
                    if let Some(ref m) = model_c {
                        for item in parse_dnf_search_output(text) {
                            m.append(&item);
                        }
                    }
                }
            },
            move |code| {
                if let Some(ref o) = output {
                    o.append(&format_result(code));
                }
            },
        );
    }

    fn on_install(&self) {
        if let Some(text) = self.selected_text() {
            let pkg = extract_package_name(&text);
            if pkg.is_empty() {
                return;
            }
            let imp = self.imp();
            if let Some(ref o) = *imp.output.borrow() {
                o.clear();
                o.expand();
            }
            self.run_dnf(&["install", "-y", &pkg]);
        }
    }

    fn on_remove(&self) {
        if let Some(text) = self.selected_text() {
            let pkg = extract_package_name(&text);
            if pkg.is_empty() {
                return;
            }
            let imp = self.imp();
            if let Some(ref o) = *imp.output.borrow() {
                o.clear();
                o.expand();
            }
            self.run_dnf(&["remove", "-y", &pkg]);
        }
    }

    fn run_dnf(&self, args: &[&str]) {
        let imp = self.imp();
        let (program, full_args): (&str, Vec<&str>) =
            if args.first() == Some(&"search") || args.first() == Some(&"list") {
                ("dnf", args.to_vec())
            } else {
                let mut v = vec!["dnf"];
                v.extend(args);
                ("pkexec", v)
            };
        let request = ProcessRequest::new(program, &full_args);
        let output = imp.output.borrow().clone();
        let output_c = output.clone();
        process::run_process(
            request,
            false,
            move |_, text| {
                if let Some(ref o) = output_c {
                    o.append(text);
                }
            },
            move |code| {
                if let Some(ref o) = output {
                    o.append(&format_result(code));
                }
            },
        );
    }
}

pub fn format_result(code: Option<i32>) -> String {
    match code {
        Some(0) => "\n[Done]".to_string(),
        Some(c) => format!("\n[Failed \u{2014} exit code {}]", c),
        None => "\n[Failed \u{2014} process error]".to_string(),
    }
}

impl Default for PackagePage {
    fn default() -> Self {
        Self::new()
    }
}
