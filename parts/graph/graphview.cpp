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

    QVector<QGraphicsItem*> functions;

    int numReady = 0;
    bool ready = false;

    QSize canvasSize;
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
    int distance = -canvasSize().width() / 2; //Distance in pixels between center of viewport and left
    double distanceCo = distance / xScale(); //Change to coordinate system
    return distanceCo + d->center.x(); //Add to center coordinates
}

double GraphView::yOffset() {
    //Bottommost coordinate
    int distance = -canvasSize().height() / 2; //Distance in pixels between center of viewport and bottom
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

    int xNumLocation;
    if (yOffset() > 0) {
        xNumLocation = canvasSize().height();
    } else if (yOffset() + canvasSize().height() / yScale() < 0) {
        xNumLocation = 0;
    } else {
        xNumLocation = canvasSize().height() + yOffset() * yScale();
    }

    if (xNumLocation + this->fontMetrics().height() > canvasSize().height()) {
        xNumLocation -= (this->fontMetrics().height() + 3);
    } else {
        xNumLocation += 3;
    }

    int yNumLocation;
    if (xOffset() > 0) {
        yNumLocation = 0;
    } else if (xOffset() + canvasSize().width() / xScale() < 0) {
        yNumLocation = canvasSize().width();
    } else {
        yNumLocation = -xOffset() * xScale();
    }
    yNumLocation += 3;

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
        for (double nextLine = (firstLine - xOffset()) * xScale(), xLine = firstLine; nextLine < canvasSize().width(); nextLine += xSpacing * xScale(), xLine += xSpacing) {
            if (abs(xLine) < 0.0000001) {
                painter->save();
                painter->setPen(Qt::blue);
            }

            painter->drawLine(nextLine, 0, nextLine, canvasSize().height());
            if (abs(xLine) < 0.0000001) {
                painter->restore();
            } else {
                if (transparency > 50) {
                    QString text = QString::number(xLine);
                    QRect textRect;
                    textRect.setWidth(fontMetrics().width(text) + 1);
                    textRect.setHeight(fontMetrics().height());
                    textRect.moveCenter(QPoint(nextLine, xNumLocation + fontMetrics().height() / 2));
                    painter->fillRect(textRect, Qt::white);
                    painter->drawText(textRect, text);
                }
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
        for (double nextLine = (firstLine - yOffset()) * yScale(), yLine = firstLine; nextLine < canvasSize().height(); nextLine += ySpacing * yScale(), yLine += ySpacing) {
            if (abs(yLine) < 0.0000001) {
                painter->save();
                painter->setPen(Qt::blue);
            }

            painter->drawLine(0, canvasSize().height() - nextLine, canvasSize().width(), canvasSize().height() - nextLine);
            if (abs(yLine) < 0.0000001) {
                painter->restore();
            } else {
                if (transparency > 50) {
                    QString text = QString::number(yLine);
                    QRect textRect;
                    textRect.setWidth(fontMetrics().width(text) + 1);
                    textRect.setHeight(fontMetrics().height());
                    textRect.moveCenter(QPoint(yNumLocation + fontMetrics().width(text) / 2, canvasSize().height() - nextLine));

                    if (textRect.right() > canvasSize().width()) {
                        textRect.translate(-textRect.width() - 6, 0);
                    }

                    painter->fillRect(textRect, Qt::white);
                    painter->drawText(textRect, text);
                }
            }
        }
    }
}

void GraphView::resizeEvent(QResizeEvent *event) {
    this->setSceneRect(0, 0, this->width(), this->height());
    this->scene()->setSceneRect(this->sceneRect());
    d->canvasSize = QSize(this->width(), this->height());

    this->setBackgroundBrush(Qt::white);
    emit canvasChanged();
}

void GraphView::render(QPainter *painter, QSizeF size) {
    this->setUpdatesEnabled(false);
    this->scene()->setSceneRect(0, 0, size.width(), size.height());
    d->canvasSize = size.toSize();

    //emit canvasChanged();
    drawBackground(painter, QRectF(0, 0, size.width(), size.height()));
    for (QGraphicsItem* item : d->functions) {
        GraphFunction* fn = (GraphFunction*) item;
        fn->redraw(true);
    }

    QEventLoop* loop = new QEventLoop();
    QMetaObject::Connection* c = new QMetaObject::Connection;
    *c = connect(this, &GraphView::readyChanged, [=](bool ready) {
        if (ready) {
            disconnect(*c);
            delete c;

            this->scene()->render(painter);

            this->setSceneRect(0, 0, this->width(), this->height());
            d->canvasSize = QSize(this->width(), this->height());

            //Redraw everything again
            for (QGraphicsItem* item : d->functions) {
                GraphFunction* fn = (GraphFunction*) item;
                fn->redraw(true);
            }
            this->setUpdatesEnabled(true);

            loop->exit();
        }
    });
    loop->exec();
    loop->deleteLater();
}

void GraphView::addFunction(QGraphicsItem* item) {
    this->scene()->addItem(item);
    d->functions.append(item);
    GraphFunction* fn = (GraphFunction*) item;
    connect(fn, &GraphFunction::readyChanged, fn, [=](bool ready) {
        if (ready) {
            d->numReady++;
            if (d->numReady >= d->functions.count() && d->ready == false) {
                d->ready = true;
                emit readyChanged(true);
            }
        } else {
            if (d->numReady != 0) {
                d->numReady--;
                d->ready = false;
                emit readyChanged(false);
            }
        }
    });
}

void GraphView::removeFunction(QGraphicsItem* item) {
    if (d->functions.contains(item)) {
        GraphFunction* fn = (GraphFunction*) item;
        if (fn->isReady()) d->numReady--;
        d->functions.removeOne(item);
        this->scene()->removeItem(item);
    }
}

QSize GraphView::canvasSize() {
    return d->canvasSize;
}
