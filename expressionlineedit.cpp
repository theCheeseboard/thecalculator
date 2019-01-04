/****************************************
 *
 *   theCalculator - Calculator
 *   Copyright (C) 2019 Victor Tran
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * *************************************/

#include "expressionlineedit.h"

#include <QKeyEvent>
#include <QTextLayout>

#define TYPED_PLACEHOLDER 0x200B

ExpressionLineEdit::ExpressionLineEdit(QWidget *parent) : QLineEdit(parent)
{

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
        return;
    } else if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
        emit returnPressed();
        return;
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

    if (event->key() == Qt::Key_Backspace) {
        this->backspace();
    } else if (event->key() == Qt::Key_Delete) {
        this->del();
    } else if (event->text() == "") {
        QLineEdit::keyPressEvent(event);
    } else {
        this->insert(event->text());
    }
}

void ExpressionLineEdit::insert(QString text) {
    errorLength = 0;
    int curPos = this->cursorPosition();

    QString leftPart = typedExpr.left(curPos);
    for (int i = 0; i < leftPart.count(); i++) {
        if (leftPart.at(i) == QChar(TYPED_PLACEHOLDER)) {
            leftPart.replace(i, 1, fixedExpr.at(i));
        }
    }
    typedExpr.replace(0, curPos, leftPart);
    typedExpr.insert(curPos, text);
    checkText(typedExpr, curPos + text.length());
}

void ExpressionLineEdit::backspace() {
    errorLength = 0;
    int curPos = this->cursorPosition();
    if (this->selectionLength() != 0) {
        this->deleteRange(this->selectionStart(), this->selectionLength());
    } else if (curPos != 0) {
        typedExpr.remove(curPos - 1, 1);
        checkText(typedExpr, --curPos);
    }
}

void ExpressionLineEdit::del() {
    errorLength = 0;
    int curPos = this->cursorPosition();
    if (this->selectionLength() != 0) {
        this->deleteRange(this->selectionStart(), this->selectionLength());
    } else if (curPos != 0) {
        typedExpr.remove(curPos, 1);
        checkText(typedExpr, curPos);
    }
}

void ExpressionLineEdit::deleteRange(int start, int length) {
    typedExpr.remove(start, length);
    checkText(typedExpr, start);
}

void ExpressionLineEdit::checkText(QString text, int originalPos) {
    int oldPos = this->cursorPosition();
    for (int i = 0; i < text.count(); i++) {
        QChar c = text.at(i);
        if (c.unicode() == TYPED_PLACEHOLDER) {
            //Remove zero width spaces
            text = text.remove(i, 1);
            if (i < originalPos) originalPos--;
            i--;
        } else if (c == "/") {
            text = text.replace(i, 1, "÷");
        } else if (c == "*") {
            text = text.replace(i, 1, "×");
        } else if (c == " ") {
            text = text.replace(i, 1, "⋅");
        }
    }

    typedExpr = text;
    fixedExpr = text;

    int bracketCount = 0;
    for (int i = 0; i < fixedExpr.length(); i++) {
        if (fixedExpr.at(i) == '(') {
            bracketCount++;
        } else if (fixedExpr.at(i) == ')') {
            bracketCount--;
        }
    }

    while (bracketCount > 0) {
        fixedExpr.append(')');
        typedExpr.append(QChar(TYPED_PLACEHOLDER));
        bracketCount--;
    }

    this->blockSignals(true);
    this->setText(fixedExpr);
    this->blockSignals(false);
    this->setCursorPosition(originalPos);
    emit cursorPositionChanged(oldPos, originalPos);

    recolorText();

    emit expressionUpdated(fixedExpr);
}

void ExpressionLineEdit::recolorText() {

    QTextCharFormat grayFormat;
    grayFormat.setForeground(this->palette().brush(QPalette::Disabled, QPalette::WindowText));

    QList<QInputMethodEvent::Attribute> attributes;
    for (int i = 0; i < typedExpr.count(); i++) {
        if (typedExpr.at(i) == QChar(TYPED_PLACEHOLDER)) {
            attributes.append(QInputMethodEvent::Attribute(QInputMethodEvent::TextFormat, i - this->cursorPosition(), 1, grayFormat));
        }
    }
    if (errorLength != 0) {
        QTextCharFormat errFormat;
        errFormat.setUnderlineStyle(QTextCharFormat::SingleUnderline);
        errFormat.setUnderlineColor(QColor(255, 0, 0));

        attributes.append(QInputMethodEvent::Attribute(QInputMethodEvent::TextFormat, errorStart - this->cursorPosition(), errorLength, errFormat));
    }
    QInputMethodEvent event(QString(), attributes);
    inputMethodEvent(&event);
}

void ExpressionLineEdit::clear() {
    typedExpr = "";
    fixedExpr = "";
    checkText(typedExpr, 0);
}

void ExpressionLineEdit::setExpression(QString expr) {
    checkText(expr, expr.count());
}

QString ExpressionLineEdit::getFixedExpression() {
    return fixedExpr;
}

QString ExpressionLineEdit::getTypedExpression() {
    return typedExpr.remove(QChar(TYPED_PLACEHOLDER));
}

void ExpressionLineEdit::setErrorRange(int start, int length) {
    errorStart = start;
    errorLength = length;

    recolorText();
}
