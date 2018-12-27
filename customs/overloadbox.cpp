#include "overloadbox.h"
#include "ui_overloadbox.h"

#include <QLineEdit>
#include <QJsonArray>
#include "branchbox.h"

OverloadBox::OverloadBox(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::OverloadBox)
{
    ui->setupUi(this);

    //Add an argument
    ui->addArgButton->click();

    //Add a branch
    BranchBox* branch = new BranchBox(true);
    ui->branchesWidget->layout()->addWidget(branch);
}

OverloadBox::~OverloadBox()
{
    delete ui;
}

void OverloadBox::on_addArgButton_clicked()
{
    QLineEdit* edit = new QLineEdit;
    argumentNames.append(edit);
    edit->setPlaceholderText(tr("Argument %n Name", nullptr, argumentNames.count()));
    ui->argumentsBox->layout()->addWidget(edit);
}

void OverloadBox::on_removeArgButton_clicked()
{
    if (argumentNames.count() > 1) {
        ui->argumentsBox->layout()->removeWidget(argumentNames.last());
        argumentNames.takeLast()->deleteLater();
    }
}

void OverloadBox::on_removeOverloadButton_clicked()
{
    emit remove();
}


void OverloadBox::on_addBranchButton_clicked()
{
    BranchBox* branch = new BranchBox(false);
    connect(branch, &BranchBox::remove, [=] {
        ui->branchesWidget->layout()->removeWidget(branch);
        branch->deleteLater();
    });

    QBoxLayout* layout = ((QBoxLayout*) ui->branchesWidget->layout());
    layout->insertWidget(layout->count() - 1, branch);
}

bool OverloadBox::check() {
    return true;
}

QJsonObject OverloadBox::save() {
    QJsonObject obj;

    QJsonArray args;
    for (QLineEdit* e : argumentNames) {
        args.append(e->text());
    }
    obj.insert("args", args);

    QJsonArray branches;
    for (int i = 0; i < ui->branchesWidget->layout()->count(); i++) {
        QObject* o = ui->branchesWidget->layout()->itemAt(i)->widget();
        branches.append(((BranchBox*) o)->save());
    }
    obj.insert("branches", branches);

    return obj;
}

void OverloadBox::load(QJsonObject obj) {
    //Populate arguments
    QJsonArray args = obj.value("args").toArray();
    argumentNames.clear();
    QLayoutItem* i = ui->argumentsBox->layout()->takeAt(0);
    while (i != nullptr) {
        i->widget()->deleteLater();
        i = ui->argumentsBox->layout()->takeAt(0);
    }
    for (QJsonValue v : args) {
        QLineEdit* edit = new QLineEdit;
        argumentNames.append(edit);
        edit->setPlaceholderText(tr("Argument %n Name", nullptr, argumentNames.count()));
        edit->setText(v.toString());
        ui->argumentsBox->layout()->addWidget(edit);
    }

    //Populate branches
    QJsonArray branches = obj.value("branches").toArray();
    i = ui->branchesWidget->layout()->takeAt(0);
    while (i != nullptr) {
        i->widget()->deleteLater();
        i = ui->branchesWidget->layout()->takeAt(0);
    }

    for (QJsonValue v : branches) {
        BranchBox* branch = new BranchBox(false);
        branch->load(v.toObject());
        connect(branch, &BranchBox::remove, [=] {
            ui->branchesWidget->layout()->removeWidget(branch);
            branch->deleteLater();
        });
        ui->branchesWidget->layout()->addWidget(branch);
    }
}
