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

#include <QApplication>
#include <QCoroTask>
#include <QGraphicsObject>
#include <idouble.h>

class GraphView;
struct GraphFunctionPrivate;

class GraphFunction : public QGraphicsObject {
        Q_OBJECT

    public:
        explicit GraphFunction(GraphView* view, QString expression = "", QGraphicsItem* parent = nullptr);
        ~GraphFunction();

        struct FunctionValue {
                idouble value;
                bool isUndefined = false;
        };

        QCoro::Task<FunctionValue> value(idouble x, QHash<idouble, GraphFunction::FunctionValue>& addHash, QHash<idouble, GraphFunction::FunctionValue> readHash);
        void setExpression(QString expression);
        void setColor(QColor color);

        void redraw(bool immediate = false);
        bool isReady();

        QRectF boundingRect() const;
        void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr);
        QPainterPath shape() const;

    signals:
        void readyChanged(bool ready);

    private:
        GraphFunctionPrivate* d;

        void hoverEnterEvent(QGraphicsSceneHoverEvent* event);
        void hoverMoveEvent(QGraphicsSceneHoverEvent* event);
        void hoverLeaveEvent(QGraphicsSceneHoverEvent* event);

        void doRedraw();
        QCoro::Task<> updateHover(QPointF pos);
};

#endif // GRAPHFUNCTION_H
