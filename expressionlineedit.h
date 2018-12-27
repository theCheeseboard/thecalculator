#ifndef EXPRESSIONLINEEDIT_H
#define EXPRESSIONLINEEDIT_H

#include <QLineEdit>

class ExpressionLineEdit : public QLineEdit
{
        Q_OBJECT
    public:
        explicit ExpressionLineEdit(QWidget *parent = nullptr);

    signals:
        void expressionUpdated(QString expression);

    public slots:

    private:
        void keyPressEvent(QKeyEvent* event);
};

#endif // EXPRESSIONLINEEDIT_H
