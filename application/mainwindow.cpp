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

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "evaluationengine.h"
#include <QActionGroup>
#include <QDesktopServices>
#include <QMenu>
#include <QMessageBox>
#include <QProcess>
#include <QRandomGenerator>
#include <QScrollBar>
#include <QScroller>
#include <QShortcut>
#include <QStackedWidget>
#include <QTimer>
#include <QToolButton>
#include <tapplication.h>
#include <tcsdtools.h>
#include <thelpmenu.h>
#include <tpopover.h>
#include <ttoast.h>

extern MainWindow* MainWin;

struct MainWindowPrivate {
        tCsdTools csd;
};

MainWindow::MainWindow(QWidget* parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow) {
    MainWin = this;
    ui->setupUi(this);
    d = new MainWindowPrivate();

    d->csd.installMoveAction(ui->topWidget);
    d->csd.installResizeAction(this);

    if (tCsdGlobal::windowControlsEdge() == tCsdGlobal::Left) {
        ui->leftCsdLayout->addWidget(d->csd.csdBoxForWidget(this));
    } else {
        ui->rightCsdLayout->addWidget(d->csd.csdBoxForWidget(this));
    }

    QActionGroup* group = new QActionGroup(this);
    group->addAction(ui->actionDegrees);
    group->addAction(ui->actionRadians);
    group->addAction(ui->actionGradians);

    QMenu* menu = new QMenu();

    QMenu* trigMenu = new QMenu();
    trigMenu->setTitle(tr("Trigonometry"));
    trigMenu->addAction(ui->actionDegrees);
    trigMenu->addAction(ui->actionRadians);
    trigMenu->addAction(ui->actionGradians);
    menu->addMenu(trigMenu);

    menu->addMenu(new tHelpMenu(this));

    menu->addSeparator();
    menu->addAction(ui->actionExit);

    this->setWindowIcon(tApplication::applicationIcon());
    ui->menuButton->setIcon(tApplication::applicationIcon());
    ui->menuButton->setIconSize(QSize(24, 24));
    ui->menuButton->setMenu(menu);
    ui->stackedWidget->setCurrentAnimation(tStackedWidget::SlideHorizontal);

    ui->calcWidget->setFocus();
}

MainWindow::~MainWindow() {
    delete d;
    delete ui;
}

void MainWindow::parserError(const char* error) {
    resultSuccess = false;
}

void MainWindow::on_actionExit_triggered() {
    QApplication::exit();
}

void MainWindow::on_backButton_clicked() {
    ui->stackedWidget->setCurrentIndex(0);
    ui->calcWidget->grabExpKeyboard(true);
}

void MainWindow::on_FunctionsButton_clicked() {
    ui->calcWidget->grabExpKeyboard(false);
    ui->stackedWidget->setCurrentIndex(1);
}

void MainWindow::changeEvent(QEvent* event) {
    QMainWindow::changeEvent(event);
    if (event->type() == QEvent::ActivationChange) {
        if (this->isActiveWindow() && ui->stackedWidget->currentIndex() == 1) {
            ui->calcWidget->grabExpKeyboard(true);
        } else {
            ui->calcWidget->grabExpKeyboard(false);
        }
    } else if (event->type() == QEvent::LanguageChange) {
        ui->retranslateUi(this);
    }
}

void MainWindow::on_actionDegrees_triggered(bool checked) {
    if (checked) {
        EvaluationEngine::setTrigonometricUnit(EvaluationEngine::Degrees);
    }
}

void MainWindow::on_actionRadians_triggered(bool checked) {
    if (checked) {
        EvaluationEngine::setTrigonometricUnit(EvaluationEngine::Radians);
    }
}

void MainWindow::on_calcWidget_manageFunctions() {
    ui->stackedWidget->setCurrentIndex(1);
}

void MainWindow::on_stackedWidget_currentChanged(int arg1) {
    if (ui->stackedWidget->widget(arg1) == ui->calcPage) {
        ui->calcWidget->grabExpKeyboard(true);
    } else {
        ui->calcWidget->grabExpKeyboard(false);
    }
}

void MainWindow::on_actionGradians_triggered(bool checked) {
    if (checked) {
        EvaluationEngine::setTrigonometricUnit(EvaluationEngine::Gradians);
    }
}

void MainWindow::on_scientificButton_toggled(bool checked) {
    if (checked) {
        ui->stackedWidget->setCurrentWidget(ui->calcPage);
    }
}

void MainWindow::on_statsButton_toggled(bool checked) {
    if (checked) {
        ui->stackedWidget->setCurrentWidget(ui->statsPage);
    }
}

void MainWindow::on_graphButton_toggled(bool checked) {
    if (checked) {
        ui->stackedWidget->setCurrentWidget(ui->graphPage);
    }
}

void MainWindow::on_functionsButton_toggled(bool checked) {
    if (checked) {
        ui->stackedWidget->setCurrentWidget(ui->funcPage);
    }
}
