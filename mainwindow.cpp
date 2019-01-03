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
#include <QJsonArray>
#include <QJsonDocument>
#include <QMessageBox>
#include <QTimer>
#include <QScrollBar>
#include <ttoast.h>
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

    ui->stackedWidget->setCurrentAnimation(tStackedWidget::SlideHorizontal);

    QActionGroup* group = new QActionGroup(this);
    group->addAction(ui->actionDegrees);
    group->addAction(ui->actionRadians);
    group->addAction(ui->actionGradians);

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
    QSettings settings;
    settings.beginGroup("customFunctions");

    //Load up all the custom functions
    ui->customFunctionsList->clear();
    for (QString key : settings.allKeys()) {
        ui->customFunctionsList->addItem(key);
    }
    settings.endGroup();

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
        if (this->isActiveWindow() && ui->stackedWidget->currentIndex() == 0) {
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

void MainWindow::on_addCustomFunction_clicked()
{
    editingFunction = "";
    ui->functionName->setText("");
    QLayoutItem* i = ui->customFunctionDefinitionWidget->layout()->takeAt(0);
    while (i != nullptr) {
        i->widget()->deleteLater();
        i = ui->customFunctionDefinitionWidget->layout()->takeAt(0);
    }

    ui->stackedWidget->setCurrentIndex(2);
}

void MainWindow::on_backButton_2_clicked()
{
    QMessageBox::StandardButton b = QMessageBox::warning(this, tr("Save this function?"), tr("Do you want to save this function?"), QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel, QMessageBox::Save);
    if (b == QMessageBox::Save) {
        //Save the function
        ui->saveCustomFunctionButton->click();
    } else if (b == QMessageBox::Discard) {
        //Don't save the function
        ui->stackedWidget->setCurrentIndex(1);
    }
}

void MainWindow::on_newOverloadButton_clicked()
{
    OverloadBox* overload = new OverloadBox();
    connect(overload, &OverloadBox::remove, [=] {
        ui->customFunctionDefinitionWidget->layout()->removeWidget(overload);
        overload->deleteLater();
    });
    ui->customFunctionDefinitionWidget->layout()->addWidget(overload);
}

void MainWindow::on_saveCustomFunctionButton_clicked()
{
    QJsonObject obj;

    if (ui->functionName->text() == "") {
        //Name is required
        tToast* toast = new tToast();
        toast->setTitle(tr("Function Name Required"));
        toast->setText(tr("A function name needs to be set"));
        toast->show(this);
        connect(toast, &tToast::dismissed, toast, &tToast::deleteLater);
        return;
    }
    obj.insert("name", ui->functionName->text());

    QJsonArray overloads;
    QList<int> argNumbers;
    for (int i = 0; i < ui->customFunctionDefinitionWidget->layout()->count(); i++) {
        QObject* o = ui->customFunctionDefinitionWidget->layout()->itemAt(i)->widget();
        OverloadBox* box = (OverloadBox*) o;

        if (!box->check()) return; //Error occurred during check, don't save

        QJsonObject save = box->save();
        int argCount = save.value("args").toArray().count();
        if (argNumbers.contains(argCount)) {
            //More than one overload with same number of arguments
            tToast* toast = new tToast();
            toast->setTitle(tr("Overload Arguments"));
            toast->setText(tr("Only one overload can have %n arguments", nullptr, argCount));
            toast->show(this);
            connect(toast, &tToast::dismissed, toast, &tToast::deleteLater);
            return;
        }
        argNumbers.append(argCount);
        overloads.append(save);
    }
    obj.insert("overloads", overloads);

    QJsonDocument doc(obj);
    QSettings settings;
    settings.beginGroup("customFunctions");

    //Save the function
    if (editingFunction != "" && settings.contains(ui->functionName->text())) {
        settings.remove(ui->functionName->text());
    }
    settings.setValue(ui->functionName->text(), doc.toBinaryData());

    //Load up all the custom functions
    ui->customFunctionsList->clear();
    for (QString key : settings.allKeys()) {
        ui->customFunctionsList->addItem(key);
    }

    settings.endGroup();
    settings.sync();

    EvaluationEngine::setupFunctions();

    ui->stackedWidget->setCurrentIndex(1);
}

void MainWindow::on_customFunctionsList_itemActivated(QListWidgetItem *item)
{
    editingFunction = item->text();

    QSettings settings;
    settings.beginGroup("customFunctions");
    QJsonDocument doc = QJsonDocument::fromBinaryData(settings.value(item->text()).toByteArray());
    settings.endGroup();

    QJsonObject obj = doc.object();
    ui->functionName->setText(obj.value("name").toString());

    //Clear overloads
    QLayoutItem* i = ui->customFunctionDefinitionWidget->layout()->takeAt(0);
    while (i != nullptr) {
        i->widget()->deleteLater();
        i = ui->customFunctionDefinitionWidget->layout()->takeAt(0);
    }

    QJsonArray overloads = obj.value("overloads").toArray();
    for (QJsonValue v : overloads) {
        OverloadBox* overload = new OverloadBox();
        overload->load(v.toObject());
        connect(overload, &OverloadBox::remove, [=] {
            ui->customFunctionDefinitionWidget->layout()->removeWidget(overload);
            overload->deleteLater();
        });
        ui->customFunctionDefinitionWidget->layout()->addWidget(overload);
    }

    ui->stackedWidget->setCurrentIndex(2);
}

void MainWindow::on_calcWidget_manageFunctions()
{
    ui->stackedWidget->setCurrentIndex(1);
}

void MainWindow::on_calcWidget_sizeHintChanged()
{
    if (ui->stackedWidget->currentIndex() == 0) {
        this->setFixedWidth(ui->calcWidget->sizeHint().width());
    }
}

void MainWindow::on_stackedWidget_currentChanged(int arg1)
{
    int newWidth = -1;
    switch (arg1) {
        case 0:
            newWidth = ui->calcWidget->sizeHint().width();
            break;
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
}

void MainWindow::on_actionGradians_triggered(bool checked)
{
    if (checked) {
        EvaluationEngine::setTrigonometricUnit(EvaluationEngine::Gradians);
    }
}

void MainWindow::on_customFunctionsList_customContextMenuRequested(const QPoint &pos)
{
    QListWidgetItem* item = ui->customFunctionsList->itemAt(pos);
    if (item != nullptr) {
        QMenu* menu = new QMenu();
        menu->addSection(tr("For %1").arg(item->text()));
        menu->addAction(QIcon::fromTheme("edit-delete"), tr("Delete"), [=] {
            QSettings settings;
            settings.beginGroup("customFunctions");
            settings.remove(item->text());
            settings.endGroup();
            settings.sync();

            EvaluationEngine::setupFunctions();

            ui->customFunctionsList->removeItemWidget(item);
        });
        menu->exec(ui->customFunctionsList->mapToGlobal(pos));
        menu->deleteLater();
    }
}
