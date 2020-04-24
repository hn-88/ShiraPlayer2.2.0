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

#ifndef CHANNEL_H
#define CHANNEL_H

#include <QObject>
#include <QtXml/QtXml>

#include "warping/Warp.h"
#include "VecMath.hpp"

class Channel : public QObject
{
    Q_OBJECT

public:
    Channel();
    void setName(const QString& name);
    QString getName();
    QDomElement domElement(const QString& name, QDomDocument& doc,bool withBlend) const;
    bool initFromDOMElement(const QDomElement& element);

    double m_initfov;
    Vec3d  m_init_view_pos;
    float  m_fwr;
    float  m_fhr;

    void setDistWarp(Warp* w);
    void setBlendWarp(Warp* w);
    Warp* getDistWarp(){return m_pDistWarp;};
    Warp* getBlendWarp(){return m_pBlendWarp;};
    Warp* getCurrentWarp() const;

    void setinitFov(double fov);
    void setinitPos(Vec3d initpos);
    Vec3d stringtoVector(QString vecstr);
    void updateClientData();

    void setWidth(int val);
    void setHeight(int val);
    int getWidth();
    int getHeight();

    void set_FWR(float fwr);
    void set_FHR(float fhr);
private:
    QString m_name;     //!< Channel name.
    Warp* m_pDistWarp;
    Warp* m_pBlendWarp;

    int width;
    int height;
};

#endif // CHANNEL_H
