#ifndef BRANCHBOX_H
#define BRANCHBOX_H

#include <QFrame>
#include <QJsonObject>

namespace Ui {
    class BranchBox;
}

class BranchBox : public QFrame
{
        Q_OBJECT

    public:
        explicit BranchBox(bool isOtherwise, QWidget *parent = nullptr);
        ~BranchBox();

        bool check();
        QJsonObject save();
        void load(QJsonObject obj);
        void flashError();

    signals:
        void remove();

    private slots:

        void on_addConditionButton_clicked();

        void on_removeBranchButton_clicked();

        void on_errorCheck_toggled(bool checked);

    private:
        Ui::BranchBox *ui;

        bool isOtherwise;
        void init();
};

#endif // BRANCHBOX_H
