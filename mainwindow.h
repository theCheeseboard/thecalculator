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
#include <iomanip>

typedef std::complex<long double> idouble;

class CalcButton;
namespace Ui {
class MainWindow;
}

struct Result {
    QString error;
    idouble result;
    bool assigned;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

    public:
        explicit MainWindow(QWidget *parent = 0);
        ~MainWindow();
    
        QList<CalcButton*> buttons;
        void assignValue(QString identifier, idouble value);

        QString evaluateExpression(QString expression);
        static void setupBuiltinFunctions();


    public slots:
        void parserError(const char* error);
        void parserResult(idouble result);

    private slots:
        void on_expandButton_clicked();

        void ButtonPressed(QString text);

        void on_ClearButton_clicked();

        void on_BackspaceButton_clicked();

        void on_EqualButton_clicked();

        void on_expressionBox_textEdited(const QString &arg1);

        void on_actionExit_triggered();

        void on_backButton_clicked();

        void on_FunctionsButton_clicked();

        void on_expressionBox_returnPressed();

        void on_actionAbout_triggered();

        void on_actionDegrees_triggered(bool checked);

        void on_actionRadians_triggered(bool checked);

        void on_actionTheCalculatorHelp_triggered();

        void on_actionFileBug_triggered();

        void on_actionSources_triggered();

    private:
        Ui::MainWindow *ui;

        bool extended = false;
        bool resultSuccess = false;
        static std::function<idouble(QList<idouble>,QString&)> createSingleArgFunction(std::function<idouble(idouble, QString&)> fn, QString fnName);
        void resizeAnswerLabel();

        static idouble toRad(idouble deg);
        static idouble toDeg(idouble rad);

        void resizeEvent(QResizeEvent* event);
        void changeEvent(QEvent* event);
        bool eventFilter(QObject *watched, QEvent *event);

        idouble currentAnswer;

        YY_BUFFER_STATE bufferState;
};

#endif // MAINWINDOW_H
