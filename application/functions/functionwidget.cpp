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
#include "functionwidget.h"
#include "ui_functionwidget.h"

#include "evaluationengine.h"
#include <QMessageBox>
#include <ttoast.h>
#include <QSettings>
#include <QJsonArray>
#include <QJsonDocument>
#include <QMenu>
#include "customs/overloadbox.h"

FunctionWidget::FunctionWidget(QWidget *parent) :
    tStackedWidget(parent),
    ui(new Ui::FunctionWidget)
{
    ui->setupUi(this);

    QSettings settings;
    settings.beginGroup("customFunctions");

    //Load up all the custom functions
    ui->customFunctionsList->clear();
    for (QString key : settings.allKeys()) {
        ui->customFunctionsList->addItem(key);
    }
    settings.endGroup();

    this->setCurrentAnimation(SlideHorizontal);
}

FunctionWidget::~FunctionWidget()
{
    delete ui;
}

void FunctionWidget::on_addCustomFunction_clicked()
{
    editingFunction = "";
    ui->functionName->setText("");
    QLayoutItem* i = ui->customFunctionDefinitionWidget->layout()->takeAt(0);
    while (i != nullptr) {
        i->widget()->deleteLater();
        i = ui->customFunctionDefinitionWidget->layout()->takeAt(0);
    }

    this->setCurrentIndex(1);
}

void FunctionWidget::on_backButton_2_clicked()
{
    QMessageBox::StandardButton b = QMessageBox::warning(this, tr("Save this function?"), tr("Do you want to save this function?"), QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel, QMessageBox::Save);
    if (b == QMessageBox::Save) {
        //Save the function
        ui->saveCustomFunctionButton->click();
    } else if (b == QMessageBox::Discard) {
        //Don't save the function
        this->setCurrentIndex(0);
    }
}

void FunctionWidget::on_newOverloadButton_clicked()
{
    OverloadBox* overload = new OverloadBox();
    connect(overload, &OverloadBox::remove, [=] {
        ui->customFunctionDefinitionWidget->layout()->removeWidget(overload);
        overload->deleteLater();
    });
    ui->customFunctionDefinitionWidget->layout()->addWidget(overload);
}

void FunctionWidget::on_saveCustomFunctionButton_clicked()
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

    this->setCurrentIndex(0);
}

void FunctionWidget::on_customFunctionsList_itemActivated(QListWidgetItem *item)
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

    this->setCurrentIndex(1);
}

void FunctionWidget::on_customFunctionsList_customContextMenuRequested(const QPoint &pos)
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
