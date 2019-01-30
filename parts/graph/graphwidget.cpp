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
#include "graphwidget.h"
#include "ui_graphwidget.h"

#include "addfunctiondialog.h"
#include <tpopover.h>
#include "graphfunction.h"

struct GraphWidgetPrivate {
    QVector<GraphFunction*> displayedFunctions;
};

GraphWidget::GraphWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::GraphWidget)
{
    ui->setupUi(this);

    d = new GraphWidgetPrivate();
    ui->rightPane->setFixedWidth(300 * theLibsGlobal::getDPIScaling());
}

GraphWidget::~GraphWidget()
{
    delete ui;
}

void GraphWidget::on_addEquationButton_clicked()
{
    AddFunctionDialog* d = new AddFunctionDialog();
    tPopover* p = new tPopover(d);
    p->setPopoverWidth(300 * theLibsGlobal::getDPIScaling());
    p->setPerformBlanking(false);
    connect(d, &AddFunctionDialog::finished, [=] {
        p->dismiss();
    });
    connect(d, &AddFunctionDialog::expressionSet, [=](QString expression, QColor col) {
        ui->equationsList->addItem(expression);

        GraphFunction* fn = new GraphFunction(ui->graphicsView, expression);
        fn->setColor(col);
        ui->graphicsView->scene()->addItem(fn);
        this->d->displayedFunctions.append(fn);
    });
    p->show(this);
}

void GraphWidget::on_centerXBox_valueChanged(double arg1)
{
    ui->graphicsView->setCenter(QPoint(ui->centerXBox->value(), ui->centerYBox->value()));
}

void GraphWidget::on_centerYBox_valueChanged(double arg1)
{
    ui->graphicsView->setCenter(QPoint(ui->centerXBox->value(), ui->centerYBox->value()));
}

void GraphWidget::on_scaleXBox_valueChanged(double arg1)
{
    ui->graphicsView->setXScale(arg1);
}


void GraphWidget::on_scaleYBox_valueChanged(double arg1)
{
    ui->graphicsView->setYScale(arg1);
}
