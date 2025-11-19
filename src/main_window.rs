use crate::actions::{Degrees, Gradians, Radians};
use crate::main_surface::MainSurface;
use contemporary::about_surface::about_surface;
use contemporary::components::pager::lift_animation::LiftAnimation;
use contemporary::components::pager::pager;
use contemporary::window::contemporary_window;
use gpui::{
    div, App, AppContext, Context, Entity, InteractiveElement, IntoElement, ParentElement,
    Render, Styled, Window,
};
use tcalc::evaluator::AngleUnit;

pub struct MainWindow {
    main_surface: Entity<MainSurface>,
    is_about_surface_open: bool,

    selected_angle_unit: Entity<AngleUnit>,
}

impl MainWindow {
    pub fn new(cx: &mut App) -> Entity<MainWindow> {
        let selected_angle_unit = cx.new(|_| AngleUnit::Degrees);
        cx.new(|cx| MainWindow {
            main_surface: cx.new(|cx| MainSurface::new(selected_angle_unit.clone(), cx)),
            is_about_surface_open: false,
            selected_angle_unit,
        })
    }

    pub fn about_surface_open(&mut self, is_open: bool) -> &Self {
        self.is_about_surface_open = is_open;
        self
    }
}

impl Render for MainWindow {
    fn render(&mut self, _window: &mut Window, cx: &mut Context<Self>) -> impl IntoElement {
        div()
            .on_action(cx.listener(|this, _: &Degrees, _, cx| {
                this.selected_angle_unit.write(cx, AngleUnit::Degrees);
                cx.notify();
            }))
            .on_action(cx.listener(|this, _: &Radians, _, cx| {
                this.selected_angle_unit.write(cx, AngleUnit::Radians);
                cx.notify();
            }))
            .on_action(cx.listener(|this, _: &Gradians, _, cx| {
                this.selected_angle_unit.write(cx, AngleUnit::Gradians);
                cx.notify();
            }))
            .size_full()
            .child(
                contemporary_window().child(
                    pager(
                        "main-window-pager",
                        if self.is_about_surface_open { 1 } else { 0 },
                    )
                    .size_full()
                    .animation(LiftAnimation::new())
                    .page(self.main_surface.clone().into_any_element())
                    .page(
                        about_surface()
                            .on_back_click(cx.listener(|this, _, _, cx| {
                                this.is_about_surface_open = false;
                                cx.notify();
                            }))
                            .into_any_element(),
                    ),
                ),
            )
    }
}
