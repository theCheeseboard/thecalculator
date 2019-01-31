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

#include <QScroller>
#include <QActionGroup>
#include <QMenu>
#include <QStackedWidget>
#include "aboutwindow.h"
#include <QRandomGenerator>
#include <QProcess>
#include "historydelegate.h"
#include <QDesktopServices>
#include <QMessageBox>
#include <QTimer>
#include <QScrollBar>
#include <ttoast.h>
#include <QToolButton>
#include <tpopover.h>
#include <QShortcut>
#include "evaluationengine.h"

#include "customs/overloadbox.h"

extern MainWindow* MainWin;
extern float getDPIScaling();
extern QMap<QString, std::function<idouble(QList<idouble>,QString&)>> customFunctions;
extern QString idbToString(idouble db);

#include "calc.bison.hpp"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    MainWin = this;
    ui->setupUi(this);

    ui->stackedWidget->setCurrentAnimation(tStackedWidget::Lift);

    QActionGroup* group = new QActionGroup(this);
    group->addAction(ui->actionDegrees);
    group->addAction(ui->actionRadians);
    group->addAction(ui->actionGradians);

    CornerButton* menuButton = new CornerButton();
    menuButton->setIcon(QIcon::fromTheme("application-menu"));
    menuButton->setFixedHeight(ui->menuBar->sizeHint().height());
    menuButton->setFlat(true);
    connect(menuButton, &QPushButton::clicked, [=] {
        tPopover* p = new tPopover(ui->leftPane);
        p->setPopoverSide(tPopover::Leading);
        p->setPopoverWidth(300 * theLibsGlobal::getDPIScaling());
        p->show(this);
    });
    ui->menuBar->setCornerWidget(menuButton, Qt::TopLeftCorner);

    QShortcut* menuShortcut = new QShortcut(this);
    menuShortcut->setKey(Qt::SHIFT | Qt::Key_Tab);
    connect(menuShortcut, &QShortcut::activated, [=] {
        menuButton->click();
    });

    ui->centralWidget->layout()->removeWidget(ui->leftPane);

    QTimer::singleShot(0, [=] {
        this->setFixedWidth(ui->calcWidget->sizeHint().width());
    });
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::parserError(const char *error) {
    resultSuccess = false;
}

void MainWindow::on_actionExit_triggered()
{
    QApplication::exit();
}

void MainWindow::on_backButton_clicked()
{
    ui->stackedWidget->setCurrentIndex(0);
    ui->calcWidget->grabExpKeyboard(true);
}

void MainWindow::on_FunctionsButton_clicked()
{
    ui->calcWidget->grabExpKeyboard(false);
    ui->stackedWidget->setCurrentIndex(1);
}

void MainWindow::on_actionAbout_triggered()
{
    AboutWindow a;
    a.exec();
}

void MainWindow::changeEvent(QEvent *event) {
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

void MainWindow::on_actionDegrees_triggered(bool checked)
{
    if (checked) {
        EvaluationEngine::setTrigonometricUnit(EvaluationEngine::Degrees);
    }
}

void MainWindow::on_actionRadians_triggered(bool checked)
{
    if (checked) {
        EvaluationEngine::setTrigonometricUnit(EvaluationEngine::Radians);
    }
}

void MainWindow::on_actionTheCalculatorHelp_triggered()
{
    QDesktopServices::openUrl(QUrl("https://vicr123.com/thecalculator/help"));
}

void MainWindow::on_actionFileBug_triggered()
{
    QDesktopServices::openUrl(QUrl("https://github.com/vicr123/thecalculator/issues"));
}

void MainWindow::on_actionSources_triggered()
{
    QDesktopServices::openUrl(QUrl("https://github.com/vicr123/thecalculator"));
}

void MainWindow::on_calcWidget_manageFunctions()
{
    ui->stackedWidget->setCurrentIndex(1);
}

void MainWindow::on_calcWidget_sizeHintChanged()
{
    if (ui->stackedWidget->currentIndex() == 1) {
        this->setFixedWidth(ui->calcWidget->sizeHint().width());
    }
}

void MainWindow::on_stackedWidget_currentChanged(int arg1)
{
    if (specificMenu != nullptr) {
        ui->menuBar->removeAction(specificMenu->menuAction());
        specificMenu = nullptr;
    }


    int newWidth = -1;
    switch (arg1) {
        case 1: //Scientific
            newWidth = ui->calcWidget->sizeHint().width();
            break;
        case 3: //Graphing
            specificMenu = ui->graphWidget->getMenu();
            ui->menuBar->insertMenu(ui->menuTrigonometry->menuAction(), specificMenu);
    }

    if (newWidth == -1) {
        this->setFixedWidth(QWIDGETSIZE_MAX);
    } else {
        tVariantAnimation* anim = new tVariantAnimation();
        anim->setStartValue(this->width());
        anim->setEndValue(newWidth);
        anim->setDuration(500);
        anim->setEasingCurve(QEasingCurve::OutCubic);
        anim->start();
        connect(anim, &tVariantAnimation::valueChanged, [=](QVariant value) {
            this->setFixedWidth(value.toInt());
        });
        connect(anim, &tVariantAnimation::finished, [=] {
            this->setFixedWidth(anim->currentValue().toInt());
            anim->deleteLater();
        });
    }

    if (arg1 == 1) {
        ui->calcWidget->grabExpKeyboard(true);
    } else {
        ui->calcWidget->grabExpKeyboard(false);
    }

    if (arg1 != 0) {
        ui->leftMenu->setCurrentRow(arg1 - 1);
    } else {
        ui->leftMenu->setCurrentRow(-1);
        ui->leftMenu->clearSelection();
    }
}

void MainWindow::on_actionGradians_triggered(bool checked)
{
    if (checked) {
        EvaluationEngine::setTrigonometricUnit(EvaluationEngine::Gradians);
    }
}

void MainWindow::on_leftMenu_currentRowChanged(int currentRow)
{
    if (currentRow != -1) {
        ui->stackedWidget->setCurrentIndex(currentRow + 1);
        tPopover* p = tPopover::popoverForWidget(ui->leftPane);
        if (p != nullptr) {
            p->dismiss();
        }
    }
}

void MainWindow::on_manageCustomFunctionsbutton_clicked()
{
    ui->stackedWidget->setCurrentIndex(0);
    tPopover* p = tPopover::popoverForWidget(ui->leftPane);
    if (p != nullptr) {
        p->dismiss();
    }
}
