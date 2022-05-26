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

#include "evaluationengine.h"

typedef void* yyscan_t;
#include "calc.bison.hpp"
#include "calc.yy.h"

#include <QSettings>

int EvaluationEngine::runningEngines = 0;
QMutex* EvaluationEngine::runningEnginesLocker = new QMutex();
EvaluationEngine::TrigonometricUnit EvaluationEngine::trigUnit = EvaluationEngine::Degrees;
QMutex* EvaluationEngine::trigUnitLocker = new QMutex();
CustomFunctionMap EvaluationEngine::customFunctions = CustomFunctionMap();

extern QString idbToString(idouble db);

struct CustomFunctionPrivate {
    CustomFunctionDefinition fn;
    QStringList desc;
    QList<QStringList> args;
    bool waitingForDocs = false;
};

CustomFunction::CustomFunction() {
    d = QSharedPointer<CustomFunctionPrivate>(new CustomFunctionPrivate());
}

CustomFunction::CustomFunction(CustomFunctionDefinition fn) {
    d = QSharedPointer<CustomFunctionPrivate>(new CustomFunctionPrivate());
    d->fn = fn;
    d->desc.append("A function");
    d->args.append(QStringList());
    d->waitingForDocs = true;
}

CustomFunction::CustomFunction(CustomFunctionDefinition function, QString desc, QStringList args) {
    d = QSharedPointer<CustomFunctionPrivate>(new CustomFunctionPrivate());
    d->fn = function;
    d->desc.append(desc);
    d->args.append(args);
}

CustomFunctionDefinition CustomFunction::getFunction() const {
    return d->fn;
}

QString CustomFunction::getDescription(int overload) const {
    return d->desc.at(overload);
}

QStringList CustomFunction::getArgs(int overload) const {
    return d->args.at(overload);
}

void CustomFunction::addOverload(QString desc, QStringList args) {
    if (d->waitingForDocs) {
        d->desc.clear();
        d->args.clear();
        d->waitingForDocs = false;
    }
    d->desc.append(desc);
    d->args.append(args);
}

int CustomFunction::overloads() {
    return d->args.count();
}

EvaluationEngine::EvaluationEngine(QObject* parent) : QObject(parent) {
    QT_TR_NOOP("%1: unknown variable");
    QT_TR_NOOP("div: division by 0 undefined");
    QT_TR_NOOP("%1: undefined function");
}

EvaluationEngine::~EvaluationEngine() {

}

tPromise<EvaluationEngine::Result>* EvaluationEngine::evaluate(QString expression, QMap<QString, idouble> variables) {
    return new tPromise<Result>([ = ](QString & error) -> Result {
        EvaluationEngine engine;
        engine.setExpression(expression);
        engine.setVariables(variables);
        return engine.evaluate();
    });
}

EvaluationEngine::Result EvaluationEngine::evaluate() {
    if (runningEngines > 50) {
        //Return a stack overflow
        Result r;
        r.type = Result::Error;
        r.error = tr("Stack Overflow");
        return r;
    }

    runningEnginesLocker->lock();
    runningEngines++;
    runningEnginesLocker->unlock();

    Result* result = new Result();

    QString expr = expression;
    if (QLocale().decimalPoint() == ',') {
        //Swap the decimal point and comma
        for (int i = 0; i < expr.count(); i++) {
            if (expr.at(i) == ',') {
                expr = expr.replace(i, 1, '.');
            } else if (expr.at(i) == '.') {
                expr = expr.replace(i, 1, ',');
            }
        }
    }
    EvaluationEngineParameters p;
    p.resultFunction = [ = ](idouble r) { //Success
        result->result = r;
        result->type = Result::Scalar;
    };
    p.errorFunction = [ = ](int location, int length, const char* s) { //Error
        result->error = QString::fromLocal8Bit(s);
        result->location = location;
        result->length = length;
        result->type = Result::Error;
    };
    p.assignFunction = [ = ](QString identifier, idouble value) { //Assignment
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
    p.equalityFunction = [ = ](bool isTrue) { //Equality
        result->isTrue = isTrue;
        result->type = Result::Equality;
    };
    p.variables = variables;

    yylex_init(&p.scanner);
    YY_BUFFER_STATE bufferState = yy_scan_string(expr.append("\n").toUtf8().constData(), p.scanner);
    yyparse(p.scanner, p);
    yy_delete_buffer(bufferState, p.scanner);
    yylex_destroy(p.scanner);

    Result retval = *result;
    delete result;

    runningEnginesLocker->lock();
    runningEngines--;
    runningEnginesLocker->unlock();

    return retval;
}

void EvaluationEngine::setExpression(QString expression) {
    this->expression = expression;
}

void EvaluationEngine::setVariables(QMap<QString, idouble> vars) {
    this->variables = vars;
}

void EvaluationEngine::setupFunctions() {
    //Clear all the functions
    customFunctions.clear();

    //Insert all the builtin functions
    customFunctions.insert("abs", createSingleArgFunction([ = ](idouble arg, QString & error) {
        return abs(arg);
    }, "abs", tr("Calculates the %1 of a %2").arg(tr("absolute value"), tr("number")), tr("number"), tr("The %1 to calculate the %2 of").arg(tr("number"), tr("absolute value"))));
    customFunctions.insert("sqrt", createSingleArgFunction([ = ](idouble arg, QString & error) {
        return sqrt(arg);
    }, "sqrt", tr("Calculates the %1 of a %2").arg(tr("square root"), tr("number")), tr("number"), tr("The %1 to calculate the %2 of").arg(tr("number"), tr("square root"))));
    customFunctions.insert("cbrt", createSingleArgFunction([ = ](idouble arg, QString & error) {
        return pow(arg, 1 / (float) 3);
    }, "cbrt", tr("Calculates the %1 of a %2").arg(tr("cube root"), tr("number")), tr("number"), tr("The %1 to calculate the %2 of").arg(tr("number"), tr("cube root"))));
    customFunctions.insert("root", CustomFunction([ = ](QList<idouble> args, QString & error) -> idouble {
        if (args.length() == 2) {
            idouble first = args.first();
            idouble second = args.at(1);

            if (second.real() == 0 && second.imag() == 0) {
                error = tr("root: arg1 (%1) out of bounds (not 0)").arg(idbToString(second));
                return 0;
            }

            return pow(first, idouble(1.0) / second);
        } else {
            error = tr("root: expected 2 arguments, got %1").arg(args.length());
            return 0;
        }
    }, tr("Calculates the %1 of a %2").arg(tr("root"), tr("number")),
    QStringList() << tr("radicand") + ":" + tr("The %1 to calculate the %2 of").arg(tr("number"), tr("root"))
        << tr("index") + ":" + tr("The number to root by")
        ));
    customFunctions.insert("fact", createSingleArgFunction([ = ](idouble arg, QString & error) -> idouble {
        if (arg.imag() != 0) {
            error = tr("fact: input (%1) not a real number").arg(idbToString(arg));
            return 0;
        } else if (arg.real() < 0) {
            error = tr("fact: input (%1) out of bounds (0 and above)").arg(idbToString(arg));
            return 0;
        } else if (arg.real() == 0) {
            return 1;
        } else {
            return arg.real() * tgamma(arg.real());
        }
    }, "fact", tr("Calculates the %1 of an %2").arg(tr("factorial"), tr("angle")), tr("angle"), tr("The %1 to calculate the %2 of").arg(tr("angle"), tr("factorial"))));
    customFunctions.insert("sin", createSingleArgFunction([ = ](idouble arg, QString & error) {
        Q_UNUSED(error)
        return sin(toRad(arg));
    }, "sin", tr("Calculates the %1 of an %2").arg(tr("sine"), tr("angle")), tr("angle"), tr("The %1 to calculate the %2 of").arg(tr("angle"), tr("sine"))));
    customFunctions.insert("cos", createSingleArgFunction([ = ](idouble arg, QString & error) {
        Q_UNUSED(error)
        return cos(toRad(arg));
    }, "cos", tr("Calculates the %1 of an %2").arg(tr("cosine"), tr("angle")), tr("angle"), tr("The %1 to calculate the %2 of").arg(tr("angle"), tr("cosine"))));
    customFunctions.insert("tan", createSingleArgFunction([ = ](idouble arg, QString & error) -> idouble {
        Q_UNUSED(error)

        if (arg.imag() == 0) {
            QString errStr = tr("tan: input (%1) out of bounds (not %2)");
            if (trigUnit == Degrees) {
                if (fmod(arg.real() - 90, 180) == 0) {
                    error = errStr.arg(idbToString(arg), "90° + 180n");
                    return 0;
                }
            } else if (trigUnit == Gradians) {
                if (fmod(arg.real() - 100, 200) == 0) {
                    error = errStr.arg(idbToString(arg), "100ᵍ + 200n");
                    return 0;
                }
            } else {
                if (fmod(arg.real() - (M_PI / 2), M_PI) == 0) {
                    error = errStr.arg(idbToString(arg), "π/2 + πn");
                    return 0;
                }
            }
        }

        return tan(toRad(arg));
    }, "tan", tr("Calculates the %1 of a %2").arg(tr("tangent"), tr("number")), tr("number"), tr("The %1 to calculate the %2 of").arg(tr("number"), tr("tangent"))));
    customFunctions.insert("conj", createSingleArgFunction([ = ](idouble arg, QString & error) {
        Q_UNUSED(error)
        return conj(arg);
    }, "conj", tr("Calculates the %1 of a %2").arg(tr("conjugate"), tr("complex number")), tr("number"), tr("The %1 to calculate the %2 of").arg(tr("complex number"), tr("conjugate"))));
    customFunctions.insert("im", createSingleArgFunction([ = ](idouble arg, QString & error) {
        Q_UNUSED(error)
        return arg.imag();
    }, "im", tr("Calculates the %1 of a %2").arg(tr("imaginary portion"), tr("complex number")), tr("number"), tr("The %1 to calculate the %2 of").arg(tr("number"), tr("imaginary portion"))));
    customFunctions.insert("re", createSingleArgFunction([ = ](idouble arg, QString & error) {
        Q_UNUSED(error)
        return arg.real();
    }, "re", tr("Calculates the %1 of a %2").arg(tr("real portion"), tr("complex number")), tr("number"), tr("The %1 to calculate the %2 of").arg(tr("number"), tr("real portion"))));
    customFunctions.insert("arg", createSingleArgFunction([ = ](idouble arg, QString & error) -> idouble {
        if (arg.real() == 0 && arg.imag() == 0) {
            error = tr("arg: phase angle of 0 undefined");
            return 0;
        }
        return fromRad(std::arg(arg));
    }, "arg", tr("Calculates the %1 of a %2").arg(tr("phase angle"), tr("complex number")), tr("number"), tr("The %1 to calculate the %2 of").arg(tr("number"), tr("phase angle"))));

    customFunctions.insert("asin", createSingleArgFunction([ = ](idouble arg, QString & error) -> idouble {
        if (abs(arg) > 1) {
            error = tr("asin: input (%1) out of bounds (between -1 and 1)").arg(idbToString(arg));
            return 0;
        } else {
            return fromRad(asin(arg));
        }
    }, "asin", tr("Calculates the %1 of an %2").arg(tr("arcsine (inverse sine)"), tr("angle")), tr("angle"), tr("The %1 to calculate the %2 of").arg(tr("angle"), tr("arcsine"))));
    customFunctions.insert("acos", createSingleArgFunction([ = ](idouble arg, QString & error) -> idouble {
        if (abs(arg) > 1) {
            error = tr("acos: input (%1) out of bounds (between -1 and 1)").arg(idbToString(arg));
            return 0;
        } else {
            return fromRad(acos(arg));
        }
    }, "acos", tr("Calculates the %1 of an %2").arg(tr("arccosine (inverse cosine)"), tr("angle")), tr("angle"), tr("The %1 to calculate the %2 of").arg(tr("angle"), tr("arccosine"))));
    customFunctions.insert("atan", createSingleArgFunction([ = ](idouble arg, QString & error) -> idouble {
        Q_UNUSED(error)

        if (arg.real() == 0 && (arg.imag() == 1 || arg.imag() == -1)) {
            error = tr("atan: input (%1) out of bounds (not i or -i)").arg(idbToString(arg));
            return 0;
        }

        return fromRad(atan(arg));
    }, "atan", tr("Calculates the %1 of an %2").arg(tr("arctangent (inverse tangent)"), tr("angle")), tr("angle"), tr("The %1 to calculate the %2 of").arg(tr("angle"), tr("arctangent"))));
    customFunctions.insert("sec", createSingleArgFunction([ = ](idouble arg, QString & error) -> idouble {
        if (arg.imag() == 0) {
            QString errStr = tr("sec: input (%1) out of bounds (not %2)");
            if (trigUnit == Degrees) {
                if (fmod(arg.real() - 90, 180) == 0) {
                    error = errStr.arg(idbToString(arg), "90° + 180n");
                    return 0;
                }
            } else if (trigUnit == Gradians) {
                if (fmod(arg.real() - 100, 200) == 0) {
                    error = errStr.arg(idbToString(arg), "100ᵍ + 200n");
                    return 0;
                }
            } else {
                if (fmod(arg.real() - (M_PI / 2), M_PI) == 0) {
                    error = errStr.arg(idbToString(arg), "π/2 + πn");
                    return 0;
                }
            }
        }

        return idouble(1) / cos(toRad(arg));
    }, "sec", tr("Calculates the %1 of an %2").arg(tr("secant"), tr("angle")), tr("angle"), tr("The %1 to calculate the %2 of").arg(tr("angle"), tr("secant"))));
    customFunctions.insert("csc", createSingleArgFunction([ = ](idouble arg, QString & error) -> idouble {
        Q_UNUSED(error)

        if (arg.imag() == 0) {
            QString errStr = tr("csc: input (%1) out of bounds (not %2)");
            if (trigUnit == Degrees) {
                if (fmod(arg.real(), 180) == 0) {
                    error = errStr.arg(idbToString(arg), "180n");
                    return 0;
                }
            } else if (trigUnit == Gradians) {
                if (fmod(arg.real(), 200) == 0) {
                    error = errStr.arg(idbToString(arg), "200n");
                    return 0;
                }
            } else {
                if (fmod(arg.real(), M_PI) == 0) {
                    error = errStr.arg(idbToString(arg), "πn");
                    return 0;
                }
            }
        }

        return idouble(1) / sin(toRad(arg));
    }, "csc", tr("Calculates the %1 of an %2").arg(tr("cosecant"), tr("angle")), tr("angle"), tr("The %1 to calculate the %2 of").arg(tr("angle"), tr("cosecant"))));
    customFunctions.insert("cot", createSingleArgFunction([ = ](idouble arg, QString & error) -> idouble {
        Q_UNUSED(error)

        if (arg.imag() == 0) {
            QString errStr = tr("cot: input (%1) out of bounds (not %2)");
            if (trigUnit == Degrees) {
                if (fmod(arg.real() - 90, 180) == 0) {
                    error = errStr.arg(idbToString(arg), "90° + 180n");
                    return 0;
                }
            } else if (trigUnit == Gradians) {
                if (fmod(arg.real() - 100, 200) == 0) {
                    error = errStr.arg(idbToString(arg), "100ᵍ + 200n");
                    return 0;
                }
            } else {
                if (fmod(arg.real() - (M_PI / 2), M_PI) == 0) {
                    error = errStr.arg(idbToString(arg), "π/2 + πn");
                    return 0;
                }
            }
        }

        return idouble(1) / tan(toRad(arg));
    }, "cot", tr("Calculates the %1 of an %2").arg(tr("cotangent"), tr("angle")), tr("angle"), tr("The %1 to calculate the %2 of").arg(tr("angle"), tr("cotangent"))));
    customFunctions.insert("asec", createSingleArgFunction([ = ](idouble arg, QString & error) -> idouble {
        Q_UNUSED(error)

        if (arg.real() == 0 && arg.imag() == 0) {
            error = tr("asec: input (%1) out of bounds (not 0)").arg(idbToString(arg));
            return 0;
        }

        return fromRad(acos(idouble(1) / arg));
    }, "asec", tr("Calculates the %1 of an %2").arg(tr("arcsecant (inverse secant)"), tr("angle")), tr("angle"), tr("The %1 to calculate the %2 of").arg(tr("angle"), tr("arcsecant"))));
    customFunctions.insert("acsc", createSingleArgFunction([ = ](idouble arg, QString & error) -> idouble{
        Q_UNUSED(error)

        if (arg.real() == 0 && arg.imag() == 0) {
            error = tr("acsc: input (%1) out of bounds (not 0)").arg(idbToString(arg));
            return 0;
        }

        return fromRad(asin(idouble(1) / arg));
    }, "acsc", tr("Calculates the %1 of an %2").arg(tr("arccosecant (inverse cosecant)"), tr("angle")), tr("angle"), tr("The %1 to calculate the %2 of").arg(tr("angle"), tr("arccosecant"))));
    customFunctions.insert("acot", createSingleArgFunction([ = ](idouble arg, QString & error) {
        Q_UNUSED(error)
        if (arg.real() == 0 && arg.imag() == 0) {
            return fromRad(M_PI / 2);
        } else {
            return fromRad(atan(idouble(1) / arg));
        }
    }, "acot", tr("Calculates the %1 of an %2").arg(tr("arccotangent (inverse cotangent)"), tr("angle")), tr("angle"), tr("The %1 to calculate the %2 of").arg(tr("angle"), tr("arccotangent"))));
    customFunctions.insert("sinh", createSingleArgFunction([ = ](idouble arg, QString & error) {
        Q_UNUSED(error)
        return sinh(arg);
    }, "sinh", tr("Calculates the %1 of an %2").arg(tr("hyperbolic sine"), tr("angle")), tr("angle"), tr("The %1 to calculate the %2 of").arg(tr("angle"), tr("hyperbolic sine"))));
    customFunctions.insert("cosh", createSingleArgFunction([ = ](idouble arg, QString & error) {
        Q_UNUSED(error)
        return cosh(arg);
    }, "cosh", tr("Calculates the %1 of an %2").arg(tr("hyperbolic cosine"), tr("angle")), tr("angle"), tr("The %1 to calculate the %2 of").arg(tr("angle"), tr("hyperbolic cosine"))));
    customFunctions.insert("tanh", createSingleArgFunction([ = ](idouble arg, QString & error) {
        Q_UNUSED(error)
        return tanh(arg);
    }, "tanh", tr("Calculates the %1 of an %2").arg(tr("hyperbolic tangent"), tr("angle")), tr("angle"), tr("The %1 to calculate the %2 of").arg(tr("angle"), tr("hyperbolic tangent"))));
    customFunctions.insert("asinh", createSingleArgFunction([ = ](idouble arg, QString & error) {
        Q_UNUSED(error)
        return asinh(arg);
    }, "asinh", tr("Calculates the %1 of an %2").arg(tr("hyperbolic arcsine (inverse hyperbolic arcsine)"), tr("angle")), tr("angle"), tr("The %1 to calculate the %2 of").arg(tr("angle"), tr("hyperbolic arcsine"))));
    customFunctions.insert("acosh", createSingleArgFunction([ = ](idouble arg, QString & error) {
        Q_UNUSED(error)
        return acosh(arg);
    }, "acosh", tr("Calculates the %1 of an %2").arg(tr("hyperbolic arccosine (inverse hyperbolic arccosine)"), tr("angle")), tr("angle"), tr("The %1 to calculate the %2 of").arg(tr("angle"), tr("hyperbolic arccosine"))));
    customFunctions.insert("atanh", createSingleArgFunction([ = ](idouble arg, QString & error) {
        Q_UNUSED(error)
        return atanh(arg);
    }, "atanh", tr("Calculates the %1 of an %2").arg(tr("hyperbolic arctangent (inverse hyperbolic arctangent)"), tr("angle")), tr("angle"), tr("The %1 to calculate the %2 of").arg(tr("angle"), tr("hyperbolic arctangent"))));

    CustomFunction logFunction([ = ](QList<idouble> args, QString & error) -> idouble {
        if (args.length() == 1) {
            //log base 10
            if (args.first().real() == 0 && args.first().imag() == 0) {
                error = tr("log: input (%1) out of bounds (not 0)").arg(idbToString(args.first()));
                return 0;
            }

            return log10(args.first());
        } else if (args.length() == 2) {
            //log base %2
            if (args.first().real() == 0 && args.first().imag() == 0) {
                error = tr("log: arg1 (%1) out of bounds (not 0)").arg(idbToString(args.first()));
                return 0;
            }

            if (args.last().real() == 1 && args.last().imag() == 0) {
                error = tr("log: arg2 (%1) out of bounds (not 1)").arg(idbToString(args.last()));
                return 0;
            }

            if (args.last().real() == 0 && args.last().imag() == 0) {
                error = tr("log: arg2 (%1) out of bounds (not 0)").arg(idbToString(args.last()));
                return 0;
            }

            return log(args.first()) / log(args.at(1));
        } else {
            error = tr("log: expected 1 or 2 arguments, got %1").arg(args.length());
            return 0;
        }
    }, tr("Calculates the %1 of an %2").arg(tr("base 10 logarithm"), tr("number")), QStringList() << tr("number") + ":" + tr("The %1 to calculate the %2 of").arg(tr("number"), tr("base 10 logarithm")));
    logFunction.addOverload(tr("Calculates the %1 of an %2").arg(tr("logarithm"), tr("number")), {
        QStringList() << tr("number") + ":" + tr("The %1 to calculate the %2 of").arg(tr("number"), tr("logarithm"))
            << tr("base") + ":" + tr("The base of the logarithm")
    });
    customFunctions.insert("log", logFunction);

    customFunctions.insert("ln", createSingleArgFunction([ = ](idouble arg, QString & error) -> idouble {
        if (arg.real() == 0 && arg.imag() == 0) {
            error = tr("ln: input (%1) out of bounds (not 0)").arg(idbToString(arg));
            return 0;
        }

        return log(arg);
    }, "ln", tr("Calculates the %1 of an %2").arg(tr("base e logarithm"), tr("number")), tr("number"), tr("The %1 to calculate the %2 of").arg(tr("number"), tr("base e logarithm"))));

    customFunctions.insert("lsh", CustomFunction([ = ](QList<idouble> args, QString & error) -> idouble {
        if (args.length() == 2) {
            idouble first = args.first();
            idouble second = args.at(1);

            if (first.imag() != 0) {
                error = tr("lsh: arg1 (%1) not a real number").arg(idbToString(first));
                return 0;
            }
            if (second.imag() != 0) {
                error = tr("lsh: arg2 (%1) not a real number").arg(idbToString(second));
                return 0;
            }

            if (floor(first.real()) != first.real()) {
                error = tr("lsh: arg1 (%1) not an integer").arg(idbToString(first));
                return 0;
            }

            if (floor(second.real()) != second.real()) {
                error = tr("lsh: arg2 (%1) not an integer").arg(idbToString(second));
                return 0;
            }

            return (int) first.real() << (int) second.real();
        } else {
            error = tr("lsh: expected 2 arguments, got %1").arg(args.length());
            return 0;
        }
    }, tr("Shifts a number to the left by a specified number of bits"), QStringList() << tr("number") + ":" + tr("The number to shift") << tr("amount") + ":" + tr("The number of bits to shift by")));
    customFunctions.insert("rsh", CustomFunction([ = ](QList<idouble> args, QString & error) -> idouble {
        if (args.length() == 2) {
            idouble first = args.first();
            idouble second = args.at(1);

            if (first.imag() != 0) {
                error = tr("rsh: arg1 (%1) not a real number").arg(idbToString(first));
                return 0;
            }
            if (second.imag() != 0) {
                error = tr("rsh: arg2 (%1) not a real number").arg(idbToString(second));
                return 0;
            }

            if (floor(first.real()) != first.real()) {
                error = tr("rsh: arg1 (%1) not an integer").arg(idbToString(first));
                return 0;
            }

            if (floor(second.real()) != second.real()) {
                error = tr("rsh: arg2 (%1) not an integer").arg(idbToString(second));
                return 0;
            }

            return (int) first.real() >> (int) second.real();
        } else {
            error = tr("rsh: expected 2 arguments, got %1").arg(args.length());
            return 0;
        }
    }, tr("Shifts a number to the right by a specified number of bits"), QStringList() << tr("number") + ":" + tr("The number to shift") << tr("amount") + ":" + tr("The number of bits to shift by")));
    customFunctions.insert("pow", CustomFunction([ = ](QList<idouble> args, QString & error) -> idouble {
        if (args.length() == 2) {
            idouble first = args.first();
            idouble second = args.at(1);

            if (first.real() == 0 && first.imag() == 0 &&
                second.real() <= 0 && second.imag() == 0) {
                error = tr("pow: arg2 (%1) out of bounds for arg1 (0) (should be positive)").arg(idbToString(second));
                return 0;
            }

            if (first.real() == 0 && first.imag() == 0 && second.real() == 0 && second.imag() != 0) {
                error = tr("pow: arg2 (%1) out of bounds for arg1 (0) (should be a real number)").arg(idbToString(second));
                return 0;
            }

            return pow(first, second);
        } else {
            error = tr("pow: expected 2 arguments, got %1").arg(args.length());
            return 0;
        }
    }, tr("Calculates an exponent"), QStringList() << tr("base") + ":" + tr("The base of the exponent") << tr("exponent") + ":" + tr("The number to exponentiate by")));
    customFunctions.insert("mod", CustomFunction([ = ](QList<idouble> args, QString & error) -> idouble {
        if (args.length() == 2) {
            idouble first = args.first();
            idouble second = args.at(1);

            if (floor(first.real()) != first.real() || first.imag() != 0) {
                error = tr("mod: arg1 (%1) not an integer").arg(idbToString(first));
                return 0;
            }

            if (floor(second.real()) != second.real() || second.imag() != 0) {
                error = tr("mod: arg2 (%1) not an integer").arg(idbToString(second));
                return 0;
            }

            if (second.real() == 0) {
                error = tr("mod: division by 0 undefined").arg(idbToString(second));
                return 0;
            }

            return (int) first.real() % (int) second.real();
        } else {
            error = tr("mod: expected 2 arguments, got %1").arg(args.length());
            return 0;
        }
    }, tr("Calculates the remainder when dividing two numbers"), QStringList() << tr("divisor") + ":" + tr("The number to be divided") << tr("dividend") + ":" + tr("The number to divide by")));

    customFunctions.insert("floor", createSingleArgFunction([ = ](idouble arg, QString & error) {
        return floor(arg.real());
    }, "floor", tr("Calculates the %1 of an %2").arg(tr("floor"), tr("number")), tr("number"), tr("The %1 to calculate the %2 of").arg(tr("number"), tr("floor"))));
    customFunctions.insert("ceil", createSingleArgFunction([ = ](idouble arg, QString & error) {
        return ceil(arg.real());
    }, "ceil", tr("Calculates the %1 of an %2").arg(tr("ceiling"), tr("number")), tr("number"), tr("The %1 to calculate the %2 of").arg(tr("number"), tr("ceiling"))));

    CustomFunction randomFunction([ = ](QList<idouble> args, QString & error) -> idouble {
        QRandomGenerator* gen = QRandomGenerator::system();

        if (args.length() == 0) {
            return idouble(gen->generate());
        } else if (args.length() == 1) {
            if (args.first().imag() != 0) {
                error = tr("random: arg1 (%1) not a real number").arg(idbToString(args.first()));
                return 0;
            }

            return idouble(gen->bounded((double) args.first().real()));
        } else if (args.length() == 2) {
            if (args.first().imag() != 0) {
                error = tr("random: arg1 (%1) not a real number").arg(idbToString(args.first()));
                return 0;
            }

            if (args.last().imag() != 0) {
                error = tr("random: arg2 (%1) not a real number").arg(idbToString(args.last()));
                return 0;
            }

            return idouble(gen->bounded((int) args.first().real(), (int) args.last().real()));
        } else {
            error = tr("random: expected 0, 1 or 2 arguments, got %1").arg(args.length());
            return 0;
        }
    }, tr("Returns a random number"), QStringList());
    randomFunction.addOverload(tr("Returns a random number in the range [0-number)"), QStringList() << tr("bound") + ":" + tr("The exclusive high bound"));
    randomFunction.addOverload(tr("Returns a random number in the range [low-high)"), QStringList() << tr("low") + ":" + tr("The inclusive low bound") << tr("high") + ":" + tr("The exclusive high bound"));
    customFunctions.insert("random", randomFunction);

    //Set up all the custom functions
    QSettings settings;
    settings.beginGroup("customFunctions");
    for (QString function : settings.allKeys()) {
        QJsonDocument doc = QJsonDocument::fromBinaryData(settings.value(function).toByteArray());
        if (doc.isObject()) {
            QJsonObject obj = doc.object();
            QJsonArray overloads = obj.value("overloads").toArray();

            CustomFunction func([ = ](QList<idouble> args, QString & error) -> idouble {
                QStringList argCounts;
                EvaluationEngine engine;
                QMap<QString, idouble> variables;

                for (QJsonValue o : overloads) {
                    QJsonObject overload = o.toObject();

                    QJsonArray fnArgs = overload.value("args").toArray();
                    if (fnArgs.count() == args.count()) {
                        //Found the correct overload to use
                        //Populate the arguments
                        for (int i = 0; i < args.count(); i++) {
                            QJsonValue arg = fnArgs.at(i);
                            if (arg.isObject()) {
                                variables.insert(arg.toObject().value("name").toString(), args.at(i));
                            } else {
                                variables.insert(arg.toString(), args.at(i));
                            }
                        }
                        engine.setVariables(variables);

                        //For each branch, evaluate the conditions and choose the branch
                        QJsonObject selectedBranch;
                        QJsonArray branches = overload.value("branches").toArray();
                        for (QJsonValue b : branches) {
                            QJsonObject branch = b.toObject();
                            if (branch.value("isOtherwise").toBool()) {
                                //Reached the final branch
                                selectedBranch = branch;
                                break;
                            } else {
                                bool currentConditionState = false;
                                QJsonArray conditions = branch.value("conditions").toArray();
                                for (QJsonValue c : conditions) {
                                    QJsonObject condition = c.toObject();
                                    engine.setExpression(condition.value("expression").toString());
                                    int connective = condition.value("connective").toInt();
                                    EvaluationEngine::Result result = engine.evaluate();
                                    bool boolResult;

                                    switch (result.type) {
                                        case EvaluationEngine::Result::Scalar:
                                            boolResult = result.result != idouble(0, 0);
                                            break;
                                        case EvaluationEngine::Result::Equality:
                                            boolResult = result.isTrue;
                                            break;
                                        case EvaluationEngine::Result::Error:
                                            //Return an error
                                            error = result.error;
                                            return 0;
                                        case EvaluationEngine::Result::Assign:
                                            //Return an error
                                            error = tr("%1: expected scalar or boolean value, got assignment").arg(function);
                                            return 0;
                                    }

                                    if (condition.value("isFirst").toBool()) {
                                        if (connective == 0) { //WHEN
                                            currentConditionState = boolResult;
                                        } else { //WHEN NOT
                                            currentConditionState = !boolResult;
                                        }
                                    } else {
                                        switch (connective) {
                                            case 0: //AND
                                                currentConditionState &= boolResult;
                                                break;
                                            case 1: //OR
                                                currentConditionState |= boolResult;
                                                break;
                                            case 2: //XOR
                                                currentConditionState ^= boolResult;
                                                break;
                                            case 3: //AND NOT
                                                currentConditionState &= !boolResult;
                                                break;
                                            case 4: //OR NOT
                                                currentConditionState |= !boolResult;
                                                break;
                                            case 5: //XOR NOT
                                                currentConditionState ^= !boolResult;
                                                break;
                                        }
                                    }
                                }

                                if (currentConditionState) { //This is the branch we need
                                    selectedBranch = branch;
                                    break;
                                }
                            }
                        }

                        //Now that we've selected a branch, evaluate the expression
                        QJsonObject retval = selectedBranch.value("return").toObject();
                        if (retval.value("isError").toBool()) {
                            //This is an error branch
                            error = function + ": " + retval.value("expression").toString();
                            return 0;
                        } else {
                            engine.setExpression(retval.value("expression").toString());
                            EvaluationEngine::Result result = engine.evaluate();

                            switch (result.type) {
                                case EvaluationEngine::Result::Scalar:
                                    return result.result;
                                case EvaluationEngine::Result::Equality:
                                    return idouble(result.isTrue);
                                case EvaluationEngine::Result::Error:
                                    //Return an error
                                    error = result.error;
                                    return 0;
                                case EvaluationEngine::Result::Assign:
                                    //Return an error
                                    error = tr("%1: expected scalar or boolean value, got assignment").arg(function);
                                    return 0;
                            }
                        }
                    } else {
                        argCounts.append(QString::number(fnArgs.count()));
                    }
                }

                QString argSpecification = argCounts.join(", ");
                if (argCounts.count() == 1) {
                    error = tr("%1: expected %n arguments, got %2", nullptr, argCounts.first().toInt()).arg(function, QString::number(args.length()));
                } else {
                    int last = argSpecification.lastIndexOf(", ");
                    argSpecification = argSpecification.replace(last, 2, " " + tr("or", "Expected 1, 2 or 3 arguments") + " ");
                    error = tr("%1: expected %2 arguments, got %3").arg(function, argSpecification, QString::number(args.length()));
                }

                return 0;
            });

            for (QJsonValue o : overloads) {
                QJsonObject overload = o.toObject();
                QStringList argList;

                QJsonArray args = overload.value("args").toArray();

                for (QJsonValue v : args) {
                    if (v.isObject()) {
                        QJsonObject o = v.toObject();
                        argList.append(o.value("name").toString() + ":" + o.value("desc").toString());
                    } else {
                        argList.append(v.toString() + ":");
                    }
                }

                func.addOverload(overload.value("desc").toString(), argList);
            }
            customFunctions.insert(function, func);
        }
    }
    settings.endGroup();
}

void EvaluationEngine::setTrigonometricUnit(TrigonometricUnit trigUnit) {
    QMutexLocker locker(trigUnitLocker);
    EvaluationEngine::trigUnit = trigUnit;
}

idouble EvaluationEngine::fromRad(idouble rad) {
    if (trigUnit == Degrees) {
        rad = idouble(rad.real() * 180 / M_PI, rad.imag());
        //rad *= idouble(180 / M_PI, 1);
    } else if (trigUnit == Gradians) {
        rad = idouble(rad.real() * 200 / M_PI, rad.imag());
        //rad *= idouble(200 / M_PI, 1);
    }
    return rad;
}

idouble EvaluationEngine::toRad(idouble deg) {
    if (trigUnit == Degrees) {
        //deg *= idouble(M_PI / 180, 1);
        deg = idouble(deg.real() * M_PI / 180, deg.imag());
    } else if (trigUnit == Gradians) {
        //deg *= idouble(M_PI / 200, 1);
        deg = idouble(deg.real() * M_PI / 200, deg.imag());
    }
    return deg;
}

CustomFunction EvaluationEngine::createSingleArgFunction(std::function<idouble (idouble, QString&)> fn, QString fnName, QString fnDesc, QString argName, QString argDesc) {
    return CustomFunction([ = ](QList<idouble> args, QString & error) -> idouble {
        if (args.length() != 1) {
            error = tr("%1: expected 1 argument, got %2").arg(fnName).arg(args.length());
            return 0;
        } else {
            return fn(args.first(), error);
        }
    }, fnDesc, QStringList() << argName + ":" + argDesc);
}
