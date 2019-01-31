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
#include <QMenu>
#include "graphfunction.h"
#include "renderdialog.h"

struct GraphWidgetPrivate {
    QVector<GraphFunction*> displayedFunctions;

    QMenu* menu;
};

GraphWidget::GraphWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::GraphWidget)
{
    ui->setupUi(this);

    d = new GraphWidgetPrivate();
    ui->rightPane->setFixedWidth(300 * theLibsGlobal::getDPIScaling());

    #if QT_VERSION >= QT_VERSION_CHECK(5, 12, 0)
        ui->scaleXBox->setStepType(QDoubleSpinBox::AdaptiveDecimalStepType);
        ui->scaleYBox->setStepType(QDoubleSpinBox::AdaptiveDecimalStepType);
    #endif

    d->menu = new QMenu(tr("Graphing"));
    d->menu->addAction(tr("Render Image"), [=] {
        RenderDialog* d = new RenderDialog(ui->graphicsView);
        tPopover* p = new tPopover(d);
        p->setPopoverWidth(400 * theLibsGlobal::getDPIScaling());
        connect(d, &RenderDialog::dismiss, p, &tPopover::dismiss);
        p->show(this->window());

    });
}

GraphWidget::~GraphWidget()
{
    delete ui;
}

QMenu* GraphWidget::getMenu() {
    return d->menu;
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
        ui->graphicsView->addFunction(fn);
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
    if (ui->linkCheck->isChecked()) {
        ui->scaleYBox->setValue(arg1);
    }
}


void GraphWidget::on_scaleYBox_valueChanged(double arg1)
{
    ui->graphicsView->setYScale(arg1);
    if (ui->linkCheck->isChecked()) {
        ui->scaleXBox->setValue(arg1);
    }
}

void GraphWidget::on_equationsList_customContextMenuRequested(const QPoint &pos)
{
    QModelIndex i = ui->equationsList->indexAt(pos);
    if (i.isValid()) {
        QMenu* menu = new QMenu();
        menu->addSection(tr("For %1").arg(i.data(Qt::DisplayRole).toString()));
        menu->addAction(QIcon::fromTheme("edit-delete"), tr("Remove"), [=] {
            GraphFunction* f = d->displayedFunctions.takeAt(i.row());
            ui->graphicsView->removeFunction(f);

            delete ui->equationsList->takeItem(i.row());
            delete f;
        });
        menu->exec(ui->equationsList->mapToGlobal(pos));
        menu->deleteLater();
    }
}

void GraphWidget::on_graphicsView_readyChanged(bool ready)
{

}
