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

#include "tuppenpreviewcanvas.h"

struct TupPenPreviewCanvas::Private
{
    int width;
    double opacity;
    QColor color;
    Qt::BrushStyle style;
};

TupPenPreviewCanvas::TupPenPreviewCanvas(QPen pen, double opacity, QWidget *parent) : QWidget(parent), k(new Private)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    k->width = pen.width();
    k->color = pen.color();
    k->opacity = opacity;
    k->style = pen.brush().style();
}

TupPenPreviewCanvas::~TupPenPreviewCanvas()
{
}

void TupPenPreviewCanvas::render(int width)
{
    k->width = width;
    update();
}

void TupPenPreviewCanvas::render(double opacity)
{
    k->width = 30;
    k->opacity = opacity;
    update();
}

void TupPenPreviewCanvas::setColor(QColor color)
{
    k->color = color;
}

QSize TupPenPreviewCanvas::minimumSizeHint() const
{
    return QSize(100, 56);
}

QSize TupPenPreviewCanvas::sizeHint() const
{
    return QSize(100, 56);
}

void TupPenPreviewCanvas::paintEvent(QPaintEvent *)
{
     QPainter painter(this);
     painter.setRenderHint(QPainter::Antialiasing, true);
     painter.fillRect(0, 0, width(), height(), QColor(255, 255, 255));

     QPen border(QColor(0, 0, 0));
     border.setWidth(0.5);
     painter.setPen(border);
     painter.drawRect(0, 0, width(), height());

     painter.translate(width() / 2, height() / 2);

     QBrush brush;
     brush = QBrush(k->color, k->style);

     QPen pen(Qt::NoPen);
     painter.setPen(pen);
     painter.setBrush(brush);
     painter.setOpacity(k->opacity);
     painter.drawEllipse(-(k->width/2), -(k->width/2), k->width, k->width);
}
