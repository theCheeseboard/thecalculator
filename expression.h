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
