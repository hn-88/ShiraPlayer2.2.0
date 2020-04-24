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

#ifndef TUPSLIDER_H
#define TUPSLIDER_H

#include <QGraphicsView>
#include <QMouseEvent>
#include <QImage>

class TupSlider : public QGraphicsView
{
    Q_OBJECT

    public:
        enum Mode { Color = 0, Size, Opacity };

        explicit TupSlider(Qt::Orientation orientation, Mode mode, const QColor& start, const QColor& end, QWidget *parent = 0);
        ~TupSlider();

        void setBrushSettings(Qt::BrushStyle style, double opacity);
        void setRange(int min, int max);
        void setColors(const QColor& start, const QColor& end);
        void setValue(int value);
        void setEnabled(bool flag);
        bool isEnabled();

    protected:
        void mousePressEvent(QMouseEvent *event);
        void mouseMoveEvent(QMouseEvent *event);
        void paintEvent(QPaintEvent *event);

        void paintScales();

    signals:
        void valueChanged(int v);

    private:
       void calculateNewPosition(int pos);
       struct Private;
       Private *const k;
};

#endif
