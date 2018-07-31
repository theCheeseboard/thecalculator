#include "mainwindow.h"
#include "ui_mainwindow.h"

extern MainWindow* MainWin;
#include "calc.bison.hpp"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    MainWin = this;
    ui->setupUi(this);

    ui->ZeroButton->setShiftedOutput("⁰");
    ui->OneButton->setShiftedOutput("¹");
    ui->TwoButton->setShiftedOutput("²");
    ui->ThreeButton->setShiftedOutput("³");
    ui->FourButton->setShiftedOutput("⁴");
    ui->FiveButton->setShiftedOutput("⁵");
    ui->SixButton->setShiftedOutput("⁶");
    ui->SevenButton->setShiftedOutput("⁷");
    ui->EightButton->setShiftedOutput("⁸");
    ui->NineButton->setShiftedOutput("⁹");

    //ui->EqualButton->setProperty("type", "positive");
    QPalette operationButtonPalette = ui->EqualButton->palette();
    operationButtonPalette.setColor(QPalette::Button, operationButtonPalette.color(QPalette::Button).darker(150));
    ui->EqualButton->setPalette(operationButtonPalette);
    ui->PlusButton->setPalette(operationButtonPalette);
    ui->MinusButton->setPalette(operationButtonPalette);
    ui->MultiplyButton->setPalette(operationButtonPalette);
    ui->DivideButton->setPalette(operationButtonPalette);
    ui->PiButton->setPalette(operationButtonPalette);
    ui->eButton->setPalette(operationButtonPalette);
    ui->LeftBracketButton->setPalette(operationButtonPalette);
    ui->RightBracketButton->setPalette(operationButtonPalette);
    ui->expandButton->setPalette(operationButtonPalette);

    QPalette functionPalette = ui->scrollArea->palette();
    functionPalette.setColor(QPalette::Button, functionPalette.color(QPalette::Button).lighter(125));
    ui->scrollArea->setPalette(functionPalette);

    ui->scrollArea->setFixedWidth(0);
    this->setFixedWidth(this->sizeHint().width());

    ui->SquareButton->setTypedOutput("²");
    ui->CubeButton->setTypedOutput("³");
    ui->ExponentButton->setTypedOutput("^");
    for (CalcButton* b : buttons) {
        connect(b, SIGNAL(output(QString)), this, SLOT(ButtonPressed(QString)));
    }
    setupBuiltinFunctions();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_expandButton_clicked()
{
    tVariantAnimation* anim = new tVariantAnimation();
    anim->setStartValue(ui->scrollArea->width());
    if (extended) {
        anim->setEndValue(0);
        ui->expandButton->setIcon(QIcon::fromTheme("arrow-right"));
        extended = false;
    } else {
        anim->setEndValue(ui->scrollArea->sizeHint().width());
        ui->expandButton->setIcon(QIcon::fromTheme("arrow-left"));
        extended = true;
    }
    anim->setDuration(500);
    anim->setEasingCurve(QEasingCurve::OutCubic);
    connect(anim, &tVariantAnimation::valueChanged, [=](QVariant value) {
        ui->scrollArea->setFixedWidth(value.toInt());
        this->setFixedWidth(this->sizeHint().width());
    });
    connect(anim, SIGNAL(finished()), anim, SLOT(deleteLater()));
    anim->start();
}

void MainWindow::ButtonPressed(QString text) {
    ui->expressionBox->insert(text);
}

void MainWindow::on_ClearButton_clicked()
{
    ui->expressionBox->clear();
}

void MainWindow::on_BackspaceButton_clicked()
{
    ui->expressionBox->backspace();
}

void MainWindow::on_EqualButton_clicked()
{
    /*Expression e(ui->expressionBox->text());
    ui->expressionBox->setText(e.evaluate());*/
    QString expression = ui->expressionBox->text();
    bufferState = yy_scan_string(expression.append("\n").toUtf8().constData());
    yyparse();
    yy_delete_buffer(bufferState);
}

void MainWindow::on_expressionBox_textEdited(const QString &arg1)
{
    bufferState = yy_scan_string(QString(arg1 + "\n").toUtf8().constData());
    yyparse();
    yy_delete_buffer(bufferState);
}

void MainWindow::parserError(const char *error) {
    ui->answerLabel->setText(QString::fromLocal8Bit(error));
}

void MainWindow::parserResult(double result) {
    ui->answerLabel->setText(QString::number(result));
}

double MainWindow::callFunction(QString name, QList<double> args, QString& error) {
    qDebug() << "Calling function:" << name << "with arguments" << args;
    if (!customFunctions.contains(name)) {
        error = tr("%1: undefined function").arg(name);
        return 0;
    } else {
        return customFunctions.value(name)(args, error);
    }
}

std::function<double(QList<double>,QString&)> MainWindow::createSingleArgFunction(std::function<double (double, QString &)> fn, QString fnName) {
    return [=](QList<double> args, QString& error) -> double {
        if (args.length() != 1) {
            error = tr("%1: expected 1 argument, got %2").arg(fnName).arg(args.length());
            return 0;
        } else {
            return fn(args.first(), error);
        }
    };
}

void MainWindow::setupBuiltinFunctions() {

    customFunctions.insert("abs", createSingleArgFunction([=](double arg, QString& error) {
        return abs(arg);
    }, "abs"));
    customFunctions.insert("sqrt", createSingleArgFunction([=](double arg, QString& error) {
        return sqrt(arg);
    }, "sqrt"));
    customFunctions.insert("cbrt", createSingleArgFunction([=](double arg, QString& error) {
        return cbrt(arg);
    }, "cbrt"));
    customFunctions.insert("sin", createSingleArgFunction([=](double arg, QString& error) {
        return sin(arg);
    }, "sin"));
    customFunctions.insert("cos", createSingleArgFunction([=](double arg, QString& error) {
        return cos(arg);
    }, "cos"));
    customFunctions.insert("tan", createSingleArgFunction([=](double arg, QString& error) {
        return tan(arg);
    }, "tan"));
    customFunctions.insert("asin", createSingleArgFunction([=](double arg, QString& error) -> double {
        if (abs(arg) > 1) {
            error = tr("asin: input (%1) out of bounds (between -1 and 1)").arg(arg);
            qDebug() << "error at " << bufferState->yy_bs_column;
            return 0;
        } else {
            return asin(arg);
        }
    }, "asin"));
    customFunctions.insert("acos", createSingleArgFunction([=](double arg, QString& error) -> double {
        if (abs(arg) > 1) {
            error = tr("asin: input (%1) out of bounds (between -1 and 1)").arg(arg);
            return 0;
        } else {
            return asin(arg);
        }
    }, "acos"));

    customFunctions.insert("log", [=](QList<double> args, QString& error) -> double {
        if (args.length() == 1) {
            //log base 10
            return log10(args.first());
        } else if (args.length() == 2) {
            //log base %2
            return log(args.first()) / log(args.at(1));
        } else {
            error = tr("log: expected 1 or 2 arguments, got %1").arg(args.length());
            return 0;
        }
    });
}
