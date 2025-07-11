use crate::actions::{Degrees, Gradians, Radians};
use crate::main_surface::MainSurfaceTab::Scientific;
use crate::scientific::scientific_page::ScientificPage;
use cntp_i18n::tr;
use contemporary::components::application_menu::ApplicationMenu;
use contemporary::components::button::button;
use contemporary::components::pager::pager;
use contemporary::styling::theme::Theme;
use contemporary::surface::surface;
use gpui::{
    App, AppContext, Context, Entity, InteractiveElement, IntoElement, Menu, MenuItem,
    ParentElement, Render, Styled, Window, div, px,
};
use std::path::Components;
use contemporary::components::icon_text::icon_text;

pub struct MainSurface {
    scientific_page: Entity<ScientificPage>,

    application_menu: Entity<ApplicationMenu>,
    selected_tab: MainSurfaceTab,
}

#[derive(PartialEq)]
enum MainSurfaceTab {
    Scientific,
}

impl MainSurfaceTab {
    fn index(&self) -> usize {
        match self {
            Scientific => 0,
        }
    }
}

impl MainSurface {
    pub fn new(cx: &mut App) -> Entity<MainSurface> {
        cx.new(|cx| MainSurface {
            scientific_page: ScientificPage::new(cx),
            application_menu: ApplicationMenu::new(
                cx,
                Menu {
                    name: "Application Menu".into(),
                    items: vec![MenuItem::submenu(Menu {
                        name: tr!("MENU_TRIGONOMETRY").into(),
                        items: vec![
                            MenuItem::action(tr!("TRIG_DEGREES"), Degrees),
                            MenuItem::action(tr!("TRIG_RADIANS"), Radians),
                            MenuItem::action(tr!("TRIG_GRADIANS"), Gradians),
                        ],
                    })],
                },
            ),
            selected_tab: Scientific,
        })
    }
}

impl Render for MainSurface {
    fn render(&mut self, window: &mut Window, cx: &mut Context<Self>) -> impl IntoElement {
        let theme = cx.global::<Theme>();

        surface()
            .actions(
                div().occlude().flex().content_stretch().child(
                    div()
                        .flex()
                        .id("action-bar")
                        .bg(theme.button_background)
                        .rounded(theme.border_radius)
                        .gap(px(2.))
                        .content_stretch()
                        .child(
                            button("scientific-button")
                                .child(icon_text("calculator-scientific".into(), tr!("SCIENTIFIC_BUTTON", "Scientific").into()))
                                .checked_when(self.selected_tab == Scientific)
                                .on_click(cx.listener(|this, _, _, cx| {
                                    this.selected_tab = Scientific;
                                    cx.notify();
                                })),
                        ),
                ),
            )
            .child(
                pager("main-pager", self.selected_tab.index())
                    .w_full()
                    .h_full()
                    .page(self.scientific_page.clone().into_any_element()),
            )
            .application_menu(self.application_menu.clone())
    }
}
