/*
 * ShiraPlayer(TM)
 * Copyright (C) 2006 Fabien Chereau
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

#include <QDebug>
#include <QSettings>
#include <QString>

#ifdef USE_OPENGL_ES2
#include "GLES2/gl2.h"
#else
#include <QtOpenGL/QtOpenGL>
#endif

#include "LandscapeMgr.hpp"
#include "Landscape.hpp"
#include "Atmosphere.hpp"
#include "StelApp.hpp"
#include "SolarSystem.hpp"
#include "StelCore.hpp"
#include "StelLocaleMgr.hpp"
#include "StelModuleMgr.hpp"
#include "StelFileMgr.hpp"
#include "Planet.hpp"
#include "StelIniParser.hpp"
#include "StelSkyDrawer.hpp"
#include "StelStyle.hpp"
#include "StelPainter.hpp"
#include "StelAppGraphicsWidget.hpp"
#include "StelObjectMgr.hpp"
#include "StelLocation.hpp"
#include "SolarSystem.hpp"
#include "Planet.hpp"
#include "StelMainWindow.hpp"
#include "StelGui.hpp"
#include "videoutils/embedaudiowarning.h"

// Class which manages the cardinal points displaying
class Cardinals
{
public:
    Cardinals(float _radius = 1.);
    virtual ~Cardinals();
    void draw(const StelCore* core, double latitude, bool gravityON = false) const;
    void setColor(const Vec3f& c) {color = c;}
    Vec3f get_color() {return color;}
    void updateI18n();
    void update(double deltaTime) {fader.update((int)(deltaTime*1000));}
    void set_fade_duration(float duration) {fader.setDuration((int)(duration*1000.f));}
    void setFlagShow(bool b){fader = b;}
    bool getFlagShow(void) const {return fader;}
private:
    float radius;
    QFont font;
    Vec3f color;
    QString sNorth, sSouth, sEast, sWest;
    LinearFader fader;
};


Cardinals::Cardinals(float _radius) : radius(_radius), color(0.6,0.2,0.2)
{
    QSettings* conf = StelApp::getInstance().getSettings();
    Q_ASSERT(conf);
    font.setPixelSize(conf->value("viewing/cardinal_font_size",30).toInt());
    // Default labels - if sky locale specified, loaded later
    // Improvement for gettext translation
    sNorth = "N";
    sSouth = "S";
    sEast = "E";
    sWest = "W";
}

Cardinals::~Cardinals()
{
}

// Draw the cardinals points : N S E W
// handles special cases at poles
void Cardinals::draw(const StelCore* core, double latitude, bool gravityON) const
{
    const StelProjectorP prj = core->getProjection(StelCore::FrameAltAz);
    StelPainter sPainter(prj);
    sPainter.setFont(font);

    if (!fader.getInterstate()) return;

    // direction text
    QString d[4];

    d[0] = sNorth;
    d[1] = sSouth;
    d[2] = sEast;
    d[3] = sWest;

    // fun polar special cases
    if (latitude ==  90.0 ) d[0] = d[1] = d[2] = d[3] = sSouth;
    if (latitude == -90.0 ) d[0] = d[1] = d[2] = d[3] = sNorth;

    sPainter.setColor(color[0],color[1],color[2],fader.getInterstate());
    glEnable(GL_BLEND);
    sPainter.enableTexture2d(true);
    // Normal transparency mode
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    Vec3d pos;
    Vec3d xy;

    float shift = sPainter.getFontMetrics().width(sNorth)/2;
    //ASAF Z=0.12f olarak deðiþtirdim
    // N for North
    pos.set(-1.f, 0.f, 0.0f);
    if (prj->project(pos,xy)) sPainter.drawText(xy[0], xy[1], d[0], 0., -shift, -shift,!prj->getFlagGravityLabels());

    // S for South
    pos.set(1.f, 0.f, 0.0f);
    if (prj->project(pos,xy)) sPainter.drawText(xy[0], xy[1], d[1], 0., -shift, -shift,!prj->getFlagGravityLabels());

    // E for East
    pos.set(0.f, 1.f, 0.0f);
    if (prj->project(pos,xy)) sPainter.drawText(xy[0], xy[1], d[2], 0., -shift, -shift,!prj->getFlagGravityLabels());

    // W for West
    pos.set(0.f, -1.f, 0.0f);
    if (prj->project(pos,xy)) sPainter.drawText(xy[0], xy[1], d[3], 0., -shift, -shift,!prj->getFlagGravityLabels());

}

// Translate cardinal labels with gettext to current sky language and update font for the language
void Cardinals::updateI18n()
{
    sNorth = q_("N");
    sSouth = q_("S");
    sEast = q_("E");
    sWest = q_("W");
}


LandscapeMgr::LandscapeMgr() : atmosphere(NULL), cardinalsPoints(NULL), landscape(NULL), flagLandscapeSetsLocation(false)
{
    setObjectName("LandscapeMgr");

    isVideoLandscape = false;
    //#ifdef SHIRAPLAYER_PRE
    if(!StelMainWindow::getInstance().getIsMultiprojector())
        videoSharedMem.setKey("videoSharedMemory");
    //#endif

    vsaturation = 1.0;
    vcontrast = 1.0;
    vbrightness = 1.0;

    fromScriptVideo = false;
}

LandscapeMgr::~LandscapeMgr()
{
    delete atmosphere;
    delete cardinalsPoints;
    delete landscape;
    landscape = NULL;

    //todo
    QObject::disconnect(vop_curr,0,0,0);

    if(vop_curr)
        delete vop_curr;

}

/*************************************************************************
 Reimplementation of the getCallOrder method
*************************************************************************/
double LandscapeMgr::getCallOrder(StelModuleActionName actionName) const
{
    if (actionName==StelModule::ActionDraw)
        return StelApp::getInstance().getModuleMgr().getModule("MeteorMgr")->getCallOrder(actionName)+20;
    if (actionName==StelModule::ActionUpdate)
        return StelApp::getInstance().getModuleMgr().getModule("SolarSystem")->getCallOrder(actionName)+10;
    return 0;
}

void LandscapeMgr::update(double deltaTime)
{
    if(!StelApp::getInstance().isVideoMode)
        atmosphere->update(deltaTime);
    landscape->update(deltaTime);
    
    if(StelApp::getInstance().isVideoMode) return;
    
    cardinalsPoints->update(deltaTime);

    // Compute the atmosphere color and intensity
    SolarSystem* ssystem = (SolarSystem*)StelApp::getInstance().getModuleMgr().getModule("SolarSystem");
    StelNavigator* nav = StelApp::getInstance().getCore()->getNavigator();

    /*  StelLocation loc = nav->getCurrentLocation();

    if (loc.planetName.indexOf("FLYBY")>-1)
    {
        PlanetP flyPlanet = ssystem->searchByEnglishNameForFlyby(loc.planetName);

        if (flyPlanet)
        {
            PlanetP byPlanet = flyPlanet.data()->getFlybyPlanet();
            qDebug()<<"FlyBy radius:"<<flyPlanet.data()->getRadius()*AU;
            qDebug()<<"By radius:"<<byPlanet.data()->getRadius()*AU;
            double altitudeDiff = flyPlanet.data()->getRadius()-byPlanet.data()->getRadius();
            qDebug()<<"Altitude diff:"<<altitudeDiff*AU;
            qDebug()<<"get earth rad:"<<ssystem->getEarth().data()->getRadius()*AU;
            qDebug()<<"get altitude"<<nav->getCurrentLocation().altitude;
        }
    }*/

    /*Vec3d fly = ssystem->getEarth().data()->getAltAzPos(nav);*/



    // Compute the sun position in local coordinate
    Vec3d sunPos = ssystem->getSun()->getAltAzPos(nav);

    // Compute the moon position in local coordinate
    Vec3d moonPos = ssystem->getMoon()->getAltAzPos(nav);
    atmosphere->computeColor(nav->getJDay(),
                             sunPos,
                             moonPos,
                             ssystem->getMoon()->getPhase(ssystem->getEarth()->getHeliocentricEclipticPos()),
                             StelApp::getInstance().getCore(),
                             nav->getCurrentLocation().latitude,
                             nav->getCurrentLocation().altitude,
                             15.f, 40.f);	// Temperature = 15c, relative humidity = 40%

    StelApp::getInstance().getCore()->getSkyDrawer()->reportLuminanceInFov(3.75+atmosphere->getAverageLuminance()*3.5, true);

    // Compute the ground luminance based on every planets around
    //	float groundLuminance = 0;
    //	const vector<Planet*>& allPlanets = ssystem->getAllPlanets();
    //	for (vector<Planet*>::const_iterator i=allPlanets.begin();i!=allPlanets.end();++i)
    //	{
    //		Vec3d pos = (*i)->getAltAzPos(nav);
    //		pos.normalize();
    //		if (pos[2] <= 0)
    //		{
    //			// No need to take this body into the landscape illumination computation
    //			// because it is under the horizon
    //		}
    //		else
    //		{
    //			// Compute the Illuminance E of the ground caused by the planet in lux = lumen/m^2
    //			float E = pow10(((*i)->get_mag(nav)+13.988)/-2.5);
    //			//qDebug() << "mag=" << (*i)->get_mag(nav) << " illum=" << E;
    //			// Luminance in cd/m^2
    //			groundLuminance += E/0.44*pos[2]*pos[2]; // 1m^2 from 1.5 m above the ground is 0.44 sr.
    //		}
    //	}
    //	groundLuminance*=atmosphere->getFadeIntensity();
    //	groundLuminance=atmosphere->getAverageLuminance()/50;
    //	qDebug() << "Atmosphere lum=" << atmosphere->getAverageLuminance() << " ground lum=" <<  groundLuminance;
    //	qDebug() << "Adapted Atmosphere lum=" << eye->adaptLuminance(atmosphere->getAverageLuminance()) << " Adapted ground lum=" << eye->adaptLuminance(groundLuminance);

    // compute global ground brightness in a simplistic way, directly in RGB
    float landscapeBrightness = 0;
    sunPos.normalize();
    moonPos.normalize();

    // We define the brigthness zero when the sun is 8 degrees below the horizon.
    float sinSunAngleRad = sin(qMin(M_PI_2, asin(sunPos[2])+8.*M_PI/180.));

    if(sinSunAngleRad < -0.1/1.5 )
        landscapeBrightness = 0.01;
    else
        landscapeBrightness = (0.01 + 1.5*(sinSunAngleRad+0.1/1.5));
    if (moonPos[2] > -0.1/1.5)
        landscapeBrightness += qMax(0.2/-12.*ssystem->getMoon()->getVMagnitude(nav),0.)*moonPos[2];

    // TODO make this more generic for non-atmosphere planets
    if(atmosphere->getFadeIntensity() == 1)
    {
        // If the atmosphere is on, a solar eclipse might darken the sky
        // otherwise we just use the sun position calculation above
        landscapeBrightness *= (atmosphere->getRealDisplayIntensityFactor()+0.1);
    }

    // TODO: should calculate dimming with solar eclipse even without atmosphere on
    landscape->setBrightness(landscapeBrightness+0.05);
}

void LandscapeMgr::draw(StelCore* core)
{

    // Draw the atmosphere
    if(!StelApp::getInstance().isVideoMode)
        atmosphere->draw(core);

    // Draw the landscape
    landscape->draw(core);

    // Draw the cardinal points
    if((!StelApp::getInstance().isVideoMode) && (!StelMainWindow::getInstance().getStellaHide()))
        cardinalsPoints->draw(core, StelApp::getInstance().getCore()->getNavigator()->getCurrentLocation().latitude);
}

void LandscapeMgr::init()
{
    QSettings* conf = StelApp::getInstance().getSettings();
    Q_ASSERT(conf);

    atmosphere = new Atmosphere();
    landscape = new LandscapeOldStyle();
    defaultLandscapeID = conf->value("init_location/landscape_name").toString();
    setCurrentLandscapeID(defaultLandscapeID, true);
    setFlagLandscape(conf->value("landscape/flag_landscape", conf->value("landscape/flag_ground", true).toBool()).toBool());
    setFlagFog(conf->value("landscape/flag_fog",true).toBool());
    setFlagAtmosphere(conf->value("landscape/flag_atmosphere").toBool());
    setAtmosphereFadeDuration(conf->value("landscape/atmosphere_fade_duration",0.5).toFloat());
    setAtmosphereLightPollutionLuminance(conf->value("viewing/light_pollution_luminance",0.0).toFloat());
    cardinalsPoints = new Cardinals();
    cardinalsPoints->setFlagShow(conf->value("viewing/flag_cardinal_points",true).toBool());
    setFlagLandscapeSetsLocation(conf->value("landscape/flag_landscape_sets_location",false).toBool());

    bool ok =true;
    setAtmosphereBortleLightPollution(conf->value("stars/init_bortle_scale",3).toInt(&ok));
    if (!ok)
    {
        conf->setValue("stars/init_bortle_scale",3);
        setAtmosphereBortleLightPollution(3);
        ok = true;
    }
    connect(this, SIGNAL(requestSetCurrentLandscapeID(const QString&)), this, SLOT(doSetCurrentLandscapeID(const QString&)));
    connect(this, SIGNAL(requestSetCurrentLandscapeName(const QString&)), this, SLOT(doSetCurrentLandscapeName(const QString&)));


    //todo
    vop_curr = new VideoClass(this);
    //vop_curr->useYUVData = StelApp::getInstance().getUseGLShaders();

    //QObject::connect(vop_curr, SIGNAL(frameReady(int)), this, SLOT(updateFrame(int)));
    QObject::connect(vop_curr, SIGNAL(preparedBuffer(QByteArray)), this, SLOT(updateFrame(QByteArray)));
    //if(!StelMainWindow::getInstance().getIsServer())
    QObject::connect(vop_curr, SIGNAL(finished()), this, SLOT(doStopVideo()));

    //vop_curr->setMarkIn(0);
    //vop_curr->setOptionRestartToMarkIn(true);

    connect(this, SIGNAL(requestSetCurrentVideoLandscapeName(const QString&)), this, SLOT(doSetCurrentLandscapetoVideo(const QString&)));

}

void LandscapeMgr::setStelStyle(const StelStyle& style)
{
    // Load colors from config file
    QSettings* conf = StelApp::getInstance().getSettings();
    QString section = style.confSectionName;

    QString defaultColor = conf->value("color/default_color").toString();
    setColorCardinalPoints(StelUtils::strToVec3f(conf->value(section+"/cardinal_color", defaultColor).toString()));
}

bool LandscapeMgr::setCurrentLandscapeID(const QString& id, bool inThread)
{
    if (inThread)
        return doSetCurrentLandscapeID(id);
    else
    {
        emit(requestSetCurrentLandscapeID(id));
        return true;
    }
}

bool LandscapeMgr::setCurrentLandscapeName(const QString& name, bool inThread,bool isVideo)
{
    //doClearLandscapeVideo();
    QMap<QString,QString> nameToDirMap = getNameToDirMap(isVideo);
    if (nameToDirMap.find(name)!=nameToDirMap.end())
    {
        if (inThread)
            return setCurrentLandscapeID(nameToDirMap[name], true);
        else
        {
            if(!isVideo)
            {
                //QMessageBox::critical(0,"",name,0,0);
                //doClearLandscapeVideo();
                StelApp::getInstance().isVideoMode = false;
                StelApp::getInstance().isVideoLandscape = false;
                emit(requestSetCurrentLandscapeName(name));
            }
            else
            {
                StelApp::getInstance().isVideoLandscape = true;
                isVideoLandscape = true;
                s_Name = name;
                emit(requestSetCurrentVideoLandscapeName(nameToDirMap[name]));
            }
            return true;
        }
    }
    else
    {
        qWarning() << "Can't find a landscape with name=" << name << endl;
        return false;
    }
}

// Change the default landscape to the landscape with the ID specified.
bool LandscapeMgr::setDefaultLandscapeID(const QString& id)
{
    if (id.isEmpty())
        return false;
    defaultLandscapeID = id;
    QSettings* conf = StelApp::getInstance().getSettings();
    conf->setValue("init_location/landscape_name", id);
    return true;
}

//! Load a landscape based on a hash of parameters mirroring the landscape.ini file
//! and make it the current landscape
bool LandscapeMgr::loadLandscape(QMap<QString, QString>& param)
{
    Landscape* newLandscape = createFromHash(param);
    if (!newLandscape)
        return false;

    if (landscape)
    {
        // Copy parameters from previous landscape to new one
        newLandscape->setFlagShow(landscape->getFlagShow());
        newLandscape->setFlagShowFog(landscape->getFlagShowFog());
        delete landscape;
        landscape = newLandscape;
    }
    currentLandscapeID = param["name"];
    // probably not particularly useful, as not in landscape.ini file

    return true;
}

void LandscapeMgr::updateI18n()
{
    // Translate all labels with the new language
    if (cardinalsPoints) cardinalsPoints->updateI18n();
}

void LandscapeMgr::setFlagLandscape(bool b)
{
    if(dynamic_cast<LandscapeVideo*>(landscape))
        if(!dynamic_cast<LandscapeVideo*>(landscape)->isLandscape)
            b= true;
    landscape->setFlagShow(b);
    StelApp::getInstance().addNetworkCommand("LandscapeMgr.setFlagLandscape("+QString("%0").arg(b)+");");
}

bool LandscapeMgr::getFlagLandscape(void) const
{
    return landscape->getFlagShow();
}

void LandscapeMgr::setFlagFog(bool b)
{
    landscape->setFlagShowFog(b);
    StelApp::getInstance().addNetworkCommand("LandscapeMgr.setFlagFog("+QString("%0").arg(b)+");");
}

bool LandscapeMgr::getFlagFog(void) const
{
    return landscape->getFlagShowFog();
}

/*********************************************************************
 Retrieve list of the names of all the available landscapes
 *********************************************************************/
QStringList LandscapeMgr::getAllLandscapeNames(bool isVideo) const
{
    QMap<QString,QString> nameToDirMap = getNameToDirMap(isVideo);
    QStringList result;

    // We just look over the map of names to IDs and extract the keys
    foreach (QString i, nameToDirMap.keys())
    {
        result += i;
    }
    return result;
}

QStringList LandscapeMgr::getAllLandscapeIDs() const
{
    QMap<QString,QString> nameToDirMap = getNameToDirMap(false);
    QStringList result;

    // We just look over the map of names to IDs and extract the keys
    foreach (QString i, nameToDirMap.values())
    {
        result += i;
    }
    return result;
}

QString LandscapeMgr::getCurrentLandscapeName() const
{
    return landscape->getName();
}

QString LandscapeMgr::getCurrentLandscapeHtmlDescription() const
{
    QString desc = QString("<h3>%1</h3>").arg(landscape->getName());
    desc += landscape->getDescription();
    desc+="<br><br>";
    desc+="<b>"+q_("Author: ")+"</b>";
    desc+=landscape->getAuthorName();
    desc+="<br>";
    desc+="<b>"+q_("Location: ")+"</b>";
    if (landscape->getLocation().longitude>-500.0 && landscape->getLocation().latitude>-500.0)
    {
        desc += StelUtils::radToDmsStrAdapt(landscape->getLocation().longitude * M_PI/180.);
        desc += "/" + StelUtils::radToDmsStrAdapt(landscape->getLocation().latitude *M_PI/180.);
        desc += QString(q_(", %1 m")).arg(landscape->getLocation().altitude);
        if (landscape->getLocation().planetName!="")
        {
            desc += "<br><b>"+q_("Planet: ")+"</b>"+landscape->getLocation().planetName;
        }
        desc += "<br><br>";
    }
    return desc;
}

//! Set flag for displaying Cardinals Points
void LandscapeMgr::setFlagCardinalsPoints(bool b)
{
    cardinalsPoints->setFlagShow(b);
    StelApp::getInstance().addNetworkCommand("LandscapeMgr.setFlagCardinalsPoints("+QString("%0").arg(b)+");");
}

//! Get flag for displaying Cardinals Points
bool LandscapeMgr::getFlagCardinalsPoints(void) const
{
    return cardinalsPoints->getFlagShow();
}

//! Set Cardinals Points color
void LandscapeMgr::setColorCardinalPoints(const Vec3f& v)
{
    cardinalsPoints->setColor(v);
}

//! Get Cardinals Points color
Vec3f LandscapeMgr::getColorCardinalPoints(void) const
{
    return cardinalsPoints->get_color();
}

///////////////////////////////////////////////////////////////////////////////////////
// Atmosphere
//! Set flag for displaying Atmosphere
void LandscapeMgr::setFlagAtmosphere(bool b)
{
    atmosphere->setFlagShow(b);
    StelApp::getInstance().addNetworkCommand("LandscapeMgr.setFlagAtmosphere("+QString("%0").arg(b)+");");
}

//! Get flag for displaying Atmosphere
bool LandscapeMgr::getFlagAtmosphere(void) const
{
    return atmosphere->getFlagShow();
}

//! Set atmosphere fade duration in s
void LandscapeMgr::setAtmosphereFadeDuration(float f)
{
    atmosphere->setFadeDuration(f);
}

//! Get atmosphere fade duration in s
float LandscapeMgr::getAtmosphereFadeDuration(void) const
{
    return atmosphere->getFadeDuration();
}

//! Set light pollution luminance level
void LandscapeMgr::setAtmosphereLightPollutionLuminance(double f)
{
    atmosphere->setLightPollutionLuminance(f);
    StelApp::getInstance().addNetworkCommand("LandscapeMgr.setAtmosphereLightPollutionLuminance("+QString("%0").arg(f)+");");
}

//! Get light pollution luminance level
double LandscapeMgr::getAtmosphereLightPollutionLuminance(void) const
{
    return atmosphere->getLightPollutionLuminance();
}

//! Set the light pollution following the Bortle Scale
void LandscapeMgr::setAtmosphereBortleLightPollution(int bIndex)
{
    // This is an empirical formula
    setAtmosphereLightPollutionLuminance(qMax(0.,0.0020*std::pow(bIndex-1, 2.1)));
}

//! Get the light pollution following the Bortle Scale
int LandscapeMgr::getAtmosphereBortleLightPollution(void)
{
    return (int)std::pow(getAtmosphereLightPollutionLuminance()/0.0020, 1./2.1) + 1;
}

void LandscapeMgr::setZRotation(double d)
{
    if (landscape)
        landscape->setZRotation(d);
}

float LandscapeMgr::getLuminance(void)
{
    return atmosphere->getRealDisplayIntensityFactor();
}

Landscape* LandscapeMgr::createFromFile(const QString& landscapeFile, const QString& landscapeId)
{
    QSettings landscapeIni(landscapeFile, StelIniFormat);
    QString s;
    if (landscapeIni.status() != QSettings::NoError)
    {
        qWarning() << "ERROR parsing landscape.ini file: " << landscapeFile;
        s = "";
    }
    else
        s = landscapeIni.value("landscape/type").toString();

    Landscape* ldscp = NULL;
    if (s=="old_style")
        ldscp = new LandscapeOldStyle();
    else if (s=="spherical")
        ldscp = new LandscapeSpherical();
    else if (s=="fisheye")
        ldscp = new LandscapeFisheye();
    else
    {
        qDebug() << "Unknown landscape type: \"" << s << "\"";

        // to avoid making this a fatal error, will load as a fisheye
        // if this fails, it just won't draw
        ldscp = new LandscapeFisheye();
    }

    ldscp->load(landscapeIni, landscapeId);
    return ldscp;
}

Landscape* LandscapeMgr::createFromHash(QMap<QString, QString>& param)
{
    // NOTE: textures should be full filename (and path)
    if (param["type"]=="old_style")
    {
        LandscapeOldStyle* ldscp = new LandscapeOldStyle();
        ldscp->create(1, param);
        return ldscp;
    }
    else if (param["type"]=="spherical")
    {
        LandscapeSpherical* ldscp = new LandscapeSpherical();
        ldscp->create(param["name"], 1, param["path"] + param["maptex"],param["angleRotateZ"].toDouble());
        return ldscp;
    }
    else
    {   //	if (s=="fisheye")
        LandscapeFisheye* ldscp = new LandscapeFisheye();
        ldscp->create(param["name"], 1, param["path"] + param["maptex"],
                param["texturefov"].toDouble(),
                param["angleRotateZ"].toDouble());
        return ldscp;
    }
}

QString LandscapeMgr::nameToID(const QString& name)
{
    QMap<QString,QString> nameToDirMap = getNameToDirMap(false);

    if (nameToDirMap.find(name)!=nameToDirMap.end())
    {
        Q_ASSERT(0);
        return "error";
    }
    else
    {
        return nameToDirMap[name];
    }
}

/****************************************************************************
 get a map of landscape name (from landscape.ini name field) to ID (dir name)
 ****************************************************************************/
QMap<QString,QString> LandscapeMgr::getNameToDirMap(bool isVideo) const
{
    QSet<QString> landscapeDirs;
    QMap<QString,QString> result;
    try
    {
        landscapeDirs = StelFileMgr::listContents("landscapes",StelFileMgr::Directory);
    }
    catch (std::runtime_error& e)
    {
        qDebug() << "ERROR while trying list landscapes:" << e.what();
    }

    foreach (const QString& dir, landscapeDirs)
    {
        try
        {
            QSettings landscapeIni(StelFileMgr::findFile("landscapes/" + dir + "/landscape.ini"), StelIniFormat);
            QString k = landscapeIni.value("landscape/name").toString();
            //ASAF
            QString t = landscapeIni.value("landscape/type").toString();
            if(!isVideo && t != "fisheyevideo" && t != "sphericalvideo")
                result[k] = dir;
            else if(isVideo && (t=="fisheyevideo" || t == "sphericalvideo"))
                result[k] = qApp->applicationDirPath()+"/landscapes/" + dir +"/"+ landscapeIni.value("landscape/filename").toString(); //dir;
            //ASAF
        }
        catch (std::runtime_error& e)
        {
            //qDebug << "WARNING: unable to successfully read landscape.ini file from landscape " << dir;
        }
    }
    return result;
}

bool LandscapeMgr::doSetCurrentLandscapeID(QString id)
{
    if (id.isEmpty())
    {
        emit(requestCompleteSetCurrentLandscapeID(false));
        return false;
    }

    // We want to lookup the landscape ID (dir) from the name.
    Landscape* newLandscape = NULL;

    try
    {
        newLandscape = createFromFile(StelFileMgr::findFile("landscapes/" + id + "/landscape.ini"), id);
    }
    catch (std::runtime_error& e)
    {
        qWarning() << "ERROR while loading default landscape " << "landscapes/" + id + "/landscape.ini" << ", (" << e.what() << ")";
    }

    if (!newLandscape)
    {
        emit(requestCompleteSetCurrentLandscapeID(false));
        return false;
    }

    if (landscape)
    {
        // Copy display parameters from previous landscape to new one
        newLandscape->setFlagShow(landscape->getFlagShow());
        newLandscape->setFlagShowFog(landscape->getFlagShowFog());
        delete landscape;
        landscape = newLandscape;
    }
    currentLandscapeID = id;

    if (getFlagLandscapeSetsLocation())
    {
        StelApp::getInstance().getCore()->getNavigator()->moveObserverTo(landscape->getLocation());
    }

    emit(requestCompleteSetCurrentLandscapeID(true));
    return true;
}

bool LandscapeMgr::doSetCurrentLandscapeName(const QString& name)
{
    if (name.isEmpty())
    {
        emit(requestCompleteSetCurrentLandscapeName(false));
        return false;
    }

    QMap<QString,QString> nameToDirMap = getNameToDirMap(false);
    if (nameToDirMap.find(name)!=nameToDirMap.end())
    {
        bool result = setCurrentLandscapeID(nameToDirMap[name], true);
        emit(requestCompleteSetCurrentLandscapeName(result));
        return result;
    }
    else
    {
        qWarning() << "Can't find a landscape with name=" << name << endl;
        emit(requestCompleteSetCurrentLandscapeName(false));
        return false;
    }
}

Landscape* LandscapeMgr::createFromFulldomeVideo(VideoClass* vop,QString _name)
{
    LandscapeVideo* ldscp = NULL;
    ldscp = new LandscapeVideo();
    ldscp->vbrightness = vbrightness;
    ldscp->vcontrast = vcontrast;
    ldscp->vsaturation = vsaturation;

    ldscp->isLandscape = false;
    float fov = -180.0;
    float rotatez = -90.0;

    if(_name !="")
    {
        ldscp->isLandscape = true;
        QSettings landscapeIni(StelFileMgr::findFile("landscapes/" + _name+ "/landscape.ini"), StelIniFormat);
        ldscp->load(landscapeIni, _name);
        fov = landscapeIni.value("landscape/texturefov", -180.0).toFloat();
        rotatez = landscapeIni.value("landscape/angle_rotatez", -90.0).toFloat();
    }
    else
    {
        QSettings* conf;
        conf = StelApp::getInstance().getSettings();
        fov = conf->value("landscape/video_texturefov", -180.0).toFloat();
        rotatez = conf->value("landscape/video_angle_rotatez", -90.0).toFloat();
        //QMessageBox::critical(0,"",QString("%0").arg(rotatez),0,0);
    }
    //ldscp->loadvop(vop,_name,fov,rotatez);
    ldscp->loadvop(_name,fov,rotatez);
    return ldscp;
}

bool LandscapeMgr::doSetCurrentLandscapetoVideo(const QString& fulldomeFile)
{
    Landscape* newLandscape = NULL;
    try
    {
        vop_curr->OpenVideo(fulldomeFile);

        newLandscape = createFromFulldomeVideo(vop_curr,s_Name);
        //dynamic_cast<LandscapeVideo*>(newLandscape)->vp = vop_curr->getPictureAtIndex(-1);
        dynamic_cast<LandscapeVideo*>(newLandscape)->init(vop_curr->GetVideoSize());

        if(isVideoLandscape)
        {
            //vop_curr->setLoop(true);
            videoFinished = false;
            doStartVideo();
        }
        else
        {
            videoFinished = false;
            //vop_curr->setLoop(false);
            //vop_curr->setOptionRevertToBlackWhenStop(false);
        }
        bool result = setCurrentLandscapeID(s_Name, true);
        emit(requestCompleteSetCurrentLandscapeName(result));
    }
    catch (std::runtime_error& e)
    {
        qWarning() << "ERROR while loading default landscape , (" << e.what() << ")";
    }

    if (!newLandscape)
    {
        emit(requestCompleteSetCurrentLandscapeID(false));
        return false;
    }

    if (landscape)
    {
        // Copy display parameters from previous landscape to new one
        newLandscape->setFlagShow(landscape->getFlagShow());
        newLandscape->setFlagShowFog(landscape->getFlagShowFog());
        //delete landscape;
        landscape = newLandscape;
    }
    return true;
}

bool LandscapeMgr::doClearLandscapeVideo()
{
    if(vop_curr != NULL)
    {
        if(!vop_curr->IsFinished())
        {
            vop_curr->StopVideo();

            StelMainGraphicsView::getInstance().startLoopTimer(true);
            StelApp::getInstance().isVideoMode = false;
            if(StelMainWindow::getInstance().getIsServer())
            {
                StelGui* sgui = dynamic_cast<StelGui*>(StelApp::getInstance().getGui());
                sgui->startAutoSave(true);
            }
        }
    }
    return true;
}
void LandscapeMgr::doStopVideo()
{
    //QMessageBox::information(0,0,"deneme",0,0);
    //vop_curr->stop();
    //#ifdef SHIRAPLAYER_PRE
    //qDebug()<<"video client da oto stop edildi";
    if(vop_curr)
    {
        //if(vop_curr->IsInternalTimer())
        {
            //if (!videoFinished)
            {
                vop_curr->CloseVideo();
                videoFinished = true;
            }
        }
        /*else
        {
            vop_curr->CloseVideo();
            videoFinished = true;
        }*/
    }
    if(!StelMainWindow::getInstance().getIsServer())
        if(!StelMainWindow::getInstance().getIsMultiprojector())
        {
            if(!fromScriptVideo)
            {
                Sleep(500);
                //qDebug()<<"servera oto stop gönderiliyor";
                LoadIntoSharedMomory(QString("%1@%2@%3@%4@%5@%6")
                                     .arg("STOP")
                                     .arg("0")
                                     .arg("0")
                                     .arg("0")
                                     .arg("0")
                                     .arg("0"));
            }
            //qDebug()<<"servera oto stop gönderildi";
        }
    //#endif
}
void  LandscapeMgr::updateFrame (QByteArray buffer)
{
    /*const double now = StelApp::getTotalRunTime();
    double dt = now-previousPaintTime;
    previousPaintTime = now;*/

    if(!dynamic_cast<LandscapeVideo*>(landscape))
        return;

    //QCoreApplication::processEvents(QEventLoop::EventLoopExec);

    //StelMainGraphicsView::getInstance().setUpdatesEnabled(true);
    //dynamic_cast<LandscapeVideo*>(landscape)->bufferIndex = i;
    dynamic_cast<LandscapeVideo*>(landscape)->vp = buffer;
    dynamic_cast<LandscapeVideo*>(landscape)->frameChanged = true;
    //StelMainGraphicsView::getInstance().update();
    //dynamic_cast<LandscapeVideo*>(landscape)->update(dt);
    //StelMainGraphicsView::getInstance().getOpenGLWin()->repaint();
    //StelMainGraphicsView::getInstance().getOpenGLWin()->updateGL();

    //StelMainGraphicsView::getInstance().getOpenGLWin()->setUpdatesEnabled(true);



    //30 Fps ve altý
    //if (vop_curr->GetVideFrameRate()<= 30)
    //    StelMainGraphicsView::getInstance().getOpenGLWin()->repaint();
    //else//30fps den yukarý
    StelMainGraphicsView::getInstance().getOpenGLWin()->makeCurrent();
    StelMainGraphicsView::getInstance().getOpenGLWin()->update();

    //StelApp::getInstance().update(dt);
    //StelMainGraphicsView::getInstance().setUpdatesEnabled(false);
    /*    QDateTime current = QDateTime::currentDateTime();
        qDebug()<<pcurrent.msecsTo(current);
        pcurrent = current;*/

    if(!StelMainWindow::getInstance().getIsMultiprojector())
    {
        //todo
        if ( (!vop_curr->IsPaused()) && (!videoFinished))
            LoadIntoSharedMomory(QString("%1@%2@%3@%4@%5@%6")
                                 .arg(vop_curr->GetTimeStrFromFrame(vop_curr->GetCurrentFrameNumber()))
                                 .arg(vop_curr->GetTimeStrFromFrame(vop_curr->GetVideoDuration()))
                                 .arg(vop_curr->GetCurrentFrameNumber())
                                 .arg(vop_curr->GetVideoDuration())
                                 .arg(vop_curr->GetVideFrameRate())
                                 .arg(vop_curr->GetFreeBufferCount()));
    }


}
void  LandscapeMgr::doStartVideo()
{
    if(vop_curr)
    {
        vop_curr->StartVideo();

        if (vop_curr->HasEmbedAudio())
        {
            QSettings settings("Sureyyasoft", "ShiraPlayer");
            settings.beginGroup("Others");
            if(!settings.value("DontAskAudioWarn",false).toBool())
            {
                QStringList strList = vop_curr->GetFileName().split("/");

                if(StelMainWindow::getInstance().getIsServer())
                {
                    EmbedAudioWarning* emb = new EmbedAudioWarning(0,
                                                                   strList.at(strList.count()-1));
                    emb->showNormal();
                }
                else
                {
                    LoadIntoSharedMomory(QString("%1@%2@%3@%4@%5@%6")
                                         .arg("WARNING")
                                         .arg(strList.at(strList.count()-1))
                                         .arg("0")
                                         .arg("0")
                                         .arg("0")
                                         .arg("0"));
                }
            }
        }
    }

}
void LandscapeMgr::doPauseVideo(bool val)
{
    if(vop_curr != NULL)
        vop_curr->PauseVideo(val,true);
}

Landscape* LandscapeMgr::createFromFrame(const QString& frameFile,QString oldName)
{
    LandscapeFisheye* ldscp = NULL;
    ldscp = new LandscapeFisheye();
    ldscp->loadFrame(QString(frameFile),oldName);
    return ldscp;
}
void LandscapeMgr::setshowDaylight(bool value)
{
    try {
        if (dynamic_cast<LandscapeFisheye*>(landscape) != 0)
            dynamic_cast<LandscapeFisheye*>(landscape)->setShowDaylight(value);
    }
    catch(std::runtime_error &e){}
}

bool LandscapeMgr::doSetCurrentLandscapetoFrame(const QString& frameFile, QString oldName, bool showDaylight, int faderDuration)
{
    // We want to lookup the landscape ID (dir) from the name.
    Landscape* newLandscape = NULL;

    try
    {
        newLandscape = createFromFrame(frameFile,oldName);
        dynamic_cast<LandscapeFisheye*>(newLandscape)->setShowDaylight(showDaylight);
        dynamic_cast<LandscapeFisheye*>(newLandscape)->setFaderDuration(faderDuration);
    }
    catch (std::runtime_error& e)
    {
        qWarning() << "ERROR while loading default landscape , (" << e.what() << ")";
    }

    if (!newLandscape)
    {
        emit(requestCompleteSetCurrentLandscapeID(false));
        return false;
    }

    if (landscape)
    {
        // Copy display parameters from previous landscape to new one
        newLandscape->setFlagShow(landscape->getFlagShow());
        newLandscape->setFlagShowFog(landscape->getFlagShowFog());
        delete landscape;
        landscape = newLandscape;
    }
    return true;
}
void LandscapeMgr::LoadIntoSharedMomory(QString text)
{
    if (videoSharedMem.isAttached())
    {
        videoSharedMem.detach();
    }
    if(text.length())
    {
        // load into shared memory
        QBuffer buffer;
        buffer.open(QBuffer::ReadWrite);
        QDataStream out(&buffer);
        out << text;
        int size = buffer.size();

        if (!videoSharedMem.create(size)) {
            return;
        }
        videoSharedMem.lock();
        char *to = (char*)videoSharedMem.data();
        const char *from = buffer.data().data();
        memcpy(to, from, qMin(videoSharedMem.size(), size));
        videoSharedMem.unlock();
    }

}

void LandscapeMgr::setVideoBrightness(double val)
{
    if(!dynamic_cast<LandscapeVideo*>(landscape))
        return;
    vbrightness = val;
    dynamic_cast<LandscapeVideo*>(landscape)->vbrightness = val;
}

void LandscapeMgr::setVideoContrast(double val)
{
    if(!dynamic_cast<LandscapeVideo*>(landscape))
        return;
    vcontrast = val;
    dynamic_cast<LandscapeVideo*>(landscape)->vcontrast = val;
}

void LandscapeMgr::setVideoSaturation(double val)
{
    if(!dynamic_cast<LandscapeVideo*>(landscape))
        return;
    vsaturation = val;
    dynamic_cast<LandscapeVideo*>(landscape)->vsaturation = val;
}
