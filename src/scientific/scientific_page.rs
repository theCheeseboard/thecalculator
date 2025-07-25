use crate::expression_box::{Alignment, ExpressionBox, TextChangeEvent};
use crate::scientific::keypad::{KeypadButtonClickEvent, keypad};
use cntp_i18n::tr;
use contemporary::components::button::{Button, button};
use contemporary::components::layer::layer;
use contemporary::styling::theme::Theme;
use gpui::{
    App, AppContext, Context, ElementId, Entity, EntityInputHandler, IntoElement, ParentElement,
    Render, SharedString, Styled, TextAlign, Window, div, px,
};

pub struct ScientificPage {
    expression_box: Entity<ExpressionBox>,

    answer: SharedString,
}

impl ScientificPage {
    pub fn new(cx: &mut App) -> Entity<ScientificPage> {
        cx.new(|cx| {
            let expression_box_text_changed_listener =
                cx.listener(Self::expression_box_text_changed);

            let scientific_page = ScientificPage {
                expression_box: ExpressionBox::new(
                    cx,
                    "",
                    tr!("EXPRESSION_PLACEHOLDER", "Expression..."),
                    px(30.).into(),
                    Alignment::Right,
                    expression_box_text_changed_listener,
                ),
                answer: "".into(),
            };

            scientific_page
        })
    }

    fn expression_box_text_changed(
        this: &mut ScientificPage,
        event: &TextChangeEvent,
        _: &mut Window,
        cx: &mut Context<Self>,
    ) {
        this.answer = event.new_text.clone();
        cx.notify()
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
                            .child(div().h(px(1.)).mx(px(10.)).bg(theme.border_color))
                            .child(self.expression_box.clone())
                            .child(
                                div()
                                    .text_size(px(25.))
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
                    .child(keypad(cx.listener(
                        |this, event: &KeypadButtonClickEvent, window, cx| {
                            let event = this.expression_box.update(cx, |expression_box, cx| {
                                match event.button.as_str() {
                                    "C" => expression_box.reset(),
                                    "<" => expression_box.backspace(window, cx),
                                    _ => expression_box.type_text(None, event.button.as_str()),
                                }

                                TextChangeEvent {
                                    new_text: expression_box.content.clone(),
                                }
                            });
                            Self::expression_box_text_changed(this, &event, window, cx);
                        },
                    ))),
            )
    }
}
