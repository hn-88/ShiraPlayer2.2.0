/*
 * ShiraPlayer(TM)
 * Copyright (C) 2003 Fabien Chereau
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
#include "StelCore.hpp"
#include "StelNavigator.hpp"
#include "StelProjector.hpp"
#include "StelProjectorClasses.hpp"
#include "StelToneReproducer.hpp"
#include "StelSkyDrawer.hpp"
#include "StelApp.hpp"
#include "StelUtils.hpp"
#include "StelGeodesicGrid.hpp"
#include "StelMovementMgr.hpp"
#include "StelModuleMgr.hpp"
#include "StelPainter.hpp"

//ASAF
#include "LandscapeMgr.hpp"
#include "Landscape.hpp"
#include "StelTextureMgr.hpp"
#include "StelPainter.hpp"
#include "StelMainGraphicsView.hpp"
#include "StelMainWindow.hpp"
#include "StelLocaleMgr.hpp"
#include "StelObjectMgr.hpp"
#include "StelObject.hpp"
#include "SkyGui.hpp"
#include <QMessageBox>
#include "StelAppGraphicsWidget.hpp"
#include <GL/glext.h>
#include "ScreenImageMgr.hpp"
//ASAF

#include <QSettings>
#include <QtOpenGL/QtOpenGL>
#include <QSettings>
#include <QDebug>
#include <QMetaEnum>
#include <QtCore/qmath.h>

/*************************************************************************
 Constructor
*************************************************************************/
StelCore::StelCore() : navigation(NULL), movementMgr(NULL), geodesicGrid(NULL), currentProjectionType(ProjectionStereographic)
{
    toneConverter = new StelToneReproducer();

    QSettings* conf = StelApp::getInstance().getSettings();
    // Create and initialize the default projector params
    QString tmpstr = conf->value("projection/viewport").toString();
    currentProjectorParams.maskType = StelProjector::stringToMaskType(tmpstr);
    const int viewport_width = conf->value("projection/viewport_width", currentProjectorParams.viewportXywh[2]).toInt();
    const int viewport_height = conf->value("projection/viewport_height", currentProjectorParams.viewportXywh[3]).toInt();
    const int viewport_x = conf->value("projection/viewport_x", 0).toInt();
    const int viewport_y = conf->value("projection/viewport_y", 0).toInt();
    currentProjectorParams.viewportXywh.set(viewport_x,viewport_y,viewport_width,viewport_height);

    const double viewportCenterX = conf->value("projection/viewport_center_x",0.5*viewport_width).toDouble();
    const double viewportCenterY = conf->value("projection/viewport_center_y",0.5*viewport_height).toDouble();
    currentProjectorParams.viewportCenter.set(viewportCenterX, viewportCenterY);
    currentProjectorParams.viewportFovDiameter = conf->value("projection/viewport_fov_diameter", qMin(viewport_width,viewport_height)).toDouble();
    currentProjectorParams.flipHorz = conf->value("projection/flip_horz",false).toBool();
    currentProjectorParams.flipVert = conf->value("projection/flip_vert",false).toBool();

    currentProjectorParams.gravityLabels = true;//conf->value("viewing/flag_gravity_labels").toBool();
    //ASAF
    flagTuiDatetime = conf->value("tui/flag_show_tui_datetime",false).toBool();
    //flagTuiSelected = conf->value("tui/flag_show_tui_short_obj_info",false).toBool();
    flagTuiLocation = conf->value("tui/flag_show_tui_location",false).toBool();
    show_only_this_datetime = conf->value("tui/flag_only_this_datetime",false).toBool();
    show_only_this_location = conf->value("tui/flag_only_this_location",false).toBool();

    m_overlayFont.setFamily("Helvetica");
    m_overlayFont.setPixelSize(64);
    showchannel_name = false;
    showFPS = false;

    //Initial freeHand values
    freeHandPen =  QColor(0, 0, 255);
    freeHandSize = 4;
    freeHandOpacity = 1.0;

    //
    isConnectedFromTablet = false;
    isConnectedFromTabletFreeVersion = false;

    logoFader = NULL;
}


/*************************************************************************
 Destructor
*************************************************************************/
StelCore::~StelCore()
{
    delete navigation; navigation=NULL;
    delete toneConverter; toneConverter=NULL;
    delete geodesicGrid; geodesicGrid=NULL;
    delete skyDrawer; skyDrawer=NULL;
}

/*************************************************************************
 Load core data and initialize with default values
*************************************************************************/
void StelCore::init()
{
    QSettings* conf = StelApp::getInstance().getSettings();

    // StelNavigator
    navigation = new StelNavigator();
    navigation->init();

    movementMgr = new StelMovementMgr(this);
    movementMgr->init();
    currentProjectorParams.fov = movementMgr->getInitFov();
    StelApp::getInstance().getModuleMgr().registerModule(movementMgr);

    QString tmpstr = conf->value("projection/type", "stereographic").toString();
    setCurrentProjectionTypeKey(tmpstr);

    skyDrawer = new StelSkyDrawer(this);
    skyDrawer->init();

    //ASAF
    conf= StelApp::getInstance().getSettings();
    screen_count=conf->value("video/screen_count",0).toInt();
    screen_index=conf->value("video/screen_index",0).toInt();
    screen_w=conf->value("video/screen_w",0).toInt();
    screen_h=conf->value("video/screen_h",0).toInt();
    screen_l = conf->value("video/screen_l",0).toInt();

    ALLFader=new LinearFader();
    ALLFader.setDuration(100);
    ALLFader=true;

    //Tilt iþlemi iin

    tilt_angle = conf->value("navigation/tilt_angle",0).toFloat(0);

    initWarpGL();

    m_birdefa = true;

    //Date String için
    datestring_font.setPixelSize(conf->value("viewing/informations_font_size",20).toInt());
//#ifdef SHIRAPLAYER_PRE
    if(!StelMainWindow::getInstance().getIsMultiprojector())
        datestring_font.setPixelSize(20);
//#endif
    //FPS font
    m_FPSFont.setFamily("Helvetica");
    m_FPSFont.setPixelSize(16);

    //Unlicensed font
    StelTextureMgr& texMgr=StelApp::getInstance().getTextureManager();
    texLogo = texMgr.createTextureLogo(StelTexture::StelTextureParams(true),":/mainWindow/gui/logo_demo.png");
    //timerLogo.start();

    if (!StelMainWindow::getInstance().getIsServer())
    {
        ALLFader.setDuration(2000);
        ALLFader = false;
    }

    //All Sky Lines Width
    skyline_width = conf->value("viewing/skyline_width",1).toInt();
}

void StelCore::initWarpGL()
{
    StelMainGraphicsView::getInstance().saveScreenShot("","");
    QImage img  = StelMainGraphicsView::getInstance().imagescreen;
    QImage texture = QGLWidget::convertToGLFormat(img);

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, StelMainGraphicsView::getInstance().m_stellaTextureID);

    glTexImage2D(GL_TEXTURE_2D, 0, 3, StelMainGraphicsView::getInstance().getOpenGLWin()->width() ,
                 StelMainGraphicsView::getInstance().getOpenGLWin()->height(), 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, texture.bits());

    //data_size = 4 * sizeof(GLubyte) * StelMainGraphicsView::getInstance().getOpenGLWin()->height() * StelMainGraphicsView::getInstance().getOpenGLWin()->width();

}

// Get the shared instance of StelGeodesicGrid.
// The returned instance is garanteed to allow for at least maxLevel levels
const StelGeodesicGrid* StelCore::getGeodesicGrid(int maxLevel) const
{
    if (geodesicGrid==NULL)
    {
        geodesicGrid = new StelGeodesicGrid(maxLevel);
    }
    else if (maxLevel>geodesicGrid->getMaxLevel())
    {
        delete geodesicGrid;
        geodesicGrid = new StelGeodesicGrid(maxLevel);
    }
    return geodesicGrid;
}

StelProjectorP StelCore::getProjection2d() const
{
    StelProjectorP prj(new StelProjector2d());
    prj->init(currentProjectorParams);
    return prj;
}

// Get an instance of projector using the current display parameters from Navigation, StelMovementMgr
// and using the given modelview matrix
StelProjectorP StelCore::getProjection(const Mat4d& modelViewMat, ProjectionType projType) const
{
    if (projType==1000)
        projType = currentProjectionType;

    StelProjectorP prj;
    switch (projType)
    {
    case ProjectionPerspective:
        prj = StelProjectorP(new StelProjectorPerspective(modelViewMat));
        break;
    case ProjectionEqualArea:
        prj = StelProjectorP(new StelProjectorEqualArea(modelViewMat));
        break;
    case ProjectionStereographic:
        prj = StelProjectorP(new StelProjectorStereographic(modelViewMat));
        break;
    case ProjectionFisheye:
        prj = StelProjectorP(new StelProjectorFisheye(modelViewMat));
        break;
    case ProjectionHammer:
        prj = StelProjectorP(new StelProjectorHammer(modelViewMat));
        break;
    case ProjectionCylinder:
        prj = StelProjectorP(new StelProjectorCylinder(modelViewMat));
        break;
    case ProjectionMercator:
        prj = StelProjectorP(new StelProjectorMercator(modelViewMat));
        break;
    case ProjectionOrthographic:
        prj = StelProjectorP(new StelProjectorOrthographic(modelViewMat));
        break;
    default:
        qWarning() << "Unknown projection type: " << (int)(projType) << "using ProjectionStereographic instead";
        prj = StelProjectorP(new StelProjectorStereographic(modelViewMat));
        Q_ASSERT(0);
    }
    prj->init(currentProjectorParams);
    return prj;
}

// Get an instance of projector using the current display parameters from Navigation, StelMovementMgr
StelProjectorP StelCore::getProjection(FrameType frameType, ProjectionType projType) const
{
    switch (frameType)
    {
    case FrameAltAz:
        return getProjection(navigation->getAltAzModelViewMat(), projType);
    case FrameHeliocentricEcliptic:
        return getProjection(navigation->getHeliocentricEclipticModelViewMat(), projType);
    case FrameObservercentricEcliptic:
        return getProjection(navigation->getObservercentricEclipticModelViewMat(), projType);
    case FrameEquinoxEqu:
        return getProjection(navigation->getEquinoxEquModelViewMat(), projType);
    case FrameJ2000:
        return getProjection(navigation->getJ2000ModelViewMat(), projType);
    case FrameGalactic:
        return getProjection(navigation->getGalacticModelViewMat(), projType);
    case FrameScreen :
        return getProjection(navigation->getScreenViewMat(), projType);
    default:
        qDebug() << "Unknown reference frame type: " << (int)frameType << ".";
    }
    Q_ASSERT(0);
    return getProjection2d();
}

// Handle the resizing of the window
void StelCore::windowHasBeenResized(float x, float y, float width, float height)
{
    // Maximize display when resized since it invalidates previous options anyway
    currentProjectorParams.viewportXywh.set(x, y, width, height);
    currentProjectorParams.viewportCenter.set(x+0.5*width, y+0.5*height);
    currentProjectorParams.viewportFovDiameter = qMin(width,height);
}

/*************************************************************************
 Update all the objects in function of the time
*************************************************************************/
void StelCore::update(double deltaTime)
{
    // Update the position of observation and time and recompute planet positions etc...
    //if(!StelApp::getInstance().isVideoMode)
    navigation->updateTime(deltaTime);

    // Transform matrices between coordinates systems
    //if(!StelApp::getInstance().isVideoMode)
    navigation->updateTransformMatrices();

    // Update direction of vision/Zoom level
    //if(!StelApp::getInstance().isVideoMode)
    movementMgr->updateMotion(deltaTime);

    //if(!StelApp::getInstance().isVideoMode)
    currentProjectorParams.fov = movementMgr->getCurrentFov();

    //if(!StelApp::getInstance().isVideoMode)
    skyDrawer->update(deltaTime);
    //ASAF
    ALLFader.update((int)(deltaTime*1000));

}


/*************************************************************************
 Execute all the pre-drawing functions
*************************************************************************/
void StelCore::preDraw()
{
    // Init openGL viewing with fov, screen size and clip planes
    currentProjectorParams.zNear = 0.000001;
    currentProjectorParams.zFar = 50.;

    skyDrawer->preDraw();

    // Clear areas not redrawn by main viewport (i.e. fisheye square viewport)
    StelPainter sPainter(getProjection2d());
    glClearColor(0,0,0,0);
    glClear(GL_COLOR_BUFFER_BIT);

}
void StelCore::drawDistortBlend()
{
    if(!StelMainWindow::getInstance().getIsServer() || (StelMainWindow::getInstance().getIsServer()&& StelMainGraphicsView::getInstance().isClientConf))// Client veya (Server olup ClientConf seçili) ise
    {
        if(StelMainGraphicsView::getInstance().m_pSelectedChannel)
        {

            glPushAttrib(GL_ALL_ATTRIB_BITS);
            glPushMatrix();

            //texture = QGLWidget::convertToGLFormat(StelMainGraphicsView::getInstance().getOpenGLWin()->grabFrameBuffer());

            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, StelMainGraphicsView::getInstance().m_stellaTextureID);

            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, StelMainGraphicsView::getInstance().getOpenGLWin()->width() ,
                            StelMainGraphicsView::getInstance().getOpenGLWin()->height(),  GL_RGBA, GL_UNSIGNED_BYTE,
                            QGLWidget::convertToGLFormat(StelMainGraphicsView::getInstance().getOpenGLWin()->grabFrameBuffer()).bits());


            if(StelMainGraphicsView::getInstance().getwarpMode() == StelMainGraphicsView::distortionMode)
            {
                glBindTexture(GL_TEXTURE_2D, StelMainGraphicsView::getInstance().m_stellaTextureID);
            }else if (StelMainGraphicsView::getInstance().getwarpMode() == StelMainGraphicsView::blendMode)
            {
                glBindTexture(GL_TEXTURE_2D, StelMainGraphicsView::getInstance().getBlendingAreaTexture());
            }else //if (StelMainGraphicsView::getInstance().getwarpMode() == StelMainGraphicsView::showMode)
            {
#ifdef SHIRAPLAYER_PRE
                glBindTexture(GL_TEXTURE_2D, StelMainGraphicsView::getInstance().m_stellaTextureID);
#else
                if(m_birdefa)
                    glBindTexture(GL_TEXTURE_2D, StelMainGraphicsView::getInstance().m_stellaTextureID);
                else
                    glBindTexture(GL_TEXTURE_2D, StelMainGraphicsView::getInstance().getBlendingAreaTexture());
#endif
            }

            glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            if(StelMainGraphicsView::getInstance().getwarpMode() == StelMainGraphicsView::distortionMode)
            {
                StelMainGraphicsView::getInstance().m_pSelectedChannel->getDistWarp()->draw(false, false);
            }else if (StelMainGraphicsView::getInstance().getwarpMode() == StelMainGraphicsView::blendMode)
            {
                StelMainGraphicsView::getInstance().m_pSelectedChannel->getBlendWarp()->draw(false,true);
            }else //if (StelMainGraphicsView::getInstance().getwarpMode() == StelMainGraphicsView::showMode)
            {
                if(m_birdefa)
                {
#ifndef SHIRAPLAYER_PRE
                    StelMainGraphicsView::getInstance().m_pSelectedChannel->getDistWarp()->draw(false, false);
#endif
                    m_birdefa = false;
                }
                else
                {
#ifndef SHIRAPLAYER_PRE
                    StelMainGraphicsView::getInstance().m_pSelectedChannel->getBlendWarp()->draw(true,true);
                    glBindTexture(GL_TEXTURE_2D, StelMainGraphicsView::getInstance().m_stellaTextureID);
                    glEnable(GL_BLEND);
                    glBlendFunc(GL_DST_COLOR, GL_ZERO);
#endif
                    glBindTexture(GL_TEXTURE_2D, StelMainGraphicsView::getInstance().m_stellaTextureID);
                    StelMainGraphicsView::getInstance().m_pSelectedChannel->getDistWarp()->draw(true, false);
                }
            }

            glPopMatrix();
            glPopAttrib();
            glFlush();
            ////



        }
    }
}

/*************************************************************************
 Update core state after drawing modules
*************************************************************************/
void StelCore::postDraw()
{
    StelProjectorP prj=getProjection(StelCore::FrameJ2000);
    StelPainter sPainter(prj);
    sPainter.drawViewportShape();

    if(!StelMainWindow::getInstance().getIsServer() || (StelMainWindow::getInstance().getIsServer()&& StelMainGraphicsView::getInstance().isClientConf))// Client veya (Server olup ClientConf seçili) ise
    {
        if(StelMainGraphicsView::getInstance().m_pSelectedChannel)
        {
            //Channel no ekrana yazýlýyor
            if(showchannel_name)
            {
                StelPainter sPainter1(getProjection2d());
                sPainter1.setFont(m_overlayFont);
                QFontMetrics fontMet(m_overlayFont);
                glColor3f(0.5,1,0.5);
                sPainter1.drawText((StelMainGraphicsView::getInstance().getOpenGLWin()->width()-fontMet.width(StelMainGraphicsView::getInstance().m_pSelectedChannel->getName()))/2,
                                   (StelMainGraphicsView::getInstance().getOpenGLWin()->height()+fontMet.height()/2)/2,
                                   StelMainGraphicsView::getInstance().m_pSelectedChannel->getName(),0,0,0,false);
            }


            if(!StelApp::getInstance().isVideoMode && !StelApp::getInstance().isVideoLandscape)
            {
                StelProjectorP prj=getProjection(StelCore::FrameJ2000);
                StelPainter sPainter(prj);

                //ASAF Tarih ve Seçilen nesnenin yazýlmasý
                Vec2f center = prj->getViewportCenter();
                float x = center[0];
                float y = center[1];
                float shift = 0.5*prj->getViewportHeight(); // viewport radius

                sPainter.setFont(datestring_font);

                if(flagTuiDatetime && show_only_this_datetime )
                {
                    double jd = getNavigator()->getJDay();

                    QString strDateTime =  StelApp::getInstance().getLocaleMgr().getPrintableDateLocal(jd) +" "
                            +StelApp::getInstance().getLocaleMgr().getPrintableTimeLocal(jd) ;

                    glColor3f(0.77,0.22,0.1);

                    //sPainter.drawText(x - shift + 30, y - 30, strDateTime,0,0,0, false);// Eski çözümde bunu kullanmýþtým
                    sPainter.drawText(x+ shift-200,20,strDateTime,0,0,0, true);
                }
                if(flagTuiLocation && show_only_this_location)
                {
                    QString strDateTime = "Enl:"+QString("%0").arg(StelApp::getInstance().getCore()->getNavigator()->getCurrentLocation().latitude)+" "
                            +"Boyl:"+QString("%0").arg(StelApp::getInstance().getCore()->getNavigator()->getCurrentLocation().longitude)+" "
                            +StelApp::getInstance().getCore()->getNavigator()->getCurrentLocation().name+" "
                            +StelApp::getInstance().getCore()->getNavigator()->getCurrentLocation().country;

                    glColor3f(0.77,0.22,0.1);

                    //sPainter.drawText(x - shift + 30, y - 30, strDateTime,0,0,0, false);// Eski çözümde bunu kullanmýþtým
                    sPainter.drawText(prj->getViewportPosX()+ 100,20,strDateTime,0,0,0, true);

                }
                //                    if(flagTuiSelected)
                //                    {
                //                        if(StelApp::getInstance().getStelObjectMgr().getSelectedObject().count()>0)
                //                        {
                //                            StelObjectP o= StelApp::getInstance().getStelObjectMgr().getSelectedObject()[0];
                //                            QString s = o->getInfoString(StelApp::getInstance().getCore(),(StelObject::InfoStringGroup)(StelObject::Name|StelObject::Magnitude|StelObject::Distance|StelObject::PlainText));
                //
                //                            glColor3fv(StelApp::getInstance().getStelObjectMgr().getSelectedObject()[0]->getInfoColor());
                //                            sPainter.drawText(x + shift - 30, y - 30,s.replace("\n"," "),0,0,0, false);
                //                        }
                //                    }
            }
        }
    }


    //#ifdef SHIRAPLAYER_PRE
    if(!StelApp::getInstance().isVideoMode && !StelApp::getInstance().isVideoLandscape)
    {
        if (flagTuiDatetime || flagTuiLocation || StelApp::getInstance().showPropGui)
        {
            StelProjectorP prj=getProjection(StelCore::FrameJ2000);
            StelPainter sPainter(prj);

            //Tarih ve Seçilen nesnenin yazýlmasý
            Vec2f center = prj->getViewportCenter();
            float x = center[0];
            float y = center[1];
            float shift = 0.5*prj->getViewportHeight(); // viewport radius

            sPainter.setFont(datestring_font);

            if(flagTuiDatetime )
            {
                double jd = getNavigator()->getJDay();

                QString strDateTime =  StelApp::getInstance().getLocaleMgr().getPrintableDateLocal(jd) +" "
                        +StelApp::getInstance().getLocaleMgr().getPrintableTimeLocal(jd) ;

                glColor3f(0.77,0.22,0.1);
                prj->gravityLabels = true;
                sPainter.drawText(x+ shift-prj->getViewportHeight()/4.0,prj->getViewportHeight()/10.0,strDateTime,0,0,0, false);
            }
            if(flagTuiLocation)
            {
                QString strDateTime = q_("RA ")+QString("%0").arg(StelApp::getInstance().getCore()->getNavigator()->getCurrentLocation().latitude)+" "
                        +q_("Dec ")+QString("%0").arg(StelApp::getInstance().getCore()->getNavigator()->getCurrentLocation().longitude)+" "
                        +StelApp::getInstance().getCore()->getNavigator()->getCurrentLocation().name+" "
                        +StelApp::getInstance().getCore()->getNavigator()->getCurrentLocation().country;

                glColor3f(0.77,0.22,0.1);
                prj->gravityLabels = true;
                //sPainter.drawText(prj->getViewportPosX()+ prj->getViewportHeight()/8.0,prj->getViewportHeight()/2.0,strDateTime,0,0,0, false);
                sPainter.drawText(x- shift*0.92,prj->getViewportHeight()/2.5,strDateTime,0,0,0, false);
            }
            if(StelApp::getInstance().showPropGui)
            {
                if(StelApp::getInstance().getStelObjectMgr().getSelectedObject().count()>0)
                {
                    StelObjectP o= StelApp::getInstance().getStelObjectMgr().getSelectedObject()[0];
                    QString s = o->getInfoString(StelApp::getInstance().getCore(),(StelObject::InfoStringGroup)(StelObject::Name|StelObject::Magnitude|StelObject::Distance|StelObject::PlainText));

                    glColor3fv(StelApp::getInstance().getStelObjectMgr().getSelectedObject()[0]->getInfoColor());
                    prj->gravityLabels = true;
                    sPainter.drawText(x + shift - 30, y - 30,s.replace("\n"," "),0,0,0, false);
                }
            }
        }
    }

    unregisteredShow();
    //#endif




    //ASAF fade-in fade-out efekti için
    //   if(!StelApp::getInstance().isServer) //Client ise
    {
        if (!ALLFader.getInterstate()) return;

        //        int h = screen_h ;//StelMainWindow::getInstance().height(); //size().width();
        //        int w = screen_w; //StelMainWindow::getInstance().width();// size().height();

        glPushAttrib(GL_ALL_ATTRIB_BITS);
        glDisable(GL_TEXTURE_2D);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_BLEND);
        // Set fadingcolor
        glColor4f(0,0,0,ALLFader.getInterstate());
        // Draw your screen-filling quad for fading
        glBegin(GL_QUADS);
        if (StelMainWindow::getInstance().getIsMultiprojector())
        {
            int wh = StelApp::getInstance().viewportRes;
            glVertex3f(0 , wh , 0);
            glVertex3f(wh , wh  , 0);
            glVertex3f(wh , 0 , 0);
            glVertex3f(-wh , -wh  , 0);
        }
        else
        {
            glVertex3f(0 , screen_h  , 0);
            glVertex3f(screen_w , screen_h  , 0);
            glVertex3f(screen_w , 0 , 0);
            glVertex3f(-screen_w , -screen_h  , 0);
        }
        glEnd();
        glDisable(GL_BLEND);
        glEnable(GL_TEXTURE_2D);
        glPopAttrib();
    }
    //ASAF


}

int StelCore::randInt(int low, int high)
{
    // Random number between low and high
    return qrand() % ((high + 1) - low) + low;
}

void StelCore::unregisteredShow()
{
    if (!StelMainWindow::getInstance().getIsMultiprojector())
        if (StelMainWindow::getInstance().getIsServer()) return;

    if( (!StelMainWindow::getInstance().is_Licenced) &&
            ((StelApp::getInstance().isVideoMode) ||
             (freehandItems.count() >0 ) ||
             ((StelMainGraphicsView::getInstance().m_pChannels.count()>0) &&
              (StelMainGraphicsView::getInstance().getwarpMode() == StelMainGraphicsView::showMode)) ||
             isConnectedFromTablet ||
             StelApp::getInstance().getstartedFlyby() ||
             isConnectedFromTabletFreeVersion  ||
             (StelApp::getInstance().messierList.count() > 0) ||
             StelApp::getInstance().usingConstModule ) ||
             (StelMainWindow::getInstance().getIsMultiprojector() && (!StelMainWindow::getInstance().getIsPluginRegistered()) )
            )

    {
        //if (timerLogo.elapsed() > 10000)  // 30sn sonra görüntülenecek
        {
            if(texLogo!= NULL)
            {
                if (texLogo->bind())
                {
                    StelProjectorP prj=getProjection(StelCore::FrameJ2000);
                    StelPainter sPainter(prj);

                    if (logoFader == NULL)
                    {
                        logoFader = new QTimeLine(5000, this);
                        logoFader->start();
                        logoRotation = 0;
                        //Vec2f center = prj->getViewportCenter();
                        xLogo = randInt((prj->getViewportWidth()-prj->getViewportHeight())/2 ,(prj->getViewportWidth()-prj->getViewportHeight())/2+ prj->getViewportHeight());
                        //center[0];
                        //qDebug()<<"xlogo:"<<xLogo;
                        yLogo = randInt(0,prj->getViewportHeight());
                        //center[1];
                        xplus = 2;
                        yplus = 2;
                    }

                    if (logoFader != NULL)
                    {

                        logoRotation = logoRotation+2;

                        if ( (xLogo > (prj->getViewportWidth()-prj->getViewportHeight())/2+prj->getViewportHeight()) && (xplus >0) )
                            xplus = -2;
                        else if ((xLogo < (prj->getViewportWidth()-prj->getViewportHeight())/2) && (xplus <0) )
                            xplus = 2;
                        xLogo = xLogo + xplus;

                        if ( (yLogo > prj->getViewportHeight()) && (yplus >0) )
                            yplus = -2;
                        else if ((yLogo < 0) && (yplus <0) )
                            yplus = 2;
                        yLogo = yLogo + yplus;

                        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // Normal transparency mode
                        glEnable(GL_BLEND);
                        sPainter.setColor(1.f, 1.f, 1.f,logoFader->currentValue()/2);
                        sPainter.enableTexture2d(true);
                        //sPainter.drawSprite2dMode(xLogo, yLogo-prj->getViewportWidth()/8, prj->getViewportWidth()/6,logoRotation);
                        sPainter.drawSprite2dMode(xLogo, yLogo, prj->getViewportWidth()/10,logoRotation);
                        sPainter.enableTexture2d(false);
                        glDisable(GL_BLEND);
                    }

                }
            }
        }
    }
}


void StelCore::displayFramerate()
{
    LandscapeMgr* lmgr = GETSTELMODULE(LandscapeMgr);
    float rate = 30;
    if (lmgr)
    {
        if(lmgr->vop_curr)
            rate =lmgr->vop_curr->getFrameRate();
    }
    StelProjectorP prj = getProjection(StelCore::FrameScreen);
    StelPainter pa(prj);

    float f_p_s_ = StelApp::getInstance().getFps();
    float x = prj->getViewportPosX()+10;
    float y = prj->getViewportHeight()-25;

    pa.setFont(m_FPSFont);
    f_p_s_ >= rate*28/30 ? glColor3fv(Vec3f(0,1,0)) : (f_p_s_ > rate*25/30 ? glColor3fv(Vec3f(1,1,0)) : glColor3fv(Vec3f(1,0,0)));

    prj->gravityLabels = false;
    pa.drawText(x,y,QString("FPS : %1").arg(QString::number(f_p_s_,'f',0)),0,0,true);
}

void StelCore::drawPaintFree()
{
    StelProjectorP prj=getProjection(StelCore::FrameScreen);
    StelPainter sPainter(prj);

    for (int i=0; i < freehandItems.count();i++)
    {
        sPainter.DrawStrokePath(freehandItems[i]->getPath(),
                                freehandItems[i]->getPen(),
                                freehandItems[i]->getSize(),
                                freehandItems[i]->getOpacity());
    }
    //freeHandPen.setWidth(freeHandSize);
    sPainter.DrawStrokePath(path, freeHandPen, freeHandSize,freeHandOpacity);
}

//! Set the current projection type to use
void StelCore::setCurrentProjectionTypeKey(QString key)
{
    const QMetaEnum& en = metaObject()->enumerator(metaObject()->indexOfEnumerator("ProjectionType"));
    currentProjectionType = (ProjectionType)en.keyToValue(key.toLatin1().data());
    if (currentProjectionType<0)
    {
        qWarning() << "Unknown projection type: " << key << "setting \"ProjectionStereographic\" instead";
        currentProjectionType = ProjectionStereographic;
    }
    const double savedFov = currentProjectorParams.fov;
    currentProjectorParams.fov = 0.0001;	// Avoid crash
    double newMaxFov = getProjection(Mat4d())->getMaxFov();
    movementMgr->setMaxFov(newMaxFov);
    currentProjectorParams.fov = qMin(newMaxFov, savedFov);
}

//! Get the current Mapping used by the Projection
QString StelCore::getCurrentProjectionTypeKey(void) const
{
    return metaObject()->enumerator(metaObject()->indexOfEnumerator("ProjectionType")).key(currentProjectionType);
}

//! Get the list of all the available projections
QStringList StelCore::getAllProjectionTypeKeys() const
{
    const QMetaEnum& en = metaObject()->enumerator(metaObject()->indexOfEnumerator("ProjectionType"));
    QStringList l;
    for (int i=0;i<en.keyCount();++i)
        l << en.key(i);
    return l;
}

//! Get the translated projection name from its TypeKey for the current locale
QString StelCore::projectionTypeKeyToNameI18n(const QString& key) const
{
    const QMetaEnum& en = metaObject()->enumerator(metaObject()->indexOfEnumerator("ProjectionType"));
    QString s(getProjection(Mat4d(), (ProjectionType)en.keysToValue(key.toLatin1()))->getNameI18());
    return s;
}

//! Get the projection TypeKey from its translated name for the current locale
QString StelCore::projectionNameI18nToTypeKey(const QString& nameI18n) const
{
    const QMetaEnum& en = metaObject()->enumerator(metaObject()->indexOfEnumerator("ProjectionType"));
    for (int i=0;i<en.keyCount();++i)
    {
        if (getProjection(Mat4d(), (ProjectionType)i)->getNameI18()==nameI18n)
            return en.valueToKey(i);
    }
    // Unknown translated name
    Q_ASSERT(0);
    return en.valueToKey(ProjectionStereographic);
}

void StelCore::setShowFPS(int value)
{
    showFPS = value;
}
