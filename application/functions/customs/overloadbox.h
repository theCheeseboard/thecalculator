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

#ifndef OVERLOADBOX_H
#define OVERLOADBOX_H

#include <QFrame>
#include <QLineEdit>
#include <QJsonObject>

namespace Ui {
    class OverloadBox;
}

class OverloadBox : public QFrame
{
        Q_OBJECT

    public:
        explicit OverloadBox(QWidget *parent = nullptr);
        ~OverloadBox();

        bool check();
        QJsonObject save();
        void load(QJsonObject obj);
        void flashError();

    private slots:
        void on_addArgButton_clicked();

        void on_removeArgButton_clicked();

        void on_removeOverloadButton_clicked();

        void on_addBranchButton_clicked();

    signals:
        void remove();

    private:
        Ui::OverloadBox *ui;

        QList<QLineEdit*> argumentNames;
        QList<QLineEdit*> argumentDesc;
};

#endif // OVERLOADBOX_H
