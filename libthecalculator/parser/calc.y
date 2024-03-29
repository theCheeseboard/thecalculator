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

// *INDENT-OFF*

%{
//Include headers
#include <QString>
#include <QtGlobal>
#include <cstdio>
#include <QApplication>
#include "evaluationengineheaders.h"
#include "evaluation/bisonflex/bisonflexevaluationengine.h"

extern QString idbToString(idouble db);

#ifdef _MSC_VER
#pragma runtime_checks( "", off )
#endif

idouble callFunction(QString name, QList<idouble> args, QString& error) {
    //qDebug() << "Calling function:" << name << "with arguments" << args;
    auto engine = BaseEvaluationEngine::current();
    if (!engine->customFunctions().contains(name)) {
        error = QApplication::translate("BisonFlexEvaluationEngine", "%1: undefined function").arg(name);
        return 0;
    } else {
        return engine->customFunctions().value(name)->getFunction()(args, error);
    }
}


bool valueExists(QString identifier, EvaluationEngineParameters p) {
    return p.variables.contains(identifier) || p.builtinVariables.contains(identifier);
}

idouble getValue(QString identifier, EvaluationEngineParameters p) {
    if (p.builtinVariables.contains(identifier)) return p.builtinVariables.value(identifier);
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

#define YYE(loc, desc) yyerror(&loc, scanner, p, desc);
#define CALL_MAINWINDOW_FUNCTION(loc, arg1, arg2, result) { \
        QString error; \
        result = new idouble(callFunction(arg1, arg2, error)); \
        if (error != "") { \
            YYE(loc, error.toUtf8().constData()); \
            YYABORT; \
        } \
    }
#define tr(str) QApplication::translate("BisonFlexEvaluationEngine", str)
#define CHECK_VALID_BITWISE(name, arg1, arg2, arg1loc, arg2loc) \
    bool shouldCalc = false; \
    if (!qFuzzyIsNull(static_cast<double>((arg1).imag())) || static_cast<int>((arg1).real()) != (arg1).real()) { \
        YYE(arg1loc, tr("Bitwise operations can only be performed on integers").toLocal8Bit().constData()); \
    } else if (!qFuzzyIsNull(static_cast<double>((arg2).imag())) || static_cast<int>((arg2).real()) != (arg2).real()) { \
        YYE(arg2loc, tr("Bitwise operations can only be performed on integers").toLocal8Bit().constData()); \
    } else { \
        shouldCalc = true; \
    } \
    if (!shouldCalc) {YYABORT;}
#define EXTRACT_BITWISE_ARGS(arg1, arg2) \
    qulonglong a1, a2; \
    a1 = static_cast<qulonglong>(arg1->real()); \
    a2 = static_cast<qulonglong>(arg2->real());

# define YYLLOC_DEFAULT(Current, Rhs, N) \
    if (N) { \
        (Current).location = YYRHSLOC(Rhs, 1).location; \
        (Current).length = 0; \
        for (int i = 1; i <= N; i++) { \
            (Current).length += YYRHSLOC(Rhs, i).length; \
        } \
    }
%}

%define parse.error verbose
%define parse.lac full
%define api.pure full
%locations

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
    extern int yylex(YYSTYPE* yylvalp, YYLTYPE* yylloc, yyscan_t scanner);
    extern void yyterminate();
    void yyerror(YYLTYPE* yylloc, yyscan_t scanner, EvaluationEngineParameters p, const char* s);
%}

%token<number> NUMBER SUPER
%token<number> LBRACKET RBRACKET
%token<number> NOT
%token<number> ADD SUBTRACT MULTIPLY DIVIDE LSH RSH SUPERSUBTRACT SUPERADD
%token<number> OR AND XOR NOR NAND XNOR
%token<number> PERCENT ABSOLUTE ENDABSOLUTE FACTORIAL RADICAL
%token<number> EOL TOKEN
%token<number> ARGSEPARATOR
%token<number> ASSIGNMENT
%token<string> IDENTIFIER

%type<number> expression
%type<number> exprid
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
|   expression power {CALL_MAINWINDOW_FUNCTION(@$, "pow", QList<idouble>() << *$1 << *$2, $$)}
|   expression ADD expression {$$ = new idouble(*$1 + *$3);}
|   expression SUBTRACT expression {$$ = new idouble(*$1 - *$3);}
|   expression MULTIPLY expression {$$ = new idouble(*$1 * *$3);}
|   NUMBER exprid {$$ = new idouble(*$1 * *$2);}
|   NUMBER LBRACKET expression RBRACKET {$$ = new idouble(*$1 * *$2);}
//|   NUMBER expression {$$ = new idouble(*$1 * *$2);}
|   expression DIVIDE expression {
        if ($3->real() == 0 && $3->imag() == 0) {
            YYE(@3, tr("Can't divide by zero").toLocal8Bit().constData());
            YYABORT;
        } else {
            $$ = new idouble(*$1 / *$3);
        }
    }
|   expression NUMBER {$$ = new idouble(*$1 * *$2);}
|   expression PERCENT expression {CALL_MAINWINDOW_FUNCTION(@$, "mod", QList<idouble>() << *$1 << *$3, $$)}
|   LBRACKET expression RBRACKET {$$ = new idouble(*$2);}
|   expression PERCENT {$$ = new idouble(*$1 / idouble(100));}
|   expression EXPONENTIATE expression {CALL_MAINWINDOW_FUNCTION(@$, "pow", QList<idouble>() << *$1 << *$3, $$)}
|   expression LSH expression {CALL_MAINWINDOW_FUNCTION(@$, "lsh", QList<idouble>() << *$1 << *$3, $$)}
|   expression RSH expression {CALL_MAINWINDOW_FUNCTION(@$, "rsh", QList<idouble>() << *$1 << *$3, $$)}
|   expression AND expression {
        CHECK_VALID_BITWISE("and", *$1, *$3, @1, @3)
        if (shouldCalc) {
            EXTRACT_BITWISE_ARGS($1, $3);
            $$ = new idouble(a1 & a2);
        }
    }
|   expression OR expression {
        CHECK_VALID_BITWISE("or", *$1, *$3, @1, @3)
        if (shouldCalc) {
            EXTRACT_BITWISE_ARGS($1, $3);
            $$ = new idouble(a1 | a2);
        }
    }
|   expression XOR expression {
        CHECK_VALID_BITWISE("xor", *$1, *$3, @1, @3)
        if (shouldCalc) {
            EXTRACT_BITWISE_ARGS($1, $3);
            $$ = new idouble(~(a1 ^ a2));
        }
    }
|   expression NAND expression {
        CHECK_VALID_BITWISE("nand", *$1, *$3, @1, @3)
        if (shouldCalc) {
            EXTRACT_BITWISE_ARGS($1, $3);
            $$ = new idouble(~(a1 & a2));
        }
    }
|   expression OR expression {
        CHECK_VALID_BITWISE("nor", *$1, *$3, @1, @3)
        if (shouldCalc) {
            EXTRACT_BITWISE_ARGS($1, $3);
            $$ = new idouble(a1 | a2);
        }
    }
|   expression XNOR expression {
        CHECK_VALID_BITWISE("xnor", *$1, *$3, @1, @3)
        if (shouldCalc) {
            EXTRACT_BITWISE_ARGS($1, $3);
            $$ = new idouble(~(a1 ^ a2));
        }
    }
|   NOT expression {
        CHECK_VALID_BITWISE("not", *$2, *$2, @2, @2)
        if (shouldCalc) {
            EXTRACT_BITWISE_ARGS($2, $2);
            $$ = new idouble(~a1);
        }
    }
|   expression FACTORIAL {CALL_MAINWINDOW_FUNCTION(@$, "fact", QList<idouble>() << *$1, $$)}
|   RADICAL expression {$$ = new idouble(sqrt(*$2));}
|   power RADICAL expression {
        CALL_MAINWINDOW_FUNCTION(@$, "root", QList<idouble>() << *$3 << *$1, $$)
    }
|   function
|   exprid {$$ = new idouble(*$1);}

exprid: IDENTIFIER {
        if (valueExists(*$1, p)) {
            $$ = new idouble(getValue(*$1, p));
        } else {
            YYE(@1, tr("Unknown variable: %1").arg(*$1).toLocal8Bit().constData());
            YYABORT;
        }
    }
|   IDENTIFIER power {
        if (valueExists(*$1, p)) {
            $$ = new idouble(pow(getValue(*$1, p), *$2));
        } else {
            YYE(@1, tr("Unknown variable: %1").arg(*$1).toLocal8Bit().constData());
            YYABORT;
        }
    }
|   IDENTIFIER EXPONENTIATE expression {
        if (valueExists(*$1, p)) {
            $$ = new idouble(pow(getValue(*$1, p), *$3));
        } else {
            YYE(@1, tr("Unknown variable: %1").arg(*$1).toLocal8Bit().constData());
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

function: IDENTIFIER LBRACKET arguments RBRACKET {CALL_MAINWINDOW_FUNCTION(@$, *$1, *$3, $$)}
|   IDENTIFIER EXPONENTIATE expression LBRACKET arguments RBRACKET {
        if ($3->real() == -1 && $3->imag() == 0 && BaseEvaluationEngine::current()->customFunctions().contains("a" + *$1)) {
            //Call special inverse function
            CALL_MAINWINDOW_FUNCTION(@$, "a" + *$1, *$5, $$);
        } else {
            idouble* result;
            CALL_MAINWINDOW_FUNCTION(@$, *$1, *$5, result)
            CALL_MAINWINDOW_FUNCTION(@$, "pow", QList<idouble>() << *result << *$3, $$);
            delete result;
        }
    }
|   IDENTIFIER power LBRACKET arguments RBRACKET {
        if ($2->real() == -1 && $2->imag() == 0 && BaseEvaluationEngine::current()->customFunctions().contains("a" + *$1)) {
            //Call special inverse function
            CALL_MAINWINDOW_FUNCTION(@$, "a" + *$1, *$4, $$);
        } else {
            idouble* result;
            CALL_MAINWINDOW_FUNCTION(@$, *$1, *$4, result)
            CALL_MAINWINDOW_FUNCTION(@$, "pow", QList<idouble>() << *result << *$2, $$);
            delete result;
        }
    }
%%

void yyerror(YYLTYPE* yylloc, yyscan_t scanner, EvaluationEngineParameters p, const char* s) {
    p.errorFunction(yylloc->location, yylloc->length, s);
}
