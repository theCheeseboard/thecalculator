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

#ifndef EVALUATIONENGINEHEADERS_H
#define EVALUATIONENGINEHEADERS_H

#include <QMap>
#include <complex>
#include <functional>

typedef std::complex<long double> idouble;
typedef void* yyscan_t;

typedef struct yyloc_t {
    int location;
    int length;
} YYLTYPE;
#define YYLTYPE yyloc_t

struct EvaluationEngineParameters {
    yyscan_t scanner;
    std::function<void(idouble)> resultFunction;
    std::function <void(int, int, const char*)> errorFunction;
    std::function<void(QString, idouble)> assignFunction;
    std::function<void(bool)> equalityFunction;
    QMap<QString, idouble> variables;
};

#endif // EVALUATIONENGINEHEADERS_H
