#include "calculatorcontroller.h"

#include <QClipboard>
#include <libcontemporary_global.h>

#include "historymodel.h"
#include <libtcalc/tc_evaluator.h>
#include <libtcalc/tc_lexer.h>
#include <libtcalc/tc_parser.h>

struct CalculatorControllerPrivate {
        QString expressionString;
        QString instantResult;
        int cursorPosition;

        HistoryModel* history;

        tcalc::evaluator evaluator{64};

        int errorStartLocation = 0;
        int errorEndLocation = 0;
};

CalculatorController::CalculatorController(QObject* parent) :
    QObject{parent}, d{new CalculatorControllerPrivate()} {
    d->history = new HistoryModel(this);
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

CalculatorController::TrigonometricUnit CalculatorController::trigonometricUnit() {
    switch (d->evaluator.trig_unit()) {
        case tcalc::angle_unit::radians:
            return CalculatorController::Radians;
        case tcalc::angle_unit::degrees:
            return CalculatorController::Degrees;
        case tcalc::angle_unit::gradians:
            return CalculatorController::Gradians;
    }
}

void CalculatorController::setTrigonometricUnit(TrigonometricUnit unit) {
    tcalc::angle_unit newTrigUnit;
    switch (unit) {
        case Degrees:
            newTrigUnit = tcalc::angle_unit::degrees;
            break;
        case Radians:
            newTrigUnit = tcalc::angle_unit::radians;
            break;
        case Gradians:
            newTrigUnit = tcalc::angle_unit::gradians;
            break;
    }
    d->evaluator.trig_unit(newTrigUnit);
    emit trigonometricUnitChanged();

    expressionStringUpdated();
}

int CalculatorController::errorStartLocation() {
    return d->errorStartLocation;
}

int CalculatorController::errorEndLocation() {
    return d->errorEndLocation;
}

QAbstractItemModel* CalculatorController::history() {
    return d->history;
}

void CalculatorController::performEvaluation() {
    auto fullExpression = d->expressionString + balancingBrackets();
    bool success;
    auto result = evaluateExpression(fullExpression, &success);

    if (!success) {
        emit evaluationError();
        return;
    }

    d->history->addHistoryEntry({fullExpression, result});

    d->expressionString = result;
    d->cursorPosition = d->expressionString.length();
    expressionStringUpdated();

    // Clear the instant result until the input changes again
    d->instantResult.clear();
    emit instantResultChanged();
}

QString CalculatorController::evaluateExpression(QString expression, bool* success) {
    *success = false;
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
            case tcalc::eval_error_type::out_of_tan_domain:
                switch (d->evaluator.trig_unit()) {
                    case tcalc::angle_unit::radians:
                        return tr("Can't tan(π/2 + πk)");
                    case tcalc::angle_unit::degrees:
                        return tr("Can't tan(90 + 180k)");
                    case tcalc::angle_unit::gradians:
                        return tr("Can't tan(100 + 200k)");
                }
            case tcalc::eval_error_type::complex_inequality:
            case tcalc::eval_error_type::none:
                break;
        }
        return tr("Unknown Error");
    }

    d->errorStartLocation = 0;
    d->errorEndLocation = 0;

    if (auto number = std::get_if<tcalc::number>(&result.value())) {
        *success = true;
        return QString::fromStdString(number->string());
    } else if (auto comparisonResult = std::get_if<bool>(&result.value())) {
        *success = true;
        return *comparisonResult ? tr("True") : tr("False");
    } else {
        return tr("Unknown Error");
    }
}

QString CalculatorController::clipboard() {
    return qApp->clipboard()->text();
}

void CalculatorController::clipboardCopy(QString text) {
    qApp->clipboard()->setText(text);
}

void CalculatorController::expressionStringUpdated() {
    emit expressionStringChanged();
    emit cursorPositionChanged();
    emit balancingBracketsChanged();

    calculateIntellisense();

    auto fullExpression = d->expressionString + balancingBrackets();
    bool success;
    d->instantResult = evaluateExpression(fullExpression, &success);
    emit instantResultChanged();
}

void CalculatorController::calculateIntellisense() {
    // Step back until we find a bracket with a function name we understand
}
