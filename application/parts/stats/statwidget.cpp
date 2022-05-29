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

StatWidget::StatWidget(QWidget* parent) :
    QWidget(parent),
    ui(new Ui::StatWidget) {
    ui->setupUi(this);

    ui->dataTable->setColumnCount(2);
    ui->dataTable->setHorizontalHeaderLabels({tr("Value"), tr("Frequency")});
    ui->dataTable->setRowCount(1);

    QSpinBox* freqBox = new QSpinBox();
    freqBox->setMinimum(1);
    freqBox->setMaximum(9999999);
    freqBox->setFrame(false);
    connect(freqBox, QOverload<int>::of(&QSpinBox::valueChanged), [ = ] {
        on_dataTable_cellChanged(0, 1);
    });
    ui->dataTable->setCellWidget(0, 1, freqBox);
}

StatWidget::~StatWidget() {
    delete ui;
}

void StatWidget::on_dataTable_cellChanged(int row, int column) {
    QTableWidgetItem* firstColData = ui->dataTable->item(row, 0);
    if ((!firstColData || firstColData->text() == "") && ui->dataTable->rowCount() > 1) {
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
        freqBox->setFrame(false);
        connect(freqBox, QOverload<int>::of(&QSpinBox::valueChanged), [ = ] {
            on_dataTable_cellChanged(newRow, 1);
        });
        ui->dataTable->setCellWidget(newRow, 1, freqBox);
    }
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
                    data.append(res.result.real());
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

    if (data.isEmpty()) {
        ui->minLabel->setText(tr("no data"));
        ui->maxLabel->setText(tr("no data"));
        ui->medLabel->setText(tr("no data"));
        ui->meanLabel->setText(tr("no data"));
        ui->sumLabel->setText(tr("no data"));
        ui->sumSquaredLabel->setText(tr("no data"));
    } else {
        //Sort the data
        //Needed to find median, min, etc.
        std::sort(data.begin(), data.end());

        ui->minLabel->setText(QString::number(data.first()));
        ui->maxLabel->setText(QString::number(data.last()));
        if (data.count() % 2 == 1) {
            //We have one single number for the median
            ui->medLabel->setText(QString::number(data.at(data.count() / 2)));
        } else {
            //We've got two numbers that we need to average
            ui->medLabel->setText(QString::number((data.at(data.count() / 2 - 1) + data.at(data.count() / 2)) / 2));
        }

        //Populate a string list
        QStringList addends;
        for (double val : data) {
            addends.append(QString::number(val));
        }

        //Calculate the mean
        EvaluationEngine::evaluate("(" + addends.join("+") + ")/" + QString::number(data.count()))->then([ = ](EvaluationEngine::Result result) {
            if (result.type == EvaluationEngine::Result::Scalar) {
                ui->meanLabel->setText(idbToString(result.result));
            } else {
                ui->meanLabel->setText(tr("undefined"));
            }
        });

        //Calculate the sum
        EvaluationEngine::evaluate(addends.join("+"))->then([ = ](EvaluationEngine::Result result) {
            if (result.type == EvaluationEngine::Result::Scalar) {
                ui->sumLabel->setText(idbToString(result.result));
            } else {
                ui->sumLabel->setText(tr("undefined"));
            }
        });

        //Calculate the sum squared
        EvaluationEngine::evaluate(addends.join("^2+") + "^2")->then([ = ](EvaluationEngine::Result result) {
            if (result.type == EvaluationEngine::Result::Scalar) {
                ui->sumSquaredLabel->setText(idbToString(result.result));
            } else {
                ui->sumSquaredLabel->setText(tr("undefined"));
            }
        });
    }
}
