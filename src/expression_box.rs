use contemporary::styling::theme::{Theme, VariableColor};
use gpui::{
    AbsoluteLength, App, Bounds, ClipboardItem, Context, CursorStyle, ElementId,
    ElementInputHandler, Entity, EntityInputHandler, FocusHandle, Focusable, GlobalElementId, Hsla,
    KeyBinding, LayoutId, MouseButton, MouseDownEvent, MouseMoveEvent, MouseUpEvent, PaintQuad,
    Pixels, Point, Radians, Rgba, ShapedLine, SharedString, Style, TextRun, TextStyle,
    TextStyleRefinement, UTF16Selection, UnderlineStyle, Window, actions, div, fill, point,
    prelude::*, px, relative, rgb, size,
};
use std::any::Any;
use std::ops::Range;
use std::rc::Rc;
use unicode_segmentation::*;

actions!(
    expression_box,
    [
        Backspace,
        Delete,
        Left,
        Right,
        SelectLeft,
        SelectRight,
        SelectAll,
        Home,
        End,
        ShowCharacterPalette,
        Paste,
        Cut,
        Copy,
        Clear,
        Commit,
        Pi,
        Radical
    ]
);

pub fn bind_expression_box_keys(cx: &mut App) {
    cx.bind_keys([
        KeyBinding::new("backspace", Backspace, None),
        KeyBinding::new("delete", Delete, None),
        KeyBinding::new("left", Left, None),
        KeyBinding::new("right", Right, None),
        KeyBinding::new("shift-left", SelectLeft, None),
        KeyBinding::new("shift-right", SelectRight, None),
        KeyBinding::new("secondary-a", SelectAll, None),
        KeyBinding::new("secondary-v", Paste, None),
        KeyBinding::new("secondary-c", Copy, None),
        KeyBinding::new("secondary-x", Cut, None),
        KeyBinding::new("home", Home, None),
        KeyBinding::new("end", End, None),
        KeyBinding::new("ctrl-cmd-space", ShowCharacterPalette, None),
        KeyBinding::new("escape", Clear, None),
        KeyBinding::new("enter", Commit, None),
        KeyBinding::new("secondary-p", Pi, None),
        KeyBinding::new("secondary-r", Radical, None),
    ]);
}

pub enum Alignment {
    Left,
    Right,
}

pub struct TextChangeEvent {
    pub new_text: SharedString,
}

pub struct CommitEvent {}

type TextChangeEventHandler = Rc<dyn Fn(&TextChangeEvent, &mut Window, &mut App)>;
type CommitEventHandler = Rc<dyn Fn(&CommitEvent, &mut Window, &mut App)>;

pub struct ExpressionBox {
    focus_handle: FocusHandle,
    pub content: SharedString,
    placeholder: SharedString,
    selected_range: Range<usize>,
    selection_reversed: bool,
    marked_range: Option<Range<usize>>,
    last_layout: Option<ShapedLine>,
    last_bounds: Option<Bounds<Pixels>>,
    alignment: Alignment,
    text_size: AbsoluteLength,
    is_selecting: bool,
    pub error_range: Option<Range<usize>>,

    text_change_event: TextChangeEventHandler,
    commit_event: CommitEventHandler,
}

impl ExpressionBox {
    pub fn new(
        cx: &mut App,
        default_text: impl Into<SharedString>,
        placeholder: impl Into<SharedString>,
        text_size: AbsoluteLength,
        alignment: Alignment,
        text_change_event: impl Fn(&TextChangeEvent, &mut Window, &mut App) + 'static,
        commit_event: impl Fn(&CommitEvent, &mut Window, &mut App) + 'static,
    ) -> Entity<Self> {
        cx.new(|cx| ExpressionBox {
            focus_handle: cx.focus_handle(),
            content: default_text.into(),
            placeholder: placeholder.into(),
            selected_range: 0..0,
            selection_reversed: false,
            marked_range: None,
            last_layout: None,
            last_bounds: None,
            is_selecting: false,
            text_size,
            alignment,
            error_range: None,
            text_change_event: Rc::new(text_change_event),
            commit_event: Rc::new(commit_event),
        })
    }
}

impl ExpressionBox {
    pub fn current_text(&self) -> SharedString {
        self.content.clone()
    }

    pub fn set_text(&mut self, new_text: SharedString, cx: &mut Context<Self>) {
        self.reset();
        self.content = new_text;
        cx.notify();
    }

    pub fn left(&mut self, _: &Left, _: &mut Window, cx: &mut Context<Self>) {
        if self.selected_range.is_empty() {
            self.move_to(self.previous_boundary(self.cursor_offset()), cx);
        } else {
            self.move_to(self.selected_range.start, cx)
        }
    }

    pub fn right(&mut self, _: &Right, _: &mut Window, cx: &mut Context<Self>) {
        if self.selected_range.is_empty() {
            self.move_to(self.next_boundary(self.selected_range.end), cx);
        } else {
            self.move_to(self.selected_range.end, cx)
        }
    }

    pub fn select_left(&mut self, _: &SelectLeft, _: &mut Window, cx: &mut Context<Self>) {
        self.select_to(self.previous_boundary(self.cursor_offset()), cx);
    }

    pub fn select_right(&mut self, _: &SelectRight, _: &mut Window, cx: &mut Context<Self>) {
        self.select_to(self.next_boundary(self.cursor_offset()), cx);
    }

    pub fn select_all(&mut self, _: &SelectAll, _: &mut Window, cx: &mut Context<Self>) {
        self.move_to(0, cx);
        self.select_to(self.content.len(), cx)
    }

    pub fn home(&mut self, _: &Home, _: &mut Window, cx: &mut Context<Self>) {
        self.move_to(0, cx);
    }

    pub fn end(&mut self, _: &End, _: &mut Window, cx: &mut Context<Self>) {
        self.move_to(self.content.len(), cx);
    }

    pub fn handle_backspace(&mut self, _: &Backspace, window: &mut Window, cx: &mut Context<Self>) {
        self.backspace(window, cx);
        (self.text_change_event)(
            &TextChangeEvent {
                new_text: self.content.clone(),
            },
            window,
            cx,
        );
        cx.notify();
    }

    pub fn backspace(&mut self, _: &mut Window, cx: &mut Context<Self>) {
        if self.selected_range.is_empty() {
            self.select_to(self.previous_boundary(self.cursor_offset()), cx)
        }
        self.type_text(None, "")
    }

    pub fn delete(&mut self, _: &Delete, window: &mut Window, cx: &mut Context<Self>) {
        if self.selected_range.is_empty() {
            self.select_to(self.next_boundary(self.cursor_offset()), cx)
        }
        self.replace_text_in_range(None, "", window, cx)
    }

    pub fn insert_pi(&mut self, _: &Pi, window: &mut Window, cx: &mut Context<Self>) {
        if self.selected_range.is_empty() {
            self.select_to(self.next_boundary(self.cursor_offset()), cx)
        }
        self.replace_text_in_range(None, "π", window, cx);

        (self.text_change_event)(
            &TextChangeEvent {
                new_text: self.content.clone(),
            },
            window,
            cx,
        );
        cx.notify();
    }

    pub fn insert_radical(&mut self, _: &Radical, window: &mut Window, cx: &mut Context<Self>) {
        if self.selected_range.is_empty() {
            self.select_to(self.next_boundary(self.cursor_offset()), cx)
        }
        self.replace_text_in_range(None, "√", window, cx);

        (self.text_change_event)(
            &TextChangeEvent {
                new_text: self.content.clone(),
            },
            window,
            cx,
        );
        cx.notify();
    }

    pub fn on_mouse_down(
        &mut self,
        event: &MouseDownEvent,
        _window: &mut Window,
        cx: &mut Context<Self>,
    ) {
        self.is_selecting = true;

        if event.modifiers.shift {
            self.select_to(self.index_for_mouse_position(event.position), cx);
        } else {
            self.move_to(self.index_for_mouse_position(event.position), cx)
        }
    }

    pub fn on_mouse_up(&mut self, _: &MouseUpEvent, _window: &mut Window, _: &mut Context<Self>) {
        self.is_selecting = false;
    }

    pub fn on_mouse_move(
        &mut self,
        event: &MouseMoveEvent,
        _: &mut Window,
        cx: &mut Context<Self>,
    ) {
        if self.is_selecting {
            self.select_to(self.index_for_mouse_position(event.position), cx);
        }
    }

    pub fn show_character_palette(
        &mut self,
        _: &ShowCharacterPalette,
        window: &mut Window,
        _: &mut Context<Self>,
    ) {
        window.show_character_palette();
    }

    pub fn paste(&mut self, _: &Paste, window: &mut Window, cx: &mut Context<Self>) {
        if let Some(text) = cx.read_from_clipboard().and_then(|item| item.text()) {
            self.replace_text_in_range(None, &text.replace("\n", " "), window, cx);
        }
    }

    pub fn copy(&mut self, _: &Copy, _: &mut Window, cx: &mut Context<Self>) {
        if !self.selected_range.is_empty() {
            cx.write_to_clipboard(ClipboardItem::new_string(
                (self.content[self.selected_range.clone()]).to_string(),
            ));
        }
    }
    pub fn cut(&mut self, _: &Cut, window: &mut Window, cx: &mut Context<Self>) {
        if !self.selected_range.is_empty() {
            cx.write_to_clipboard(ClipboardItem::new_string(
                (self.content[self.selected_range.clone()]).to_string(),
            ));
            self.replace_text_in_range(None, "", window, cx)
        }
    }

    pub fn move_to(&mut self, offset: usize, cx: &mut Context<Self>) {
        self.selected_range = offset..offset;
        cx.notify()
    }

    pub fn cursor_offset(&self) -> usize {
        if self.selection_reversed {
            self.selected_range.start
        } else {
            self.selected_range.end
        }
    }

    pub fn index_for_mouse_position(&self, position: Point<Pixels>) -> usize {
        if self.content.is_empty() {
            return 0;
        }

        let (Some(bounds), Some(line)) = (self.last_bounds.as_ref(), self.last_layout.as_ref())
        else {
            return 0;
        };
        if position.y < bounds.top() {
            return 0;
        }
        if position.y > bounds.bottom() {
            return self.content.len();
        }

        line.closest_index_for_x(
            position.x
                - match self.alignment {
                    Alignment::Left => bounds.left(),
                    Alignment::Right => bounds.right() - line.width,
                },
        )
    }

    pub fn select_to(&mut self, offset: usize, cx: &mut Context<Self>) {
        if self.selection_reversed {
            self.selected_range.start = offset
        } else {
            self.selected_range.end = offset
        };
        if self.selected_range.end < self.selected_range.start {
            self.selection_reversed = !self.selection_reversed;
            self.selected_range = self.selected_range.end..self.selected_range.start;
        }
        cx.notify()
    }

    pub fn offset_from_utf16(&self, offset: usize) -> usize {
        let mut utf8_offset = 0;
        let mut utf16_count = 0;

        for ch in self.content.chars() {
            if utf16_count >= offset {
                break;
            }
            utf16_count += ch.len_utf16();
            utf8_offset += ch.len_utf8();
        }

        utf8_offset
    }

    pub fn offset_to_utf16(&self, offset: usize) -> usize {
        let mut utf16_offset = 0;
        let mut utf8_count = 0;

        for ch in self.content.chars() {
            if utf8_count >= offset {
                break;
            }
            utf8_count += ch.len_utf8();
            utf16_offset += ch.len_utf16();
        }

        utf16_offset
    }

    pub fn range_to_utf16(&self, range: &Range<usize>) -> Range<usize> {
        self.offset_to_utf16(range.start)..self.offset_to_utf16(range.end)
    }

    pub fn range_from_utf16(&self, range_utf16: &Range<usize>) -> Range<usize> {
        self.offset_from_utf16(range_utf16.start)..self.offset_from_utf16(range_utf16.end)
    }

    pub fn previous_boundary(&self, offset: usize) -> usize {
        let balancing_brackets = ")".repeat(self.balancing_brackets());

        format!("{}{balancing_brackets}", self.content)
            .grapheme_indices(true)
            .rev()
            .find_map(|(idx, _)| (idx < offset).then_some(idx))
            .unwrap_or(0)
    }

    pub fn next_boundary(&self, offset: usize) -> usize {
        let balancing_brackets = ")".repeat(self.balancing_brackets());

        format!("{}{balancing_brackets}", self.content)
            .grapheme_indices(true)
            .find_map(|(idx, _)| (idx > offset).then_some(idx))
            .unwrap_or(self.content.len())
    }

    pub fn handle_clear(&mut self, _: &Clear, window: &mut Window, cx: &mut Context<Self>) {
        self.reset();
        (self.text_change_event)(
            &TextChangeEvent {
                new_text: self.content.clone(),
            },
            window,
            cx,
        );
        cx.notify();
    }

    pub fn reset(&mut self) {
        self.content = "".into();
        self.selected_range = 0..0;
        self.selection_reversed = false;
        self.marked_range = None;
        self.last_layout = None;
        self.last_bounds = None;
        self.is_selecting = false;
    }

    fn text_style(&self, window: &Window) -> TextStyle {
        let mut refined = TextStyleRefinement::default();
        refined.font_size = Some(self.text_size);
        window.text_style().refined(refined)
    }

    fn balancing_brackets(&self) -> usize {
        let mut count: usize = 0;
        for ch in self.content.chars() {
            if ch == '(' {
                count += 1;
            } else if ch == ')' {
                if let Some(checked_count) = count.checked_sub(1) {
                    count = checked_count
                } else {
                    return 0;
                }
            }
        }
        count
    }

    fn replace_input_text(&mut self, text: &str) -> String {
        text.replace("*", "×").replace(" ", "×").replace("/", "÷")
    }

    pub fn type_text(&mut self, range_utf16: Option<Range<usize>>, new_text: &str) {
        let new_text = self.replace_input_text(new_text);
        let range = range_utf16
            .as_ref()
            .map(|range_utf16| self.range_from_utf16(range_utf16))
            .or(self.marked_range.clone())
            .unwrap_or(self.selected_range.clone());

        if self.content.len() < range.start {
            let missing_brackets = range.start - self.content.len();
            self.content = format!("{}{}", self.content, ")".repeat(missing_brackets)).into();
        }
        self.content = (self.content[0..range.start].to_owned()
            + new_text.as_str()
            + &self.content[range.end..])
            .into();
        self.selected_range = range.start + new_text.len()..range.start + new_text.len();
        self.marked_range.take();
    }

    pub fn handle_commit(&mut self, _: &Commit, window: &mut Window, cx: &mut Context<Self>) {
        (self.commit_event)(&CommitEvent {}, window, cx);
    }
}

impl EntityInputHandler for ExpressionBox {
    fn text_for_range(
        &mut self,
        range_utf16: Range<usize>,
        actual_range: &mut Option<Range<usize>>,
        _window: &mut Window,
        _cx: &mut Context<Self>,
    ) -> Option<String> {
        let range = self.range_from_utf16(&range_utf16);
        actual_range.replace(self.range_to_utf16(&range));
        Some(self.content[range].to_string())
    }

    fn selected_text_range(
        &mut self,
        _ignore_disabled_input: bool,
        _window: &mut Window,
        _cx: &mut Context<Self>,
    ) -> Option<UTF16Selection> {
        Some(UTF16Selection {
            range: self.range_to_utf16(&self.selected_range),
            reversed: self.selection_reversed,
        })
    }

    fn marked_text_range(
        &self,
        _window: &mut Window,
        _cx: &mut Context<Self>,
    ) -> Option<Range<usize>> {
        self.marked_range
            .as_ref()
            .map(|range| self.range_to_utf16(range))
    }

    fn unmark_text(&mut self, _window: &mut Window, _cx: &mut Context<Self>) {
        self.marked_range = None;
    }

    fn replace_text_in_range(
        &mut self,
        range_utf16: Option<Range<usize>>,
        new_text: &str,
        window: &mut Window,
        cx: &mut Context<Self>,
    ) {
        self.type_text(range_utf16, new_text);

        (self.text_change_event)(
            &TextChangeEvent {
                new_text: self.content.clone(),
            },
            window,
            cx,
        );
        cx.notify();
    }

    fn replace_and_mark_text_in_range(
        &mut self,
        range_utf16: Option<Range<usize>>,
        new_text: &str,
        new_selected_range_utf16: Option<Range<usize>>,
        _window: &mut Window,
        cx: &mut Context<Self>,
    ) {
        let new_text = self.replace_input_text(new_text);
        let range = range_utf16
            .as_ref()
            .map(|range_utf16| self.range_from_utf16(range_utf16))
            .or(self.marked_range.clone())
            .unwrap_or(self.selected_range.clone());

        self.content = (self.content[0..range.start].to_owned()
            + new_text.as_str()
            + &self.content[range.end..])
            .into();
        if !new_text.is_empty() {
            self.marked_range = Some(range.start..range.start + new_text.len());
        } else {
            self.marked_range = None;
        }
        self.selected_range = new_selected_range_utf16
            .as_ref()
            .map(|range_utf16| self.range_from_utf16(range_utf16))
            .map(|new_range| new_range.start + range.start..new_range.end + range.end)
            .unwrap_or_else(|| range.start + new_text.len()..range.start + new_text.len());

        cx.notify();
    }

    fn bounds_for_range(
        &mut self,
        range_utf16: Range<usize>,
        bounds: Bounds<Pixels>,
        _window: &mut Window,
        _cx: &mut Context<Self>,
    ) -> Option<Bounds<Pixels>> {
        let last_layout = self.last_layout.as_ref()?;
        let range = self.range_from_utf16(&range_utf16);
        Some(Bounds::from_corners(
            point(
                bounds.left() + last_layout.x_for_index(range.start),
                bounds.top(),
            ),
            point(
                bounds.left() + last_layout.x_for_index(range.end),
                bounds.bottom(),
            ),
        ))
    }

    fn character_index_for_point(
        &mut self,
        point: Point<Pixels>,
        _window: &mut Window,
        _cx: &mut Context<Self>,
    ) -> Option<usize> {
        let line_point = self.last_bounds?.localize(&point)?;
        let last_layout = self.last_layout.as_ref()?;

        assert_eq!(last_layout.text, self.content);
        let utf8_index = last_layout.index_for_x(point.x - line_point.x)?;
        Some(self.offset_to_utf16(utf8_index))
    }
}

struct ExpressionBoxTextElement {
    input: Entity<ExpressionBox>,
}

struct PrepaintState {
    line: Option<ShapedLine>,
    cursor: Option<PaintQuad>,
    selection: Option<PaintQuad>,
    origin: Point<Pixels>,
}

impl IntoElement for ExpressionBoxTextElement {
    type Element = Self;

    fn into_element(self) -> Self::Element {
        self
    }
}

impl Element for ExpressionBoxTextElement {
    type RequestLayoutState = ();
    type PrepaintState = PrepaintState;

    fn id(&self) -> Option<ElementId> {
        None
    }

    fn source_location(&self) -> Option<&'static core::panic::Location<'static>> {
        None
    }

    fn request_layout(
        &mut self,
        _id: Option<&GlobalElementId>,
        _inspector_id: Option<&gpui::InspectorElementId>,
        window: &mut Window,
        cx: &mut App,
    ) -> (LayoutId, Self::RequestLayoutState) {
        let input = self.input.read(cx);
        let mut style = Style::default();
        style.size.width = relative(1.).into();
        style.size.height = input
            .text_style(window)
            .line_height_in_pixels(window.rem_size())
            .into();
        (window.request_layout(style, [], cx), ())
    }

    fn prepaint(
        &mut self,
        _id: Option<&GlobalElementId>,
        _inspector_id: Option<&gpui::InspectorElementId>,
        bounds: Bounds<Pixels>,
        _request_layout: &mut Self::RequestLayoutState,
        window: &mut Window,
        cx: &mut App,
    ) -> Self::PrepaintState {
        let input = self.input.read(cx);
        let content = input.content.clone();
        let selected_range = input.selected_range.clone();
        let cursor = input.cursor_offset();
        let style = window.text_style();
        let theme = cx.global::<Theme>();

        let balancing_brackets = ")".repeat(input.balancing_brackets());

        let (display_text, text_color) = if content.is_empty() {
            (
                input.placeholder.clone(),
                Hsla::from(theme.foreground.disabled()),
            )
        } else {
            (content.clone(), style.color)
        };

        let final_text = format!("{}{balancing_brackets}", display_text).into();

        let run = TextRun {
            len: display_text.len(),
            font: style.font(),
            color: text_color,
            background_color: None,
            underline: None,
            strikethrough: None,
        };
        let mut runs = if let Some(marked_range) = input.marked_range.as_ref() {
            vec![
                TextRun {
                    len: marked_range.start,
                    ..run.clone()
                },
                TextRun {
                    len: marked_range.end - marked_range.start,
                    underline: Some(UnderlineStyle {
                        color: Some(run.color),
                        thickness: px(1.0),
                        wavy: false,
                    }),
                    ..run.clone()
                },
                TextRun {
                    len: display_text.len() - marked_range.end,
                    ..run.clone()
                },
            ]
            .into_iter()
            .filter(|run| run.len > 0)
            .collect()
        } else if let Some(error_range) = input.error_range.as_ref() {
            vec![
                TextRun {
                    len: error_range.start,
                    ..run.clone()
                },
                TextRun {
                    len: error_range.end - error_range.start,
                    underline: Some(UnderlineStyle {
                        color: Some(rgb(0xFF0000).into()),
                        thickness: px(1.0),
                        wavy: true,
                    }),
                    ..run.clone()
                },
                TextRun {
                    len: display_text.len().checked_sub(error_range.end).unwrap_or(0),
                    ..run.clone()
                },
            ]
            .into_iter()
            .filter(|run| run.len > 0)
            .collect()
        } else {
            vec![run]
        };

        // Add the runs for the balancing brackets
        runs.push(TextRun {
            len: balancing_brackets.len(),
            font: style.font(),
            color: theme.foreground.disabled().into(),
            background_color: None,
            underline: None,
            strikethrough: None,
        });

        let font_size = input.text_size.to_pixels(window.rem_size());
        let line = window
            .text_system()
            .shape_line(final_text, font_size, &runs, None);

        let origin = match input.alignment {
            Alignment::Left => bounds.left(),
            Alignment::Right => bounds.right() - line.width,
        };

        let cursor_pos = line.x_for_index(cursor);
        let (selection, cursor) = if selected_range.is_empty() {
            (
                None,
                Some(fill(
                    Bounds::new(
                        point(origin + cursor_pos, bounds.top()),
                        size(px(1.), bounds.bottom() - bounds.top()),
                    ),
                    theme.foreground,
                )),
            )
        } else {
            (
                Some(fill(
                    Bounds::from_corners(
                        point(
                            origin + line.x_for_index(selected_range.start),
                            bounds.top(),
                        ),
                        point(
                            origin + line.x_for_index(selected_range.end),
                            bounds.bottom(),
                        ),
                    ),
                    Rgba {
                        a: 0.5,
                        ..theme.button_background
                    },
                )),
                None,
            )
        };
        PrepaintState {
            origin: Point {
                x: origin,
                y: bounds.top(),
            },
            line: Some(line),
            cursor,
            selection,
        }
    }

    fn paint(
        &mut self,
        _id: Option<&GlobalElementId>,
        _inspector_id: Option<&gpui::InspectorElementId>,
        bounds: Bounds<Pixels>,
        _request_layout: &mut Self::RequestLayoutState,
        prepaint: &mut Self::PrepaintState,
        window: &mut Window,
        cx: &mut App,
    ) {
        let input = self.input.read(cx);
        let focus_handle = input.focus_handle.clone();
        window.handle_input(
            &focus_handle,
            ElementInputHandler::new(bounds, self.input.clone()),
            cx,
        );
        if let Some(selection) = prepaint.selection.take() {
            window.paint_quad(selection)
        }
        let line = prepaint.line.take().unwrap();
        line.paint(
            prepaint.origin,
            input
                .text_style(window)
                .line_height_in_pixels(window.rem_size()),
            window,
            cx,
        )
        .unwrap();

        if let Some(cursor) = prepaint.cursor.take() {
            window.paint_quad(cursor);
        }

        self.input.update(cx, |input, _cx| {
            input.last_layout = Some(line);
            input.last_bounds = Some(bounds);
        });
    }
}

impl Render for ExpressionBox {
    fn render(&mut self, _window: &mut Window, cx: &mut Context<Self>) -> impl IntoElement {
        div()
            .flex()
            .key_context("TextInput")
            .track_focus(&self.focus_handle(cx))
            .cursor(CursorStyle::IBeam)
            .on_action(cx.listener(Self::handle_backspace))
            .on_action(cx.listener(Self::delete))
            .on_action(cx.listener(Self::left))
            .on_action(cx.listener(Self::right))
            .on_action(cx.listener(Self::select_left))
            .on_action(cx.listener(Self::select_right))
            .on_action(cx.listener(Self::select_all))
            .on_action(cx.listener(Self::home))
            .on_action(cx.listener(Self::end))
            .on_action(cx.listener(Self::show_character_palette))
            .on_action(cx.listener(Self::paste))
            .on_action(cx.listener(Self::cut))
            .on_action(cx.listener(Self::copy))
            .on_action(cx.listener(Self::handle_clear))
            .on_action(cx.listener(Self::handle_commit))
            .on_action(cx.listener(Self::insert_pi))
            .on_action(cx.listener(Self::insert_radical))
            .on_mouse_down(MouseButton::Left, cx.listener(Self::on_mouse_down))
            .on_mouse_up(MouseButton::Left, cx.listener(Self::on_mouse_up))
            .on_mouse_up_out(MouseButton::Left, cx.listener(Self::on_mouse_up))
            .on_mouse_move(cx.listener(Self::on_mouse_move))
            .child(div().w_full().p(px(4.)).child(ExpressionBoxTextElement {
                input: cx.entity().clone(),
            }))
    }
}

impl Focusable for ExpressionBox {
    fn focus_handle(&self, _: &App) -> FocusHandle {
        self.focus_handle.clone()
    }
}
