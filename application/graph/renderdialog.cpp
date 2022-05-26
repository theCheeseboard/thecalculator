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
#include "renderdialog.h"
#include "ui_renderdialog.h"

#include <QFileDialog>
#include <tpopover.h>
#include "graphview.h"

RenderDialog::RenderDialog(GraphView* view, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::RenderDialog)
{
    ui->setupUi(this);

    this->v = view;
}

RenderDialog::~RenderDialog()
{
    delete ui;
}

void RenderDialog::on_browseForFileButton_clicked()
{
    QFileDialog* d = new QFileDialog();
    d->setAcceptMode(QFileDialog::AcceptSave);
    d->setNameFilter(tr("Portable Network Graphics (*.png)"));
    if (d->exec() == QFileDialog::Accepted) {
        ui->fileLocation->setText(d->selectedFiles().first());
    }
}

void RenderDialog::on_backButton_clicked()
{
    emit dismiss();
}

void RenderDialog::on_doneButton_clicked()
{
    QSizeF size(ui->imageWidth->value(), ui->imageHeight->value());
    QImage image(size.toSize(), QImage::Format_ARGB32);
    QPainter p(&image);

    v->render(&p, size);

    image.save(ui->fileLocation->text());
    emit dismiss();
}
