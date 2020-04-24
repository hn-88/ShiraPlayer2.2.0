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


#include "StelTextureMgr.hpp"
#include "StelPresentImageTile.hpp"
#include "StelApp.hpp"
#include "StelFileMgr.hpp"
#include "StelUtils.hpp"
#include "StelTexture.hpp"
#include "StelProjector.hpp"
#include "StelToneReproducer.hpp"
#include "StelCore.hpp"
#include "StelSkyDrawer.hpp"
#include "StelPainter.hpp"
#include "StelMainWindow.hpp"
#include "StelAppGraphicsWidget.hpp"

#include <QDebug>

StelPresentImageTile::StelPresentImageTile()
{
    initCtor();
}

void StelPresentImageTile::initCtor()
{
    minResolution = -1;
    luminance = -1;
    alphaBlend = false;
    noTexture = false;
    texFader = NULL;
    vsource = NULL;
    mixPresentImage = false;
}

// Constructor
StelPresentImageTile::StelPresentImageTile(const QString& url, StelPresentImageTile* parent) : MultiLevelJsonBase(parent)
{
    initCtor();
    if (parent!=NULL)
    {
        luminance = parent->luminance;
        alphaBlend = parent->alphaBlend;
    }
    initFromUrl(url);
}

// Constructor from a map used for JSON files with more than 1 level
StelPresentImageTile::StelPresentImageTile(const QVariantMap& map, StelPresentImageTile* parent) : MultiLevelJsonBase(parent)
{
    initCtor();
    if (parent!=NULL)
    {
        luminance = parent->luminance;
        alphaBlend = parent->alphaBlend;
    }
    initFromQVariantMap(map);

}

// Destructor
StelPresentImageTile::~StelPresentImageTile()
{
}

void StelPresentImageTile::draw(StelCore* core, StelPainter& sPainter, float opacity)
{
    const StelProjectorP prj = core->getProjection(StelCore::FrameJ2000);

    const float limitLuminance = core->getSkyDrawer()->getLimitLuminance();
    QMultiMap<double, StelPresentImageTile*> result;
    getTilesToDraw(result, core, prj->getViewportConvexPolygon(0, 0), limitLuminance, true);

    int numToBeLoaded=0;
    foreach (StelPresentImageTile* t, result)
        if (t->isReadyToDisplay()==false)
            ++numToBeLoaded;
    updatePercent(result.size(), numToBeLoaded);

    // Draw in the good order
    sPainter.enableTexture2d(true);
    QMap<double, StelPresentImageTile*>::Iterator i = result.end();
    while (i!=result.begin())
    {
        --i;
        i.value()->drawTile(core, sPainter,opacity);
    }

    deleteUnusedSubTiles();
}

// Return the list of tiles which should be drawn.
void StelPresentImageTile::getTilesToDraw(QMultiMap<double, StelPresentImageTile*>& result, StelCore* core, const SphericalRegionP& viewPortPoly, float limitLuminance, bool recheckIntersect)
{

    //#ifndef NDEBUG
    //    // When this method is called, we can assume that:
    //    // - the parent tile min resolution was reached
    //    // - the parent tile is intersecting FOV
    //    // - the parent tile is not scheduled for deletion
    //    const StelPresentImageTile* parent = qobject_cast<StelPresentImageTile*>(QObject::parent());
    //    if (parent!=NULL)
    //    {
    //        Q_ASSERT(isDeletionScheduled()==false);
    //        const double degPerPixel = 1./core->getProjection(StelCore::FrameJ2000)->getPixelPerRadAtCenter()*180./M_PI;
    //        Q_ASSERT(degPerPixel<parent->minResolution);
    //
    //        Q_ASSERT(parent->isDeletionScheduled()==false);
    //    }
    //#endif

    // An error occured during loading
    if (errorOccured)
        return;

    //    // The JSON file is currently being downloaded
    //    if (downloading)
    //    {
    //        //qDebug() << "Downloading " << contructorUrl;
    //        return;
    //    }

    //    // Check that we are in the screen
    //    bool fullInScreen = true;
    //    bool intersectScreen = false;
    //    if (recheckIntersect)
    //    {
    //        if (skyConvexPolygons.isEmpty())
    //        {
    //            // If no polygon is defined, we assume that the tile covers the whole sky
    //            fullInScreen=false;
    //            intersectScreen=true;
    //        }
    //        else
    //        {
    //            foreach (const SphericalRegionP& poly, skyConvexPolygons)
    //            {
    //                if (viewPortPoly->contains(poly))
    //                {
    //                    intersectScreen = true;
    //                }
    //                else
    //                {
    //                    fullInScreen = false;
    //                    if (viewPortPoly->intersects(poly))
    //                        intersectScreen = true;
    //                }
    //            }
    //        }
    //    }
    //    // The tile is outside screen
    //    if (fullInScreen==false && intersectScreen==false)
    //    {
    //        // Schedule a deletion
    //        scheduleChildsDeletion();
    //        return;
    //    }
    //
    //    // The tile is in screen, and it is a precondition that its resolution is higher than the limit
    //    // make sure that it's not going to be deleted
    //    cancelDeletion();

    //    if (noTexture==false)
    {
        if (!tex)
        {
            // The tile has an associated texture, but it is not yet loaded: load it now
            StelTextureMgr& texMgr=StelApp::getInstance().getTextureManager();
            //            qDebug()<<"IsServer:"<<StelMainWindow::getInstance().getIsServer()<< "\nStelPresent Video tex:"<< tex
            //                    <<"IsVideo:"<<isVideo;

            if (!isVideo)
                tex = texMgr.createTextureThread(absoluteImageURI, StelTexture::StelTextureParams(true));
            else
            {
                texLogo = texMgr.createTextureLogo(StelTexture::StelTextureParams(true));
                tex = texMgr.createTextureVideo(StelTexture::StelTextureParams(true));//vsource->getFrameWidth(),vsource->getFrameHeight());
            }
            if (!tex)
            {
                qWarning() << "WARNING : Can't create tile: " << absoluteImageURI;
                errorOccured = true;
                return;
            }
        }

        // The tile is in screen and has a texture: every test passed :) The tile will be displayed
        result.insert(minResolution, this);
    }

    // Check if we reach the resolution limit
    const double degPerPixel = 1./core->getProjection(StelCore::FrameJ2000)->getPixelPerRadAtCenter()*180./M_PI;
    if (degPerPixel < minResolution)
    {
        if (subTiles.isEmpty() && !subTilesUrls.isEmpty())
        {
            // Load the sub tiles because we reached the maximum resolution and they are not yet loaded
            foreach (QVariant s, subTilesUrls)
            {
                StelPresentImageTile* nt;
                if (s.type()==QVariant::Map)
                    nt = new StelPresentImageTile(s.toMap(), this);
                else
                {
                    Q_ASSERT(s.type()==QVariant::String);
                    nt = new StelPresentImageTile(s.toString(), this);
                }
                subTiles.append(nt);
            }
        }
        //        // Try to add the subtiles
        //        foreach (MultiLevelJsonBase* tile, subTiles)
        //        {
        //            qobject_cast<StelPresentImageTile*>(tile)->getTilesToDraw(result, core, viewPortPoly, limitLuminance, !fullInScreen);
        //        }
    }
    else
    {
        // The subtiles should not be displayed because their resolution is too high
        scheduleChildsDeletion();
    }
}

// Draw the image on the screen.
bool StelPresentImageTile::drawTile(StelCore* core, StelPainter& sPainter,float opacity)
{
    if (!tex->bind())
        return false;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // Normal transparency mode
    if (mixPresentImage)
        glBlendFunc(GL_ONE, GL_ONE);
    sPainter.setColor(1,1,1, opacity);

    try
    {
       // if( (!StelAppGraphicsWidget::getInstance().isDrawingPreview))
        {
            if(vsource != NULL)
            {
                if(!vsource->isPaused())
                {
                    vsource->startShader();
                    sPainter.xoffsetrate = (vsource->videoGeo.height()*1000.0) / vsource->videoGeo.width()/1000.0;
                    vsource->update(opacity,0);
                }
                else
                    tex->bind();
            }

            foreach (const SphericalRegionP& poly, skyConvexPolygons)
            {

                StelVertexArray varr = poly.data()->getFillVertexArray();

                static const int nbPoints = 20;
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

                Vec3d s1 = varr.vertex[0];
                Vec3d s2 = varr.vertex[1];
                Vec3d s3 = varr.vertex[2];
                Vec3d s4 = varr.vertex[3];
                Mat4d B(s1[0], s1[1], s1[2], 1, s2[0], s2[1], s2[2], 1, s3[0], s3[1], s3[2], 1, s4[0], s4[1], s4[2], 1);
                Mat4d A(varr.texCoords[0].v[0] , varr.texCoords[0].v[1] , 0.f, 1.f,
                        varr.texCoords[1].v[0] , varr.texCoords[1].v[1] , 0.f, 1.f,
                        varr.texCoords[2].v[0] , varr.texCoords[2].v[1] , 0.f, 1.f,
                        varr.texCoords[3].v[0] , varr.texCoords[3].v[1] , 1.f, 1.f);

                Mat4d X = B * A.inverse();

                QVector<Vec3d> contour;
                contour.reserve(texCoords.size());
                foreach (const Vec2f& v, texCoords)
                    contour << X * Vec3d(v[0], v[1], 0.);

                varr.vertex = contour;
                varr.texCoords = texCoords;
                varr.primitiveType = StelVertexArray::Triangles;

                sPainter.drawStelVertexArray(varr,true);
            }

            if(vsource != NULL)
            {
                if(!vsource->isPaused())
                    vsource->stopShader();
            }
        }
        if(vsource != NULL)
        {
            if(!StelMainWindow::getInstance().is_Licenced)
            {
                glColor4f(1.0f,1.0f,1.0f,0.3f);
                glBlendFunc(GL_SRC_ALPHA,GL_ONE);
                if(texLogo!= NULL)
                {
                    if (!texLogo->bind())
                        return false;
                    foreach (const SphericalRegionP& poly, skyConvexPolygons)
                    {
                        sPainter.drawSphericalRegion(poly.data(), StelPainter::SphericalPolygonDrawModeTextureFill,NULL,true,0.0);
                    }
                }
            }
        }
    }
    catch (std::runtime_error &e)
    {
        qWarning() << "StelPresentTile::drawTile error: " << e.what();
    }

    glDisable(GL_BLEND);

    return true;
}

// Return true if the tile is fully loaded and can be displayed
bool StelPresentImageTile::isReadyToDisplay() const
{
    return tex && tex->canBind();
}

// Load the tile from a valid QVariantMap
void StelPresentImageTile::loadFromQVariantMap(const QVariantMap& map)
{
    if (map.contains("imageCredits"))
    {
        QVariantMap dsCredits = map.value("imageCredits").toMap();
        dataSetCredits.shortCredits = dsCredits.value("short").toString();
        dataSetCredits.fullCredits = dsCredits.value("full").toString();
        dataSetCredits.infoURL = dsCredits.value("infoUrl").toString();
    }
    if (map.contains("serverCredits"))
    {
        QVariantMap sCredits = map.value("serverCredits").toMap();
        serverCredits.shortCredits = sCredits.value("short").toString();
        serverCredits.fullCredits = sCredits.value("full").toString();
        serverCredits.infoURL = sCredits.value("infoUrl").toString();
    }
    if (map.contains("description"))
    {
        htmlDescription = map.value("description").toString();
        if (parent()==NULL)
        {
            htmlDescription+= "<h3>URL: "+contructorUrl+"</h3>";
        }
    }
    else
    {
        if (parent()==NULL)
        {
            htmlDescription= "<h3>URL: "+contructorUrl+"</h3>";
        }
    }

    shortName = map.value("shortName").toString();
    if (shortName.isEmpty())
        shortName = "no name";
    bool ok=false;
    if (!map.contains("minResolution"))
        throw std::runtime_error(qPrintable(QString("minResolution is mandatory")));
    minResolution = map.value("minResolution").toDouble(&ok);
    if (!ok)
    {
        throw std::runtime_error(qPrintable(QString("minResolution expect a double value, found: \"%1\"").arg(map.value("minResolution").toString())));
    }

    if (map.contains("luminance"))
    {
        luminance = map.value("luminance").toDouble(&ok);
        if (!ok)
            throw std::runtime_error("luminance expect a float value");
        qWarning() << "luminance in preview JSON files is deprecated. Replace with maxBrightness.";
    }

    if (map.contains("maxBrightness"))
    {
        luminance = map.value("maxBrightness").toDouble(&ok);
        if (!ok)
            throw std::runtime_error("maxBrightness expect a float value");
        luminance = StelApp::getInstance().getCore()->getSkyDrawer()->surfacebrightnessToLuminance(luminance);
    }

    if (map.contains("alphaBlend"))
    {
        alphaBlend = map.value("alphaBlend").toBool();
    }

    // Load the convex polygons (if any)
    QVariantList polyList = map.value("skyConvexPolygons").toList();
    if (polyList.empty())
        polyList = map.value("worldCoords").toList();
    else
        qWarning() << "skyConvexPolygons in preview JSON files is deprecated. Replace with worldCoords.";

    // Load the matching textures positions (if any)
    QVariantList texCoordList = map.value("textureCoords").toList();
    if (!texCoordList.isEmpty() && polyList.size()!=texCoordList.size())
        throw std::runtime_error("the number of convex polygons does not match the number of texture space polygon");

    for (int i=0;i<polyList.size();++i)
    {
        const QVariant& polyRaDec = polyList.at(i);
        QVector<Vec3d> vertices;
        foreach (const QVariant& vRaDec, polyRaDec.toList())
        {
            const QVariantList vl = vRaDec.toList();
            Vec3d v;
            StelUtils::spheToRect(vl.at(0).toDouble(&ok)*M_PI/180., vl.at(1).toDouble(&ok)*M_PI/180., v);
            if (!ok)
                throw std::runtime_error("wrong Ra and Dec, expect a double value");
            vertices.append(v);
        }
        Q_ASSERT(vertices.size()==4);

        if (!texCoordList.isEmpty())
        {
            const QVariant& polyXY = texCoordList.at(i);
            QVector<Vec2f> texCoords;
            foreach (const QVariant& vXY, polyXY.toList())
            {
                const QVariantList vl = vXY.toList();
                texCoords.append(Vec2f(vl.at(0).toDouble(&ok), vl.at(1).toDouble(&ok)));
                if (!ok)
                    throw std::runtime_error("wrong X and Y, expect a double value");
            }
            Q_ASSERT(texCoords.size()==4);

            SphericalTexturedConvexPolygon* pol = new SphericalTexturedConvexPolygon(vertices, texCoords);
            Q_ASSERT(pol->checkValid());

            skyConvexPolygons.clear();
            skyConvexPolygons.append(SphericalRegionP(pol));
            //
            //            //Yeni hesaplar
            //            StelVertexArray artPolygon;
            //            int texSizeX = 406, texSizeY= 405;
            //            //tex->getDimensions(texSizeX, texSizeY);
            //
            //            Vec3d s1 = Vec3d(vertices[0].v[0],vertices[0].v[1],0);
            //            Vec3d s2 = Vec3d(vertices[1].v[0],vertices[1].v[1],0);
            //            Vec3d s3 = Vec3d(vertices[2].v[0],vertices[2].v[1],0);
            //            Vec3d s4 = Vec3d(vertices[3].v[0],vertices[3].v[1],0);
            //            unsigned int x1=406,y1=405,x3=406,y3=0,x2=0,y2=405;
            //
            //                //QMessageBox::information(0,"",QString("%0-%1-%2-%3").arg(s1.v[0]).arg(s1.v[1]).arg(s2.v[0]).arg(s2.v[1]),0,0);
            //
            //            //Vec3d s4 = s1 + ((s2 - s1) ^ (s3 - s1));
            //
            //            Mat4d B(s1[0], s1[1], s1[2], 1, s2[0], s2[1], s2[2], 1, s3[0], s3[1], s3[2], 1, s4[0], s4[1], s4[2], 1);
            //            Mat4d A(x1, texSizeY - y1, 0.f, 1.f, x2, texSizeY - y2, 0.f, 1.f, x3, texSizeY - y3, 0.f, 1.f, x1, texSizeY - y1, texSizeX, 1.f);
            //            Mat4d X = B * A.inverse();
            //
            //            // Tesselate on the plan assuming a tangential projection for the image
            //            static const int nbPoints=5;
            //            QVector<Vec2f> texCoords1;
            //            texCoords1.reserve(nbPoints*nbPoints*6);
            //            for (int j=0;j<nbPoints;++j)
            //            {
            //                for (int i=0;i<nbPoints;++i)
            //                {
            //                    texCoords1 << Vec2f(((float)i)/nbPoints, ((float)j)/nbPoints);
            //                    texCoords1 << Vec2f(((float)i+1.f)/nbPoints, ((float)j)/nbPoints);
            //                    texCoords1 << Vec2f(((float)i)/nbPoints, ((float)j+1.f)/nbPoints);
            //                    texCoords1 << Vec2f(((float)i+1.f)/nbPoints, ((float)j)/nbPoints);
            //                    texCoords1 << Vec2f(((float)i+1.f)/nbPoints, ((float)j+1.f)/nbPoints);
            //                    texCoords1 << Vec2f(((float)i)/nbPoints, ((float)j+1.f)/nbPoints);
            //                }
            //            }
            //
            //            QVector<Vec3d> contour;
            //            contour.reserve(texCoords1.size());
            //            foreach (const Vec2f& v, texCoords1)
            //                contour << X *Vec3d(v[0]*texSizeX, v[1]*texSizeY, 0.);
            //
            //            artPolygon.vertex=contour;
            //            artPolygon.texCoords=texCoords1;
            //            artPolygon.primitiveType=StelVertexArray::Triangles;
            //
            //            skyConvexPolygons.append(artPolygon);


        }
        else
        {
            SphericalConvexPolygon* pol = new SphericalConvexPolygon(vertices);
            Q_ASSERT(pol->checkValid());
            skyConvexPolygons.clear();
            skyConvexPolygons.append(SphericalRegionP(pol));
        }
    }

    if (map.contains("imageUrl"))
    {
        QString imageUrl = map.value("imageUrl").toString();
        if (baseUrl.startsWith("http://"))
        {
            absoluteImageURI = baseUrl+imageUrl;
        }
        else
        {
            try
            {
                absoluteImageURI = StelFileMgr::findFile(baseUrl+imageUrl);
            }
            catch (std::runtime_error& er)
            {
                // Maybe the user meant a file in stellarium local files
                absoluteImageURI = imageUrl;
            }
        }
        // QMessageBox::information(0,absoluteImageURI,absoluteImageURI,0,0);
        if(absoluteImageURI=="")
            isVideo = true;
        else
            isVideo = false;
    }
    //    else
    //    {
    //        noTexture = true;
    //    }

    // This is a list of URLs to the child tiles or a list of already loaded map containing child information
    // (in this later case, the StelPresentImageTile objects will be created later)
    subTilesUrls = map.value("subTiles").toList();
    for (QVariantList::Iterator i=subTilesUrls.begin(); i!=subTilesUrls.end();++i)
    {
        if (i->type()==QVariant::Map)
        {
            // Check if the JSON object is a reference, i.e. if it contains a $ref key
            QVariantMap m = i->toMap();
            if (m.size()==1 && m.contains("$ref"))
            {
                *i=QString(m["$ref"].toString());
            }
        }
    }
    // 	if (subTilesUrls.size()>10)
    // 	{
    // 		qWarning() << "Large tiles number for " << shortName << ": " << subTilesUrls.size();
    // 	}
}

// Convert the image informations to a map following the JSON structure.
QVariantMap StelPresentImageTile::toQVariantMap() const
{
    QVariantMap res;
    QVariantMap polys;

    // Image credits
    QVariantMap imCredits;
    if (!dataSetCredits.shortCredits.isEmpty())
        imCredits["short"]=dataSetCredits.shortCredits;
    if (!dataSetCredits.fullCredits.isEmpty())
        imCredits["full"]=dataSetCredits.fullCredits;
    if (!dataSetCredits.infoURL.isEmpty())
        imCredits["infoUrl"]=dataSetCredits.infoURL;
    if (!imCredits.empty())
        res["imageCredits"]=imCredits;

    // Server credits
    QVariantMap serCredits;
    if (!serverCredits.shortCredits.isEmpty())
        imCredits["short"]=serverCredits.shortCredits;
    if (!serverCredits.fullCredits.isEmpty())
        imCredits["full"]=serverCredits.fullCredits;
    if (!serverCredits.infoURL.isEmpty())
        imCredits["infoUrl"]=serverCredits.infoURL;
    if (!serCredits.empty())
        res["serverCredits"]=serCredits;

    // Misc
    if (!shortName.isEmpty())
        res["shortName"] = shortName;
    if (minResolution>0)
        res["minResolution"]=minResolution;
    if (luminance>0)
        res["maxBrightness"]=StelApp::getInstance().getCore()->getSkyDrawer()->luminanceToSurfacebrightness(luminance);
    if (alphaBlend)
        res["alphaBlend"]=true;
    if (noTexture==false)
        res["imageUrl"]=absoluteImageURI;

    // Polygons
    // TODO
    for (int i=0;i<skyConvexPolygons.count();i++)
    {
        SphericalRegion* cast = dynamic_cast<SphericalRegion*>(skyConvexPolygons[i].data());
        polys=cast->toQVariant();
        res["polygons"] = polys;
    }


    // textures positions
    // TODO

    if (!subTilesUrls.empty())
    {
        res["subTiles"] = subTilesUrls;
    }

    return res;
}
StelTextureSP StelPresentImageTile::getStelTextureSP()
{
    return tex;
}

void StelPresentImageTile::SetVideoSource(VideoSource *vsrc)
{
    this->vsource = vsrc;
}

VideoSource* StelPresentImageTile::GetVideoSource()
{
    return this->vsource;
}

void StelPresentImageTile::setMixWithSky(bool value)
{
    this->mixPresentImage = value;
}



