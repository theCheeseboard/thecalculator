#ifndef BASEEVALUATIONENGINE_H
#define BASEEVALUATIONENGINE_H

#include "idouble.h"
#include <QCoroTask>
#include <QObject>

typedef std::function<idouble(QList<idouble>, QString&)> CustomFunctionDefinition;

class LIBTHECALCULATOR_EXPORT CustomFunction {
    public:
        virtual CustomFunctionDefinition getFunction() const = 0;
        virtual QString getDescription(int overload = 0) const = 0;
        virtual QStringList getArgs(int overload = 0) const = 0;

        virtual int overloads() = 0;
};

typedef QSharedPointer<CustomFunction> CustomFunctionPtr;
typedef QMap<QString, CustomFunctionPtr> CustomFunctionMap;

struct BaseEvaluationEnginePrivate;
class BaseEvaluationEngine : public QObject {
        Q_OBJECT
    public:
        explicit BaseEvaluationEngine(QObject* parent = nullptr);
        ~BaseEvaluationEngine();

        enum TrigonometricUnit {
            Degrees,
            Radians,
            Gradians
        };

        struct Result {
                enum ResultType {
                    Error,
                    Scalar,
                    Assign,
                    Equality
                };

                QString error;
                int location;
                int length;

                idouble result;

                bool assigned;
                QString identifier;
                idouble value;

                bool isTrue;

                ResultType type;
        };

        static BaseEvaluationEngine* current();

        virtual QCoro::Task<Result> evaluate(QString expression, QMap<QString, idouble> variables = QMap<QString, idouble>()) = 0;
        void setTrigonometricUnit(TrigonometricUnit trigUnit);
        TrigonometricUnit trigonometricUnit();

        virtual CustomFunctionMap customFunctions() const = 0;

    signals:

    private:
        BaseEvaluationEnginePrivate* d;
};

#endif // BASEEVALUATIONENGINE_H
