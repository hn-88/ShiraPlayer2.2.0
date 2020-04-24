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

#include "SolarSystem.hpp"
#include "StelTexture.hpp"
#include "stellplanet.h"
#include "Orbit.hpp"
#include "StelNavigator.hpp"
#include "StelProjector.hpp"
#include "StelApp.hpp"
#include "StelCore.hpp"
#include "StelTextureMgr.hpp"
#include "StelObjectMgr.hpp"
#include "StelLocaleMgr.hpp"
#include "StelSkyCultureMgr.hpp"
#include "StelFileMgr.hpp"
#include "StelModuleMgr.hpp"
#include "StelIniParser.hpp"
#include "Planet.hpp"
#include "StelNavigator.hpp"
#include "StelSkyDrawer.hpp"
#include "StelStyle.hpp"
#include "StelUtils.hpp"
#include "StelPainter.hpp"
#include "TrailGroup.hpp"
#include "StelMainWindow.hpp"

#include <functional>
#include <algorithm>

#include <QTextStream>
#include <QSettings>
#include <QVariant>
#include <QString>
#include <QStringList>
#include <QMap>
#include <QMultiMap>
#include <QMapIterator>
#include <QDebug>

SolarSystem::SolarSystem() : moonScale(1.),
    flagOrbits(false),
    flagLightTravelTime(false),
    allTrails(NULL),
    flagflybyAtm(true),
    flagAxises(false)
{
    planetNameFont.setPixelSize(13);
    setObjectName("SolarSystem");
}

void SolarSystem::setFontSize(float newFontSize)
{
    planetNameFont.setPixelSize(newFontSize);
}

SolarSystem::~SolarSystem()
{
    // release selected:
    selected.clear();
    foreach (Orbit* orb, orbits)
    {
        delete orb;
        orb = NULL;
    }
    sun.clear();
    moon.clear();
    earth.clear();
    //ASAF
    venus.clear();
    mars.clear();
    saturn.clear();
    jupiter.clear();
    merkur.clear();
    uranus.clear();
    neptun.clear();
    pluto.clear();
    //ASAF
    Planet::hintCircleTex.clear();
    Planet::texEarthShadow.clear();
    Planet::texAtmosphere.clear();

    delete allTrails;
    allTrails = NULL;
}

/*************************************************************************
 Reimplementation of the getCallOrder method
*************************************************************************/
double SolarSystem::getCallOrder(StelModuleActionName actionName) const
{
    if (actionName==StelModule::ActionDraw)
        return StelApp::getInstance().getModuleMgr().getModule("StarMgr")->getCallOrder(actionName)+10;
    return 0;
}

// Init and load the solar system data
void SolarSystem::init()
{
    QSettings* conf = StelApp::getInstance().getSettings();
    Q_ASSERT(conf);

    loadPlanets();	// Load planets data
    loadInternalPlanets(); // Load Internal planets data

    // Compute position and matrix of sun and all the satellites (ie planets)
    // for the first initialization Q_ASSERT that center is sun center (only impacts on light speed correction)
    computePositions(StelUtils::getJDFromSystem());

    setSelected("");	// Fix a bug on macosX! Thanks Fumio!
    setFlagMoonScale(conf->value("viewing/flag_moon_scaled", conf->value("viewing/flag_init_moon_scaled", "false").toBool()).toBool());  // name change
    setMoonScale(conf->value("viewing/moon_scale", 5.0).toDouble());
    setFlagPlanets(conf->value("astro/flag_planets").toBool());
    setFlagHints(conf->value("astro/flag_planets_hints").toBool());
    setFlagLabels(conf->value("astro/flag_planets_labels", true).toBool());
    setFlagSelectedLabel(conf->value("astro/flag_planet_selected_label", true).toBool());
    setLabelsAmount(conf->value("astro/labels_amount", 3.).toDouble());
    setFlagOrbits(conf->value("astro/flag_planets_orbits").toBool());
    setFlagAxises(conf->value("astro/flag_planets_axises").toBool());
    setFlagEquline(conf->value("astro/flag_planets_equatorline").toBool());
    setFlagLightTravelTime(conf->value("astro/flag_light_travel_time", false).toBool());
    //ASAF
    setFlagPlanetsScale(conf->value("viewing/flag_planet_scaled",false).toBool());
    setMerkurScale(conf->value("viewing/merkur_scale", 5.0).toDouble());
    setVenusScale(conf->value("viewing/venus_scale", 5.0).toDouble());
    setMarsScale(conf->value("viewing/mars_scale", 5.0).toDouble());
    setSaturnScale(conf->value("viewing/saturn_scale", 5.0).toDouble());
    setJupiterScale(conf->value("viewing/jupiter_scale", 5.0).toDouble());
    setPlutoScale(conf->value("viewing/pluto_scale", 5.0).toDouble());
    setNeptunScale(conf->value("viewing/neptun_scale", 5.0).toDouble());
    setUranusScale(conf->value("viewing/uranus_scale", 5.0).toDouble());
    setEarthScale(conf->value("viewing/earth_scale", 5.0).toDouble());
    //ASAF

    recreateTrails();

    setFlagTrails(conf->value("astro/flag_object_trails", false).toBool());

    planetNameFont.setPixelSize(conf->value("viewing/planet_font_size",13).toInt());

    GETSTELMODULE(StelObjectMgr)->registerStelObjectMgr(this);
    texPointer = StelApp::getInstance().getTextureManager().createTexture("textures/pointeur4.png");
    Planet::hintCircleTex = StelApp::getInstance().getTextureManager().createTexture("textures/planet-indicator.png");
}

void SolarSystem::recreateTrails()
{
    // Create a trail group containing all the planets orbiting the sun (not including satellites)
    if (allTrails!=NULL)
        delete allTrails;
    allTrails = new TrailGroup(365.f);
    foreach (const PlanetP& p, getSun()->satellites)
    {
        allTrails->addObject((QSharedPointer<StelObject>)p, &trailColor);
    }
}

void SolarSystem::drawPointer(const StelCore* core)
{
    const StelNavigator* nav = core->getNavigator();
    const StelProjectorP prj = core->getProjection(StelCore::FrameJ2000);

    const QList<StelObjectP> newSelected = GETSTELMODULE(StelObjectMgr)->getSelectedObject("Planet");
    if (!newSelected.empty())
    {
        const StelObjectP obj = newSelected[0];
        Vec3d pos=obj->getJ2000EquatorialPos(nav);
        Vec3d screenpos;
        // Compute 2D pos and return if outside screen
        if (!prj->project(pos, screenpos))
            return;


        StelPainter sPainter(prj);
        sPainter.setColor(1.0f,0.3f,0.3f);

        float size = obj->getAngularSize(core)*M_PI/180.*prj->getPixelPerRadAtCenter()*2.;
        size+=40.f + 10.f*std::sin(2.f * StelApp::getInstance().getTotalRunTime());

        texPointer->bind();

        sPainter.enableTexture2d(true);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // Normal transparency mode

        size*=0.5;
        const float angleBase = StelApp::getInstance().getTotalRunTime() * 10;
        // We draw 4 instances of the sprite at the corners of the pointer
        for (int i = 0; i < 4; ++i)
        {
            const float angle = angleBase + i * 90;
            const double x = screenpos[0] + size * cos(angle / 180 * M_PI);
            const double y = screenpos[1] + size * sin(angle / 180 * M_PI);
            sPainter.drawSprite2dMode(x, y, 10, angle);
        }

        //--Selected label
        if(getFlagSelectedLabel())
        {
            const StelNavigator* nav = core->getNavigator();
            const StelProjectorP prj = core->getProjection(StelCore::FrameJ2000);
            sPainter.setFont(planetNameFont);
            // Draw nameI18 + scaling if it's not == 1.
            Planet* pl = static_cast<Planet*>(obj.data());
            float tmp = 10.f + pl->getAngularSize(core)*M_PI/180.*prj->getPixelPerRadAtCenter()/1.44; // Shift for nameI18 printing
            sPainter.setColor(pl->labelColor[0], pl->labelColor[1], pl->labelColor[2]);
            sPainter.drawText(pl->screenPos[0],pl->screenPos[1], pl->getSkyLabel(nav), 0, tmp, tmp, false);
        }
        //---
    }
}

void ellipticalOrbitPosFunc(double jd,double xyz[3], void* userDataPtr)
{
    static_cast<EllipticalOrbit*>(userDataPtr)->positionAtTimevInVSOP87Coordinates(jd, xyz);
}
void cometOrbitPosFunc(double jd,double xyz[3], void* userDataPtr)
{
    static_cast<CometOrbit*>(userDataPtr)->positionAtTimevInVSOP87Coordinates(jd, xyz);
}

// Init and load the solar system data
void SolarSystem::loadPlanets()
{
    qDebug() << "Loading Solar System data ...";
    QString iniFile;
    try
    {
        iniFile = StelFileMgr::findFile("data/ssystem.ini");
    }
    catch(std::runtime_error& e)
    {
        qWarning() << "ERROR while loading ssysyem.ini (unable to find data/ssystem.ini): " << e.what() << endl;
        return;
    }
    QSettings pd(iniFile, StelIniFormat);
    if (pd.status() != QSettings::NoError)
    {
        qWarning() << "ERROR while parsing ssysyem.ini file";
        return;
    }

    // QSettings does not allow us to say that the sections of the file
    // will be listed in the same order  as in the file like the old
    // InitParser used to so we can no longer assume that.
    //
    // This means we must first decide what order to read the sections
    // of the file in (each section contains one planet) to avoid setting
    // the parent Planet* to one which has not yet been created.
    //
    // Stage 1: Make a map of body names back to the section names
    // which they come from. Also make a map of body name to parent body
    // name. These two maps can be made in a single pass through the
    // sections of the file.
    //
    // Stage 2: Make an ordered list of section names such that each
    // item is only ever dependent on items which appear earlier in the
    // list.
    // 2a: Make a QMultiMap relating the number of levels of dependency
    //     to the body name, i.e.
    //     0 -> Sun
    //     1 -> Mercury
    //     1 -> Venus
    //     1 -> Earth
    //     2 -> Moon
    //     etc.
    // 2b: Populate an ordered list of section names by iterating over
    //     the QMultiMap.  This type of contains is always sorted on the
    //     key, so it's easy.
    //     i.e. [sol, earth, moon] is fine, but not [sol, moon, earth]
    //
    // Stage 3: iterate over the ordered sections decided in stage 2,
    // creating the planet objects from the QSettings data.

    // Stage 1 (as described above).
    QMap<QString, QString> secNameMap;
    QMap<QString, QString> parentMap;
    QStringList sections = pd.childGroups();
    for (int i=0; i<sections.size(); ++i)
    {
        const QString secname = sections.at(i);
        const QString englishName = pd.value(secname+"/name").toString();
        const QString strParent = pd.value(secname+"/parent").toString();
        secNameMap[englishName] = secname;
        if (strParent!="none" && !strParent.isEmpty() && !englishName.isEmpty())
            parentMap[englishName] = strParent;
    }

    // Stage 2a (as described above).
    QMultiMap<int, QString> depLevelMap;
    for (int i=0; i<sections.size(); ++i)
    {
        const QString englishName = pd.value(sections.at(i)+"/name").toString();

        // follow dependencies, incrementing level when we have one
        // till we run out.
        QString p=englishName;
        int level = 0;
        while(parentMap.contains(p) && parentMap[p]!="none")
        {
            level++;
            p = parentMap[p];
        }

        depLevelMap.insert(level, secNameMap[englishName]);
    }

    // Stage 2b (as described above).
    QStringList orderedSections;
    QMapIterator<int, QString> levelMapIt(depLevelMap);
    while(levelMapIt.hasNext())
    {
        levelMapIt.next();
        orderedSections << levelMapIt.value();
    }

    // Stage 3 (as described above).
    int readOk=0;
    int totalPlanets=0;
    for (int i = 0;i<orderedSections.size();++i)
    {
        totalPlanets++;
        const QString secname = orderedSections.at(i);
        const QString englishName = pd.value(secname+"/name").toString();
        const QString strParent = pd.value(secname+"/parent").toString();
        PlanetP parent;
        if (strParent!="none")
        {
            // Look in the other planets the one named with strParent
            foreach (const PlanetP& p, systemPlanets)
            {
                if (p->getEnglishName()==strParent)
                {
                    parent = p;
                    break;
                }
            }
            if (parent.isNull())
            {
                qWarning() << "ERROR : can't find parent solar system body for " << englishName;
                abort();
                continue;
            }
        }

        const QString funcName = pd.value(secname+"/coord_func").toString();
        posFuncType posfunc=NULL;
        void* userDataPtr=NULL;
        OsulatingFunctType *osculatingFunc = 0;
        bool closeOrbit = pd.value(secname+"/closeOrbit", true).toBool();

        if (funcName=="ell_orbit")
        {
            // Read the orbital elements
            const double epoch = pd.value(secname+"/orbit_Epoch",J2000).toDouble();
            const double eccentricity = pd.value(secname+"/orbit_Eccentricity").toDouble();
            if (eccentricity >= 1.0) closeOrbit = false;
            double pericenterDistance = pd.value(secname+"/orbit_PericenterDistance",-1e100).toDouble();
            double semi_major_axis;
            if (pericenterDistance <= 0.0) {
                semi_major_axis = pd.value(secname+"/orbit_SemiMajorAxis",-1e100).toDouble();
                if (semi_major_axis <= -1e100) {
                    qDebug() << "ERROR: " << englishName
                             << ": you must provide orbit_PericenterDistance or orbit_SemiMajorAxis";
                    abort();
                } else {
                    semi_major_axis /= AU;
                    Q_ASSERT(eccentricity != 1.0); // parabolic orbits have no semi_major_axis
                    pericenterDistance = semi_major_axis * (1.0-eccentricity);
                }
            } else {
                pericenterDistance /= AU;
                semi_major_axis = (eccentricity == 1.0)
                        ? 0.0 // parabolic orbits have no semi_major_axis
                        : pericenterDistance / (1.0-eccentricity);
            }
            double meanMotion = pd.value(secname+"/orbit_MeanMotion",-1e100).toDouble();
            double period;
            if (meanMotion <= -1e100) {
                period = pd.value(secname+"/orbit_Period",-1e100).toDouble();
                if (period <= -1e100) {
                    meanMotion = (eccentricity == 1.0)
                            ? 0.01720209895 * (1.5/pericenterDistance)
                              * sqrt(0.5/pericenterDistance)
                            : (semi_major_axis > 0.0)
                              ? 0.01720209895 / (semi_major_axis*sqrt(semi_major_axis))
                              : 0.01720209895 / (-semi_major_axis*sqrt(-semi_major_axis));
                    period = 2.0*M_PI/meanMotion;
                } else {
                    meanMotion = 2.0*M_PI/period;
                }
            } else {
                period = 2.0*M_PI/meanMotion;
            }
            const double inclination = pd.value(secname+"/orbit_Inclination").toDouble()*(M_PI/180.0);
            const double ascending_node = pd.value(secname+"/orbit_AscendingNode").toDouble()*(M_PI/180.0);
            double arg_of_pericenter = pd.value(secname+"/orbit_ArgOfPericenter",-1e100).toDouble();
            double long_of_pericenter;
            if (arg_of_pericenter <= -1e100) {
                long_of_pericenter = pd.value(secname+"/orbit_LongOfPericenter").toDouble()*(M_PI/180.0);
                arg_of_pericenter = long_of_pericenter - ascending_node;
            } else {
                arg_of_pericenter *= (M_PI/180.0);
                long_of_pericenter = arg_of_pericenter + ascending_node;
            }
            double mean_anomaly = pd.value(secname+"/orbit_MeanAnomaly",-1e100).toDouble();
            double mean_longitude;
            if (mean_anomaly <= -1e100) {
                mean_longitude = pd.value(secname+"/orbit_MeanLongitude").toDouble()*(M_PI/180.0);
                mean_anomaly = mean_longitude - long_of_pericenter;
            } else {
                mean_anomaly *= (M_PI/180.0);
                mean_longitude = mean_anomaly + long_of_pericenter;
            }

            // when the parent is the sun use ecliptic rathe than sun equator:
            const double parentRotObliquity = parent->getParent()
                    ? parent->getRotObliquity()
                    : 0.0;
            const double parent_rot_asc_node = parent->getParent()
                    ? parent->getRotAscendingnode()
                    : 0.0;
            double parent_rot_j2000_longitude = 0.0;
            if (parent->getParent()) {
                const double c_obl = cos(parentRotObliquity);
                const double s_obl = sin(parentRotObliquity);
                const double c_nod = cos(parent_rot_asc_node);
                const double s_nod = sin(parent_rot_asc_node);
                const Vec3d OrbitAxis0( c_nod,       s_nod,        0.0);
                const Vec3d OrbitAxis1(-s_nod*c_obl, c_nod*c_obl,s_obl);
                const Vec3d OrbitPole(  s_nod*s_obl,-c_nod*s_obl,c_obl);
                const Vec3d J2000Pole(StelNavigator::matJ2000ToVsop87.multiplyWithoutTranslation(Vec3d(0,0,1)));
                Vec3d J2000NodeOrigin(J2000Pole^OrbitPole);
                J2000NodeOrigin.normalize();
                parent_rot_j2000_longitude = atan2(J2000NodeOrigin*OrbitAxis1,J2000NodeOrigin*OrbitAxis0);
            }

            // Create an elliptical orbit
            EllipticalOrbit *orb = new EllipticalOrbit(pericenterDistance,
                                                       eccentricity,
                                                       inclination,
                                                       ascending_node,
                                                       arg_of_pericenter,
                                                       mean_anomaly,
                                                       period,
                                                       epoch,
                                                       parentRotObliquity,
                                                       parent_rot_asc_node,
                                                       parent_rot_j2000_longitude);
            orbits.push_back(orb);

            userDataPtr = orb;
            posfunc = &ellipticalOrbitPosFunc;
        }
        else if (funcName=="comet_orbit")
        {
            // Read the orbital elements
            // orbit_PericenterDistance,orbit_SemiMajorAxis: given in AU
            // orbit_MeanMotion: given in degrees/day
            // orbit_Period: given in days
            // orbit_TimeAtPericenter,orbit_Epoch: JD
            // orbit_MeanAnomaly,orbit_Inclination,orbit_ArgOfPericenter,orbit_AscendingNode: given in degrees
            const double eccentricity = pd.value(secname+"/orbit_Eccentricity",0.0).toDouble();
            if (eccentricity >= 1.0) closeOrbit = false;
            double pericenterDistance = pd.value(secname+"/orbit_PericenterDistance",-1e100).toDouble();
            double semi_major_axis;
            if (pericenterDistance <= 0.0) {
                semi_major_axis = pd.value(secname+"/orbit_SemiMajorAxis",-1e100).toDouble();
                if (semi_major_axis <= -1e100) {
                    qWarning() << "ERROR: " << englishName
                               << ": you must provide orbit_PericenterDistance or orbit_SemiMajorAxis";
                    abort();
                } else {
                    Q_ASSERT(eccentricity != 1.0); // parabolic orbits have no semi_major_axis
                    pericenterDistance = semi_major_axis * (1.0-eccentricity);
                }
            } else {
                semi_major_axis = (eccentricity == 1.0)
                        ? 0.0 // parabolic orbits have no semi_major_axis
                        : pericenterDistance / (1.0-eccentricity);
            }
            double meanMotion = pd.value(secname+"/orbit_MeanMotion",-1e100).toDouble();
            if (meanMotion <= -1e100) {
                const double period = pd.value(secname+"/orbit_Period",-1e100).toDouble();
                if (period <= -1e100) {
                    if (parent->getParent()) {
                        qWarning() << "ERROR: " << englishName
                                   << ": when the parent body is not the sun, you must provide "
                                   << "either orbit_MeanMotion or orbit_Period";
                    } else {
                        // in case of parent=sun: use Gaussian gravitational constant
                        // for calculating meanMotion:
                        meanMotion = (eccentricity == 1.0)
                                ? 0.01720209895 * (1.5/pericenterDistance)
                                  * sqrt(0.5/pericenterDistance)
                                : (semi_major_axis > 0.0)
                                  ? 0.01720209895 / (semi_major_axis*sqrt(semi_major_axis))
                                  : 0.01720209895 / (-semi_major_axis*sqrt(-semi_major_axis));
                    }
                } else {
                    meanMotion = 2.0*M_PI/period;
                }
            } else {
                meanMotion *= (M_PI/180.0);
            }
            double time_at_pericenter = pd.value(secname+"/orbit_TimeAtPericenter",-1e100).toDouble();
            if (time_at_pericenter <= -1e100) {
                const double epoch = pd.value(secname+"/orbit_Epoch",-1e100).toDouble();
                double mean_anomaly = pd.value(secname+"/orbit_MeanAnomaly",-1e100).toDouble();
                if (epoch <= -1e100 || mean_anomaly <= -1e100) {
                    qWarning() << "ERROR: " << englishName
                               << ": when you do not provide orbit_TimeAtPericenter, you must provide both "
                               << "orbit_Epoch and orbit_MeanAnomaly";
                    abort();
                } else {
                    mean_anomaly *= (M_PI/180.0);
                    time_at_pericenter = epoch - mean_anomaly / meanMotion;
                }
            }
            const double inclination = pd.value(secname+"/orbit_Inclination").toDouble()*(M_PI/180.0);
            const double arg_of_pericenter = pd.value(secname+"/orbit_ArgOfPericenter").toDouble()*(M_PI/180.0);
            const double ascending_node = pd.value(secname+"/orbit_AscendingNode").toDouble()*(M_PI/180.0);
            const double parentRotObliquity = parent->getParent()
                    ? parent->getRotObliquity()
                    : 0.0;
            const double parent_rot_asc_node = parent->getParent()
                    ? parent->getRotAscendingnode()
                    : 0.0;
            double parent_rot_j2000_longitude = 0.0;
            if (parent->getParent()) {
                const double c_obl = cos(parentRotObliquity);
                const double s_obl = sin(parentRotObliquity);
                const double c_nod = cos(parent_rot_asc_node);
                const double s_nod = sin(parent_rot_asc_node);
                const Vec3d OrbitAxis0( c_nod,       s_nod,        0.0);
                const Vec3d OrbitAxis1(-s_nod*c_obl, c_nod*c_obl,s_obl);
                const Vec3d OrbitPole(  s_nod*s_obl,-c_nod*s_obl,c_obl);
                const Vec3d J2000Pole(StelNavigator::matJ2000ToVsop87.multiplyWithoutTranslation(Vec3d(0,0,1)));
                Vec3d J2000NodeOrigin(J2000Pole^OrbitPole);
                J2000NodeOrigin.normalize();
                parent_rot_j2000_longitude = atan2(J2000NodeOrigin*OrbitAxis1,J2000NodeOrigin*OrbitAxis0);
            }
            CometOrbit *orb = new CometOrbit(pericenterDistance,
                                             eccentricity,
                                             inclination,
                                             ascending_node,
                                             arg_of_pericenter,
                                             time_at_pericenter,
                                             meanMotion,
                                             parentRotObliquity,
                                             parent_rot_asc_node,
                                             parent_rot_j2000_longitude);
            orbits.push_back(orb);
            userDataPtr = orb;
            posfunc = &cometOrbitPosFunc;
        }

        if (funcName=="sun_special")
            posfunc = &get_sun_helio_coordsv;

        if (funcName=="mercury_special") {
            posfunc = &get_mercury_helio_coordsv;
            osculatingFunc = &get_mercury_helio_osculating_coords;
        }

        if (funcName=="venus_special") {
            posfunc = &get_venus_helio_coordsv;
            osculatingFunc = &get_venus_helio_osculating_coords;
        }

        if (funcName=="earth_special") {
            posfunc = &get_earth_helio_coordsv;
            osculatingFunc = &get_earth_helio_osculating_coords;
        }

        if (funcName=="lunar_special")
            posfunc = &get_lunar_parent_coordsv;

        if (funcName=="mars_special") {
            posfunc = &get_mars_helio_coordsv;
            osculatingFunc = &get_mars_helio_osculating_coords;
        }

        if (funcName=="phobos_special")
            posfunc = posFuncType(get_phobos_parent_coordsv);

        if (funcName=="deimos_special")
            posfunc = &get_deimos_parent_coordsv;

        if (funcName=="jupiter_special") {
            posfunc = &get_jupiter_helio_coordsv;
            osculatingFunc = &get_jupiter_helio_osculating_coords;
        }

        if (funcName=="europa_special")
            posfunc = &get_europa_parent_coordsv;

        if (funcName=="calisto_special")
            posfunc = &get_callisto_parent_coordsv;

        if (funcName=="io_special")
            posfunc = &get_io_parent_coordsv;

        if (funcName=="ganymede_special")
            posfunc = &get_ganymede_parent_coordsv;

        if (funcName=="saturn_special") {
            posfunc = &get_saturn_helio_coordsv;
            osculatingFunc = &get_saturn_helio_osculating_coords;
        }

        if (funcName=="mimas_special")
            posfunc = &get_mimas_parent_coordsv;

        if (funcName=="enceladus_special")
            posfunc = &get_enceladus_parent_coordsv;

        if (funcName=="tethys_special")
            posfunc = &get_tethys_parent_coordsv;

        if (funcName=="dione_special")
            posfunc = &get_dione_parent_coordsv;

        if (funcName=="rhea_special")
            posfunc = &get_rhea_parent_coordsv;

        if (funcName=="titan_special")
            posfunc = &get_titan_parent_coordsv;

        if (funcName=="iapetus_special")
            posfunc = &get_iapetus_parent_coordsv;

        if (funcName=="hyperion_special")
            posfunc = &get_hyperion_parent_coordsv;

        if (funcName=="uranus_special") {
            posfunc = &get_uranus_helio_coordsv;
            osculatingFunc = &get_uranus_helio_osculating_coords;
        }

        if (funcName=="miranda_special")
            posfunc = &get_miranda_parent_coordsv;

        if (funcName=="ariel_special")
            posfunc = &get_ariel_parent_coordsv;

        if (funcName=="umbriel_special")
            posfunc = &get_umbriel_parent_coordsv;

        if (funcName=="titania_special")
            posfunc = &get_titania_parent_coordsv;

        if (funcName=="oberon_special")
            posfunc = &get_oberon_parent_coordsv;

        if (funcName=="neptune_special") {
            posfunc = posFuncType(get_neptune_helio_coordsv);
            osculatingFunc = &get_neptune_helio_osculating_coords;
        }

        if (funcName=="pluto_special")
            posfunc = &get_pluto_helio_coordsv;


        if (posfunc==NULL)
        {
            qWarning() << "ERROR : can't find posfunc " << funcName << " for " << englishName;
            exit(-1);
        }

        QString texture = pd.value(secname+"/tex_map").toString();
        if(!StelMainWindow::getInstance().getIsMultiprojector())
        {
            if (StelMainWindow::getInstance().getIsServer())
            {
                QStringList strTex = texture.split('.');
                if (strTex.count() == 2)
                {
                    if (StelFileMgr::exists("textures/"+strTex[0]+"_small.png"))
                    {
                        texture = strTex[0]+"_small.png";
                    }
                }
            }
        }
        // Create the Planet and add it to the list
        PlanetP p(new Planet(englishName,
                             pd.value(secname+"/lighting").toBool(),
                             pd.value(secname+"/radius").toDouble()/AU,
                             pd.value(secname+"/oblateness", 0.0).toDouble(),
                             StelUtils::strToVec3f(pd.value(secname+"/color").toString()),
                             pd.value(secname+"/albedo").toDouble(),
                             texture,
                             pd.value(secname+"/tex_halo").toString(),
                             posfunc,
                             userDataPtr,
                             osculatingFunc,
                             closeOrbit,
                             pd.value(secname+"/hidden", 0).toBool(),
                             pd.value(secname+"/atmosphere", false).toBool(),
                             StelUtils::strToVec3f(pd.value(secname+"/orbitcolor","1,0,0").toString())));
        if (!parent.isNull())
        {
            parent->satellites.append(p);
            p->parent = parent;
        }
        if (secname=="earth") earth = p;
        if (secname=="sun") sun = p;
        if (secname=="moon") moon = p;
        //ASAF
        if (secname=="venus") venus = p;
        if (secname=="mars") mars = p;
        if (secname=="jupiter") jupiter = p;
        if (secname=="saturn") saturn = p;
        if (secname=="mercury") merkur = p;
        if (secname=="neptune") neptun = p;
        if (secname=="pluto") pluto = p;
        if (secname=="uranus") uranus = p;
        //ASAF

        double rotObliquity = pd.value(secname+"/rot_obliquity",0.).toDouble()*(M_PI/180.0);
        double rotAscNode = pd.value(secname+"/rot_equator_ascending_node",0.).toDouble()*(M_PI/180.0);

        // Use more common planet North pole data if available
        // NB: N pole as defined by IAU (NOT right hand rotation rule)
        // NB: J2000 epoch
        double J2000NPoleRA = pd.value(secname+"/rot_pole_ra", 0.).toDouble()*M_PI/180.;
        double J2000NPoleDE = pd.value(secname+"/rot_pole_de", 0.).toDouble()*M_PI/180.;

        if(J2000NPoleRA || J2000NPoleDE)
        {
            Vec3d J2000NPole;
            StelUtils::spheToRect(J2000NPoleRA,J2000NPoleDE,J2000NPole);

            Vec3d vsop87Pole(StelNavigator::matJ2000ToVsop87.multiplyWithoutTranslation(J2000NPole));

            double ra, de;
            StelUtils::rectToSphe(&ra, &de, vsop87Pole);

            rotObliquity = (M_PI_2 - de);
            rotAscNode = (ra + M_PI_2);

            // qDebug() << "\tCalculated rotational obliquity: " << rotObliquity*180./M_PI << endl;
            // qDebug() << "\tCalculated rotational ascending node: " << rotAscNode*180./M_PI << endl;
        }

        p->setRotationElements(
                    pd.value(secname+"/rot_periode", pd.value(secname+"/orbit_Period", 24.).toDouble()).toDouble()/24.,
                    pd.value(secname+"/rot_rotation_offset",0.).toDouble(),
                    pd.value(secname+"/rot_epoch", J2000).toDouble(),
                    rotObliquity,
                    rotAscNode,
                    pd.value(secname+"/rot_precession_rate",0.).toDouble()*M_PI/(180*36525),
                    pd.value(secname+"/orbit_visualization_period",0.).toDouble());

        //ASAF de�i�tirdim
        if (pd.value(secname+"/rings", 0).toBool()) {
            const double rMin = pd.value(secname+"/ring_inner_size").toDouble()/AU;
            const double rMax = pd.value(secname+"/ring_outer_size").toDouble()/AU;
            Ring *r = new Ring(rMin,rMax,pd.value(secname+"/tex_ring").toString());
            p->setRings(r);
        }

        systemPlanets.push_back(p);
        readOk++;
    }

    // special case: load earth shadow  ure
    Planet::texEarthShadow = StelApp::getInstance().getTextureManager().createTexture("textures/earth-shadow.png");

    Planet::texAtmosphere = StelApp::getInstance().getTextureManager().createTexture("textures/neb.png");
    qDebug() << "Loaded" << readOk << "/" << totalPlanets << "planet orbits";
}

void SolarSystem::loadInternalPlanets()
{
    QList<PlanetP> internalPlanets;

    foreach (PlanetP p, systemPlanets)
    {
        if  ( ( p == earth ) || ( p == moon ) || ( p == mars ) || ( p == merkur ) ||
              ( p == venus ) || ( p == jupiter ) || ( p == saturn ) || ( p == uranus ) ||
              ( p == pluto ) || ( p == neptun  ) || ( p == sun ))
        {
            //Flyby Planets
            PlanetP planet = PlanetP( new Planet(p) );
            planet.data()->setEnglishName(QString("FLYBY-%0").arg(planet.data()->getEnglishName()));
            //qDebug()<<planet.data()->getEnglishName();
            if ( (p == earth) || (p == moon) || (p == mars) )
            {
                planet.data()->setflybFactor(1.5);
                planet.data()->setRadius(1.5 *planet.data()->getRadius());
            }
            else if( p == saturn )
            {
                planet.data()->setflybFactor(2.5);
                planet.data()->setRadius(2.5 *planet.data()->getRadius());
            }
            else
            {
                planet.data()->setflybFactor(2);
                planet.data()->setRadius(2 *planet.data()->getRadius());
            }
            //planet.data()->setRotPeriod(15000);//planet.data()->getRotationElements().period);

            planet.data()->setTexMapName("nomap.png");
            planet.data()->setHidden(true);
            planet.data()->setFlybyPlanet(p);

            if (p == moon)
                planet.data()->parent = earth;
            internalPlanets.push_back(planet);

            //Also Solar System flyby
            if (p == sun)
            {
                //Son gezegen plutonun g�ne�e olan uzakl��� referans al�n�yor
                PlanetP planet = PlanetP( new Planet(p) );
                planet.data()->setEnglishName("FLYBY-SS");
                planet.data()->setflybFactor(10532);
                planet.data()->setRadius(10532 *planet.data()->getRadius());

                planet.data()->setTexMapName("nomap.png");
                planet.data()->setHidden(true);
                planet.data()->setFlybyPlanet(p);
                planet.data()->setRotObliquity(0);

                internalPlanets.push_back(planet);
            }
        }
    }
    systemPlanets.append(internalPlanets);

}


// Compute the position for every elements of the solar system.
// The order is not important since the position is computed relatively to the mother body
void SolarSystem::computePositions(double date, const Vec3d& observerPos)
{
    if (flagLightTravelTime)
    {
        foreach (PlanetP p, systemPlanets)
        {
            p->computePositionWithoutOrbits(date);
        }
        foreach (PlanetP p, systemPlanets)
        {
            const double light_speed_correction = (p->getHeliocentricEclipticPos()-observerPos).length() * (AU / (SPEED_OF_LIGHT * 86400));
            p->computePosition(date-light_speed_correction);
        }
    }
    else
    {
        foreach (PlanetP p, systemPlanets)
        {
            p->computePosition(date);
        }
    }
    computeTransMatrices(date, observerPos);
}

// Compute the transformation matrix for every elements of the solar system.
// The elements have to be ordered hierarchically, eg. it's important to compute earth before moon.
void SolarSystem::computeTransMatrices(double date, const Vec3d& observerPos)
{
    if (flagLightTravelTime)
    {
        foreach (PlanetP p, systemPlanets)
        {
            const double light_speed_correction = (p->getHeliocentricEclipticPos()-observerPos).length() * (AU / (SPEED_OF_LIGHT * 86400));
            p->computeTransMatrix(date-light_speed_correction);
        }
    }
    else
    {
        foreach (PlanetP p, systemPlanets)
        {
            p->computeTransMatrix(date);
        }
    }
}

// And sort them from the furthest to the closest to the observer
struct biggerDistance : public std::binary_function<PlanetP, PlanetP, bool>
{
    bool operator()(PlanetP p1, PlanetP p2)
    {
        return p1->getDistance() > p2->getDistance();
    }
};

// Draw all the elements of the solar system
// We are supposed to be in heliocentric coordinate
void SolarSystem::draw(StelCore* core)
{
    if (!flagShow)
        return;

    StelNavigator* nav = core->getNavigator();

    // Compute each Planet distance to the observer
    Vec3d obsHelioPos = nav->getObserverHeliocentricEclipticPos();

    foreach (PlanetP p, systemPlanets)
    {
        p->computeDistance(obsHelioPos);
    }

    // And sort them from the furthest to the closest
    sort(systemPlanets.begin(),systemPlanets.end(),biggerDistance());

    if (trailFader.getInterstate()>0.0000001)
    {
        StelPainter* sPainter = new StelPainter(core->getProjection2d());
        allTrails->setOpacity(trailFader.getInterstate());
        allTrails->draw(core, sPainter);
        delete sPainter;
    }

    // Draw the elements
    float maxMagLabel=core->getSkyDrawer()->getLimitMagnitude()*0.80+(labelsAmount*1.2f)-2.f;
    foreach (const PlanetP& p, systemPlanets)
    {        
        p->draw(core, maxMagLabel, planetNameFont);
    }

    if (GETSTELMODULE(StelObjectMgr)->getFlagSelectedObjectPointer())
        drawPointer(core);
}

void SolarSystem::setStelStyle(const StelStyle& style)
{
    // Load colors from config file
    QSettings* conf = StelApp::getInstance().getSettings();
    QString section = "color"; //style.confSectionName; Asaf gece g�r��� meselesinden iptal ettim

    QString defaultColor = conf->value(section+"/default_color").toString();
    setLabelsColor(StelUtils::strToVec3f(conf->value(section+"/planet_names_color", defaultColor).toString()));
    setOrbitsColor(StelUtils::strToVec3f(conf->value(section+"/planet_orbits_color", defaultColor).toString()));
    setTrailsColor(StelUtils::strToVec3f(conf->value(section+"/object_trails_color", defaultColor).toString()));
    setAxisesColor(StelUtils::strToVec3f(conf->value(section+"/planet_axis_color", "0,76,153").toString()));
    setEqulineColor(StelUtils::strToVec3f(conf->value(section+"/planet_equator_color", "1,0,0").toString()));

    // Recreate the trails to apply new colors
    recreateTrails();
}

PlanetP SolarSystem::searchByEnglishName(QString planetEnglishName) const
{
    foreach (const PlanetP& p, systemPlanets)
    {
        if (p->getEnglishName() == planetEnglishName)
            if (!p->internal)
                return p;
    }
    return PlanetP();
}

PlanetP SolarSystem::searchByEnglishNameForFlyby(QString planetEnglishName) const
{
    foreach (const PlanetP& p, systemPlanets)
    {
        if (p->getEnglishName() == planetEnglishName)
            return p;
    }
    return PlanetP();
}

StelObjectP SolarSystem::searchByNameI18n(const QString& planetNameI18) const
{
    foreach (const PlanetP& p, systemPlanets)
    {
        if (p->getNameI18n() == planetNameI18)
            if (!p->internal)
                return qSharedPointerCast<StelObject>(p);
    }
    return StelObjectP();
}


StelObjectP SolarSystem::searchByName(const QString& name) const
{
    foreach (const PlanetP& p, systemPlanets)
    {
        if (p->getEnglishName() == name)
            if (!p->internal)
                return qSharedPointerCast<StelObject>(p);
    }
    return StelObjectP();
}

// Search if any Planet is close to position given in earth equatorial position and return the distance
StelObjectP SolarSystem::search(Vec3d pos, const StelCore* core) const
{
    pos.normalize();
    PlanetP closest;
    double cos_angle_closest = 0.;
    Vec3d equPos;

    foreach (const PlanetP& p, systemPlanets)
    {
        equPos = p->getEquinoxEquatorialPos(core->getNavigator());
        equPos.normalize();
        double cos_ang_dist = equPos*pos;
        if (cos_ang_dist>cos_angle_closest)
        {
            closest = p;
            cos_angle_closest = cos_ang_dist;
        }
    }

    if (cos_angle_closest>0.999)
    {
        if (!closest->internal)
            return qSharedPointerCast<StelObject>(closest);
    }
    else return StelObjectP();
}

// Return a stl vector containing the planets located inside the limFov circle around position v
QList<StelObjectP> SolarSystem::searchAround(const Vec3d& vv, double limitFov, const StelCore* core) const
{
    QList<StelObjectP> result;
    if (!getFlagPlanets())
        return result;

    Vec3d v = core->getNavigator()->j2000ToEquinoxEqu(vv);
    v.normalize();
    double cosLimFov = std::cos(limitFov * M_PI/180.);
    Vec3d equPos;

    foreach (const PlanetP& p, systemPlanets)
    {
        equPos = p->getEquinoxEquatorialPos(core->getNavigator());
        equPos.normalize();
        if (equPos*v>=cosLimFov)
        {
            if (!p->internal)
                result.append(qSharedPointerCast<StelObject>(p));
        }
    }
    return result;
}

// Update i18 names from english names according to passed translator
void SolarSystem::updateI18n()
{
    StelTranslator& trans = StelApp::getInstance().getLocaleMgr().getSkyTranslator();
    foreach (PlanetP p, systemPlanets)
        p->translateName(trans);
}

QString SolarSystem::getPlanetHashString(void)
{
    QString str;
    QTextStream oss(&str);
    foreach (const PlanetP& p, systemPlanets)
    {
        if (!p->getParent().isNull() && p->getParent()->getEnglishName() != "Sun")
        {
            oss << p->getParent()->getEnglishName() << " : ";
        }
        oss << p->getEnglishName() << endl;
        oss << p->getEnglishName() << endl;
    }
    return str;
}

void SolarSystem::setFlagTrails(bool b)
{
    trailFader = b;
    if (b)
        allTrails->reset();
}

bool SolarSystem::getFlagTrails() const
{
    return (bool)trailFader;
}

void SolarSystem::setFlagHints(bool b)
{
    foreach (PlanetP p, systemPlanets)
        p->setFlagHints(b);
    StelApp::getInstance().addNetworkCommand("SolarSystem.setFlagHints("+ QString("%0").arg(b) +");");
}

bool SolarSystem::getFlagHints(void) const
{
    foreach (const PlanetP& p, systemPlanets)
    {
        if (p->getFlagHints())
            return true;
    }
    return false;
}

void SolarSystem::setFlagLabels(bool b)
{
    foreach (PlanetP p, systemPlanets)
        p->setFlagLabels(b);
    StelApp::getInstance().addNetworkCommand("SolarSystem.setFlagLabels("+QString("%0").arg(b)+");");
}

bool SolarSystem::getFlagLabels() const
{
    foreach (const PlanetP& p, systemPlanets)
    {
        if (p->getFlagLabels())
            return true;
    }
    return false;
}

void SolarSystem::setFlagOrbits(bool b)
{
    flagOrbits = b;
    if (!b || !selected || selected==sun)
    {
        foreach (PlanetP p, systemPlanets)
            p->setFlagOrbits(b);
    }
    else
    {
        // If a Planet is selected and orbits are on, fade out non-selected ones
        foreach (PlanetP p, systemPlanets)
        {
            if (selected == p)
                p->setFlagOrbits(b);
            else
                p->setFlagOrbits(false);
        }
    }
    StelApp::getInstance().addNetworkCommand("SolarSystem.setFlagOrbits("+ QString("%0").arg(b) +");");
}

void SolarSystem::setFlagAxises(bool b)
{
    flagAxises = b;
    foreach (PlanetP p, systemPlanets)
        p->setFlagAxises(b);

    StelApp::getInstance().addNetworkCommand("SolarSystem.setFlagAxises("+ QString("%0").arg(b) +");");
}

void SolarSystem::setFlagEquline(bool b)
{
    flagEquline = b;
    foreach (PlanetP p, systemPlanets)
        p->setFlagEquline(b);

    StelApp::getInstance().addNetworkCommand("SolarSystem.setFlagEquline("+ QString("%0").arg(b) +");");

}

void SolarSystem::setFlagLightTravelTime(bool b)
{
    flagLightTravelTime = b;
    StelApp::getInstance().addNetworkCommand("SolarSystem.setFlagLightTravelTime("+ QString("%0").arg(b) +");");
}

void SolarSystem::setSelected(PlanetP obj)
{
    if ((obj && obj->getType() == "Planet") && (!obj->internal))
        selected = obj;
    else
        selected.clear();;
    // Undraw other objects hints, orbit, trails etc..
    setFlagHints(getFlagHints());
    setFlagOrbits(getFlagOrbits());
}


void SolarSystem::update(double deltaTime)
{
    trailFader.update(deltaTime*1000);
    if (trailFader.getInterstate()>0.f)
    {
        allTrails->update();
    }

    foreach (PlanetP p, systemPlanets)
    {
        p->update((int)(deltaTime*1000));
    }
}


// is a lunar eclipse close at hand?
bool SolarSystem::nearLunarEclipse()
{
    // TODO: could replace with simpler test

    Vec3d e = getEarth()->getEclipticPos();
    Vec3d m = getMoon()->getEclipticPos();  // relative to earth
    Vec3d mh = getMoon()->getHeliocentricEclipticPos();  // relative to sun

    // shadow location at earth + moon distance along earth vector from sun
    Vec3d en = e;
    en.normalize();
    Vec3d shadow = en * (e.length() + m.length());

    // find shadow radii in AU
    double r_penumbra = shadow.length()*702378.1/AU/e.length() - 696000/AU;

    // modify shadow location for scaled moon
    Vec3d mdist = shadow - mh;
    if(mdist.length() > r_penumbra + 2000/AU) return 0;   // not visible so don't bother drawing

    return 1;
}

//! Find and return the list of at most maxNbItem objects auto-completing the passed object I18n name
QStringList SolarSystem::listMatchingObjectsI18n(const QString& objPrefix, int maxNbItem) const
{
    QStringList result;
    if (maxNbItem==0)
        return result;
    QString objw = objPrefix.toUpper();
    foreach (const PlanetP& p, systemPlanets)
    {
        QString constw = p->getNameI18n().mid(0, objw.size()).toUpper();
        if ((constw==objw) && (!p->internal))
        {
            result << p->getNameI18n();
            if (result.size()==maxNbItem)
                return result;
        }
    }
    return result;
}

void SolarSystem::selectedObjectChangeCallBack(StelModuleSelectAction action)
{
    const QList<StelObjectP> newSelected = GETSTELMODULE(StelObjectMgr)->getSelectedObject("Planet");
    if (!newSelected.empty())
        setSelected(qSharedPointerCast<Planet>(newSelected[0]));
}

// Activate/Deactivate planets display
void SolarSystem::setFlagPlanets(bool b)
{
    flagShow=b;
    StelApp::getInstance().addNetworkCommand("SolarSystem.setFlagPlanets("+ QString("%0").arg(b) +");");
}

bool SolarSystem::getFlagPlanets(void) const {return flagShow;}

// Set/Get planets names color
void SolarSystem::setLabelsColor(const Vec3f& c) {Planet::setLabelColor(c);}
const Vec3f& SolarSystem::getLabelsColor(void) const {return Planet::getLabelColor();}

// Set/Get orbits lines color
void SolarSystem::setOrbitsColor(const Vec3f& c) {Planet::setOrbitColor(c);}
Vec3f SolarSystem::getOrbitsColor(void) const {return Planet::getOrbitColor();}

// Set/Get axises lines color
void SolarSystem::setAxisesColor(const Vec3f& c) {Planet::setAxisColor(c);}
Vec3f SolarSystem::getAxisesColor(void) const {return Planet::getAxisColor();}

// Set/Get equator lines color
Vec3f SolarSystem::getEqulineColor() const { return Planet::getEquColor();}
void SolarSystem::setEqulineColor(const Vec3f &c){ Planet::setEquColor(c); }

// Set/Get if Moon display is scaled
void SolarSystem::setFlagMoonScale(bool b)
{
    if (!b) getMoon()->setSphereScale(1);
    else  getMoon()->setSphereScale(moonScale);
    flagMoonScale = b;
    StelApp::getInstance().addNetworkCommand("SolarSystem.setFlagMoonScale("+ QString("%0").arg(b) +");");
}
// Set/Get Moon display scaling factor
void SolarSystem::setMoonScale(double f)
{
    moonScale = f;
    if (flagMoonScale)
    {
        getMoon()->setSphereScale(moonScale);
        StelApp::getInstance().addNetworkCommand("SolarSystem.setMoonScale("+QString("%0").arg(f)+");");
    }
}

//ASAF  Set/Get if planets display is scaled
void SolarSystem::setFlagPlanetsScale(bool b)
{
    if (!b)
    {
        getMerkur()->setSphereScale(1);
        getMars()->setSphereScale(1);
        getVenus()->setSphereScale(1);
        getSaturn()->setSphereScale(1);
        getJupiter()->setSphereScale(1);
        getUranus()->setSphereScale(1);
        getPluto()->setSphereScale(1);
        getNeptun()->setSphereScale(1);
        //ring scale
        getSaturn()->getRings()->setRingScale(1);
    }
    else
    {
        getMerkur()->setSphereScale(merkurScale);
        getMars()->setSphereScale(marsScale);
        getVenus()->setSphereScale(venusScale);
        getSaturn()->setSphereScale(saturnScale);
        getJupiter()->setSphereScale(jupiterScale);
        getUranus()->setSphereScale(uranusScale);
        getPluto()->setSphereScale(plutoScale);
        getNeptun()->setSphereScale(neptunScale);
        //ring scale
        getSaturn()->getRings()->setRingScale(saturnScale);
    }
    flagPlanetsScale = b;
    StelApp::getInstance().addNetworkCommand("SolarSystem.setFlagPlanetsScale("+ QString("%0").arg(b) +");");
}
void SolarSystem::setMerkurScale(double f)
{
    merkurScale = f;
    if (flagPlanetsScale)
    {
        getMerkur()->setSphereScale(merkurScale);
        StelApp::getInstance().addNetworkCommand("SolarSystem.setMerkurScale("+QString("%0").arg(f)+");");
    }
}
void SolarSystem::setVenusScale(double f)
{
    venusScale = f;
    if (flagPlanetsScale)
    {
        getVenus()->setSphereScale(venusScale);
        StelApp::getInstance().addNetworkCommand("SolarSystem.setVenusScale("+QString("%0").arg(f)+");");
    }
}
void SolarSystem::setMarsScale(double f)
{
    marsScale = f;
    if (flagPlanetsScale)
    {
        getMars()->setSphereScale(marsScale);
        StelApp::getInstance().addNetworkCommand("SolarSystem.setMarsScale("+QString("%0").arg(f)+");");
    }

}
void SolarSystem::setJupiterScale(double f)
{
    jupiterScale = f;
    if (flagPlanetsScale)
    {
        getJupiter()->setSphereScale(jupiterScale);
        StelApp::getInstance().addNetworkCommand("SolarSystem.setJupiterScale("+QString("%0").arg(f)+");");
    }
}
void SolarSystem::setSaturnScale(double f)
{
    saturnScale = f;
    if (flagPlanetsScale)
    {
        getSaturn()->setSphereScale(saturnScale);
        getSaturn()->getRings()->setRingScale(saturnScale);
        StelApp::getInstance().addNetworkCommand("SolarSystem.setSaturnScale("+QString("%0").arg(f)+");");
    }
}
void SolarSystem::setUranusScale(double f){
    uranusScale = f;
    if (flagPlanetsScale)
    {
        getUranus()->setSphereScale(uranusScale);
        StelApp::getInstance().addNetworkCommand("SolarSystem.setUranusScale("+QString("%0").arg(f)+");");
    }
}
void SolarSystem::setPlutoScale(double f)
{
    plutoScale = f;
    if (flagPlanetsScale)
    {
        getPluto()->setSphereScale(plutoScale);
        StelApp::getInstance().addNetworkCommand("SolarSystem.setPlutoScale("+QString("%0").arg(f)+");");
    }
}
void SolarSystem::setNeptunScale(double f)
{
    neptunScale = f;
    if (flagPlanetsScale)
    {
        getNeptun()->setSphereScale(neptunScale);
        StelApp::getInstance().addNetworkCommand("SolarSystem.setNeptunScale("+QString("%0").arg(f)+");");
    }
}
void SolarSystem::setEarthScale(double f)
{
    earthScale = f;
    if (flagPlanetsScale)
    {
        getEarth()->setSphereScale(earthScale);
        StelApp::getInstance().addNetworkCommand("SolarSystem.setEarthsScale("+QString("%0").arg(f)+");");
    }
}

void SolarSystem::setFlagFlybyAtm(bool b)
{
    flagflybyAtm = b;
    foreach (PlanetP p, systemPlanets)
        p->setFlagFlybyAtmosphere(b);

    StelApp::getInstance().addNetworkCommand("SolarSystem.setFlagFlybyAtm("+ QString("%0").arg(b) +");");
}

void SolarSystem::setFlagLighting(bool b)
{
    foreach (const PlanetP& p, systemPlanets)
    {
        if (!p->internal)
            p->setFlagLighting(b);
    }
}

// Set selected planets by englishName
void SolarSystem::setSelected(const QString& englishName)
{
    setSelected(searchByEnglishName(englishName));
}

// Get the list of all the planet english names
QStringList SolarSystem::getAllPlanetEnglishNames() const
{
    QStringList res;
    foreach (const PlanetP& p, systemPlanets)
    {
        if (!p->internal)
            res.append(p->englishName);
    }
    return res;
}

void SolarSystem::reloadPlanets()
{
    //Save flag states
    bool flagScaleMoon = getFlagMoonScale();
    float moonScale = getMoonScale();
    bool flagPlanets = getFlagPlanets();
    bool flagHints = getFlagHints();
    bool flagLabels = getFlagLabels();
    bool flagOrbits = getFlagOrbits();

    //Unload all Solar System objects
    selected.clear();//Release the selected one
    foreach (Orbit* orb, orbits)
    {
        delete orb;
        orb = NULL;
    }
    orbits.clear();

    sun.clear();
    moon.clear();
    earth.clear();
    Planet::texEarthShadow.clear(); //Loaded in loadPlanets()

    delete allTrails;
    allTrails = NULL;

    foreach (PlanetP p, systemPlanets)
    {
        p->satellites.clear();
        p.clear();
    }
    systemPlanets.clear();
    //Memory leak? What's the proper way of cleaning shared pointers?

    //Re-load the ssystem.ini file
    loadPlanets();
    computePositions(StelUtils::getJDFromSystem());
    setSelected("");
    recreateTrails();

    //Restore flag states
    setFlagMoonScale(flagScaleMoon);
    setMoonScale(moonScale);
    setFlagPlanets(flagPlanets);
    setFlagHints(flagHints);
    setFlagLabels(flagLabels);
    setFlagOrbits(flagOrbits);

    //Restore translations
    updateI18n();

}
