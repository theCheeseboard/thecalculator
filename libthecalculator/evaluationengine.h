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

#ifndef EVALUATIONENGINE_H
#define EVALUATIONENGINE_H

#include <QObject>
#include <complex>
#include <tpromise.h>
#include "evaluationengineheaders.h"

QString numberFormatToString(long double number);
QString idbToString(idouble db);
uint qHash(const idouble& key);

struct CustomFunctionPrivate;
typedef std::function<idouble(QList<idouble>,QString&)> CustomFunctionDefinition;

class CustomFunction {
    public:
        CustomFunction();
        CustomFunction(CustomFunctionDefinition function);
        CustomFunction(CustomFunctionDefinition function, QString desc, QStringList args);

        CustomFunctionDefinition getFunction() const;
        QString getDescription(int overload = 0) const;
        QStringList getArgs(int overload = 0) const;

        void addOverload(QString desc, QStringList args);
        int overloads();

    private:
        QSharedPointer<CustomFunctionPrivate> d;
};

typedef QMap<QString, CustomFunction> CustomFunctionMap;

class EvaluationEngine : public QObject
{
        Q_OBJECT
    public:
        explicit EvaluationEngine(QObject *parent = nullptr);
        ~EvaluationEngine();

        enum TrigonometricUnit {
            Degrees,
            Radians,
            Gradians
        };

        static void setupFunctions();
        static CustomFunction createSingleArgFunction(std::function<idouble(idouble, QString&)> fn, QString fnName, QString fnDesc = "", QString argName = "", QString argDesc = "");

        static CustomFunctionMap customFunctions;

        struct Result {
            enum ResultType {
                Error,
                Scalar,
                Assign,
                Equality
            };

            QString error;
            int location;
            int length;

            idouble result;

            bool assigned;
            QString identifier;
            idouble value;

            bool isTrue;

            ResultType type;
        };

        static tPromise<Result>* evaluate(QString expression, QMap<QString, idouble> variables = QMap<QString, idouble>());
        Result evaluate();
        void setExpression(QString expression);
        void setVariables(QMap<QString, idouble> vars);
        static void setTrigonometricUnit(TrigonometricUnit trigUnit);

    signals:

    public slots:

    private:
        QString expression;
        QMap<QString, idouble> variables;
        static TrigonometricUnit trigUnit;

        static int runningEngines;
        static QMutex *runningEnginesLocker, *trigUnitLocker;


        static idouble toRad(idouble deg);
        static idouble fromRad(idouble rad);
};

#endif // EVALUATIONENGINE_H
