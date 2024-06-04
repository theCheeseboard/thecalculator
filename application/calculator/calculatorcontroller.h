#ifndef CALCULATORCONTROLLER_H
#define CALCULATORCONTROLLER_H

#include <QAbstractItemModel>
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
        Q_PROPERTY(int errorStartLocation READ errorStartLocation NOTIFY instantResultChanged FINAL)
        Q_PROPERTY(int errorEndLocation READ errorEndLocation NOTIFY instantResultChanged FINAL)
        Q_PROPERTY(QAbstractItemModel* history READ history CONSTANT FINAL)
        Q_PROPERTY(TrigonometricUnit trigonometricUnit READ trigonometricUnit WRITE setTrigonometricUnit NOTIFY trigonometricUnitChanged FINAL)
        QML_ELEMENT
    public:
        explicit CalculatorController(QObject* parent = nullptr);
        ~CalculatorController();

        enum TrigonometricUnit {
            Degrees,
            Radians,
            Gradians
        };
        Q_ENUM(TrigonometricUnit)

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

        TrigonometricUnit trigonometricUnit();
        void setTrigonometricUnit(TrigonometricUnit unit);

        int errorStartLocation();
        int errorEndLocation();

        QAbstractItemModel* history();

        Q_SCRIPTABLE void performEvaluation();

        QString evaluateExpression(QString expression, bool* success);

    signals:
        void expressionStringChanged();
        void instantResultChanged();
        void cursorPositionChanged();
        void balancingBracketsChanged();
        void intellisenseChanged();
        void trigonometricUnitChanged();
        Q_SCRIPTABLE void evaluationError();

    private:
        CalculatorControllerPrivate* d;

        void expressionStringUpdated();
        void calculateIntellisense();
};

#endif // CALCULATORCONTROLLER_H
