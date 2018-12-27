#ifndef OVERLOADBOX_H
#define OVERLOADBOX_H

#include <QFrame>
#include <QLineEdit>
#include <QJsonObject>

namespace Ui {
    class OverloadBox;
}

class OverloadBox : public QFrame
{
        Q_OBJECT

    public:
        explicit OverloadBox(QWidget *parent = nullptr);
        ~OverloadBox();

        bool check();
        QJsonObject save();
        void load(QJsonObject obj);

    private slots:
        void on_addArgButton_clicked();

        void on_removeArgButton_clicked();

        void on_removeOverloadButton_clicked();

        void on_addBranchButton_clicked();

    signals:
        void remove();

    private:
        Ui::OverloadBox *ui;

        QList<QLineEdit*> argumentNames;
};

#endif // OVERLOADBOX_H
