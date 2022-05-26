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

#include "expression.h"

Number::Number() {

}

Number::Number(real numerator, real denominator) {
    set(numerator, denominator);
}

Number::Number(real num) {
    setNumerator(num);
}

Number::Number(integer num) {
    setNumerator(num);
}

void Number::set(real numerator, real denominator) {
    real num = numerator;
    real den = denominator;

    while (ceil(num) != num) {
        num *= 10;
        den *= 10;
    }
    while (ceil(den) != den) {
        num *= 10;
        den *= 10;
    }

    n = num;
    d = den;
    simplify();
}

void Number::setNumerator(real numerator) {
    real num = numerator;
    integer den = d;
    while (ceil(num) != num) {
        num *= 10;
        den *= 10;
    }
    n = num;
    d = den;
    simplify();
}

void Number::setDenominator(real denominator) {
    integer num = n;
    real den = denominator;
    while (ceil(den) != den) {
        num *= 10;
        den *= 10;
    }
    n = num;
    d = den;
    simplify();
}

real Number::getNumber() {
    return (real) n / (real) d;
}

integer Number::numerator() {
    return n;
}

integer Number::denominator() {
    return d;
}

void Number::simplify() {
    integer divisor = gcd(n, d);
    n /= divisor;
    d /= divisor;
}

integer Number::gcd(integer a, integer b) {
    if (b == 0) {
        return a;
    }

    return gcd(b, a % b);
}

Number Number::operator=(const int other) {
    Number n;
    n.setNumerator(other);
    return n;
}

Number Number::operator=(const integer other) {
    Number n;
    n.setNumerator(other);
    return n;
}

Number Number::operator=(const double other) {
    Number n;
    n.setNumerator(other);
    return n;
}

Number operator+(Number lhs, Number rhs) {
    Number n;
    integer den = lhs.denominator() * rhs.denominator();
    n.set((lhs.numerator() * rhs.denominator()) + (rhs.numerator() * lhs.denominator()), den);
    return n;
}

Number operator-(Number lhs, Number rhs) {
    Number n;
    integer den = lhs.denominator() * rhs.denominator();
    n.set((lhs.numerator() * rhs.denominator()) - (rhs.numerator() * lhs.denominator()), den);
    return n;
}

Number operator*(Number lhs, Number rhs) {
    Number n;
    n.set((real) lhs.numerator() * (real) rhs.numerator(), (real) lhs.denominator() * (real) rhs.denominator());
    return n;
}

Number operator/(Number lhs, Number rhs) {
    Number n;
    n.set((real) lhs.numerator() / (real) rhs.numerator(), (real) lhs.denominator() / (real) rhs.denominator());
    return n;
}

Number::operator real() {
    return getNumber();
}

Evaluation::operator QString() {
    return text;
}

CalculatorCharacter::CalculatorCharacter(QChar& c) {
    if (c.isDigit()) {
        type = Number;
        number = QString::number(c.digitValue());
    } else if (c == u'⁰' || c == u'¹' || c == u'²' || c == u'³' || c == u'⁴' || c == u'⁵' || c == u'⁶' || c == u'⁷' || c == u'⁸' || c == u'⁹') {
        type = Super;
        if (c == u'⁰') number = "0";
        else if (c == u'¹') number = "1";
        else if (c == u'²') number = "2";
        else if (c == u'³') number = "3";
        else if (c == u'⁴') number = "4";
        else if (c == u'⁵') number = "5";
        else if (c == u'⁶') number = "6";
        else if (c == u'⁷') number = "7";
        else if (c == u'⁸') number = "8";
        else if (c == u'⁹') number = "9";
    } else if (c == u'+' || c == u'-' || c == u'*' || c == u'/' || c == u'×' || c == u'÷' || c == u'^') {
        type = Operation;
        number = QChar(c);
        if (c == u'×') {
            number = "*";
        } else if (c == u'÷') {
            number = "/";
        }
    } else if (c == u'(') {
        type = StartBracket;
        number = "(";
    } else if (c == u')') {
        type = EndBracket;
        number = ")";
    } else {
        type = Letter;
        number = QChar(c);
    }
}

Expression::Expression(QString expr, QObject *parent) : QObject(parent)
{
    originalExpr = expr;
}

Evaluation Expression::evaluate() {
    Evaluation eval;
    QString expression = originalExpr.remove(" ");

    //Step through, looking for brackets
    int currentBrackets = 0;
    int bracketsStart;
    for (int i = 0; i < expression.length(); i++) {
        if (expression[i] == u'(') {
            currentBrackets++;
            if (currentBrackets == 1) {
                bracketsStart = i;
            }
        } else if (expression[i] == u')') {
            currentBrackets--;
            if (currentBrackets == 0) {
                int count = i - bracketsStart;
                Expression e(expression.mid(bracketsStart + 1, count - 1));
                expression.replace(bracketsStart + 1, count - 1, e.evaluate());
                i = bracketsStart;
            }
        }
    }

    if (currentBrackets > 0) {
        while (currentBrackets > 0) {
            currentBrackets--;
            expression.append(")");
        }

        int count = expression.count() - bracketsStart;
        Expression e(expression.mid(bracketsStart + 1, count - 1));
        expression.replace(bracketsStart + 1, count - 1, e.evaluate());
    }

    //Replace constants
    expression.replace("π", "(" + QString::number(M_PI, 'g', 20) + ")");
    expression.replace("e", "(" + QString::number(M_E, 'g', 20) + ")");

    //Evaluate functions
    QList<Token> tokens;

    Token::type previousToken = Token::None;
    CalculatorCharacter::Type previousChar = CalculatorCharacter::None;

    QString currentTokenPart1, currentTokenPart2, op;
    auto finalizeToken = [=] {
        Token t;
        switch (previousToken) {
            case Token::Num:
                t.t = Token::Num;
                t.number = currentTokenPart1.toDouble();
                t.op = op;
                return t;
        }
    };

    for (int i = 0; i < expression.length(); i++) {
        CalculatorCharacter c = expression[i];
        switch (c.type) {
            case CalculatorCharacter::Number: {
                if (previousToken != Token::Num) {
                    tokens.append(finalizeToken());
                    previousToken = Token::Num;
                    currentTokenPart1 = "";
                    currentTokenPart2 = "";
                    op = "";
                }
                currentTokenPart1 += c.number;
                break;
            }
            case CalculatorCharacter::Operation: {
                if (op != "") {
                    //More than one operation in a row
                    //Syntax error
                    eval.error = "Syntax Error";
                    eval.text = "Syntax Error";
                    return eval;
                }
                op = c.number;
                break;
            }
            case CalculatorCharacter::Super: {
                if (previousToken == Token::Function) {
                    //Raise the function to this power
                //} else if (previousToken == Token::Super) {

                } else {
                    //Super in unexpected place
                    //Syntax error
                    eval.error = "Syntax Error";
                    eval.text = "Syntax Error";
                    return eval;
                }
            }
        }
        previousChar = c.type;
    }


    //eval.text = expression;
    return eval;
}
