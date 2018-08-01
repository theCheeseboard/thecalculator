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
    ui->rightShiftButton->setPalette(operationButtonPalette);
    ui->leftShiftButton->setPalette(operationButtonPalette);
    ui->imaginaryButton->setPalette(operationButtonPalette);
    ui->percentButton->setPalette(operationButtonPalette);

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
    QString errorText = QString::fromLocal8Bit(error);
    if (errorText.startsWith("syntax error")) {
        ui->answerLabel->setText("");
    } else {
        ui->answerLabel->setText(errorText);
    }
}

void MainWindow::parserResult(idouble result) {
    currentAnswer = result;
    ui->answerLabel->setText(idbToString(result));
}

idouble MainWindow::callFunction(QString name, QList<idouble> args, QString& error) {
    //qDebug() << "Calling function:" << name << "with arguments" << args;
    if (!customFunctions.contains(name)) {
        error = tr("%1: undefined function").arg(name);
        return 0;
    } else {
        return customFunctions.value(name)(args, error);
    }
}

std::function<idouble(QList<idouble>,QString&)> MainWindow::createSingleArgFunction(std::function<idouble (idouble, QString &)> fn, QString fnName) {
    return [=](QList<idouble> args, QString& error) -> idouble {
        if (args.length() != 1) {
            error = tr("%1: expected 1 argument, got %2").arg(fnName).arg(args.length());
            return 0;
        } else {
            return fn(args.first(), error);
        }
    };
}

void MainWindow::setupBuiltinFunctions() {
    customFunctions.insert("abs", createSingleArgFunction([=](idouble arg, QString& error) {
        return abs(arg);
    }, "abs"));
    customFunctions.insert("sqrt", createSingleArgFunction([=](idouble arg, QString& error) {
        return sqrt(arg);
    }, "sqrt"));
    /*customFunctions.insert("cbrt", createSingleArgFunction([=](idouble arg, QString& error) {
        return cbrt(arg);
    }, "cbrt"));*/
    customFunctions.insert("fact", createSingleArgFunction([=](idouble arg, QString& error) -> idouble {
        if (arg.imag() != 0) {
            error = tr("fact: input (%1) not a real number").arg(idbToString(arg));
            return 0;
        } else if (arg.real() < 0) {
            error = tr("fact: input (%1) out of bounds (0 and above)").arg(idbToString(arg));
            return 0;
        } else if (arg.real() == 0) {
            return 1;
        } else {
            return arg.real() * tgamma(arg.real());
        }
    }, "abs"));
    customFunctions.insert("sin", createSingleArgFunction([=](idouble arg, QString& error) {
        Q_UNUSED(error)
        return sin(arg);
    }, "sin"));
    customFunctions.insert("cos", createSingleArgFunction([=](idouble arg, QString& error) {
        Q_UNUSED(error)
        return cos(arg);
    }, "cos"));
    customFunctions.insert("tan", createSingleArgFunction([=](idouble arg, QString& error) {
        Q_UNUSED(error)
        return tan(arg);
    }, "tan"));
    customFunctions.insert("conj", createSingleArgFunction([=](idouble arg, QString& error) {
        Q_UNUSED(error)
        return conj(arg);
    }, "conj"));
    customFunctions.insert("im", createSingleArgFunction([=](idouble arg, QString& error) {
        Q_UNUSED(error)
        return arg.imag();
    }, "im"));
    customFunctions.insert("re", createSingleArgFunction([=](idouble arg, QString& error) {
        Q_UNUSED(error)
        return arg.real();
    }, "re"));
    customFunctions.insert("asin", createSingleArgFunction([=](idouble arg, QString& error) -> idouble {
        if (abs(arg) > 1) {
            error = tr("asin: input (%1) out of bounds (between -1 and 1)").arg(idbToString(arg));
            //qDebug() << "error at " << bufferState->yy_bs_column;
            return 0;
        } else {
            return asin(arg);
        }
    }, "asin"));
    customFunctions.insert("acos", createSingleArgFunction([=](idouble arg, QString& error) -> idouble {
        if (abs(arg) > 1) {
            error = tr("acos: input (%1) out of bounds (between -1 and 1)").arg(idbToString(arg));
            return 0;
        } else {
            return asin(arg);
        }
    }, "acos"));

    customFunctions.insert("log", [=](QList<idouble> args, QString& error) -> idouble {
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
    customFunctions.insert("ln", createSingleArgFunction([=](idouble arg, QString& error) {
        Q_UNUSED(error)
        return log(arg);
    }, "ln"));

    customFunctions.insert("lsh", [=](QList<idouble> args, QString& error) -> idouble {
        if (args.length() == 2) {
            idouble first = args.first();
            idouble second = args.at(1);

            if (first.imag() != 0) {
                error = tr("lsh: arg1 (%1) not a real number").arg(idbToString(first));
                return 0;
            }
            if (second.imag() != 0) {
                error = tr("lsh: arg2 (%1) not a real number").arg(idbToString(second));
                return 0;
            }

            if (floor(first.real()) != first.real()) {
                error = tr("lsh: arg1 (%1) not an integer").arg(idbToString(first));
                return 0;
            }

            if (floor(second.real()) != second.real()) {
                error = tr("lsh: arg2 (%2) not an integer").arg(idbToString(second));
                return 0;
            }

            return (int) first.real() << (int) second.real();
        } else {
            error = tr("lsh: expected 2 arguments, got %1").arg(args.length());
            return 0;
        }
    });
    customFunctions.insert("rsh", [=](QList<idouble> args, QString& error) -> idouble {
        if (args.length() == 2) {
            idouble first = args.first();
            idouble second = args.at(1);

            if (first.imag() != 0) {
                error = tr("rsh: arg1 (%1) not a real number").arg(idbToString(first));
                return 0;
            }
            if (second.imag() != 0) {
                error = tr("rsh: arg2 (%1) not a real number").arg(idbToString(second));
                return 0;
            }

            if (floor(first.real()) != first.real()) {
                error = tr("rsh: arg1 (%1) not an integer").arg(idbToString(first));
                return 0;
            }

            if (floor(second.real()) != second.real()) {
                error = tr("rsh: arg2 (%2) not an integer").arg(idbToString(second));
                return 0;
            }

            return (int) first.real() >> (int) second.real();
        } else {
            error = tr("rsh: expected 2 arguments, got %1").arg(args.length());
            return 0;
        }
    });

    customFunctions.insert("floor", createSingleArgFunction([=](idouble arg, QString& error) {
        return floor(arg.real());
    }, "floor"));
    customFunctions.insert("ceil", createSingleArgFunction([=](idouble arg, QString& error) {
        return ceil(arg.real());
    }, "ceil"));
}

QString MainWindow::idbToString(idouble db) {
    if (db.real() != 0 && db.imag() == 0) {
        return QString::number(db.real());
    } else if (db.real() == 0 && db.imag() == 0) {
        return "0";
    } else if (db.real() != 0 && db.imag() == 1) {
        return QString::number(db.real()) + " + i";
    } else if (db.real() != 0 && db.imag() > 0) {
        return QString::number(db.real()) + " + " + QString::number(db.imag()) + "i";
    } else if (db.real() != 0 && db.imag() == -1) {
        return QString::number(db.real()) + " - i";
    } else if (db.real() != 0 && db.imag() < 0) {
        return QString::number(db.real()) + " - " + QString::number(-db.imag()) + "i";
    } else if (db.imag() == 1) {
        return "i";
    } else if (db.imag() == -1) {
        return "-i";
    } else {
        return QString::number(db.imag()) + "i";
    }
}
