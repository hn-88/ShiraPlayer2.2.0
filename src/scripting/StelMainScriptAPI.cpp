/*
 * ShiraPlayer(TM)
 * Copyright (C) 2002 Fabien Chereau
 * Copyright (C) 2007 Fabien Chereau
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


#include "StelMainScriptAPI.hpp"
#include "StelMainScriptAPIProxy.hpp"
#include "StelScriptMgr.hpp"

#include "ConstellationMgr.hpp"
#include "GridLinesMgr.hpp"
#include "LandscapeMgr.hpp"
#include "MeteorMgr.hpp"
#include "NebulaMgr.hpp"
#include "Planet.hpp"
#include "SolarSystem.hpp"
#include "StarMgr.hpp"
#include "StelApp.hpp"
#include "StelAudioMgr.hpp"
#include "StelCore.hpp"
#include "StelFileMgr.hpp"
#include "StelLocation.hpp"
#include "StelLocationMgr.hpp"
#include "StelMainGraphicsView.hpp"
#include "StelModuleMgr.hpp"
#include "StelMovementMgr.hpp"
#include "StelNavigator.hpp"
#include "StelObject.hpp"
#include "StelObjectMgr.hpp"
#include "StelProjector.hpp"
#include "StelSkyCultureMgr.hpp"
#include "StelSkyDrawer.hpp"
#include "StelSkyLayerMgr.hpp"
#include "StelUtils.hpp"
#include "StelGuiBase.hpp"
#include "videoutils/audioclass.h"
#include "StelPresentMgr.hpp"


#include <QAction>
#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QRegExp>
#include <QSet>
#include <QStringList>
#include <QTemporaryFile>

#include <cmath>

StelMainScriptAPI::StelMainScriptAPI(QObject *parent) : QObject(parent)
{
    if(StelSkyLayerMgr* smgr = GETSTELMODULE(StelSkyLayerMgr))
    {
        connect(this, SIGNAL(requestLoadSkyImage(const QString&, const QString&, double, double, double, double, double, double, double, double, double, double, bool)), smgr, SLOT(loadSkyImage(const QString&, const QString&, double, double, double, double, double, double, double, double, double, double, bool)));
        connect(this, SIGNAL(requestRemoveSkyImage(const QString&)), smgr, SLOT(removeSkyLayer(const QString&)));
    }

   /* connect(this, SIGNAL(requestLoadSound(const QString&, const QString&)), StelApp::getInstance().getStelAudioMgr(), SLOT(loadSound(const QString&, const QString&)));
    connect(this, SIGNAL(requestPlaySound(const QString&)), StelApp::getInstance().getStelAudioMgr(), SLOT(playSound(const QString&)));
    connect(this, SIGNAL(requestPauseSound(const QString&)), StelApp::getInstance().getStelAudioMgr(), SLOT(pauseSound(const QString&)));
    connect(this, SIGNAL(requestStopSound(const QString&)), StelApp::getInstance().getStelAudioMgr(), SLOT(stopSound(const QString&)));
    connect(this, SIGNAL(requestDropSound(const QString&)), StelApp::getInstance().getStelAudioMgr(), SLOT(dropSound(const QString&)));
    */
    connect(this, SIGNAL(requestExit()), this->parent(), SLOT(stopScript()));
    connect(this, SIGNAL(requestSetNightMode(bool)), &StelApp::getInstance(), SLOT(setVisionModeNight(bool)));
    connect(this, SIGNAL(requestSetProjectionMode(QString)), StelApp::getInstance().getCore(), SLOT(setCurrentProjectionTypeKey(QString)));
    connect(this, SIGNAL(requestSetSkyCulture(QString)), &StelApp::getInstance().getSkyCultureMgr(), SLOT(setCurrentSkyCultureID(QString)));
    connect(this, SIGNAL(requestSetDiskViewport(bool)), StelMainGraphicsView::getInstance().getMainScriptAPIProxy(), SLOT(setDiskViewport(bool)));
    //ASAF
    connect(this, SIGNAL(requestLoadVideo(const QString&)),StelMainGraphicsView::getInstance().getMainScriptAPIProxy(),SLOT(loadVideo(const QString&)));
    connect(this, SIGNAL(requestPlayVideo(bool)),StelMainGraphicsView::getInstance().getMainScriptAPIProxy(),SLOT(playVideo(bool)));
    connect(this, SIGNAL(requestPauseVideo(bool)),StelMainGraphicsView::getInstance().getMainScriptAPIProxy(),SLOT(pauseVideo(bool)));
    connect(this, SIGNAL(requestStopVideo()),StelMainGraphicsView::getInstance().getMainScriptAPIProxy(),SLOT(stopVideo()));
    connect(this, SIGNAL(requestPlayAudio(const QString&)),StelMainGraphicsView::getInstance().getMainScriptAPIProxy(),SLOT(playAudio(const QString&)));
    connect(this, SIGNAL(requestStopAudio()),StelMainGraphicsView::getInstance().getMainScriptAPIProxy(),SLOT(stopAudio()));

    connect(this,SIGNAL(requestPrepareFlyBy()),StelMainGraphicsView::getInstance().getMainScriptAPIProxy(),SLOT(prepareFlyBy()));
    connect(this,SIGNAL(requestSetFlyBy(QString,bool)),StelMainGraphicsView::getInstance().getMainScriptAPIProxy(),SLOT(setFlyBy(QString,bool)));
    connect(this,SIGNAL(requestGoHome()),StelMainGraphicsView::getInstance().getMainScriptAPIProxy(),SLOT(goHome()));

    if(StelPresentMgr* spmgr = GETSTELMODULE(StelPresentMgr))
    {
        connect(this, SIGNAL(requestLoadPresentImage(const QString&, const QString&, double, double, double, double, double, double, double, double, double, double, bool,bool)), spmgr, SLOT(loadPresentImage(const QString&, const QString&, double, double, double, double, double, double, double, double, double, double, bool,bool)));
        connect(this, SIGNAL(requestLoadNewPresentImage(const QString&, const QString&,double,double,double,double,double,double,bool,bool,double)),spmgr,SLOT(loadPresentImage(const QString&, const QString&,double,double,double,double,double,double,bool,bool,double)));
        connect(this, SIGNAL(requestRemovePresentImage(const QString&)), spmgr, SLOT(removePresentImage(const QString&)));

        connect(this,SIGNAL(requestShowPresentImage(const QString&,bool)),spmgr,SLOT(showPresentImage(QString,bool)));

        connect(this,SIGNAL(requestMixPresentImage(const QString&,bool)),spmgr,SLOT(setMixWithSky(QString,bool)));

        connect(this,SIGNAL(requestSetPresentProperties(const QString&,const QString&)),
                            spmgr,
                            SLOT(setLayerProperties(const QString&,const QString&)));

        connect(this,SIGNAL(requestSetLayerdomeORsky(const QString&,bool)),spmgr,SLOT(setLayerdomeORsky(const QString&,bool)));
        connect(this,SIGNAL(requestPVideoContrast(int,double)),spmgr,SLOT(setPVideoContrast(int,double)));
        connect(this,SIGNAL(requestPVideoBrightness(int,double)),spmgr,SLOT(setPVideoBrightness(int,double)));
        connect(this,SIGNAL(requestPVideoSaturation(int,double)),spmgr,SLOT(setPVideoSaturation(int,double)));

        connect(this, SIGNAL(requestLoadNewPresentVideo(QString,QString,double,double,double,double,double,double,bool,bool,double)),spmgr,SLOT(loadPresentVideo(QString,QString,double,double,double,double,double,double,bool,bool,double)));
        connect(this,SIGNAL(requestPlayPresentVideo(QString,bool)),spmgr,SLOT(playPresentVideo(QString,bool)));
        connect(this,SIGNAL(requestPauseTogglePresentVideo(QString)),spmgr,SLOT(pauseTogglePresentVideo(QString)));
        connect(this,SIGNAL(requestRemovePresentVideo(QString)),spmgr,SLOT(removePresentVideo(QString)));
    }

}


StelMainScriptAPI::~StelMainScriptAPI()
{
}

ScriptSleeper& StelMainScriptAPI::getScriptSleeper(void)
{
    return scriptSleeper;
}

//! Set the current date in Julian Day
//! @param JD the Julian Date
void StelMainScriptAPI::setJDay(double JD)
{
    StelApp::getInstance().getCore()->getNavigator()->setJDay(JD);
}

//! Get the current date in Julian Day
//! @return the Julian Date
double StelMainScriptAPI::getJDay(void) const
{
    return StelApp::getInstance().getCore()->getNavigator()->getJDay();
}

void StelMainScriptAPI::setDate(const QString& dt, const QString& spec)
{
    StelApp::getInstance().getCore()->getNavigator()->setJDay(jdFromDateString(dt, spec));
}

QString StelMainScriptAPI::getDate(const QString& spec)
{
    return StelUtils::jdToIsoString(getJDay());
}

//! Set time speed in JDay/sec
//! @param ts time speed in JDay/sec
void StelMainScriptAPI::setTimeRate(double ts)
{
    // 1 second = .00001157407407407407 JDay
    StelApp::getInstance().getCore()->getNavigator()->setTimeRate(ts * 0.00001157407407407407 * scriptSleeper.getRate());
}

//! Get time speed in JDay/sec
//! @return time speed in JDay/sec
double StelMainScriptAPI::getTimeRate(void) const
{
    return StelApp::getInstance().getCore()->getNavigator()->getTimeRate() / (0.00001157407407407407 * scriptSleeper.getRate());
}

bool StelMainScriptAPI::isRealTime()
{
    return StelApp::getInstance().getCore()->getNavigator()->getIsTimeNow();
}

void StelMainScriptAPI::setRealTime()
{
    StelApp::getInstance().isDiscreteTimeSteps = false;//ASAF
    setTimeRate(1.0);
    StelApp::getInstance().getCore()->getNavigator()->setTimeNow();
}

void StelMainScriptAPI::wait(double t)
{
    scriptSleeper.sleep(t*1000);
}

void StelMainScriptAPI::waitFor(const QString& dt, const QString& spec)
{
    double JD = jdFromDateString(dt, spec);
    StelNavigator* nav = StelApp::getInstance().getCore()->getNavigator();
    Q_ASSERT(nav);
    double timeSpeed = nav->getTimeRate();

    if (timeSpeed == 0.)
    {
        qWarning() << "waitFor called with no time passing - would be infinite. not waiting!";
        return;
    }
    else if (timeSpeed > 0)
    {
        while(nav->getJDay() < JD)
            scriptSleeper.sleep(200);
    }
    else
    {
        while(nav->getJDay() > JD)
            scriptSleeper.sleep(200);
    }
}

void StelMainScriptAPI::setObserverLocation(double longitude, double latitude, double altitude, double duration, const QString& name, const QString& planet)
{
    StelNavigator* nav = StelApp::getInstance().getCore()->getNavigator();
    Q_ASSERT(nav);
    SolarSystem* ssmgr = GETSTELMODULE(SolarSystem);
    Q_ASSERT(ssmgr);

    StelLocation loc = nav->getCurrentLocation();
    loc.longitude = longitude;
    loc.latitude = latitude;
    if (altitude > -1000)
        loc.altitude = altitude;
    if (ssmgr->searchByName(planet))
        loc.planetName = planet;
    loc.name = name;
    nav->moveObserverTo(loc, duration,duration);
}

void StelMainScriptAPI::setObserverLocation(const QString id, double duration)
{
    StelNavigator* nav = StelApp::getInstance().getCore()->getNavigator();
    Q_ASSERT(nav);
    StelLocation loc = StelApp::getInstance().getLocationMgr().locationForSmallString(id);
    // How best to test to see if the lookup of the name was a success?
    // On failure, it returns Paris, but maybe we _want_ Paris.
    // Ugly. -MNG
    if (id!="Paris, France" && (loc.name=="Paris" && loc.country=="France"))
        return;	// location find fail

    nav->moveObserverTo(loc, duration,duration);
}

QString StelMainScriptAPI::getObserverLocation()
{
    return StelApp::getInstance().getCore()->getNavigator()->getCurrentLocation().getID();
}

void StelMainScriptAPI::screenshot(const QString& prefix, bool invert, const QString& dir)
{
    bool oldInvertSetting = StelMainGraphicsView::getInstance().getFlagInvertScreenShotColors();
    StelMainGraphicsView::getInstance().setFlagInvertScreenShotColors(invert);
    StelMainGraphicsView::getInstance().saveScreenShot(prefix, dir);
    StelMainGraphicsView::getInstance().setFlagInvertScreenShotColors(oldInvertSetting);
}

void StelMainScriptAPI::setGuiVisible(bool b)
{
    StelApp::getInstance().getGui()->setVisible(b);
}

void StelMainScriptAPI::setMinFps(float m)
{
    StelMainGraphicsView::getInstance().setMinFps(m);
}

float StelMainScriptAPI::getMinFps()
{
    return StelMainGraphicsView::getInstance().getMinFps();
}

void StelMainScriptAPI::setMaxFps(float m)
{
    StelMainGraphicsView::getInstance().setMaxFps(m);
}

float StelMainScriptAPI::getMaxFps()
{
    return StelMainGraphicsView::getInstance().getMaxFps();
}

QString StelMainScriptAPI::getMountMode()
{
    if (GETSTELMODULE(StelMovementMgr)->getMountMode() == StelMovementMgr::MountEquinoxEquatorial)
        return "equatorial";
    else
        return "azimuthal";
}

void StelMainScriptAPI::setMountMode(const QString& mode)
{
    if (mode=="equatorial")
        GETSTELMODULE(StelMovementMgr)->setMountMode(StelMovementMgr::MountEquinoxEquatorial);
    else if (mode=="azimuthal")
        GETSTELMODULE(StelMovementMgr)->setMountMode(StelMovementMgr::MountAltAzimuthal);
}

bool StelMainScriptAPI::getNightMode()
{
    return StelApp::getInstance().getVisionModeNight();
}

void StelMainScriptAPI::setNightMode(bool b)
{
    emit(requestSetNightMode(b));
}

QString StelMainScriptAPI::getProjectionMode()
{
    return StelApp::getInstance().getCore()->getCurrentProjectionTypeKey();
}

void StelMainScriptAPI::setProjectionMode(const QString& id)
{
    emit(requestSetProjectionMode(id));
}

QStringList StelMainScriptAPI::getAllSkyCultureIDs(void)
{
    return StelApp::getInstance().getSkyCultureMgr().getSkyCultureListIDs();
}

QString StelMainScriptAPI::getSkyCulture()
{
    return StelApp::getInstance().getSkyCultureMgr().getCurrentSkyCultureID();
}

void StelMainScriptAPI::setSkyCulture(const QString& id)
{
    emit(requestSetSkyCulture(id));
}

bool StelMainScriptAPI::getFlagGravityLabels()
{
    return StelApp::getInstance().getCore()->getProjection(Mat4d())->getFlagGravityLabels();
}

void StelMainScriptAPI::setFlagGravityLabels(bool b)
{
    StelApp::getInstance().getCore()->setFlagGravityLabels(b);
}

bool StelMainScriptAPI::getDiskViewport()
{
    return StelApp::getInstance().getCore()->getProjection(Mat4d())->getMaskType() == StelProjector::MaskDisk;
}

void StelMainScriptAPI::setDiskViewport(bool b)
{
    emit(requestSetDiskViewport(b));
}


void StelMainScriptAPI::loadPresentImage(const QString& id, const QString& filename,
                                         double ra0, double dec0,
                                         double ra1, double dec1,
                                         double ra2, double dec2,
                                         double ra3, double dec3,
                                         double minRes, double maxBright,
                                         bool visible,bool screenORdome)
{
    QString path = filename;// "scripts/" + filename;
    emit(requestLoadPresentImage(id, path, ra0, dec0, ra1, dec1, ra2, dec2, ra3, dec3, minRes, maxBright, visible,screenORdome));
}

void StelMainScriptAPI::loadNewPresentImage(const QString& id, const QString& filename,
                                            double ra, double dec, double angSize, double rotation,
                                            double minRes, double maxBright, bool visible,
                                            bool screenORdome,double aspectratio)
{
    //        Vec3d XYZ;
    //        const double RADIUS_NEB = 1.;
    //        StelUtils::spheToRect(ra*M_PI/180., dec*M_PI/180., XYZ);
    //        XYZ*=RADIUS_NEB;
    //        double texSize = RADIUS_NEB * sin(angSize/2/60*M_PI/180);
    //        Mat4f matPrecomp = Mat4f::translation(XYZ) *
    //                                           Mat4f::zrotation(ra*M_PI/180.) *
    //                                           Mat4f::yrotation(-dec*M_PI/180.) *
    //                                           Mat4f::xrotation(rotation*M_PI/180.);
    //
    //        Vec3d corners[4];
    //                corners[0] = matPrecomp * Vec3d(0.,-texSize,-texSize);
    //                corners[1] = matPrecomp * Vec3f(0.f,-texSize, texSize);
    //                corners[2] = matPrecomp * Vec3f(0.f, texSize,-texSize);
    //                corners[3] = matPrecomp * Vec3d(0., texSize, texSize);
    //
    //        // convert back to ra/dec (radians)
    //        Vec3d cornersRaDec[4];
    //        for(int i=0; i<4; i++)
    //                StelUtils::rectToSphe(&cornersRaDec[i][0], &cornersRaDec[i][1], corners[i]);
    //
    //        loadPresentImage(id, filename,
    //                                 cornersRaDec[0][0]*180./M_PI, cornersRaDec[0][1]*180./M_PI,
    //                                 cornersRaDec[1][0]*180./M_PI, cornersRaDec[1][1]*180./M_PI,
    //                                 cornersRaDec[3][0]*180./M_PI, cornersRaDec[3][1]*180./M_PI,
    //                                 cornersRaDec[2][0]*180./M_PI, cornersRaDec[2][1]*180./M_PI,
    //                                 minRes, maxBright, visible, screenORdome);

    //QMessageBox::critical(0,"script",filename,0,0);
    //StelPresentMgr* sPmgr = GETSTELMODULE(StelPresentMgr);
    //sPmgr->loadPresentImage(id,filename,ra,dec,angSize,rotation,minRes,maxBright,visible,screenORdome,aspectratio);

    emit(requestLoadNewPresentImage(id,filename,ra,dec,angSize,rotation,minRes,maxBright,visible,screenORdome,aspectratio));


}

void StelMainScriptAPI::loadNewPresentVideo(const QString &id, const QString &filename, double ra, double dec, double angSize, double rotation, double minRes, double maxBright, bool visible, bool screenORdome, double aspectratio)
{
    emit(requestLoadNewPresentVideo(id,filename,ra,dec,angSize,rotation,minRes,maxBright,visible,screenORdome,aspectratio));

}

void StelMainScriptAPI::playPresentVideo(const QString &id,bool status)
{
    emit(requestPlayPresentVideo(id,status));
}

void StelMainScriptAPI::pauseTogglePresentVideo(const QString &id)
{
    emit(requestPauseTogglePresentVideo(id));
}

void StelMainScriptAPI::removePresentVideo(const QString &id)
{
    emit(requestRemovePresentVideo(id));
}
void StelMainScriptAPI::setPresentProperties(const QString& id, const QString& command)
{
    emit(requestSetPresentProperties(id,command));
}
void StelMainScriptAPI::setLayerdomeORsky(const QString& id,bool bscreen)
{
    emit(requestSetLayerdomeORsky(id,bscreen));
}

void StelMainScriptAPI::showPresentImage(const QString& id,bool visible)
{
    emit(requestShowPresentImage(id,visible));
}

void StelMainScriptAPI::mixPresentImage(const QString &id, bool value)
{
    emit(requestMixPresentImage(id,value));
}
void StelMainScriptAPI::removePresentImage(const QString& id)
{
    emit(requestRemovePresentImage(id));
}

void StelMainScriptAPI::loadSkyImage(const QString& id, const QString& filename,
                                     double ra0, double dec0,
                                     double ra1, double dec1,
                                     double ra2, double dec2,
                                     double ra3, double dec3,
                                     double minRes, double maxBright, bool visible)
{
    QString path = "scripts/" + filename;
    emit(requestLoadSkyImage(id, path, ra0, dec0, ra1, dec1, ra2, dec2, ra3, dec3, minRes, maxBright, visible));
}

void StelMainScriptAPI::loadSkyImage(const QString& id, const QString& filename,
                                     const QString& ra0, const QString& dec0,
                                     const QString& ra1, const QString& dec1,
                                     const QString& ra2, const QString& dec2,
                                     const QString& ra3, const QString& dec3,
                                     double minRes, double maxBright, bool visible)
{
    loadSkyImage(id, filename,
                 StelUtils::getDecAngle(ra0) *180./M_PI, StelUtils::getDecAngle(dec0)*180./M_PI,
                 StelUtils::getDecAngle(ra1) *180./M_PI, StelUtils::getDecAngle(dec1)*180./M_PI,
                 StelUtils::getDecAngle(ra2) *180./M_PI, StelUtils::getDecAngle(dec2)*180./M_PI,
                 StelUtils::getDecAngle(ra3) *180./M_PI, StelUtils::getDecAngle(dec3)*180./M_PI,
                 minRes, maxBright, visible);
}

void StelMainScriptAPI::loadSkyImage(const QString& id, const QString& filename,
                                     double ra, double dec, double angSize, double rotation,
                                     double minRes, double maxBright, bool visible)
{
    Vec3d XYZ;
    const double RADIUS_NEB = 1.;
    StelUtils::spheToRect(ra*M_PI/180., dec*M_PI/180., XYZ);
    XYZ*=RADIUS_NEB;
    double texSize = RADIUS_NEB * sin(angSize/2/60*M_PI/180);
    Mat4f matPrecomp = Mat4f::translation(XYZ.toVec3f()) *
                       Mat4f::zrotation(ra*M_PI/180.) *
                       Mat4f::yrotation(-dec*M_PI/180.) *
                       Mat4f::xrotation(rotation*M_PI/180.);

    Vec3f corners[4];
    corners[0] = matPrecomp * Vec3f(0.f,-texSize,-texSize);
    corners[1] = matPrecomp * Vec3f(0.f,-texSize, texSize);
    corners[2] = matPrecomp * Vec3f(0.f, texSize,-texSize);
    corners[3] = matPrecomp * Vec3f(0.f, texSize, texSize);

    // convert back to ra/dec (radians)
    Vec3d cornersRaDec[4];
    for(int i=0; i<4; i++)
        StelUtils::rectToSphe(&cornersRaDec[i][0], &cornersRaDec[i][1], corners[i].toVec3d());

    loadSkyImage(id, filename,
                 cornersRaDec[0][0]*180./M_PI, cornersRaDec[0][1]*180./M_PI,
                 cornersRaDec[1][0]*180./M_PI, cornersRaDec[1][1]*180./M_PI,
                 cornersRaDec[3][0]*180./M_PI, cornersRaDec[3][1]*180./M_PI,
                 cornersRaDec[2][0]*180./M_PI, cornersRaDec[2][1]*180./M_PI,
                 minRes, maxBright, visible);
}

void StelMainScriptAPI::loadSkyImage(const QString& id, const QString& filename,
                                     const QString& ra, const QString& dec, double angSize, double rotation,
                                     double minRes, double maxBright, bool visible)
{
    loadSkyImage(id, filename, StelUtils::getDecAngle(ra)*180./M_PI,
                 StelUtils::getDecAngle(dec)*180./M_PI, angSize,
                 rotation, minRes, maxBright, visible);
}

void StelMainScriptAPI::removeSkyImage(const QString& id)
{
    emit(requestRemoveSkyImage(id));
}

void StelMainScriptAPI::loadSound(const QString& filename, const QString& id)
{
    QString path;
    try
    {
        path = StelFileMgr::findFile("scripts/" + filename);
    }
    catch(std::runtime_error& e)
    {
        qWarning() << "cannot play sound" << filename << ":" << e.what();
        return;
    }

    emit(requestLoadSound(path, id));
}

void StelMainScriptAPI::playSound(const QString& id)
{
    emit(requestPlaySound(id));
}

void StelMainScriptAPI::pauseSound(const QString& id)
{
    emit(requestPauseSound(id));
}

void StelMainScriptAPI::stopSound(const QString& id)
{
    emit(requestStopSound(id));
}

void StelMainScriptAPI::dropSound(const QString& id)
{
    emit(requestDropSound(id));
}

int StelMainScriptAPI::getScreenWidth(void)
{
    return StelMainGraphicsView::getInstance().size().width();
}

int StelMainScriptAPI::getScreenHeight(void)
{
    return StelMainGraphicsView::getInstance().size().height();
}

double StelMainScriptAPI::getScriptRate(void)
{
    return scriptSleeper.getRate();
}

void StelMainScriptAPI::setScriptRate(double r)
{
    return scriptSleeper.setRate(r);
}

void StelMainScriptAPI::setSelectedObjectInfo(const QString& level)
{
    if (level == "AllInfo")
        StelApp::getInstance().getGui()->setInfoTextFilters(StelObject::InfoStringGroup(StelObject::AllInfo));
    else if (level == "ShortInfo")
        StelApp::getInstance().getGui()->setInfoTextFilters(StelObject::InfoStringGroup(StelObject::ShortInfo));
    else if (level == "None")
        StelApp::getInstance().getGui()->setInfoTextFilters((StelObject::InfoStringGroup)0);
    else
        qWarning() << "setSelectedObjectInfo unknown level string \"" << level << "\"";
}

void StelMainScriptAPI::exit(void)
{
    emit(requestExit());
}

void StelMainScriptAPI::quitStellarium(void)
{
    QCoreApplication::exit();
}

void StelMainScriptAPI::debug(const QString& s)
{
    qDebug() << "script: " << s;
    StelMainGraphicsView::getInstance().getScriptMgr().debug(s);
}

double StelMainScriptAPI::jdFromDateString(const QString& dt, const QString& spec)
{
    QDateTime qdt;
    double JD;

    // 2008-03-24T13:21:01
    QRegExp isoRe("^\\d{4}[:\\-]\\d\\d[:\\-]\\d\\dT\\d?\\d:\\d\\d:\\d\\d$");
    QRegExp nowRe("^(now)?(\\s*([+\\-])\\s*(\\d+(\\.\\d+)?)\\s*(second|seconds|minute|minutes|hour|hours|day|days|week|weeks))(\\s+(sidereal)?)?");

    if (dt == "now")
        return StelUtils::getJDFromSystem();
    else if (isoRe.exactMatch(dt))
    {
        qdt = QDateTime::fromString(dt, Qt::ISODate);

        if (spec=="local")
            JD = StelUtils::qDateTimeToJd(qdt.toUTC());
        else
            JD = StelUtils::qDateTimeToJd(qdt);
        return JD;
    }
    else if (nowRe.exactMatch(dt))
    {
        double delta;
        double unit;
        double dayLength = 1.0;

        if (nowRe.capturedTexts().at(1)=="now")
            JD = StelUtils::getJDFromSystem();
        else
            JD = StelApp::getInstance().getCore()->getNavigator()->getJDay();

        if (nowRe.capturedTexts().at(8) == "sidereal")
            dayLength = StelApp::getInstance().getCore()->getNavigator()->getLocalSideralDayLength();

        QString unitString = nowRe.capturedTexts().at(6);
        if (unitString == "seconds" || unitString == "second")
            unit = dayLength / (24*3600.);
        else if (unitString == "minutes" || unitString == "minute")
            unit = dayLength / (24*60.);
        else if (unitString == "hours" || unitString == "hour")
            unit = dayLength / (24.);
        else if (unitString == "days" || unitString == "day")
            unit = dayLength;
        else if (unitString == "weeks" || unitString == "week")
            unit = dayLength * 7.;
        else
        {
            qWarning() << "StelMainScriptAPI::setDate - unknown time unit:" << nowRe.capturedTexts().at(4);
            unit = 0;
        }

        delta = nowRe.capturedTexts().at(4).toDouble();

        if (nowRe.capturedTexts().at(3) == "+")
            JD += (unit * delta);
        else if (nowRe.capturedTexts().at(3) == "-")
            JD -= (unit * delta);

        return JD;
    }
    else
    {
        qWarning() << "StelMainScriptAPI::jdFromDateString error - date string" << dt << "not recognised, returning \"now\"";
        return StelUtils::getJDFromSystem();
    }
}

void StelMainScriptAPI::selectObjectByName(const QString& name, bool pointer)
{
    StelObjectMgr* omgr = GETSTELMODULE(StelObjectMgr);
    omgr->setFlagSelectedObjectPointer(pointer);
    if (name=="")
        omgr->unSelect();
    else
        omgr->findAndSelect(name);
}

QVariantMap StelMainScriptAPI::getObjectPosition(const QString& name)
{
    StelObjectMgr* omgr = GETSTELMODULE(StelObjectMgr);
    StelObjectP obj = omgr->searchByName(name);
    QVariantMap map;

    if (!obj)
    {
        debug("getObjectData WARNING - object not found: " + name);
        map.insert("found", false);
        return map;
    }
    else
    {
        map.insert("found", true);
    }

    StelNavigator* nav = StelApp::getInstance().getCore()->getNavigator();
    Vec3d pos;
    double ra, dec, alt, azi;

    // ra/dec
    pos = obj->getEquinoxEquatorialPos(nav);
    StelUtils::rectToSphe(&ra, &dec, pos);
    map.insert("ra", ra*180./M_PI);
    map.insert("dec", dec*180./M_PI);

    // ra/dec in J2000
    pos = obj->getJ2000EquatorialPos(nav);
    StelUtils::rectToSphe(&ra, &dec, pos);
    map.insert("raJ2000", ra*180./M_PI);
    map.insert("decJ2000", dec*180./M_PI);

    // altitude/azimuth
    pos = obj->getAltAzPos(nav);
    StelUtils::rectToSphe(&azi, &alt, pos);
    map.insert("altitude", alt*180./M_PI);
    map.insert("azimuth", azi*180./M_PI);

    return map;
}

void StelMainScriptAPI::clear(const QString& state)
{
    LandscapeMgr* lmgr = GETSTELMODULE(LandscapeMgr);
    SolarSystem* ssmgr = GETSTELMODULE(SolarSystem);
    MeteorMgr* mmgr = GETSTELMODULE(MeteorMgr);
    StelSkyDrawer* skyd = StelApp::getInstance().getCore()->getSkyDrawer();
    ConstellationMgr* cmgr = GETSTELMODULE(ConstellationMgr);
    StarMgr* smgr = GETSTELMODULE(StarMgr);
    NebulaMgr* nmgr = GETSTELMODULE(NebulaMgr);
    GridLinesMgr* glmgr = GETSTELMODULE(GridLinesMgr);
    StelMovementMgr* movmgr = GETSTELMODULE(StelMovementMgr);
    StelCore* core = StelApp::getInstance().getCore();

    if (state.toLower() == "natural")
    {
        //#ifdef SHIRAPLAYER_PRE
        movmgr->setMountMode(StelMovementMgr::MountAltAzimuthal); // ASAF bu sat�rlar planetaryum i�in iptal edilmek zorunda
        //#endif
        skyd->setFlagTwinkle(true);
        skyd->setFlagLuminanceAdaptation(true);
        ssmgr->setFlagPlanets(true);
        ssmgr->setFlagHints(false);
        ssmgr->setFlagOrbits(false);
        ssmgr->setFlagAxises(false);
        ssmgr->setFlagMoonScale(false);
        ssmgr->setFlagTrails(false);
        mmgr->setZHR(10);
        glmgr->setFlagAzimuthalGrid(false);
        glmgr->setFlagEquatorGrid(false);
        glmgr->setFlagEquatorLine(false);
        glmgr->setFlagEclipticLine(false);
        glmgr->setFlagMeridianLine(false);
        glmgr->setFlagEquatorJ2000Grid(false);
        lmgr->setFlagCardinalsPoints(false);
        cmgr->setFlagLines(false);
        cmgr->setFlagLabels(false);
        cmgr->setFlagBoundaries(false);
        cmgr->setFlagArt(false);
        smgr->setFlagLabels(false);
        ssmgr->setFlagLabels(false);
        nmgr->setFlagHints(false);
        lmgr->setFlagLandscape(true);
        lmgr->setFlagAtmosphere(true);
        lmgr->setFlagFog(true);
    }
    else if (state.toLower() == "starchart")
    {
        //#ifdef SHIRAPLAYER_PRE
        movmgr->setMountMode(StelMovementMgr::MountEquinoxEquatorial);// ASAF bu sat�rlar planetaryum i�in iptal edilmek zorunda
        //#endif
        skyd->setFlagTwinkle(false);
        skyd->setFlagLuminanceAdaptation(false);
        ssmgr->setFlagPlanets(true);
        ssmgr->setFlagHints(false);
        ssmgr->setFlagOrbits(false);
        ssmgr->setFlagAxises(false);
        ssmgr->setFlagMoonScale(false);
        ssmgr->setFlagTrails(false);
        mmgr->setZHR(0);
        glmgr->setFlagAzimuthalGrid(false);
        glmgr->setFlagEquatorGrid(true);
        glmgr->setFlagEquatorLine(false);
        glmgr->setFlagEclipticLine(false);
        glmgr->setFlagMeridianLine(false);
        glmgr->setFlagEquatorJ2000Grid(false);
        lmgr->setFlagCardinalsPoints(false);
        cmgr->setFlagLines(true);
        cmgr->setFlagLabels(true);
        cmgr->setFlagBoundaries(true);
        cmgr->setFlagArt(false);
        smgr->setFlagLabels(true);
        ssmgr->setFlagLabels(true);
        nmgr->setFlagHints(true);
        lmgr->setFlagLandscape(false);
        lmgr->setFlagAtmosphere(false);
        lmgr->setFlagFog(false);
    }
    else if (state.toLower() == "deepspace")
    {
        //#ifdef SHIRAPLAYER_PRE
        movmgr->setMountMode(StelMovementMgr::MountEquinoxEquatorial);// ASAF bu sat�rlar planetaryum i�in iptal edilmek zorunda
        //#endif
        skyd->setFlagTwinkle(false);
        skyd->setFlagLuminanceAdaptation(false);
        ssmgr->setFlagPlanets(false);
        ssmgr->setFlagHints(false);
        ssmgr->setFlagOrbits(false);
        ssmgr->setFlagAxises(false);
        ssmgr->setFlagMoonScale(false);
        ssmgr->setFlagTrails(false);
        mmgr->setZHR(0);
        glmgr->setFlagAzimuthalGrid(false);
        glmgr->setFlagEquatorGrid(false);
        glmgr->setFlagEquatorLine(false);
        glmgr->setFlagEclipticLine(false);
        glmgr->setFlagMeridianLine(false);
        glmgr->setFlagEquatorJ2000Grid(false);
        lmgr->setFlagCardinalsPoints(false);
        cmgr->setFlagLines(false);
        cmgr->setFlagLabels(false);
        cmgr->setFlagBoundaries(false);
        cmgr->setFlagArt(false);
        smgr->setFlagLabels(false);
        ssmgr->setFlagLabels(false);
        nmgr->setFlagHints(false);
        lmgr->setFlagLandscape(false);
        lmgr->setFlagAtmosphere(false);
        lmgr->setFlagFog(false);
    }
    else if (state.toLower() == "spaceship")
    {
        recordData.clear();
        recordData.append(QString("%0").arg(skyd->getFlagTwinkle()));skyd->setFlagTwinkle(false);
        recordData.append(QString("%0").arg(skyd->getFlagLuminanceAdaptation()));skyd->setFlagLuminanceAdaptation(false);
        recordData.append(QString("%0").arg(ssmgr->getFlagPlanets()));ssmgr->setFlagPlanets(true);
        recordData.append(QString("%0").arg(ssmgr->getFlagHints()));ssmgr->setFlagHints(false);
        recordData.append(QString("%0").arg(ssmgr->getFlagMoonScale()));ssmgr->setFlagMoonScale(false);
        recordData.append(QString("%0").arg(ssmgr->getFlagTrails()));ssmgr->setFlagTrails(false);
        recordData.append(QString("%0").arg(mmgr->getZHR()));mmgr->setZHR(0);
        recordData.append(QString("%0").arg(glmgr->getFlagAzimuthalGrid()));glmgr->setFlagAzimuthalGrid(false);
        recordData.append(QString("%0").arg(glmgr->getFlagEquatorGrid()));glmgr->setFlagEquatorGrid(false);
        recordData.append(QString("%0").arg(glmgr->getFlagEquatorLine()));glmgr->setFlagEquatorLine(false);
        recordData.append(QString("%0").arg(glmgr->getFlagEclipticLine()));glmgr->setFlagEclipticLine(false);
        recordData.append(QString("%0").arg(glmgr->getFlagMeridianLine()));glmgr->setFlagMeridianLine(false);
        recordData.append(QString("%0").arg(glmgr->getFlagEquatorJ2000Grid()));glmgr->setFlagEquatorJ2000Grid(false);
        recordData.append(QString("%0").arg(lmgr->getFlagCardinalsPoints()));lmgr->setFlagCardinalsPoints(false);
        recordData.append(QString("%0").arg(smgr->getFlagLabels()));smgr->setFlagLabels(false);
        recordData.append(QString("%0").arg(nmgr->getFlagHints()));nmgr->setFlagHints(false);
        recordData.append(QString("%0").arg(lmgr->getFlagLandscape()));lmgr->setFlagLandscape(false);
        recordData.append(QString("%0").arg(lmgr->getFlagAtmosphere()));lmgr->setFlagAtmosphere(false);
        recordData.append(QString("%0").arg(lmgr->getFlagFog()));lmgr->setFlagFog(false);
        recordData.append(QString("%0").arg(ssmgr->getFlagOrbits()));ssmgr->setFlagOrbits(false);

        recordData.append(QString("%0").arg(ssmgr->getFlagPlanetsScale()));//ssmgr->setFlagPlanetsScale(false);
        recordData.append(QString("%0").arg(ssmgr->getMars()->getanimSphereFader()));ssmgr->getMars()->setanimSphereFader(false);
        recordData.append(QString("%0").arg(ssmgr->getVenus()->getanimSphereFader()));ssmgr->getVenus()->setanimSphereFader(false);
        recordData.append(QString("%0").arg(ssmgr->getSaturn()->getanimSphereFader()));ssmgr->getSaturn()->setanimSphereFader(false);
        recordData.append(QString("%0").arg(ssmgr->getJupiter()->getanimSphereFader()));ssmgr->getJupiter()->setanimSphereFader(false);
        recordData.append(QString("%0").arg(ssmgr->getUranus()->getanimSphereFader()));ssmgr->getUranus()->setanimSphereFader(false);
        recordData.append(QString("%0").arg(ssmgr->getPluto()->getanimSphereFader()));ssmgr->getPluto()->setanimSphereFader(false);
        recordData.append(QString("%0").arg(ssmgr->getNeptun()->getanimSphereFader()));ssmgr->getNeptun()->setanimSphereFader(false);
        recordData.append(QString("%0").arg(ssmgr->getMerkur()->getanimSphereFader()));ssmgr->getMerkur()->setanimSphereFader(false);

        Vec3d initViewPos = core->getNavigator()->j2000ToAltAz(movmgr->getViewDirectionJ2000());
        QString strview = QString("%1,%2,%3").arg(initViewPos[0])
                                             .arg(initViewPos[1])
                                             .arg(initViewPos[2]);
        recordData.append(strview);
        
        StelApp::getInstance().startedFlyby = true;
    }
    else if (state.toLower() == "returnback")
    {
        //if ( recordData.count() != 20 ) return;

        skyd->setFlagTwinkle(recordData[0].toInt());
        skyd->setFlagLuminanceAdaptation(recordData[1].toInt());
        ssmgr->setFlagPlanets(recordData[2].toInt());
        ssmgr->setFlagHints(recordData[3].toInt());
        ssmgr->setFlagMoonScale(recordData[4].toInt());
        ssmgr->setFlagTrails(recordData[5].toInt());
        mmgr->setZHR(recordData[6].toInt());
        glmgr->setFlagAzimuthalGrid(recordData[7].toInt());
        glmgr->setFlagEquatorGrid(recordData[8].toInt());
        glmgr->setFlagEquatorLine(recordData[9].toInt());
        glmgr->setFlagEclipticLine(recordData[10].toInt());
        glmgr->setFlagMeridianLine(recordData[11].toInt());
        glmgr->setFlagEquatorJ2000Grid(recordData[12].toInt());
        lmgr->setFlagCardinalsPoints(recordData[13].toInt());
        smgr->setFlagLabels(recordData[14].toInt());
        nmgr->setFlagHints(recordData[15].toInt());
        lmgr->setFlagLandscape(recordData[16].toInt());
        lmgr->setFlagAtmosphere(recordData[17].toInt());
        lmgr->setFlagFog(recordData[18].toInt());
        ssmgr->setFlagOrbits(recordData[19].toInt());

        ssmgr->setFlagPlanetsScale(recordData[20].toInt());
        ssmgr->getMars()->setanimSphereFader(recordData[21].toInt());
        ssmgr->getVenus()->setanimSphereFader(recordData[22].toInt());
        ssmgr->getSaturn()->setanimSphereFader(recordData[23].toInt());
        ssmgr->getJupiter()->setanimSphereFader(recordData[24].toInt());
        ssmgr->getUranus()->setanimSphereFader(recordData[25].toInt());
        ssmgr->getPluto()->setanimSphereFader(recordData[26].toInt());
        ssmgr->getNeptun()->setanimSphereFader(recordData[27].toInt());
        ssmgr->getMerkur()->setanimSphereFader(recordData[28].toInt());

        Vec3d initViewPos = StelUtils::strToVec3f(recordData[29]).toVec3d();
        Vec3d viewDirectionJ2000 = core->getNavigator()->altAzToJ2000(initViewPos);
        movmgr->setViewDirectionJ2000(viewDirectionJ2000);
        //qDebug()<<"returnback"<<ssmgr->getFlagPlanetsScale()<<"--"
        //          <<ssmgr->getSaturn()->getanimSphereFader();
    }
    else
    {
        qWarning() << "WARNING clear(" << state << ") - state not known";
    }
}

double StelMainScriptAPI::getViewAltitudeAngle()
{
    const Vec3d& current = StelApp::getInstance().getCore()->getNavigator()->j2000ToAltAz(GETSTELMODULE(StelMovementMgr)->getViewDirectionJ2000());
    double alt, azi;
    StelUtils::rectToSphe(&azi, &alt, current);
    return alt*180/M_PI; // convert to degrees from radians
}

double StelMainScriptAPI::getViewAzimuthAngle()
{
    const Vec3d& current = StelApp::getInstance().getCore()->getNavigator()->j2000ToAltAz(GETSTELMODULE(StelMovementMgr)->getViewDirectionJ2000());
    double alt, azi;
    StelUtils::rectToSphe(&azi, &alt, current);
    // The returned azimuth angle is in radians and set up such that:
    // N=+/-PI; E=PI/2; S=0; W=-PI/2;
    // But we want compass bearings, i.e. N=0, E=90, S=180, W=270
    return std::fmod(((azi*180/M_PI)*-1)+180., 360.);
}

double StelMainScriptAPI::getViewRaAngle()
{
    const Vec3d& current = StelApp::getInstance().getCore()->getNavigator()->j2000ToEquinoxEqu(GETSTELMODULE(StelMovementMgr)->getViewDirectionJ2000());
    double ra, dec;
    StelUtils::rectToSphe(&ra, &dec, current);
    // returned RA angle is in range -PI .. PI, but we want 0 .. 360
    return std::fmod((ra*180/M_PI)+360., 360.); // convert to degrees from radians
}

double StelMainScriptAPI::getViewDecAngle()
{
    const Vec3d& current = StelApp::getInstance().getCore()->getNavigator()->j2000ToEquinoxEqu(GETSTELMODULE(StelMovementMgr)->getViewDirectionJ2000());
    double ra, dec;
    StelUtils::rectToSphe(&ra, &dec, current);
    return dec*180/M_PI; // convert to degrees from radians
}

double StelMainScriptAPI::getViewRaJ2000Angle()
{
    Vec3d current = GETSTELMODULE(StelMovementMgr)->getViewDirectionJ2000();
    double ra, dec;
    StelUtils::rectToSphe(&ra, &dec, current);
    // returned RA angle is in range -PI .. PI, but we want 0 .. 360
    return std::fmod((ra*180/M_PI)+360., 360.); // convert to degrees from radians
}

double StelMainScriptAPI::getViewDecJ2000Angle()
{
    Vec3d current = GETSTELMODULE(StelMovementMgr)->getViewDirectionJ2000();
    double ra, dec;
    StelUtils::rectToSphe(&ra, &dec, current);
    return dec*180/M_PI; // convert to degrees from radians
}

void StelMainScriptAPI::moveToAltAzi(const QString& alt, const QString& azi, float duration)
{
    StelMovementMgr* mvmgr = GETSTELMODULE(StelMovementMgr);
    Q_ASSERT(mvmgr);

    //GETSTELMODULE(StelObjectMgr)->unSelect();//ASAF

    Vec3d aim;
    double dAlt = StelUtils::getDecAngle(alt);
    double dAzi = M_PI - StelUtils::getDecAngle(azi);

    StelUtils::spheToRect(dAzi,dAlt,aim);
    mvmgr->moveToJ2000(StelApp::getInstance().getCore()->getNavigator()->altAzToJ2000(aim), duration);
}

void StelMainScriptAPI::moveToRaDec(const QString& ra, const QString& dec, float duration)
{
    StelMovementMgr* mvmgr = GETSTELMODULE(StelMovementMgr);
    Q_ASSERT(mvmgr);

    //GETSTELMODULE(StelObjectMgr)->unSelect();//ASAF

    Vec3d aim;
    double dRa = StelUtils::getDecAngle(ra);
    double dDec = StelUtils::getDecAngle(dec);

    StelUtils::spheToRect(dRa,dDec,aim);
    mvmgr->moveToJ2000(StelApp::getInstance().getCore()->getNavigator()->equinoxEquToJ2000(aim), duration);
}

void StelMainScriptAPI::moveToRaDecJ2000(const QString& ra, const QString& dec, float duration)
{
    StelMovementMgr* mvmgr = GETSTELMODULE(StelMovementMgr);
    Q_ASSERT(mvmgr);

    GETSTELMODULE(StelObjectMgr)->unSelect();

    Vec3d aimJ2000, aimEquofDate;
    double dRa = StelUtils::getDecAngle(ra);
    double dDec = StelUtils::getDecAngle(dec);

    StelUtils::spheToRect(dRa,dDec,aimJ2000);
    aimEquofDate = StelApp::getInstance().getCore()->getNavigator()->j2000ToEquinoxEqu(aimJ2000);
    mvmgr->moveToJ2000(aimEquofDate, duration);
}

//ASAF
void StelMainScriptAPI::setDiscreteTime(const QString& type)
{
    StelApp::getInstance().isDiscreteTimeSteps = true;
    if(type == "seconds")
        StelApp::getInstance().m_discrete = StelApp::DiscreteTimeStepsSeconds;
    else if(type == "minutes")
        StelApp::getInstance().m_discrete = StelApp::DiscreteTimeStepsMinutes;
    else if(type == "hours")
        StelApp::getInstance().m_discrete = StelApp::DiscreteTimeStepsHours;
    else if(type == "days")
        StelApp::getInstance().m_discrete = StelApp::DiscreteTimeStepsDays;
    else if(type == "years")
        StelApp::getInstance().m_discrete = StelApp::DiscreteTimeStepsYears;
    else
    {
        StelApp::getInstance().isDiscreteTimeSteps = false;
        return;
    }
}

void StelMainScriptAPI::setTimeNow()
{
    StelApp::getInstance().getCore()->getNavigator()->setTimeNow();
}

void StelMainScriptAPI::SetFade(bool fader)
{
    StelApp::getInstance().getCore()->allFaderColor = Vec3f(0,0,0);
    StelApp::getInstance().getCore()->ALLFader.setDuration(2000);
    StelApp::getInstance().getCore()->ALLFader = fader;
}

void StelMainScriptAPI::showDateTime(bool state)
{
    StelApp::getInstance().getCore()->flagTuiDatetime = state;
}
void StelMainScriptAPI::showLocation(bool state)
{
    StelApp::getInstance().getCore()->flagTuiLocation = state;
}
void StelMainScriptAPI::showProperties(bool state)
{
    StelApp::getInstance().showPropGui = state;
}

void StelMainScriptAPI::selectObjectByEqPos(const QString& ra, const QString& dec, bool pointer)
{
    StelObjectMgr* omgr = GETSTELMODULE(StelObjectMgr);
    omgr->setFlagSelectedObjectPointer(pointer);

    Vec3d pos;
    pos.set(QString("%0").arg(ra).toDouble(),QString("%0").arg(dec).toDouble(),0);
    omgr->findAndSelect(StelApp::getInstance().getCore(),pos);

}

void StelMainScriptAPI::selectObjectByI18Name(const QString& name, bool pointer)
{
    StelObjectMgr* omgr = GETSTELMODULE(StelObjectMgr);
    omgr->setFlagSelectedObjectPointer(pointer);
    if (name=="")
        omgr->unSelect();
    else
        omgr->findAndSelectI18n(name);
}

void StelMainScriptAPI::loadVideo(const QString& filename)
{
    emit(requestLoadVideo(filename));
}
void StelMainScriptAPI::playVideo(bool is_Audio)
{
    emit(requestPlayVideo(is_Audio));
}

void StelMainScriptAPI::pauseVideo(bool pause)
{
    emit(requestPauseVideo(pause));
}

void StelMainScriptAPI::stopVideo()
{
    emit (requestStopVideo());
}
int StelMainScriptAPI::getTotalTimeVideo()
{
    int result = 0;
    LandscapeMgr* lmgr = GETSTELMODULE(LandscapeMgr);
    //todo
    /*if (lmgr->vop_curr)
        result = lmgr->vop_curr->getTimeSnFromFrame(lmgr->vop_curr->getEnd());*/
    return result;
}
void StelMainScriptAPI::playAudio(const QString& filename)
{
    emit(requestPlayAudio(filename));
}

void StelMainScriptAPI::stopAudio()
{
    emit(requestStopAudio());
}

int  StelMainScriptAPI::getTotalTimeAudio()
{
    //todo
    //return getTimeSnFromFrame(getEnd());
    return 0;
}
void StelMainScriptAPI::setPVideoContrast(int presentID,double value)
{
    emit(requestPVideoContrast(presentID,value));
}
void StelMainScriptAPI::setPVideoBrightness(int presentID,double value)
{
   emit(requestPVideoBrightness(presentID,value));
}
void StelMainScriptAPI::setPVideoSaturation(int presentID, double value)
{
   emit(requestPVideoSaturation(presentID,value));
}

//Bazi kisayollar
void StelMainScriptAPI::setFlipHorz(bool b)
{
    StelApp::getInstance().getCore()->setFlipHorz(b);
}
void StelMainScriptAPI::setFlipVert(bool b)
{
    StelApp::getInstance().getCore()->setFlipVert(b);
}
void StelMainScriptAPI::decreaseTimeSpeed()
{
    StelApp::getInstance().getCore()->navigation->decreaseTimeSpeed();
}
void StelMainScriptAPI::increaseTimeSpeed()
{
    StelApp::getInstance().getCore()->navigation->increaseTimeSpeed();
}
void StelMainScriptAPI::decreaseTimeSpeedLess()
{
    StelApp::getInstance().getCore()->navigation->decreaseTimeSpeedLess();
}
void StelMainScriptAPI::increaseTimeSpeedLess()
{
    StelApp::getInstance().getCore()->navigation->increaseTimeSpeedLess();
}

void StelMainScriptAPI::prepareFlyBy()
{
    emit(requestPrepareFlyBy());
}

void StelMainScriptAPI::setFlyBy(const QString &planetFlyByname, bool isInner)
{
    emit(requestSetFlyBy(planetFlyByname,isInner));
}

void StelMainScriptAPI::goHome()
{
    emit(requestGoHome());
}
void StelMainScriptAPI::showFPS(bool value)
{
    StelApp::getInstance().getCore()->setShowFPS(value);
}

//ASAF
