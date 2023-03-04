#include "baseevaluationengine.h"

#include "bisonflex/bisonflexevaluationengine.h"
#include <QMutexLocker>

struct BaseEvaluationEnginePrivate {
        BaseEvaluationEngine::TrigonometricUnit trigUnit = BaseEvaluationEngine::Degrees;
        QMutex trigUnitLocker;
};

BaseEvaluationEngine::BaseEvaluationEngine(QObject* parent) :
    QObject{parent} {
    d = new BaseEvaluationEnginePrivate();
}

BaseEvaluationEngine::~BaseEvaluationEngine() {
    delete d;
}

BaseEvaluationEngine* BaseEvaluationEngine::current() {
    static BaseEvaluationEngine* instance = new BisonFlexEvaluationEngine();
    return instance;
}

void BaseEvaluationEngine::setTrigonometricUnit(TrigonometricUnit trigUnit) {
    QMutexLocker locker(&d->trigUnitLocker);
    d->trigUnit = trigUnit;
}

BaseEvaluationEngine::TrigonometricUnit BaseEvaluationEngine::trigonometricUnit() {
    return d->trigUnit;
}
