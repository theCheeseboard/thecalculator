use contemporary::components::button::button;
use contemporary::hsv::Hsva;
use contemporary::styling::theme::Theme;
use gpui::{
    App, IntoElement, ParentElement, RenderOnce, Rgba, SharedString, Styled, Window, div, px,
};
use std::rc::Rc;

pub struct KeypadButtonClickEvent {
    pub button: String,
}

type KeypadButtonClickHandler = Rc<dyn Fn(&KeypadButtonClickEvent, &mut Window, &mut App)>;

#[derive(IntoElement)]
pub struct Keypad {
    keypad_button_click_handler: KeypadButtonClickHandler,
}

pub fn keypad(
    callback: impl Fn(&KeypadButtonClickEvent, &mut Window, &mut App) + 'static,
) -> Keypad {
    Keypad {
        keypad_button_click_handler: Rc::new(callback),
    }
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

        let clear_button_handler = self.keypad_button_click_handler.clone();
        let backspace_button_handler = self.keypad_button_click_handler.clone();

        let mut numbers = div().flex().flex_col().flex_grow().gap(px(2.)).child(
            div()
                .flex()
                .flex_grow()
                .child(
                    button("clear-button")
                        .child("C")
                        .flex_grow()
                        .flex_basis(px(10.))
                        .on_click(move |_, window, cx| {
                            clear_button_handler(
                                &KeypadButtonClickEvent { button: "C".to_string() },
                                window,
                                cx,
                            )
                        }),
                )
                .child({
                    let mut backspace_button = button("backspace-button")
                        .child("<")
                        .flex_grow()
                        .flex_basis(px(10.))
                        .on_click(move |_, window, cx| {
                            backspace_button_handler(
                                &KeypadButtonClickEvent { button: "<".to_string() },
                                window,
                                cx,
                            )
                        });
                    backspace_button.style().flex_grow = Some(1.);
                    backspace_button
                }),
        );

        for row in keypad_numbers.iter() {
            let mut row_div = div().flex().flex_grow().gap(px(2.));
            for number in row.iter() {
                let button_handler = self.keypad_button_click_handler.clone();
                let button_text = number.to_string();
                row_div = row_div.child(
                    button(SharedString::from(format!("{number}-button")))
                        .child(number.to_string())
                        .flex_grow()
                        .flex_basis(px(10.))
                        .on_click(move |_, window, cx| {
                            button_handler(
                                &KeypadButtonClickEvent { button: button_text.clone() },
                                window,
                                cx,
                            )
                        }),
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
                let button_handler = self.keypad_button_click_handler.clone();
                let button_text = number.to_string();
                
                row_div = row_div.child(
                    button(SharedString::from(format!("{number}-button")))
                        .child(number.to_string())
                        .button_color(operations_background)
                        .flex_grow()
                        .flex_basis(px(10.))
                        .on_click(move |_, window, cx| {
                            button_handler(
                                &KeypadButtonClickEvent { button: button_text.clone() },
                                window,
                                cx,
                            )
                        }),
                );
            }
            operations = operations.child(row_div);
        }

        let equals_button_handler = self.keypad_button_click_handler.clone();
        operations = operations.child(
            button("equals-button")
                .child("=")
                .button_color(operations_background)
                .flex_grow()
                .on_click(move |_, window, cx| {
                    equals_button_handler(
                        &KeypadButtonClickEvent { button: "=".to_string() },
                        window,
                        cx,
                    )
                }),
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
