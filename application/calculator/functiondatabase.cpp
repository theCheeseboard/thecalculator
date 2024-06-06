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
    d->functions.insert("sin", {
                                   "sin", {
                                           {tr("Calculates the sine of an angle"),
                                                  {{"angle", tr("Angle to calculate the sine of")}}},
                                           }
    });
    d->functions.insert("cos", {
                                   "cos", {
                                           {tr("Calculates the cosine of an angle"),
                                                  {{"angle", tr("Angle to calculate the cosine of")}}},
                                           }
    });
    d->functions.insert("tan", {
                                   "tan", {
                                           {tr("Calculates the tangent of an angle"),
                                                  {{"angle", tr("Angle to calculate the tangent of")}}},
                                           }
    });
    d->functions.insert("log", {
                                   "log", {{tr("Calculates the logarithm (base 10) of a number"),
                                               {{"x", tr("Number to take the logarithm of")}}},
                                           {tr("Calculates the logarithm of a number, specifying a base"),
                                                  {{"x", tr("Number to take the logarithm of")},
                                                      {"base", tr("Base of the logarithm")}}}}
    });
    d->functions.insert("ln", {
                                  "ln", {
                                         {tr("Calculates the natural logarithm (base e) of a number"),
                                                {{"x", tr("Number to take the natural logarithm of")}}},
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
