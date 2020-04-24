/*
 * ShiraPlayer(TM)
 * Copyright (C) 2005 Fabien Chereau
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

#include "StelProjector.hpp"
#include "StelLoadingBar.hpp"
#include "StelApp.hpp"
#include "StelTextureMgr.hpp"
#include "StelLocaleMgr.hpp"
#include "StelMainGraphicsView.hpp"
#include "StelMainWindow.hpp"

#include "StelPainter.hpp"
#include "StelCore.hpp"
#include "shiraplayerform.hpp"
#include "licenceutils/qlisansform.h"

#include <QDebug>
#include <QtOpenGL/QtOpenGL>


StelLoadingBar::StelLoadingBar(float fontSize, const QString&  splashTex,
                               const QString& extraTextString, float extraTextSize,
                               float extraTextPosx, float extraTextPosy) :
    width(512), height(512), extraText(extraTextString), sPainter(NULL)
{
    extraTextFont.setPixelSize(extraTextSize);
    LicenceTextFont.setPixelSize(12);
    versionTextFont.setPixelSize(20);

    sPainter = new StelPainter(StelApp::getInstance().getCore()->getProjection2d());
    int screenw = sPainter->getProjector()->getViewportWidth();
    int screenh = sPainter->getProjector()->getViewportHeight();
    splashx = sPainter->getProjector()->getViewportPosX() + (screenw - width)/2;
    splashy = sPainter->getProjector()->getViewportPosY() + (screenh - height)/2;
    if (!splashTex.isEmpty())
        splash = StelApp::getInstance().getTextureManager().createTexture(splashTex);
    extraTextPos.set(extraTextPosx, extraTextPosy);
    timeCounter = StelApp::getInstance().getTotalRunTime();

    QSettings settings("Sureyyasoft", "ShiraPlayer");
    settings.beginGroup("Licenses");
    dt = settings.value("record_date",QDateTime::currentDateTime()).toDate();
    uname =settings.value("user","").toString();
    lcode =settings.value("license_code","").toString();
    settings.endGroup();

    QLisansForm frm;
    if ( QString::compare(lcode,frm.KodOlustur(dt,uname),Qt::CaseInsensitive) == 0)
        StelMainWindow::getInstance().is_Licenced = true;
    else
        StelMainWindow::getInstance().is_Licenced = false;



}

StelLoadingBar::~StelLoadingBar()
{
    delete sPainter;
}

void StelLoadingBar::Draw(float val)
{
    glClearColor(1.0f,1.0f,1.0f,0.0f);
    sPainter->setColor(1.f, 1.f, 1.f,0.5f);
    sPainter->enableTexture2d(true);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Draw the splash screen if available"
    if (splash)
    {
        splash->bind();
        sPainter->drawRect2d(splashx, splashy, width, height);
    }

    //sPainter->setFont(extraTextFont);
    //sPainter->drawText(splashx + extraTextPos[0], splashy + extraTextPos[1]-sPainter->getFontMetrics().height()-1, extraText);

    sPainter->setFont(LicenceTextFont);
    if(StelMainWindow::getInstance().is_Licenced)
        sPainter->drawText(-20+splashx + extraTextPos[0], -75+splashy + extraTextPos[1]-sPainter->getFontMetrics().height()-1, "Licensed User: "+ uname);
    //sPainter->drawText(-20+splashx + extraTextPos[0], -88+splashy + extraTextPos[1]-sPainter->getFontMetrics().height()-1, "Licenced Date: "+ dt.toString(Qt::DefaultLocaleShortDate));
    else
        sPainter->drawText(-20+splashx + extraTextPos[0], -75+splashy + extraTextPos[1]-sPainter->getFontMetrics().height()-1, "UnLicensed User");

    sPainter->setFont(versionTextFont);
    sPainter->drawText(-194+splashx + extraTextPos[0], -75+splashy + extraTextPos[1]-sPainter->getFontMetrics().height()-1, "ver: "+ extraText);


    //StelMainGraphicsView::getInstance().swapBuffer();	// And swap the buffers


}
