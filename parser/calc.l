%{
#include <QString>
#include <qmath.h>
#include "evaluationengineheaders.h"
#include "calc.bison.hpp"
%}

%option noyywrap
%option reentrant bison-bridge
%option header-file="calc.yy.h"

%%
[0-9]+(\.[0-9]+)?i?|i {
    QString token = QString::fromLocal8Bit(yytext);
    if (token == "i") {
        yylval->number = new idouble(0, 1);
    } else if (token.endsWith("i")) {
        token.chop(1);
        yylval->number = new idouble(0, token.toDouble());
    } else {
        yylval->number = new idouble(token.toDouble());
    }
    return NUMBER;
}
("⁰"|"¹"|"²"|"³"|"⁴"|"⁵"|"⁶"|"⁷"|"⁸"|"⁹")+"ⁱ"?|"ⁱ" {
    QString token = QString::fromLocal8Bit(yytext);
    QString number;
                   for (QChar c : token) {
                   if (c == "⁰") number.append("0");
                   else if (c == "¹") number.append("1");
                   else if (c == "²") number.append("2");
                   else if (c == "³") number.append("3");
                   else if (c == "⁴") number.append("4");
                   else if (c == "⁵") number.append("5");
                   else if (c == "⁶") number.append("6");
                   else if (c == "⁷") number.append("7");
                   else if (c == "⁸") number.append("8");
                   else if (c == "⁹") number.append("9");
                   else if (c == "ⁱ") number.append("i");
                   }
                  if (number == "i") {
                      yylval->number = new idouble(0, 1);
                  } else if (number.endsWith("i")) {
                      token.chop(1);
                      yylval->number = new idouble(0, number.toDouble());
                  } else {
                      yylval->number = new idouble(number.toDouble());
                  }
    return SUPER;
}
"+" {return ADD;}
"⁺" {return SUPERADD;}
"-" {return SUBTRACT;}
"⁻" {return SUPERSUBTRACT;}
"*" {return MULTIPLY;}
"×" {return MULTIPLY;}
"⋅" {return MULTIPLY;}
"÷" {return DIVIDE;}
"/" {return DIVIDE;}
"^" {return EXPONENTIATE;}
"\n" {return EOL;}
"(" {return LBRACKET;}
")" {return RBRACKET;}
"√" {return RADICAL;}
"%" {return PERCENT;}
"!" {return FACTORIAL;}
"<<" {return LSH;}
">>" {return RSH;}
">" {return GREATER;}
"<" {return LESS;}
"≥" {return GREATEREQUAL;}
"≤" {return LESSEQUAL;}
"==" {return EQUALITY;}
"≠" {return NOTEQUALITY;}
[[:alpha:]_][[:alnum:]_]* {
    if (strcmp("e", yytext) == 0) {
        yylval->number = new idouble(M_E, 0);
        return NUMBER;
    } else {
        yylval->string = new QString(yytext);
        return IDENTIFIER;
    }
}
"," {return ARGSEPARATOR;}
"π" {yylval->number = new idouble(M_PI, 0); return NUMBER; }
"=" {return ASSIGNMENT;}

