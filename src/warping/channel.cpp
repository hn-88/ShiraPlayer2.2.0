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

#include "channel.h"
#include "StelMainGraphicsView.hpp"
#include "StelApp.hpp"

Channel::Channel()
{
    m_name = "Channel";

    m_pDistWarp = new Warp;
    m_pDistWarp->setShowCtrlPoints(true);
    m_pDistWarp->setEnabled(true);
    //m_pDistWarp->setGridSize(12);
    //m_pDistWarp->reset();

    m_pBlendWarp = new Warp;
    m_pBlendWarp->setShowCtrlPoints(true);
    m_pBlendWarp->setEnabled(true);
    //m_pBlendWarp->reset();

    height = 600;
    width = 800;
    m_fwr = 1.0;
    m_fhr = 1.0;
    m_initfov = 60.0;
}
void Channel::setName(const QString& name)
{
    m_name = name;
}
QString Channel::getName()
{
    return m_name;
}

QDomElement Channel::domElement(const QString& name, QDomDocument& doc, bool withBlend) const
{
    QDomElement de = doc.createElement(name);
    de.setAttribute("name", m_name);
    de.setAttribute("initfov",m_initfov);
    de.setAttribute("initviewpos",QString("%1, %2, %3").arg(m_init_view_pos[0]).arg(m_init_view_pos[1]).arg(m_init_view_pos[2]));
    de.setAttribute("width",width);
    de.setAttribute("height",height);
    de.setAttribute("fwr",m_fwr);
    de.setAttribute("fhr",m_fhr);

    de.appendChild(m_pDistWarp->domElement("Distorter", doc));
    if (withBlend)
      de.appendChild(m_pBlendWarp->domElement("Blend", doc));

    return de;
}
bool Channel::initFromDOMElement(const QDomElement& element)
{
    if (!element.isNull())
    {
        m_name = element.attribute("name");
        m_initfov = element.attribute("initfov").toDouble();
        m_init_view_pos = stringtoVector(element.attribute("initviewpos"));
        width = element.attribute("width").toInt();
        height = element.attribute("height").toInt();
        m_fwr = element.attribute("fwr").toFloat();
        m_fhr = element.attribute("fhr").toFloat();

        if (!element.firstChildElement("Distorter").isNull())
            m_pDistWarp->initFromDOMElement(element.firstChildElement("Distorter"));
        if (!element.firstChildElement("Blend").isNull())
            m_pBlendWarp->initFromDOMElement(element.firstChildElement("Blend"));

    }
}

void Channel::setDistWarp(Warp* w)
{
    m_pDistWarp = w;
}

void Channel::setBlendWarp(Warp* w)
{
    m_pBlendWarp = w;
}

void Channel::setinitFov(double fov)
{
    m_initfov = fov;
}
 void Channel::setinitPos(Vec3d initpos)
 {
    m_init_view_pos = initpos;
 }

 Vec3d Channel::stringtoVector(QString vecstr)
 {
    QStringList strlist = vecstr.split(",");
    return Vec3d(strlist[0].toDouble(),strlist[1].toDouble(),strlist[2].toDouble());
 }

 Warp* Channel::getCurrentWarp() const
 {
     if(StelMainGraphicsView::getInstance().getwarpMode() == StelMainGraphicsView::distortionMode)
         return m_pDistWarp;
     return m_pBlendWarp;

 }

 void Channel::updateClientData()
 {
     QDomDocument doc;
     doc.appendChild(domElement("Channel", doc,true));
     StelApp::getInstance().getRsync()->sendChanges(RSYNC_COMMAND_CHANNEL, doc.toString(0));

 }

 void Channel::setWidth(int val)
 {
     if (val != width)
        width = val;
 }

 void Channel::setHeight(int val)
 {
     if(val != height )
         height = val;
 }

 int Channel::getWidth()
 {
     return width;
 }

 int Channel::getHeight()
 {
     return height;
 }

 void Channel::set_FWR(float fwr)
 {
     m_fwr = fwr;
 }

 void Channel::set_FHR(float fhr)
 {
     m_fhr = fhr;
 }
