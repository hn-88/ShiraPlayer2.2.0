/*
 * ShiraPlayer(TM)
 * Copyright (C) 2008 Fabien Chereau
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


#ifndef _MAPLABEL_HPP_
#define _MAPLABEL_HPP_

#include <QLabel>

//! @class MapLabel
//! Special QLabel that shows a world map 
class MapLabel : public QLabel
{
	Q_OBJECT

public:
	MapLabel(QWidget *parent = 0);
	~MapLabel();
	//! Set the current cursor position
	//! @param longitude longitude in degree in range [-180;180[
	//! @param latitude latitude in degree in range [-90;90]
	void setCursorPos(double longitude, double latitude);
	
signals:
	//! Signal emitted when we click on the map
	void positionChanged(double longitude, double latitude);

protected:
	virtual void mousePressEvent(QMouseEvent * event);

private:
	QLabel* cursor;
};

#endif // _MAPLABEL_HPP_
