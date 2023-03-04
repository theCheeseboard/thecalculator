/****************************************
 *
 *   theCalculator - Calculator
 *   Copyright (C) 2019 Victor Tran
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * *************************************/

#include "bisonflexevaluationengine.h"
#include "evaluationengineheaders.h"

typedef void* yyscan_t;
#include "calc.bison.h"
#include "calc.yy.h"

#include <QCoroFuture>
#include <QSettings>
#include <QtConcurrent>

struct BisonFlexEvaluationEnginePrivate {
        QString expression;
        QMap<QString, idouble> variables;
        CustomFunctionMap customFunctions;

        static int runningEngines;
        static QMutex* runningEnginesLocker;
};

int BisonFlexEvaluationEnginePrivate::runningEngines = 0;
QMutex* BisonFlexEvaluationEnginePrivate::runningEnginesLocker = new QMutex();

QString numberFormatToString(long double number) {
    std::stringstream stream;
    stream << std::setprecision(10) << number;

    QString str = QString::fromStdString(stream.str());
    if (QLocale().decimalPoint() == ',') {
        // Replace all decimals with commas
        str.replace(".", ",");
    }
    return str;
}

QString idbToString(idouble db) {
    long double real = db.real();
    long double imag = db.imag();
    if (real != 0 && imag == 0) {
        return numberFormatToString(real);
    } else if (real == 0 && imag == 0) {
        return "0";
    } else if (real != 0 && imag == 1) {
        return numberFormatToString(real) + " + i";
    } else if (real != 0 && imag > 0) {
        return numberFormatToString(real) + " + " + numberFormatToString(imag) + "i";
    } else if (real != 0 && imag == -1) {
        return numberFormatToString(real) + " - i";
    } else if (real != 0 && imag < 0) {
        return numberFormatToString(real) + " - " + numberFormatToString(-imag) + "i";
    } else if (imag == 1) {
        return "i";
    } else if (imag == -1) {
        return "-i";
    } else {
        return numberFormatToString(imag) + "i";
    }
}

struct CustomFunctionPrivate {
        CustomFunctionDefinition fn;
        QStringList desc;
        QList<QStringList> args;
        bool waitingForDocs = false;
};

BisonFlexCustomFunction::BisonFlexCustomFunction() {
    d = QSharedPointer<CustomFunctionPrivate>(new CustomFunctionPrivate());
}

BisonFlexCustomFunction::BisonFlexCustomFunction(CustomFunctionDefinition fn) {
    d = QSharedPointer<CustomFunctionPrivate>(new CustomFunctionPrivate());
    d->fn = fn;
    d->desc.append("A function");
    d->args.append(QStringList());
    d->waitingForDocs = true;
}

BisonFlexCustomFunction::BisonFlexCustomFunction(CustomFunctionDefinition function, QString desc, QStringList args) {
    d = QSharedPointer<CustomFunctionPrivate>(new CustomFunctionPrivate());
    d->fn = function;
    d->desc.append(desc);
    d->args.append(args);
}

CustomFunctionDefinition BisonFlexCustomFunction::getFunction() const {
    return d->fn;
}

QString BisonFlexCustomFunction::getDescription(int overload) const {
    return d->desc.at(overload);
}

QStringList BisonFlexCustomFunction::getArgs(int overload) const {
    return d->args.at(overload);
}

void BisonFlexCustomFunction::addOverload(QString desc, QStringList args) {
    if (d->waitingForDocs) {
        d->desc.clear();
        d->args.clear();
        d->waitingForDocs = false;
    }
    d->desc.append(desc);
    d->args.append(args);
}

int BisonFlexCustomFunction::overloads() {
    return d->args.count();
}

BisonFlexEvaluationEngine::BisonFlexEvaluationEngine(QObject* parent) :
    BaseEvaluationEngine(parent) {
    QT_TR_NOOP("%1: unknown variable");
    QT_TR_NOOP("div: division by 0 undefined");
    QT_TR_NOOP("%1: undefined function");

    d = new BisonFlexEvaluationEnginePrivate();
    this->setupFunctions();
}

BisonFlexEvaluationEngine::~BisonFlexEvaluationEngine() {
    delete d;
}

QCoro::Task<BisonFlexEvaluationEngine::Result> BisonFlexEvaluationEngine::evaluate(QString expression, QMap<QString, idouble> variables) {
    co_return co_await QtConcurrent::run([](QString expression, QMap<QString, idouble> variables) {
        BisonFlexEvaluationEngine engine;
        engine.setExpression(std::move(expression));
        engine.setVariables(std::move(variables));
        return engine.evaluate();
    },
        expression, variables);
}

BisonFlexEvaluationEngine::Result BisonFlexEvaluationEngine::evaluate() {
    if (d->runningEngines > 50) {
        // Return a stack overflow
        Result r;
        r.type = Result::Error;
        r.error = tr("Stack Overflow");
        return r;
    }

    d->runningEnginesLocker->lock();
    d->runningEngines++;
    d->runningEnginesLocker->unlock();

    Result* result = new Result();

    QString expr = d->expression;
    if (QLocale().decimalPoint() == ',') {
        // Swap the decimal point and comma
        for (int i = 0; i < expr.size(); i++) {
            if (expr.at(i) == ',') {
                expr = expr.replace(i, 1, '.');
            } else if (expr.at(i) == '.') {
                expr = expr.replace(i, 1, ',');
            }
        }
    }
    EvaluationEngineParameters p;
    p.resultFunction = [=](idouble r) { // Success
        result->result = r;
        result->type = Result::Scalar;
    };
    p.errorFunction = [=](int location, int length, const char* s) { // Error
        result->error = QString::fromLocal8Bit(s);
        result->location = location;
        result->length = length;
        result->type = Result::Error;
    };
    p.assignFunction = [=](QString identifier, idouble value) { // Assignment
        if (p.builtinVariables.contains(identifier)) {
            result->error = tr("Can't assign to builtin variable %1").arg(identifier);
            result->type = Result::Error;
        } else {
            result->assigned = true;
            result->identifier = identifier;
            result->value = value;
            result->type = Result::Assign;
        }
    };
    p.equalityFunction = [=](bool isTrue) { // Equality
        result->isTrue = isTrue;
        result->type = Result::Equality;
    };
    p.variables = d->variables;

    yylex_init(&p.scanner);
    YY_BUFFER_STATE bufferState = yy_scan_string(expr.append("\n").toUtf8().constData(), p.scanner);
    yyparse(p.scanner, p);
    yy_delete_buffer(bufferState, p.scanner);
    yylex_destroy(p.scanner);

    Result retval = *result;
    delete result;

    d->runningEnginesLocker->lock();
    d->runningEngines--;
    d->runningEnginesLocker->unlock();

    return retval;
}

void BisonFlexEvaluationEngine::setExpression(QString expression) {
    d->expression = expression;
}

void BisonFlexEvaluationEngine::setVariables(QMap<QString, idouble> vars) {
    d->variables = vars;
}

void BisonFlexEvaluationEngine::setupFunctions() {
    // Clear all the functions
    d->customFunctions.clear();

    // Insert all the builtin functions
    d->customFunctions.insert("abs", createSingleArgFunction([=](idouble arg, QString& error) {
        return abs(arg);
    },
                                         "abs", tr("Calculates the %1 of a %2").arg(tr("absolute value"), tr("number")), tr("number"), tr("The %1 to calculate the %2 of").arg(tr("number"), tr("absolute value"))));
    d->customFunctions.insert("sqrt", createSingleArgFunction([=](idouble arg, QString& error) {
        return sqrt(arg);
    },
                                          "sqrt", tr("Calculates the %1 of a %2").arg(tr("square root"), tr("number")), tr("number"), tr("The %1 to calculate the %2 of").arg(tr("number"), tr("square root"))));
    d->customFunctions.insert("cbrt", createSingleArgFunction([=](idouble arg, QString& error) {
        return pow(arg, 1 / (float) 3);
    },
                                          "cbrt", tr("Calculates the %1 of a %2").arg(tr("cube root"), tr("number")), tr("number"), tr("The %1 to calculate the %2 of").arg(tr("number"), tr("cube root"))));
    d->customFunctions.insert("root", QSharedPointer<BisonFlexCustomFunction>(new BisonFlexCustomFunction([=](QList<idouble> args, QString& error) -> idouble {
        if (args.length() == 2) {
            idouble first = args.first();
            idouble second = args.at(1);

            if (second.real() == 0 && second.imag() == 0) {
                error = tr("Can't take the zeroth root of a number");
                return 0;
            }

            return pow(first, idouble(1.0) / second);
        } else {
            error = tr("The root function takes 2 arguments");
            return 0;
        }
    },
                                                                                  tr("Calculates the %1 of a %2").arg(tr("root"), tr("number")), QStringList() << tr("radicand") + ":" + tr("The %1 to calculate the %2 of").arg(tr("number"), tr("root")) << tr("index") + ":" + tr("The number to root by")))
                                          .staticCast<CustomFunction>());
    d->customFunctions.insert("fact", createSingleArgFunction([=](idouble arg, QString& error) -> idouble {
        if (arg.imag() != 0) {
            error = tr("Can't take the factorial of a non-real number");
            return 0;
        } else if (arg.real() < 0) {
            error = tr("Can't take the factorial of a negative number");
            return 0;
        } else if (arg.real() == 0) {
            return 1;
        } else {
            return arg.real() * tgamma(arg.real());
        }
    },
                                          "fact", tr("Calculates the %1 of an %2").arg(tr("factorial"), tr("angle")), tr("angle"), tr("The %1 to calculate the %2 of").arg(tr("angle"), tr("factorial"))));
    d->customFunctions.insert("sin", createSingleArgFunction([=](idouble arg, QString& error) {
        Q_UNUSED(error)
        return sin(toRad(arg));
    },
                                         "sin", tr("Calculates the %1 of an %2").arg(tr("sine"), tr("angle")), tr("angle"), tr("The %1 to calculate the %2 of").arg(tr("angle"), tr("sine"))));
    d->customFunctions.insert("cos", createSingleArgFunction([=](idouble arg, QString& error) {
        Q_UNUSED(error)
        return cos(toRad(arg));
    },
                                         "cos", tr("Calculates the %1 of an %2").arg(tr("cosine"), tr("angle")), tr("angle"), tr("The %1 to calculate the %2 of").arg(tr("angle"), tr("cosine"))));
    d->customFunctions.insert("tan", createSingleArgFunction([=](idouble arg, QString& error) -> idouble {
        Q_UNUSED(error)

        if (arg.imag() == 0) {
            QString errStr = tr("Can't take the tangent of a number that satisfies the equation %1");
            auto trigUnit = BaseEvaluationEngine::current()->trigonometricUnit();
            if (trigUnit == Degrees) {
                if (fmod(arg.real() - 90, 180) == 0) {
                    error = errStr.arg("90° + 180n");
                    return 0;
                }
            } else if (trigUnit == Gradians) {
                if (fmod(arg.real() - 100, 200) == 0) {
                    error = errStr.arg("100ᵍ + 200n");
                    return 0;
                }
            } else {
                if (fmod(arg.real() - (M_PI / 2), M_PI) == 0) {
                    error = errStr.arg("π/2 + πn");
                    return 0;
                }
            }
        }

        return tan(toRad(arg));
    },
                                         "tan", tr("Calculates the %1 of a %2").arg(tr("tangent"), tr("number")), tr("number"), tr("The %1 to calculate the %2 of").arg(tr("number"), tr("tangent"))));
    d->customFunctions.insert("conj", createSingleArgFunction([=](idouble arg, QString& error) {
        Q_UNUSED(error)
        return conj(arg);
    },
                                          "conj", tr("Calculates the %1 of a %2").arg(tr("conjugate"), tr("complex number")), tr("number"), tr("The %1 to calculate the %2 of").arg(tr("complex number"), tr("conjugate"))));
    d->customFunctions.insert("im", createSingleArgFunction([=](idouble arg, QString& error) {
        Q_UNUSED(error)
        return arg.imag();
    },
                                        "im", tr("Calculates the %1 of a %2").arg(tr("imaginary portion"), tr("complex number")), tr("number"), tr("The %1 to calculate the %2 of").arg(tr("number"), tr("imaginary portion"))));
    d->customFunctions.insert("re", createSingleArgFunction([=](idouble arg, QString& error) {
        Q_UNUSED(error)
        return arg.real();
    },
                                        "re", tr("Calculates the %1 of a %2").arg(tr("real portion"), tr("complex number")), tr("number"), tr("The %1 to calculate the %2 of").arg(tr("number"), tr("real portion"))));
    d->customFunctions.insert("arg", createSingleArgFunction([=](idouble arg, QString& error) -> idouble {
        if (arg.real() == 0 && arg.imag() == 0) {
            error = tr("Can't take the phase angle of 0");
            return 0;
        }
        return fromRad(std::arg(arg));
    },
                                         "arg", tr("Calculates the %1 of a %2").arg(tr("phase angle"), tr("complex number")), tr("number"), tr("The %1 to calculate the %2 of").arg(tr("number"), tr("phase angle"))));

    d->customFunctions.insert("asin", createSingleArgFunction([=](idouble arg, QString& error) -> idouble {
        if (abs(arg) > 1) {
            error = tr("Can't take the inverse sine of a number outside -1 and 1");
            return 0;
        } else {
            return fromRad(asin(arg));
        }
    },
                                          "asin", tr("Calculates the %1 of an %2").arg(tr("arcsine (inverse sine)"), tr("angle")), tr("angle"), tr("The %1 to calculate the %2 of").arg(tr("angle"), tr("arcsine"))));
    d->customFunctions.insert("acos", createSingleArgFunction([=](idouble arg, QString& error) -> idouble {
        if (abs(arg) > 1) {
            error = tr("Can't take the inverse cosine of a number outside -1 and 1");
            return 0;
        } else {
            return fromRad(acos(arg));
        }
    },
                                          "acos", tr("Calculates the %1 of an %2").arg(tr("arccosine (inverse cosine)"), tr("angle")), tr("angle"), tr("The %1 to calculate the %2 of").arg(tr("angle"), tr("arccosine"))));
    d->customFunctions.insert("atan", createSingleArgFunction([=](idouble arg, QString& error) -> idouble {
        Q_UNUSED(error)

        if (arg.real() == 0 && (arg.imag() == 1 || arg.imag() == -1)) {
            error = tr("Can't take the inverse tangent of i or -i").arg(idbToString(arg));
            return 0;
        }

        return fromRad(atan(arg));
    },
                                          "atan", tr("Calculates the %1 of an %2").arg(tr("arctangent (inverse tangent)"), tr("angle")), tr("angle"), tr("The %1 to calculate the %2 of").arg(tr("angle"), tr("arctangent"))));
    d->customFunctions.insert("sec", createSingleArgFunction([=](idouble arg, QString& error) -> idouble {
        if (arg.imag() == 0) {
            QString errStr = tr("Can't take the secant of a number that satisfies the equation %1");
            auto trigUnit = BaseEvaluationEngine::current()->trigonometricUnit();
            if (trigUnit == Degrees) {
                if (fmod(arg.real() - 90, 180) == 0) {
                    error = errStr.arg("90° + 180n");
                    return 0;
                }
            } else if (trigUnit == Gradians) {
                if (fmod(arg.real() - 100, 200) == 0) {
                    error = errStr.arg("100ᵍ + 200n");
                    return 0;
                }
            } else {
                if (fmod(arg.real() - (M_PI / 2), M_PI) == 0) {
                    error = errStr.arg("π/2 + πn");
                    return 0;
                }
            }
        }

        return idouble(1) / cos(toRad(arg));
    },
                                         "sec", tr("Calculates the %1 of an %2").arg(tr("secant"), tr("angle")), tr("angle"), tr("The %1 to calculate the %2 of").arg(tr("angle"), tr("secant"))));
    d->customFunctions.insert("csc", createSingleArgFunction([=](idouble arg, QString& error) -> idouble {
        Q_UNUSED(error)

        if (arg.imag() == 0) {
            QString errStr = tr("Can't take the cosecant of a number that satisfies the equation %1");
            auto trigUnit = BaseEvaluationEngine::current()->trigonometricUnit();
            if (trigUnit == Degrees) {
                if (fmod(arg.real(), 180) == 0) {
                    error = errStr.arg("180n");
                    return 0;
                }
            } else if (trigUnit == Gradians) {
                if (fmod(arg.real(), 200) == 0) {
                    error = errStr.arg("200n");
                    return 0;
                }
            } else {
                if (fmod(arg.real(), M_PI) == 0) {
                    error = errStr.arg("πn");
                    return 0;
                }
            }
        }

        return idouble(1) / sin(toRad(arg));
    },
                                         "csc", tr("Calculates the %1 of an %2").arg(tr("cosecant"), tr("angle")), tr("angle"), tr("The %1 to calculate the %2 of").arg(tr("angle"), tr("cosecant"))));
    d->customFunctions.insert("cot", createSingleArgFunction([=](idouble arg, QString& error) -> idouble {
        Q_UNUSED(error)

        if (arg.imag() == 0) {
            QString errStr = tr("Can't take the cotangent of a number that satisfies the equation %1");
            auto trigUnit = BaseEvaluationEngine::current()->trigonometricUnit();
            if (trigUnit == Degrees) {
                if (fmod(arg.real() - 90, 180) == 0) {
                    error = errStr.arg("90° + 180n");
                    return 0;
                }
            } else if (trigUnit == Gradians) {
                if (fmod(arg.real() - 100, 200) == 0) {
                    error = errStr.arg("100ᵍ + 200n");
                    return 0;
                }
            } else {
                if (fmod(arg.real() - (M_PI / 2), M_PI) == 0) {
                    error = errStr.arg("π/2 + πn");
                    return 0;
                }
            }
        }

        return idouble(1) / tan(toRad(arg));
    },
                                         "cot", tr("Calculates the %1 of an %2").arg(tr("cotangent"), tr("angle")), tr("angle"), tr("The %1 to calculate the %2 of").arg(tr("angle"), tr("cotangent"))));
    d->customFunctions.insert("asec", createSingleArgFunction([=](idouble arg, QString& error) -> idouble {
        Q_UNUSED(error)

        if (arg.real() == 0 && arg.imag() == 0) {
            error = tr("Can't take the inverse secant of 0").arg(idbToString(arg));
            return 0;
        }

        return fromRad(acos(idouble(1) / arg));
    },
                                          "asec", tr("Calculates the %1 of an %2").arg(tr("arcsecant (inverse secant)"), tr("angle")), tr("angle"), tr("The %1 to calculate the %2 of").arg(tr("angle"), tr("arcsecant"))));
    d->customFunctions.insert("acsc", createSingleArgFunction([=](idouble arg, QString& error) -> idouble {
        Q_UNUSED(error)

        if (arg.real() == 0 && arg.imag() == 0) {
            error = tr("Can't take the inverse cosecant of 0").arg(idbToString(arg));
            return 0;
        }

        return fromRad(asin(idouble(1) / arg));
    },
                                          "acsc", tr("Calculates the %1 of an %2").arg(tr("arccosecant (inverse cosecant)"), tr("angle")), tr("angle"), tr("The %1 to calculate the %2 of").arg(tr("angle"), tr("arccosecant"))));
    d->customFunctions.insert("acot", createSingleArgFunction([=](idouble arg, QString& error) {
        Q_UNUSED(error)
        if (arg.real() == 0 && arg.imag() == 0) {
            return fromRad(M_PI / 2);
        } else {
            return fromRad(atan(idouble(1) / arg));
        }
    },
                                          "acot", tr("Calculates the %1 of an %2").arg(tr("arccotangent (inverse cotangent)"), tr("angle")), tr("angle"), tr("The %1 to calculate the %2 of").arg(tr("angle"), tr("arccotangent"))));
    d->customFunctions.insert("sinh", createSingleArgFunction([=](idouble arg, QString& error) {
        Q_UNUSED(error)
        return sinh(arg);
    },
                                          "sinh", tr("Calculates the %1 of an %2").arg(tr("hyperbolic sine"), tr("angle")), tr("angle"), tr("The %1 to calculate the %2 of").arg(tr("angle"), tr("hyperbolic sine"))));
    d->customFunctions.insert("cosh", createSingleArgFunction([=](idouble arg, QString& error) {
        Q_UNUSED(error)
        return cosh(arg);
    },
                                          "cosh", tr("Calculates the %1 of an %2").arg(tr("hyperbolic cosine"), tr("angle")), tr("angle"), tr("The %1 to calculate the %2 of").arg(tr("angle"), tr("hyperbolic cosine"))));
    d->customFunctions.insert("tanh", createSingleArgFunction([=](idouble arg, QString& error) {
        Q_UNUSED(error)
        return tanh(arg);
    },
                                          "tanh", tr("Calculates the %1 of an %2").arg(tr("hyperbolic tangent"), tr("angle")), tr("angle"), tr("The %1 to calculate the %2 of").arg(tr("angle"), tr("hyperbolic tangent"))));
    d->customFunctions.insert("asinh", createSingleArgFunction([=](idouble arg, QString& error) {
        Q_UNUSED(error)
        return asinh(arg);
    },
                                           "asinh", tr("Calculates the %1 of an %2").arg(tr("hyperbolic arcsine (inverse hyperbolic arcsine)"), tr("angle")), tr("angle"), tr("The %1 to calculate the %2 of").arg(tr("angle"), tr("hyperbolic arcsine"))));
    d->customFunctions.insert("acosh", createSingleArgFunction([=](idouble arg, QString& error) {
        Q_UNUSED(error)
        return acosh(arg);
    },
                                           "acosh", tr("Calculates the %1 of an %2").arg(tr("hyperbolic arccosine (inverse hyperbolic arccosine)"), tr("angle")), tr("angle"), tr("The %1 to calculate the %2 of").arg(tr("angle"), tr("hyperbolic arccosine"))));
    d->customFunctions.insert("atanh", createSingleArgFunction([=](idouble arg, QString& error) {
        Q_UNUSED(error)
        return atanh(arg);
    },
                                           "atanh", tr("Calculates the %1 of an %2").arg(tr("hyperbolic arctangent (inverse hyperbolic arctangent)"), tr("angle")), tr("angle"), tr("The %1 to calculate the %2 of").arg(tr("angle"), tr("hyperbolic arctangent"))));

    auto logFunction = QSharedPointer<BisonFlexCustomFunction>(new BisonFlexCustomFunction([=](QList<idouble> args, QString& error) -> idouble {
        if (args.length() == 1) {
            // log base 10
            if (args.first().real() == 0 && args.first().imag() == 0) {
                error = tr("Can't take the logarithm of 0");
                return 0;
            }

            return log10(args.first());
        } else if (args.length() == 2) {
            // log base %2
            if (args.first().real() == 0 && args.first().imag() == 0) {
                error = tr("Can't take the logarithm of 0");
                return 0;
            }

            if (args.last().real() == 1 && args.last().imag() == 0) {
                error = tr("Can't take a logarithm with a base of 1");
                return 0;
            }

            if (args.last().real() == 0 && args.last().imag() == 0) {
                error = tr("Can't take a logarithm with a base of 0");
                return 0;
            }

            return log(args.first()) / log(args.at(1));
        } else {
            error = tr("The log function takes 1 or 2 arguments");
            return 0;
        }
    },
        tr("Calculates the %1 of an %2").arg(tr("base 10 logarithm"), tr("number")), QStringList() << tr("number") + ":" + tr("The %1 to calculate the %2 of").arg(tr("number"), tr("base 10 logarithm"))));
    logFunction->addOverload(tr("Calculates the %1 of an %2").arg(tr("logarithm"), tr("number")), {QStringList() << tr("number") + ":" + tr("The %1 to calculate the %2 of").arg(tr("number"), tr("logarithm"))
                                                                                                                 << tr("base") + ":" + tr("The base of the logarithm")});
    d->customFunctions.insert("log", logFunction.staticCast<CustomFunction>());

    d->customFunctions.insert("ln", createSingleArgFunction([=](idouble arg, QString& error) -> idouble {
        if (arg.real() == 0 && arg.imag() == 0) {
            error = tr("Can't take the logarithm of 0");
            return 0;
        }

        return log(arg);
    },
                                        "ln", tr("Calculates the %1 of an %2").arg(tr("base e logarithm"), tr("number")), tr("number"), tr("The %1 to calculate the %2 of").arg(tr("number"), tr("base e logarithm"))));

    d->customFunctions.insert("lsh", QSharedPointer<BisonFlexCustomFunction>(new BisonFlexCustomFunction([=](QList<idouble> args, QString& error) -> idouble {
        if (args.length() == 2) {
            idouble first = args.first();
            idouble second = args.at(1);

            if (first.imag() != 0) {
                error = tr("Can't bit shift a non-integer");
                return 0;
            }
            if (second.imag() != 0) {
                error = tr("Can't bit shift by a non-integer");
                return 0;
            }

            if (floor(first.real()) != first.real()) {
                error = tr("Can't bit shift a non-integer");
                return 0;
            }

            if (floor(second.real()) != second.real()) {
                error = tr("Can't bit shift by a non-integer");
                return 0;
            }

            return (int) first.real() << (int) second.real();
        } else {
            error = tr("The lsh function takes 2 arguments");
            return 0;
        }
    },
                                                                                 tr("Shifts a number to the left by a specified number of bits"), QStringList() << tr("number") + ":" + tr("The number to shift") << tr("amount") + ":" + tr("The number of bits to shift by")))
                                         .staticCast<CustomFunction>());
    d->customFunctions.insert("rsh", QSharedPointer<BisonFlexCustomFunction>(new BisonFlexCustomFunction([=](QList<idouble> args, QString& error) -> idouble {
        if (args.length() == 2) {
            idouble first = args.first();
            idouble second = args.at(1);

            if (first.imag() != 0) {
                error = tr("Can't bit shift a non-integer");
                return 0;
            }
            if (second.imag() != 0) {
                error = tr("Can't bit shift by a non-integer");
                return 0;
            }

            if (floor(first.real()) != first.real()) {
                error = tr("Can't bit shift a non-integer");
                return 0;
            }

            if (floor(second.real()) != second.real()) {
                error = tr("Can't bit shift by a non-integer");
                return 0;
            }

            return (int) first.real() >> (int) second.real();
        } else {
            error = tr("The rsh function takes 2 arguments");
            return 0;
        }
    },
                                                                                 tr("Shifts a number to the right by a specified number of bits"), QStringList() << tr("number") + ":" + tr("The number to shift") << tr("amount") + ":" + tr("The number of bits to shift by")))
                                         .staticCast<CustomFunction>());
    d->customFunctions.insert("pow", QSharedPointer<BisonFlexCustomFunction>(new BisonFlexCustomFunction([=](QList<idouble> args, QString& error) -> idouble {
        if (args.length() == 2) {
            idouble first = args.first();
            idouble second = args.at(1);

            if (first.real() == 0 && first.imag() == 0 &&
                second.real() == 0 && second.imag() == 0) {
                error = tr("Can't take 0 to the power of 0");
                return 0;
            }

            if (first.real() == 0 && first.imag() == 0 &&
                second.real() < 0 && second.imag() == 0) {
                error = tr("Can't take 0 to the power of a negative number");
                return 0;
            }

            if (first.real() == 0 && first.imag() == 0 && second.real() == 0 && second.imag() != 0) {
                error = tr("Can't take 0 to the power of a non-real number");
                return 0;
            }

            return pow(first, second);
        } else {
            error = tr("The pow function takes 2 arguments");
            return 0;
        }
    },
                                                                                 tr("Calculates an exponent"), QStringList() << tr("base") + ":" + tr("The base of the exponent") << tr("exponent") + ":" + tr("The number to exponentiate by")))
                                         .staticCast<CustomFunction>());
    d->customFunctions.insert("mod", QSharedPointer<BisonFlexCustomFunction>(new BisonFlexCustomFunction([=](QList<idouble> args, QString& error) -> idouble {
        if (args.length() == 2) {
            idouble first = args.first();
            idouble second = args.at(1);

            if (floor(first.real()) != first.real() || first.imag() != 0) {
                error = tr("Can't take the modulo of a non-integer");
                return 0;
            }

            if (floor(second.real()) != second.real() || second.imag() != 0) {
                error = tr("Can't take the modulo when the divisor is a non-integer");
                return 0;
            }

            if (second.real() == 0) {
                error = tr("Can't divide by zero");
                return 0;
            }

            return (int) first.real() % (int) second.real();
        } else {
            error = tr("The mod function takes 2 arguments");
            return 0;
        }
    },
                                                                                 tr("Calculates the remainder when dividing two numbers"), QStringList() << tr("divisor") + ":" + tr("The number to be divided") << tr("dividend") + ":" + tr("The number to divide by")))
                                         .staticCast<CustomFunction>());

    d->customFunctions.insert("floor", createSingleArgFunction([=](idouble arg, QString& error) {
        return floor(arg.real());
    },
                                           "floor", tr("Calculates the %1 of an %2").arg(tr("floor"), tr("number")), tr("number"), tr("The %1 to calculate the %2 of").arg(tr("number"), tr("floor"))));
    d->customFunctions.insert("ceil", createSingleArgFunction([=](idouble arg, QString& error) {
        return ceil(arg.real());
    },
                                          "ceil", tr("Calculates the %1 of an %2").arg(tr("ceiling"), tr("number")), tr("number"), tr("The %1 to calculate the %2 of").arg(tr("number"), tr("ceiling"))));

    auto randomFunction = QSharedPointer<BisonFlexCustomFunction>(new BisonFlexCustomFunction([=](QList<idouble> args, QString& error) -> idouble {
        QRandomGenerator* gen = QRandomGenerator::system();

        if (args.length() == 0) {
            return idouble(gen->generate());
        } else if (args.length() == 1) {
            if (args.first().imag() != 0) {
                error = tr("Can't generate a random number with an upper bound of a non-real number");
                return 0;
            }

            return idouble(gen->bounded((double) args.first().real()));
        } else if (args.length() == 2) {
            if (args.first().imag() != 0) {
                error = tr("Can't generate a random number with an upper bound of a non-real number");
                return 0;
            }

            if (args.last().imag() != 0) {
                error = tr("Can't generate a random number with a lower bound of a non-real number");
                return 0;
            }

            return idouble(gen->bounded((int) args.first().real(), (int) args.last().real()));
        } else {
            error = tr("The random function takes 0-2 arguments").arg(args.length());
            return 0;
        }
    },
        tr("Returns a random number"), QStringList()));
    randomFunction->addOverload(tr("Returns a random number in the range [0-number)"), QStringList() << tr("bound") + ":" + tr("The exclusive high bound"));
    randomFunction->addOverload(tr("Returns a random number in the range [low-high)"), QStringList() << tr("low") + ":" + tr("The inclusive low bound") << tr("high") + ":" + tr("The exclusive high bound"));
    d->customFunctions.insert("random", randomFunction.staticCast<CustomFunction>());

    // Set up all the custom functions
    QSettings settings;
    settings.beginGroup("customFunctions");
    for (QString function : settings.allKeys()) {
        QJsonDocument doc = QJsonDocument::fromJson(settings.value(function).toByteArray());
        if (doc.isObject()) {
            QJsonObject obj = doc.object();
            QJsonArray overloads = obj.value("overloads").toArray();

            auto func = QSharedPointer<BisonFlexCustomFunction>(new BisonFlexCustomFunction([=](QList<idouble> args, QString& error) -> idouble {
                QStringList argCounts;
                BisonFlexEvaluationEngine engine;
                QMap<QString, idouble> variables;

                for (QJsonValue o : overloads) {
                    QJsonObject overload = o.toObject();

                    QJsonArray fnArgs = overload.value("args").toArray();
                    if (fnArgs.count() == args.count()) {
                        // Found the correct overload to use
                        // Populate the arguments
                        for (int i = 0; i < args.count(); i++) {
                            QJsonValue arg = fnArgs.at(i);
                            if (arg.isObject()) {
                                variables.insert(arg.toObject().value("name").toString(), args.at(i));
                            } else {
                                variables.insert(arg.toString(), args.at(i));
                            }
                        }
                        engine.setVariables(variables);

                        // For each branch, evaluate the conditions and choose the branch
                        QJsonObject selectedBranch;
                        QJsonArray branches = overload.value("branches").toArray();
                        for (const QJsonValue& b : qAsConst(branches)) {
                            QJsonObject branch = b.toObject();
                            if (branch.value("isOtherwise").toBool()) {
                                // Reached the final branch
                                selectedBranch = branch;
                                break;
                            } else {
                                bool currentConditionState = false;
                                QJsonArray conditions = branch.value("conditions").toArray();
                                for (const QJsonValue& c : qAsConst(conditions)) {
                                    QJsonObject condition = c.toObject();
                                    engine.setExpression(condition.value("expression").toString());
                                    int connective = condition.value("connective").toInt();
                                    BisonFlexEvaluationEngine::Result result = engine.evaluate();
                                    bool boolResult;

                                    switch (result.type) {
                                        case BisonFlexEvaluationEngine::Result::Scalar:
                                            boolResult = result.result != idouble(0, 0);
                                            break;
                                        case BisonFlexEvaluationEngine::Result::Equality:
                                            boolResult = result.isTrue;
                                            break;
                                        case BisonFlexEvaluationEngine::Result::Error:
                                            // Return an error
                                            error = result.error;
                                            return 0;
                                        case BisonFlexEvaluationEngine::Result::Assign:
                                            // Return an error
                                            error = tr("The function definition for %1 contains a condition which is an assignment").arg(function);
                                            return 0;
                                    }

                                    if (condition.value("isFirst").toBool()) {
                                        if (connective == 0) { // WHEN
                                            currentConditionState = boolResult;
                                        } else { // WHEN NOT
                                            currentConditionState = !boolResult;
                                        }
                                    } else {
                                        switch (connective) {
                                            case 0: // AND
                                                currentConditionState &= boolResult;
                                                break;
                                            case 1: // OR
                                                currentConditionState |= boolResult;
                                                break;
                                            case 2: // XOR
                                                currentConditionState ^= boolResult;
                                                break;
                                            case 3: // AND NOT
                                                currentConditionState &= !boolResult;
                                                break;
                                            case 4: // OR NOT
                                                currentConditionState |= !boolResult;
                                                break;
                                            case 5: // XOR NOT
                                                currentConditionState ^= !boolResult;
                                                break;
                                        }
                                    }
                                }

                                if (currentConditionState) { // This is the branch we need
                                    selectedBranch = branch;
                                    break;
                                }
                            }
                        }

                        // Now that we've selected a branch, evaluate the expression
                        QJsonObject retval = selectedBranch.value("return").toObject();
                        if (retval.value("isError").toBool()) {
                            // This is an error branch
                            error = function + ": " + retval.value("expression").toString();
                            return 0;
                        } else {
                            engine.setExpression(retval.value("expression").toString());
                            BisonFlexEvaluationEngine::Result result = engine.evaluate();

                            switch (result.type) {
                                case BisonFlexEvaluationEngine::Result::Scalar:
                                    return result.result;
                                case BisonFlexEvaluationEngine::Result::Equality:
                                    return idouble(result.isTrue);
                                case BisonFlexEvaluationEngine::Result::Error:
                                    // Return an error
                                    error = result.error;
                                    return 0;
                                case BisonFlexEvaluationEngine::Result::Assign:
                                    // Return an error
                                    error = tr("The function definition for %1 contains a condition which is an assignment").arg(function);
                                    return 0;
                            }
                        }
                    } else {
                        argCounts.append(QString::number(fnArgs.count()));
                    }
                }

                QString argSpecification = argCounts.join(", ");
                if (argCounts.count() == 1) {
                    error = tr("The %1 function takes %n arguments", nullptr, argCounts.first().toInt()).arg(function);
                } else {
                    int last = argSpecification.lastIndexOf(", ");
                    argSpecification = argSpecification.replace(last, 2, " " + tr("or", "Expected 1, 2 or 3 arguments") + " ");
                    error = tr("The %1 function takes %2 arguments").arg(function, argSpecification);
                }

                return 0;
            }));

            for (const QJsonValue& o : qAsConst(overloads)) {
                QJsonObject overload = o.toObject();
                QStringList argList;

                QJsonArray args = overload.value("args").toArray();

                for (const QJsonValue& v : qAsConst(args)) {
                    if (v.isObject()) {
                        QJsonObject o = v.toObject();
                        argList.append(o.value("name").toString() + ":" + o.value("desc").toString());
                    } else {
                        argList.append(v.toString() + ":");
                    }
                }

                func->addOverload(overload.value("desc").toString(), argList);
            }
            d->customFunctions.insert(function, func.staticCast<CustomFunction>());
        }
    }
    settings.endGroup();
}

idouble BisonFlexEvaluationEngine::fromRad(idouble rad) {
    auto trigUnit = BaseEvaluationEngine::current()->trigonometricUnit();
    if (trigUnit == Degrees) {
        rad = idouble(rad.real() * 180 / M_PI, rad.imag());
        // rad *= idouble(180 / M_PI, 1);
    } else if (trigUnit == Gradians) {
        rad = idouble(rad.real() * 200 / M_PI, rad.imag());
        // rad *= idouble(200 / M_PI, 1);
    }
    return rad;
}

idouble BisonFlexEvaluationEngine::toRad(idouble deg) {
    auto trigUnit = BaseEvaluationEngine::current()->trigonometricUnit();
    if (trigUnit == Degrees) {
        // deg *= idouble(M_PI / 180, 1);
        deg = idouble(deg.real() * M_PI / 180, deg.imag());
    } else if (trigUnit == Gradians) {
        // deg *= idouble(M_PI / 200, 1);
        deg = idouble(deg.real() * M_PI / 200, deg.imag());
    }
    return deg;
}

CustomFunctionPtr BisonFlexEvaluationEngine::createSingleArgFunction(std::function<idouble(idouble, QString&)> fn, QString fnName, QString fnDesc, QString argName, QString argDesc) {
    return QSharedPointer<BisonFlexCustomFunction>(new BisonFlexCustomFunction([=](QList<idouble> args, QString& error) -> idouble {
        if (args.length() != 1) {
            error = tr("The %1 function takes 1 argument").arg(fnName);
            return 0;
        } else {
            return fn(args.first(), error);
        }
    },
                                                       fnDesc, QStringList() << argName + ":" + argDesc))
        .staticCast<CustomFunction>();
}

CustomFunctionMap BisonFlexEvaluationEngine::customFunctions() const {
    return d->customFunctions;
}
