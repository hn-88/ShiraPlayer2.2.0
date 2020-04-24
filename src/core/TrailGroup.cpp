/*
 * ShiraPlayer(TM)
 * Copyright (C) 2010 Fabien Chereau
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

#include "TrailGroup.hpp"
#include "StelNavigator.hpp"
#include "StelApp.hpp"
#include "StelPainter.hpp"
#include "StelObject.hpp"
#include "Planet.hpp"
#include "StelToneReproducer.hpp"
#include "StelMovementMgr.hpp"
#include "StarMgr.hpp"
#include "StelModuleMgr.hpp"
#include <QtOpenGL/QtOpenGL>
#include <QDebug>

TrailGroup::TrailGroup(float te) : timeExtent(te), opacity(1.f)
{
    j2000ToTrailNative=Mat4d::identity();
    j2000ToTrailNativeInverted=Mat4d::identity();
}

static QVector<Vec3d> vertexArray;
static QVector<Vec4f> colorArray;
static QVector<Vec4f> colorArrayBack;
void TrailGroup::draw(StelCore* core, StelPainter* sPainter)
{
    int lineBackwidth = 3;
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    float currentTime = core->getNavigator()->getJDay();
    if(allTrails[0].stelObject.data()->getType() =="Star")
        sPainter->setProjector(core->getProjection(core->getNavigator()->getAltAzModelViewMat()*j2000ToTrailNativeInverted));
    else
        sPainter->setProjector(core->getProjection(core->getNavigator()->getJ2000ModelViewMat()*j2000ToTrailNativeInverted));

    foreach (const Trail& trail, allTrails)
    {
        if(trail.stelObject.data()->getType() =="Star")
        {
            double limitTrailVal = 5.0;
            StarMgr* smgr = GETSTELMODULE(StarMgr);
            if (smgr->getFlagMagLevel())
                if (smgr->getlimitMagValue()< 5.0 )
                   limitTrailVal =  smgr->getlimitMagValue();
            if (trail.stelObject.data()->getVMagnitude(core->getNavigator())> limitTrailVal)
                continue;
        }
        Planet* hpl = dynamic_cast<Planet*>(trail.stelObject.data());
        if (hpl!=NULL)
        {
            // Avoid drawing the trails if the object is the home planet
            QString homePlanetName = hpl==NULL ? "" : hpl->getEnglishName();
            if (homePlanetName==StelApp::getInstance().getCore()->getNavigator()->getCurrentLocation().planetName)
                continue;
        }
        const QList<Vec3d>& posHistory = trail.posHistory;
        vertexArray.resize(posHistory.size());
        colorArray.resize(posHistory.size());
        colorArrayBack.resize(posHistory.size());
        for (int i=0;i<posHistory.size();++i)
        {
            float mag = 1.0;
            if(trail.stelObject.data()->getType() =="Star")
            {
                //----
                //const StelNavigator* nav = core->getNavigator();
                const StelToneReproducer* eye = core->getToneReproducer();

                // determine intensity
                float Mag1 = (double)rand()/((double)RAND_MAX+1)*6.75f - 3;
                float Mag2 = (double)rand()/((double)RAND_MAX+1)*6.75f - 3;
                float Mag = (Mag1 + Mag2)/2.0f;

                mag = (5. + Mag) / 256.0;
                if (mag>250) mag = mag - 256;

                float term1 = std::exp(-0.92103f*(mag + 12.12331f)) * 108064.73f;

                float cmag=1.f;
                float rmag;

                // Compute the equivalent star luminance for a 5 arc min circle and convert it
                // in function of the eye adaptation
                rmag = eye->adaptLuminanceScaled(term1);
                rmag = rmag/powf(core->getMovementMgr()->getCurrentFov(),0.85f)*500.f;

                // if size of star is too small (blink) we put its size to 1.2 --> no more blink
                // And we compensate the difference of brighteness with cmag
                if (rmag<1.2f) {
                    cmag=rmag*rmag/1.44f;
                }
                mag = cmag;  // assumes white
                //------------
            }

            float colorRatio = 1.f-(currentTime-times.at(i))/timeExtent;
            colorArray[i].set(trail.color[0], trail.color[1], trail.color[2], colorRatio*opacity*mag);
            if(trail.stelObject.data()->getType() =="Star")
                colorArrayBack[i].set(trail.color[0], trail.color[1], trail.color[2], 0.3*mag);
            vertexArray[i]=posHistory.at(i);
        }
        if(trail.stelObject.data()->getType() =="Star")
        {
            if (trail.stelObject.data()->getVMagnitude(core->getNavigator())< 0 )
                lineBackwidth = 6;
            sPainter->setVertexPointer(3, GL_DOUBLE, vertexArray.constData());
            sPainter->setColorPointer(4, GL_FLOAT, colorArray.constData());
            glLineWidth(1);
            sPainter->enableClientStates(true, false, true);
            sPainter->drawFromArray(StelPainter::LineStrip, vertexArray.size(), 0, true);
            sPainter->setColorPointer(4, GL_FLOAT, colorArrayBack.constData());
            glLineWidth(lineBackwidth);
            sPainter->drawFromArray(StelPainter::LineStrip, vertexArray.size(), 0, true);
            sPainter->enableClientStates(false);
            glLineWidth(1);
        }
        else
        {
            sPainter->setVertexPointer(3, GL_DOUBLE, vertexArray.constData());
            sPainter->setColorPointer(4, GL_FLOAT, colorArray.constData());
            sPainter->enableClientStates(true, false, true);
            sPainter->drawFromArray(StelPainter::LineStrip, vertexArray.size(), 0, true);
            sPainter->enableClientStates(false);
        }



    }
}

// Add 1 point to all the curves at current time and suppress too old points
void TrailGroup::update()
{
    StelNavigator* nav = StelApp::getInstance().getCore()->getNavigator();
    times.append(nav->getJDay());
    for (QList<Trail>::Iterator iter=allTrails.begin();iter!=allTrails.end();++iter)
    {
        if(iter->stelObject.data()->getType() =="Star")
            iter->posHistory.append(j2000ToTrailNative*iter->stelObject->getAltAzPos(nav));
        else
            iter->posHistory.append(j2000ToTrailNative*iter->stelObject->getJ2000EquatorialPos(nav));
    }
    if (nav->getJDay()-times.at(0)>timeExtent)
    {
        times.pop_front();
        for (QList<Trail>::Iterator iter=allTrails.begin();iter!=allTrails.end();++iter)
        {
            iter->posHistory.pop_front();
        }
    }
}

// Set the matrix to use to post process J2000 positions before storing in the trail
void TrailGroup::setJ2000ToTrailNative(const Mat4d& m)
{
    j2000ToTrailNative=m;
    j2000ToTrailNativeInverted=m.inverse();
}

void TrailGroup::addObject(const StelObjectP& obj, const Vec3f* col)
{
    allTrails.append(TrailGroup::Trail(obj, col==NULL ? obj->getInfoColor() : *col));
}

void TrailGroup::reset()
{
    times.clear();
    for (QList<Trail>::Iterator iter=allTrails.begin();iter!=allTrails.end();++iter)
    {
        iter->posHistory.clear();
    }
}
