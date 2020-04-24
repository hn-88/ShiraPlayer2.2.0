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


#ifndef STELPRESENTIMAGETILE_HPP
#define STELPRESENTIMAGETILE_HPP

#include "StelTextureTypes.hpp"
#include "StelSphereGeometry.hpp"
#include "MultiLevelJsonBase.hpp"
#include "StelSkyPolygon.hpp"

#include <QTimeLine>
//todo
#include "presenter/VideoSource.h"

class QIODevice;
class StelCore;
class StelPainter;


//! Base class for any astro image with a fixed position
class StelPresentImageTile : public MultiLevelJsonBase
{
    Q_OBJECT

    friend class StelSkyLayerMgr;

public:
    //! Default constructor
    StelPresentImageTile();

    //! Constructor
    StelPresentImageTile(const QString& url, StelPresentImageTile* parent=NULL);
    //! Constructor
    StelPresentImageTile(const QVariantMap& map, StelPresentImageTile* parent);

    //! Destructor
    ~StelPresentImageTile();

    //! Draw the image on the screen.
    void draw(StelCore* core, StelPainter& sPainter, float opacity=1.);

    //! Return the dataset credits to use in the progress bar
    DataSetCredits getDataSetCredits() const {return dataSetCredits;}

    //! Return the server credits to use in the progress bar
    ServerCredits getServerCredits() const {return serverCredits;}

    //! Return true if the tile is fully loaded and can be displayed
    bool isReadyToDisplay() const;

    //! Convert the image informations to a map following the JSON structure.
    //! It can be saved as JSON using the StelJsonParser methods.
    QVariantMap toQVariantMap() const;

    //! Return the absolute path/URL to the image file
    QString getAbsoluteImageURI() const {return absoluteImageURI;}

    //! Return an HTML description of the image to be displayed in the GUI.
    virtual QString getLayerDescriptionHtml() const {return htmlDescription;}

    //! Reimplement the abstract method.
    //! Load the tile from a valid QVariantMap.
    virtual void loadFromQVariantMap(const QVariantMap& map);

    StelTextureSP getStelTextureSP();

    //todo
    void SetVideoSource(VideoSource *vsrc);
    VideoSource* GetVideoSource();

    void setMixWithSky(bool value);
protected:

    //! The credits of the server where this data come from
    ServerCredits serverCredits;

    //! The credits for the data set
    DataSetCredits dataSetCredits;

    //! URL where the image is located
    QString absoluteImageURI;

    //! The image luminance in cd/m^2
    float luminance;

    //! Whether the texture must be blended
    bool alphaBlend;

    //! True if the tile is just a list of other tiles without texture for itself
    bool noTexture;

    //! list of all the polygons.
    QList<SphericalRegionP> skyConvexPolygons;
    //QList<StelVertexArray> skyConvexPolygons;

    //! The texture of the tile
    StelTextureSP tex;

    //! Minimum resolution of the data of the texture in degree/pixel
    float minResolution;

private:
    //! init the StelPresentImageTile
    void initCtor();

    //! Return the list of tiles which should be drawn.
    //! @param result a map containing resolution, pointer to the tiles
    void getTilesToDraw(QMultiMap<double, StelPresentImageTile*>& result, StelCore* core, const SphericalRegionP& viewPortPoly, float limitLuminance, bool recheckIntersect=true);

    //! Draw the image on the screen.
    //! @return true if the tile was actually displayed
    bool drawTile(StelCore* core, StelPainter& sPainter,float opacity);

    //! Return the minimum resolution
    double getMinResolution() const {return minResolution;}

    //! The list of all the subTiles URL or already loaded JSON map for this tile
    QVariantList subTilesUrls;

    // Used for smooth fade in
    QTimeLine* texFader;

    QString htmlDescription;

    //todo
    VideoSource* vsource;
    bool isVideo;

    bool mixPresentImage;

    StelTextureSP texLogo;
};



#endif // STELPRESENTIMAGETILE_HPP


