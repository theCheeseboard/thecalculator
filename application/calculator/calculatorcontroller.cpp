#include "calculatorcontroller.h"

#include <libcontemporary_global.h>

struct CalculatorControllerPrivate {
        QString expressionString;
        QString instantResult;
        int cursorPosition;
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

void CalculatorController::performEvaluation() {
    emit evaluationError();
}

void CalculatorController::expressionStringUpdated() {
    emit expressionStringChanged();
    emit cursorPositionChanged();
    emit balancingBracketsChanged();

    calculateIntellisense();

    // TODO: Calculate instant result
    auto fullExpression = d->expressionString + balancingBrackets();
    d->instantResult = fullExpression;
    emit instantResultChanged();
}

void CalculatorController::calculateIntellisense() {
    // Step back until we find a bracket with a function name we understand
}
