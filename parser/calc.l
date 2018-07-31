%{
#include <QString>
#include <qmath.h>
#include <complex>
#include "calc.bison.hpp"
%}

%option noyywrap

%%
[0-9]+(\.[0-9]+)?i? {yylval.number = atof(yytext); return NUMBER; }
"+" {return ADD;}
"-" {return SUBTRACT;}
"*" {return MULTIPLY;}
"×" {return MULTIPLY;}
"÷" {return DIVIDE;}
"/" {return DIVIDE;}
"^" {return EXPONENTIATE;}
"\n" {return EOL;}
"(" {return LBRACKET;}
")" {return RBRACKET;}
"²" {return SQUARE;}
"³" {return CUBE;}
"√" {return ROOT;}
"%" {return PERCENT;}
"!" {return FACTORIAL;}
[[:alpha:]_][[:alnum:]_]* {
    if (strcmp("e", yytext) == 0) {
        yylval.number = M_E;
        return NUMBER;
    } else {
        yylval.string = new QString(yytext);
        return IDENTIFIER;
    }
}
"," {return ARGSEPARATOR;}
"π" {yylval.number = M_PI; return NUMBER; }
%%
