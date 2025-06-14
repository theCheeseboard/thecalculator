use gpui::Global;

pub struct Details {
    pub application_name: &'static str,
    pub application_version: &'static str,
    pub desktop_entry: &'static str,
}

impl Global for Details {}
