#include "conditionbox.h"
#include "ui_conditionbox.h"

#include <tvariantanimation.h>
#include <ttoast.h>

ConditionBox::ConditionBox(bool isFirst, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ConditionBox)
{
    ui->setupUi(this);

    this->isFirst = isFirst;
    init();
}

ConditionBox::~ConditionBox()
{
    delete ui;
}

void ConditionBox::init() {
    if (isFirst) {
        ui->whenComboBox->setVisible(true);
        ui->comboBox->setVisible(false);
        ui->removeButton->setVisible(false);
    } else {
        ui->whenComboBox->setVisible(false);
        ui->comboBox->setVisible(true);
        ui->removeButton->setVisible(true);
    }
}

void ConditionBox::on_removeButton_clicked()
{
    emit remove();
}

bool ConditionBox::check() {
    if (ui->expressionBox->text() == "") {
        //Return value required
        tToast* toast = new tToast();
        toast->setTitle(tr("Expression required"));
        toast->setText(tr("An expression is required for this condition"));
        toast->show(this->window());
        connect(toast, &tToast::dismissed, toast, &tToast::deleteLater);
        this->flashError();
        return false;
    }
    return true;
}

QJsonObject ConditionBox::save() {
    QJsonObject obj;

    obj.insert("isFirst", isFirst);
    if (isFirst) {
        obj.insert("connective", ui->whenComboBox->currentIndex());
    } else {
        obj.insert("connective", ui->comboBox->currentIndex());
    }
    obj.insert("expression", ui->expressionBox->text());

    return obj;
}

void ConditionBox::load(QJsonObject obj) {
    this->isFirst = obj.value("isFirst").toBool();
    this->init();

    if (isFirst) {
        ui->whenComboBox->setCurrentIndex(obj.value("connective").toInt());
    } else {
        ui->comboBox->setCurrentIndex(obj.value("connective").toInt());
    }
    ui->expressionBox->setText(obj.value("expression").toString());
}


void ConditionBox::flashError() {
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
