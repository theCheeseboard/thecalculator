/****************************************
 *
 *   INSERT-PROJECT-NAME-HERE - INSERT-GENERIC-NAME-HERE
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
#ifndef FUNCTIONWIDGET_H
#define FUNCTIONWIDGET_H

#include <tstackedwidget.h>
#include <QListWidgetItem>

namespace Ui {
    class FunctionWidget;
}

class FunctionWidget : public tStackedWidget
{
        Q_OBJECT

    public:
        explicit FunctionWidget(QWidget *parent = nullptr);
        ~FunctionWidget();

    private slots:
        void on_addCustomFunction_clicked();

        void on_backButton_2_clicked();

        void on_newOverloadButton_clicked();

        void on_saveCustomFunctionButton_clicked();

        void on_customFunctionsList_itemActivated(QListWidgetItem *item);

        void on_customFunctionsList_customContextMenuRequested(const QPoint &pos);

    private:
        Ui::FunctionWidget *ui;

        QString editingFunction = "";
};

#endif // FUNCTIONWIDGET_H
