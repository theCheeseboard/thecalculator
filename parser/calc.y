%{
//Include headers
#include <QString>
#include <cstdio>
#include "mainwindow.h"
#include <complex>

//Define flex functions
extern int yylex(void);
extern void yyterminate();
void yyerror(const char* s);
extern MainWindow* MainWin;
%}

%define parse.trace

%union {
    QString* string;
    double number;
    QList<double>* arguments;
}

%token<number> NUMBER
%token<number> LBRACKET RBRACKET
%token<number> ADD SUBTRACT MULTIPLY DIVIDE
%token<number> PERCENT ABSOLUTE ENDABSOLUTE FACTORIAL
%token<number> EOL
%token<number> ARGSEPARATOR
%token<string> IDENTIFIER

%type<number> expression
%type<number> function
%type<arguments> arguments

%destructor { delete $$; } <string>
%destructor { delete $$; } <arguments>

//Set operator precedence
%left ADD SUBTRACT
%left MULTIPLY DIVIDE
%left EXPONENTIATE SQUARE CUBE ROOT
%left FACTORIAL PERCENT
%left LBRACKET RBRACKET

%%
line: expression EOL { MainWin->parserResult($1); }

expression: SUBTRACT expression {$$ = -$2;}
|   NUMBER {$$ = $1;}
|   expression ADD expression {$$ = $1 + $3;}
|   expression SUBTRACT expression {$$ = $1 - $3;}
|   expression MULTIPLY expression {$$ = $1 * $3;}
|   NUMBER expression {$$ = $1 * $2;}
|   expression DIVIDE expression {
        if ($3 == 0) {
            yyerror("div: division by 0 is undefined");
            YYABORT;
        } else {
            $$ = $1 / $3;
        }
    }
|   LBRACKET expression RBRACKET {$$ = $2;}
|   expression PERCENT {$$ = $$ / 100;}
|   expression EXPONENTIATE expression {$$ = pow($1, $3);}
|   expression SQUARE {$$ = pow($1, 2);}
|   expression CUBE {$$ = pow($1, 3);}
|   expression FACTORIAL {$$ = $1 * tgamma($1);}
|   function

arguments: expression {$$ = new QList<double>(); $$->append($1);}
|   arguments ARGSEPARATOR expression {$$ = new QList<double>(*$1); $$->append($3);}
|   %empty {$$ = new QList<double>();}

function: IDENTIFIER LBRACKET arguments RBRACKET {
        QString error;
        $$ = MainWin->callFunction(*$1, *$3, error);
        if (error != "") {
            yyerror(error.toUtf8().constData());
            YYABORT;
        }
    }
%%
/*int main(int argc, char** argv) {
    yyin = stdin;
    yyparse();
}*/

void yyerror(const char* s) {
    printf("ERROR: %s\n", s);
    MainWin->parserError(s);
}
