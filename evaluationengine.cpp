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

CustomFunction::CustomFunction() {

}

CustomFunction::CustomFunction(CustomFunctionDefinition fn) {
    this->fn = fn;
}

CustomFunctionDefinition CustomFunction::getFunction() const {
    return this->fn;
}

EvaluationEngine::EvaluationEngine(QObject *parent) : QObject(parent)
{

}

EvaluationEngine::~EvaluationEngine() {

}

tPromise<EvaluationEngine::Result>* EvaluationEngine::evaluate(QString expression, QMap<QString, idouble> variables) {
    return new tPromise<Result>([=](QString& error) -> Result {
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
    EvaluationEngineParameters p;
    p.resultFunction = [=](idouble r) { //Success
        result->result = r;
        result->type = Result::Scalar;
    };
    p.errorFunction = [=](const char* s) { //Error
        result->error = QString::fromLocal8Bit(s);
        result->type = Result::Error;
    };
    p.assignFunction = [=](QString identifier, idouble value) { //Assignment
        result->assigned = true;
        result->identifier = identifier;
        result->value = value;
        result->type = Result::Assign;
    };
    p.equalityFunction = [=](bool isTrue) { //Equality
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
    customFunctions.insert("abs", createSingleArgFunction([=](idouble arg, QString& error) {
        return abs(arg);
    }, "abs"));
    customFunctions.insert("sqrt", createSingleArgFunction([=](idouble arg, QString& error) {
        return sqrt(arg);
    }, "sqrt"));
    customFunctions.insert("cbrt", createSingleArgFunction([=](idouble arg, QString& error) {
        return pow(arg, 1 / (float) 3);
    }, "cbrt"));
    customFunctions.insert("fact", createSingleArgFunction([=](idouble arg, QString& error) -> idouble {
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
    }, "abs"));
    customFunctions.insert("sin", createSingleArgFunction([=](idouble arg, QString& error) {
        Q_UNUSED(error)
        return sin(toRad(arg));
    }, "sin"));
    customFunctions.insert("cos", createSingleArgFunction([=](idouble arg, QString& error) {
        Q_UNUSED(error)
        return cos(toRad(arg));
    }, "cos"));
    customFunctions.insert("tan", createSingleArgFunction([=](idouble arg, QString& error) -> idouble {
        Q_UNUSED(error)

        if (trigUnit == Degrees) {
            if (fmod(arg.real() - 90, 180) == 0) {
                error = tr("tan: input (%1) out of bounds (not 90° + 180n)").arg(idbToString(arg));
                return 0;
            }
        } else {
            if (fmod(arg.real() - (M_PI / 2), M_PI) == 0) {
                error = tr("tan: input (%1) out of bounds (not π/2 + πn)").arg(idbToString(arg));
                return 0;
            }
        }

        return tan(toRad(arg));
    }, "tan"));
    customFunctions.insert("conj", createSingleArgFunction([=](idouble arg, QString& error) {
        Q_UNUSED(error)
        return conj(arg);
    }, "conj"));
    customFunctions.insert("im", createSingleArgFunction([=](idouble arg, QString& error) {
        Q_UNUSED(error)
        return arg.imag();
    }, "im"));
    customFunctions.insert("re", createSingleArgFunction([=](idouble arg, QString& error) {
        Q_UNUSED(error)
        return arg.real();
    }, "re"));
    customFunctions.insert("asin", createSingleArgFunction([=](idouble arg, QString& error) -> idouble {
        if (abs(arg) > 1) {
            error = tr("asin: input (%1) out of bounds (between -1 and 1)").arg(idbToString(arg));
            return 0;
        } else {
            return toDeg(asin(arg));
        }
    }, "asin"));
    customFunctions.insert("acos", createSingleArgFunction([=](idouble arg, QString& error) -> idouble {
        if (abs(arg) > 1) {
            error = tr("acos: input (%1) out of bounds (between -1 and 1)").arg(idbToString(arg));
            return 0;
        } else {
            return toDeg(acos(arg));
        }
    }, "acos"));
    customFunctions.insert("atan", createSingleArgFunction([=](idouble arg, QString& error) -> idouble {
        Q_UNUSED(error)
        return toDeg(atan(arg));
    }, "acos"));
    customFunctions.insert("sec", createSingleArgFunction([=](idouble arg, QString& error) {
        Q_UNUSED(error)
        return idouble(1) / cos(toRad(arg));
    }, "sec"));
    customFunctions.insert("csc", createSingleArgFunction([=](idouble arg, QString& error) {
        Q_UNUSED(error)
        return idouble(1) / sin(toRad(arg));
    }, "csc"));
    customFunctions.insert("cot", createSingleArgFunction([=](idouble arg, QString& error) {
        Q_UNUSED(error)
        return idouble(1) / tan(toRad(arg));
    }, "cot"));
    customFunctions.insert("asec", createSingleArgFunction([=](idouble arg, QString& error) {
        Q_UNUSED(error)
        return toDeg(acos(idouble(1) / arg));
    }, "asec"));
    customFunctions.insert("acsc", createSingleArgFunction([=](idouble arg, QString& error) {
        Q_UNUSED(error)
        return toDeg(asin(idouble(1) / arg));
    }, "acsc"));
    customFunctions.insert("acot", createSingleArgFunction([=](idouble arg, QString& error) {
        Q_UNUSED(error)
        return toDeg(atan(idouble(1) / arg));
    }, "acot"));
    customFunctions.insert("sinh", createSingleArgFunction([=](idouble arg, QString& error) {
        Q_UNUSED(error)
        return sinh(arg);
    }, "sinh"));
    customFunctions.insert("cosh", createSingleArgFunction([=](idouble arg, QString& error) {
        Q_UNUSED(error)
        return cosh(arg);
    }, "cosh"));
    customFunctions.insert("tanh", createSingleArgFunction([=](idouble arg, QString& error) {
        Q_UNUSED(error)
        return tanh(arg);
    }, "tanh"));
    customFunctions.insert("asinh", createSingleArgFunction([=](idouble arg, QString& error) {
        Q_UNUSED(error)
        return asinh(arg);
    }, "asinh"));
    customFunctions.insert("acosh", createSingleArgFunction([=](idouble arg, QString& error) {
        Q_UNUSED(error)
        return acosh(arg);
    }, "acosh"));
    customFunctions.insert("atanh", createSingleArgFunction([=](idouble arg, QString& error) {
        Q_UNUSED(error)
        return atanh(arg);
    }, "atanh"));

    customFunctions.insert("log", CustomFunction([=](QList<idouble> args, QString& error) -> idouble {
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
    }));
    customFunctions.insert("ln", createSingleArgFunction([=](idouble arg, QString& error) -> idouble {
        if (arg.real() == 0 && arg.imag() == 0) {
            error = tr("ln: input (%1) out of bounds (not 0)").arg(idbToString(arg));
            return 0;
        }

        return log(arg);
    }, "ln"));

    customFunctions.insert("lsh", CustomFunction([=](QList<idouble> args, QString& error) -> idouble {
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
                error = tr("lsh: arg2 (%2) not an integer").arg(idbToString(second));
                return 0;
            }

            return (int) first.real() << (int) second.real();
        } else {
            error = tr("lsh: expected 2 arguments, got %1").arg(args.length());
            return 0;
        }
    }));
    customFunctions.insert("rsh", CustomFunction([=](QList<idouble> args, QString& error) -> idouble {
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
                error = tr("rsh: arg2 (%2) not an integer").arg(idbToString(second));
                return 0;
            }

            return (int) first.real() >> (int) second.real();
        } else {
            error = tr("rsh: expected 2 arguments, got %1").arg(args.length());
            return 0;
        }
    }));
    customFunctions.insert("pow", CustomFunction([=](QList<idouble> args, QString& error) -> idouble {
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
    }));

    customFunctions.insert("floor", createSingleArgFunction([=](idouble arg, QString& error) {
        return floor(arg.real());
    }, "floor"));
    customFunctions.insert("ceil", createSingleArgFunction([=](idouble arg, QString& error) {
        return ceil(arg.real());
    }, "ceil"));

    customFunctions.insert("random", CustomFunction([=](QList<idouble> args, QString& error) -> idouble {
        QRandomGenerator gen;

        if (args.length() == 0) {
            return idouble(gen.generate());
        } else if (args.length() == 1) {
            if (args.first().imag() != 0) {
                error = tr("random: arg1 (%1) not a real number").arg(idbToString(args.first()));
                return 0;
            }

            return idouble(gen.bounded((double) args.first().real()));
        } else if (args.length() == 2) {
            if (args.first().imag() != 0) {
                error = tr("random: arg1 (%1) not a real number").arg(idbToString(args.first()));
                return 0;
            }

            if (args.last().imag() != 0) {
                error = tr("random: arg2 (%1) not a real number").arg(idbToString(args.last()));
                return 0;
            }

            return idouble(gen.bounded((int) args.first().real(), (int) args.last().real()));
        } else {
            error = tr("random: expected 0, 1 or 2 arguments, got %1").arg(args.length());
            return 0;
        }
    }));

    //Set up all the custom functions
    QSettings settings;
    settings.beginGroup("customFunctions");
    for (QString function : settings.allKeys()) {
        QJsonDocument doc = QJsonDocument::fromBinaryData(settings.value(function).toByteArray());
        if (doc.isObject()) {
            QJsonObject obj = doc.object();
            customFunctions.insert(function, CustomFunction([=](QList<idouble> args, QString& error) -> idouble {
                QStringList argCounts;
                EvaluationEngine engine;
                QMap<QString, idouble> variables;

                QJsonArray overloads = obj.value("overloads").toArray();
                for (QJsonValue o : overloads) {
                    QJsonObject overload = o.toObject();

                    QJsonArray fnArgs = overload.value("args").toArray();
                    if (fnArgs.count() == args.count()) {
                        //Found the correct overload to use
                        //Populate the arguments
                        for (int i = 0; i < args.count(); i++) {
                            variables.insert(fnArgs.at(i).toString(), args.at(i));
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
            }));
        }
    }
    settings.endGroup();
}

void EvaluationEngine::setTrigonometricUnit(TrigonometricUnit trigUnit) {
    QMutexLocker locker(trigUnitLocker);
    EvaluationEngine::trigUnit = trigUnit;
}

idouble EvaluationEngine::toDeg(idouble rad) {
    if (trigUnit == Degrees) {
        rad *= idouble(180 / M_PI);
    }
    return rad;
}

idouble EvaluationEngine::toRad(idouble deg) {
    if (trigUnit == Degrees) {
        deg *= idouble(M_PI / 180);
    }
    return deg;
}

std::function<idouble(QList<idouble>,QString&)> EvaluationEngine::createSingleArgFunction(std::function<idouble (idouble, QString &)> fn, QString fnName) {
    return [=](QList<idouble> args, QString& error) -> idouble {
        if (args.length() != 1) {
            error = tr("%1: expected 1 argument, got %2").arg(fnName).arg(args.length());
            return 0;
        } else {
            return fn(args.first(), error);
        }
    };
}
