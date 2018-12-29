#ifndef EVALUATIONENGINE_H
#define EVALUATIONENGINE_H

#include <QObject>
#include <complex>
#include <tpromise.h>
#include "evaluationengineheaders.h"

typedef std::function<idouble(QList<idouble>,QString&)> CustomFunctionDefinition;
class CustomFunction {
    public:
        CustomFunction();
        CustomFunction(CustomFunctionDefinition function);

        CustomFunctionDefinition getFunction() const;

    private:
        CustomFunctionDefinition fn;
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
            Radians
        };

        static void setupFunctions();
        static std::function<idouble(QList<idouble>,QString&)> createSingleArgFunction(std::function<idouble(idouble, QString&)> fn, QString fnName);

        static CustomFunctionMap customFunctions;

        struct Result {
            enum ResultType {
                Error,
                Scalar,
                Assign,
                Equality
            };

            QString error;

            idouble result;

            bool assigned;
            QString identifier;
            idouble value;

            bool isTrue;

            ResultType type;
        };

        static tPromise<Result>* evaluate(QString expression, QMap<QString, idouble> variables);
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
        static idouble toDeg(idouble rad);
};

#endif // EVALUATIONENGINE_H
