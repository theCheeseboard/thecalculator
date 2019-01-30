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

#include "graphview.h"
#include "evaluationengine.h"
#include <math.h>
#include <QGraphicsSceneHoverEvent>
#include <QGraphicsPathItem>
#include <QGraphicsSimpleTextItem>

extern QString idbToString(idouble db);

struct GraphFunctionPrivate {
    QHash<idouble, GraphFunction::FunctionValue> yvalues;

    QString expression;
    GraphView* parentView;
    QColor color = QColor(0, 200, 0);
    QMetaObject::Connection canvasChangedConnection;

    QGraphicsSimpleTextItem* textItem;
    QGraphicsItemGroup* graphGroup;
    QGraphicsPathItem* colItem;
};

GraphFunction::GraphFunction(GraphView* view, QString expression, QGraphicsItem *parent) : QGraphicsItem(parent)
{
    d = new GraphFunctionPrivate();

    d->parentView = view;
    d->expression = expression;
    d->canvasChangedConnection = QObject::connect(view, &GraphView::canvasChanged, [=] {
        this->redraw();
    });

    this->setAcceptHoverEvents(true);

    d->graphGroup = new QGraphicsItemGroup(this);

    d->colItem = new QGraphicsPathItem(this);
    d->colItem->setPen(QPen(Qt::transparent));
    d->colItem->setVisible(false);

    d->textItem = new QGraphicsSimpleTextItem(this);
    d->textItem->setVisible(false);

    if (expression != "") {
        redraw();
    }
}

GraphFunction::~GraphFunction() {
    QObject::disconnect(d->canvasChangedConnection);

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

void GraphFunction::redraw() {
    while (d->graphGroup->childItems().count() > 0) {
        delete d->graphGroup->childItems().takeFirst();
    }

    QPainterPath path;

    int numPoints = d->parentView->width() * 10;
    double coordinatesInWidth = d->parentView->width() / d->parentView->xScale();
    double unroundedPrecision = coordinatesInWidth / numPoints;

    double floorPrecision = ceil(log10(unroundedPrecision));
    const double precision = pow(10, floorPrecision);

    //Start at xOffset
    bool nextMove = true;
    double firstPoint = floor(d->parentView->xOffset() / precision) * precision; //First point in cartesian coordinates
    for (double nextPoint = firstPoint, xPoint = (firstPoint - d->parentView->xOffset()) * d->parentView->xScale();
         xPoint < d->parentView->width(); nextPoint += precision, xPoint += precision * d->parentView->xScale()) {
        FunctionValue v = value(idouble(nextPoint));
        if (v.isUndefined || abs(v.value.imag()) > 0.000001) {
            nextMove = true;
        } else {
            //Calculate the y pixel coordinate
            double yOffset = v.value.real() - d->parentView->yOffset(); //Cartesian coordinates from the bottom of the viewport
            int top = d->parentView->height() - yOffset * d->parentView->yScale();
            if (d->parentView->height() < top) {
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
    d->graphGroup->addToGroup(item);
}

GraphFunction::FunctionValue GraphFunction::value(idouble x) {
    if (d->expression == "") {
        FunctionValue v;
        v.isUndefined = true;
        return v;
    }

    if (d->yvalues.contains(x)) {
        return d->yvalues.value(x);
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
        d->yvalues.insert(x, v);
        return v;
    }
}

QRectF GraphFunction::boundingRect() const {
    return QRectF(0, 0, d->parentView->width(), d->parentView->height());
}

void GraphFunction::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
    painter->setRenderHint(QPainter::Antialiasing);
    for (QGraphicsItem* child : this->childItems()) {
        child->paint(painter, option, widget);
    }
}

void GraphFunction::hoverEnterEvent(QGraphicsSceneHoverEvent *event) {
    d->textItem->setVisible(true);
    d->colItem->setVisible(true);

    QPainterPath colPath;
    colPath.addRect(0, 0, 16 * theLibsGlobal::getDPIScaling(), 16 * theLibsGlobal::getDPIScaling());
    d->colItem->setPath(colPath);

    this->hoverMoveEvent(event);
}

void GraphFunction::hoverMoveEvent(QGraphicsSceneHoverEvent *event) {
    QStringList textParts;
    textParts.append("f(x) = " + d->expression);

    double xValue = d->parentView->xOffset() + event->pos().x() / d->parentView->xScale();
    textParts.append(QString("x: ").append(QString::number(xValue)));

    FunctionValue v = value(idouble(xValue));
    if (v.isUndefined) {
        textParts.append(QString("y: ").append(tr("undefined")));
    } else {
        textParts.append(QString("y: ").append(idbToString(v.value)));
    }

    d->textItem->setText(textParts.join("\n"));
    d->textItem->setPos(event->pos() + QPoint(32, 10) * theLibsGlobal::getDPIScaling());

    d->colItem->setPos(event->pos() + QPoint(10, 10) * theLibsGlobal::getDPIScaling());

    if (d->textItem->boundingRect().right() > d->parentView->width()) {
        //Move the hover to the other side
        d->textItem->moveBy(-(d->textItem->boundingRect().width() + 42 * theLibsGlobal::getDPIScaling()), 0);
        d->colItem->moveBy(-(d->textItem->boundingRect().width() + 42 * theLibsGlobal::getDPIScaling()), 0);
    }
}

void GraphFunction::hoverLeaveEvent(QGraphicsSceneHoverEvent *event) {
    d->textItem->setVisible(false);
    d->textItem->setText("");
    d->colItem->setVisible(false);
    d->colItem->setPath(QPainterPath());
}

QPainterPath GraphFunction::shape() const {
    QPainterPath path;
    for (QGraphicsItem* item : d->graphGroup->childItems()) {
        QGraphicsPathItem* pathItem = (QGraphicsPathItem*) item;
        path = path.united(pathItem->path());
    }

    QPainterPathStroker stroker;
    stroker.setWidth(5);
    return stroker.createStroke(path);
}
