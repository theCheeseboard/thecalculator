/****************************************
 *
 *   theCalculator - Calculator
 *   Copyright (C) 2019 Victor Tran
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * *************************************/

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
#include <QTimer>
#include <QScrollBar>
#include <ttoast.h>
#include "evaluationengine.h"

#include "customs/overloadbox.h"

extern MainWindow* MainWin;
extern float getDPIScaling();
extern QMap<QString, std::function<idouble(QList<idouble>,QString&)>> customFunctions;
extern QString idbToString(idouble db);

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
    ui->helpWidget->setFixedHeight(0);
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
    ui->historyWidget->setItemDelegate(historyDelegate);
    connect(ui->historyWidget->verticalScrollBar(), &QScrollBar::rangeChanged, [=](int min, int max) {
        if (historyAtBottom) ui->historyWidget->verticalScrollBar()->setValue(max);
    });
    connect(ui->historyWidget->verticalScrollBar(), &QScrollBar::valueChanged, [=](int value) {
        if (value == ui->historyWidget->verticalScrollBar()->maximum()) {
            historyAtBottom = true;
        } else {
            historyAtBottom = false;
        }
    });

    ui->scrollArea->setFixedWidth(0);
    ui->answerContainer->setFixedHeight(ui->answerLabel->height());

    QTimer::singleShot(0, [=] {
        this->setFixedWidth(this->sizeHint().width() * getDPIScaling());
    });

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
    QString expression = ui->expressionBox->text();

    EvaluationEngine::evaluate(expression, variables)->then([=](EvaluationEngine::Result r) {
        switch (r.type) {
            case EvaluationEngine::Result::Scalar: {
                currentAnswer = r.result;
                ui->expressionBox->setText(idbToString(r.result));

                QListWidgetItem* historyItem = new QListWidgetItem();
                historyItem->setText(expression + " = " + idbToString(r.result));
                historyItem->setIcon(QIcon::fromTheme("dialog-information"));
                ui->historyWidget->addItem(historyItem);
                break;
            }
            case EvaluationEngine::Result::Error: {
                QString answerText;
                ui->answerLabel->setText(r.error);

                resizeAnswerLabel();
                break;
            }
            case EvaluationEngine::Result::Assign: {
                variables.insert(r.identifier, r.value);
                ui->answerLabel->setText(tr("%1 assigned to %2").arg(r.identifier, idbToString(r.value)));

                QListWidgetItem* historyItem = new QListWidgetItem();
                historyItem->setText(r.identifier + " = " + idbToString(r.value));
                historyItem->setIcon(QIcon::fromTheme("dialog-information"));
                ui->historyWidget->addItem(historyItem);
                break;
            }
            case EvaluationEngine::Result::Equality: {
                ui->answerLabel->setText(r.isTrue ? tr("TRUE") : tr("FALSE"));

                QListWidgetItem* historyItem = new QListWidgetItem();
                historyItem->setText(expression + " = " + ui->answerLabel->text());
                historyItem->setIcon(QIcon::fromTheme("dialog-information"));
                ui->historyWidget->addItem(historyItem);
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

void MainWindow::on_actionDegrees_triggered(bool checked)
{
    if (checked) {
        EvaluationEngine::setTrigonometricUnit(EvaluationEngine::Degrees);
    }
}

void MainWindow::on_actionRadians_triggered(bool checked)
{
    if (checked) {
        EvaluationEngine::setTrigonometricUnit(EvaluationEngine::Radians);
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
    EvaluationEngine::evaluate(newString, variables)->then([=](EvaluationEngine::Result r) {
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

    EvaluationEngine::setupFunctions();

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

void MainWindow::on_expressionBox_cursorPositionChanged(int arg1, int arg2)
{
    QString relevantText = ui->expressionBox->text().left(arg2);
    //Find the previous function
    QRegularExpression regex("\\w+?(?=[⁰¹²³⁴⁵⁶⁷⁸⁹⁺⁻ⁱ]*\\()");
    QRegularExpressionMatchIterator matchIterator = regex.globalMatch(relevantText);

    //Select the appropriate match
    QStack<QString> matchSelector;
    QStack<int> matchPositions;
    QChar lastChar = ' ';
    for (int i = 0; i < relevantText.count(); i++) {
        QChar c = relevantText.at(i);
        if (c == '(') {
            if (matchIterator.hasNext() && QRegularExpression("\\w").match(lastChar).hasMatch()) {
                QRegularExpressionMatch m = matchIterator.next();
                matchSelector.push(m.captured());
                matchPositions.push(m.capturedEnd());
            } else {
                //Push an empty string so it will be popped when it finds )
                matchSelector.push("");
                matchPositions.push(i);
            }
        } else if (c == ')') {
            if (!matchSelector.isEmpty()) {
                matchSelector.pop();
                matchPositions.pop();
                //Otherwise we'll continue and try to get the function anyway
            }
        }

        //Ignore exponents
        if (!QRegularExpression("[⁰¹²³⁴⁵⁶⁷⁸⁹⁺⁻ⁱ]").match(c).hasMatch()) {
            lastChar = c;
        }
    }

    tVariantAnimation* anim = new tVariantAnimation();
    anim->setStartValue(ui->helpWidget->height());
    anim->setEndValue(0);
    if (!matchSelector.isEmpty()) {
        //We're currently in a function definition
        QString currentFunction = matchSelector.pop();
        int currentPosition = matchPositions.pop();
        while (currentFunction == "" && !matchSelector.isEmpty()) {
            currentFunction = matchSelector.pop();
            currentPosition = matchPositions.pop();
        }

        if (EvaluationEngine::customFunctions.contains(currentFunction)) {
            //Figure out the current argument
            int currentArgument = 0;

            if (relevantText.count() - currentPosition == 1) {
                currentArgument = -1;
            } else {
                int bracketCount = -1;
                for (int i = currentPosition; i < relevantText.count(); i++) {
                    QChar c = relevantText.at(i);
                    if (c == '(') {
                        bracketCount++;
                    } else if (c == ')') {
                        bracketCount--;
                        if (bracketCount < 0) break; //Too many closing brackets
                    } else if (c == ',') {
                        if (bracketCount == 0) currentArgument++;
                    }
                }
            }

            CustomFunction fn = EvaluationEngine::customFunctions.value(currentFunction);
            if (currentFunctionHelp != currentFunction) {
                currentOverload = 0;
                currentFunctionHelp = currentFunction;
                numOverloads = fn.overloads();
            }

            int usedOverload = currentOverload;
            if (fn.getArgs(usedOverload).count() <= currentArgument && usedOverload + 1 < numOverloads) {
                usedOverload++;
            }
            ui->funHelpDesc->setText(fn.getDescription(usedOverload));

            QStringList args = fn.getArgs(usedOverload);
            if (args.count() == 0) {
                ui->funHelpArgs->setText(tr("No arguments"));
                ui->funHelpName->setText(currentFunction + "() : " + tr("function"));
            } else {
                QStringList functionArgList;
                QStringList argList;
                for (int i = 0; i < args.count(); i++) {
                    QString arg = args.at(i);
                    QString argName = arg.left(arg.indexOf(":"));
                    QString argDesc = arg.mid(arg.indexOf(":") + 1);

                    if (currentArgument == i || (currentArgument == -1 && i == 0)) {
                        argList.append("<b>" + argName + ": " + argDesc + "</b>");
                    } else {
                        argList.append(argName);
                    }
                    functionArgList.append(argName);
                }

                ui->funHelpName->setText(currentFunction + "(" + functionArgList.join(",") + ") : " + tr("function"));
                ui->funHelpArgs->setText(argList.join(" · "));
            }

            ui->funHelpOverloads->setText(QString::number(usedOverload + 1) + "/" + QString::number(numOverloads));
            currentOverload = usedOverload;

            anim->setEndValue(ui->helpWidget->sizeHint().height());
        }
    }
    anim->setDuration(250);
    anim->setEasingCurve(QEasingCurve::OutCubic);
    connect(anim, &tVariantAnimation::valueChanged, [=](QVariant value) {
       ui->helpWidget->setFixedHeight(value.toInt());
    });
    connect(anim, &tVariantAnimation::finished, anim, &tVariantAnimation::deleteLater);
    anim->start();

    if (anim->endValue() == 0) {
        currentFunctionHelp = "";
    }
}

void MainWindow::on_nextOverload_clicked()
{
    if (currentOverload + 1 != numOverloads) {
        currentOverload++;
        on_expressionBox_cursorPositionChanged(ui->expressionBox->cursorPosition(), ui->expressionBox->cursorPosition());
    }
}

void MainWindow::on_previousOverload_clicked()
{
    if (currentOverload != 0) {
        currentOverload--;
        on_expressionBox_cursorPositionChanged(ui->expressionBox->cursorPosition(), ui->expressionBox->cursorPosition());
    }
}
