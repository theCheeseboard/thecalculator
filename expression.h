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

#ifndef EXPRESSION_H
#define EXPRESSION_H

#include <QObject>
#include <QtMath>
#include <QDebug>

typedef long long integer;
typedef long double real;

class Number {
    integer n, d = 1;

    void simplify();
    integer gcd(integer a, integer b);

public:
    Number();
    Number(real numerator, real denominator);
    Number(real num);
    Number(integer num);

    void set(real numerator, real denominator);
    void setNumerator(real numerator);
    void setDenominator(real denominator);
    real getNumber();

    integer numerator();
    integer denominator();

    Number operator=(const int other);
    Number operator=(integer other);
    Number operator=(double other);

    operator real();
};

Number operator+(Number lhs, const Number rhs);
Number operator-(Number lhs, const Number rhs);
Number operator*(Number lhs, const Number rhs);
Number operator/(Number lhs, const Number rhs);

struct Token {
    enum type {
        Num,
        Function,
        None
    };

    Number number;
    QString op;
    QString function;
    Number functionExponent;
    type t;
};

struct Evaluation {
    enum errorType {
        None,
        DivisionByZero,
        Syntax
    };

    QString text;
    QString error;

    operator QString();
};

struct CalculatorCharacter {
    CalculatorCharacter(QCharRef c);

    enum Type {
        Number,
        Super,
        Operation,
        StartBracket,
        EndBracket,
        Letter,
        None
    };

    QString number;
    Type type;
};

class Expression : public QObject
{
    Q_OBJECT
public:
    explicit Expression(QString expr, QObject *parent = nullptr);


signals:

public slots:
    Evaluation evaluate();

private:
    QString originalExpr;
};

#endif // EXPRESSION_H
