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
#include "nthrootpopover.h"
#include "ui_nthrootpopover.h"

#include <terrorflash.h>

NthRootPopover::NthRootPopover(QWidget* parent) :
    QWidget(parent),
    ui(new Ui::NthRootPopover) {
    ui->setupUi(this);

    this->setFocusProxy(ui->baseEdit);
    ui->titleLabel->setBackButtonShown(true);
}

NthRootPopover::~NthRootPopover() {
    delete ui;
}

void NthRootPopover::on_okButton_clicked() {
    QString acceptText;
    QString baseText = ui->baseEdit->text();
    QString number = ui->numberEdit->text();
    bool isBaseReal;
    baseText.toDouble(&isBaseReal);

    bool hasError = false;
    if (baseText.isEmpty()) {
        hasError = true;
        tErrorFlash::flashError(ui->baseEdit);
    }
    if (number.isEmpty()) {
        hasError = true;
        tErrorFlash::flashError(ui->numberEdit);
    }
    if (hasError) return;

    if (isBaseReal) {
        QString expBase = baseText.replace("0", "⁰")
            .replace("1", "¹")
            .replace("2", "²")
            .replace("3", "³")
            .replace("4", "⁴")
            .replace("5", "⁵")
            .replace("6", "⁶")
            .replace("7", "⁷")
            .replace("8", "⁸")
            .replace("9", "⁹");

        emit accepted(QStringLiteral("%1√(%2)").arg(expBase).arg(number));
    } else {
        emit accepted(QStringLiteral("(%2)^(1/(%1))").arg(baseText).arg(number));
    }
}

void NthRootPopover::on_titleLabel_backButtonClicked() {
    emit rejected();
}
