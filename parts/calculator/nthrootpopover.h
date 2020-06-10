/****************************************
 *
 *   INSERT-PROJECT-NAME-HERE - INSERT-GENERIC-NAME-HERE
 *   Copyright (C) 2020 Victor Tran
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
#ifndef NTHROOTPOPOVER_H
#define NTHROOTPOPOVER_H

#include <QWidget>

namespace Ui {
    class NthRootPopover;
}

class NthRootPopover : public QWidget {
        Q_OBJECT

    public:
        explicit NthRootPopover(QWidget* parent = nullptr);
        ~NthRootPopover();

    private slots:
        void on_backButton_clicked();

        void on_okButton_clicked();

    signals:
        void rejected();
        void accepted(QString text);

    private:
        Ui::NthRootPopover* ui;
};

#endif // NTHROOTPOPOVER_H