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

// Class used to manage group of constellation

#include <config.h>

#include <vector>
#include <QDebug>
#include <QFile>
#include <QSettings>
#include <QRegExp>
#include <QString>
#include <QStringList>

#include "ConstellationMgr.hpp"
#include "Constellation.hpp"
#include "StarMgr.hpp"
#include "StelUtils.hpp"
#include "StelApp.hpp"
#include "StelTextureMgr.hpp"
#include "StelProjector.hpp"
#include "StelObjectMgr.hpp"
#include "StelLocaleMgr.hpp"
#include "StelSkyCultureMgr.hpp"
#include "StelModuleMgr.hpp"
#include "StelFileMgr.hpp"
#include "StelCore.hpp"
#include "StelStyle.hpp"
#include "StelPainter.hpp"

using namespace std;

// constructor which loads all data from appropriate files
ConstellationMgr::ConstellationMgr(StarMgr *_hip_stars) :
    hipStarMgr(_hip_stars),
    flagNames(0),
    flagLines(0),
    flagArt(0),
    flagBoundaries(0)
{
    setObjectName("ConstellationMgr");
    Q_ASSERT(hipStarMgr);
    isolateSelected = false;
    asterFont.setPixelSize(15);
}

ConstellationMgr::~ConstellationMgr()
{
    std::vector<Constellation *>::iterator iter;

    for (iter = asterisms.begin(); iter != asterisms.end(); iter++)
    {
        delete(*iter);
    }

    vector<vector<Vec3f> *>::iterator iter1;
    for (iter1 = allBoundarySegments.begin(); iter1 != allBoundarySegments.end(); ++iter1)
    {
        delete (*iter1);
    }
    allBoundarySegments.clear();
}

void ConstellationMgr::init()
{
    QSettings* conf = StelApp::getInstance().getSettings();
    Q_ASSERT(conf);

    lastLoadedSkyCulture = "dummy";
    asterFont.setPixelSize(conf->value("viewing/constellation_font_size",16).toInt());
    setFlagLines(conf->value("viewing/flag_constellation_drawing").toBool());
    setFlagLabels(conf->value("viewing/flag_constellation_name").toBool());
    setFlagBoundaries(conf->value("viewing/flag_constellation_boundaries",false).toBool());
    setArtIntensity(conf->value("viewing/constellation_art_intensity", 0.5).toDouble());
    setArtFadeDuration(conf->value("viewing/constellation_art_fade_duration",2.).toDouble());
    setFlagArt(conf->value("viewing/flag_constellation_art").toBool());
    setFlagIsolateSelected(conf->value("viewing/flag_constellation_isolate_selected",
                                       conf->value("viewing/flag_constellation_pick", false).toBool() ).toBool());

    setLinesWidth(conf->value("viewing/flag_constellation_linewidth",1).toInt());


    LoadDefaultList();

    GETSTELMODULE(StelObjectMgr)->registerStelObjectMgr(this);
}

/*************************************************************************
 Reimplementation of the getCallOrder method
*************************************************************************/
double ConstellationMgr::getCallOrder(StelModuleActionName actionName) const
{
    if (actionName==StelModule::ActionDraw)
        return StelApp::getInstance().getModuleMgr().getModule("GridLinesMgr")->getCallOrder(actionName)+10;
    return 0;
}

void ConstellationMgr::updateSkyCulture(const QString& skyCultureDir)
{
    // Check if the sky culture changed since last load, if not don't load anything
    if (lastLoadedSkyCulture == skyCultureDir)
        return;

    // Find constellation art.  If this doesn't exist, warn, but continue using ""
    // the loadLinesAndArt function knows how to handle this (just loads lines).
    QString conArtFile;
    try
    {
        conArtFile = StelFileMgr::findFile("skycultures/"+skyCultureDir+"/constellationsart.fab");
    }
    catch (std::runtime_error& e)
    {
        qDebug() << "No constellationsart.fab file found for sky culture " << skyCultureDir;
    }

    try
    {
        loadLinesAndArt(StelFileMgr::findFile("skycultures/"+skyCultureDir+"/constellationship.fab"), conArtFile, skyCultureDir);

        // load constellation names
        loadNames(StelFileMgr::findFile("skycultures/" + skyCultureDir + "/constellation_names.eng.fab"));

        // Translate constellation names for the new sky culture
        updateI18n();

        // as constellations have changed, clear out any selection and retest for match!
        selectedObjectChangeCallBack(StelModule::ReplaceSelection);
    }
    catch (std::runtime_error& e)
    {
        qWarning() << "ERROR: while loading new constellation data for sky culture "
                   << skyCultureDir << ", reason: " << e.what() << endl;
    }

    // TODO: do we need to have an else { clearBoundaries(); } ?
    if ((skyCultureDir=="western")||(skyCultureDir=="western_color")||(skyCultureDir=="hevelius"))
    {
        try
        {
            loadBoundaries(StelFileMgr::findFile("data/constellations_boundaries.dat"));
        }
        catch (std::runtime_error& e)
        {
            qWarning() << "ERROR loading constellation boundaries file: " << e.what();
        }
    }

    lastLoadedSkyCulture = skyCultureDir;
}

void ConstellationMgr::setStelStyle(const StelStyle& style)
{
    QSettings* conf = StelApp::getInstance().getSettings();

    // Load colors from config file
    QString defaultColor = conf->value("color/default_color").toString();
    setLinesColor(StelUtils::strToVec3f(conf->value("color/const_lines_color",    defaultColor).toString()));
    setBoundariesColor(StelUtils::strToVec3f(conf->value("color/const_boundary_color", "0.8,0.3,0.3").toString()));
    setLabelsColor(StelUtils::strToVec3f(conf->value("color/const_names_color",    defaultColor).toString()));
}

void ConstellationMgr::selectedObjectChangeCallBack(StelModuleSelectAction action)
{
    StelObjectMgr* omgr = GETSTELMODULE(StelObjectMgr);
    Q_ASSERT(omgr);
    const QList<StelObjectP> newSelected = omgr->getSelectedObject();
    if (newSelected.empty())
    {
        // Even if do not have anything selected, KEEP constellation selection intact
        // (allows viewing constellations without distraction from star pointer animation)
        // setSelected(NULL);
        return;
    }

    const QList<StelObjectP> newSelectedConst = omgr->getSelectedObject("Constellation");
    if (!newSelectedConst.empty())
    {
        // If removing this selection
        if(action == StelModule::RemoveFromSelection) {
            unsetSelectedConst((Constellation *)newSelectedConst[0].data());
        } else {
            // Add constellation to selected list (do not select a star, just the constellation)
            setSelectedConst((Constellation *)newSelectedConst[0].data());
        }
    }
    else
    {
        const QList<StelObjectP> newSelectedStar = omgr->getSelectedObject("Star");
        if (!newSelectedStar.empty())
        {
            //			if (!added)
            //				setSelected(NULL);
            setSelected(newSelectedStar[0].data());
        }
        else
        {
            //			if (!added)
            setSelected(NULL);
        }
    }
}

void ConstellationMgr::setLinesColor(const Vec3f& c)
{
    Constellation::lineColor = c;
}

Vec3f ConstellationMgr::getLinesColor() const
{
    return Constellation::lineColor;
}

void ConstellationMgr::setLinesWidth(const int w)
{
    linesWidth = w;
}

void ConstellationMgr::setBoundariesColor(const Vec3f& c)
{
    Constellation::boundaryColor = c;
}

Vec3f ConstellationMgr::getBoundariesColor() const
{
    return Constellation::boundaryColor;
}

void ConstellationMgr::setLabelsColor(const Vec3f& c)
{
    Constellation::labelColor = c;
}

Vec3f ConstellationMgr::getLabelsColor() const
{
    return Constellation::labelColor;
}

void ConstellationMgr::setFontSize(double newFontSize)
{
    asterFont.setPixelSize(newFontSize);
}

double ConstellationMgr::getFontSize() const
{
    return asterFont.pixelSize();
}

// Load line and art data from files
void ConstellationMgr::loadLinesAndArt(const QString &fileName, const QString &artfileName, const QString& cultureName)
{
    //	QFile in(fileName);
    //	if (!in.open(QIODevice::ReadOnly | QIODevice::Text))
    //	{
    //		qWarning() << "Can't open constellation data file" << fileName  << "for culture" << cultureName;
    //		Q_ASSERT(0);
    //	}
    //
    //	int totalRecords=0;
    //	QString record;
    //	QRegExp commentRx("^(\\s*#.*|\\s*)$");
    //	while (!in.atEnd())
    //	{
    //		record = QString::fromUtf8(in.readLine());
    //		if (!commentRx.exactMatch(record))
    //			totalRecords++;
    //	}
    //	in.seek(0);
    //
    //	// delete existing data, if any
    //	vector < Constellation * >::iterator iter;
    //	for (iter = asterisms.begin(); iter != asterisms.end(); ++iter)
    //		delete(*iter);
    //
    //	asterisms.clear();
    //	selected.clear();
    //	Constellation *cons = NULL;
    //
    //	// read the file, adding a record per non-comment line
    //	int currentLineNumber = 0;	// line in file
    //	int readOk = 0;			// count of records processed OK
    //	while (!in.atEnd())
    //	{
    //		record = QString::fromUtf8(in.readLine());
    //		currentLineNumber++;
    //		if (commentRx.exactMatch(record))
    //			continue;
    //
    //		cons = new Constellation;
    //		if(cons->read(record, hipStarMgr))
    //		{
    //			cons->artFader.setMaxValue(artMaxIntensity);
    //			asterisms.push_back(cons);
    //			++readOk;
    //		}
    //		else
    //		{
    //			qWarning() << "ERROR reading constellation rec at line " << currentLineNumber << "for culture" << cultureName;
    //			delete cons;
    //		}
    //	}
    //	in.close();
    //	qDebug() << "Loaded" << readOk << "/" << totalRecords << "constellation records successfully for culture" << cultureName;
    //
    //	// Set current states
    //	setFlagArt(flagArt);
    //	setFlagLines(flagLines);
    //	setFlagLabels(flagNames);
    //	setFlagBoundaries(flagBoundaries);
    //
    //	// It's possible to have no art - just constellations
    //	if (artfileName.isNull() || artfileName.isEmpty())
    //		return;
    //	QFile fic(artfileName);
    //	if (!fic.open(QIODevice::ReadOnly | QIODevice::Text))
    //	{
    //		qWarning() << "Can't open constellation art file" << fileName  << "for culture" << cultureName;
    //		return;
    //	}
    //
    //	totalRecords=0;
    //	while (!fic.atEnd())
    //	{
    //		record = QString::fromUtf8(fic.readLine());
    //		if (!commentRx.exactMatch(record))
    //			totalRecords++;
    //	}
    //	fic.seek(0);
    //
    //	// Read the constellation art file with the following format :
    //	// ShortName texture_file x1 y1 hp1 x2 y2 hp2
    //	// Where :
    //	// shortname is the international short name (i.e "Lep" for Lepus)
    //	// texture_file is the graphic file of the art texture
    //	// x1 y1 are the x and y texture coordinates in pixels of the star of hipparcos number hp1
    //	// x2 y2 are the x and y texture coordinates in pixels of the star of hipparcos number hp2
    //	// The coordinate are taken with (0,0) at the top left corner of the image file
    //	QString shortname;
    //	QString texfile;
    //	unsigned int x1, y1, x2, y2, x3, y3, hp1, hp2, hp3;
    //	QString tmpstr;
    //
    //	currentLineNumber = 0;	// line in file
    //	readOk = 0;		// count of records processed OK
    //
    //	while (!fic.atEnd())
    //	{
    //		++currentLineNumber;
    //		record = QString::fromUtf8(fic.readLine());
    //		if (commentRx.exactMatch(record))
    //			continue;
    //
    //		// prevent leaving zeros on numbers from being interpretted as octal numbers
    //		record.replace(" 0", " ");
    //		QTextStream rStr(&record);
    //		rStr >> shortname >> texfile >> x1 >> y1 >> hp1 >> x2 >> y2 >> hp2 >> x3 >> y3 >> hp3;
    //		if (rStr.status()!=QTextStream::Ok)
    //		{
    //			qWarning() << "ERROR parsing constellation art record at line" << currentLineNumber << "of art file for culture" << cultureName;
    //			continue;
    //		}
    //
    //		// Draw loading bar
    //// 		lb.SetMessage(q_("Loading Constellation Art: %1/%2").arg(currentLineNumber).arg(totalRecords));
    //// 		lb.Draw((float)(currentLineNumber)/totalRecords);
    //
    //		cons = NULL;
    //		cons = findFromAbbreviation(shortname);
    //		if (!cons)
    //		{
    //			qWarning() << "ERROR in constellation art file at line" << currentLineNumber << "for culture" << cultureName
    //					   << "constellation" << shortname << "unknown";
    //		}
    //		else
    //		{
    //			QString texturePath(texfile);
    //			try
    //			{
    //				texturePath = StelFileMgr::findFile("skycultures/"+cultureName+"/"+texfile);
    //			}
    //			catch (std::runtime_error& e)
    //			{
    //				// if the texture isn't found in the skycultures/[culture] directory,
    //				// try the central textures diectory.
    //				qWarning() << "WARNING, could not locate texture file " << texfile
    //					 << " in the skycultures/" << cultureName
    //					 << " directory...  looking in general textures/ directory...";
    //				try
    //				{
    //					texturePath = StelFileMgr::findFile(QString("textures/")+texfile);
    //				}
    //				catch(exception& e2)
    //				{
    //					qWarning() << "ERROR: could not find texture, " << texfile << ": " << e2.what();
    //				}
    //			}
    //
    //			cons->artTexture = StelApp::getInstance().getTextureManager().createTextureThread(texturePath);
    //
    //			int texSizeX, texSizeY;
    //			if (cons->artTexture==NULL || !cons->artTexture->getDimensions(texSizeX, texSizeY))
    //			{
    //				qWarning() << "Texture dimension not available";
    //			}
    //
    //			const StelNavigator* nav = StelApp::getInstance().getCore()->getNavigator();
    //			Vec3f s1 = hipStarMgr->searchHP(hp1)->getJ2000EquatorialPos(nav);
    //			Vec3f s2 = hipStarMgr->searchHP(hp2)->getJ2000EquatorialPos(nav);
    //			Vec3f s3 = hipStarMgr->searchHP(hp3)->getJ2000EquatorialPos(nav);
    //
    //			// To transform from texture coordinate to 2d coordinate we need to find X with XA = B
    //			// A formed of 4 points in texture coordinate, B formed with 4 points in 3d coordinate
    //			// We need 3 stars and the 4th point is deduced from the other to get an normal base
    //			// X = B inv(A)
    //			Vec3f s4 = s1 + ((s2 - s1) ^ (s3 - s1));
    //			Mat4f B(s1[0], s1[1], s1[2], 1, s2[0], s2[1], s2[2], 1, s3[0], s3[1], s3[2], 1, s4[0], s4[1], s4[2], 1);
    //			Mat4f A(x1, texSizeY - y1, 0.f, 1.f, x2, texSizeY - y2, 0.f, 1.f, x3, texSizeY - y3, 0.f, 1.f, x1, texSizeY - y1, texSizeX, 1.f);
    //			Mat4f X = B * A.inverse();
    //
    //			QVector<Vec3d> contour(4);
    //			contour[0] = X * Vec3f(0., 0., 0.);
    //			contour[1] = X * Vec3f(texSizeX, 0., 0.);
    //			contour[2] = X * Vec3f(texSizeX, texSizeY, 0.);
    //			contour[3] = X * Vec3f(0, texSizeY, 0.);
    //			contour[0].normalize();
    //			contour[1].normalize();
    //			contour[2].normalize();
    //			contour[3].normalize();
    //
    //			QVector<Vec2f> texCoords(4);
    //			texCoords[0].set(0,0);
    //			texCoords[1].set(1,0);
    //			texCoords[2].set(1,1);
    //			texCoords[3].set(0,1);
    //			cons->artPolygon.setContour(contour, texCoords);
    //			Q_ASSERT(cons->artPolygon.checkValid());
    //			++readOk;
    //		}
    //	}
    //
    //	qDebug() << "Loaded" << readOk << "/" << totalRecords << "constellation art records successfully for culture" << cultureName;
    //	fic.close();
    QFile in(fileName);
    if (!in.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qWarning() << "Can't open constellation data file" << fileName  << "for culture" << cultureName;
        Q_ASSERT(0);
    }

    int totalRecords=0;
    QString record;
    QRegExp commentRx("^(\\s*#.*|\\s*)$");
    while (!in.atEnd())
    {
        record = QString::fromUtf8(in.readLine());
        if (!commentRx.exactMatch(record))
            totalRecords++;
    }
    in.seek(0);

    // delete existing data, if any
    vector < Constellation * >::iterator iter;
    for (iter = asterisms.begin(); iter != asterisms.end(); ++iter)
        delete(*iter);

    asterisms.clear();
    Constellation *cons = NULL;

    // read the file, adding a record per non-comment line
    int currentLineNumber = 0;	// line in file
    int readOk = 0;			// count of records processed OK
    while (!in.atEnd())
    {
        record = QString::fromUtf8(in.readLine());
        currentLineNumber++;
        if (commentRx.exactMatch(record))
            continue;

        cons = new Constellation;
        if(cons->read(record, hipStarMgr))
        {
            cons->artFader.setMaxValue(artMaxIntensity);
            cons->setFlagArt(flagArt);
            cons->setFlagBoundaries(flagBoundaries);
            cons->setFlagLines(flagLines);
            cons->setFlagName(flagNames);
            asterisms.push_back(cons);
            ++readOk;
        }
        else
        {
            qWarning() << "ERROR reading constellation rec at line " << currentLineNumber << "for culture" << cultureName;
            delete cons;
        }
    }
    in.close();
    qDebug() << "Loaded" << readOk << "/" << totalRecords << "constellation records successfully for culture" << cultureName;

    // Set current states
    setFlagArt(flagArt);
    setFlagLines(flagLines);
    setFlagLabels(flagNames);
    setFlagBoundaries(flagBoundaries);

    // It's possible to have no art - just constellations
    if (artfileName.isNull() || artfileName.isEmpty())
        return;
    QFile fic(artfileName);
    if (!fic.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qWarning() << "Can't open constellation art file" << fileName  << "for culture" << cultureName;
        return;
    }

    totalRecords=0;
    while (!fic.atEnd())
    {
        record = QString::fromUtf8(fic.readLine());
        if (!commentRx.exactMatch(record))
            totalRecords++;
    }
    fic.seek(0);

    // Read the constellation art file with the following format :
    // ShortName texture_file x1 y1 hp1 x2 y2 hp2
    // Where :
    // shortname is the international short name (i.e "Lep" for Lepus)
    // texture_file is the graphic file of the art texture
    // x1 y1 are the x and y texture coordinates in pixels of the star of hipparcos number hp1
    // x2 y2 are the x and y texture coordinates in pixels of the star of hipparcos number hp2
    // The coordinate are taken with (0,0) at the top left corner of the image file
    QString shortname;
    QString texfile;
    unsigned int x1, y1, x2, y2, x3, y3, hp1, hp2, hp3;

    currentLineNumber = 0;	// line in file
    readOk = 0;		// count of records processed OK

    while (!fic.atEnd())
    {
        ++currentLineNumber;
        record = QString::fromUtf8(fic.readLine());
        if (commentRx.exactMatch(record))
            continue;

        // prevent leaving zeros on numbers from being interpretted as octal numbers
        record.replace(" 0", " ");
        QTextStream rStr(&record);
        rStr >> shortname >> texfile >> x1 >> y1 >> hp1 >> x2 >> y2 >> hp2 >> x3 >> y3 >> hp3;
        if (rStr.status()!=QTextStream::Ok)
        {
            qWarning() << "ERROR parsing constellation art record at line" << currentLineNumber << "of art file for culture" << cultureName;
            continue;
        }

        // Draw loading bar
        // 		lb.SetMessage(q_("Loading Constellation Art: %1/%2").arg(currentLineNumber).arg(totalRecords));
        // 		lb.Draw((float)(currentLineNumber)/totalRecords);

        cons = NULL;
        cons = findFromAbbreviation(shortname);
        if (!cons)
        {
            qWarning() << "ERROR in constellation art file at line" << currentLineNumber << "for culture" << cultureName
                       << "constellation" << shortname << "unknown";
        }
        else
        {
            QString texturePath(texfile);
            try
            {
                texturePath = StelFileMgr::findFile("skycultures/"+cultureName+"/"+texfile);
            }
            catch (std::runtime_error& e)
            {
                // if the texture isn't found in the skycultures/[culture] directory,
                // try the central textures diectory.
                qWarning() << "WARNING, could not locate texture file " << texfile
                           << " in the skycultures/" << cultureName
                           << " directory...  looking in general textures/ directory...";
                try
                {
                    texturePath = StelFileMgr::findFile(QString("textures/")+texfile);
                }
                catch(std::exception& e2)
                {
                    qWarning() << "ERROR: could not find texture, " << texfile << ": " << e2.what();
                }
            }
            cons->textureFile = texfile;
            cons->artTexture = StelApp::getInstance().getTextureManager().createTextureThread(texturePath);

            int texSizeX, texSizeY;
            if (cons->artTexture==NULL || !cons->artTexture->getDimensions(texSizeX, texSizeY))
            {
                qWarning() << "Texture dimension not available";
            }

            StelCore* core = StelApp::getInstance().getCore();
            Vec3d s1 = hipStarMgr->searchHP(hp1)->getJ2000EquatorialPos(core->getNavigator());
            Vec3d s2 = hipStarMgr->searchHP(hp2)->getJ2000EquatorialPos(core->getNavigator());
            Vec3d s3 = hipStarMgr->searchHP(hp3)->getJ2000EquatorialPos(core->getNavigator());

            // To transform from texture coordinate to 2d coordinate we need to find X with XA = B
            // A formed of 4 points in texture coordinate, B formed with 4 points in 3d coordinate
            // We need 3 stars and the 4th point is deduced from the other to get an normal base
            // X = B inv(A)
            Vec3d s4 = s1 + ((s2 - s1) ^ (s3 - s1));
            Mat4d B(s1[0], s1[1], s1[2], 1, s2[0], s2[1], s2[2], 1, s3[0], s3[1], s3[2], 1, s4[0], s4[1], s4[2], 1);
            Mat4d A(x1, texSizeY - y1, 0.f, 1.f, x2, texSizeY - y2, 0.f, 1.f, x3, texSizeY - y3, 0.f, 1.f, x1, texSizeY - y1, texSizeX, 1.f);
            Mat4d X = B * A.inverse();

            // Tesselate on the plan assuming a tangential projection for the image
            static const int nbPoints=5;
            QVector<Vec2f> texCoords;
            texCoords.reserve(nbPoints*nbPoints*6);
            for (int j=0;j<nbPoints;++j)
            {
                for (int i=0;i<nbPoints;++i)
                {
                    texCoords << Vec2f(((float)i)/nbPoints, ((float)j)/nbPoints);
                    texCoords << Vec2f(((float)i+1.f)/nbPoints, ((float)j)/nbPoints);
                    texCoords << Vec2f(((float)i)/nbPoints, ((float)j+1.f)/nbPoints);
                    texCoords << Vec2f(((float)i+1.f)/nbPoints, ((float)j)/nbPoints);
                    texCoords << Vec2f(((float)i+1.f)/nbPoints, ((float)j+1.f)/nbPoints);
                    texCoords << Vec2f(((float)i)/nbPoints, ((float)j+1.f)/nbPoints);
                }
            }

            QVector<Vec3d> contour;
            contour.reserve(texCoords.size());
            foreach (const Vec2f& v, texCoords)
                contour << X * Vec3d(v[0]*texSizeX, v[1]*texSizeY, 0.);

            cons->artPolygon.vertex=contour;
            cons->artPolygon.texCoords=texCoords;
            cons->artPolygon.primitiveType=StelVertexArray::Triangles;

            Vec3d tmp(X * Vec3d(0.5*texSizeX, 0.5*texSizeY, 0.));
            tmp.normalize();
            Vec3d tmp2(X * Vec3d(0., 0., 0.));
            tmp2.normalize();
            cons->boundingCap.n=tmp;
            cons->boundingCap.d=tmp*tmp2;
            ++readOk;
        }
    }

    qDebug() << "Loaded" << readOk << "/" << totalRecords << "constellation art records successfully for culture" << cultureName;
    fic.close();
}

void ConstellationMgr::draw(StelCore* core)
{
    StelNavigator* nav = core->getNavigator();
    const StelProjectorP prj = core->getProjection(StelCore::FrameJ2000);
    StelPainter sPainter(prj);
    sPainter.setFont(asterFont);
    drawLines(sPainter, nav);
    drawNames(sPainter);
    drawArt(sPainter);
    drawBoundaries(sPainter);
}

// Draw constellations art textures
void ConstellationMgr::drawArt(StelPainter& sPainter) const
{
    glBlendFunc(GL_ONE, GL_ONE);
    sPainter.enableTexture2d(true);
    glEnable(GL_BLEND);
    glEnable(GL_CULL_FACE);

    vector < Constellation * >::const_iterator iter;
    SphericalRegionP region = sPainter.getProjector()->getViewportConvexPolygon();
    for (iter = asterisms.begin(); iter != asterisms.end(); ++iter)
    {
        (*iter)->drawArtOptim(sPainter, *region);
    }

    glDisable(GL_CULL_FACE);
}

// Draw constellations lines
void ConstellationMgr::drawLines(StelPainter& sPainter, const StelNavigator* nav) const
{
    sPainter.enableTexture2d(false);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glLineWidth(linesWidth);
    const SphericalCap viewportHalfspace = sPainter.getProjector()->getBoundingSphericalCap();
    vector < Constellation * >::const_iterator iter;
    for (iter = asterisms.begin(); iter != asterisms.end(); ++iter)
    {
        (*iter)->drawOptim(sPainter, nav, viewportHalfspace);
    }
    glLineWidth(1.0f);
}

// Draw the names of all the constellations
void ConstellationMgr::drawNames(StelPainter& sPainter) const
{
    glEnable(GL_BLEND);
    sPainter.enableTexture2d(true);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    vector < Constellation * >::const_iterator iter;
    for (iter = asterisms.begin(); iter != asterisms.end(); iter++)
    {
        // Check if in the field of view
        if (sPainter.getProjector()->projectCheck((*iter)->XYZname.toVec3d(), (*iter)->XYname ))
            (*iter)->drawName(sPainter);
    }
}

Constellation *ConstellationMgr::isStarIn(const StelObject* s) const
{
    vector < Constellation * >::const_iterator iter;
    for (iter = asterisms.begin(); iter != asterisms.end(); ++iter)
    {
        // Check if the star is in one of the constellation
        if ((*iter)->isStarIn(s))
        {
            return (*iter);
        }
    }
    return NULL;
}

Constellation* ConstellationMgr::findFromAbbreviation(const QString& abbreviation) const
{
    // search in uppercase only
    QString tname = abbreviation.toUpper();

    vector < Constellation * >::const_iterator iter;
    for (iter = asterisms.begin(); iter != asterisms.end(); ++iter)
    {
        if ((*iter)->abbreviation == tname)
            return (*iter);
    }
    return NULL;
}

// Can't find constellation from a position because it's not well localized
QList<StelObjectP> ConstellationMgr::searchAround(const Vec3d& v, double limitFov, const StelCore* core) const
{
    return QList<StelObjectP>();
}

void ConstellationMgr::loadNames(const QString& namesFile)
{
    // Constellation not loaded yet
    if (asterisms.empty()) return;

    // clear previous names
    vector < Constellation * >::const_iterator iter;
    for (iter = asterisms.begin(); iter != asterisms.end(); ++iter)
    {
        (*iter)->englishName.clear();
    }

    // Open file
    QFile commonNameFile(namesFile);
    if (!commonNameFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug() << "Cannot open file" << namesFile;
        return;
    }

    // Now parse the file
    // lines to ignore which start with a # or are empty
    QRegExp commentRx("^(\\s*#.*|\\s*)$");

    // lines which look like records - we use the RE to extract the fields
    // which will be available in recRx.capturedTexts()
    QRegExp recRx("^\\s*(\\w+)\\s+\"(.*)\"\\s+_[(]\"(.*)\"[)]\\n");

    // Some more variables to use in the parsing
    Constellation *aster;
    QString record, shortName;

    // keep track of how many records we processed.
    int totalRecords=0;
    int readOk=0;
    int lineNumber=0;
    while (!commonNameFile.atEnd())
    {
        record = QString::fromUtf8(commonNameFile.readLine());
        lineNumber++;

        // Skip comments
        if (commentRx.exactMatch(record))
            continue;

        totalRecords++;

        if (!recRx.exactMatch(record))
        {
            qWarning() << "ERROR - cannot parse record at line" << lineNumber << "in constellation names file" << namesFile;
        }
        else
        {
            shortName = recRx.capturedTexts().at(1);
            aster = findFromAbbreviation(shortName);
            // If the constellation exists, set the English name
            if (aster != NULL)
            {
                aster->nativeName = recRx.capturedTexts().at(2);
                aster->englishName = recRx.capturedTexts().at(3);
                readOk++;
            }
            else
            {
                qWarning() << "WARNING - constellation abbreviation" << shortName << "not found when loading constellation names";
            }
        }
    }
    commonNameFile.close();
    qDebug() << "Loaded" << readOk << "/" << totalRecords << "constellation names";
}

void ConstellationMgr::updateI18n()
{
    StelTranslator trans("stellarium-skycultures", StelFileMgr::getLocaleDir(), StelApp::getInstance().getLocaleMgr().getSkyTranslator().getTrueLocaleName());
    vector < Constellation * >::const_iterator iter;
    for (iter = asterisms.begin(); iter != asterisms.end(); ++iter)
    {
        (*iter)->nameI18 = trans.qtranslate((*iter)->englishName);
    }
}

// update faders
void ConstellationMgr::update(double deltaTime)
{
    vector < Constellation * >::const_iterator iter;
    for (iter = asterisms.begin(); iter != asterisms.end(); ++iter)
    {
        (*iter)->update((int)(deltaTime*1000));
    }
}


void ConstellationMgr::setArtIntensity(double _max)
{
    artMaxIntensity = _max;
    vector < Constellation * >::const_iterator iter;
    for (iter = asterisms.begin(); iter != asterisms.end(); ++iter)
        (*iter)->artFader.setMaxValue(_max);

    StelApp::getInstance().addNetworkCommand("ConstellationMgr.setArtIntensity("+QString("%0").arg(_max)+");");
    QSettings* conf = StelApp::getInstance().getSettings();
    conf->setValue("viewing/constellation_art_intensity", _max);
}

void ConstellationMgr::setArtFadeDuration(float duration)
{
    artFadeDuration = duration;
    vector < Constellation * >::const_iterator iter;
    for (iter = asterisms.begin(); iter != asterisms.end(); ++iter)
        (*iter)->artFader.setDuration((int) (duration * 1000.f));
}

void ConstellationMgr::setFlagLines(bool b)
{
    flagLines = b;
    if (selected.begin() != selected.end()  && isolateSelected)
    {
        vector < Constellation * >::const_iterator iter;
        for (iter = selected.begin(); iter != selected.end(); ++iter)
            (*iter)->setFlagLines(b);
    }
    else
    {
        vector < Constellation * >::const_iterator iter;
        for (iter = asterisms.begin(); iter != asterisms.end(); ++iter)
            (*iter)->setFlagLines(b);
    }
    StelApp::getInstance().addNetworkCommand("ConstellationMgr.setFlagLines("+QString("%0").arg(b)+");");
}

void ConstellationMgr::setFlagBoundaries(bool b)
{
    flagBoundaries = b;
    if (selected.begin() != selected.end() && isolateSelected)
    {
        vector < Constellation * >::const_iterator iter;
        for (iter = selected.begin(); iter != selected.end(); ++iter)
            (*iter)->setFlagBoundaries(b);
    }
    else
    {
        vector < Constellation * >::const_iterator iter;
        for (iter = asterisms.begin(); iter != asterisms.end(); ++iter)
            (*iter)->setFlagBoundaries(b);
    }
    StelApp::getInstance().addNetworkCommand("ConstellationMgr.setFlagBoundaries("+QString("%0").arg(b)+");");
}

void ConstellationMgr::setFlagArt(bool b)
{
    flagArt = b;
    if (selected.begin() != selected.end() && isolateSelected)
    {
        vector < Constellation * >::const_iterator iter;
        for (iter = selected.begin(); iter != selected.end(); ++iter)
            (*iter)->setFlagArt(b);
    }
    else
    {
        vector < Constellation * >::const_iterator iter;
        for (iter = asterisms.begin(); iter != asterisms.end(); ++iter)
            (*iter)->setFlagArt(b);
    }
    StelApp::getInstance().addNetworkCommand("ConstellationMgr.setFlagArt("+QString("%0").arg(b)+");");
}


void ConstellationMgr::setFlagLabels(bool b)
{
    flagNames = b;
    if (selected.begin() != selected.end() && isolateSelected)
    {
        vector < Constellation * >::const_iterator iter;
        for (iter = selected.begin(); iter != selected.end(); ++iter)
            (*iter)->setFlagName(b);
    }
    else
    {
        vector < Constellation * >::const_iterator iter;
        for (iter = asterisms.begin(); iter != asterisms.end(); ++iter)
            (*iter)->setFlagName(b);
    }
    StelApp::getInstance().addNetworkCommand("ConstellationMgr.setFlagLabels("+QString("%0").arg(b)+");");
}

StelObject* ConstellationMgr::getSelected(void) const {
    return *selected.begin();  // TODO return all or just remove this method
}

void ConstellationMgr::LoadDefaultList()
{
    SpringList.clear();
    WinterList.clear();
    SummerList.clear();
    AutumnList.clear();
    leftList.clear();
    middleList.clear();
    rightList.clear();

 /*   SpringList <<"BOO"<<"CVN"<<"COM"<<"CRV"<<"DRA"<<"LIB"<<"LUP"<<"UMA"<<"UMI"<<"VIR"<<"CRB"<<"CEN"<<"HYA"<<"CRT"<<"ANT"<<"VEL"<<"SEX"<<"LEO"<<"LMI"<<"CNC"<<"LYN" ;
    WinterList <<"CMA"<<"CMI"<<"GEM"<<"MON"<<"ORI"<<"AUR"<<"TAU"<<"LEP"<<"COL"<<"PIC"<<"DOR"<<"CAE"<<"HOR";
    SummerList <<"AQL"<<"CRA"<<"CYG"<<"DEL"<<"LYR"<<"SGE"<<"SGR"<<"SCT"<<"VUL"<<"ARA"<<"TEL"<<"SCO"<<"OPH"<<"HER"<<"NOR" ;
    AutumnList <<"AND"<<"ARI"<<"CAM"<<"CAS"<<"CET"<<"ERI"<<"PER"<<"PSC"<<"SCL"<<"TRI"<<"CEP"<<"LAC"<<"PEG"<<"AQR"<<"PSA"<<"PHE"<<"GRU"<<"MIC"<<"CAP"<<"EQU"<<"IND"<<"TUC";
*/
    SpringList <<"Boo"<<"Cvn"<<"Com"<<"Crv"<<"Dra"<<"Lib"<<"Lup"<<"UMa"<<"Umi"<<"Vir"<<"CrB"<<"Cen"<<"Hya"<<"Crt"<<"Ant"<<"Vel"<<"Sex"<<"Leo"<<"LMi"<<"Cnc"<<"Lyn" ;
    WinterList <<"CMa"<<"CMi"<<"Gem"<<"Mon"<<"Ori"<<"Aur"<<"Tau"<<"Lep"<<"Col"<<"Pic"<<"Dor"<<"Cae"<<"Hor";
    SummerList <<"Aql"<<"CrA"<<"Cyg"<<"Del"<<"Lyr"<<"Sge"<<"Sgr"<<"Sct"<<"Vul"<<"Ara"<<"Tel"<<"Sco"<<"Oph"<<"Her"<<"Nor" ;
    AutumnList <<"And"<<"Ari"<<"Cam"<<"Cas"<<"Cet"<<"Eri"<<"Per"<<"Psc"<<"Scl"<<"Tri"<<"Cep"<<"Lac"<<"Peg"<<"Aqr"<<"PsA"<<"Phe"<<"Gru"<<"Mic"<<"Cap"<<"Equ"<<"Ind"<<"Tuc";


    leftList<<"BDI"<<"Umi"<<"UMa"<<"BAS"<<"Boo"<<"SMT"<<"Cyg"<<"Lyr"<<"Aql"<<"Sco"<<"SQP"<<"Peg"<<"And"<<"Cas"<<"Cet"<<"WIT"<<"Ori"<<"CMa"<<"CMi"<<"Tau";
    middleList<<"Vir"<<"Crv"<<"Leo"<<"SPT"<<"Cnc"<<"Sgr"<<"Lib"<<"Oph"<<"Her"<<"CrB"<<"Per"<<"Cap"<<"Aqr"<<"Psc"<<"Ari"<<"Aur"<<"Gem"<<"Mon"<<"WID"<<"Lep";
    rightList<<q_("Spring Const.\nAll On")<<q_("Summer Const.\nAll On")<<q_("Autumn Const.\nAll On")<<q_("Winter Const.\nAll On")<<q_("Change\nArt->Line")<<q_("Ecliptical\nConst.")<<q_("All Const.")<<q_("Constellation\nLabels");

}

void ConstellationMgr::setSelected(const QString& abbreviation)
{
    Constellation * c = findFromAbbreviation(abbreviation);
    if(c != NULL) setSelectedConst(c);
}

StelObjectP ConstellationMgr::setSelectedStar(const QString& abbreviation)
{
    Constellation * c = findFromAbbreviation(abbreviation);
    if(c != NULL) {
        setSelectedConst(c);
        return c->getBrightestStarInConstellation();
    }
    return NULL;
}

void ConstellationMgr::setSelectedConst(Constellation * c)
{
    // update states for other constellations to fade them out
    if (c != NULL)
    {
        selected.push_back(c);

        // Propagate current settings to newly selected constellation
        c->setFlagLines(getFlagLines());
        c->setFlagName(getFlagLabels());
        c->setFlagArt(getFlagArt());
        c->setFlagBoundaries(getFlagBoundaries());

        if (isolateSelected)
        {
            vector < Constellation * >::const_iterator iter;
            for (iter = asterisms.begin(); iter != asterisms.end(); ++iter)
            {

                bool match = 0;
                vector < Constellation * >::const_iterator s_iter;
                for (s_iter = selected.begin(); s_iter != selected.end(); ++s_iter)
                {
                    if( (*iter)==(*s_iter) )
                    {
                        match=true; // this is a selected constellation
                        break;
                    }
                }

                if(!match)
                {
                    // Not selected constellation
                    (*iter)->setFlagLines(false);
                    (*iter)->setFlagName(false);
                    (*iter)->setFlagArt(false);
                    (*iter)->setFlagBoundaries(false);
                }
            }
            Constellation::singleSelected = true;  // For boundaries
        }
        else
            Constellation::singleSelected = false; // For boundaries
    }
    else
    {
        if (selected.begin() == selected.end()) return;

        // Otherwise apply standard flags to all constellations
        vector < Constellation * >::const_iterator iter;
        for (iter = asterisms.begin(); iter != asterisms.end(); ++iter)
        {
            (*iter)->setFlagLines(getFlagLines());
            (*iter)->setFlagName(getFlagLabels());
            (*iter)->setFlagArt(getFlagArt());
            (*iter)->setFlagBoundaries(getFlagBoundaries());
        }

        // And remove all selections
        selected.clear();


    }
}


//! Remove a constellation from the selected constellation list
void ConstellationMgr::unsetSelectedConst(Constellation * c)
{
    if (c != NULL)
    {

        vector < Constellation * >::const_iterator iter;
        int n=0;
        for (iter = selected.begin(); iter != selected.end(); ++iter)
        {
            if( (*iter)->getEnglishName() == c->getEnglishName() )
            {
                selected.erase(selected.begin()+n, selected.begin()+n+1);
                iter--;
                n--;
            }
            n++;
        }

        // If no longer any selection, restore all flags on all constellations
        if (selected.begin() == selected.end()) {

            // Otherwise apply standard flags to all constellations
            for (iter = asterisms.begin(); iter != asterisms.end(); ++iter)
            {
                (*iter)->setFlagLines(getFlagLines());
                (*iter)->setFlagName(getFlagLabels());
                (*iter)->setFlagArt(getFlagArt());
                (*iter)->setFlagBoundaries(getFlagBoundaries());
            }

            Constellation::singleSelected = false; // For boundaries

        } else if(isolateSelected) {

            // No longer selected constellation
            c->setFlagLines(false);
            c->setFlagName(false);
            c->setFlagArt(false);
            c->setFlagBoundaries(false);

            Constellation::singleSelected = true;  // For boundaries
        }
    }
}

int ConstellationMgr::getSelectedCount()
{
    return selected.size();
}

void ConstellationMgr::unsetSelectedConstWithoutAll(Constellation *c)
{
    if (c != NULL)
    {

        vector < Constellation * >::const_iterator iter;
        int n=0;
        for (iter = selected.begin(); iter != selected.end(); ++iter)
        {
            if( (*iter)->getEnglishName() == c->getEnglishName() )
            {
                selected.erase(selected.begin()+n, selected.begin()+n+1);
                iter--;
                n--;
            }
            n++;
        }
        if(isolateSelected) {

            // No longer selected constellation
            c->setFlagLines(false);
            c->setFlagName(false);
            c->setFlagArt(false);
            c->setFlagBoundaries(false);

            Constellation::singleSelected = true;  // For boundaries
        }
    }
}


bool ConstellationMgr::loadBoundaries(const QString& boundaryFile)
{
    Constellation *cons = NULL;
    unsigned int i, j;

    // delete existing boundaries if any exist
    vector<vector<Vec3f> *>::iterator iter;
    for (iter = allBoundarySegments.begin(); iter != allBoundarySegments.end(); ++iter)
    {
        delete (*iter);
    }
    allBoundarySegments.clear();

    qDebug() << "Loading constellation boundary data ... ";

    // Modified boundary file by Torsten Bronger with permission
    // http://pp3.sourceforge.net
    QFile dataFile(boundaryFile);
    if (!dataFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qWarning() << "Boundary file " << boundaryFile << " not found";
        return false;
    }

    QTextStream istr(&dataFile);
    float DE, RA;
    float oDE, oRA;
    Vec3f XYZ;
    unsigned num, numc;
    vector<Vec3f> *points = NULL;
    QString consname;
    i = 0;
    while (!istr.atEnd())
    {
        points = new vector<Vec3f>;

        num = 0;
        istr >> num;
        if(num == 0) continue;  // empty line

        for (j=0;j<num;j++)
        {
            istr >> RA >> DE;

            oRA =RA;
            oDE= DE;

            RA*=M_PI/12.;     // Convert from hours to rad
            DE*=M_PI/180.;    // Convert from deg to rad

            // Calc the Cartesian coord with RA and DE
            StelUtils::spheToRect(RA,DE,XYZ);
            points->push_back(XYZ);
        }

        // this list is for the de-allocation
        allBoundarySegments.push_back(points);

        istr >> numc;
        // there are 2 constellations per boundary

        for (j=0;j<numc;j++)
        {
            istr >> consname;
            // not used?
            if (consname == "SER1" || consname == "SER2") consname = "SER";

            cons = findFromAbbreviation(consname);
            if (!cons)
                qWarning() << "ERROR while processing boundary file - cannot find constellation: " << consname;
            else
                cons->isolatedBoundarySegments.push_back(points);
        }

        if (cons) cons->sharedBoundarySegments.push_back(points);
        i++;

    }
    dataFile.close();
    qDebug() << "Loaded" << i << "constellation boundary segments";
    delete points;

    return true;
}

void ConstellationMgr::drawBoundaries(StelPainter& sPainter) const
{
    sPainter.enableTexture2d(false);
    glDisable(GL_BLEND);
#ifndef USE_OPENGL_ES2
    glLineStipple(2, 0x3333);
    glEnable(GL_LINE_STIPPLE);
    glLineWidth(4.0f); //ASAF
#endif
    vector < Constellation * >::const_iterator iter;
    for (iter = asterisms.begin(); iter != asterisms.end(); ++iter)
    {
        (*iter)->drawBoundaryOptim(sPainter);
    }
#ifndef USE_OPENGL_ES2
    glLineWidth(1.0f); //ASAF
    glDisable(GL_LINE_STIPPLE);
#endif
}

///unsigned int ConstellationMgr::getFirstSelectedHP(void) {
///  if (selected) return selected->asterism[0]->get_hp_number();
///  return 0;
///}

StelObjectP ConstellationMgr::searchByNameI18n(const QString& nameI18n) const
{
    QString objw = nameI18n.toUpper();

    vector <Constellation*>::const_iterator iter;
    for (iter = asterisms.begin(); iter != asterisms.end(); ++iter)
    {
        QString objwcap = (*iter)->nameI18.toUpper();
        if (objwcap==objw) return *iter;
    }
    return NULL;
}

StelObjectP ConstellationMgr::searchByName(const QString& name) const
{
    QString objw = name.toUpper();
    vector <Constellation*>::const_iterator iter;
    for (iter = asterisms.begin(); iter != asterisms.end(); ++iter)
    {
        QString objwcap = (*iter)->englishName.toUpper();
        if (objwcap==objw) return *iter;

        objwcap = (*iter)->abbreviation.toUpper();
        if (objwcap==objw) return *iter;
    }
    return NULL;
}

QStringList ConstellationMgr::listMatchingObjectsI18n(const QString& objPrefix, int maxNbItem) const
{
    QStringList result;
    if (maxNbItem==0) return result;

    QString objw = objPrefix.toUpper();

    vector < Constellation * >::const_iterator iter;
    for (iter = asterisms.begin(); iter != asterisms.end(); ++iter)
    {
        QString constw = (*iter)->getNameI18n().mid(0, objw.size()).toUpper();
        if (constw==objw)
        {
            result << (*iter)->getNameI18n();
            if (result.size()==maxNbItem)
                return result;
        }
    }
    return result;
}

//ASAF
void ConstellationMgr::ShowHideArt(QString name,bool sh)
{
    bool b = sh;
    if (selected.begin() != selected.end() && isolateSelected)
    {
        vector < Constellation * >::const_iterator iter;
        for (iter = selected.begin(); iter != selected.end(); ++iter)
        {
            if ((*iter)->getNameI18n()==name)
            {
                (*iter)->setFlagArt(b);
            }
        }
    }
    else
    {
        vector < Constellation * >::const_iterator iter;
        for (iter = asterisms.begin(); iter != asterisms.end(); ++iter)
        {
            if ((*iter)->getNameI18n()==name)
            {
                (*iter)->setFlagArt(b);
            }
        }
    }
}

QStringList ConstellationMgr::GetAllNameI18n()
{
    QStringList result;

    if (selected.begin() != selected.end() && isolateSelected)
    {
        vector < Constellation * >::const_iterator iter;
        for (iter = selected.begin(); iter != selected.end(); ++iter)
        {
            result.append((*iter)->getNameI18n());
        }
    }
    else
    {
        vector < Constellation * >::const_iterator iter;
        for (iter = asterisms.begin(); iter != asterisms.end(); ++iter)
        {
            result.append((*iter)->getNameI18n());
        }
    }
    return result;
}

QStringList ConstellationMgr::GetAllNames()
{
    QStringList result;

    //    if (selected.begin() != selected.end() && isolateSelected)
    //    {
    //        vector < Constellation * >::const_iterator iter;
    //        for (iter = selected.begin(); iter != selected.end(); ++iter)
    //        {
    //            result.append((*iter)->getEnglishName());
    //        }
    //    }
    //    else
    {
        vector < Constellation * >::const_iterator iter;
        for (iter = asterisms.begin(); iter != asterisms.end(); ++iter)
        {
            result.append((*iter)->getEnglishName());
        }
    }
    return result;
}

QStringList ConstellationMgr::GetAllShortNames()
{
    QStringList result;

    vector < Constellation * >::const_iterator iter;
    for (iter = asterisms.begin(); iter != asterisms.end(); ++iter)
    {
        result.append((*iter)->getShortNameOrj());
    }
    return result;
}

void ConstellationMgr::ShowHideZodiac(bool sh, bool line, bool isname)
{
    vector < Constellation * >::const_iterator iter;
    for (iter = asterisms.begin(); iter != asterisms.end(); ++iter)
    {
        QString name =(*iter)->getEnglishName();
        if (      name=="PSC"
                  || name=="ARI"
                  || name=="TAU"
                  || name=="GEM"
                  || name=="CNC"
                  || name=="LEO"
                  || name=="VIR"
                  || name=="LIB"
                  || name=="SCO"
                  || name=="SGR"
                  || name=="CAP"
                  || name=="AQR")
        {
            (*iter)->setFlagName(isname);
            if(line) (*iter)->setFlagLines(sh);
            else (*iter)->setFlagArt(sh);
            if (sh) selected.push_back((*iter));
            else unsetSelectedConstWithoutAll((*iter));

        }
    }

}

void ConstellationMgr::ShowSummerConst(bool sh, bool line, bool isname)
{
    vector < Constellation * >::const_iterator iter;
    for (iter = asterisms.begin(); iter != asterisms.end(); ++iter)
    {
        QString name =(*iter)->getShortNameOrj();
        //        if (      name=="AQL"
        //                  || name=="CRA"
        //                  || name=="CYG"
        //                  || name=="DEL"
        //                  || name=="LYR"
        //                  || name=="SGE"
        //                  || name=="SGR"
        //                  || name=="SCT"
        //                  || name=="VUL"
        //                  || name=="ARA"
        //                  || name=="TEL"
        //                  || name=="SCO"
        //                  || name=="OPH"
        //                  || name=="HER"
        //                  || name=="NOR"  )
        if (SummerList.indexOf(name)>=0)
        {
            (*iter)->setFlagName(isname);
            if(line) (*iter)->setFlagLines(sh);
            else (*iter)->setFlagArt(sh);
            if (sh) selected.push_back((*iter));
            else unsetSelectedConstWithoutAll((*iter));

        }
    }

}

void ConstellationMgr::ShowSpringConst(bool sh, bool line, bool isname)
{
    vector < Constellation * >::const_iterator iter;
    for (iter = asterisms.begin(); iter != asterisms.end(); ++iter)
    {
        QString name =(*iter)->getShortNameOrj();
        //        if (      name=="BOO"
        //                  || name=="CVN"
        //                  || name=="COM"
        //                  || name=="CRV"
        //                  || name=="DRA"
        //                  || name=="LIB"
        //                  || name=="LUP"
        //                  || name=="UMA"
        //                  || name=="UMI"
        //                  || name=="VIR"
        //                  || name=="CRB"
        //                  || name=="CEN"
        //                  || name=="HYA"
        //                  || name=="CRT"
        //                  || name=="ANT"
        //                  || name=="VEL"
        //                  || name=="SEX"
        //                  || name=="LEO"
        //                  || name=="LMI"
        //                  || name=="CNC"
        //                  || name=="LYN"  )
        if (SpringList.indexOf(name)>=0)
        {
            (*iter)->setFlagName(isname);
            if(line) (*iter)->setFlagLines(sh);
            else (*iter)->setFlagArt(sh);
            if (sh) selected.push_back((*iter));
            else unsetSelectedConstWithoutAll((*iter));

        }
    }


}

void ConstellationMgr::ShowAutumnConst(bool sh, bool line, bool isname)
{
    vector < Constellation * >::const_iterator iter;
    for (iter = asterisms.begin(); iter != asterisms.end(); ++iter)
    {
        QString name =(*iter)->getShortNameOrj();
        //        if (      name=="AND"
        //                  || name=="ARI"
        //                  || name=="CAM"
        //                  || name=="CAS"
        //                  || name=="CET"
        //                  || name=="ERI"
        //                  || name=="PER"
        //                  || name=="PSC"
        //                  || name=="SCL"
        //                  || name=="TRI"
        //                  || name=="CEP"
        //                  || name=="LAC"
        //                  || name=="PEG"
        //                  || name=="AQR"
        //                  || name=="PSA"
        //                  || name=="PHE"
        //                  || name=="GRU"
        //                  || name=="MIC"
        //                  || name=="CAP"
        //                  || name=="EQU"
        //                  || name=="IND"
        //                  || name=="TUC"  )
        if (AutumnList.indexOf(name)>=0)
        {
            (*iter)->setFlagName(isname);
            if(line) (*iter)->setFlagLines(sh);
            else (*iter)->setFlagArt(sh);
            if (sh) selected.push_back((*iter));
            else unsetSelectedConstWithoutAll((*iter));

        }
    }


}

void ConstellationMgr::ShowWinterConst(bool sh, bool line, bool isname)
{
    vector < Constellation * >::const_iterator iter;
    for (iter = asterisms.begin(); iter != asterisms.end(); ++iter)
    {
        QString name=(*iter)->getShortNameOrj();
        //        if (      name=="CMA"
        //                  || name=="CMI"
        //                  || name=="GEM"
        //                  || name=="MON"
        //                  || name=="ORI"
        //                  || name=="AUR"
        //                  || name=="TAU"
        //                  || name=="LEP"
        //                  || name=="COL"
        //                  || name=="PIC"
        //                  || name=="DOR"
        //                  || name=="CAE"
        //                  || name=="HOR" )
        if (WinterList.indexOf(name)>=0)
        {
            (*iter)->setFlagName(isname);
            if(line) (*iter)->setFlagLines(sh);
            else (*iter)->setFlagArt(sh);

            if (sh) selected.push_back((*iter));
            else unsetSelectedConstWithoutAll((*iter));

        }
    }


}

void ConstellationMgr::ShowAllConst(bool sh, bool line, bool isname)
{
    vector < Constellation * >::const_iterator iter;
    for (iter = asterisms.begin(); iter != asterisms.end(); ++iter)
    {
        (*iter)->setFlagName(isname);
        if(line) (*iter)->setFlagLines(sh);
        else (*iter)->setFlagArt(sh);
        if (sh) selected.push_back((*iter));
        else unsetSelectedConstWithoutAll((*iter));
    }

}

void ConstellationMgr::ShowSelectedLabels(bool sh)
{
    vector < Constellation * >::const_iterator iter;
    for (iter = selected.begin(); iter != selected.end(); ++iter)
    {
        (*iter)->setFlagName(sh);
    }
}

void ConstellationMgr::clearAllConst()
{
    ShowAllConst(false,false,false);
    ShowAllConst(false,true,false);
    selected.clear();
}
//ASAF
