#ifndef EVALUATIONENGINEHEADERS_H
#define EVALUATIONENGINEHEADERS_H

#include <QMap>
#include <complex>
#include <functional>

typedef std::complex<long double> idouble;
typedef void* yyscan_t;

struct EvaluationEngineParameters {
    yyscan_t scanner;
    std::function<void(idouble)> resultFunction;
    std::function <void(const char*)> errorFunction;
    std::function<void(QString, idouble)> assignFunction;
    std::function<void(bool)> equalityFunction;
    QMap<QString, idouble> variables;
};

#endif // EVALUATIONENGINEHEADERS_H
