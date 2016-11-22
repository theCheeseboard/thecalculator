#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->lineEdit->installEventFilter(this);
    this->installEventFilter(this);
    ui->instructionLabel->setVisible(false);
    ui->doneButton->setVisible(false);
    ui->functionsFrame->setParent(this);
    ui->functionsFrame->move(0, this->height());

    ui->functionsTable->setHorizontalHeaderLabels(QStringList() << "Function" << "Equation");
    ui->functionsTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);

    this->setFixedHeight(this->sizeHint().height());
    this->setFixedWidth(700);

    //Read in saved functions
    settings.beginGroup("functions");
    for (QString key : settings.allKeys()) {
        ui->functionsTable->insertRow(ui->functionsTable->rowCount());
        QTableWidgetItem* item2 = new QTableWidgetItem;
        item2->setText(settings.value(key).toString());
        ui->functionsTable->setItem(ui->functionsTable->rowCount() - 1, 1, item2);
        QTableWidgetItem* item1 = new QTableWidgetItem;
        item1->setText(key.remove("functions/"));
        ui->functionsTable->setItem(ui->functionsTable->rowCount() - 1, 0, item1);
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_lineEdit_returnPressed()
{
    ui->evaluateButton->click();
}

void MainWindow::on_evaluateButton_clicked()
{
    bool errorOccurred;
    QString error;
    //Store the current expression
    ui->previousExpression->setText(ui->lineEdit->text() + " =");
    qDebug() << "Calculating expression: " + ui->lineEdit->text();
    qDebug() << "";

    //Evaluate the expression
    QString expression = ui->lineEdit->text();
    QString evaluated = evaluate(expression, errorOccurred);

    if (errorOccurred) {
        qDebug() << "[ERROR] " + evaluated;
        ui->lineEdit->setText(evaluated + " [ERROR!]");
    } else {
        qDebug() << "Answer: " + evaluated;
        ui->lineEdit->setText(evaluated);
        previousAnswer = evaluated;
    }

}

QString MainWindow::evaluate(QString expression, bool &error) {
    qDebug() << "Calculating expression step: " + expression;

    //Evaluate user defined functions
    bool stageComplete = false;
    while (!stageComplete) {
        stageComplete = true;
        for (int i = 0; i < ui->functionsTable->rowCount(); i++) {
            QString name = ui->functionsTable->item(i, 0)->text();
            if (expression.contains(name.left(name.indexOf("(")))) {
                int index = expression.indexOf(name.left(name.indexOf("(")));
                int lengthOfSub;
                stageComplete = false;

                //Evaluate brackets
                QString functionArg;
                uint currentBracketCount = 0;
                uint currentBracketStart;

                int length = expression.length();
                for (int i = index; i < length; i++) {
                    if (expression.at(i) == '(') {
                        if (currentBracketCount == 0) {
                            currentBracketStart = i;
                        }
                        currentBracketCount++;
                    } else if (expression.at(i) == ')') {
                        if (currentBracketCount == 0) {
                            error = true;
                            return "Mismatched Brackets";
                        } else if (currentBracketCount == 1) {
                            currentBracketCount--;
                            lengthOfSub = i - index + 1;
                            functionArg = evaluate(expression.mid(currentBracketStart + 1, i - currentBracketStart - 1), error);
                            if (error) {
                                error = true;
                                return functionArg;
                            } else {
                                //Terminate the loop early
                                i = length;
                            }
                        } else {
                            currentBracketCount--;
                        }
                    }
                }

                if (currentBracketCount != 0) {
                    error = true;
                    return "Mismatched Brackets";
                }

                if (error) {
                    error = true;
                    return functionArg;
                } else {
                    QString functionExp = ui->functionsTable->item(i, 1)->text();
                    functionExp.replace("x", "(" + functionArg + ")");

                    //Substitute function
                    expression.replace(index, lengthOfSub, "(" + functionExp + ")");
                }
            }
        }
    }

    //Replace constants with proper numbers.
    //Do this now so that we can put operations next to them.
    expression.replace("π", "(" + QString::number(M_PI, 'g', 20) + ")");
    expression.replace("e", "(" + QString::number(M_E, 'g', 20) + ")");
    expression.replace("Ans", "(" + previousAnswer + ")");
    expression.replace("x", "(" + ui->xInput->text() + ")");
    expression.replace("y", "(" + ui->yInput->text() + ")");

    //Check that trigonometric functions and logarithmic functions have operations. Insert multiplication signs if neccessary.
    bool noReplace = false;
    while (!noReplace) {
        noReplace = true;
        for (int i = 0; i < expression.length(); i++) {
            if (i < expression.count() - 4) {
                if ((expression.mid(i, 4) == "asin" ||
                     expression.mid(i, 4) == "acos" ||
                     expression.mid(i, 4) == "atan")) {
                    if (i != 0 && expression.at(i - 1) != ' ') {
                        noReplace = false;
                        expression.insert(i, " × ");
                        i = expression.length();
                    }
                }
            }

            if (i < expression.count() - 3) {
                if ((expression.mid(i, 3) == "sin" ||
                     expression.mid(i, 3) == "cos" ||
                     expression.mid(i, 3) == "tan" ||
                     expression.mid(i, 3) == "log")) {
                    if (i != 0 && expression.at(i - 1) != ' ' && expression.at(i - 1) != 'a') {
                        noReplace = false;
                        expression.insert(i, " × ");
                        i = expression.length();
                    }
                }
            }

            if (i < expression.count() - 2) {
                if (expression.mid(i, 2) == "ln") {
                    if (i != 0 && expression.at(i - 1) != ' ') {
                        noReplace = false;
                        expression.insert(i, " × ");
                        i = expression.length();
                    }
                }
            }
        }
    }

    //Check that each bracket is prepended (or appended) with an operation. Insert multiplcation signs if neccessary.
    noReplace = false;
    while (!noReplace) {
        noReplace = true;
        for (int i = 0; i < expression.length(); i++) {
            if (expression.at(i) == '(' && i != 0) {
                if (i != 1 && (expression.mid(i - 3, 3) == "sin" ||
                                expression.mid(i - 3, 3) == "cos" ||
                                expression.mid(i - 3, 3) == "tan" ||
                                expression.mid(i - 3, 3) == "log")) {
                    //Do nothing
                } else {
                    if (expression.at(i - 1) != ' ' && expression.mid(i - 2, 2) != "ln") {
                        noReplace = false;
                        expression.insert(i, " × ");
                    }
                }
            } else if (expression.at(i) == ')') {
                if (expression.count() > i + 1) {
                    if (expression.at(i + 1) != ' ' && expression.at(i + 1) != ')') {
                        noReplace = false;
                        expression.insert(i + 1, " × ");
                    }
                }
            }
        }
    }

    //Start by looking for any brackets and evaluate those first.
    bool noBrackets = false;
    while (!noBrackets) {
        noBrackets = true;
        uint currentBracketCount = 0;
        uint currentBracketStart;

        int length = expression.length();
        for (int i = 0; i < length; i++) {
            if (expression.at(i) == '(') {
                if (currentBracketCount == 0) {
                    currentBracketStart = i;
                }
                currentBracketCount++;
            } else if (expression.at(i) == ')') {
                if (currentBracketCount == 0) {
                    error = true;
                    return "Mismatched Brackets";
                } else if (currentBracketCount == 1) {
                    currentBracketCount--;
                    QString evaluatedBracket = evaluate(expression.mid(currentBracketStart + 1, i - currentBracketStart - 1), error);
                    if (error) {
                        error = true;
                        return evaluatedBracket;
                    } else {
                        expression.replace(currentBracketStart, i - currentBracketStart + 1, evaluatedBracket);

                        //Terminate the loop early
                        noBrackets = false;
                        i = length;
                    }
                } else {
                    currentBracketCount--;
                }
            }
        }

        if (currentBracketCount > 0) {
            error = true;
            return "Mismatched Brackets";
        }
    }

    //Calculate trigonometric and logarithmic functions.
    stageComplete = false;
    while (expression.contains("sin") || expression.contains("cos") || expression.contains("tan") || expression.contains("log") || expression.contains("ln")) {
        if (expression.contains("asin")) {
            int index = expression.indexOf("asin");
            index += 4; //Skip past the sin word

            //Now read until the end of the expression or until a space
            QString number;
            for (int i = index; i < expression.count(); i++) {
                if (expression.at(i) == ' ') {
                    index = i;
                    i = expression.count();
                } else {
                    number.append(expression.at(i));
                }
            }
            if (index == 3) index = expression.count();

            //Do some error checking
            if (number == "") {
                error = true;
                return "Invalid Syntax";
            } else if (number.toDouble() > 1 || number.toDouble() < -1) {
                error = true;
                return "Inverse sine of number outside of -1 and 1";
            }

            //Calculate the inverse sine of the number
            double result;
            if (ui->actionDegrees->isChecked()) {
                result = qRadiansToDegrees(qAsin(number.toDouble()));
            } else {
                result = qAsin(number.toDouble());
            }
            expression.replace(index - 4 - number.length(), number.length() + 4, QString::number(result, 'g', 15));
        } else if (expression.contains("acos")) {
            int index = expression.indexOf("acos");
            index += 4; //Skip past the acos word

            //Now read until the end of the expression or until a space
            QString number;
            for (int i = index; i < expression.count(); i++) {
                if (expression.at(i) == ' ') {
                    index = i;
                    i = expression.count();
                } else {
                    number.append(expression.at(i));
                }
            }
            if (index == 3) index = expression.count();

            if (number == "") {
                error = true;
                return "Invalid Syntax";
            } else if (number.toDouble() > 1 || number.toDouble() < -1) {
                error = true;
                return "Inverse cosine of number outside of -1 and 1";
            }

            //Calculate the inverse cosine of the number
            double result;
            if (ui->actionDegrees->isChecked()) {
                result = qRadiansToDegrees(qAcos(number.toDouble()));
            } else {
                result = qAcos(number.toDouble());
            }
            expression.replace(index - 4 - number.length(), number.length() + 4, QString::number(result, 'g', 15));
        } else if (expression.contains("atan")) {
            int index = expression.indexOf("atan");
            index += 4; //Skip past the atan word

            //Now read until the end of the expression or until a space
            QString number;
            for (int i = index; i < expression.count(); i++) {
                if (expression.at(i) == ' ') {
                    index = i;
                    i = expression.count();
                } else {
                    number.append(expression.at(i));
                }
            }
            if (index == 3) index = expression.count();

            if (number == "") {
                error = true;
                return "Invalid Syntax";
            }

            //Calculate the inverse tangent of the number
            double result;
            if (ui->actionDegrees->isChecked()) {
                result = qRadiansToDegrees(qAtan(number.toDouble()));
            } else {
                result = qAtan(number.toDouble());
            }
            expression.replace(index - 4 - number.length(), number.length() + 4, QString::number(result, 'g', 15));
        } else if (expression.contains("sin")) {
            int index = expression.indexOf("sin");
            index += 3; //Skip past the sin word

            //Now read until the end of the expression or until a space
            QString number;
            for (int i = index; i < expression.count(); i++) {
                if (expression.at(i) == ' ') {
                    index = i;
                    i = expression.count();
                } else {
                    number.append(expression.at(i));
                }
            }
            if (index == 3) index = expression.count();

            if (number == "") {
                error = true;
                return "Invalid Syntax";
            }

            //Calculate the sine of the number
            double result;
            if (ui->actionDegrees->isChecked()) {
                result = qSin(qDegreesToRadians(number.toDouble()));
            } else {
                result = qSin(number.toDouble());
            }
            expression.replace(index - 3 - number.length(), number.length() + 3, QString::number(result, 'g', 15));
        } else if (expression.contains("cos")) {
            int index = expression.indexOf("cos");
            index += 3; //Skip past the cos word

            //Now read until the end of the expression or until a space
            QString number;
            for (int i = index; i < expression.count(); i++) {
                if (expression.at(i) == ' ') {
                    index = i;
                    i = expression.count();
                } else {
                    number.append(expression.at(i));
                }
            }
            if (index == 3) index = expression.count();

            if (number == "") {
                error = true;
                return "Invalid Syntax";
            }

            //Calculate the cosine of the number
            double result;
            if (ui->actionDegrees->isChecked()) {
                result = qCos(qDegreesToRadians(number.toDouble()));
            } else {
                result = qCos(number.toDouble());
            }
            expression.replace(index - 3 - number.length(), number.length() + 3, QString::number(result, 'g', 15));
        } else if (expression.contains("tan")) {
            int index = expression.indexOf("tan");
            index += 3; //Skip past the tan word

            //Now read until the end of the expression or until a space
            QString number;
            for (int i = index; i < expression.count(); i++) {
                if (expression.at(i) == ' ') {
                    index = i;
                    i = expression.count();
                } else {
                    number.append(expression.at(i));
                }
            }
            if (index == 3) index = expression.count();

            //Check for errors
            if (number == "") {
                error = true;
                return "Invalid Syntax";
            } else if (ui->actionDegrees->isChecked()) {
                if (fmod(number.toDouble(), 90) == 0 && fmod(number.toDouble(), 180) != 0) {
                    //Tan of multiple of 90
                    error = true;
                    return "Tan of multiple of 180 from 90";
                }
            } else if (ui->actionRadians->isChecked()) {
                if (fmod(number.toDouble(), M_PI / 2) == 0 && fmod(number.toDouble(), M_PI) != 0) {
                    //Tan of multiple of PI/2
                    error = true;
                    return "Tan of multiple of π from π/2";
                }
            }

            //Calculate the tangent of the number
            double result;
            if (ui->actionDegrees->isChecked()) {
                result = qTan(qDegreesToRadians(number.toDouble()));
            } else {
                result = qTan(number.toDouble());
            }
            expression.replace(index - 3 - number.length(), number.length() + 3, QString::number(result, 'g', 15));
        } else if (expression.contains("log")) {
            int index = expression.indexOf("log");
            index += 3; //Skip past the log word

            //Now read until the end of the expression or until a space
            QString number;
            for (int i = index; i < expression.count(); i++) {
                if (expression.at(i) == ' ') {
                    index = i;
                    i = expression.count();
                } else {
                    number.append(expression.at(i));
                }
            }
            if (index == 3) index = expression.count();

            //Check for errors
            if (number == "") {
                error = true;
                return "Invalid Syntax";
            } else if (number == "0") {
                error = true;
                return "Log of 0";
            } else if (number.toDouble() < 0) {
                error = true;
                return "Log of negative number";
            }

            //Calculate the log of the number
            double result = log10(number.toDouble());
            expression.replace(index - 3 - number.length(), number.length() + 3, QString::number(result, 'g', 15));
        } else if (expression.contains("ln")) {
            int index = expression.indexOf("ln");
            index += 2; //Skip past the ln word

            //Now read until the end of the expression or until a space
            QString number;
            for (int i = index; i < expression.count(); i++) {
                if (expression.at(i) == ' ') {
                    index = i;
                    i = expression.count();
                } else {
                    number.append(expression.at(i));
                }
            }
            if (index == 3) index = expression.count();

            //Check for errors
            if (number == "") {
                error = true;
                return "Invalid Syntax";
            } else if (number == "0") {
                error = true;
                return "Log of 0";
            } else if (number.toDouble() < 0) {
                error = true;
                return "Log of negative number";
            }

            //Calculate the natural log of the number
            double result = qLn(number.toDouble());
            expression.replace(index - 2 - number.length(), number.length() + 2, QString::number(result, 'g', 15));
        }
    }

    if (expression.startsWith(" × ")) {
        expression.remove(0, 3);
    } else if (expression.startsWith(" - ")) {
        //Remove spaces so this becomes a negative number
        expression.remove(0, 1);
        expression.remove(1, 1);
    }

    if (expression.endsWith(" × ")) {
        expression.remove(expression.count() - 4, 3);
    }
    QStringList seperation = expression.split(" ", QString::SkipEmptyParts);
    QStringList numbers;
    QStringList operations;
    for (QString part : seperation) {
        if (part == "+" || part == "-" || part == "×" || part == "÷" || part == "^" || part == "<<" || part == ">>") {
            operations.append(part);
        } else {
            numbers.append(part);
        }
    }

    if (numbers.count() != operations.count() + 1) {
        error = true;
        return "Invalid Syntax";
    }

    /*
     * Now the numbers list contains all the numbers, and the operation
     * after that number is contained in the same index in the Operations
     * list.
     * */

    //Now do orders/powers/whatever.
    stageComplete = false;
    while (!stageComplete) {
        stageComplete = true;
        for (int i = 0; i < operations.count(); i++) {
            QString operation = operations.at(i);
            if (operation == "^") { //Raise this number to the power of the next one.
                double arg1 = numbers.at(i).toDouble();
                double arg2 = numbers.at(i + 1).toDouble();

                //Check for potential mathematical errors
                if (arg1 == 0 && arg2 == 0) {
                    //0^0 is undefined.
                    error = true;
                    return "Zero to the power of zero";
                } else if (arg1 < 0 && arg2 == 0.5) {
                    //Square root of a negative number
                    error = true;
                    return "Square root of a negative number";
                }

                double result = qPow(arg1, arg2);

                if (result != result) {
                    error = true;
                    return "Internal Error";
                }
                numbers.replace(i, QString::number(result, 'g', 20));
                numbers.removeAt(i + 1);
                operations.removeAt(i);

                //Exit the loop and restart (because indicies may have changed)
                i = operations.count();
                stageComplete = false;
            }
        }
    }

    //Now do multiplication, division and binary shifts
    stageComplete = false;
    while (!stageComplete) {
        stageComplete = true;
        for (int i = 0; i < operations.count(); i++) {
            QString operation = operations.at(i);
            if (operation == "×") { //Multiply this number with the next one.
                double arg1 = numbers.at(i).toDouble();
                double arg2 = numbers.at(i + 1).toDouble();
                double result = arg1 * arg2;
                numbers.replace(i, QString::number(result, 'g', 20));
                numbers.removeAt(i + 1);
                operations.removeAt(i);

                //Exit the loop and restart (because indicies may have changed)
                i = operations.count();
                stageComplete = false;
            } else if (operation == "÷") { //Divide this number with the next one
                double arg1 = numbers.at(i).toDouble();
                double arg2 = numbers.at(i + 1).toDouble();

                //Check for division by zero
                if (arg2 == 0) {
                    error = true;
                    return "Division by zero";
                }

                double result = arg1 / arg2;
                numbers.replace(i, QString::number(result, 'g', 20));
                numbers.removeAt(i + 1);
                operations.removeAt(i);

                //Exit the loop and restart (because indicies may have changed)
                i = operations.count();
                stageComplete = false;
            } else if (operation == ">>") { //Right Shift this number with the next one
                double arg1 = numbers.at(i).toDouble();
                double arg2 = numbers.at(i + 1).toDouble();

                //Check for errors
                double integerpart;
                if (modf(arg1, &integerpart) != 0 || modf(arg2, &integerpart) != 0) {
                    error = true;
                    return "Right shift with non integers";
                }

                int result = (int) arg1 >> (int) arg2;
                numbers.replace(i, QString::number(result, 'g', 20));
                numbers.removeAt(i + 1);
                operations.removeAt(i);

                //Exit the loop and restart (because indicies may have changed)
                i = operations.count();
                stageComplete = false;
            } else if (operation == "<<") { //Left Shift this number with the next one
                double arg1 = numbers.at(i).toDouble();
                double arg2 = numbers.at(i + 1).toDouble();

                //Check for errors
                double integerpart;
                if (modf(arg1, &integerpart) != 0 || modf(arg2, &integerpart) != 0) {
                    error = true;
                    return "Left shift with non integers";
                }

                int result = (int) arg1 << (int) arg2;
                numbers.replace(i, QString::number(result, 'g', 20));
                numbers.removeAt(i + 1);
                operations.removeAt(i);

                //Exit the loop and restart (because indicies may have changed)
                i = operations.count();
                stageComplete = false;
            }
        }
    }

    //Now do addition and subtraction
    stageComplete = false;
    while (!stageComplete) {
        stageComplete = true;
        for (int i = 0; i < operations.count(); i++) {
            QString operation = operations.at(i);
            if (operation == "+") { //Add this number with the next one.
                double arg1 = numbers.at(i).toDouble();
                double arg2 = numbers.at(i + 1).toDouble();
                double result = arg1 + arg2;
                numbers.replace(i, QString::number(result, 'g', 20));
                numbers.removeAt(i + 1);
                operations.removeAt(i);

                //Exit the loop and restart (because indicies may have changed)
                i = operations.count();
                stageComplete = false;
            } else if (operation == "-") { //Subtract this number with the next one
                double arg1 = numbers.at(i).toDouble();
                double arg2 = numbers.at(i + 1).toDouble();
                double result = arg1 - arg2;
                numbers.replace(i, QString::number(result, 'g', 20));
                numbers.removeAt(i + 1);
                operations.removeAt(i);

                //Exit the loop and restart (because indicies may have changed)
                i = operations.count();
                stageComplete = false;
            }
        }
    }

    expression = numbers.first();
    if (expression.contains("e")) {
        expression.replace("e", " × 10 ^ (");
        expression.append(")");
    }

    if (expression == "inf") {
        error = true;
        return "Overflow";
    } else if (expression == "-inf") {
        error = true;
        return "Negative Overflow";
    }

    qDebug() << "Answer: " + expression;
    qDebug() << "";
    return expression;
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event) {
    if (watched == ui->lineEdit) {
        suppressCursorChecking = true;
        if (event->type() == QEvent::KeyPress) {
            QKeyEvent* keyEvent = (QKeyEvent*) event;
            switch (keyEvent->key()) {
            /*
            case Qt::Key_1:
                ui->lineEdit->insert("1");
                break;
            case Qt::Key_2:
                ui->lineEdit->insert("2");
                break;
            case Qt::Key_3:
                ui->lineEdit->insert("3");
                break;
            case Qt::Key_4:
                ui->lineEdit->insert("4");
                break;
            case Qt::Key_5:
                ui->lineEdit->insert("5");
                break;
            case Qt::Key_6:
                ui->lineEdit->insert("6");
                break;
            case Qt::Key_7:
                ui->lineEdit->insert("7");
                break;
            case Qt::Key_8:
                ui->lineEdit->insert("8");
                break;
            case Qt::Key_9:
                ui->lineEdit->insert("9");
                break;
            case Qt::Key_0:
                ui->lineEdit->insert("0");
                break;*/
            case Qt::Key_Backspace:
                backspace();
                break;
            case Qt::Key_Left:
                if (ui->lineEdit->cursorPosition() != 0) {
                    if (ui->lineEdit->text().at(ui->lineEdit->cursorPosition() - 1) == ' ') {
                        ui->lineEdit->cursorBackward(false);
                        while (ui->lineEdit->text().at(ui->lineEdit->cursorPosition() - 1) != QChar(' ')) {
                            ui->lineEdit->cursorBackward(false);
                        }
                        ui->lineEdit->cursorBackward(false);
                    } else {
                        ui->lineEdit->cursorBackward(false);
                    }
                }
                break;
            case Qt::Key_Right:
                if (ui->lineEdit->text().count() > ui->lineEdit->cursorPosition()) {
                    if (ui->lineEdit->text().at(ui->lineEdit->cursorPosition()) == ' ') {
                        ui->lineEdit->cursorForward(false);
                        while (ui->lineEdit->text().at(ui->lineEdit->cursorPosition()) != QChar(' ')) {
                            ui->lineEdit->cursorForward(false);
                        }
                        ui->lineEdit->cursorForward(false);
                    } else {
                        ui->lineEdit->cursorForward(false);
                    }
                }
                break;
            case Qt::Key_Minus:
                ui->lineEdit->insert(" - ");
                break;
            case Qt::Key_Plus:
                ui->lineEdit->insert(" + ");
                break;
            case Qt::Key_Asterisk:
                ui->lineEdit->insert(" × ");
                break;
            case Qt::Key_Slash:
                ui->lineEdit->insert(" ÷ ");
                break;
            case Qt::Key_AsciiCircum:
                ui->lineEdit->insert(" ^ ");
                break;
            case Qt::Key_Less:
                ui->lineEdit->insert(" << ");
                break;
            case Qt::Key_Greater:
                ui->lineEdit->insert(" >> ");
                break;
            /*case Qt::Key_Period:
                ui->lineEdit->insert(".");
                break;
            case Qt::Key_ParenLeft:
                ui->lineEdit->insert("(");
                break;
            case Qt::Key_ParenRight:
                ui->lineEdit->insert(")");
                break;*/
            case Qt::Key_P:
                if (keyEvent->modifiers() & Qt::ControlModifier) {
                    ui->lineEdit->insert("π");
                } else {
                    suppressCursorChecking = false;
                    return false;
                }
                break;
            /*case Qt::Key_E:
                ui->lineEdit->insert("e");
                break;
            case Qt::Key_S:
                if (keyEvent->modifiers() & Qt::ShiftModifier) {
                    ui->lineEdit->insert("asin");
                } else {
                    ui->lineEdit->insert("sin");
                }
                break;
            case Qt::Key_C:
                if (keyEvent->modifiers() & Qt::ShiftModifier) {
                    ui->lineEdit->insert("acos");
                } else {
                    ui->lineEdit->insert("cos");
                }
                break;
            case Qt::Key_T:
                if (keyEvent->modifiers() & Qt::ShiftModifier) {
                    ui->lineEdit->insert("atan");
                } else {
                    ui->lineEdit->insert("tan");
                }
                break;
            case Qt::Key_X:
                ui->lineEdit->insert("x");
                break;
            case Qt::Key_Y:
                ui->lineEdit->insert("y");
                break;*/
            case Qt::Key_Return:
            case Qt::Key_Enter:
                ui->evaluateButton->click();
                break;
            case Qt::Key_Escape:
                ui->clearButton->click();
                break;
            default:
                suppressCursorChecking = false;
                return false;
            }
            suppressCursorChecking = false;
            return true;
        }
        suppressCursorChecking = false;
    }
    return false;
}

void MainWindow::on_lineEdit_cursorPositionChanged(int arg1, int arg2)
{
    /*if (!suppressCursorChecking) {
        if (arg2 != 0 && ui->lineEdit->text().at(arg2 - 1) == ' ') {
            ui->lineEdit->cursorBackward(false);
        } else if (ui->lineEdit->text().count() < arg2 && ui->lineEdit->text().at(arg2) == ' ') {
            ui->lineEdit->cursorForward(false);
        }
    }*/
}

void MainWindow::on_clearButton_clicked()
{
    ui->lineEdit->setText("");
    ui->previousExpression->setText("");
}

void MainWindow::on_oneButton_clicked()
{
    ui->lineEdit->insert("1");
}

void MainWindow::on_twoButton_clicked()
{
    ui->lineEdit->insert("2");
}

void MainWindow::on_threeButton_clicked()
{
    ui->lineEdit->insert("3");
}

void MainWindow::on_fourButton_clicked()
{
    ui->lineEdit->insert("4");
}

void MainWindow::on_fiveButton_clicked()
{
    ui->lineEdit->insert("5");
}

void MainWindow::on_sixButton_clicked()
{
    ui->lineEdit->insert("6");
}

void MainWindow::on_sevenButton_clicked()
{
    ui->lineEdit->insert("7");
}

void MainWindow::on_eightButton_clicked()
{
    ui->lineEdit->insert("8");
}

void MainWindow::on_nineButton_clicked()
{
    ui->lineEdit->insert("9");
}

void MainWindow::on_zeroButton_clicked()
{
    ui->lineEdit->insert("0");
}

void MainWindow::on_pointButton_clicked()
{
    ui->lineEdit->insert(".");
}

void MainWindow::on_plusButton_clicked()
{
    ui->lineEdit->insert(" + ");
}

void MainWindow::on_minusButton_clicked()
{
    ui->lineEdit->insert(" - ");
}

void MainWindow::on_multiplyButton_clicked()
{
    ui->lineEdit->insert(" × ");
}

void MainWindow::on_divideButton_clicked()
{
    ui->lineEdit->insert(" ÷ ");
}

void MainWindow::on_openBracketButton_clicked()
{
    ui->lineEdit->insert("(");
}

void MainWindow::on_closeBracketButton_clicked()
{
    ui->lineEdit->insert(")");
}

void MainWindow::on_piButton_clicked()
{
    ui->lineEdit->insert("π");
}

void MainWindow::on_backspaceButton_clicked()
{
    backspace();
}

void MainWindow::on_sinButton_clicked()
{
    ui->lineEdit->insert("sin");
}

void MainWindow::on_cosButton_clicked()
{
    ui->lineEdit->insert("cos");
}

void MainWindow::on_tanButton_clicked()
{
    ui->lineEdit->insert("tan");
}

void MainWindow::on_actionRadians_triggered()
{
    ui->actionDegrees->setChecked(false);
    ui->actionRadians->setChecked(true);
}

void MainWindow::on_actionDegrees_triggered()
{
    ui->actionDegrees->setChecked(true);
    ui->actionRadians->setChecked(false);
}

void MainWindow::on_actionExit_triggered()
{
    this->close();
}

void MainWindow::on_powerButton_clicked()
{
    ui->lineEdit->insert(" ^ ");
}

void MainWindow::on_exponentButton_clicked()
{
    ui->lineEdit->insert(" × 10 ^ ");
}

void MainWindow::backspace() {
    if (ui->lineEdit->cursorPosition() != 0) {
        /*if (ui->lineEdit->cursorPosition() != 1 &&

                (ui->lineEdit->text().mid(ui->lineEdit->cursorPosition() - 3, 3) == "sin" ||
                ui->lineEdit->text().mid(ui->lineEdit->cursorPosition() - 3, 3) == "cos" ||
                ui->lineEdit->text().mid(ui->lineEdit->cursorPosition() - 3, 3) == "tan" ||
                 ui->lineEdit->text().mid(ui->lineEdit->cursorPosition() - 3, 3) == "log" ||
                ui->lineEdit->text().mid(ui->lineEdit->cursorPosition() - 3, 3) == "Ans")) {
            ui->lineEdit->backspace();
            ui->lineEdit->backspace();
            ui->lineEdit->backspace();
        } else if (ui->lineEdit->text().mid(ui->lineEdit->cursorPosition() - 2, 2) == "ln") {
            ui->lineEdit->backspace();
            ui->lineEdit->backspace();
        } else */ if (ui->lineEdit->text().at(ui->lineEdit->cursorPosition() - 1) == ' ') {
            ui->lineEdit->backspace();
            while (ui->lineEdit->text().at(ui->lineEdit->cursorPosition() - 1) != QChar(' ')) {
                ui->lineEdit->backspace();
            }
            ui->lineEdit->backspace();
        } else {
            ui->lineEdit->backspace();
        }
    }
}

void MainWindow::on_reciporicalButton_clicked()
{
    ui->lineEdit->insert(" ^ ( - 1)");
}


void MainWindow::on_AnsButton_clicked()
{
    ui->lineEdit->insert("Ans");
}

void MainWindow::on_logButton_clicked()
{
    ui->lineEdit->insert("log");
}

void MainWindow::on_lnButton_clicked()
{
    ui->lineEdit->insert("ln");
}

void MainWindow::on_xButton_clicked()
{
    ui->lineEdit->insert("x");
}

void MainWindow::on_yButton_clicked()
{
    ui->lineEdit->insert("y");
}

void MainWindow::on_functionsButton_clicked()
{
    ui->functionsFrame->setGeometry(0, this->height() + 300, this->width(), this->height() + 300);

    QPropertyAnimation* winAnim = new QPropertyAnimation();
    winAnim->setStartValue(this->height());
    winAnim->setEndValue(this->height() + 300);
    winAnim->setDuration(500);
    winAnim->setEasingCurve(QEasingCurve::OutCubic);
    connect(winAnim, &QPropertyAnimation::valueChanged, [=](QVariant value) {
        this->setFixedHeight(value.toInt());
    });
    connect(winAnim, SIGNAL(finished()), winAnim, SLOT(deleteLater()));

    QPropertyAnimation* anim = new QPropertyAnimation(ui->functionsFrame, "geometry");
    anim->setStartValue(ui->functionsFrame->geometry());
    anim->setEndValue(QRect(0, ui->oneButton->mapTo(this, QPoint(0, 0)).y(), this->width(), this->height() + 300 - ui->oneButton->mapTo(this, QPoint(0, 0)).y()));
    anim->setDuration(500);
    anim->setEasingCurve(QEasingCurve::OutCubic);
    connect(anim, SIGNAL(finished()), anim, SLOT(deleteLater()));

    QParallelAnimationGroup* group = new QParallelAnimationGroup();
    group->addAnimation(winAnim);
    group->addAnimation(anim);
    connect(group, SIGNAL(finished()), group, SLOT(deleteLater()));
    group->start();
}

void MainWindow::on_functionsClose_clicked()
{
    QPropertyAnimation* winAnim = new QPropertyAnimation();
    winAnim->setStartValue(this->height());
    winAnim->setEndValue(this->height() - 300);
    winAnim->setDuration(500);
    winAnim->setEasingCurve(QEasingCurve::OutCubic);
    connect(winAnim, &QPropertyAnimation::valueChanged, [=](QVariant value) {
        this->setFixedHeight(value.toInt());
    });
    connect(winAnim, SIGNAL(finished()), winAnim, SLOT(deleteLater()));

    QPropertyAnimation* anim = new QPropertyAnimation(ui->functionsFrame, "geometry");
    anim->setStartValue(ui->functionsFrame->geometry());
    anim->setEndValue(QRect(0, this->height() - 300, this->width(), this->height() - ui->oneButton->mapTo(this, QPoint(0, 0)).y()));
    anim->setDuration(500);
    anim->setEasingCurve(QEasingCurve::OutCubic);
    connect(anim, SIGNAL(finished()), anim, SLOT(deleteLater()));

    QParallelAnimationGroup* group = new QParallelAnimationGroup();
    group->addAnimation(winAnim);
    group->addAnimation(anim);
    connect(group, SIGNAL(finished()), group, SLOT(deleteLater()));
    group->start();
}

void MainWindow::on_functionsNew_clicked()
{
    QString function = QInputDialog::getText(this, "Enter Function", "Enter the name of this function. This should take the form of f(x)", QLineEdit::Normal, "function(x)");
    if (function != "") {
        ui->instructionLabel->setVisible(true);
        ui->doneButton->setVisible(true);
        ui->evaluateButton->setEnabled(false);
        ui->functionsButton->setEnabled(false);

        ui->functionsTable->insertRow(ui->functionsTable->rowCount());
        QTableWidgetItem* item1 = new QTableWidgetItem;
        item1->setText(function);
        ui->functionsTable->setItem(ui->functionsTable->rowCount() - 1, 0, item1);
        QTableWidgetItem* item2 = new QTableWidgetItem;
        ui->functionsTable->setItem(ui->functionsTable->rowCount() - 1, 1, item2);

        editingFunction = ui->functionsTable->rowCount() - 1;

        this->layout()->update();
        QPropertyAnimation* winAnim = new QPropertyAnimation();
        winAnim->setStartValue(this->height());
        winAnim->setEndValue(this->sizeHint().height());
        winAnim->setDuration(500);
        winAnim->setEasingCurve(QEasingCurve::OutCubic);
        connect(winAnim, &QPropertyAnimation::valueChanged, [=](QVariant value) {
            this->setFixedHeight(value.toInt());
        });
        connect(winAnim, SIGNAL(finished()), winAnim, SLOT(deleteLater()));

        QPropertyAnimation* anim = new QPropertyAnimation(ui->functionsFrame, "geometry");
        anim->setStartValue(ui->functionsFrame->geometry());
        anim->setEndValue(QRect(0, this->sizeHint().height(), this->width(), this->sizeHint().height() - ui->oneButton->mapTo(this, QPoint(0, 0)).y()));
        anim->setDuration(500);
        anim->setEasingCurve(QEasingCurve::OutCubic);
        connect(anim, SIGNAL(finished()), anim, SLOT(deleteLater()));

        QParallelAnimationGroup* group = new QParallelAnimationGroup();
        group->addAnimation(winAnim);
        group->addAnimation(anim);
        connect(group, SIGNAL(finished()), group, SLOT(deleteLater()));
        group->start();

        /*QPropertyAnimation* anim = new QPropertyAnimation(ui->functionsFrame, "geometry");
        anim->setStartValue(ui->functionsFrame->geometry());
        anim->setEndValue(QRect(0, this->height(), this->width(), this->height() - ui->oneButton->mapTo(this, QPoint(0, 0)).y()));
        anim->setDuration(500);
        anim->setEasingCurve(QEasingCurve::OutCubic);
        connect(anim, SIGNAL(finished()), anim, SLOT(deleteLater()));
        anim->start();*/
    }
}

void MainWindow::on_doneButton_clicked()
{
    ui->functionsTable->item(editingFunction, 1)->setText(ui->lineEdit->text());

    settings.beginGroup("functions");
    settings.setValue(ui->functionsTable->item(editingFunction, 0)->text(), ui->lineEdit->text());
    settings.endGroup();

    ui->instructionLabel->setVisible(false);
    ui->doneButton->setVisible(false);
    ui->evaluateButton->setEnabled(true);
    ui->functionsButton->setEnabled(true);

    this->layout()->update();
    QPropertyAnimation* winAnim = new QPropertyAnimation();
    winAnim->setStartValue(this->height());
    winAnim->setEndValue(this->sizeHint().height() + 300);
    winAnim->setDuration(500);
    winAnim->setEasingCurve(QEasingCurve::OutCubic);
    connect(winAnim, &QPropertyAnimation::valueChanged, [=](QVariant value) {
        this->setFixedHeight(value.toInt());
    });
    connect(winAnim, SIGNAL(finished()), winAnim, SLOT(deleteLater()));

    QPropertyAnimation* anim = new QPropertyAnimation(ui->functionsFrame, "geometry");
    anim->setStartValue(ui->functionsFrame->geometry());
    anim->setEndValue(QRect(0, ui->oneButton->mapTo(this, QPoint(0, 0)).y(), this->width(), this->sizeHint().height() + 300 - ui->oneButton->mapTo(this, QPoint(0, 0)).y()));
    anim->setDuration(500);
    anim->setEasingCurve(QEasingCurve::OutCubic);
    connect(anim, SIGNAL(finished()), anim, SLOT(deleteLater()));

    QParallelAnimationGroup* group = new QParallelAnimationGroup();
    group->addAnimation(winAnim);
    group->addAnimation(anim);
    connect(group, SIGNAL(finished()), group, SLOT(deleteLater()));
    group->start();
    /*QPropertyAnimation* anim = new QPropertyAnimation(ui->functionsFrame, "geometry");
    anim->setStartValue(ui->functionsFrame->geometry());
    anim->setEndValue(QRect(0, ui->oneButton->mapTo(this, QPoint(0, 0)).y() - ui->instructionLabel->height(), this->width(), this->height() - ui->oneButton->mapTo(this, QPoint(0, 0)).y() + ui->instructionLabel->height()));
    anim->setDuration(500);
    anim->setEasingCurve(QEasingCurve::OutCubic);
    connect(anim, SIGNAL(finished()), anim, SLOT(deleteLater()));
    anim->start();*/
}

void MainWindow::on_functionsInsert_clicked()
{
    QString input = QInputDialog::getText(this, "Parameters", "Enter Parameter for x");
    if (input != "") {
        QString function = ui->functionsTable->item(ui->functionsTable->currentRow(), 0)->text();
        function.replace("x", input);
        ui->lineEdit->insert(function);
    }
}

void MainWindow::on_functionsTable_currentCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn)
{
    if (currentRow == -1) {
        ui->functionsInsert->setEnabled(false);
    } else {
        ui->functionsInsert->setEnabled(true);
    }
}
