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

#ifndef TUPPENPREVIEWCANVAS_H
#define TUPPENPREVIEWCANVAS_H

#include <QWidget>
#include <QPainter>
#include <QPaintEvent>
#include <QPen>
#include <QSize>

class TupPenPreviewCanvas : public QWidget
{
    Q_OBJECT

    public:
        TupPenPreviewCanvas(QPen pen, double opacity, QWidget *parent = 0);
        ~TupPenPreviewCanvas();

        QSize minimumSizeHint() const;
        QSize sizeHint() const;

    public slots:
        void render(int width);
        void render(double opacity);
        void setColor(QColor color);
        
    protected:
        void paintEvent(QPaintEvent *e);
   
    private:
        struct Private;
        Private *const k;
};

#endif
