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
        QString idbToString(idouble db);

    public slots:
        void parserError(const char* error);
        void parserResult(idouble result);
        idouble callFunction(QString name, QList<idouble> args, QString& error);

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
        QMap<QString, std::function<idouble(QList<idouble>,QString&)>> customFunctions;
        void setupBuiltinFunctions();
        std::function<idouble(QList<idouble>,QString&)> createSingleArgFunction(std::function<idouble(idouble, QString&)> fn, QString fnName);

        idouble currentAnswer;


        YY_BUFFER_STATE bufferState;
};

#endif // MAINWINDOW_H
