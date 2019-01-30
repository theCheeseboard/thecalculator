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
#include "graphview.h"

#include <QPainter>
#include "graphfunction.h"
#include <math.h>
#include <QDebug>

struct GraphViewPrivate {
    double xOffset;
    double yOffset;
    double xScale = 10;
    double yScale = 10;
    QPointF center;
};

GraphView::GraphView(QWidget *parent) : QGraphicsView(parent)
{
    d = new GraphViewPrivate();

    d->center = QPointF(0, 0);
    this->setScene(new QGraphicsScene());
    this->setAutoFillBackground(true);

    this->setBackgroundBrush(Qt::white);
}

GraphView::~GraphView() {
    delete d;
}

void GraphView::setCenter(QPointF center) {
    d->center = center;

    this->setBackgroundBrush(Qt::white);
    emit canvasChanged();
}

void GraphView::setXScale(double xScale) {
    d->xScale = xScale;

    this->setBackgroundBrush(Qt::white);
    emit canvasChanged();
}

void GraphView::setYScale(double yScale) {
    d->yScale = yScale;

    this->setBackgroundBrush(Qt::white);
    emit canvasChanged();
}

double GraphView::xScale() {
    return d->xScale;
}

double GraphView::yScale() {
    return d->yScale;
}

double GraphView::xOffset() {
    //Leftmost coordinate
    int distance = -this->width() / 2; //Distance in pixels between center of viewport and left
    double distanceCo = distance / xScale(); //Change to coordinate system
    return distanceCo + d->center.x(); //Add to center coordinates
}

double GraphView::yOffset() {
    //Bottommost coordinate
    int distance = -this->height() / 2; //Distance in pixels between center of viewport and bottom
    double distanceCo = distance / yScale(); //Change to coordinate system
    return distanceCo + d->center.y(); //Add to center coordinates
}

void GraphView::drawBackground(QPainter *painter, const QRectF& rect) {
    //Fill the background with white
    painter->fillRect(rect, Qt::white);

    const double spacings[] = {
        00000.00001,
        00000.00010,
        00000.00100,
        00000.01000,
        00000.10000,
        00001.00000,
        00010.00000,
        00100.00000,
        01000.00000,
        10000.00000
    };

    for (double xSpacing : spacings) {
        int transparency;
        if (xSpacing * xScale() > 100) {
            transparency = 255 * 100 / xSpacing * xScale();
        } else {
            transparency = 255 * xSpacing * xScale() / 100;
        }
        if (transparency < 10) continue; //Don't bother with insignificant lines

        painter->setPen(QColor(0, 0, 0, transparency));
        double firstLine = ceil(xOffset() / xSpacing) * xSpacing;
        for (float nextLine = (firstLine - xOffset()) * xScale(), xLine = firstLine; nextLine < this->width(); nextLine += xSpacing * xScale(), xLine += xSpacing) {
            if (abs(xLine) < 0.0001) {
                painter->save();
                painter->setPen(Qt::blue);
            }
            painter->drawLine(nextLine, 0, nextLine, this->height());
            if (abs(xLine) < 0.0001) {
                painter->restore();
            }
        }
    }

    //Draw the y grid
    for (double ySpacing : spacings) {
        int transparency;
        if (ySpacing * yScale() > 100) {
            transparency = 255 * 100 / ySpacing * yScale();
        } else {
            transparency = 255 * ySpacing * yScale() / 100;
        }
        if (transparency < 10) continue; //Don't bother with insignificant lines

        painter->setPen(QColor(0, 0, 0, transparency));
        double firstLine = ceil(yOffset() / ySpacing) * ySpacing;
        for (float nextLine = (firstLine - yOffset()) * yScale(), yLine = firstLine; nextLine < this->height(); nextLine += ySpacing * yScale(), yLine += ySpacing) {
            if (abs(yLine) < 0.0001) {
                painter->save();
                painter->setPen(Qt::blue);
            }
            painter->drawLine(0, this->height() - nextLine, this->width(), this->height() - nextLine);
            if (abs(yLine) < 0.0001) {
                painter->restore();
            }
        }
    }
}

void GraphView::resizeEvent(QResizeEvent *event) {
    this->setSceneRect(0, 0, this->width(), this->height());
    this->scene()->setSceneRect(this->sceneRect());

    emit canvasChanged();
}
