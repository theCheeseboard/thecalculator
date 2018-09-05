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

class MainWindow : public QMainWindow
{
    Q_OBJECT

    public:
        explicit MainWindow(QWidget *parent = 0);
        ~MainWindow();
    
        QList<CalcButton*> buttons;
        QString idbToString(idouble db);
        QString numberFormatToString(long double format);

    public slots:
        void parserError(const char* error);
        void parserResult(idouble result);
        void assignValue(QString identifier, idouble value);
        bool valueExists(QString identifer);
        idouble getValue(QString identifer);
        idouble callFunction(QString name, QList<idouble> args, QString& error);

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

    private:
        Ui::MainWindow *ui;

        bool extended = false;
        bool explicitEvaluation = false;
        bool resultSuccess = false;
        QMap<QString, std::function<idouble(QList<idouble>,QString&)>> customFunctions;
        void setupBuiltinFunctions();
        std::function<idouble(QList<idouble>,QString&)> createSingleArgFunction(std::function<idouble(idouble, QString&)> fn, QString fnName);
        void resizeAnswerLabel();

        idouble toRad(idouble deg);
        idouble toDeg(idouble rad);

        void resizeEvent(QResizeEvent* event);
        bool eventFilter(QObject *watched, QEvent *event);

        idouble currentAnswer;
        QMap<QString, idouble> variables;

        YY_BUFFER_STATE bufferState;
};

#endif // MAINWINDOW_H
