#ifndef CONDITIONBOX_H
#define CONDITIONBOX_H

#include <QWidget>
#include <QJsonObject>

namespace Ui {
    class ConditionBox;
}

class ConditionBox : public QWidget
{
        Q_OBJECT

    public:
        explicit ConditionBox(bool isFirst, QWidget *parent = nullptr);
        ~ConditionBox();

        bool check();
        QJsonObject save();
        void load(QJsonObject obj);
        void flashError();

    signals:
        void remove();

    private slots:
        void on_removeButton_clicked();

    private:
        Ui::ConditionBox *ui;

        bool isFirst;
        void init();
};

#endif // CONDITIONBOX_H
