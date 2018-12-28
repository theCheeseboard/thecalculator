#ifndef EVALUATIONENGINE_H
#define EVALUATIONENGINE_H

#include <QObject>
#include <complex>
#include <tpromise.h>
//#include "calc.bison.hpp"
typedef void* yyscan_t;

typedef std::complex<long double> idouble;

struct EvaluationEngineParameters {
    yyscan_t scanner;
    std::function<void(idouble)> resultFunction;
    std::function <void(const char*)> errorFunction;
    std::function<void(QString, idouble)> assignFunction;
    std::function<void(bool)> equalityFunction;
    QMap<QString, idouble> variables;
};


class EvaluationEngine : public QObject
{
        Q_OBJECT
    public:
        explicit EvaluationEngine(QObject *parent = nullptr);

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
};

#endif // EVALUATIONENGINE_H
