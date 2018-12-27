#ifndef EVALUATIONENGINE_H
#define EVALUATIONENGINE_H

#include <QObject>
#include <complex>
#include <tpromise.h>

typedef std::complex<long double> idouble;

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

        static tPromise<Result>* evaluate(QString expression);

    signals:

    public slots:
};

#endif // EVALUATIONENGINE_H
