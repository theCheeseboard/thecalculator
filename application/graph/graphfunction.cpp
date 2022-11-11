/****************************************
 *
 *   INSERT-PROJECT-NAME-HERE - INSERT-GENERIC-NAME-HERE
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
#include "graphfunction.h"

#include "evaluationengine.h"
#include "graphview.h"
#include <QGraphicsPathItem>
#include <QGraphicsSceneHoverEvent>
#include <QGraphicsSimpleTextItem>
#include <QTimer>
#include <math.h>

namespace std {
    extern size_t qHash(const idouble& t, size_t seed);
}

struct GraphFunctionPrivate {
        QHash<idouble, GraphFunction::FunctionValue> yvalues;

        QString expression;
        GraphView* parentView;
        QColor color = QColor(0, 200, 0);

        QGraphicsSimpleTextItem* textItem;
        QGraphicsItemGroup* graphGroup;
        QGraphicsPathItem* colItem;

        QSharedPointer<bool> oldStillWorking;
        QSharedPointer<QMutex> oldStillWorkingMutex;
        int numComplete, numTotal;
        bool ready = false;
        QPainterPath boundingRect;

        QTimer* redrawTimer;
};

GraphFunction::GraphFunction(GraphView* view, QString expression, QGraphicsItem* parent) :
    QGraphicsObject(parent) {
    d = new GraphFunctionPrivate();

    d->parentView = view;
    d->expression = expression;
    connect(view, &GraphView::canvasChanged, this, [=] {
        this->redraw();
    });

    this->setAcceptHoverEvents(true);

    d->graphGroup = new QGraphicsItemGroup(this);

    d->colItem = new QGraphicsPathItem(this);
    d->colItem->setPen(QPen(Qt::transparent));
    d->colItem->setVisible(false);

    d->textItem = new QGraphicsSimpleTextItem(this);
    d->textItem->setVisible(false);

    d->redrawTimer = new QTimer();
    d->redrawTimer->setInterval(500);
    d->redrawTimer->setSingleShot(true);
    connect(d->redrawTimer, &QTimer::timeout, this, [=] {
        this->doRedraw();
    });

    if (expression != "") {
        redraw();
    }
}

GraphFunction::~GraphFunction() {
    delete d->graphGroup;
    delete d->textItem;
    delete d;
}

void GraphFunction::setExpression(QString expression) {
    d->expression = expression;
    d->yvalues.clear();

    redraw();
}

void GraphFunction::setColor(QColor color) {
    d->color = color;
    d->colItem->setBrush(color);
}

void GraphFunction::redraw(bool immediate) {
    // Clear everything
    if (!d->oldStillWorking.isNull()) {
        QMutexLocker locker(d->oldStillWorkingMutex.data());
        *d->oldStillWorking = false;
    }
    while (d->graphGroup->childItems().count() > 0) {
        delete d->graphGroup->childItems().takeFirst();
    }
    d->boundingRect = QPainterPath();

    if (d->ready) {
        d->ready = false;
        emit readyChanged(false);
    }

    if (immediate) {
        this->doRedraw();
    } else {
        // Start the timer so we space out the redraw events
        if (d->redrawTimer->isActive()) {
            d->redrawTimer->stop();
        }

        d->redrawTimer->start();
    }
}

void GraphFunction::doRedraw() {
    QPainterPath path;

    int numPoints = d->parentView->canvasSize().width() * 10;
    double coordinatesInWidth = d->parentView->canvasSize().width() / d->parentView->xScale();
    double unroundedPrecision = coordinatesInWidth / numPoints;

    double floorPrecision = ceil(log10(unroundedPrecision));
    const double precision = pow(10, floorPrecision);

    // Start at xOffset
    double firstPoint = floor(d->parentView->xOffset() / precision) * precision; // First point in cartesian coordinates
    double lastPoint = firstPoint + d->parentView->canvasSize().width() / d->parentView->xScale();

    // Draw chunks of 100 points
    struct PromiseReturn {
            QPainterPath path;
            QHash<idouble, GraphFunction::FunctionValue> values;
    };

    QSharedPointer<bool> stillWorking(new bool(true));
    QSharedPointer<QMutex> stillWorkingMutex(new QMutex);
    d->oldStillWorking = stillWorking;
    d->oldStillWorkingMutex = stillWorkingMutex;
    d->numComplete = 0;
    d->numTotal = 0;

    for (double nextFirstPoint = firstPoint; nextFirstPoint < lastPoint; nextFirstPoint += precision * 1000) {
        QHash<idouble, GraphFunction::FunctionValue> localCache = d->yvalues;
        (new tPromise<PromiseReturn>([=](QString& error) -> PromiseReturn {
            PromiseReturn retval;
            bool nextMove = true;
            for (double nextPoint = nextFirstPoint, xPoint = (nextFirstPoint - d->parentView->xOffset()) * d->parentView->xScale(), i = 0;
                 i < 1000; nextPoint += precision, xPoint += precision * d->parentView->xScale(), i++) {
                if (!stillWorking.data()) return PromiseReturn(); // Bail out

                FunctionValue v = value(idouble(nextPoint), retval.values, localCache);
                if (v.isUndefined || abs(v.value.imag()) > 0.000001) {
                    nextMove = true;
                } else {
                    // Calculate the y pixel coordinate
                    double yOffset = v.value.real() - d->parentView->yOffset(); // Cartesian coordinates from the bottom of the viewport
                    int top = d->parentView->canvasSize().height() - yOffset * d->parentView->yScale();
                    if (d->parentView->canvasSize().height() < top) {
                        nextMove = true;
                    } else {
                        if (nextMove) {
                            retval.path.moveTo(xPoint, top);
                        } else {
                            retval.path.lineTo(xPoint, top);
                        }
                        nextMove = false;
                    }
                }
            }

            return retval;
        }))->then([=](PromiseReturn retval) {
            QMutexLocker locker(stillWorkingMutex.data());
            if (stillWorking.data()) {
                locker.unlock();

                QGraphicsPathItem* item = new QGraphicsPathItem(retval.path);
                item->setPen(QPen(d->color, 3));
                d->graphGroup->addToGroup(item);
                d->boundingRect = d->boundingRect.united(retval.path);

                for (idouble key : retval.values.keys()) {
                    d->yvalues.insert(key, retval.values.value(key));
                }

                d->numComplete++;
                if (d->numComplete == d->numTotal && !d->ready) {
                    d->ready = true;
                    emit readyChanged(true);
                }
            }
        });
        d->numTotal++;
    }

    this->update();
    /*
    for (double nextPoint = firstPoint, xPoint = (firstPoint - d->parentView->xOffset()) * d->parentView->xScale();
         xPoint < d->parentView->canvasSize().width(); nextPoint += precision, xPoint += precision * d->parentView->xScale()) {
        FunctionValue v = value(idouble(nextPoint));
        if (v.isUndefined || abs(v.value.imag()) > 0.000001) {
            nextMove = true;
        } else {
            //Calculate the y pixel coordinate
            double yOffset = v.value.real() - d->parentView->yOffset(); //Cartesian coordinates from the bottom of the viewport
            int top = d->parentView->canvasSize().height() - yOffset * d->parentView->yScale();
            if (d->parentView->canvasSize().height() < top) {
                nextMove = true;
            } else {
                if (nextMove) {
                    path.moveTo(xPoint, top);
                } else {
                    path.lineTo(xPoint, top);
                }
                nextMove = false;
            }
        }
    }

    QGraphicsPathItem* item = new QGraphicsPathItem(path);
    item->setPen(QPen(d->color, 3));
    d->graphGroup->addToGroup(item);*/
}

GraphFunction::FunctionValue GraphFunction::value(idouble x, QHash<idouble, GraphFunction::FunctionValue>& addHash, QHash<idouble, GraphFunction::FunctionValue> readHash) {
    if (d->expression == "") {
        FunctionValue v;
        v.isUndefined = true;
        return v;
    }

    if (readHash.contains(x)) {
        return readHash.value(x);
    } else {
        QMap<QString, idouble> vars;
        vars.insert("x", x);

        EvaluationEngine e;
        e.setVariables(vars);
        e.setTrigonometricUnit(EvaluationEngine::Radians);
        e.setExpression(d->expression);
        EvaluationEngine::Result evaluation = e.evaluate();

        FunctionValue v;
        switch (evaluation.type) {
            case EvaluationEngine::Result::Scalar:
                v.value = evaluation.result;
                break;
            default:
                v.isUndefined = true;
                break;
        }
        addHash.insert(x, v);
        return v;
    }
}

QRectF GraphFunction::boundingRect() const {
    return QRectF(0, 0, d->parentView->canvasSize().width(), d->parentView->canvasSize().height());
}

void GraphFunction::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
    painter->setRenderHint(QPainter::Antialiasing);
    for (QGraphicsItem* child : this->childItems()) {
        child->paint(painter, option, widget);
    }
}

void GraphFunction::hoverEnterEvent(QGraphicsSceneHoverEvent* event) {
    d->textItem->setVisible(true);
    d->colItem->setVisible(true);

    QPainterPath colPath;
    colPath.addRect(0, 0, SC_DPI(16), SC_DPI(16));
    d->colItem->setPath(colPath);

    this->hoverMoveEvent(event);
}

void GraphFunction::hoverMoveEvent(QGraphicsSceneHoverEvent* event) {
    QStringList textParts;
    textParts.append("f(x) = " + d->expression);

    double xValue = d->parentView->xOffset() + event->pos().x() / d->parentView->xScale();
    textParts.append(QString("x: ").append(QString::number(xValue)));

    FunctionValue v = value(idouble(xValue), d->yvalues, d->yvalues);
    if (v.isUndefined) {
        textParts.append(QString("y: ").append(tr("undefined")));
    } else {
        textParts.append(QString("y: ").append(idbToString(v.value)));
    }

    d->textItem->setText(textParts.join("\n"));
    d->textItem->setPos(event->pos() + SC_DPI_T(QPoint(32, 10), QPoint));

    d->colItem->setPos(event->pos() + SC_DPI_T(QPoint(10, 10), QPoint));

    if (d->textItem->boundingRect().right() > d->parentView->canvasSize().width()) {
        // Move the hover to the other side
        d->textItem->moveBy(-(d->textItem->boundingRect().width() + SC_DPI(42)), 0);
        d->colItem->moveBy(-(d->textItem->boundingRect().width() + SC_DPI(42)), 0);
    }
}

void GraphFunction::hoverLeaveEvent(QGraphicsSceneHoverEvent* event) {
    d->textItem->setVisible(false);
    d->textItem->setText("");
    d->colItem->setVisible(false);
    d->colItem->setPath(QPainterPath());
}

QPainterPath GraphFunction::shape() const {
    QPainterPathStroker stroker;
    stroker.setWidth(5);
    return stroker.createStroke(d->boundingRect);
}

bool GraphFunction::isReady() {
    return d->ready;
}
