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

#ifndef EXPRESSIONLINEEDIT_H
#define EXPRESSIONLINEEDIT_H

#include <QLineEdit>

class ExpressionLineEdit : public QLineEdit
{
        Q_OBJECT
    public:
        explicit ExpressionLineEdit(QWidget *parent = nullptr);

        QString getFixedExpression();
        QString getTypedExpression();

        void insert(QString text);
        void backspace();
        void del();
        void deleteRange(int start, int length);
        void clear();

    signals:
        void expressionUpdated(QString expression);

    public slots:
        void setExpression(QString newExpr);

    private:
        void keyPressEvent(QKeyEvent* event);

        void checkText(QString text, int originalPos);

        QString typedExpr, fixedExpr;
};

#endif // EXPRESSIONLINEEDIT_H
