#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPalette>
#include <QPushButton>
#include <tvariantanimation.h>
#include <QScrollArea>
#include "calcbutton.h"
#include <QLineEdit>
#include <QLabel>
#include <functional>
#include "expression.h"
#include "calc.h"
#include <complex>

typedef std::complex<double> idouble;

class CalcButton;
namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

    public:
        explicit MainWindow(QWidget *parent = 0);
        ~MainWindow();
    
        QList<CalcButton*> buttons;

    public slots:
        void parserError(const char* error);
        void parserResult(double result);
        double callFunction(QString name, QList<double> args, QString& error);

    private slots:
        void on_expandButton_clicked();

        void ButtonPressed(QString text);

        void on_ClearButton_clicked();

        void on_BackspaceButton_clicked();

        void on_EqualButton_clicked();

        void on_expressionBox_textEdited(const QString &arg1);

    private:
        Ui::MainWindow *ui;

        bool extended = false;
        QMap<QString, std::function<double(QList<double>,QString&)>> customFunctions;
        void setupBuiltinFunctions();
        std::function<double(QList<double>,QString&)> createSingleArgFunction(std::function<double(double, QString&)> fn, QString fnName);

        YY_BUFFER_STATE bufferState;
};

#endif // MAINWINDOW_H
