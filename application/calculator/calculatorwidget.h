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
#ifndef CALCULATORWIDGET_H
#define CALCULATORWIDGET_H

#include <QWidget>
#include "historydelegate.h"
#include "evaluationengineheaders.h"
#include <QCoroTask>

class CalcButton;

namespace Ui {
    class CalculatorWidget;
}

class CalculatorWidget : public QWidget {
        Q_OBJECT

    public:
        explicit CalculatorWidget(QWidget* parent = nullptr);
        ~CalculatorWidget();

        static QList<CalcButton*> buttons;
        QSize sizeHint() const;

    public slots:
        void grabExpKeyboard(bool grab);

    private slots:
        void on_expandButton_clicked();

        void ButtonPressed(QString text);

        void on_ClearButton_clicked();

        void on_BackspaceButton_clicked();

        QCoro::Task<> on_EqualButton_clicked();

        QCoro::Task<> on_expressionBox_expressionUpdated(const QString &expression);

        void on_expressionBox_cursorPositionChanged(int arg1, int arg2);

        void on_nextOverload_clicked();

        void on_previousOverload_clicked();

        void on_expressionBox_returnPressed();

        void on_NthRootButton_clicked();

        void on_LogBaseButton_clicked();

    private:
        Ui::CalculatorWidget* ui;

        void resizeAnswerLabel();
        void flashError();

        void resizeEvent(QResizeEvent* event);

        HistoryDelegate* historyDelegate;
        bool historyAtBottom = true;
        bool extended = false;

        int currentOverload = 0;
        int numOverloads;
        QString currentFunctionHelp;
        int forceWidth = -1;
};

#endif // CALCULATORWIDGET_H
