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
#include "tupcolorwidget.h"

#include <QPainter>
#include <QDebug>

struct TupColorWidget::Private
{
    QBrush brush;
    int index;
    bool editable;
    bool selected;
    bool isCell;
    QSize size;
};

TupColorWidget::TupColorWidget(int index, const QBrush &brush, const QSize &size, bool isCell) : k(new Private)
{
    k->index = index;
    k->editable = true;
    k->selected = false;
    k->brush = brush;
    k->isCell = isCell;
    k->size = size;
    setFixedSize(k->size);
}

TupColorWidget::~TupColorWidget()
{
}

QSize TupColorWidget::sizeHint() const 
{
    return k->size;
}

void TupColorWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.fillRect(rect(), k->brush);
    QRect border = rect();

    if (k->selected && k->editable) {
        if (k->isCell) {
#ifndef Q_OS_ANDROID
            painter.setPen(QPen(Qt::white, 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
            painter.drawRect(border);
            painter.setPen(QPen(QColor(255, 255, 255, 50), 10, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
            painter.drawRect(border);
#endif
        } else {
#ifdef Q_OS_ANDROID
            int width = 20;
#else
            int width = 10;
#endif
            painter.setPen(QPen(QColor(200, 200, 200), width, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
            painter.drawRect(border);
            painter.setPen(QPen(QColor(190, 190, 190), 4, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
            painter.drawRect(border);
            painter.setPen(QPen(QColor(150, 150, 150), 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
            painter.drawRect(border);
        }
    } else {
        painter.setPen(QPen(QColor(190, 190, 190), 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        painter.drawRect(border);
    }
}

void TupColorWidget::mousePressEvent(QMouseEvent *event)
{
    Q_UNUSED(event);

    emit clicked(k->index);
    setState(true);
}

void TupColorWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    emit doubledClicked(k->index);
}

QColor TupColorWidget::color()
{
    return k->brush.color();
}

void TupColorWidget::setState(bool isSelected)
{
    k->selected = isSelected;
    update();
}

bool TupColorWidget::isSelected()
{
    return k->selected;
}

void TupColorWidget::setBrush(const QBrush &brush) 
{
    k->brush = brush;
    update();
}

void TupColorWidget::setEditable(bool flag) 
{
    k->editable = flag;
}
