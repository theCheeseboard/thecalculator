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
%}

%define parse.trace
%define parse.error verbose
%define parse.lac full

%union {
    QString* string;
    idouble* number;
    QList<idouble>* arguments;
}

%token<number> NUMBER
%token<number> LBRACKET RBRACKET
%token<number> ADD SUBTRACT MULTIPLY DIVIDE LSH RSH
%token<number> PERCENT ABSOLUTE ENDABSOLUTE FACTORIAL RADICAL
%token<number> EOL
%token<number> ARGSEPARATOR
%token<string> IDENTIFIER

%type<number> expression
%type<number> function
%type<arguments> arguments

%destructor { delete $$; } <string>
%destructor { delete $$; } <arguments>
//%destructor { delete $$; } <number>

//Set operator precedence
%left ADD SUBTRACT
%left MULTIPLY DIVIDE
%left LSH RSH
%left EXPONENTIATE SQUARE CUBE RADICAL
%left FACTORIAL PERCENT
%left LBRACKET RBRACKET

%%
line: expression EOL { MainWin->parserResult(*$1); }

expression: SUBTRACT expression {$$ = new idouble(-$2->real(), $2->imag());}
|   NUMBER {$$ = new idouble(*$1);}
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
|   expression EXPONENTIATE expression {$$ = new idouble(pow(*$1, *$3));}
|   expression SQUARE {$$ = new idouble(pow(*$1, 2));}
|   expression CUBE {$$ = new idouble(pow(*$1, 3));}
|   expression LSH expression {
        QString error;
        $$ = new idouble(MainWin->callFunction("lsh", QList<idouble>() << *$1 << *$3, error));
        if (error != "") {
            yyerror(error.toUtf8().constData());
            YYABORT;
        }
    }
|   expression RSH expression {
        QString error;
        $$ = new idouble(MainWin->callFunction("rsh", QList<idouble>() << *$1 << *$3, error));
        if (error != "") {
            yyerror(error.toUtf8().constData());
            YYABORT;
        }
    }
|   expression FACTORIAL {
        QString error;
        $$ = new idouble(MainWin->callFunction("fact", QList<idouble>() << *$1, error));
        if (error != "") {
            yyerror(error.toUtf8().constData());
            YYABORT;
        }
    }
|   RADICAL expression {$$ = new idouble(sqrt(*$2));}
|   CUBE RADICAL expression {$$ = new idouble(pow(*$3, 1.0/3.0));}
|   function

arguments: expression {$$ = new QList<idouble>(); $$->append(*$1);}
|   arguments ARGSEPARATOR expression {$$ = new QList<idouble>(*$1); $$->append(*$3);}
|   %empty {$$ = new QList<idouble>();}

function: IDENTIFIER LBRACKET arguments RBRACKET {
        QString error;
        $$ = new idouble(MainWin->callFunction(*$1, *$3, error));
        if (error != "") {
            yyerror(error.toUtf8().constData());
            YYABORT;
        }
    }
|   IDENTIFIER EXPONENTIATE expression LBRACKET arguments RBRACKET {
        QString error;
        idouble single = MainWin->callFunction(*$1, *$5, error);
        if (error != "") {
            yyerror(error.toUtf8().constData());
            YYABORT;
        }

        $$ = new idouble(pow(single, *$3));
    }
|   IDENTIFIER SQUARE LBRACKET arguments RBRACKET {
        QString error;
        idouble single = MainWin->callFunction(*$1, *$4, error);
        if (error != "") {
            yyerror(error.toUtf8().constData());
            YYABORT;
        }

        $$ = new idouble(pow(single, 2));
    }
|   IDENTIFIER CUBE LBRACKET arguments RBRACKET {
        QString error;
        idouble single = MainWin->callFunction(*$1, *$4, error);
        if (error != "") {
            yyerror(error.toUtf8().constData());
            YYABORT;
        }

        $$ = new idouble(pow(single, 3));
    }
%%

void yyerror(const char* s) {
    printf("ERROR: %s\n", s);
    MainWin->parserError(s);
}
