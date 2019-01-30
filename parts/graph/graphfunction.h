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
#ifndef GRAPHFUNCTION_H
#define GRAPHFUNCTION_H

#include "evaluationengineheaders.h"
#include <QObject>
#include <QGraphicsItem>
#include <QApplication>

class GraphView;
struct GraphFunctionPrivate;

class GraphFunction : public QGraphicsItem
{
    Q_DECLARE_TR_FUNCTIONS(GraphFunction)

    public:
        explicit GraphFunction(GraphView* view, QString expression = "", QGraphicsItem *parent = nullptr);
        ~GraphFunction();

        struct FunctionValue {
            idouble value;
            bool isUndefined = false;
        };

        FunctionValue value(idouble x);
        void setExpression(QString expression);
        void setColor(QColor color);

        void redraw();

        QRectF boundingRect() const;
        void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr);
        QPainterPath shape() const;

    private:
        GraphFunctionPrivate* d;

        void hoverEnterEvent(QGraphicsSceneHoverEvent* event);
        void hoverMoveEvent(QGraphicsSceneHoverEvent* event);
        void hoverLeaveEvent(QGraphicsSceneHoverEvent* event);
};

#endif // GRAPHFUNCTION_H
