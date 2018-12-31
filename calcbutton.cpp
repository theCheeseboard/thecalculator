#include "calcbutton.h"

extern MainWindow* MainWin;

CalcButton::CalcButton(QWidget *parent) : QPushButton(parent)
{
    /*connect(this, &CalcButton::clicked, [=] {
        emit output(to);
    });*/
    MainWin->buttons.append(this);
}

QString CalcButton::typedOutput() {
    return to;
}

void CalcButton::setTypedOutput(QString typed) {
    to = typed;
}

void CalcButton::setText(const QString &text) {
    QPushButton::setText(text);

    if (QRegularExpression("[a-z]+").match(text).captured() == text) {
        to = text + "(";
    } else {
        to = text;
    }
}

QString CalcButton::shiftedOutput() {
    return so;
}

void CalcButton::setShiftedOutput(QString typed) {
    so = typed;
}

void CalcButton::mousePressEvent(QMouseEvent *event) {
    /*if (event->button() == Qt::LeftButton) {
        emit output(to);
    } else if (event->button() == Qt::RightButton && so != "") {
        emit output(so);
    }*/
    this->animateClick(60000);
}

void CalcButton::mouseReleaseEvent(QMouseEvent *event) {
    this->animateClick(0);
    QRect geo = this->geometry();
    geo.moveTopLeft(QPoint(0, 0));
    if (geo.contains(event->pos())) {
        if (event->button() == Qt::LeftButton) {
            emit output(to);
        } else if (event->button() == Qt::RightButton && so != "") {
            emit output(so);
        }
    }
}

