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
#include "addfunctiondialog.h"
#include "ui_addfunctiondialog.h"

#include <QPainter>
#include <QPixmap>

AddFunctionDialog::AddFunctionDialog(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AddFunctionDialog)
{
    ui->setupUi(this);

    ui->colorBox->addItem(getColIcon(QColor(0, 200, 0)), tr("Green"), QColor(0, 200, 0));
    ui->colorBox->addItem(getColIcon(QColor(200, 0, 0)), tr("Red"), QColor(200, 0, 0));
    ui->colorBox->addItem(getColIcon(QColor(0, 100, 255)), tr("Aqua"), QColor(0, 100, 255));
    ui->colorBox->addItem(getColIcon(QColor(85, 0, 255)), tr("Purple"), QColor(85, 0, 255));
}

AddFunctionDialog::~AddFunctionDialog()
{
    delete ui;
}

void AddFunctionDialog::on_backButton_clicked()
{
    emit finished();
}

void AddFunctionDialog::on_doneButton_clicked()
{
    emit expressionSet(ui->expressionBox->getFixedExpression(), ui->colorBox->currentData().value<QColor>());
    emit finished();
}

QIcon AddFunctionDialog::getColIcon(QColor col) {
    QPixmap px(16, 16);
    QPainter painter(&px);
    painter.fillRect(0, 0, 16, 16, col);
    return QIcon(px);
}
