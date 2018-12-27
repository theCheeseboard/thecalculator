#include "conditionbox.h"
#include "ui_conditionbox.h"

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
