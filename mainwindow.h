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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPalette>
#include <QPushButton>
#include <tvariantanimation.h>
#include <QScrollArea>
#include "calcbutton.h"
#include <QLineEdit>
#include <QLabel>
#include <functional>
#include "expression.h"
#include <complex>
#include <iomanip>
#include <QListWidgetItem>
#include "historydelegate.h"

typedef std::complex<long double> idouble;

class CalcButton;
namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

    public:
        explicit MainWindow(QWidget *parent = 0);
        ~MainWindow();

        QMap<QString, idouble> variables;

    public slots:
        void parserError(const char* error);

    private slots:
        void on_actionExit_triggered();

        void on_backButton_clicked();

        void on_FunctionsButton_clicked();

        void on_actionAbout_triggered();

        void on_actionDegrees_triggered(bool checked);

        void on_actionRadians_triggered(bool checked);

        void on_actionTheCalculatorHelp_triggered();

        void on_actionFileBug_triggered();

        void on_actionSources_triggered();

        void on_addCustomFunction_clicked();

        void on_backButton_2_clicked();

        void on_newOverloadButton_clicked();

        void on_saveCustomFunctionButton_clicked();

        void on_customFunctionsList_itemActivated(QListWidgetItem *item);

        void on_calcWidget_manageFunctions();

        void on_calcWidget_sizeHintChanged();

        void on_stackedWidget_currentChanged(int arg1);

        void on_actionGradians_triggered(bool checked);

    private:
        Ui::MainWindow *ui;

        bool resultSuccess = false;

        void changeEvent(QEvent* event);

        idouble currentAnswer;
        QSettings settings;
        QString editingFunction = "";
};

#endif // MAINWINDOW_H
