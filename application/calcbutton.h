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

#ifndef CALCBUTTON_H
#define CALCBUTTON_H

#include <QWidget>
#include <QPushButton>
#include <QMouseEvent>
#include "calculator/calculatorwidget.h"

class MainWindow;
class CalcButton : public QPushButton
{
    Q_OBJECT
    Q_PROPERTY(QString typedOutput READ typedOutput WRITE setTypedOutput DESIGNABLE true)
    Q_PROPERTY(QString shiftedOutput READ shiftedOutput WRITE setShiftedOutput DESIGNABLE true)
public:
    explicit CalcButton(QWidget *parent = nullptr);

    QString typedOutput();
    QString shiftedOutput();
signals:
    void output(QString output);

public slots:
    void setTypedOutput(QString typed);
    void setShiftedOutput(QString typed);
    void setText(const QString &text);

private:
    QString to;
    QString so;

    void mousePressEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);
};

#endif // CALCBUTTON_H
