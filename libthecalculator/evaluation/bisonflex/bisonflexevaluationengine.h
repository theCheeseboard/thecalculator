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

#ifndef BISONFLEXEVALUATIONENGINE_H
#define BISONFLEXEVALUATIONENGINE_H

#include "../baseevaluationengine.h"
#include "idouble.h"
#include "libthecalculator_global.h"
#include <QCoroTask>
#include <QMutex>
#include <QObject>
#include <complex>

LIBTHECALCULATOR_EXPORT QString numberFormatToString(long double number);

struct CustomFunctionPrivate;

class LIBTHECALCULATOR_EXPORT BisonFlexCustomFunction : public CustomFunction {
    public:
        BisonFlexCustomFunction();
        BisonFlexCustomFunction(CustomFunctionDefinition function);
        BisonFlexCustomFunction(CustomFunctionDefinition function, QString desc, QStringList args);

        CustomFunctionDefinition getFunction() const;
        QString getDescription(int overload = 0) const;
        QStringList getArgs(int overload = 0) const;

        void addOverload(QString desc, QStringList args);
        int overloads();

    private:
        QSharedPointer<CustomFunctionPrivate> d;
};

struct BisonFlexEvaluationEnginePrivate;
class BisonFlexEvaluationEngine : public BaseEvaluationEngine {
        Q_OBJECT
    public:
        explicit BisonFlexEvaluationEngine(QObject* parent = nullptr);
        ~BisonFlexEvaluationEngine();

        void setupFunctions();
        static CustomFunctionPtr createSingleArgFunction(std::function<idouble(idouble, QString&)> fn, QString fnName, QString fnDesc = "", QString argName = "", QString argDesc = "");

        CustomFunctionMap customFunctions() const;

        QCoro::Task<Result> evaluate(QString expression, QMap<QString, idouble> variables = QMap<QString, idouble>());
        Result evaluate();
        void setExpression(QString expression);
        void setVariables(QMap<QString, idouble> vars);

    signals:

    public slots:

    private:
        BisonFlexEvaluationEnginePrivate* d;

        static idouble toRad(idouble deg);
        static idouble fromRad(idouble rad);
};

#endif // BISONFLEXEVALUATIONENGINE_H
