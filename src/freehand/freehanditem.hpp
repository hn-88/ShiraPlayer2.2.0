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

#ifndef FREEHANDITEM_HPP
#define FREEHANDITEM_HPP

#include <QObject>
#include <QPen>
#include <QSharedPointer>
class freehandItem
{
public:
    freehandItem(QPainterPath path, QPen pen,int size, double opacity);

    void setPen (QPen val) { m_pen = val;}
    QPen getPen () { return m_pen ; }

    void setSize(int val) {m_size = val; }
    int getSize() { return m_size;}

    void setOpacity(int val) {m_opacity = val; }
    double getOpacity() {return m_opacity;}

    void setPath(QPainterPath val) { m_path = val;}
    QPainterPath getPath() { return m_path;}
private:
    QPen m_pen;
    QPainterPath m_path;
    int m_size;
    Qt::BrushStyle m_brushStyle;
    double m_opacity;

};
typedef QSharedPointer<freehandItem> freehandItemPtr;

#endif // FREEHANDITEM_HPP
