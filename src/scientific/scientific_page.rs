use crate::expression_box::{Alignment, ExpressionBox};
use crate::scientific::keypad::keypad;
use cntp_i18n::tr;
use contemporary::components::button::{Button, button};
use contemporary::components::layer::layer;
use contemporary::styling::theme::Theme;
use gpui::{
    App, AppContext, Context, ElementId, Entity, IntoElement, ParentElement, Render, Styled,
    TextAlign, Window, div, px,
};

pub struct ScientificPage {
    expression_box: Entity<ExpressionBox>,

    answer: String,
}

impl ScientificPage {
    pub fn new(cx: &mut App) -> Entity<ScientificPage> {
        cx.new(|cx| {
            let scientific_page = ScientificPage {
                expression_box: ExpressionBox::new(
                    cx,
                    "",
                    tr!("EXPRESSION_PLACEHOLDER", "Expression..."),
                    px(30.).into(),
                    Alignment::Right,
                ),
                answer: "".into(),
            };

            scientific_page
        })
    }
}

fn tool_button(id: impl Into<ElementId>) -> Button {
    button(id).flat().p(px(3.))
}

impl Render for ScientificPage {
    fn render(&mut self, _window: &mut Window, cx: &mut Context<Self>) -> impl IntoElement {
        let theme = cx.global::<Theme>();

        div()
            .h_full()
            .w_full()
            .flex()
            .flex_col()
            .child(div().h(px(40.)))
            .child(
                div()
                    .flex()
                    .flex_col()
                    .flex_grow()
                    .p(px(12.))
                    .gap(px(10.))
                    .child(
                        layer("calculation-layer")
                            .flex()
                            .flex_col()
                            .flex_grow()
                            .child(div().flex_grow())
                            .child(div().h(px(1.)).bg(theme.border_color))
                            .child(self.expression_box.clone())
                            .child(
                                div()
                                    .text_size(px(30.))
                                    .text_align(TextAlign::Right)
                                    .child(self.answer.clone()),
                            )
                            .child(
                                div()
                                    .flex()
                                    .gap(px(3.))
                                    .p(px(3.))
                                    .child(
                                        tool_button("angle-units")
                                            .child(tr!("TRIG_DEGREES_SHORT", "DEG")),
                                    )
                                    .child(
                                        tool_button("output-range")
                                            .child(tr!("RANGE_COMPLEX_SHORT", "CMPLX")),
                                    ),
                            ),
                    )
                    .child(keypad()),
            )
    }
}
