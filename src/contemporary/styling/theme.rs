use crate::contemporary::styling::contemporary::{ContemporaryDark, ContemporaryTheme};
use gpui::{Global, Pixels, Rgba};

#[cfg(target_os = "macos")]
use crate::contemporary::styling::macos::create_macos_theme;
use crate::contemporary::styling::rgb::{rgb, rgb_tuple};

pub struct Theme {
    pub background: Rgba,
    pub foreground: Rgba,

    pub system_font_family: &'static str,
    pub system_font_size: Pixels,
    pub heading_font_size: Pixels,

    pub button_background: Rgba,
    pub button_foreground: Rgba,
    pub button_hover_background: Rgba,
    pub button_active_background: Rgba,

    pub border_radius: Pixels,
}

impl Default for Theme {
    fn default() -> Self {
        #[cfg(target_os = "macos")]
        {
            return create_macos_theme();
        }

        Self {
            background: ContemporaryDark::BACKGROUND,
            foreground: ContemporaryDark::FOREGROUND,
            system_font_family: ContemporaryDark::SYSTEM_FONT_FAMILY,
            system_font_size: ContemporaryDark::SYSTEM_FONT_SIZE,
            heading_font_size: ContemporaryDark::HEADING_FONT_SIZE,
            button_background: rgb_tuple(0, 50, 150),
            button_foreground: ContemporaryDark::FOREGROUND,
            button_hover_background: rgb_tuple(0, 75, 225),
            button_active_background: rgb_tuple(0, 33, 100),
            border_radius: ContemporaryDark::BORDER_RADIUS,
        }
    }
}

impl Global for Theme {}
