#include "branchbox.h"
#include "ui_branchbox.h"

#include "conditionbox.h"
#include <QJsonArray>
#include <tvariantanimation.h>
#include <ttoast.h>

BranchBox::BranchBox(bool isOtherwise, QWidget *parent) :
    QFrame(parent),
    ui(new Ui::BranchBox)
{
    ui->setupUi(this);

    this->isOtherwise = isOtherwise;
    init();
}

BranchBox::~BranchBox()
{
    delete ui;
}

void BranchBox::init()  {
    if (isOtherwise) {
        ui->conditionsWidget->setVisible(false);
        ui->returnLabel->setText(tr("OTHERWISE RETURN"));
        ui->addConditionButton->setVisible(false);
        ui->removeBranchButton->setVisible(false);
    } else {
        ConditionBox* cond = new ConditionBox(true); //First condition box
        ui->conditionsWidget->layout()->addWidget(cond);
    }
}

void BranchBox::on_addConditionButton_clicked()
{
    ConditionBox* cond = new ConditionBox(false);
    connect(cond, &ConditionBox::remove, [=] {
        ui->conditionsWidget->layout()->removeWidget(cond);
        cond->deleteLater();
    });
    ui->conditionsWidget->layout()->addWidget(cond);
}

void BranchBox::on_removeBranchButton_clicked()
{
    emit remove();
}

bool BranchBox::check() {
    if (ui->returnValue->text() == "") {
        //Return value required
        tToast* toast = new tToast();
        toast->setTitle(tr("Return value required"));
        toast->setText(tr("A return value is required for this branch"));
        toast->show(this->window());
        connect(toast, &tToast::dismissed, toast, &tToast::deleteLater);
        this->flashError();
        return false;
    }

    for (int i = 0; i < ui->conditionsWidget->layout()->count(); i++) {
        QObject* o = ui->conditionsWidget->layout()->itemAt(i)->widget();
        if (!((ConditionBox*) o)->check()) {
            return false;
        }
    }
    return true;
}

QJsonObject BranchBox::save() {
    QJsonObject obj;

    obj.insert("isOtherwise", isOtherwise);

    QJsonArray conditions;
    for (int i = 0; i < ui->conditionsWidget->layout()->count(); i++) {
        QObject* o = ui->conditionsWidget->layout()->itemAt(i)->widget();
        conditions.append(((ConditionBox*) o)->save());
    }
    obj.insert("conditions", conditions);

    QJsonObject ret;
    ret.insert("expression", ui->returnValue->text());
    ret.insert("isError", ui->errorCheck->isChecked());

    obj.insert("return", ret);

    return obj;
}

void BranchBox::on_errorCheck_toggled(bool checked)
{
    if (checked) {
        ui->returnValue->setPlaceholderText(tr("Error Description"));
    } else {
        ui->returnValue->setPlaceholderText(tr("Expression to calculate"));
    }
}

void BranchBox::load(QJsonObject obj) {
    this->isOtherwise = obj.value("isOtherwise").toBool();
    this->init();


    //Populate conditions
    QJsonArray conditions = obj.value("conditions").toArray();
    QLayoutItem* i = ui->conditionsWidget->layout()->takeAt(0);
    while (i != nullptr) {
        i->widget()->deleteLater();
        i = ui->conditionsWidget->layout()->takeAt(0);
    }

    for (QJsonValue v : conditions) {
        ConditionBox* cond = new ConditionBox(false);
        cond->load(v.toObject());
        connect(cond, &ConditionBox::remove, [=] {
            ui->conditionsWidget->layout()->removeWidget(cond);
            cond->deleteLater();
        });
        ui->conditionsWidget->layout()->addWidget(cond);
    }

    QJsonObject ret = obj.value("return").toObject();
    ui->returnValue->setText(ret.value("expression").toString());
    ui->errorCheck->setChecked(ret.value("isError").toBool());
}


void BranchBox::flashError() {
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
