#include "calculatorcontroller.h"

#include <libcontemporary_global.h>

#include <libtcalc/tc_evaluator.h>
#include <libtcalc/tc_lexer.h>
#include <libtcalc/tc_parser.h>

struct CalculatorControllerPrivate {
        QString expressionString;
        QString instantResult;
        int cursorPosition;

        tcalc::evaluator evaluator{64};

        int errorStartLocation = 0;
        int errorEndLocation = 0;
};

CalculatorController::CalculatorController(QObject* parent) :
    QObject{parent}, d{new CalculatorControllerPrivate()} {
}

CalculatorController::~CalculatorController() {
    delete d;
}

QString CalculatorController::expressionString() {
    return d->expressionString;
}

void CalculatorController::setExpressionString(QString expressionString) {
    d->expressionString = expressionString;
    expressionStringUpdated();
}

void CalculatorController::clearExpression() {
    d->expressionString.clear();
    d->cursorPosition = 0;
    expressionStringUpdated();
}

void CalculatorController::pressKey(QString key) {
    d->expressionString.insert(d->cursorPosition, key);
    d->cursorPosition += key.length();
    expressionStringUpdated();
}

void CalculatorController::backspace() {
    if (d->cursorPosition == 0) return;

    d->expressionString.removeAt(d->cursorPosition - 1);
    d->cursorPosition -= 1;
    expressionStringUpdated();
}

int CalculatorController::cursorPosition() {
    return d->cursorPosition;
}

void CalculatorController::setCursorPosition(int cursorPosition) {
    d->cursorPosition = cursorPosition;
    emit cursorPositionChanged();
}

void CalculatorController::cursorLeft() {
    if (d->cursorPosition == 0) return;
    d->cursorPosition -= 1;
    emit cursorPositionChanged();
}

void CalculatorController::cursorRight() {
    if (d->cursorPosition == d->expressionString.length()) return;
    d->cursorPosition += 1;
    emit cursorPositionChanged();
}

QString CalculatorController::balancingBrackets() {
    int brackets = 0;
    for (auto c : d->expressionString) {
        if (c == '(') {
            brackets++;
        } else if (c == ')') {
            brackets--;

            // We have mismatched brackets anyway
            if (brackets < 0) brackets = 0;
        }
    }

    return QString().fill(')', brackets);
}

QString CalculatorController::instantResult() {
    return d->instantResult;
}

bool CalculatorController::intellisenseAvailable() {
    return false;
}

QString CalculatorController::intellisenseFunction() {
    return "pow(base, exponent)";
}

QString CalculatorController::intellisenseDescription() {
    return "Describe the pow function";
}

QString CalculatorController::intellisenseArguments() {
    QStringList args;
    args.append("base: the base of the exponent");
    return args.join(libContemporaryCommon::humanReadablePartJoinString());
}

int CalculatorController::errorStartLocation() {
    return d->errorStartLocation;
}

int CalculatorController::errorEndLocation() {
    return d->errorEndLocation;
}

void CalculatorController::performEvaluation() {
    auto fullExpression = d->expressionString + balancingBrackets();
    emit evaluationError();
}

QString CalculatorController::evaluateExpression(QString expression) {
    if (expression.isEmpty()) {
        d->errorStartLocation = 0;
        d->errorEndLocation = 0;
        return {};
    }

    tcalc::lexer lexer(expression.toStdString(), QLocale().decimalPoint() != ",");
    tcalc::parser parser(std::move(lexer), 64);

    auto expr = parser.parse_expression();
    if (!parser.diagnostic_bag().empty()) {
        d->errorStartLocation = 0;
        d->errorEndLocation = 0;
        return tr("Syntax Error");
    }

    auto result = d->evaluator.evaluate(expr);
    if (result.is_error()) {
        d->errorStartLocation = result.error().position.start_index;
        d->errorEndLocation = result.error().position.end_index;
        switch (result.error().type) {
            case tcalc::eval_error_type::invalid_program:
                return tr("Syntax Error");
            case tcalc::eval_error_type::divide_by_zero:
                return tr("Can't divide by zero");
            case tcalc::eval_error_type::log_zero:
                return tr("Can't take the logarithm of zero");
            case tcalc::eval_error_type::undefined_variable:
                return tr("Undefined Variable");
            case tcalc::eval_error_type::undefined_function:
                return tr("Undefined Function");
            case tcalc::eval_error_type::log_base:
                return tr("Can't take a logarithm of base 0 or 1");
            case tcalc::eval_error_type::bad_arity:
                return tr("Bad Arity");
            case tcalc::eval_error_type::complex_inequality:
            case tcalc::eval_error_type::none:
            default:
                return tr("Unknown Error");
        }
    }

    d->errorStartLocation = 0;
    d->errorEndLocation = 0;

    if (auto number = std::get_if<tcalc::number>(&result.value())) {
        return QString::fromStdString(number->string());
    } else if (auto comparisonResult = std::get_if<bool>(&result.value())) {
        return *comparisonResult ? tr("True") : tr("False");
    } else {
        return tr("Unknown Error");
    }
}

void CalculatorController::expressionStringUpdated() {
    emit expressionStringChanged();
    emit cursorPositionChanged();
    emit balancingBracketsChanged();

    calculateIntellisense();

    // TODO: Calculate instant result
    auto fullExpression = d->expressionString + balancingBrackets();
    d->instantResult = evaluateExpression(fullExpression);
    emit instantResultChanged();
}

void CalculatorController::calculateIntellisense() {
    // Step back until we find a bracket with a function name we understand
}
