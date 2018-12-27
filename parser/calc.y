%{
//Include headers
#include <QString>
#include <cstdio>
#include <complex>
#include <functional>
#include "mainwindow.h"
#include "calc.yy.h"

extern MainWindow* MainWin;
extern QMap<QString, std::function<idouble(QList<idouble>,QString&)>> customFunctions;
extern QMap<QString, idouble> variables;
extern bool explicitEvaluation;
extern bool resSuccess;

idouble callFunction(QString name, QList<idouble> args, QString& error) {
    //qDebug() << "Calling function:" << name << "with arguments" << args;
    if (!customFunctions.contains(name)) {
        error = QApplication::translate("parser", "%1: undefined function").arg(name);
        return 0;
    } else {
        return customFunctions.value(name)(args, error);
    }
}


bool valueExists(QString identifier) {
    return variables.contains(identifier);
}

idouble getValue(QString identifier) {
    return variables.value(identifier);
}

//Define helper functions

#define YYE(desc) yyerror(scanner, resultFunction, errorFunction, assignFunction, equalityFunction, desc);
#define CALL_MAINWINDOW_FUNCTION(arg1, arg2, result) { \
        QString error; \
        result = new idouble(callFunction(arg1, arg2, error)); \
        if (error != "") { \
            YYE(error.toUtf8().constData()); \
            YYABORT; \
        } \
    }

%}

%define parse.error verbose
%define parse.lac full
%define api.pure full

%param { yyscan_t scanner }

%parse-param { std::function<void(idouble)> resultFunction }
             { std::function <void(const char*)> errorFunction }
             { std::function<void(QString, idouble)> assignFunction }
             { std::function<void(bool)> equalityFunction }

%union {
    QString* string;
    idouble* number;
    QList<idouble>* arguments;
    bool* boolean;
}

%{
    //Define flex functions
    extern int yylex(YYSTYPE* yylvalp, yyscan_t scanner);
    extern void yyterminate();
    void yyerror(yyscan_t scanner, std::function<void(idouble)> resultFunction, std::function <void(const char*)> errorFunction, std::function<void(QString, idouble)> assignFunction, std::function<void(bool)> equalityFunction, const char* s);

    void assignValue(QString identifier, idouble value, std::function<void(QString, idouble)> assignFunction) {
        if (explicitEvaluation) {
            variables.insert(identifier, value);
        }

        if (MainWin != nullptr) {
            assignFunction(identifier, value);
        }

        resSuccess = true;
    }
%}

%token<number> NUMBER SUPER
%token<number> LBRACKET RBRACKET
%token<number> ADD SUBTRACT MULTIPLY DIVIDE LSH RSH SUPERSUBTRACT SUPERADD
%token<number> PERCENT ABSOLUTE ENDABSOLUTE FACTORIAL RADICAL
%token<number> EOL
%token<number> ARGSEPARATOR
%token<number> ASSIGNMENT
%token<string> IDENTIFIER

%type<number> expression
%type<number> function
%type<number> power
%type<number> line
%type<boolean> truefalse
%type<arguments> arguments

%destructor { delete $$; } <string>
%destructor { delete $$; } <arguments>
//%destructor { delete $$; } <number>

//Set operator precedence
%left ADD SUBTRACT
%left MULTIPLY DIVIDE
%left LSH RSH
%left EXPONENTIATE RADICAL SUPER
%left FACTORIAL PERCENT
%left LBRACKET RBRACKET
%left ASSIGNMENT GREATER LESS EQUALITY

%%
line: expression EOL {
          resSuccess = true;
          resultFunction(*$1);
    }
|   IDENTIFIER ASSIGNMENT expression EOL { assignValue(*$1, *$3, assignFunction); }
|   truefalse EOL {
        resSuccess = true;
        equalityFunction(*$1);
}

truefalse: expression EQUALITY expression { *$$ = (*$1 == *$3); }
|   expression ASSIGNMENT expression { *$$ = (*$1 == *$3); }
|   expression GREATER expression { *$$ = (abs(*$1) > abs(*$3)); }
|   expression LESS expression { *$$ = (abs(*$1) < abs(*$3)); }

expression: SUBTRACT expression {$$ = new idouble(-$2->real(), -$2->imag());}
|   NUMBER {$$ = new idouble(*$1);}
|   expression power {CALL_MAINWINDOW_FUNCTION("pow", QList<idouble>() << *$1 << *$2, $$)}
|   expression ADD expression {$$ = new idouble(*$1 + *$3);}
|   expression SUBTRACT expression {$$ = new idouble(*$1 - *$3);}
|   expression MULTIPLY expression {$$ = new idouble(*$1 * *$3);}
//|   NUMBER expression {$$ = new idouble(*$1 * *$2);}
|   expression DIVIDE expression {
        if ($3->real() == 0 && $3->imag() == 0) {
            YYE("div: division by 0 undefined");
            YYABORT;
        } else {
            $$ = new idouble(*$1 / *$3);
        }
    }
|   LBRACKET expression RBRACKET {$$ = new idouble(*$2);}
|   expression PERCENT {$$ = new idouble(*$1 / idouble(100));}
|   expression EXPONENTIATE expression {CALL_MAINWINDOW_FUNCTION("pow", QList<idouble>() << *$1 << *$3, $$)}
|   IDENTIFIER EXPONENTIATE expression {
        if (valueExists(*$1)) {
            $$ = new idouble(pow(getValue(*$1), *$3));
        } else {
            YYE((*$1).append(": unknown variable").toLocal8Bit().constData());
            YYABORT;
        }
    }
|   IDENTIFIER power {
        if (valueExists(*$1)) {
            $$ = new idouble(pow(getValue(*$1), *$2));
        } else {
            YYE((*$1).append(": unknown variable").toLocal8Bit().constData());
            YYABORT;
        }
    }
|   expression LSH expression {CALL_MAINWINDOW_FUNCTION("lsh", QList<idouble>() << *$1 << *$3, $$)}
|   expression RSH expression {CALL_MAINWINDOW_FUNCTION("rsh", QList<idouble>() << *$1 << *$3, $$)}
|   expression FACTORIAL {CALL_MAINWINDOW_FUNCTION("fact", QList<idouble>() << *$1, $$)}
|   RADICAL expression {$$ = new idouble(sqrt(*$2));}
|   power RADICAL expression {CALL_MAINWINDOW_FUNCTION("pow", QList<idouble>() << *$3 << idouble(1.0) / *$1, $$)}
|   function
|   IDENTIFIER {
        if (valueExists(*$1)) {
            $$ = new idouble(getValue(*$1));
        } else {
            YYE((*$1).append(": unknown variable").toLocal8Bit().constData());
            YYABORT;
        }
    }

power: SUPER
|   SUPERSUBTRACT power {$$ = new idouble(-$2->real(), $2->imag());}
|   power SUPERADD power {$$ = new idouble(*$1 + *$3);}
|   power SUPERSUBTRACT power {$$ = new idouble(*$1 - *$3);}

arguments: expression {$$ = new QList<idouble>(); $$->append(*$1);}
|   arguments ARGSEPARATOR expression {$$ = new QList<idouble>(*$1); $$->append(*$3);}
|   %empty {$$ = new QList<idouble>();}

function: IDENTIFIER LBRACKET arguments RBRACKET {CALL_MAINWINDOW_FUNCTION(*$1, *$3, $$)}
|   IDENTIFIER EXPONENTIATE expression LBRACKET arguments RBRACKET {
        idouble* result;
        CALL_MAINWINDOW_FUNCTION(*$1, *$5, result)
        CALL_MAINWINDOW_FUNCTION("pow", QList<idouble>() << *result << *$3, $$);
        delete result;
    }
|   IDENTIFIER power LBRACKET arguments RBRACKET {
        idouble* result;
        CALL_MAINWINDOW_FUNCTION(*$1, *$4, result)
        CALL_MAINWINDOW_FUNCTION("pow", QList<idouble>() << *result << *$2, $$);
        delete result;
    }
%%

void yyerror(yyscan_t scanner, std::function<void(idouble)> resultFunction, std::function <void(const char*)> errorFunction, std::function<void(QString, idouble)> assignFunction, std::function<void(bool)> equalityFunction, const char* s) {
    resSuccess = false;

    errorFunction(s);
}
