%{
#include <QString>
#include <qmath.h>
#include <complex>
typedef std::complex<double> idouble;

#include "calc.bison.hpp"
%}

%option noyywrap

%%
[0-9]+(\.[0-9]+)?i?|i {
    QString token = QString::fromLocal8Bit(yytext);
    if (token == "i") {
        yylval.number = new idouble(0, 1);
    } else if (token.endsWith("i")) {
        token.chop(1);
        yylval.number = new idouble(0, token.toDouble());
    } else {
        yylval.number = new idouble(token.toDouble());
    }
    return NUMBER;
}
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
        yylval.number = new idouble(M_E);
        return NUMBER;
    } else {
        yylval.string = new QString(yytext);
        return IDENTIFIER;
    }
}
"," {return ARGSEPARATOR;}
"π" {yylval.number = new idouble(M_PI); return NUMBER; }
%%
