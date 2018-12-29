#include "expressionlineedit.h"

#include <QKeyEvent>

ExpressionLineEdit::ExpressionLineEdit(QWidget *parent) : QLineEdit(parent)
{
    connect(this, &QLineEdit::textEdited, [=](QString arg1) {
        int anchorPosition = this->cursorPosition();

        QString newString = arg1;
        if (newString.contains("/")) newString.replace("/", "÷");
        if (newString.contains("*")) newString.replace("*", "×");
        if (newString.contains(" ")) newString.replace(" ", "⋅");
        this->setText(newString);
        this->setCursorPosition(anchorPosition);

        emit expressionUpdated(newString);
    });
}

void ExpressionLineEdit::keyPressEvent(QKeyEvent *event) {
    if (event->modifiers() & Qt::ControlModifier) {
        switch (event->key()) {
            case Qt::Key_P: this->insert("π"); break;
            case Qt::Key_0: this->insert("⁰"); break;
            case Qt::Key_1: this->insert("¹"); break;
            case Qt::Key_2: this->insert("²"); break;
            case Qt::Key_3: this->insert("³"); break;
            case Qt::Key_4: this->insert("⁴"); break;
            case Qt::Key_5: this->insert("⁵"); break;
            case Qt::Key_6: this->insert("⁶"); break;
            case Qt::Key_7: this->insert("⁷"); break;
            case Qt::Key_8: this->insert("⁸"); break;
            case Qt::Key_9: this->insert("⁹"); break;
            case Qt::Key_I: this->insert("ⁱ"); break;
            case Qt::Key_Equal: this->insert("⁺"); break;
            case Qt::Key_Minus: this->insert("⁻"); break;
            case Qt::Key_R: this->insert("√"); break;
            default: QLineEdit::keyPressEvent(event);
        }
    } else if (this->cursorPosition() != 0) {
        QChar charBefore = this->text().at(this->cursorPosition() - 1);
        if (event->text() == "=") {
            if (charBefore == '>') {
                //Change into greater than or equal to sign
                this->backspace();
                this->insert("≥");
                return; //Don't do the normal stuff
            } else if (charBefore == '<') {
                //Change into less than or equal to sign
                this->backspace();
                this->insert("≤");
                return; //Don't do the normal stuff
            } else if (charBefore == '!') {
                //Change into not equal to sign
                this->backspace();
                this->insert("≠");
                return; //Don't do the normal stuff
            }
        } else if (event->text() == "/") {
            if (charBefore == '=') {
                //Change into not equal to sign
                this->backspace();
                this->insert("≠");
                return; //Don't do the normal stuff
            }
        }
    }

    QLineEdit::keyPressEvent(event);
}
