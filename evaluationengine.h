#ifndef EVALUATIONENGINE_H
#define EVALUATIONENGINE_H

#include <QObject>
#include <complex>
#include <tpromise.h>
#include "evaluationengineheaders.h"


class EvaluationEngine : public QObject
{
        Q_OBJECT
    public:
        explicit EvaluationEngine(QObject *parent = nullptr);
        ~EvaluationEngine();

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

    signals:

    public slots:

    private:
        QString expression;
        QMap<QString, idouble> variables;

        static int runningEngines;
        static QMutex* runningEnginesLocker;
};

#endif // EVALUATIONENGINE_H
