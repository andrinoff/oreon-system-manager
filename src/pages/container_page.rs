use crate::pages::package_page::format_result;
use crate::process::{
    self, extract_distrobox_name, extract_docker_name, parse_distrobox_list_output,
    parse_docker_ps_output, ProcessRequest,
};
use crate::widgets::collapsible_output::CollapsibleOutput;
use glib::subclass::prelude::ObjectSubclassIsExt;
use gtk4::glib;
use gtk4::prelude::*;
use gtk4::ListView;
use gtk4::{
    Button, Label, Orientation, PolicyType, ScrolledWindow, SignalListItemFactory, StringList,
    StringObject,
};

glib::wrapper! {
    pub struct ContainerPage(ObjectSubclass<imp::Imp>)
        @extends gtk4::Box, gtk4::Widget, @implements gtk4::Orientable, gtk4::Accessible, gtk4::Buildable, gtk4::ConstraintTarget;
}

mod imp {
    use super::*;
    use gtk4::subclass::prelude::*;
    use std::cell::RefCell;

    pub struct Imp {
        pub docker_list: RefCell<Option<StringList>>,
        pub distrobox_list: RefCell<Option<StringList>>,
        pub docker_selection: RefCell<Option<u32>>,
        pub distrobox_selection: RefCell<Option<u32>>,
        pub output: RefCell<Option<CollapsibleOutput>>,
    }

    impl Default for Imp {
        fn default() -> Self {
            Self {
                docker_list: RefCell::new(None),
                distrobox_list: RefCell::new(None),
                docker_selection: RefCell::new(None),
                distrobox_selection: RefCell::new(None),
                output: RefCell::new(None),
            }
        }
    }

    #[glib::object_subclass]
    impl ObjectSubclass for Imp {
        const NAME: &'static str = "OreonContainerPage";
        type Type = super::ContainerPage;
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
        if let Some(so) = item.item().and_then(|i| i.downcast::<StringObject>().ok()) {
            label.set_label(&so.string());
        }
    });
    factory
}

fn make_tab(model: &StringList, actions: &[&str]) -> (gtk4::Box, gtk4::SingleSelection) {
    let tab = gtk4::Box::new(Orientation::Vertical, 8);
    tab.set_margin_top(8);

    let btn_row = gtk4::Box::new(Orientation::Horizontal, 8);
    btn_row.set_margin_start(0);
    btn_row.set_margin_end(0);
    for action in actions {
        let btn = Button::with_label(action);
        if *action == "Start" || *action == "Enter" {
            btn.add_css_class("suggested-action");
        } else if *action == "Remove" || *action == "Delete" {
            btn.add_css_class("destructive-action");
        }
        btn_row.append(&btn);
    }
    let stretch = gtk4::Box::new(Orientation::Horizontal, 0);
    stretch.set_hexpand(true);
    btn_row.append(&stretch);
    tab.append(&btn_row);

    let selection_model = gtk4::SingleSelection::new(Some(model.clone()));
    let factory = make_list_factory();
    let list_view = ListView::new(Some(selection_model.clone()), Some(factory));
    list_view.set_vexpand(true);

    let scrolled = ScrolledWindow::new();
    scrolled.set_child(Some(&list_view));
    scrolled.set_policy(PolicyType::Automatic, PolicyType::Automatic);
    scrolled.set_vexpand(true);
    tab.append(&scrolled);

    (tab, selection_model)
}

impl ContainerPage {
    pub fn new() -> Self {
        let obj: Self = glib::Object::new();
        let box_ref: &gtk4::Box = obj.upcast_ref();

        let title = Label::new(Some("Containers"));
        title.set_widget_name("pageTitle");
        title.set_halign(gtk4::Align::Start);
        box_ref.append(&title);

        let sub = Label::new(Some("Manage Docker and Distrobox containers."));
        sub.set_widget_name("pageSubtitle");
        sub.set_halign(gtk4::Align::Start);
        sub.set_wrap(true);
        box_ref.append(&sub);
        box_ref.append(&gtk4::Box::new(Orientation::Vertical, 18));

        let card = gtk4::Frame::new(None);
        card.set_vexpand(true);
        let notebook = gtk4::Notebook::new();

        let docker_model = StringList::new(&[]);
        let (docker_tab, docker_sel) =
            make_tab(&docker_model, &["Start", "Stop", "Remove", "Refresh"]);
        notebook.append_page(&docker_tab, Some(&Label::new(Some("Docker"))));

        let distrobox_model = StringList::new(&[]);
        let (distrobox_tab, distrobox_sel) =
            make_tab(&distrobox_model, &["Enter", "Stop", "Delete", "Refresh"]);
        notebook.append_page(&distrobox_tab, Some(&Label::new(Some("Distrobox"))));

        card.set_child(Some(&notebook));
        box_ref.append(&card);
        box_ref.append(&gtk4::Box::new(Orientation::Vertical, 14));

        let output = CollapsibleOutput::new();
        box_ref.append(&output);

        *obj.imp().docker_list.borrow_mut() = Some(docker_model.clone());
        *obj.imp().distrobox_list.borrow_mut() = Some(distrobox_model.clone());
        *obj.imp().output.borrow_mut() = Some(output.clone());

        // Docker selection
        let obj_c = obj.clone();
        docker_sel.connect_selection_changed(move |sel, _, _| {
            *obj_c.imp().docker_selection.borrow_mut() = Some(sel.selected());
        });

        // Docker buttons
        let docker_buttons: Vec<Button> = docker_tab
            .first_child()
            .and_then(|w| w.downcast::<gtk4::Box>().ok())
            .map(|b| {
                let mut btns = Vec::new();
                let mut child = b.first_child();
                while let Some(c) = child {
                    if let Some(btn) = c.downcast_ref::<Button>() {
                        btns.push(btn.clone());
                    }
                    child = c.next_sibling();
                }
                btns
            })
            .unwrap_or_default();

        for btn in &docker_buttons {
            let label = btn.label().map(|s| s.to_string()).unwrap_or_default();
            let obj_c = obj.clone();
            btn.connect_clicked(move |_| obj_c.on_docker_action(&label));
        }

        // Distrobox selection
        let obj_c = obj.clone();
        distrobox_sel.connect_selection_changed(move |sel, _, _| {
            *obj_c.imp().distrobox_selection.borrow_mut() = Some(sel.selected());
        });

        // Distrobox buttons
        let distrobox_buttons: Vec<Button> = distrobox_tab
            .first_child()
            .and_then(|w| w.downcast::<gtk4::Box>().ok())
            .map(|b| {
                let mut btns = Vec::new();
                let mut child = b.first_child();
                while let Some(c) = child {
                    if let Some(btn) = c.downcast_ref::<Button>() {
                        btns.push(btn.clone());
                    }
                    child = c.next_sibling();
                }
                btns
            })
            .unwrap_or_default();

        for btn in &distrobox_buttons {
            let label = btn.label().map(|s| s.to_string()).unwrap_or_default();
            let obj_c = obj.clone();
            btn.connect_clicked(move |_| obj_c.on_distrobox_action(&label));
        }

        // Auto-refresh
        let obj_c = obj.clone();
        glib::idle_add_local_once(move || {
            obj_c.refresh_docker();
            obj_c.refresh_distrobox();
        });

        obj
    }

    fn docker_selected(&self) -> Option<String> {
        let imp = self.imp();
        let model = imp.docker_list.borrow();
        let idx = imp.docker_selection.borrow();
        if let (Some(m), Some(i)) = (model.as_ref(), *idx) {
            if (i as usize) < m.n_items() as usize {
                return Some(m.string(i).map(|s| s.to_string()).unwrap_or_default());
            }
        }
        None
    }

    fn distrobox_selected(&self) -> Option<String> {
        let imp = self.imp();
        let model = imp.distrobox_list.borrow();
        let idx = imp.distrobox_selection.borrow();
        if let (Some(m), Some(i)) = (model.as_ref(), *idx) {
            if (i as usize) < m.n_items() as usize {
                return Some(m.string(i).map(|s| s.to_string()).unwrap_or_default());
            }
        }
        None
    }

    fn refresh_docker(&self) {
        let imp = self.imp();
        if let Some(m) = imp.docker_list.borrow().as_ref() {
            m.splice(0, m.n_items(), &[]);
        }
        let request = ProcessRequest::new(
            "docker",
            &[
                "ps",
                "-a",
                "--format",
                "{{.Names}}\t{{.Status}}\t{{.Image}}",
            ],
        );
        let output = imp.output.borrow().clone();
        let model = imp.docker_list.borrow().clone();
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
                        for item in parse_docker_ps_output(text) {
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

    fn refresh_distrobox(&self) {
        let imp = self.imp();
        if let Some(m) = imp.distrobox_list.borrow().as_ref() {
            m.splice(0, m.n_items(), &[]);
        }
        let request = ProcessRequest::new("distrobox", &["list"]);
        let output = imp.output.borrow().clone();
        let model = imp.distrobox_list.borrow().clone();
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
                        for item in parse_distrobox_list_output(text) {
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

    fn on_docker_action(&self, action: &str) {
        if action == "Refresh" {
            self.refresh_docker();
            return;
        }
        if let Some(text) = self.docker_selected() {
            let name = extract_docker_name(&text);
            let imp = self.imp();
            if let Some(ref o) = *imp.output.borrow() {
                o.expand();
            }
            match action {
                "Start" => self.run_docker(&["start", &name]),
                "Stop" => self.run_docker(&["stop", &name]),
                "Remove" => self.run_docker(&["rm", &name]),
                _ => {}
            }
        }
    }

    fn on_distrobox_action(&self, action: &str) {
        if action == "Refresh" {
            self.refresh_distrobox();
            return;
        }
        if let Some(text) = self.distrobox_selected() {
            let name = extract_distrobox_name(&text);
            let imp = self.imp();
            if let Some(ref o) = *imp.output.borrow() {
                o.expand();
            }
            match action {
                "Enter" => self.run_distrobox(&["enter", &name]),
                "Stop" => self.run_distrobox(&["stop", &name]),
                "Delete" => self.run_distrobox(&["rm", &name]),
                _ => {}
            }
        }
    }

    fn run_docker(&self, args: &[&str]) {
        let imp = self.imp();
        let request = ProcessRequest::new("docker", args);
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

    fn run_distrobox(&self, args: &[&str]) {
        let imp = self.imp();
        let request = ProcessRequest::new("distrobox", args);
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

impl Default for ContainerPage {
    fn default() -> Self {
        Self::new()
    }
}
