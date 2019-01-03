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

%{
//Include headers
#include <QString>
#include <cstdio>
#include "mainwindow.h"
#include "evaluationengineheaders.h"
#include "evaluationengine.h"

extern MainWindow* MainWin;

idouble callFunction(QString name, QList<idouble> args, QString& error) {
    //qDebug() << "Calling function:" << name << "with arguments" << args;
    if (!EvaluationEngine::customFunctions.contains(name)) {
        error = QApplication::translate("EvaluationEngine", "%1: undefined function").arg(name);
        return 0;
    } else {
        return EvaluationEngine::customFunctions.value(name).getFunction()(args, error);
    }
}


bool valueExists(QString identifier, EvaluationEngineParameters p) {
    return p.variables.contains(identifier);
}

idouble getValue(QString identifier, EvaluationEngineParameters p) {
    return p.variables.value(identifier);
}

double absArg(idouble n) {
    double retval = abs(n);
    if ((n.real() == 0 && n.imag() < 0) || (abs(arg(n)) > M_PI_2)) {
        //Make it negative
        retval *= -1;
    }
    return retval;
}

//Define helper functions

#define YYE(desc) yyerror(scanner, p, desc);
#define CALL_MAINWINDOW_FUNCTION(arg1, arg2, result) { \
        QString error; \
        result = new idouble(callFunction(arg1, arg2, error)); \
        if (error != "") { \
            YYE(error.toUtf8().constData()); \
            YYABORT; \
        } \
    }
#define tr(str) QApplication::translate("EvaluationEngine", str)
%}

%define parse.error verbose
%define parse.lac full
%define api.pure full


%param { yyscan_t scanner }

%parse-param { EvaluationEngineParameters p }

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
    void yyerror(yyscan_t scanner, EvaluationEngineParameters p, const char* s);
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
%left ASSIGNMENT GREATER LESS GREATEREQUAL LESSEQUAL EQUALITY NOTEQUALITY

%%
line: expression EOL {
          p.resultFunction(*$1);
    }
|   IDENTIFIER ASSIGNMENT expression EOL { p.assignFunction(*$1, *$3); }
|   truefalse EOL {
        p.equalityFunction(*$1);
}

truefalse: expression EQUALITY expression { *$$ = (*$1 == *$3); }
|   expression ASSIGNMENT expression { *$$ = (*$1 == *$3); }
|   expression GREATER expression { *$$ = (absArg(*$1) > absArg(*$3)); }
|   expression LESS expression { *$$ = (absArg(*$1) < absArg(*$3)); }
|   expression GREATEREQUAL expression { *$$ = (absArg(*$1) >= absArg(*$3)); }
|   expression LESSEQUAL expression { *$$ = (absArg(*$1) <= absArg(*$3)); }
|   expression NOTEQUALITY expression { *$$ = (*$1 != *$3); }

expression: SUBTRACT expression {$$ = new idouble(-$2->real(), -$2->imag());}
|   NUMBER {$$ = new idouble(*$1);}
|   expression power {CALL_MAINWINDOW_FUNCTION("pow", QList<idouble>() << *$1 << *$2, $$)}
|   expression ADD expression {$$ = new idouble(*$1 + *$3);}
|   expression SUBTRACT expression {$$ = new idouble(*$1 - *$3);}
|   expression MULTIPLY expression {$$ = new idouble(*$1 * *$3);}
//|   NUMBER expression {$$ = new idouble(*$1 * *$2);}
|   expression DIVIDE expression {
        if ($3->real() == 0 && $3->imag() == 0) {
            YYE(tr("div: division by 0 undefined").toLocal8Bit().constData());
            YYABORT;
        } else {
            $$ = new idouble(*$1 / *$3);
        }
    }
|   expression NUMBER {$$ = new idouble(*$1 * *$2);}
|   expression PERCENT expression {CALL_MAINWINDOW_FUNCTION("mod", QList<idouble>() << *$1 << *$3, $$)}
|   LBRACKET expression RBRACKET {$$ = new idouble(*$2);}
|   expression PERCENT {$$ = new idouble(*$1 / idouble(100));}
|   expression EXPONENTIATE expression {CALL_MAINWINDOW_FUNCTION("pow", QList<idouble>() << *$1 << *$3, $$)}
|   IDENTIFIER EXPONENTIATE expression {
        if (valueExists(*$1, p)) {
            $$ = new idouble(pow(getValue(*$1, p), *$3));
        } else {
            YYE(tr("%1: unknown variable").arg(*$1).toLocal8Bit().constData());
            YYABORT;
        }
    }
|   IDENTIFIER power {
        if (valueExists(*$1, p)) {
            $$ = new idouble(pow(getValue(*$1, p), *$2));
        } else {
            YYE(tr("%1: unknown variable").arg(*$1).toLocal8Bit().constData());
            YYABORT;
        }
    }
|   expression LSH expression {CALL_MAINWINDOW_FUNCTION("lsh", QList<idouble>() << *$1 << *$3, $$)}
|   expression RSH expression {CALL_MAINWINDOW_FUNCTION("rsh", QList<idouble>() << *$1 << *$3, $$)}
|   expression FACTORIAL {CALL_MAINWINDOW_FUNCTION("fact", QList<idouble>() << *$1, $$)}
|   RADICAL expression {$$ = new idouble(sqrt(*$2));}
|   power RADICAL expression {
        CALL_MAINWINDOW_FUNCTION("root", QList<idouble>() << *$3 << *$1, $$)
    }
|   function
|   IDENTIFIER {
        if (valueExists(*$1, p)) {
            $$ = new idouble(getValue(*$1, p));
        } else {
            YYE(tr("%1: unknown variable").arg(*$1).toLocal8Bit().constData());
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

void yyerror(yyscan_t scanner, EvaluationEngineParameters p, const char* s) {
    p.errorFunction(s);
}
