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
#include "statwidget.h"
#include "ui_statwidget.h"

#include <QSpinBox>
#include "evaluationengine.h"

StatWidget::StatWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::StatWidget)
{
    ui->setupUi(this);

    ui->dataTable->setColumnCount(2);
    ui->dataTable->setHorizontalHeaderLabels({tr("Value"), tr("Frequency")});
    ui->dataTable->setRowCount(1);

    QSpinBox* freqBox = new QSpinBox();
    freqBox->setMinimum(1);
    freqBox->setMaximum(9999999);
    connect(freqBox, QOverload<int>::of(&QSpinBox::valueChanged), [=] {
        on_dataTable_cellChanged(0, 1);
    });
    ui->dataTable->setCellWidget(0, 1, freqBox);
}

StatWidget::~StatWidget()
{
    delete ui;
}

void StatWidget::on_dataTable_cellChanged(int row, int column)
{
    QTableWidgetItem* firstColData = ui->dataTable->item(row, 0);
    if (!firstColData || firstColData->text() == "") {
        //Remove this row; we don't need it any more
        ui->dataTable->removeRow(row);
    }

    calculateData();

    QTableWidgetItem* secondLastItem = ui->dataTable->item(ui->dataTable->rowCount() - 1, 0);
    if (secondLastItem && secondLastItem->text() != "") {
        //Add an extra row
        int newRow = ui->dataTable->rowCount();
        ui->dataTable->setRowCount(newRow + 1);

        QSpinBox* freqBox = new QSpinBox();
        freqBox->setMinimum(1);
        freqBox->setMaximum(9999999);
        connect(freqBox, QOverload<int>::of(&QSpinBox::valueChanged), [=] {
            on_dataTable_cellChanged(newRow, 1);
        });
        ui->dataTable->setCellWidget(newRow, 1, freqBox);
    }

    //Calculate the mean

}

void StatWidget::calculateData() {
    QVector<double> data;
    for (int i = 0; i < ui->dataTable->rowCount(); i++) {
        QTableWidgetItem* firstColData = ui->dataTable->item(i, 0);
        if (!firstColData) continue;

        EvaluationEngine engine;
        engine.setExpression(firstColData->text());
        EvaluationEngine::Result res = engine.evaluate();

        switch (res.type) {
            case EvaluationEngine::Result::Scalar: {
                QSpinBox* freqData = (QSpinBox*) ui->dataTable->cellWidget(i, 1);
                for (int i = 0; i < freqData->value(); i++) {
                    data.append(res.value.real());
                }
                break;
            }
            default:
                //This is an error
                break;
        }
    }

    QLocale locale;
    ui->countLabel->setText(locale.toString(data.count()));
}
