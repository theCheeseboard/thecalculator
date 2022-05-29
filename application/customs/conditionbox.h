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

#ifndef CONDITIONBOX_H
#define CONDITIONBOX_H

#include <QWidget>
#include <QJsonObject>

namespace Ui {
    class ConditionBox;
}

class ConditionBox : public QWidget
{
        Q_OBJECT

    public:
        explicit ConditionBox(bool isFirst, QWidget *parent = nullptr);
        ~ConditionBox();

        bool check();
        QJsonObject save();
        void load(QJsonObject obj);
        void flashError();

    signals:
        void remove();

    private slots:
        void on_removeButton_clicked();

    private:
        Ui::ConditionBox *ui;

        bool isFirst;
        void init();
};

#endif // CONDITIONBOX_H
