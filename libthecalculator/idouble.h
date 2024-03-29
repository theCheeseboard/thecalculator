/****************************************
 *
 *   INSERT-PROJECT-NAME-HERE - INSERT-GENERIC-NAME-HERE
 *   Copyright (C) 2022 Victor Tran
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
#ifndef IDOUBLE_H
#define IDOUBLE_H

#include "libthecalculator_global.h"
#include <complex>

typedef std::complex<long double> idouble;

LIBTHECALCULATOR_EXPORT QString idbToString(idouble db);

namespace std {
    template<> struct hash<idouble> {
            size_t operator()(const idouble& key, size_t seed = 0) const;
    };
} // namespace std

#endif // IDOUBLE_H
