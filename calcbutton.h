#ifndef CALCBUTTON_H
#define CALCBUTTON_H

#include <QWidget>
#include <QPushButton>
#include <QMouseEvent>
#include "mainwindow.h"

class MainWindow;
class CalcButton : public QPushButton
{
    Q_OBJECT
    Q_PROPERTY(QString typedOutput READ typedOutput WRITE setTypedOutput DESIGNABLE true)
    Q_PROPERTY(QString shiftedOutput READ shiftedOutput WRITE setShiftedOutput DESIGNABLE true)
public:
    explicit CalcButton(QWidget *parent = nullptr);

    QString typedOutput();
    QString shiftedOutput();
signals:
    void output(QString output);

public slots:
    void setTypedOutput(QString typed);
    void setShiftedOutput(QString typed);
    void setText(const QString &text);

private:
    QString to;
    QString so;

    void mousePressEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);
};

#endif // CALCBUTTON_H
