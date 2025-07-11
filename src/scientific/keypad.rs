use contemporary::components::button::button;
use contemporary::hsv::Hsva;
use contemporary::styling::theme::Theme;
use gpui::{
    App, IntoElement, ParentElement, RenderOnce, Rgba, SharedString, Styled, Window, div, px,
};

#[derive(IntoElement)]
pub struct Keypad {}

pub fn keypad() -> Keypad {
    Keypad {}
}

impl RenderOnce for Keypad {
    fn render(self, _window: &mut Window, cx: &mut App) -> impl IntoElement {
        let theme = cx.global::<Theme>();
        let keypad_numbers = [
            ["7", "8", "9"],
            ["4", "5", "6"],
            ["1", "2", "3"],
            [".", "0", "Ans"],
        ];

        let mut numbers = div().flex().flex_col().flex_grow().gap(px(2.)).child(
            div()
                .flex()
                .flex_grow()
                .child(
                    button("clear-button")
                        .child("C")
                        .flex_grow()
                        .flex_basis(px(10.)),
                )
                .child({
                    let mut backspace_button = button("backspace-button")
                        .child("<")
                        .flex_grow()
                        .flex_basis(px(10.));
                    backspace_button.style().flex_grow = Some(1.);
                    backspace_button
                }),
        );

        for row in keypad_numbers.iter() {
            let mut row_div = div().flex().flex_grow().gap(px(2.));
            for number in row.iter() {
                row_div = row_div.child(
                    button(SharedString::from(format!("{number}-button")))
                        .child(number.to_string())
                        .flex_grow()
                        .flex_basis(px(10.)),
                );
            }
            numbers = numbers.child(row_div);
        }

        let keypad_operations = [
            ["(", ")", "%"],
            ["π", "e", "i"],
            ["×", "÷", "<<"],
            ["+", "-", ">>"],
        ];

        let mut operations_background: Hsva = theme.button_background.into();
        operations_background = operations_background.darker(2.);

        let mut operations = div()
            .flex()
            .flex_col()
            .flex_grow()
            .bg::<Rgba>(operations_background.into())
            .rounded(theme.border_radius)
            .gap(px(2.));
        for row in keypad_operations.iter() {
            let mut row_div = div().flex().flex_grow().gap(px(2.));
            for number in row.iter() {
                row_div = row_div.child(
                    button(SharedString::from(format!("{number}-button")))
                        .child(number.to_string())
                        .button_color(operations_background)
                        .flex_grow()
                        .flex_basis(px(10.)),
                );
            }
            operations = operations.child(row_div);
        }

        operations = operations.child(
            button("equals-button")
                .child("=")
                .button_color(operations_background)
                .flex_grow(),
        );

        div()
            .flex()
            .bg(theme.button_background)
            .rounded(theme.border_radius)
            .gap(px(2.))
            .child(numbers)
            .child(operations)
    }
}
