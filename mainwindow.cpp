#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QScroller>
#include <QActionGroup>
#include <QMenu>
#include <QStackedWidget>
#include "aboutwindow.h"
#include <QRandomGenerator>
#include <QProcess>
#include "historydelegate.h"
#include <QDesktopServices>
#include <QJsonArray>
#include <QJsonDocument>
#include <QMessageBox>
#include <ttoast.h>
#include "evaluationengine.h"

#include "customs/overloadbox.h"

extern MainWindow* MainWin;
extern float getDPIScaling();
extern QMap<QString, std::function<idouble(QList<idouble>,QString&)>> customFunctions;
extern QMap<QString, idouble> variables;
extern bool explicitEvaluation;
extern QString idbToString(idouble db);
extern bool isDegrees;

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
    ui->stackedWidget->setCurrentAnimation(tStackedWidget::SlideHorizontal);

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
    QScroller::grabGesture(ui->historyWidget, QScroller::LeftMouseButtonGesture);

    historyDelegate = new HistoryDelegate();

    ui->scrollArea->setFixedWidth(0);
    this->setFixedWidth(this->sizeHint().width() * getDPIScaling());
    ui->answerContainer->setFixedHeight(ui->answerLabel->height());

    ui->SquareButton->setTypedOutput("²");
    ui->CubeButton->setTypedOutput("³");
    ui->ExponentButton->setTypedOutput("^");
    ui->inverseButton->setTypedOutput("⁻¹");
    for (CalcButton* b : buttons) {
        connect(b, SIGNAL(output(QString)), this, SLOT(ButtonPressed(QString)));
    }
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

    EvaluationEngine::evaluate(expression)->then([=](EvaluationEngine::Result r) {
        switch (r.type) {
            case EvaluationEngine::Result::Scalar:
                currentAnswer = r.result;
                ui->answerLabel->setText(idbToString(r.result));
                resizeAnswerLabel();
                break;
            case EvaluationEngine::Result::Error: {
                QString answerText;
                ui->answerLabel->setText(r.error);

                resizeAnswerLabel();
                break;
            }
            case EvaluationEngine::Result::Assign: {
                ui->answerLabel->setText(tr("%1 assigned to %2").arg(r.identifier, idbToString(r.value)));

                QListWidgetItem* historyItem = new QListWidgetItem();
                historyItem->setText(r.identifier + " = " + idbToString(r.value));
                historyItem->setIcon(QIcon::fromTheme("dialog-information"));
                ui->historyWidget->addItem(historyItem);
                break;
            }
            case EvaluationEngine::Result::Equality: {
                ui->answerLabel->setText(r.isTrue ? tr("TRUE") : tr("FALSE"));
            }
        }
    });

    if (resultSuccess) {
        variables.insert("Ans", currentAnswer);
        ui->answerLabel->setText("");
        ui->expressionBox->setText(idbToString(currentAnswer));

        QListWidgetItem* historyItem = new QListWidgetItem();
        historyItem->setText(expression + " = " + idbToString(currentAnswer));
        ui->historyWidget->addItem(historyItem);
        ui->historyWidget->setItemDelegateForRow(ui->historyWidget->row(historyItem), historyDelegate);
    }

    explicitEvaluation = false;
}

void MainWindow::parserError(const char *error) {
    resultSuccess = false;
}

void MainWindow::parserResult(idouble result) {
    currentAnswer = result;
    ui->answerLabel->setText(idbToString(result));
    resizeAnswerLabel();
    resultSuccess = true;
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

void MainWindow::setupFunctions() {
    //Clear all the functions
    customFunctions.clear();

    //Insert all the builtin functions
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

        if (isDegrees) {
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
    customFunctions.insert("sinh", createSingleArgFunction([=](idouble arg, QString& error) {
        Q_UNUSED(error)
        return sinh(arg);
    }, "sinh"));
    customFunctions.insert("cosh", createSingleArgFunction([=](idouble arg, QString& error) {
        Q_UNUSED(error)
        return cosh(arg);
    }, "cosh"));
    customFunctions.insert("tanh", createSingleArgFunction([=](idouble arg, QString& error) {
        Q_UNUSED(error)
        return tanh(arg);
    }, "tanh"));
    customFunctions.insert("asinh", createSingleArgFunction([=](idouble arg, QString& error) {
        Q_UNUSED(error)
        return asinh(arg);
    }, "asinh"));
    customFunctions.insert("acosh", createSingleArgFunction([=](idouble arg, QString& error) {
        Q_UNUSED(error)
        return acosh(arg);
    }, "acosh"));
    customFunctions.insert("atanh", createSingleArgFunction([=](idouble arg, QString& error) {
        Q_UNUSED(error)
        return atanh(arg);
    }, "atanh"));

    customFunctions.insert("log", [=](QList<idouble> args, QString& error) -> idouble {
        if (args.length() == 1) {
            //log base 10
            if (args.first().real() == 0 && args.first().imag() == 0) {
                error = tr("log: input (%1) out of bounds (not 0)").arg(idbToString(args.first()));
                return 0;
            }

            return log10(args.first());
        } else if (args.length() == 2) {
            //log base %2
            if (args.first().real() == 0 && args.first().imag() == 0) {
                error = tr("log: arg1 (%1) out of bounds (not 0)").arg(idbToString(args.first()));
                return 0;
            }

            if (args.last().real() == 1 && args.last().imag() == 0) {
                error = tr("log: arg2 (%1) out of bounds (not 1)").arg(idbToString(args.last()));
                return 0;
            }

            if (args.last().real() == 0 && args.last().imag() == 0) {
                error = tr("log: arg2 (%1) out of bounds (not 0)").arg(idbToString(args.last()));
                return 0;
            }

            return log(args.first()) / log(args.at(1));
        } else {
            error = tr("log: expected 1 or 2 arguments, got %1").arg(args.length());
            return 0;
        }
    });
    customFunctions.insert("ln", createSingleArgFunction([=](idouble arg, QString& error) -> idouble {
        if (arg.real() == 0 && arg.imag() == 0) {
            error = tr("ln: input (%1) out of bounds (not 0)").arg(idbToString(arg));
            return 0;
        }

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

    customFunctions.insert("random", [=](QList<idouble> args, QString& error) -> idouble {
        QRandomGenerator gen;

        if (args.length() == 0) {
            return idouble(gen.generate());
        } else if (args.length() == 1) {
            if (args.first().imag() != 0) {
                error = tr("random: arg1 (%1) not a real number").arg(idbToString(args.first()));
                return 0;
            }

            return idouble(gen.bounded((double) args.first().real()));
        } else if (args.length() == 2) {
            if (args.first().imag() != 0) {
                error = tr("random: arg1 (%1) not a real number").arg(idbToString(args.first()));
                return 0;
            }

            if (args.last().imag() != 0) {
                error = tr("random: arg2 (%1) not a real number").arg(idbToString(args.last()));
                return 0;
            }

            return idouble(gen.bounded((int) args.first().real(), (int) args.last().real()));
        } else {
            error = tr("random: expected 0, 1 or 2 arguments, got %1").arg(args.length());
            return 0;
        }
    });

    //Set up all the custom functions
    QSettings settings;
    settings.beginGroup("customFunctions");
    for (QString function : settings.allKeys()) {
        QJsonDocument doc = QJsonDocument::fromBinaryData(settings.value(function).toByteArray());
        if (doc.isObject()) {
            QJsonObject obj = doc.object();
            customFunctions.insert(function, [=](QList<idouble> args, QString& error) -> idouble {
                QStringList argCounts;

                QJsonArray overloads = obj.value("overloads").toArray();
                for (QJsonValue overload : overloads) {
                    QJsonObject o = overload.toObject();

                    QJsonArray fnArgs = o.value("args").toArray();
                    if (fnArgs.count() == args.count()) {
                        //Found the correct overload to use
                        return 601;
                    } else {
                        argCounts.append(QString::number(fnArgs.count()));
                    }
                }

                QString argSpecification = argCounts.join(", ");
                if (argCounts.count() == 1) {
                    error = tr("%1: expected %n arguments, got %2", nullptr, argCounts.first().toInt()).arg(function, QString::number(args.length()));
                } else {
                    int last = argSpecification.lastIndexOf(", ");
                    argSpecification = argSpecification.replace(last, 2, " " + tr("or", "Expected 1, 2 or 3 arguments") + " ");
                    error = tr("%1: expected %2 arguments, got %3").arg(function, argSpecification, QString::number(args.length()));
                }

                return 0;
            });
        }
    }
    settings.endGroup();
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

    ui->buttonsWidget->setFixedHeight(ui->ClearButton->sizeHint().height() * 5);
}

void MainWindow::on_actionExit_triggered()
{
    QApplication::exit();
}

idouble MainWindow::toDeg(idouble rad) {
    if (isDegrees) {
        rad *= idouble(180 / M_PI);
    }
    return rad;
}

idouble MainWindow::toRad(idouble deg) {
    if (isDegrees) {
        deg *= idouble(M_PI / 180);
    }
    return deg;
}

void MainWindow::on_backButton_clicked()
{
    ui->menuBar->setVisible(true);
    ui->stackedWidget->setCurrentIndex(0);
    ui->expressionBox->grabKeyboard();
}

void MainWindow::on_FunctionsButton_clicked()
{
    QSettings settings;
    settings.beginGroup("customFunctions");

    //Load up all the custom functions
    ui->customFunctionsList->clear();
    for (QString key : settings.allKeys()) {
        ui->customFunctionsList->addItem(key);
    }
    settings.endGroup();

    ui->expressionBox->releaseKeyboard();
    ui->menuBar->setVisible(false);
    ui->stackedWidget->setCurrentIndex(1);
}

void MainWindow::on_expressionBox_returnPressed()
{
    ui->EqualButton->click();
}

void MainWindow::on_actionAbout_triggered()
{
    AboutWindow a;
    a.exec();
}

void MainWindow::changeEvent(QEvent *event) {
    QMainWindow::changeEvent(event);
    if (event->type() == QEvent::ActivationChange) {
        if (this->isActiveWindow() && ui->stackedWidget->currentIndex() == 0) {
            ui->expressionBox->grabKeyboard();
        } else {
            ui->expressionBox->releaseKeyboard();
        }
    } else if (event->type() == QEvent::LanguageChange) {
        ui->retranslateUi(this);
    }
}

void MainWindow::assignValue(QString identifier, idouble value) {
    if (explicitEvaluation) {
        ui->answerLabel->setText(tr("%1 assigned to %2").arg(identifier, idbToString(value)));

        QListWidgetItem* historyItem = new QListWidgetItem();
        historyItem->setText(identifier + " = " + idbToString(value));
        historyItem->setIcon(QIcon::fromTheme("dialog-information"));
        ui->historyWidget->addItem(historyItem);
    } else {
        ui->answerLabel->setText(tr("Assign %1 to %2").arg(identifier, idbToString(value)));
    }
}

void MainWindow::on_actionDegrees_triggered(bool checked)
{
    if (checked) {
        isDegrees = true;
    }
}

void MainWindow::on_actionRadians_triggered(bool checked)
{
    if (checked) {
        isDegrees = false;
    }
}

void MainWindow::on_actionTheCalculatorHelp_triggered()
{
    QDesktopServices::openUrl(QUrl("https://vicr123.com/thecalculator/help"));
}

void MainWindow::on_actionFileBug_triggered()
{
    QDesktopServices::openUrl(QUrl("https://github.com/vicr123/thecalculator/issues"));
}

void MainWindow::on_actionSources_triggered()
{
    QDesktopServices::openUrl(QUrl("https://github.com/vicr123/thecalculator"));
}

void MainWindow::on_addCustomFunction_clicked()
{
    editingFunction = "";
    ui->functionName->setText("");
    QLayoutItem* i = ui->customFunctionDefinitionWidget->layout()->takeAt(0);
    while (i != nullptr) {
        i->widget()->deleteLater();
        i = ui->customFunctionDefinitionWidget->layout()->takeAt(0);
    }

    ui->stackedWidget->setCurrentIndex(2);
}

void MainWindow::on_backButton_2_clicked()
{
    QMessageBox::StandardButton b = QMessageBox::warning(this, tr("Save this function?"), tr("Do you want to save this function?"), QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel, QMessageBox::Save);
    if (b == QMessageBox::Save) {
        //Save the function
        ui->saveCustomFunctionButton->click();
    } else if (b == QMessageBox::Discard) {
        //Don't save the function
        ui->stackedWidget->setCurrentIndex(1);
    }
}

void MainWindow::on_newOverloadButton_clicked()
{
    OverloadBox* overload = new OverloadBox();
    connect(overload, &OverloadBox::remove, [=] {
        ui->customFunctionDefinitionWidget->layout()->removeWidget(overload);
        overload->deleteLater();
    });
    ui->customFunctionDefinitionWidget->layout()->addWidget(overload);
}

void MainWindow::on_expressionBox_expressionUpdated(const QString &newString)
{
    EvaluationEngine::evaluate(newString)->then([=](EvaluationEngine::Result r) {
        switch (r.type) {
            case EvaluationEngine::Result::Scalar:
                ui->answerLabel->setText(idbToString(r.result));
                break;
            case EvaluationEngine::Result::Error:
                if (r.error.startsWith("syntax error")) {
                    ui->answerLabel->setText("");
                } else {
                    ui->answerLabel->setText(r.error);
                }
                break;
            case EvaluationEngine::Result::Assign:
                ui->answerLabel->setText(tr("Assign %1 to %2").arg(r.identifier, idbToString(r.value)));
                break;
            case EvaluationEngine::Result::Equality:
                ui->answerLabel->setText(r.isTrue ? tr("TRUE") : tr("FALSE"));
        }
    });
}

void MainWindow::on_saveCustomFunctionButton_clicked()
{
    QJsonObject obj;

    if (ui->functionName->text() == "") {
        //Name is required
        tToast* toast = new tToast();
        toast->setTitle(tr("Function Name Required"));
        toast->setText(tr("A function name needs to be set"));
        toast->show(this);
        connect(toast, &tToast::dismissed, toast, &tToast::deleteLater);
        return;
    }
    obj.insert("name", ui->functionName->text());

    QJsonArray overloads;
    QList<int> argNumbers;
    for (int i = 0; i < ui->customFunctionDefinitionWidget->layout()->count(); i++) {
        QObject* o = ui->customFunctionDefinitionWidget->layout()->itemAt(i)->widget();
        OverloadBox* box = (OverloadBox*) o;

        if (!box->check()) return; //Error occurred during check, don't save

        QJsonObject save = box->save();
        int argCount = save.value("args").toArray().count();
        if (argNumbers.contains(argCount)) {
            //More than one overload with same number of arguments
            tToast* toast = new tToast();
            toast->setTitle(tr("Overload Arguments"));
            toast->setText(tr("Only one overload can have %n arguments", nullptr, argCount));
            toast->show(this);
            connect(toast, &tToast::dismissed, toast, &tToast::deleteLater);
            return;
        }
        argNumbers.append(argCount);
        overloads.append(save);
    }
    obj.insert("overloads", overloads);

    QJsonDocument doc(obj);
    QSettings settings;
    settings.beginGroup("customFunctions");

    //Save the function
    if (editingFunction != "" && settings.contains(ui->functionName->text())) {
        settings.remove(ui->functionName->text());
    }
    settings.setValue(ui->functionName->text(), doc.toBinaryData());

    //Load up all the custom functions
    ui->customFunctionsList->clear();
    for (QString key : settings.allKeys()) {
        ui->customFunctionsList->addItem(key);
    }

    settings.endGroup();
    settings.sync();

    setupFunctions();

    ui->stackedWidget->setCurrentIndex(1);
}

void MainWindow::on_customFunctionsList_itemActivated(QListWidgetItem *item)
{
    editingFunction = item->text();

    QSettings settings;
    settings.beginGroup("customFunctions");
    QJsonDocument doc = QJsonDocument::fromBinaryData(settings.value(item->text()).toByteArray());
    settings.endGroup();

    QJsonObject obj = doc.object();
    ui->functionName->setText(obj.value("name").toString());

    //Clear overloads
    QLayoutItem* i = ui->customFunctionDefinitionWidget->layout()->takeAt(0);
    while (i != nullptr) {
        i->widget()->deleteLater();
        i = ui->customFunctionDefinitionWidget->layout()->takeAt(0);
    }

    QJsonArray overloads = obj.value("overloads").toArray();
    for (QJsonValue v : overloads) {
        OverloadBox* overload = new OverloadBox();
        overload->load(v.toObject());
        connect(overload, &OverloadBox::remove, [=] {
            ui->customFunctionDefinitionWidget->layout()->removeWidget(overload);
            overload->deleteLater();
        });
        ui->customFunctionDefinitionWidget->layout()->addWidget(overload);
    }

    ui->stackedWidget->setCurrentIndex(2);
}
