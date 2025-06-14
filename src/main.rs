mod contemporary;
mod surface_list;

use crate::contemporary::application::Details;
use crate::contemporary::setup::{Contemporary, ContemporaryMenus, setup_contemporary};
use crate::contemporary::surface::Surface;
use crate::contemporary::window::{ContemporaryWindow, PushPop, contemporary_window_options};
use crate::surface_list::{HelloWorld, SurfaceList};
use gpui::{
    App, AppContext, Application, Bounds, IntoElement, ParentElement, Render, Styled, WindowBounds,
    WindowOptions, px, size,
};
use std::rc::Rc;
use crate::contemporary::about_surface::AboutSurface;

fn main() {
    Application::new().run(|cx: &mut App| {
        let bounds = Bounds::centered(None, size(px(800.0), px(600.0)), cx);

        let default_window_options = contemporary_window_options(cx);
        cx.open_window(
            WindowOptions {
                window_bounds: Some(WindowBounds::Windowed(bounds)),
                ..default_window_options
            },
            |_, cx| {
                let mut window = ContemporaryWindow::new(cx);
                let weak_window = window.downgrade();
                let weak_widow = window.downgrade();

                setup_contemporary(
                    cx,
                    Contemporary {
                        details: Details {
                            application_name: "theCalculator",
                            desktop_entry: "com.vicr123.thecalculator",
                            application_version: "3.0",
                        },
                        menus: ContemporaryMenus {
                            menus: vec![],
                            on_about: Rc::new(move |cx| {
                                let about_surface = AboutSurface::new(cx, weak_widow.clone());
                                let a_surface = cx.new(|_| SurfaceList::About(about_surface));
                                let sf = Surface::new(cx, a_surface);
                                weak_widow.upgrade().unwrap().push(cx, sf);
                            }),
                        },
                    },
                );
                
                let window_contents = cx.new(|cx| {
                    SurfaceList::HelloWorld(cx.new(|_| HelloWorld {
                        window: weak_window,
                    }))
                });
                let surface = Surface::new(cx, window_contents);
                window.push(cx, surface);
                window
            },
        )
        .unwrap();
        cx.activate(true);
    });
}
