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

#ifndef STELPRESENTMGR_HPP
#define STELPRESENTMGR_HPP

#include <QString>
#include <QStringList>
#include <QMap>
#include <QVariantMap>
#include <QtOpenGL/QtOpenGL>
#include "StelModule.hpp"
#include "StelSkyLayer.hpp"
#include "presenter/VideoSource.h"

class StelCore;
class StelPresentImageTile;
class QProgressBar;

class StelPresentMgr : public StelModule
{
    Q_OBJECT
public:
    StelPresentMgr();
    ~StelPresentMgr();

    ///////////////////////////////////////////////////////////////////////////
    // Methods defined in the StelModule class
    //! Initialize
    virtual void init();

    //! Draws sky background
    virtual void draw(StelCore* core);

    //! Update state which is time dependent.
    virtual void update(double deltaTime) {;}

    //! Update i18
    virtual void updateI18n() {;}

    //! Determines the order in which the various modules are drawn.
    virtual double getCallOrder(StelModuleActionName actionName) const;

    ///////////////////////////////////////////////////////////////////////////
    // Other specific methods
    //! Add a new layer.
    //! @param show defined whether the layer should be shown by default
    //! @return the reference key to use when accessing this layer later on.
    QString insertSkyPresent(StelSkyLayerP l,const QString& keyHint=QString(), bool show=true,bool domeORsky=false);

    //! Remove a layer.
    void removePresentImage(StelSkyLayerP l);

    //! Get the list of all the currently loaded layers.
    QMap<QString, StelSkyLayerP> getAllSkyLayers() const;

    StelSkyLayerP getPresentImage(const QString& key) const;

    QString activePresentImageID;

public slots:

    void setFlagShow(bool b) {flagShow = b;}
    bool getFlagShow() const {return flagShow;}
    //! Load an image from a file. This should not be called directly from
    //! scripts because it is not thread safe.  Instead use the simiarly
    //! named function in the core scripting object.
    //! @param id a string identifier for the image
    //! @param filename the name of the image file to load.  Will be
    //! searched for using StelFileMgr, so partial names are fine.
    //! @param ra0 right ascention of corner 0 in degrees
    //! @param dec0 declenation of corner 0 in degrees
    //! @param ra1 right ascention of corner 1 in degrees
    //! @param dec1 declenation of corner 1 in degrees
    //! @param ra2 right ascention of corner 2 in degrees
    //! @param dec2 declenation of corner 2 in degrees
    //! @param ra3 right ascention of corner 3 in degrees
    //! @param dec3 declenation of corner 3 in degrees
    //! @param minRes the minimum resolution setting for the image
    //! @param maxBright the maximum brightness setting for the image
    //! @param visible initial visibility setting
    //! @param screenORdome resim kubbede mi ekranda m�
    bool loadPresentImage(const QString& id, const QString& filename,
                                      double ra0, double dec0,
                                      double ra1, double dec1,
                                      double ra2, double dec2,
                                      double ra3, double dec3,
                                      double minRes, double maxBright, bool visible, bool domeORsky);


    //! Convenience function which allows loading of a sky image based on a
    //! central coordinate, angular size and rotation.
    //! @param id a string ID to be used when referring to this
    //! image (e.g. when changing the displayed status or deleting it.
    //! @param filename the file name of the image.  If a relative
    //! path is specified, "scripts/" will be prefixed before the
    //! image is searched for using StelFileMgr.
    //! @param ra The right ascension of the center of the image in J2000 frame degrees
    //! @param dec The declenation of the center of the image in J2000 frame degrees
    //! @param angSize The angular size of the image in arc minutes
    //! @param rotation The clockwise rotation angle of the image in degrees
    //! @param minRes The minimum resolution setting for the image
    //! @param maxBright The maximum brightness setting for the image
    //! @param visible The initial visibility of the image
    //! @param screenORdome resim kubbede mi ekranda m�
    bool loadPresentImage(const QString& id, const QString& filename,
                                      double ra, double dec, double angSize, double rotation,
                                      double minRes=2.5, double maxBright=14, bool visible=true, bool domeORsky=false,double aspect=1.0);

    bool loadPresentVideo(const QString& id, const QString& filename,
                                      double ra, double dec, double angSize, double rotation,
                                      double minRes=2.5, double maxBright=14, bool visible=true, bool domeORsky=false,double aspect=1.0);

    void playPresentVideo(const QString& id,bool status);

    void pauseTogglePresentVideo(const QString& id);

    void removePresentVideo(const QString& id);

    ///////////////////////////////////////////////////////////////////////////
    // Other slots
    //! Remove a present from the list.
    //! Note: this is not thread safe, and so should not be used directly
    //! from scripts - use the similarly named function in the core
    //! scripting API object to delete SkyLayers.
    //! @param key the reference key (id) generated by insertSkyImage.
    void removePresentImage(const QString& key);

    //! Decide to show or not to show a layer by it's ID.
    //! @param id the id of the layer whose status is to be changed.
    //! @param b the new shown value:
    //! - true means the specified image will be shown.
    //! - false means the specified image will not be shown.
    void showPresentImage(const QString& id, bool b);

    void removeWithFade(const QString& id);

    void setLayerProperties(const QString& id,QVariantMap vmNew);
    void setLayerProperties(const QString& id, const QString& filename,
                                            double ra, double dec, double angSize, double rotation,
                                            double minRes, double maxBright,double aspect,bool isVideo);
    void setLayerProperties(const QString& id, const QString& command);

    void setLayerdomeORsky(const QString& id,bool bscreen);
    GLuint getLayerTexture(const QString& id);

    void LayerFaderfinished();


    void SetLayerVideoSource(const QString& id, VideoSource* vsrc);
    VideoSource* GetLayerVideoSource(const QString& id);

    void setPVideoContrast(int presentID,double value);
    void setPVideoBrightness(int presentID, double value);
    void setPVideoSaturation(int presentID, double value);

    void setMixWithSky(QString presentID, bool value);

private slots:
    void finishedFadeForRemove();

private:

    //! Store the informations needed for a graphical element layer.
    class SkyPresentElem
    {
    public:
        SkyPresentElem(StelSkyLayerP t, bool show=true ,bool domeORsky=false);
        ~SkyPresentElem();
        StelSkyLayerP layer;
        QProgressBar* progressBar;
        bool show;
        bool domeORsky;
    };

    SkyPresentElem* skyLayerElemForPresent(const StelSkyLayer*);

    QString keyForLayer(const StelSkyLayer*);

    //! Map image key/layer
    QMap<QString, SkyPresentElem*> allSkyPresents;

    // Whether to draw at all
    bool flagShow;

    VideoSource* vsource;
};

#endif // STELPRESENTMGR_HPP
