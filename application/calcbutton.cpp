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

#include "calcbutton.h"
#include <QRegularExpression>

extern MainWindow* MainWin;

CalcButton::CalcButton(QWidget *parent) : QPushButton(parent)
{
    /*connect(this, &CalcButton::clicked, [=] {
        emit output(to);
    });*/
    CalculatorWidget::buttons.append(this);
}

QString CalcButton::typedOutput() {
    return to;
}

void CalcButton::setTypedOutput(QString typed) {
    to = typed;
}

void CalcButton::setText(const QString &text) {
    QPushButton::setText(text);

    if (QRegularExpression("[a-z]+").match(text).captured() == text) {
        to = text + "(";
    } else {
        to = text;
    }
}

QString CalcButton::shiftedOutput() {
    return so;
}

void CalcButton::setShiftedOutput(QString typed) {
    so = typed;
}

void CalcButton::mousePressEvent(QMouseEvent *event) {
    /*if (event->button() == Qt::LeftButton) {
        emit output(to);
    } else if (event->button() == Qt::RightButton && so != "") {
        emit output(so);
    }*/
    this->animateClick();
}

void CalcButton::mouseReleaseEvent(QMouseEvent *event) {
    this->animateClick();
    if (underMouse()) {
        if (event->button() == Qt::LeftButton) {
            emit output(to);
        } else if (event->button() == Qt::RightButton && so != "") {
            emit output(so);
        }
    }
}

