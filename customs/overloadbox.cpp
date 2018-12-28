#include "overloadbox.h"
#include "ui_overloadbox.h"

#include <QLineEdit>
#include <QJsonArray>
#include <ttoast.h>
#include <tvariantanimation.h>
#include "branchbox.h"

OverloadBox::OverloadBox(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::OverloadBox)
{
    ui->setupUi(this);

    ui->removeOverloadButton->setProperty("type", "destructive");

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
    for (QLineEdit* e : argumentNames) {
        if (e->text() == "") {
            //Each argument needs a name
            tToast* toast = new tToast();
            toast->setTitle(tr("Overload Arguments"));
            toast->setText(tr("Each argument must have a name"));
            toast->show(this->window());
            connect(toast, &tToast::dismissed, toast, &tToast::deleteLater);
            this->flashError();
            return false;
        }
    }

    for (int i = 0; i < ui->branchesWidget->layout()->count(); i++) {
        QObject* o = ui->branchesWidget->layout()->itemAt(i)->widget();
        if (!((BranchBox*) o)->check()) return false;
    }

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

void OverloadBox::flashError() {
    tVariantAnimation* a = new tVariantAnimation();
    a->setStartValue(QColor(200, 0, 0));
    a->setEndValue(this->palette().color(QPalette::Window));
    a->setDuration(1000);
    a->setEasingCurve(QEasingCurve::Linear);
    connect(a, &tVariantAnimation::finished, a, &tVariantAnimation::deleteLater);
    connect(a, &tVariantAnimation::valueChanged, [=](QVariant value) {
        QPalette pal = this->palette();
        pal.setColor(QPalette::Window, value.value<QColor>());
        this->setPalette(pal);
    });
    a->start();
}
