use crate::contemporary::button::button;
use crate::contemporary::window::{ContemporaryWindow, PushPop};
use gpui::{
    App, AppContext, Context, Entity, IntoElement, ParentElement, Render, Styled, WeakEntity,
    Window, div, px,
};

pub struct AboutSurface<T>
where
    T: Render,
{
    window: WeakEntity<ContemporaryWindow<T>>,
}

impl<T> AboutSurface<T>
where
    T: Render,
{
    pub fn new(cx: &mut App, window: WeakEntity<ContemporaryWindow<T>>) -> Entity<Self> {
        cx.new(|_| AboutSurface { window })
    }
}

impl<T> Render for AboutSurface<T>
where
    T: Render,
{
    fn render(&mut self, _window: &mut Window, _cx: &mut Context<Self>) -> impl IntoElement {
        let window = self.window.clone();

        div().flex().flex_col().child(
            button("y")
                .child("about page")
                .on_click(move |_, _, cx| window.upgrade().unwrap().pop(cx)),
        )
    }
}
