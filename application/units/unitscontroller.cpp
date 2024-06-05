#include "unitscontroller.h"

#include <libtcalc/tc_evaluator.h>
#include <libtcalc/tc_lexer.h>
#include <libtcalc/tc_parser.h>

struct UnitsControllerPrivate {
    tcalc::evaluator evaluator{64};
};

UnitsController::UnitsController(QObject* parent) :
    QObject{parent}, d{new UnitsControllerPrivate()} {
}

UnitsController::~UnitsController() {
    delete d;
}

QString UnitsController::evaluate(QString expression, QString x, bool forward) {
    if (!expression.contains("x")) {
        if (forward) {
            if (expression.endsWith("+")) {
                expression = expression.replace("+", "-x");
            } else {
                expression = QStringLiteral("(x)(%1)^-1").arg(expression);
            }
        } else {
            expression = expression.append("x");
        }
    }

    tcalc::lexer lexer(expression.toStdString(), QLocale().decimalPoint() != ",");
    tcalc::parser parser(std::move(lexer), 64);

    auto expr = parser.parse_expression();
    if (!parser.diagnostic_bag().empty()) {
        return tr("Error");
    }

    [[maybe_unused]]
    auto setXResult = d->evaluator.evaluate(tcalc::parser(tcalc::lexer(QStringLiteral("x=%1").arg(x).toStdString(), true), 64).parse_expression());
    if (setXResult.is_error()) {
        return tr("Error");
    }
    d->evaluator.commit_result(setXResult.value());

    auto result = d->evaluator.evaluate(expr);
    if (result.is_error()) {
        return tr("Error");
    }

    if (auto number = std::get_if<tcalc::number>(&result.value())) {
        return QString::fromStdString(number->string());
    } else {
        return tr("Error");
    }
}
