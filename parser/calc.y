%{
//Include headers
#include <QString>
#include <cstdio>
#include <complex>
#include "mainwindow.h"

//Define flex functions
extern int yylex(void);
extern void yyterminate();
void yyerror(const char* s);
extern MainWindow* MainWin;

//Define helper functions
#define CALL_MAINWINDOW_FUNCTION(arg1, arg2, result) { \
        QString error; \
        result = new idouble(MainWin->callFunction(arg1, arg2, error)); \
        if (error != "") { \
            yyerror(error.toUtf8().constData()); \
            YYABORT; \
        } \
    }
%}

%define parse.error verbose
%define parse.lac full

%union {
    QString* string;
    idouble* number;
    QList<idouble>* arguments;
}

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
%left ASSIGNMENT

%%
line: expression EOL { MainWin->parserResult(*$1); }
|   IDENTIFIER ASSIGNMENT expression EOL { MainWin->assignValue(*$1, *$3); }

expression: SUBTRACT expression {$$ = new idouble(-$2->real(), $2->imag());}
|   NUMBER {$$ = new idouble(*$1);}
|   expression power {CALL_MAINWINDOW_FUNCTION("pow", QList<idouble>() << *$1 << *$2, $$)}
|   expression ADD expression {$$ = new idouble(*$1 + *$3);}
|   expression SUBTRACT expression {$$ = new idouble(*$1 - *$3);}
|   expression MULTIPLY expression {$$ = new idouble(*$1 * *$3);}
//|   NUMBER expression {$$ = new idouble(*$1 * *$2);}
|   expression DIVIDE expression {
        if ($3->real() == 0 && $3->imag() == 0) {
            yyerror("div: division by 0 undefined");
            YYABORT;
        } else {
            $$ = new idouble(*$1 / *$3);
        }
    }
|   LBRACKET expression RBRACKET {$$ = new idouble(*$2);}
|   expression PERCENT {$$ = new idouble(*$1 / idouble(100));}
|   expression EXPONENTIATE expression {CALL_MAINWINDOW_FUNCTION("pow", QList<idouble>() << *$1 << *$3, $$)}
|   IDENTIFIER EXPONENTIATE expression {
        if (MainWin->valueExists(*$1)) {
            $$ = new idouble(pow(MainWin->getValue(*$1), *$3));
        } else {
            yyerror("var: unknown variable");
            YYABORT;
        }
    }
|   IDENTIFIER power {
        if (MainWin->valueExists(*$1)) {
            $$ = new idouble(pow(MainWin->getValue(*$1), *$2));
        } else {
            yyerror("var: unknown variable");
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
        if (MainWin->valueExists(*$1)) {
            $$ = new idouble(MainWin->getValue(*$1));
        } else {
            yyerror("var: unknown variable");
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

void yyerror(const char* s) {
    printf("ERROR: %s\n", s);
    MainWin->parserError(s);
}
