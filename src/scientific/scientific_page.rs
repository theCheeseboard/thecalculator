use std::sync::{Arc, RwLock};
use std::thread;
use crate::expression_box::{Alignment, ExpressionBox, TextChangeEvent};
use crate::scientific::keypad::{KeypadButtonClickEvent, keypad};
use cntp_i18n::{I18nString, Quote, tr};
use contemporary::components::button::{Button, button};
use contemporary::components::context_menu::ContextMenuItem;
use contemporary::components::layer::layer;
use contemporary::styling::theme::{Theme, ThemeStorage, VariableColor};
use gpui::{AppContext, Context, ElementId, Entity, InteractiveElement, IntoElement, ListAlignment, ListState, ParentElement, Render, SharedString, StatefulInteractiveElement, Styled, TextAlign, Window, div, list, px, rgba, AsyncApp, WeakEntity};
use std::time::{Duration, Instant};
use tcalc::evaluator::eval_result::{
    DomainViolation, EvalError, EvalValue, FactorialDomainViolation, LogarithmDomainViolation,
    NumError, OrdinalDomainViolation,
};
use tcalc::evaluator::{AngleUnit, CancellationTokenSource, Evaluator, Number, Real};
use tcalc::lexer::Lexer;
use tcalc::parser::Parser;
use crate::rwlock_evaluator_extensions::RwlockEvaluatorExtensions;

pub struct ScientificPage {
    selected_angle_unit: Entity<AngleUnit>,
    expression_box: Entity<ExpressionBox>,

    answer: SharedString,
    answer_error_animation_start: Option<Instant>,

    evaluator: Arc<RwLock<Evaluator<Real>>>,
    supplementary: SharedString,
    cancellation_source: CancellationTokenSource,

    history_items: Vec<HistoryItem<Real>>,
    history_list_state: ListState,
}

struct HistoryItem<Num: Number> {
    expression: String,
    result: Num,
}

impl ScientificPage {
    pub fn new(selected_angle_unit: Entity<AngleUnit>, cx: &mut Context<Self>) -> ScientificPage {
        cx.observe(&selected_angle_unit, |this, selected_angle_unit, cx| {
            let selected_angle_unit = selected_angle_unit.read(cx);
            this.evaluator.cancel_evaluation_and_write(&this.cancellation_source).set_angle_unit(selected_angle_unit.clone());
            cx.notify();
        })
        .detach();

        let expression_box_text_changed_listener = cx.listener(Self::expression_box_text_changed);
        let expression_box_commit_listener = cx.listener(|this, _, window, cx| {
            this.equals(window, cx);
        });

        let scientific_page = ScientificPage {
            selected_angle_unit,
            expression_box: ExpressionBox::new(
                cx,
                "",
                tr!("EXPRESSION_PLACEHOLDER", "Expression..."),
                px(30.).into(),
                Alignment::Right,
                expression_box_text_changed_listener,
                expression_box_commit_listener,
            ),
            answer: Default::default(),
            answer_error_animation_start: None,
            evaluator: Arc::new(RwLock::new(Evaluator::new())),
            supplementary: Default::default(),
            cancellation_source: CancellationTokenSource::new(),
            history_items: Vec::new(),
            history_list_state: ListState::new(0, ListAlignment::Bottom, px(200.)),
        };

        scientific_page
    }

    fn expression_box_text_changed(
        &mut self,
        _: &TextChangeEvent,
        window: &mut Window,
        cx: &mut Context<Self>,
    ) {
        cx.defer_in(window, |this, _, cx| {
            this.perform_on_the_fly_calculation(cx)
        });
    }

    fn perform_on_the_fly_calculation(&mut self, cx: &mut Context<Self>) {
        self.cancellation_source.cancel();
        self.cancellation_source = CancellationTokenSource::new();

        // Clear the answer box because the calculation may take some time
        self.answer = Default::default();

        let expression = self.expression_box.read(cx).current_text().to_string();
        if expression.is_empty() {
            self.supplementary = Default::default();
            return;
        }

        // TODO: i18n
        let lexer = Lexer::new(expression, true);
        let mut parser = Parser::new(lexer);
        let statements = parser.parse_all();
        if statements.len() == 1 {
            let statement = statements.first().unwrap().clone();
            let cancellation_token = self.cancellation_source.token();

            let (tx, rx) = async_channel::bounded(1);

            let evaluator = self.evaluator.clone();
            let angle_unit = evaluator.read().unwrap().angle_unit().clone();

            thread::spawn(move || {
                let evaluator = evaluator.write().unwrap();
                let result = evaluator.evaluate(&statement, cancellation_token);
                let _ = tx.send_blocking(result);
            });

            cx.spawn(async move |weak_this: WeakEntity<Self>, cx: &mut AsyncApp| {
                let Ok(result) = rx.recv().await else {
                    return;
                };

                let _ = weak_this.update(cx, |this, cx| {
                    match result {
                        Ok(EvalValue::Numeric(result)) => {
                            let answer = result.to_string();
                            this.answer = answer.clone().into();
                            match result.to_string_truncated_or_less(10) {
                                Ok(approximate_result) => {
                                    if approximate_result == answer {
                                        this.supplementary = Default::default();
                                    } else {
                                        this.supplementary = format!("≈ {approximate_result}").into()
                                    }
                                }
                                Err(_) => this.supplementary = Default::default(),
                            }
                        }
                        Ok(EvalValue::AssignedVariable {
                               variable_name,
                               value,
                           }) => {
                            this.answer = "Assigned Variable".into();
                        }
                        Ok(EvalValue::Comparison(result)) => {
                            this.answer = match result {
                                true => tr!("COMPARISON_RESULT_TRUE", "True").into(),
                                false => tr!("COMPARISON_RESULT_FALSE", "False").into(),
                            };
                            this.supplementary = Default::default();
                        }
                        Err(err) => {
                            this.answer = match err {
                                EvalError::InvalidProgram => Default::default(),
                                _ => eval_error_to_string(&err, &angle_unit).into(),
                            };
                            this.supplementary = Default::default();
                        }
                    }
                });
            }).detach();
        } else {
            self.answer = Default::default();
        }
        cx.notify()
    }

    fn equals(&mut self, window: &mut Window, cx: &mut Context<Self>) {
        self.cancellation_source.cancel();
        self.cancellation_source = CancellationTokenSource::new();

        cx.defer_in(window, |this, window, cx| {
            let expression = this.expression_box.read(cx).current_text().to_string();
            if expression.is_empty() {
                this.answer = Default::default();
                this.trigger_error_animation();
                return;
            }

            // TODO: i18n
            let lexer = Lexer::new(expression.clone(), true);
            let mut parser = Parser::new(lexer);
            let statements = parser.parse_all();
            // TODO: ?
            if statements.len() == 1 {
                let statement = statements.first().unwrap();
                let mut evaluator = this.evaluator.write().unwrap();
                let result = evaluator
                    .evaluate(statement, this.cancellation_source.token());

                match result {
                    Ok(value) => {
                        evaluator.apply_evaluation_effects(value.clone());
                        match value {
                            EvalValue::Numeric(result) => {
                                this.supplementary = Default::default();
                                this.expression_box.update(cx, |expression_box, cx| {
                                    expression_box.set_text(result.to_string().into(), cx);
                                });
                                this.history_items.push(HistoryItem { result, expression });
                                this.history_list_state.reset(this.history_items.len());
                            }
                            EvalValue::AssignedVariable {
                                variable_name,
                                value,
                            } => {
                                this.answer = "Assigned Variable".into();
                            }
                            EvalValue::Comparison(result) => {
                                this.answer = match result {
                                    true => tr!("COMPARISON_RESULT_TRUE", "True").into(),
                                    false => tr!("COMPARISON_RESULT_FALSE", "False").into(),
                                };
                            }
                        }
                    }
                    Err(err) => {
                        this.answer =
                            eval_error_to_string(&err, evaluator.angle_unit()).into();
                        drop(evaluator);

                        this.trigger_error_animation();
                    }
                }
            } else {
                this.answer = "".into();
            }

            cx.notify()
        });
    }

    fn trigger_error_animation(&mut self) {
        self.answer_error_animation_start = Some(Instant::now());
    }

    fn keypad_button_click(
        &mut self,
        event: &KeypadButtonClickEvent,
        window: &mut Window,
        cx: &mut Context<Self>,
    ) {
        if event.button == "=" {
            self.equals(window, cx);
        } else {
            let event = self.expression_box.update(cx, |expression_box, cx| {
                match event.button.as_str() {
                    "C" => expression_box.reset(),
                    "<" => expression_box.backspace(window, cx),
                    "ln" | "log" | "sin" | "cos" | "tan" | "asin" | "acos" | "atan" | "sinh"
                    | "cosh" | "tanh" | "asinh" | "acosh" | "atanh" | "sec" | "csc" | "cot"
                    | "asec" | "acsc" | "acot" | "sech" | "csch" | "coth" | "acoth" | "asech"
                    | "acsch" => {
                        expression_box.type_text(None, format!("{}(", event.button).as_str())
                    }
                    "xⁿ" => expression_box.type_text(None, "^"),
                    "ⁿ√" => (),   // TODO
                    "logₙ" => (), // TODO
                    _ => expression_box.type_text(None, event.button.as_str()),
                }

                TextChangeEvent {
                    new_text: expression_box.content.clone(),
                }
            });
            Self::expression_box_text_changed(self, &event, window, cx);
        }
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

        let selected_angle_unit = self.selected_angle_unit.read(cx);

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
                            .child(
                                list(
                                    self.history_list_state.clone(),
                                    cx.processor(|this, i, _, cx| {
                                        let theme = cx.theme();
                                        let history_item: &HistoryItem<Real> =
                                            &this.history_items[i];
                                        let expression = history_item.expression.clone();
                                        div()
                                            .id(i)
                                            .py(px(2.))
                                            .px(px(8.))
                                            .w_full()
                                            .child(
                                                div()
                                                    .rounded(theme.border_radius)
                                                    .p(px(4.))
                                                    .flex()
                                                    .flex_col()
                                                    .child(
                                                        div()
                                                            .child(history_item.expression.clone()),
                                                    )
                                                    .child(
                                                        div()
                                                            .flex()
                                                            .justify_end()
                                                            .text_size(px(25.))
                                                            .child(format!(
                                                                "= {}",
                                                                history_item.result.to_string()
                                                            )),
                                                    )
                                                    .hover(|david| {
                                                        david.bg(theme.button_background.hover())
                                                    }),
                                            )
                                            .on_click(cx.listener(move |this, _, _, cx| {
                                                let expression = expression.clone();
                                                this.expression_box.update(
                                                    cx,
                                                    move |expression_box, cx| {
                                                        expression_box
                                                            .set_text(expression.into(), cx);
                                                        cx.notify();
                                                    },
                                                );
                                                this.perform_on_the_fly_calculation(cx);
                                            }))
                                            .into_any_element()
                                    }),
                                )
                                .flex_grow(),
                            )
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
                                            .child(match selected_angle_unit {
                                                AngleUnit::Degrees => {
                                                    tr!("TRIG_DEGREES_SHORT", "DEG")
                                                }
                                                AngleUnit::Radians => {
                                                    tr!("TRIG_RADIANS_SHORT", "RAD")
                                                }
                                                AngleUnit::Gradians => {
                                                    tr!("TRIG_GRADIANS_SHORT", "GRAD")
                                                }
                                            })
                                            .with_menu(vec![
                                                ContextMenuItem::menu_item()
                                                    .label(tr!("TRIG_DEGREES"))
                                                    .on_triggered(cx.listener(|this, _, _, cx| {
                                                        this.selected_angle_unit
                                                            .write(cx, AngleUnit::Degrees);
                                                        this.perform_on_the_fly_calculation(cx);
                                                    }))
                                                    .build(),
                                                ContextMenuItem::menu_item()
                                                    .label(tr!("TRIG_RADIANS"))
                                                    .on_triggered(cx.listener(|this, _, _, cx| {
                                                        this.selected_angle_unit
                                                            .write(cx, AngleUnit::Radians);
                                                        this.perform_on_the_fly_calculation(cx);
                                                    }))
                                                    .build(),
                                                ContextMenuItem::menu_item()
                                                    .label(tr!("TRIG_GRADIANS"))
                                                    .on_triggered(cx.listener(|this, _, _, cx| {
                                                        this.selected_angle_unit
                                                            .write(cx, AngleUnit::Gradians);
                                                        this.perform_on_the_fly_calculation(cx);
                                                    }))
                                                    .build(),
                                            ]),
                                    )
                                    .child(
                                        tool_button("output-range")
                                            .child(tr!("RANGE_REAL_SHORT", "REAL")),
                                    )
                                    .child(div().flex_grow())
                                    .child(self.supplementary.clone()),
                            ),
                    )
                    .child(keypad(cx.listener(Self::keypad_button_click))),
            )
    }
}

pub fn eval_error_to_string(error: &EvalError, current_angle_unit: &AngleUnit) -> I18nString {
    match error {
        EvalError::NumberError(NumError::InternalError(_)) => {
            tr!("NUMBER_ERROR_INTERNAL_ERROR", "Internal Error")
        }
        EvalError::NumberError(NumError::OperationCancelledError) => {
            tr!(
                "NUMBER_ERROR_OPERATION_CANCELLED_ERROR",
                "Operation Canceled"
            )
        }
        EvalError::NumberError(NumError::PrecisionOverflow) => {
            tr!("NUMBER_ERROR_PRECISION_OVERFLOW", "Precision Overflow")
        }
        EvalError::NumberError(NumError::DomainViolation(DomainViolation::DivisionByZero)) => {
            tr!("DOMAIN_VIOLATION_DIVISION_BY_ZERO", "Can't divide by zero")
        }
        EvalError::NumberError(NumError::DomainViolation(
            DomainViolation::LogarithmDomainViolation(LogarithmDomainViolation::LogOfNegative),
        )) => {
            tr!(
                "DOMAIN_VIOLATION_LOGARITHM_OF_NEGATIVE",
                "Can't take the logarithm of a negative number"
            )
        }
        EvalError::NumberError(NumError::DomainViolation(
            DomainViolation::LogarithmDomainViolation(LogarithmDomainViolation::LogOfZero),
        )) => {
            tr!(
                "DOMAIN_VIOLATION_LOGARITHM_OF_ZERO",
                "Can't take the logarithm of zero"
            )
        }
        EvalError::NumberError(NumError::DomainViolation(DomainViolation::NthRoot(root))) => {
            // TODO: Which number?
            tr!(
                "DOMAIN_VIOLATION_NTH_ROOT",
                "Can't take the nth root of a number"
            )
        }
        EvalError::NumberError(NumError::DomainViolation(DomainViolation::TanDomainViolation)) => {
            match current_angle_unit {
                AngleUnit::Degrees => {
                    tr!(
                        "DOMAIN_VIOLATION_TAN_DEGREES",
                        "Can't take the tangent of 90 + 180k, k ∈ ℤ"
                    )
                }
                AngleUnit::Radians => {
                    tr!(
                        "DOMAIN_VIOLATION_TAN_RADIANS",
                        "Can't take the tangent of π/2 + πk, k ∈ ℤ"
                    )
                }
                AngleUnit::Gradians => {
                    tr!(
                        "DOMAIN_VIOLATION_TAN_GRADIANS",
                        "Can't take the tangent of 100 + 200k, k ∈ ℤ"
                    )
                }
            }
        }
        EvalError::NumberError(NumError::DomainViolation(DomainViolation::AsinDomainViolation)) => {
            tr!(
                "DOMAIN_VIOLATION_ASIN",
                "Can't take the inverse sine of n < -1, n > 1"
            )
        }
        EvalError::NumberError(NumError::DomainViolation(
            DomainViolation::OrdinalDomainViolation(OrdinalDomainViolation::ZeroBaseZeroOrder),
        )) => {
            // TODO
            tr!(
                "DOMAIN_VIOLATION_ZERO_BASE_ZERO_ORDER",
                "Can't take 0 to the power of 0"
            )
        }
        EvalError::NumberError(NumError::DomainViolation(
            DomainViolation::OrdinalDomainViolation(OrdinalDomainViolation::ZeroBaseNegativeOrder),
        )) => {
            tr!(
                "DOMAIN_VIOLATION_ZERO_BASE_NEGATIVE_ORDER",
                "Can't take 0 to the power of a negative number"
            )
        }
        EvalError::NumberError(NumError::DomainViolation(
            DomainViolation::OrdinalDomainViolation(
                OrdinalDomainViolation::NegativeBaseNonIntegerOrder,
            ),
        )) => {
            tr!(
                "DOMAIN_VIOLATION_ZERO_BASE_NON_INTEGER_ORDER",
                "Can't take a negative number to the power of a non-integer number"
            )
        }
        EvalError::NumberError(NumError::DomainViolation(
            DomainViolation::FactorialDomainViolation(FactorialDomainViolation::NegativeBase),
        )) => {
            tr!(
                "DOMAIN_VIOLATION_NEGATIVE_BASE_FACTORIAL",
                "Can't take the factorial of a negative number"
            )
        }
        EvalError::NumberError(NumError::DomainViolation(
            DomainViolation::FactorialDomainViolation(FactorialDomainViolation::NonIntegerBase),
        )) => {
            tr!(
                "DOMAIN_VIOLATION_NON_INTEGER_BASE_FACTORIAL",
                "Can't take the factorial of a non-integer"
            )
        }
        EvalError::NumberError(NumError::Overflow) => {
            tr!("NUMBER_ERROR_OVERFLOW", "Overflow")
        }
        EvalError::AssignToConstant(name) => {
            tr!(
                "EVAL_ERROR_ASSIGN_TO_CONSTANT",
                "Can't assign to constant {{name}}",
                name:Quote=name
            )
        }
        EvalError::UndefinedVariable(name) => {
            tr!("EVAL_ERROR_UNDEFINED_VARIABLE", "Undefined variable {{name}}", name:Quote=name)
        }
        EvalError::UndefinedFunction(name) => {
            tr!("EVAL_ERROR_UNDEFINED_FUNCTION", "Undefined function {{name}}", name:Quote=name)
        }
        EvalError::InvalidArgumentCount => {
            tr!("EVAL_ERROR_INVALID_ARGUMENT_COUNT", "Invalid arguments")
        }
        EvalError::ComplexInequality => {
            tr!("EVAL_ERROR_COMPLEX_INEQUALITY", "Complex inequality")
        }
        EvalError::InvalidProgram => {
            tr!("EVAL_ERROR_INVALID_PROGRAM", "Syntax Error")
        }
    }
}
