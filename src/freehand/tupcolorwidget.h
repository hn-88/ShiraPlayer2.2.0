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

#ifndef TUPCOLORWIDGET_H
#define TUPCOLORWIDGET_H

#include <QBrush>
#include <QSize>
#include <QPaintEvent>
#include <QWidget>

class TupColorWidget : public QWidget
{
    Q_OBJECT

    public:
        TupColorWidget(int index, const QBrush &brush, const QSize &size, bool isCell);
        ~TupColorWidget();
        QSize sizeHint() const;
        QColor color();
        void setState(bool isSelected);
        bool isSelected();
        void setBrush(const QBrush &brush);
        void setEditable(bool flag);

    protected:
        void paintEvent(QPaintEvent *painter);
        void mousePressEvent(QMouseEvent *event);
        void mouseDoubleClickEvent(QMouseEvent *event);

    signals:
        void clicked(int index);        
        void doubledClicked(int index);

    private:
        struct Private;
        Private *const k;
};

#endif
