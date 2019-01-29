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
#ifndef GRAPHVIEW_H
#define GRAPHVIEW_H

#include <QGraphicsView>

struct GraphViewPrivate;
class GraphView : public QGraphicsView
{
        Q_OBJECT
    public:
        explicit GraphView(QWidget *parent = nullptr);
        ~GraphView();

        double xScale();
        double yScale();
        double xOffset();
        double yOffset();

        void setCenter(QPointF center);

    signals:
        void canvasChanged();

    public slots:

    protected:
        void drawBackground(QPainter* painter, const QRectF& rect);

    private:
        GraphViewPrivate* d;

        void resizeEvent(QResizeEvent* event);
};

#endif // GRAPHVIEW_H
