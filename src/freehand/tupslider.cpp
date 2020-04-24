/*
 * ShiraPlayer(TM)
 * Copyright (C) 2012 Asaf Yurdakul
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 * ShiraPlayer is a trademark of Sureyyasoft.
 */

#include "tupslider.h"

#include <QDesktopWidget>
#include <QDebug>

struct TupSlider::Private
{
    int min;
    int max;
    QColor startColor;
    QColor endColor;
    QImage *image;
    int value;
    bool enabled;
    Qt::Orientation orientation;
    TupSlider::Mode mode;
    Qt::BrushStyle style;
    double opacity;
    int currentBase;
};

TupSlider::TupSlider(Qt::Orientation orientation, TupSlider::Mode mode, const QColor &start, const QColor &end, QWidget *parent) : QGraphicsView(parent), k(new Private)
{
    k->orientation = orientation; 
    k->mode = mode;
    k->startColor = start;
    k->endColor = end;
    k->value = 0;
    k->enabled = true;

    if (k->orientation == Qt::Vertical)
        k->image = new QImage(QString(":/graphicGui/gui/tip_v.png"));
    else 
        k->image = new QImage(QString(":/graphicGui/gui/tip_h.png"));

    setUpdatesEnabled(true);

    if (k->orientation == Qt::Vertical)
        k->currentBase = viewport()->height();
    else
        k->currentBase = viewport()->width();

}

TupSlider::~TupSlider()
{
}

void TupSlider::setBrushSettings(Qt::BrushStyle style, double opacity)
{
    k->style = style;
    k->opacity = opacity;
}

void TupSlider::setRange(int min, int max)
{
    k->min = min;
    k->max = max;
}

void TupSlider::setValue(int value)
{
    if (k->orientation == Qt::Vertical) {
        int height = viewport()->height();
        if (value == k->max) {
            k->value = height - k->image->size().height();
        } else if (value == k->min) {
            k->value = 0;
        } else {
            k->value = height*value/k->max;
        }
    } else {
        int width = viewport()->width();
        if (value == k->max) {
            k->value = width - k->image->size().width();
        } else if (value == k->min) {
            k->value = 0;
        } else {
            k->value = width*value/k->max;
        }
    }

    this->update();
}

void TupSlider::setEnabled(bool flag)
{
    k->enabled = flag;
    this->update();
}

bool TupSlider::isEnabled()
{
    return k->enabled;
}

void TupSlider::setColors(const QColor &start, const QColor &end)
{
    k->startColor = start;
    k->endColor = end;
    this->update();
}

void TupSlider::mousePressEvent(QMouseEvent *event)
{
    if (!k->enabled)
        return;

    int pos = -1;
    if (k->orientation == Qt::Vertical)
        pos = event->y();
    else
        pos = event->x();

    calculateNewPosition(pos);
}

void TupSlider::mouseMoveEvent(QMouseEvent *event)
{
    if (!k->enabled) 
        return;

    int pos = -1;
    if (k->orientation == Qt::Vertical) 
        pos = event->y();
    else
        pos = event->x();

    calculateNewPosition(pos);
}

void TupSlider::calculateNewPosition(int pos)
{
    int length = -1;

    if (k->orientation == Qt::Vertical) {
        length = viewport()->height();
        k->value = pos;
        if (pos > (length - k->image->size().height())) {
            k->value = length - k->image->size().height();
            this->update();
            emit valueChanged(k->min);
            return;
        }
    } else {
        length = viewport()->width();
        if (pos > (length - k->image->size().width())) {
            k->value = length - k->image->size().width();
            this->update();
            emit valueChanged(k->max);
            return;
        }
        k->value = pos;
    }

    int value = -1;
    if (k->orientation == Qt::Vertical) 
        value = k->min + (k->max - k->min) * (1.0 - float(pos)/float(length));
    else 
        value = k->min + (k->max - k->min) * (float(pos)/float(length));

    if (k->value < 0)
        k->value = 0;

    if (value < k->min)
        value = k->min;

    this->update();
    emit valueChanged(value);
}

void TupSlider::paintScales()
{
    QPainter painter(viewport());

    if (!k->enabled) {
        QColor color(232, 232, 232);
        painter.setPen(color);
        painter.setBrush(color);
        painter.drawRect(0, 0, viewport()->width(), viewport()->height());
        return;
    }

    int width = viewport()->width();
    int height = viewport()->height();
    int imageW = k->image->size().width()-16;
    int imageH = k->image->size().height()-16;
    int length = viewport()->width();
    if (k->orientation == Qt::Vertical)
        length = viewport()->height();
    int segments = 32;
    int delta = length/(segments-1);

    if (k->mode == Color) {
        for (int section=0; section<=segments; section++) {
             QColor color;
             int r;
             int g;
             int b;
             r = section*(k->endColor.red() - k->startColor.red()) / segments + k->startColor.red();
             g = section*(k->endColor.green() - k->startColor.green()) / segments + k->startColor.green();
             b = section*(k->endColor.blue() - k->startColor.blue()) / segments + k->startColor.blue();

             if ((r > -1 && r < 256) && (g > -1 && g < 256) && (b > -1 && b < 256)) {
                 color.setRed(r);
                 color.setGreen(g);
                 color.setBlue(b);

                 painter.setPen(color);
                 painter.setBrush(color);

                 if (k->orientation == Qt::Vertical)
                     painter.drawRect((width - imageW)/2, section*delta, imageW, delta);
                 else 
                     painter.drawRect(section*delta, (height - imageH)/2, delta, imageH);
            }
        }
    } else if (k->mode == Size) {
               painter.setPen(QColor(232, 232, 232));
               painter.setBrush(QBrush(k->endColor, k->style));
               painter.setOpacity(k->opacity);
               painter.setRenderHint(QPainter::Antialiasing);

               QPainterPath path;

               if (k->orientation == Qt::Vertical) {
                   path = QPainterPath(QPointF(0, 0));
                   path.lineTo(QPointF(0, height));
                   path.lineTo(QPointF(width, height));
                   path.lineTo(QPointF(0, 0));
               } else {
                   path = QPainterPath(QPointF(0, height));
                   path.lineTo(QPointF(width, 0)); 
                   path.lineTo(QPointF(width, height));
                   path.lineTo(QPointF(0, height));
               }

               painter.drawPath(path);
               painter.setOpacity(1.0);
    } else {
               double opacityDelta = 1.0/32; 
               double opacity = 0;
               for (int section=0; section<=segments; section++) {
                    painter.setPen(QColor(232, 232, 232));
                    painter.setBrush(k->endColor);
                    painter.setOpacity(opacity);
                    opacity += opacityDelta;

                    if (k->orientation == Qt::Vertical) {
                        painter.drawRect((width - imageW)/2, section*delta, imageW, delta);
                     } else {
                        painter.drawRect(section*delta, (height - imageH)/2, delta, imageH);
                     }
               }
               painter.setOpacity(1.0);
    }


    if (k->orientation == Qt::Vertical) {
        int h = viewport()->height();
        int imgW = k->image->size().width();
        if (k->value >= h || k->currentBase != h) {
            k->value = (k->value * h)/k->currentBase;;
            k->currentBase = h;
        }

        painter.drawImage((width/2)-(imgW/2), k->value, *k->image);
    } else {
        int w = viewport()->width();
        int imgH = k->image->size().height();
        if (k->value >= w || k->currentBase != w) {
            k->value = (k->value * w)/k->currentBase;
            k->currentBase = w;
        }

        painter.drawImage(k->value, (height/2)-(imgH/2), *k->image);
    }
}

void TupSlider::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    paintScales();
}
