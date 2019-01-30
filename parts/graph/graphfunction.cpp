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
#include <QGraphicsPathItem>

struct GraphFunctionPrivate {
    QHash<idouble, GraphFunction::FunctionValue> yvalues;

    QString expression;
    GraphView* parentView;
    QColor color = QColor(0, 200, 0);
};

GraphFunction::GraphFunction(GraphView* view, QString expression, QGraphicsItem *parent) : QGraphicsItem(parent)
{
    d = new GraphFunctionPrivate();

    d->parentView = view;
    d->expression = expression;
    QObject::connect(view, &GraphView::canvasChanged, [=] {
        this->redraw();
    });

    if (expression != "") {
        redraw();
    }
}

GraphFunction::~GraphFunction() {
    delete d;
}

void GraphFunction::setExpression(QString expression) {
    d->expression = expression;
    d->yvalues.clear();

    redraw();
}

void GraphFunction::setColor(QColor color) {
    d->color = color;
}

void GraphFunction::redraw() {
    while (this->childItems().count() > 0) {
        delete this->childItems().takeFirst();
    }

    QPainterPath path;

    int numPoints = d->parentView->width() * 10;
    double coordinatesInWidth = d->parentView->width() / d->parentView->xScale();
    double unroundedPrecision = coordinatesInWidth / numPoints;

    double floorPrecision = ceil(log10(unroundedPrecision));
    const double precision = pow(10, floorPrecision);

    //Start at xOffset
    bool nextMove = true;
    double firstPoint = floor(d->parentView->xOffset() * precision) / precision; //First point in cartesian coordinates
    for (double nextPoint = firstPoint, xPoint = (firstPoint - d->parentView->xOffset()) * d->parentView->xScale();
         xPoint < d->parentView->width(); nextPoint += precision, xPoint += precision * d->parentView->xScale()) {
        FunctionValue v = value(idouble(nextPoint));
        if (v.isUndefined || v.value.imag() != 0) {
            nextMove = true;
        } else {
            //Calculate the y pixel coordinate
            double yOffset = v.value.real() - d->parentView->yOffset(); //Cartesian coordinates from the bottom of the viewport
            int top = d->parentView->height() - yOffset * d->parentView->yScale();
            if (nextMove) {
                path.moveTo(xPoint, top);
            } else {
                path.lineTo(xPoint, top);
            }
            nextMove = false;
        }
    }

    QGraphicsPathItem* item = new QGraphicsPathItem(path);
    item->setPen(QPen(d->color, 3));
    item->setParentItem(this);
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
