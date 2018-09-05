#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QScroller>
#include <QActionGroup>
#include <QMenu>
#include <QStackedWidget>
extern MainWindow* MainWin;
extern float getDPIScaling();

#include "calc.bison.hpp"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    MainWin = this;
    ui->setupUi(this);

    QActionGroup* group = new QActionGroup(this);
    group->addAction(ui->actionDegrees);
    group->addAction(ui->actionRadians);

    ui->expressionBox->grabKeyboard();
    ui->expressionBox->installEventFilter(this);

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
    ui->PlusButton->setShiftedOutput("⁺");
    ui->MinusButton->setShiftedOutput("⁻");
    ui->imaginaryButton->setShiftedOutput("ⁱ");

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

    QScroller::grabGesture(ui->scrollArea, QScroller::LeftMouseButtonGesture);

    ui->scrollArea->setFixedWidth(0);
    this->setFixedWidth(this->sizeHint().width() * getDPIScaling());
    ui->answerContainer->setFixedHeight(ui->answerLabel->height());

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
        ui->scrollArea->setFixedWidth(value.toInt() * getDPIScaling());
        this->setFixedWidth(this->sizeHint().width() * getDPIScaling() - ui->scrollArea->width() * getDPIScaling() + ui->scrollArea->width());
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
    explicitEvaluation = true;
    QString expression = ui->expressionBox->text();
    bufferState = yy_scan_string(expression.append("\n").toUtf8().constData());
    yyparse();
    yy_delete_buffer(bufferState);

    if (resultSuccess) {
        variables.insert("Ans", currentAnswer);
        ui->answerLabel->setText("");
        ui->expressionBox->setText(idbToString(currentAnswer));
    }

    explicitEvaluation = false;
}

void MainWindow::on_expressionBox_textEdited(const QString &arg1)
{
    int anchorPosition = ui->expressionBox->cursorPosition();

    QString newString = arg1;
    if (newString.contains("/")) newString.replace("/", "÷");
    if (newString.contains("*")) newString.replace("*", "×");
    if (newString.contains(" ")) newString.replace(" ", "⋅");
    ui->expressionBox->setText(newString);
    ui->expressionBox->setCursorPosition(anchorPosition);

    bufferState = yy_scan_string(QString(newString + "\n").toUtf8().constData());
    yyparse();
    yy_delete_buffer(bufferState);
}

void MainWindow::parserError(const char *error) {
    QString errorText = QString::fromLocal8Bit(error);
    QString answerText;
    if (errorText.startsWith("syntax error") && !explicitEvaluation) {
        ui->answerLabel->setText("");
    } else {
        ui->answerLabel->setText(errorText);
    }

    resizeAnswerLabel();
    resultSuccess = false;
}

void MainWindow::parserResult(idouble result) {
    currentAnswer = result;
    ui->answerLabel->setText(idbToString(result));
    resizeAnswerLabel();
    resultSuccess = true;
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
    customFunctions.insert("cbrt", createSingleArgFunction([=](idouble arg, QString& error) {
        return pow(arg, 1 / (float) 3);
    }, "cbrt"));
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
        return sin(toRad(arg));
    }, "sin"));
    customFunctions.insert("cos", createSingleArgFunction([=](idouble arg, QString& error) {
        Q_UNUSED(error)
        return cos(toRad(arg));
    }, "cos"));
    customFunctions.insert("tan", createSingleArgFunction([=](idouble arg, QString& error) -> idouble {
        Q_UNUSED(error)

        if (ui->actionDegrees->isChecked()) {
            if (fmod(arg.real() - 90, 180) == 0) {
                error = tr("tan: input (%1) out of bounds (not 90° + 180n)").arg(idbToString(arg));
                return 0;
            }
        } else {
            if (fmod(arg.real() - (M_PI / 2), M_PI) == 0) {
                error = tr("tan: input (%1) out of bounds (not π/2 + πn)").arg(idbToString(arg));
                return 0;
            }
        }

        return tan(toRad(arg));
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
            return 0;
        } else {
            return toDeg(asin(arg));
        }
    }, "asin"));
    customFunctions.insert("acos", createSingleArgFunction([=](idouble arg, QString& error) -> idouble {
        if (abs(arg) > 1) {
            error = tr("acos: input (%1) out of bounds (between -1 and 1)").arg(idbToString(arg));
            return 0;
        } else {
            return toDeg(acos(arg));
        }
    }, "acos"));
    customFunctions.insert("atan", createSingleArgFunction([=](idouble arg, QString& error) -> idouble {
        Q_UNUSED(error)
        return toDeg(atan(arg));
    }, "acos"));
    customFunctions.insert("sec", createSingleArgFunction([=](idouble arg, QString& error) {
        Q_UNUSED(error)
        return idouble(1) / cos(toRad(arg));
    }, "sec"));
    customFunctions.insert("csc", createSingleArgFunction([=](idouble arg, QString& error) {
        Q_UNUSED(error)
        return idouble(1) / sin(toRad(arg));
    }, "csc"));
    customFunctions.insert("cot", createSingleArgFunction([=](idouble arg, QString& error) {
        Q_UNUSED(error)
        return idouble(1) / tan(toRad(arg));
    }, "cot"));
    customFunctions.insert("asec", createSingleArgFunction([=](idouble arg, QString& error) {
        Q_UNUSED(error)
        return toDeg(acos(idouble(1) / arg));
    }, "asec"));
    customFunctions.insert("acsc", createSingleArgFunction([=](idouble arg, QString& error) {
        Q_UNUSED(error)
        return toDeg(asin(idouble(1) / arg));
    }, "acsc"));
    customFunctions.insert("acot", createSingleArgFunction([=](idouble arg, QString& error) {
        Q_UNUSED(error)
        return toDeg(atan(idouble(1) / arg));
    }, "acot"));

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
    customFunctions.insert("pow", [=](QList<idouble> args, QString& error) -> idouble {
        if (args.length() == 2) {
            idouble first = args.first();
            idouble second = args.at(1);

            if (first.real() == 0 && first.imag() == 0 &&
                    second.real() <= 0 && second.imag() == 0) {
                error = tr("pow: arg2 (%1) out of bounds for arg1 (0) (should be positive)").arg(idbToString(second));
                return 0;
            }

            if (first.real() == 0 && first.imag() == 0 && second.real() == 0 && second.imag() != 0) {
                error = tr("pow: arg2 (%1) out of bounds for arg1 (0) (should be a real number)").arg(idbToString(second));
                return 0;
            }

            if (first.real() == 0 && first.imag() == 0 &&
                    second.real() > 0) {
                error = tr("pow: arg2 (%1) out of bounds for arg1 (0) (should be positive)").arg(idbToString(second));
            }

            return pow(first, second);
        } else {
            error = tr("pow: expected 2 arguments, got %1").arg(args.length());
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
    long double real = db.real();
    long double imag = db.imag();
    if (real != 0 && imag == 0) {
        return numberFormatToString(real);
    } else if (real == 0 && imag == 0) {
        return "0";
    } else if (real != 0 && imag == 1) {
        return numberFormatToString(real) + " + i";
    } else if (real != 0 && imag > 0) {
        return numberFormatToString(real) + " + " + numberFormatToString(imag) + "i";
    } else if (real != 0 && imag == -1) {
        return numberFormatToString(imag) + " - i";
    } else if (real != 0 && imag < 0) {
        return numberFormatToString(real) + " - " + numberFormatToString(-imag) + "i";
    } else if (imag == 1) {
        return "i";
    } else if (imag == -1) {
        return "-i";
    } else {
        return numberFormatToString(imag) + "i";
    }
}

QString MainWindow::numberFormatToString(long double number) {
    std::stringstream stream;
    stream << std::setprecision(10) << number;

    return QString::fromStdString(stream.str());
}

void MainWindow::assignValue(QString identifier, idouble value) {
    if (explicitEvaluation) {
        variables.insert(identifier, value);
        ui->answerLabel->setText(tr("%1 assigned to %2").arg(identifier, idbToString(value)));
    } else {
        ui->answerLabel->setText(tr("Assign %1 to %2").arg(identifier, idbToString(value)));
    }
}

bool MainWindow::valueExists(QString identifier) {
    return variables.contains(identifier);
}

idouble MainWindow::getValue(QString identifier) {
    return variables.value(identifier);
}

void MainWindow::resizeAnswerLabel() {
    QFont font = this->font();
    font.setPointSize(15);
    QFontMetricsF metrics(font);
    qreal width = metrics.width(ui->answerLabel->text());
    if (width > ui->answerContainer->width()) {
        font.setPointSizeF(15.0 * (float) ui->answerContainer->width() / width);
    }
    ui->answerLabel->setFont(font);
}

void MainWindow::resizeEvent(QResizeEvent* event) {
    resizeAnswerLabel();
}

void MainWindow::on_actionExit_triggered()
{
    QApplication::exit();
}

idouble MainWindow::toDeg(idouble rad) {
    if (ui->actionDegrees->isChecked()) {
        rad *= idouble(180 / M_PI);
    }
    return rad;
}

idouble MainWindow::toRad(idouble deg) {
    if (ui->actionDegrees->isChecked()) {
        deg *= idouble(M_PI / 180);
    }
    return deg;
}

void MainWindow::on_backButton_clicked()
{
    ui->menuBar->setVisible(true);
    ui->stackedWidget->setCurrentIndex(0);
}

void MainWindow::on_FunctionsButton_clicked()
{
    ui->menuBar->setVisible(false);
    ui->stackedWidget->setCurrentIndex(1);
}

void MainWindow::on_expressionBox_returnPressed()
{
    ui->EqualButton->click();
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event) {
    if (watched == ui->expressionBox) {
        if (event->type() == QEvent::KeyPress) {
            QKeyEvent* e = (QKeyEvent*) event;

            if (e->modifiers() & Qt::ControlModifier) {
                switch (e->key()) {
                    case Qt::Key_P: ui->expressionBox->insert("π"); break;
                    case Qt::Key_0: ui->expressionBox->insert("⁰"); break;
                    case Qt::Key_1: ui->expressionBox->insert("¹"); break;
                    case Qt::Key_2: ui->expressionBox->insert("²"); break;
                    case Qt::Key_3: ui->expressionBox->insert("³"); break;
                    case Qt::Key_4: ui->expressionBox->insert("⁴"); break;
                    case Qt::Key_5: ui->expressionBox->insert("⁵"); break;
                    case Qt::Key_6: ui->expressionBox->insert("⁶"); break;
                    case Qt::Key_7: ui->expressionBox->insert("⁷"); break;
                    case Qt::Key_8: ui->expressionBox->insert("⁸"); break;
                    case Qt::Key_9: ui->expressionBox->insert("⁹"); break;
                    case Qt::Key_I: ui->expressionBox->insert("ⁱ"); break;
                    case Qt::Key_Equal: ui->expressionBox->insert("⁺"); break;
                    case Qt::Key_Minus: ui->expressionBox->insert("⁻"); break;
                    case Qt::Key_R: ui->expressionBox->insert("√"); break;
                }
            }
        }
    }
    return false;
}
