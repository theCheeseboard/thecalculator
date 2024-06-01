#ifndef CALCULATORCONTROLLER_H
#define CALCULATORCONTROLLER_H

#include <QObject>
#include <QQmlEngine>

struct CalculatorControllerPrivate;
class CalculatorController : public QObject {
        Q_OBJECT
        Q_PROPERTY(QString expressionString READ expressionString WRITE setExpressionString NOTIFY expressionStringChanged FINAL)
        Q_PROPERTY(QString instantResult READ instantResult NOTIFY instantResultChanged FINAL)
        Q_PROPERTY(QString balancingBrackets READ balancingBrackets NOTIFY balancingBracketsChanged FINAL)
        Q_PROPERTY(int cursorPosition READ cursorPosition WRITE setCursorPosition NOTIFY cursorPositionChanged FINAL)
        Q_PROPERTY(bool intellisenseAvailable READ intellisenseAvailable NOTIFY intellisenseChanged FINAL)
        Q_PROPERTY(QString intellisenseFunction READ intellisenseFunction NOTIFY intellisenseChanged FINAL)
        Q_PROPERTY(QString intellisenseDescription READ intellisenseDescription NOTIFY intellisenseChanged FINAL)
        Q_PROPERTY(QString intellisenseArguments READ intellisenseArguments NOTIFY intellisenseChanged FINAL)
        QML_ELEMENT
    public:
        explicit CalculatorController(QObject* parent = nullptr);
        ~CalculatorController();

        QString expressionString();
        void setExpressionString(QString expressionString);
        Q_SCRIPTABLE void clearExpression();
        Q_SCRIPTABLE void pressKey(QString key);
        Q_SCRIPTABLE void backspace();

        int cursorPosition();
        void setCursorPosition(int cursorPosition);
        Q_SCRIPTABLE void cursorLeft();
        Q_SCRIPTABLE void cursorRight();

        QString balancingBrackets();
        QString instantResult();

        bool intellisenseAvailable();
        QString intellisenseFunction();
        QString intellisenseDescription();
        QString intellisenseArguments();

        Q_SCRIPTABLE void performEvaluation();

        QString evaluateExpression(QString expression);

    signals:
        void expressionStringChanged();
        void instantResultChanged();
        void cursorPositionChanged();
        void balancingBracketsChanged();
        void intellisenseChanged();
        Q_SCRIPTABLE void evaluationError();

    private:
        CalculatorControllerPrivate* d;

        void expressionStringUpdated();
        void calculateIntellisense();
};

#endif // CALCULATORCONTROLLER_H
