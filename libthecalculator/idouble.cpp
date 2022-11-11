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
#include "idouble.h"

#include <functional>

std::size_t std::hash<idouble>::operator()(const idouble& key, size_t seed) const {
    /*QByteArray hash = QCryptographicHash::hash(idbToString(key).toUtf8(), QCryptographicHash::Md5);

                uint hashValue = 0; //this can overflow, it's fine
                for (char c : hash) {
                    hashValue += c;
                }
                return hashValue;*/

    int n1 = 99999997;
    int realHash = fmod(std::hash<long double>()(key.real()), n1);
    int imHash = std::hash<long double>()(key.imag());
    return realHash ^ imHash;
}
