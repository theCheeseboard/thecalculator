mod actions;
mod expression_box;
mod main_surface;
mod main_window;
mod scientific;

use crate::actions::{Degrees, Gradians, Radians};
use crate::expression_box::bind_expression_box_keys;
use crate::main_window::MainWindow;
use cntp_i18n::{I18N_MANAGER, tr, tr_load};
use cntp_icon_tool_macros::application_icon;
use contemporary::application::{ApplicationLink, Details, License, new_contemporary_application};
use contemporary::components::dialog_box::StandardButton::No;
use contemporary::macros::application_details;
use contemporary::setup::{Contemporary, ContemporaryMenus, setup_contemporary};
use contemporary::window::contemporary_window_options;
use gpui::{App, Bounds, Menu, MenuItem, WindowBounds, WindowOptions, px, size};
use smol_macros::main;
use std::rc::Rc;

fn mane() {
    application_icon!("../dist/baseicon.svg");
    new_contemporary_application().run(|cx: &mut App| {
        I18N_MANAGER.write().unwrap().load_source(tr_load!());
        let bounds = Bounds::centered(None, size(px(800.0), px(600.0)), cx);

        bind_expression_box_keys(cx);

        let default_window_options = contemporary_window_options(cx, "theCalculator".into());
        cx.open_window(
            WindowOptions {
                window_bounds: Some(WindowBounds::Windowed(bounds)),
                ..default_window_options
            },
            |_, cx| {
                let window = MainWindow::new(cx);
                let weak_window = window.downgrade();

                setup_contemporary(
                    cx,
                    Contemporary {
                        details: Details {
                            generatable: application_details!(),
                            copyright_holder: "Victor Tran",
                            copyright_year: "2025",
                            application_version: "3.0",
                            license: License::Gpl3OrLater,
                            links: [
                                (
                                    ApplicationLink::FileBug,
                                    "https://github.com/vicr123/thecalculator/issues",
                                ),
                                (
                                    ApplicationLink::SourceCode,
                                    "https://github.com/vicr123/thecalculator",
                                ),
                            ]
                            .into(),
                        },
                        menus: ContemporaryMenus {
                            menus: vec![Menu {
                                name: tr!("MENU_TRIGONOMETRY", "Trigonometry").into(),
                                items: vec![
                                    MenuItem::action(tr!("TRIG_DEGREES", "Degrees"), Degrees),
                                    MenuItem::action(tr!("TRIG_RADIANS", "Radians"), Radians),
                                    MenuItem::action(tr!("TRIG_GRADIANS", "Gradians"), Gradians),
                                ],
                            }],
                            on_about: Rc::new(move |cx| {
                                weak_window.upgrade().unwrap().update(cx, |window, cx| {
                                    window.about_surface_open(true);
                                    cx.notify()
                                })
                            }),
                            on_settings: None,
                        },
                    },
                );

                window
            },
        )
        .unwrap();
        cx.activate(true);
    });
}

main! {
    async fn main() {
        mane()
    }
}
