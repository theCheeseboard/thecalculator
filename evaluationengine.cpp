#include "evaluationengine.h"

typedef void* yyscan_t;
#include "calc.bison.hpp"
#include "calc.yy.h"
#include "calc.h"

EvaluationEngine::EvaluationEngine(QObject *parent) : QObject(parent)
{

}

tPromise<EvaluationEngine::Result>* EvaluationEngine::evaluate(QString expression) {
    return new tPromise<Result>([=](QString& error) -> Result {
        Result* result = new Result();

        QString expr = expression;

        yyscan_t scanner;
        yylex_init(&scanner);
        YY_BUFFER_STATE bufferState = yy_scan_string(expr.append("\n").toUtf8().constData(), scanner);
        yyparse(scanner, [=](idouble r) { //Success
            result->result = r;
            result->type = Result::Scalar;
        }, [=](const char* s) { //Error
            result->error = QString::fromLocal8Bit(s);
            result->type = Result::Error;
        }, [=](QString identifier, idouble value) { //Assignment
            result->assigned = true;
            result->identifier = identifier;
            result->value = value;
            result->type = Result::Assign;
        }, [=](bool isTrue) { //Equality
            result->isTrue = isTrue;
            result->type = Result::Equality;
        });
        yy_delete_buffer(bufferState, scanner);
        yylex_destroy(scanner);

        Result retval = *result;
        delete result;
        return retval;
    });
}
