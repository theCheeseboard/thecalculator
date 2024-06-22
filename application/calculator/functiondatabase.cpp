#include "functiondatabase.h"

#include <QMap>

struct FunctionDatabasePrivate {
        QMap<QString, FunctionDatabase::Function> functions;
};

FunctionDatabase::FunctionDatabase(QObject* parent) :
    QObject{parent}, d{new FunctionDatabasePrivate()} {
    d->functions.insert("sqrt", {
                                    "sqrt", {
                                             {tr("Calculates the square root of a number"),
                                                    {{"x", tr("Number to calculate the square root of")}}},
                                             }
    });
    d->functions.insert("cbrt", {
                                    "cbrt", {
                                             {tr("Calculates the cube root of a number"),
                                                    {{"x", tr("Number to calculate the cube root of")}}},
                                             }
    });
    d->functions.insert("root", {
                                    "root", {
                                             {tr("Calculates the nth root of a number"),
                                                    {{"x", tr("Number to calculate the nth root of")},
                                                        {tr("degree", "Polynomial degree"), tr("Degree of root to calculate")}}},
                                             }
    });
    d->functions.insert("sin", {
                                   "sin", {
                                           {tr("Calculates the sine of an angle"),
                                                  {{tr("angle"), tr("Angle to calculate the sine of")}}},
                                           }
    });
    d->functions.insert("cos", {
                                   "cos", {
                                           {tr("Calculates the cosine of an angle"),
                                                  {{tr("angle"), tr("Angle to calculate the cosine of")}}},
                                           }
    });
    d->functions.insert("tan", {
                                   "tan", {
                                           {tr("Calculates the tangent of an angle"),
                                                  {{tr("angle"), tr("Angle to calculate the tangent of")}}},
                                           }
    });
    d->functions.insert("asin", {
                                    "asin", {
                                             {tr("Calculates the inverse sine of an angle"),
                                                    {{tr("angle"), tr("Angle to calculate the inverse sine of")}}},
                                             }
    });
    d->functions.insert("acos", {
                                    "acos", {
                                             {tr("Calculates the inverse cosine of an angle"),
                                                    {{tr("angle"), tr("Angle to calculate the inverse cosine of")}}},
                                             }
    });
    d->functions.insert("atan", {
                                    "atan", {
                                             {tr("Calculates the inverse tangent of an angle"),
                                                    {{tr("angle"), tr("Angle to calculate the inverse tangent of")}}},
                                             }
    });
    d->functions.insert("sinh", {
                                    "sinh", {
                                             {tr("Calculates the hyperbolic sine of an angle"),
                                                    {{tr("angle"), tr("Angle to calculate the hyperbolic sine of")}}},
                                             }
    });
    d->functions.insert("cosh", {
                                    "cosh", {
                                             {tr("Calculates the hyperbolic cosine of an angle"),
                                                    {{tr("angle"), tr("Angle to calculate the hyperbolic cosine of")}}},
                                             }
    });
    d->functions.insert("tanh", {
                                    "tanh", {
                                             {tr("Calculates the hyperbolic tangent of an angle"),
                                                    {{tr("angle"), tr("Angle to calculate the hyperbolic tangent of")}}},
                                             }
    });
    d->functions.insert("asinh", {
                                     "asinh", {
                                               {tr("Calculates the inverse hyperbolic sine of an angle"),
                                                      {{tr("angle"), tr("Angle to calculate the inverse hyperbolic sine of")}}},
                                               }
    });
    d->functions.insert("acosh", {
                                     "acosh", {
                                               {tr("Calculates the inverse hyperbolic cosine of an angle"),
                                                      {{tr("angle"), tr("Angle to calculate the inverse hyperbolic cosine of")}}},
                                               }
    });
    d->functions.insert("atanh", {
                                     "atanh", {
                                               {tr("Calculates the inverse hyperbolic tangent of an angle"),
                                                      {{tr("angle"), tr("Angle to calculate the inverse hyperbolic tangent of")}}},
                                               }
    });
    d->functions.insert("sec", {
                                   "sec", {
                                           {tr("Calculates the secant of an angle"),
                                                  {{tr("angle"), tr("Angle to calculate the secant of")}}},
                                           }
    });
    d->functions.insert("csc", {
                                   "csc", {
                                           {tr("Calculates the cosecant of an angle"),
                                                  {{tr("angle"), tr("Angle to calculate the cosecant of")}}},
                                           }
    });
    d->functions.insert("cot", {
                                   "cot", {
                                           {tr("Calculates the cotangent of an angle"),
                                                  {{tr("angle"), tr("Angle to calculate the cotangent of")}}},
                                           }
    });
    d->functions.insert("asec", {
                                    "asec", {
                                             {tr("Calculates the inverse secant of an angle"),
                                                    {{tr("angle"), tr("Angle to calculate the inverse secant of")}}},
                                             }
    });
    d->functions.insert("acsc", {
                                    "acsc", {
                                             {tr("Calculates the inverse cosecant of an angle"),
                                                    {{tr("angle"), tr("Angle to calculate the inverse cosecant of")}}},
                                             }
    });
    d->functions.insert("acot", {
                                    "acot", {
                                             {tr("Calculates the inverse cotangent of an angle"),
                                                    {{tr("angle"), tr("Angle to calculate the inverse cotangent of")}}},
                                             }
    });
    d->functions.insert("sech", {
                                    "sech", {
                                             {tr("Calculates the hyperbolic secant of an angle"),
                                                    {{tr("angle"), tr("Angle to calculate the hyperbolic secant of")}}},
                                             }
    });
    d->functions.insert("csch", {
                                    "csch", {
                                             {tr("Calculates the hyperbolic cosecant of an angle"),
                                                    {{tr("angle"), tr("Angle to calculate the hyperbolic cosecant of")}}},
                                             }
    });
    d->functions.insert("coth", {
                                    "coth", {
                                             {tr("Calculates the hyperbolic cotangent of an angle"),
                                                    {{tr("angle"), tr("Angle to calculate the hyperbolic cotangent of")}}},
                                             }
    });
    d->functions.insert("asech", {
                                     "asech", {
                                               {tr("Calculates the inverse hyperbolic secant of an angle"),
                                                      {{tr("angle"), tr("Angle to calculate the inverse hyperbolic secant of")}}},
                                               }
    });
    d->functions.insert("acsch", {
                                     "acsch", {
                                               {tr("Calculates the inverse hyperbolic cosecant of an angle"),
                                                      {{tr("angle"), tr("Angle to calculate the inverse hyperbolic cosecant of")}}},
                                               }
    });
    d->functions.insert("acoth", {
                                     "acoth", {
                                               {tr("Calculates the inverse hyperbolic cotangent of an angle"),
                                                      {{tr("angle"), tr("Angle to calculate the inverse hyperbolic cotangent of")}}},
                                               }
    });
    d->functions.insert("log", {
                                   "log", {{tr("Calculates the logarithm (base 10) of a number"),
                                               {{"x", tr("Number to take the logarithm of")}}},
                                           {tr("Calculates the logarithm of a number, specifying a base"),
                                                  {{"x", tr("Number to take the logarithm of")},
                                                      {tr("base"), tr("Base of the logarithm")}}}}
    });
    d->functions.insert("ln", {
                                  "ln", {
                                         {tr("Calculates the natural logarithm (base e) of a number"),
                                                {{"x", tr("Number to take the natural logarithm of")}}},
                                         }
    });
    d->functions.insert("abs", {
                                   "abs", {
                                           {tr("Calculates the absolute value of a number"),
                                                  {{"x", tr("Number to take the absolute value of")}}},
                                           }
    });
    d->functions.insert("exp", {
                                   "exp", {
                                           {tr("Calculates e raised to the power of a number"),
                                                  {{"x", tr("Number to raise e to the power of")}}},
                                           }
    });
    d->functions.insert("re", {
                                  "re", {
                                         {tr("Returns the real part of a complex number"),
                                                {{"x", tr("Number to return the real part of")}}},
                                         }
    });
    d->functions.insert("im", {
                                  "im", {
                                         {tr("Returns the imaginary part of a complex number"),
                                                {{"x", tr("Number to return the imaginary part of")}}},
                                         }
    });
    d->functions.insert("arg", {
                                   "arg", {
                                           {tr("Returns the phase angle of a complex number"),
                                                  {{"x", tr("Number to return the phase angle of")}}},
                                           }
    });
    d->functions.insert("conj", {
                                    "conj", {
                                             {tr("Returns the complex conjugate of a complex number"),
                                                    {{"x", tr("Number to return the complex conjugate of")}}},
                                             }
    });
}

FunctionDatabase::~FunctionDatabase() {
    delete d;
}

bool FunctionDatabase::haveFunction(QString name) {
    return d->functions.contains(name);
}

FunctionDatabase::Function FunctionDatabase::function(QString name) {
    return d->functions.value(name);
}
