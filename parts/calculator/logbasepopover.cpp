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
#include "logbasepopover.h"
#include "ui_logbasepopover.h"

#include <terrorflash.h>

LogBasePopover::LogBasePopover(QWidget* parent) :
    QWidget(parent),
    ui(new Ui::LogBasePopover) {
    ui->setupUi(this);

    this->setFocusProxy(ui->baseEdit);
    ui->titleLabel->setBackButtonShown(true);
}

LogBasePopover::~LogBasePopover() {
    delete ui;
}

void LogBasePopover::on_okButton_clicked() {
    QString acceptText;
    QString baseText = ui->baseEdit->text();
    QString number = ui->numberEdit->text();

    bool hasError = false;
    if (number.isEmpty()) {
        hasError = true;
        tErrorFlash::flashError(ui->numberEdit);
    }
    if (hasError) return;

    if (baseText == "e") {
        emit accepted(QStringLiteral("ln(%1)").arg(number));
    } else if (baseText == "10" || baseText.isEmpty()) {
        emit accepted(QStringLiteral("log(%1)").arg(number));
    } else {
        emit accepted(QStringLiteral("log(%1,%2)").arg(number).arg(baseText));
    }
}

void LogBasePopover::on_titleLabel_backButtonClicked() {
    emit rejected();
}
