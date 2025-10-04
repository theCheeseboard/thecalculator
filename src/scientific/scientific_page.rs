use crate::expression_box::{Alignment, ExpressionBox, TextChangeEvent};
use crate::scientific::keypad::{KeypadButtonClickEvent, keypad};
use cntp_i18n::tr;
use contemporary::components::button::{Button, button};
use contemporary::components::dialog_box::StandardButton::No;
use contemporary::components::layer::layer;
use contemporary::styling::theme::Theme;
use gpui::{
    AnimationExt, App, AppContext, Context, ElementId, Entity, EntityInputHandler, IntoElement,
    ParentElement, PromptButton, PromptLevel, Render, SharedString, Styled, TextAlign, Window, div,
    px, rgba,
};
use std::time::{Duration, Instant};

pub struct ScientificPage {
    expression_box: Entity<ExpressionBox>,

    answer: SharedString,
    answer_error_animation_start: Option<Instant>,
}

impl ScientificPage {
    pub fn new(cx: &mut App) -> Entity<ScientificPage> {
        cx.new(|cx| {
            let expression_box_text_changed_listener =
                cx.listener(Self::expression_box_text_changed);
            let expression_box_commit_listener = cx.listener(|this, _, window, cx| {
                this.equals(window, cx);
            });

            let scientific_page = ScientificPage {
                expression_box: ExpressionBox::new(
                    cx,
                    "",
                    tr!("EXPRESSION_PLACEHOLDER", "Expression..."),
                    px(30.).into(),
                    Alignment::Right,
                    expression_box_text_changed_listener,
                    expression_box_commit_listener,
                ),
                answer: "".into(),
                answer_error_animation_start: None,
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

    fn equals(&mut self, window: &mut Window, cx: &mut Context<Self>) {
        self.trigger_error_animation();
        cx.notify()
    }

    fn trigger_error_animation(&mut self) {
        self.answer_error_animation_start = Some(Instant::now());
    }
}

fn tool_button(id: impl Into<ElementId>) -> Button {
    button(id).flat().p(px(3.))
}

impl Render for ScientificPage {
    fn render(&mut self, window: &mut Window, cx: &mut Context<Self>) -> impl IntoElement {
        let theme = cx.global::<Theme>();

        let animation_progress = if let Some(animation_start_time) =
            self.answer_error_animation_start
            && animation_start_time.elapsed() < Duration::from_millis(500)
        {
            window.request_animation_frame();
            1. - animation_start_time.elapsed().as_millis() as f64 / 500.
        } else {
            0.
        };

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
                        layer()
                            .flex()
                            .flex_col()
                            .flex_grow()
                            .child(div().flex_grow())
                            .child(div().h(px(1.)).mx(px(10.)).bg(theme.border_color))
                            .child(self.expression_box.clone())
                            .child(
                                div()
                                    .rounded(theme.border_radius)
                                    .text_size(px(25.))
                                    .text_align(TextAlign::Right)
                                    .child(self.answer.clone())
                                    .bg(rgba(
                                        0xFF000000
                                            + (0xFF as f64 * animation_progress).round() as u32,
                                    )),
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
                            if event.button == "=" {
                                this.equals(window, cx);
                            } else {
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
                            }
                        },
                    ))),
            )
    }
}
