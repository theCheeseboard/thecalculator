#include "evaluationengine.h"

typedef void* yyscan_t;
#include "calc.bison.hpp"
#include "calc.yy.h"
#include "calc.h"

EvaluationEngine::EvaluationEngine(QObject *parent) : QObject(parent)
{

}

tPromise<EvaluationEngine::Result>* EvaluationEngine::evaluate(QString expression, QMap<QString, idouble> variables) {
    return new tPromise<Result>([=](QString& error) -> Result {
        Result* result = new Result();
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

        QString expr = expression;

        yylex_init(&p.scanner);
        YY_BUFFER_STATE bufferState = yy_scan_string(expr.append("\n").toUtf8().constData(), p.scanner);
        yyparse(p.scanner, p);
        yy_delete_buffer(bufferState, p.scanner);
        yylex_destroy(p.scanner);

        Result retval = *result;
        delete result;
        return retval;
    });
}

EvaluationEngine::Result EvaluationEngine::evaluate() {
    Result result;
    QEventLoop* loop = new QEventLoop();

    this->evaluate(expression, variables)->then([=, &result](Result r) {
        result = r;
        loop->quit();
    });
    loop->exec();

    return result;
}

void EvaluationEngine::setExpression(QString expression) {
    this->expression = expression;
}

void EvaluationEngine::setVariables(QMap<QString, idouble> vars) {
    this->variables = vars;
}
