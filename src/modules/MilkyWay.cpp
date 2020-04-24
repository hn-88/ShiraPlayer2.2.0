/*
 * ShiraPlayer(TM)
 * Copyright (C) 2002 Fabien Chereau
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

#include "MilkyWay.hpp"
#include "StelFader.hpp"
#include "StelTexture.hpp"
#include "StelUtils.hpp"
#include "StelNavigator.hpp"
#include "StelProjector.hpp"
#include "StelToneReproducer.hpp"
#include "StelApp.hpp"
#include "StelTextureMgr.hpp"
#include "StelCore.hpp"
#include "StelSkyDrawer.hpp"
#include "StelPainter.hpp"
#include "StelMainWindow.hpp"


#include <QDebug>
#include <QSettings>

// Class which manages the displaying of the Milky Way
MilkyWay::MilkyWay() : color(1.f, 1.f, 1.f)
{
    setObjectName("MilkyWay");
    fader = new LinearFader();
}

MilkyWay::~MilkyWay()
{
    delete fader;
    fader = NULL;
}

void MilkyWay::init()
{
    QSettings* conf = StelApp::getInstance().getSettings();
    Q_ASSERT(conf);

    if (StelMainWindow::getInstance().getIsServer())
    {
        if(!StelMainWindow::getInstance().getIsMultiprojector())
            tex = StelApp::getInstance().getTextureManager().createTexture("textures/milkyway_small.png");
        else
            tex = StelApp::getInstance().getTextureManager().createTexture("textures/milkyway.png");
    }
    else
        tex = StelApp::getInstance().getTextureManager().createTexture("textures/milkyway.png");

    setFlagShow(conf->value("astro/flag_milky_way").toBool());
    setIntensity(conf->value("astro/milky_way_intensity",1.).toDouble());
}


void MilkyWay::update(double deltaTime) {fader->update((int)(deltaTime*1000));}

void MilkyWay::setFlagShow(bool b){*fader = b; StelApp::getInstance().addNetworkCommand("MilkyWayMgr.setFlagShow("+ QString("%0").arg(b) +");");}
bool MilkyWay::getFlagShow() const {return *fader;}

void MilkyWay::draw(StelCore* core)
{
    StelNavigator* nav = core->getNavigator();
    const StelProjectorP prj = core->getProjection(nav->getJ2000ModelViewMat()*
                                                   Mat4d::xrotation(M_PI/180*23)*
                                                   Mat4d::yrotation(M_PI/180*120)*
                                                   Mat4d::zrotation(M_PI/180*7));
    StelToneReproducer* eye = core->getToneReproducer();

    Q_ASSERT(tex);	// A texture must be loaded before calling this

    // This RGB color corresponds to the night blue scotopic color = 0.25, 0.25 in xyY mode.
    // since milky way is always seen white RGB value in the texture (1.0,1.0,1.0)
    Vec3f c;
    //	if (StelApp::getInstance().getVisionModeNight())
    //		c = Vec3f(0.34165, 0, 0);
    //	else
    c = Vec3f(0.34165, 0.429666, 0.63586);

    float lum = core->getSkyDrawer()->surfacebrightnessToLuminance(13.5);

    // Get the luminance scaled between 0 and 1
    float aLum =eye->adaptLuminanceScaled(lum*fader->getInterstate());

    // Bound a maximum luminance
    aLum = qMin(0.38, aLum*2.);

    // intensity of 1.0 is "proper", but allow boost for dim screens
    c*=aLum*intensity;

    if (c[0]<0) c[0]=0;
    if (c[1]<0) c[1]=0;
    if (c[2]<0) c[2]=0;

    StelPainter sPainter(prj);
    sPainter.setColor(c[0],c[1],c[2]);
    glEnable(GL_CULL_FACE);
    sPainter.enableTexture2d(true);
    glDisable(GL_BLEND);
    tex->bind();
    sPainter.sSphere(1.,1.0,20,20,1);
    glDisable(GL_CULL_FACE);
}
