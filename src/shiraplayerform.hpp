/*
 * ShiraPlayer(TM)
 * Copyright (C) 2011 Asaf Yurdakul
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

#ifndef _SHIRAPLAYERFORM_HPP
#define _SHIRAPLAYERFORM_HPP

#include <QGraphicsView>

class ShiraPlayerForm : public QGraphicsView
{
Q_OBJECT
public:
    ShiraPlayerForm(QWidget* parent);
    virtual ~ShiraPlayerForm();

    //! Get the ShiraPlayerForm singleton instance.
    static ShiraPlayerForm& getInstance() {Q_ASSERT(singleton); return *singleton;}
    void SettextLogoPos(int x,int y);

    QTimer* minFpsTimer;
private:
    //! The ShiraPlayerForm singleton
    static ShiraPlayerForm* singleton;
    QGraphicsPixmapItem* texLogo;


protected:
    virtual void resizeEvent(QResizeEvent* event);
signals:

public slots:
    void ShowContextMenu(const QPoint& pos) ;

};

#endif // _SHIRAPLAYERFORM_HPP
