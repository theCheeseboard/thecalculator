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

#include "calculatorwidget.h"
#include "ui_calculatorwidget.h"

#include <QScroller>
#include <QScrollBar>
#include "historydelegate.h"
#include "calcbutton.h"
#include "mainwindow.h"
#include <tvariantanimation.h>
#include "evaluationengine.h"

QList<CalcButton*> CalculatorWidget::buttons = QList<CalcButton*>();
extern QString idbToString(idouble db);

extern MainWindow* MainWin;

CalculatorWidget::CalculatorWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CalculatorWidget)
{
    ui->setupUi(this);

    ui->expressionBox->grabKeyboard();
    ui->helpWidget->setFixedHeight(0);

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

    ui->SquareButton->setTypedOutput("²");
    ui->CubeButton->setTypedOutput("³");
    ui->ExponentButton->setTypedOutput("^");
    ui->inverseButton->setTypedOutput("⁻¹");
    ui->imaginaryButton->setTypedOutput("i");
    ui->eButton->setTypedOutput("e");
    ui->PointButton->setText(QLocale().decimalPoint());
    for (CalcButton* b : buttons) {
        connect(b, SIGNAL(output(QString)), this, SLOT(ButtonPressed(QString)));
    }
}

CalculatorWidget::~CalculatorWidget()
{
    delete ui;
}

void CalculatorWidget::on_expandButton_clicked()
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
        ui->scrollArea->setFixedWidth(value.toInt() * theLibsGlobal::getDPIScaling());
        forceWidth = QWidget::sizeHint().width() * theLibsGlobal::getDPIScaling() - ui->scrollArea->width() * theLibsGlobal::getDPIScaling() + ui->scrollArea->width();
        emit sizeHintChanged();
    });
    connect(anim, SIGNAL(finished()), anim, SLOT(deleteLater()));
    anim->start();
}


void CalculatorWidget::ButtonPressed(QString text) {
    ui->expressionBox->insert(text);
}

void CalculatorWidget::on_ClearButton_clicked()
{
    ui->expressionBox->clear();
}

void CalculatorWidget::on_BackspaceButton_clicked()
{
    ui->expressionBox->backspace();
}

void CalculatorWidget::on_EqualButton_clicked()
{
    QString expression = ui->expressionBox->getFixedExpression();

    EvaluationEngine::evaluate(expression, MainWin->variables)->then([=](EvaluationEngine::Result r) {
        switch (r.type) {
            case EvaluationEngine::Result::Scalar: {
                ui->expressionBox->setExpression(idbToString(r.result));

                QListWidgetItem* historyItem = new QListWidgetItem();
                historyItem->setText(expression + " = " + idbToString(r.result));
                historyItem->setIcon(QIcon::fromTheme("dialog-information"));
                ui->historyWidget->addItem(historyItem);

                MainWin->variables.insert("Ans", r.result);
                break;
            }
            case EvaluationEngine::Result::Error: {
                QString answerText;
                ui->answerLabel->setText(r.error);
                ui->expressionBox->setErrorRange(r.location, r.length);

                resizeAnswerLabel();
                flashError();
                break;
            }
            case EvaluationEngine::Result::Assign: {
                MainWin->variables.insert(r.identifier, r.value);
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
}

void CalculatorWidget::resizeAnswerLabel() {
    QFont font = this->font();
    font.setPointSize(15);
    QFontMetricsF metrics(font);
    qreal width = metrics.width(ui->answerLabel->text());
    if (width > ui->answerContainer->width()) {
        font.setPointSizeF(15.0 * (float) ui->answerContainer->width() / width);
    }
    ui->answerLabel->setFont(font);
}

void CalculatorWidget::resizeEvent(QResizeEvent* event) {
    resizeAnswerLabel();

    ui->buttonsWidget->setFixedHeight(ui->ClearButton->sizeHint().height() * 5);
}


void CalculatorWidget::on_expressionBox_expressionUpdated(const QString &newString)
{
    EvaluationEngine::evaluate(newString, MainWin->variables)->then([=](EvaluationEngine::Result r) {
        switch (r.type) {
            case EvaluationEngine::Result::Scalar:
                ui->answerLabel->setText(idbToString(r.result));
                break;
            case EvaluationEngine::Result::Error:
                if (r.error.startsWith("syntax error")) {
                    ui->answerLabel->setText("");
                    ui->expressionBox->setErrorRange(0, 0);
                } else {
                    ui->answerLabel->setText(r.error);
                    ui->expressionBox->setErrorRange(r.location, r.length);
                }

                resizeAnswerLabel();
                break;
            case EvaluationEngine::Result::Assign:
                ui->answerLabel->setText(tr("Assign %1 to %2").arg(r.identifier, idbToString(r.value)));
                break;
            case EvaluationEngine::Result::Equality:
                ui->answerLabel->setText(r.isTrue ? tr("TRUE") : tr("FALSE"));
        }
    });
}


void CalculatorWidget::on_expressionBox_cursorPositionChanged(int arg1, int arg2)
{
    QString relevantText = ui->expressionBox->getFixedExpression().left(arg2);
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

    QChar argSep = ',';
    if (QLocale().decimalPoint() == ',') argSep = '.';

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
                    } else if (c == argSep) {
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

                ui->funHelpName->setText(currentFunction + "(" + functionArgList.join(argSep) + ") : " + tr("function"));
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

void CalculatorWidget::on_nextOverload_clicked()
{
    if (currentOverload + 1 != numOverloads) {
        currentOverload++;
        on_expressionBox_cursorPositionChanged(ui->expressionBox->cursorPosition(), ui->expressionBox->cursorPosition());
    }
}

void CalculatorWidget::on_previousOverload_clicked()
{
    if (currentOverload != 0) {
        currentOverload--;
        on_expressionBox_cursorPositionChanged(ui->expressionBox->cursorPosition(), ui->expressionBox->cursorPosition());
    }
}

void CalculatorWidget::on_expressionBox_returnPressed()
{
    ui->EqualButton->click();
}

void CalculatorWidget::grabExpKeyboard(bool grab) {
    if (grab) {
        ui->expressionBox->grabKeyboard();
    } else {
        ui->expressionBox->releaseKeyboard();
    }
}

QSize CalculatorWidget::sizeHint() const {
    QSize initialSizeHint = QWidget::sizeHint();
    if (forceWidth != -1) {
        initialSizeHint.setWidth(forceWidth);
    }
    return initialSizeHint;
}

void CalculatorWidget::flashError() {
    tVariantAnimation* a = new tVariantAnimation();
    a->setStartValue(QColor(200, 0, 0));
    a->setEndValue(this->palette().color(QPalette::Window));
    a->setDuration(1000);
    a->setEasingCurve(QEasingCurve::Linear);
    connect(a, &tVariantAnimation::finished, a, &tVariantAnimation::deleteLater);
    connect(a, &tVariantAnimation::valueChanged, [=](QVariant value) {
        QPalette pal = this->palette();
        pal.setColor(QPalette::Window, value.value<QColor>());
        ui->expressionBox->setPalette(pal);
        ui->answerContainer->setPalette(pal);
    });
    a->start();
}
