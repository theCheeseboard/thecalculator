#include "calculatorcontroller.h"

#include <QClipboard>
#include <QRegularExpression>
#include <QStack>
#include <libcontemporary_global.h>
#include <ranges/trange.h>

#include "functiondatabase.h"
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

        QString intellisenseFunction;
        FunctionDatabase::Function intellisenseFunctionData;
        int intellisenseCurrentOverload = 0;
        int intellisenseCurrentArgument = 0;
        FunctionDatabase functionDatabase;
        bool intellisenseAutoChangeOverload = true;
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

    calculateIntellisense();
}

void CalculatorController::cursorLeft() {
    if (d->cursorPosition == 0) return;
    d->cursorPosition -= 1;
    emit cursorPositionChanged();

    calculateIntellisense();
}

void CalculatorController::cursorRight() {
    if (d->cursorPosition == d->expressionString.length()) return;
    d->cursorPosition += 1;
    emit cursorPositionChanged();

    calculateIntellisense();
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
    return !d->intellisenseFunction.isEmpty();
}

QString CalculatorController::intellisenseFunction() {
    if (d->intellisenseFunctionData.overloads.count() <= d->intellisenseCurrentOverload) return {};
    auto overload = d->intellisenseFunctionData.overloads.at(d->intellisenseCurrentOverload);

    return QStringLiteral("%1(%2)").arg(d->intellisenseFunctionData.name, tRange(overload.arguments).map<QString>([](FunctionDatabase::Function::Overload::Argument argument) {
        return argument.name;
    }).toList()
                                                                              .join(QLocale().decimalPoint() == "," ? "; " : ", "));
}

QString CalculatorController::intellisenseDescription() {
    if (d->intellisenseFunctionData.overloads.count() <= d->intellisenseCurrentOverload) return {};
    auto overload = d->intellisenseFunctionData.overloads.at(d->intellisenseCurrentOverload);

    return overload.description;
}

QString CalculatorController::intellisenseArguments() {
    if (d->intellisenseFunctionData.overloads.count() <= d->intellisenseCurrentOverload) return {};
    auto overload = d->intellisenseFunctionData.overloads.at(d->intellisenseCurrentOverload);
    auto args = tRange(overload.arguments).map<QString>([this](FunctionDatabase::Function::Overload::Argument argument, int index) {
        if (d->intellisenseCurrentArgument == index) {
            return QStringLiteral("**%1: %2**").arg(argument.name, argument.description);
        } else {
            return argument.name;
        }
    }).toList();
    return args.join(libContemporaryCommon::humanReadablePartJoinString());
}

int CalculatorController::intellisenseCurrentOverload() {
    return d->intellisenseCurrentOverload;
}

int CalculatorController::intellisenseTotalOverloads() {
    return d->intellisenseFunctionData.overloads.length();
}

void CalculatorController::intellisenseNextOverload() {
    if (d->intellisenseCurrentOverload == d->intellisenseFunctionData.overloads.length() - 1) return;
    d->intellisenseCurrentOverload += 1;
    d->intellisenseAutoChangeOverload = false;
    emit intellisenseChanged();
}

void CalculatorController::intellisensePreviousOverload() {
    if (d->intellisenseCurrentOverload == 0) return;
    d->intellisenseCurrentOverload -= 1;
    d->intellisenseAutoChangeOverload = false;
    emit intellisenseChanged();
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

bool CalculatorController::complexMode() {
    return d->evaluator.complex_mode();
}

void CalculatorController::setComplexMode(bool complexMode) {
    d->evaluator.complex_mode(complexMode);
    emit complexModeChanged();

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
    auto result = evaluateExpression(fullExpression, true, false, &success);

    if (!success) {
        // Place the error in the instant result area
        // because the instant result suppresses errors
        d->instantResult = result;
        emit instantResultChanged();
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

QString CalculatorController::evaluateExpression(QString expression, bool commit, bool isInstantResult, bool* success) {
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

        if (isInstantResult) return {};
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
            case tcalc::eval_error_type::out_of_sec_domain:
                switch (d->evaluator.trig_unit()) {
                    case tcalc::angle_unit::radians:
                        return tr("Can't sec(π/2 + πk)");
                    case tcalc::angle_unit::degrees:
                        return tr("Can't sec(90 + 180k)");
                    case tcalc::angle_unit::gradians:
                        return tr("Can't sec(100 + 200k)");
                }
            case tcalc::eval_error_type::out_of_csc_domain:
                switch (d->evaluator.trig_unit()) {
                    case tcalc::angle_unit::radians:
                        return tr("Can't csc(πk)");
                    case tcalc::angle_unit::degrees:
                        return tr("Can't csc(180k)");
                    case tcalc::angle_unit::gradians:
                        return tr("Can't csc(200k)");
                }
            case tcalc::eval_error_type::out_of_cot_domain:
                switch (d->evaluator.trig_unit()) {
                    case tcalc::angle_unit::radians:
                        return tr("Can't cot(πk)");
                    case tcalc::angle_unit::degrees:
                        return tr("Can't cot(180k)");
                    case tcalc::angle_unit::gradians:
                        return tr("Can't cot(200k)");
                }
            case tcalc::eval_error_type::out_of_asec_domain:
                return tr("Can't asec(0)");
            case tcalc::eval_error_type::out_of_acsc_domain:
                return tr("Can't acsc(0)");
            case tcalc::eval_error_type::out_of_csch_domain:
                return tr("Can't csch(0)");
            case tcalc::eval_error_type::out_of_coth_domain:
                return tr("Can't coth(0)");
            case tcalc::eval_error_type::out_of_asech_domain:
                return tr("Can't asech(0)");
            case tcalc::eval_error_type::out_of_acsch_domain:
                return tr("Can't acsch(0)");
            case tcalc::eval_error_type::out_of_acoth_domain:
                return tr("Can't acoth(-1) or acoth(0) or acoth(1)");
            case tcalc::eval_error_type::zero_pow_zero:
                return tr("Can't take 0 to the power of 0");
            case tcalc::eval_error_type::assign_to_constant:
                return tr("Can't assign to a constant value");
            case tcalc::eval_error_type::zero_root:
                return tr("Can't take the zeroth root of a number");
            case tcalc::eval_error_type::real_mode_complex_result:
                return tr("The result of an expression is complex");
            case tcalc::eval_error_type::overflow:
            case tcalc::eval_error_type::nan_error:
                return tr("Overflow");
            case tcalc::eval_error_type::complex_inequality:
            case tcalc::eval_error_type::none:
                break;
        }
        return tr("Unknown Error");
    }

    d->errorStartLocation = 0;
    d->errorEndLocation = 0;

    if (commit) {
        d->evaluator.commit_result(result.value());
    }

    if (auto number = std::get_if<tcalc::number>(&result.value())) {
        *success = true;
        return QString::fromStdString(number->string());
    } else if (auto comparisonResult = std::get_if<bool>(&result.value())) {
        *success = true;
        return *comparisonResult ? tr("True") : tr("False");
    } else if (auto assignResult = std::get_if<tcalc::assign_result>(&result.value())) {
        *success = true;
        if (commit) {
            return QString::fromStdString(assignResult->value.string());
        } else {
            return tr("= to set: %1 = %2").arg(QString::fromStdString(assignResult->variable), QString::fromStdString(assignResult->value.string()));
        }
    } else {
        if (isInstantResult) return {};
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
    d->instantResult = evaluateExpression(fullExpression, false, true, &success);
    emit instantResultChanged();
}

void CalculatorController::calculateIntellisense() {
    // Step back until we find a bracket with a function name we understand
    QString relevantText = d->expressionString.left(d->cursorPosition);

    // Find the previous function
    QRegularExpression regex("\\w+?(?=[⁰¹²³⁴⁵⁶⁷⁸⁹⁺⁻ⁱ]*\\()");
    QRegularExpressionMatchIterator matchIterator = regex.globalMatch(relevantText);

    // Select the appropriate match
    QStack<QString> matchSelector;
    QStack<int> matchPositions;
    QChar lastChar = ' ';
    for (int i = 0; i < relevantText.length(); i++) {
        QChar c = relevantText.at(i);
        if (c == '(') {
            if (matchIterator.hasNext() && QRegularExpression("\\w").match(lastChar).hasMatch()) {
                QRegularExpressionMatch m = matchIterator.next();
                matchSelector.push(m.captured());
                matchPositions.push(m.capturedEnd());
            } else {
                // Push an empty string so it will be popped when it finds )
                matchSelector.push("");
                matchPositions.push(i);
            }
        } else if (c == ')') {
            if (!matchSelector.isEmpty()) {
                matchSelector.pop();
                matchPositions.pop();
                // Otherwise we'll continue and try to get the function anyway
            }
        }

        // Ignore exponents
        if (!QRegularExpression("[⁰¹²³⁴⁵⁶⁷⁸⁹⁺⁻ⁱ]").match(c).hasMatch()) {
            lastChar = c;
        }
    }

    QChar argSep = ',';
    if (QLocale().decimalPoint() == ',') argSep = ';';

    if (!matchSelector.isEmpty()) {
        // We're currently in a function definition
        auto currentFunction = matchSelector.pop();
        auto currentPosition = matchPositions.pop();
        while (currentFunction == "" && !matchSelector.isEmpty()) {
            currentFunction = matchSelector.pop();
            currentPosition = matchPositions.pop();
        }

        if (d->intellisenseFunction != currentFunction) {
            d->intellisenseAutoChangeOverload = true;
            d->intellisenseCurrentOverload = 0;
        }

        if (d->functionDatabase.haveFunction(currentFunction)) {
            // Figure out the current argument
            int currentArgument = 0;

            int bracketCount = -1;
            for (int i = currentPosition; i < relevantText.size(); i++) {
                QChar c = relevantText.at(i);
                if (c == '(') {
                    bracketCount++;
                } else if (c == ')') {
                    bracketCount--;
                    if (bracketCount < 0) break; // Too many closing brackets
                } else if (c == argSep) {
                    if (bracketCount == 0) currentArgument++;
                }
            }

            if (currentArgument != -1) {
                d->intellisenseCurrentArgument = currentArgument;
                d->intellisenseFunctionData = d->functionDatabase.function(currentFunction);
                d->intellisenseFunction = currentFunction;

                if (d->intellisenseAutoChangeOverload) {
                    // Find the first overload with n arguments
                    for (auto i = 0; i < d->intellisenseFunctionData.overloads.length(); i++) {
                        if (d->intellisenseFunctionData.overloads.at(i).arguments.length() > d->intellisenseCurrentArgument) {
                            d->intellisenseCurrentOverload = i;
                            break;
                        }
                    }
                }

                emit intellisenseChanged();
                return;
            }
        }
    }

    d->intellisenseFunction.clear();
    d->intellisenseAutoChangeOverload = true;
    d->intellisenseCurrentOverload = 0;
    emit intellisenseChanged();
}
