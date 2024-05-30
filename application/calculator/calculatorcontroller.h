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

    signals:
        void expressionStringChanged();
        void instantResultChanged();
        void cursorPositionChanged();
        void balancingBracketsChanged();

    private:
        CalculatorControllerPrivate* d;

        void expressionStringUpdated();
};

#endif // CALCULATORCONTROLLER_H
