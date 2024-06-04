#ifndef UNITSCONTROLLER_H
#define UNITSCONTROLLER_H

#include <QObject>
#include <QQmlEngine>

struct UnitsControllerPrivate;
class UnitsController : public QObject {
        Q_OBJECT
        QML_ELEMENT
    public:
        explicit UnitsController(QObject* parent = nullptr);
        ~UnitsController();

        Q_SCRIPTABLE QString evaluate(QString expression, QString x, bool forward);

    signals:

    private:
        UnitsControllerPrivate* d;
};

#endif // UNITSCONTROLLER_H
