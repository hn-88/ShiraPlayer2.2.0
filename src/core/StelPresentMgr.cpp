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


#include "StelPresentMgr.hpp"
#include "StelModuleMgr.hpp"
#include "StelApp.hpp"
#include "StelCore.hpp"
#include "StelFileMgr.hpp"
#include "StelProjector.hpp"
#include "StelPresentImageTile.hpp"
#include "StelPainter.hpp"
#include "StelGuiBase.hpp"
#include "StelCore.hpp"
#include "StelTexture.hpp"
#include "StelMainWindow.hpp"

#include <QtOpenGL/QtOpenGL>
#include <QtNetwork/QNetworkAccessManager>
#include <stdexcept>
#include <QDebug>
#include <QString>
#include <QProgressBar>
#include <QVariantMap>
#include <QVariantList>
#include <QSettings>

StelPresentMgr::StelPresentMgr(void) : flagShow(true)
{
    setObjectName("StelPresentMgr");
}
StelPresentMgr::~StelPresentMgr()
{
    foreach (SkyPresentElem* s, allSkyPresents)
        delete s;
}

StelPresentMgr::SkyPresentElem::SkyPresentElem(StelSkyLayerP t, bool ashow,bool adomeORsky) : layer(t),progressBar(NULL), show(ashow),domeORsky(adomeORsky)
{;}

StelPresentMgr::SkyPresentElem::~SkyPresentElem()
{
    if (progressBar)
        progressBar->deleteLater();
    progressBar = NULL;
}
/*************************************************************************
 Reimplementation of the getCallOrder method
*************************************************************************/
double StelPresentMgr::getCallOrder(StelModuleActionName actionName) const
{
    if (actionName==StelModule::ActionDraw)
        return StelApp::getInstance().getModuleMgr().getModule("LandscapeMgr")->getCallOrder(actionName)+10;
    
    return 0;
}

StelPresentMgr::SkyPresentElem* StelPresentMgr::skyLayerElemForPresent(const StelSkyLayer* t)
{
    foreach (SkyPresentElem* e, allSkyPresents)
    {
        if (e->layer==t)
        {
            return e;
        }
    }
    return NULL;
}

QString StelPresentMgr::keyForLayer(const StelSkyLayer* t)
{
    return allSkyPresents.key(skyLayerElemForPresent(t));
}

// read from stream
void StelPresentMgr::init()
{
    
}

// Draw all the multi-res images collection
void StelPresentMgr::draw(StelCore* core)
{
    if (!flagShow) return;

    double curfov = core->currentProjectorParams.fov;

    foreach (SkyPresentElem* s, allSkyPresents)
    {
        if(s->domeORsky == false)// Sky
        {
            core->currentProjectorParams.fov = curfov;
            StelPainter sPainter(core->getProjection(StelCore::FrameAltAz));
            glBlendFunc(GL_ONE, GL_ONE);
            glEnable(GL_BLEND);
            s->layer->draw(core, sPainter, s->layer->texFader->currentValue());
        }

    }

    foreach (SkyPresentElem* s, allSkyPresents)
    {
        if(s->domeORsky == true)// Dome
        {
            core->currentProjectorParams.fov=180.0;
            StelPainter sPainter(core->getProjection(StelCore::FrameScreen));
            glBlendFunc(GL_ONE, GL_ONE);
            glEnable(GL_BLEND);
            s->layer->draw(core, sPainter, s->layer->texFader->currentValue());
        }

    }
}

bool StelPresentMgr::loadPresentImage(const QString& id, const QString& filename,
                                      double ra, double dec,
                                      double angSize, double rotation,
                                      double minRes, double maxBright,
                                      bool visible, bool domeORsky,double aspect)
{
    ra = -1 * ra;
    rotation =rotation+90.0;

    Vec3d XYZ;
    const double RADIUS_NEB = 1.;
    StelUtils::spheToRect(ra*M_PI/180., dec*M_PI/180., XYZ);
    XYZ*=RADIUS_NEB;
    double texSize = RADIUS_NEB * sin(angSize/2/60*M_PI/180);
    Mat4d matPrecomp =  Mat4d::translation(XYZ) *
            Mat4d::zrotation(ra*M_PI/180.) *
            Mat4d::yrotation(-dec*M_PI/180.) *
            Mat4d::xrotation(rotation*M_PI/180.);
    
    Vec3d corners[4];

    corners[0] = matPrecomp * Vec3d(0.f,-texSize,-texSize*aspect);
    corners[1] = matPrecomp * Vec3d(0.f,-texSize, texSize*aspect);
    corners[2] = matPrecomp * Vec3d(0.f, texSize,-texSize*aspect);
    corners[3] = matPrecomp * Vec3d(0.f, texSize, texSize*aspect);

    // convert back to ra/dec (radians)
    Vec3d cornersRaDec[4];
    for(int i=0; i<4; i++)
        StelUtils::rectToSphe(&cornersRaDec[i][0], &cornersRaDec[i][1], corners[i]);
    
    return loadPresentImage(id, filename,
                            cornersRaDec[0][0]*180./M_PI, cornersRaDec[0][1]*180./M_PI,
            cornersRaDec[1][0]*180./M_PI, cornersRaDec[1][1]*180./M_PI,
            cornersRaDec[3][0]*180./M_PI, cornersRaDec[3][1]*180./M_PI,
            cornersRaDec[2][0]*180./M_PI, cornersRaDec[2][1]*180./M_PI,
            minRes, maxBright, visible, domeORsky);
}

bool StelPresentMgr::loadPresentVideo(const QString &id, const QString &filename, double ra, double dec, double angSize, double rotation, double minRes, double maxBright, bool visible, bool domeORsky, double aspect)
{
    if( (!StelMainWindow::getInstance().getIsServer()) ||
            (StelMainWindow::getInstance().getIsMultiprojector()) )
    {
        VideoClass* vfile= new VideoClass();
        vfile->setFlatVideo(true);
        vfile->OpenVideo(filename);
        double r_aspectratio = vfile->GetAspectRatio();
        loadPresentImage(id,"",ra,dec,angSize,rotation,minRes,
                         maxBright,visible,domeORsky,r_aspectratio);

        vsource = new VideoSource(vfile,0);
        vsource->init(vfile->GetVideoSize());
        SetLayerVideoSource(id,vsource);
    }
}

void StelPresentMgr::playPresentVideo(const QString &id, bool status)
{
    if( (!StelMainWindow::getInstance().getIsServer()) ||
            (StelMainWindow::getInstance().getIsMultiprojector()) )
    {
        if(vsource != NULL)
            vsource->play(status);
    }
}

void StelPresentMgr::pauseTogglePresentVideo(const QString &id)
{
    if( (!StelMainWindow::getInstance().getIsServer()) ||
            (StelMainWindow::getInstance().getIsMultiprojector()) )
    {
        if(vsource != NULL)
        {
            if (vsource->isPaused())
                vsource->pause(false);
            else
                vsource->pause(true);
        }
    }
}

void StelPresentMgr::removePresentVideo(const QString &id)
{
    if( (!StelMainWindow::getInstance().getIsServer()) ||
            (StelMainWindow::getInstance().getIsMultiprojector()) )
    {
        activePresentImageID = id;

        if(vsource != NULL)
        {
            vsource->stop();
            vsource= NULL;
            delete vsource;
            removePresentImage(id);
        }
    }
}

bool  StelPresentMgr::loadPresentImage(const QString& id, const QString& filename,
                                       double ra0, double dec0,
                                       double ra1, double dec1,
                                       double ra2, double dec2,
                                       double ra3, double dec3,
                                       double minRes, double maxBright, bool visible, bool domeORsky)
{
    if (allSkyPresents.contains("id"))
    {
        qWarning() << "Image ID" << id << "already exists, removing old image before loading";
        removePresentImage(id);
    }

    QString m_filename = filename;
    //#ifndef SHIRAPLAYER_PRE
    if(StelMainWindow::getInstance().getIsMultiprojector())
    {
        if (!StelMainWindow::getInstance().getIsServer() && m_filename!="")
        {
            QStringList lst = m_filename.split("/");
            QSettings* conf= StelApp::getInstance().getSettings();
            QString fmedia_path = conf->value("main/flat_media_path", "C:/").toString();
            m_filename =  fmedia_path +"/" +lst[lst.count()-1];
        }
    }
    //#endif


    QString path;
    // Possible exception sources:
    // - StelFileMgr file not found
    // - list index out of range in insertSkyImage
    try
    {
        if(m_filename!="")
        {
            /*  bool enc = false;
            if (m_filename.endsWith(".encrypt"))
            {
                enc = true;
                m_filename = m_filename.remove(".encrypt");
            }*/
            path = StelFileMgr::findFile(m_filename);
            // if (enc) path = path + ".encrypt";
        }
        else
            path ="";
        
        QVariantMap vm;
        QVariantList l;
        QVariantList cl; // coordinates list for adding worldCoords and textureCoords
        QVariantList c;  // a list for a pair of coordinates
        QVariantList ol; // outer list - we want a structure 3 levels deep...
        vm["imageUrl"] = QVariant(path);
        vm["maxBrightness"] = QVariant(maxBright);
        vm["minResolution"] = QVariant(minRes);
        vm["shortName"] = QVariant(id);
        
        // textureCoords (define the ordering of worldCoords)
        cl.clear();
        ol.clear();
        if(m_filename!= "")
        {
            c.clear(); c.append(0); c.append(0); cl.append(QVariant(c));
            c.clear(); c.append(1); c.append(0); cl.append(QVariant(c));
            c.clear(); c.append(1); c.append(1); cl.append(QVariant(c));
            c.clear(); c.append(0); c.append(1); cl.append(QVariant(c));
        }
        else
        {
            c.clear(); c.append(0); c.append(1); cl.append(QVariant(c));
            c.clear(); c.append(1); c.append(1); cl.append(QVariant(c));
            c.clear(); c.append(1); c.append(0); cl.append(QVariant(c));
            c.clear(); c.append(0); c.append(0); cl.append(QVariant(c));
        }
        ol.append(QVariant(cl));
        vm["textureCoords"] = ol;
        
        // world coordinates
        cl.clear();
        ol.clear();
        
        c.clear(); c.append(ra0); c.append(dec0); cl.append(QVariant(c));
        c.clear(); c.append(ra1); c.append(dec1); cl.append(QVariant(c));
        c.clear(); c.append(ra2); c.append(dec2); cl.append(QVariant(c));
        c.clear(); c.append(ra3); c.append(dec3); cl.append(QVariant(c));
        
        ol.append(QVariant(cl));
        vm["worldCoords"] = ol;

        StelSkyLayerP tile = StelSkyLayerP(new StelPresentImageTile(vm, 0));
        tile->willchangeProject= false;
        QString key = insertSkyPresent(tile, m_filename, visible,domeORsky);
        if(visible)
            tile->setFlagShow(visible);
        connect(tile->texFader,SIGNAL(finished()),this,SLOT(LayerFaderfinished()));
        activePresentImageID = id;
        if (key == id)
            return true;
        else
            return false;
    }
    catch (std::runtime_error& e)
    {
        qWarning() << "Could not find image" << m_filename << ":" << e.what();
        return false;
    }
}

QString StelPresentMgr::insertSkyPresent(StelSkyLayerP tile, const QString& keyHint, bool ashow,bool domeORsky)
{
    SkyPresentElem* bEl = new SkyPresentElem(tile, ashow,domeORsky);
    QString key = tile->getKeyHint();
    if (key.isEmpty() || key=="no name")
    {
        if (!keyHint.isEmpty())
        {
            key = keyHint;
        }
        else
        {
            key = "no name";
        }
    }
    if (allSkyPresents.contains(key))
    {
        QString suffix = "_01";
        int i=1;
        while (allSkyPresents.contains(key+suffix))
        {
            suffix=QString("_%1").arg(i);
            ++i;
        }
        key+=suffix;
    }
    allSkyPresents.insert(key,bEl);

    return key;
}


// Remove a sky image tile from the list of background images
void StelPresentMgr::removePresentImage(const QString& key)
{
    //qDebug() << "StelSkyLayerMgr::removeSkyImage removing image:" << key;
    if (allSkyPresents.contains(key))
    {
        SkyPresentElem* bEl = allSkyPresents[key];
        delete bEl;
        allSkyPresents.remove(key);
    }
    else
    {
        qDebug() << "StelPresentMgr::removeSkyLayer there is no such key" << key << "nothing is removed";
    }
}

// Remove a sky image tile from the list of background images
void StelPresentMgr::removePresentImage(StelSkyLayerP l)
{
    const QString k = keyForLayer(l.data());
    if (!k.isEmpty())
        removePresentImage(k);
}
//! Get the list of all the currently loaded layers.
QMap<QString, StelSkyLayerP> StelPresentMgr::getAllSkyLayers() const
{
    QMap<QString, StelSkyLayerP> res;
    for (QMap<QString, StelPresentMgr::SkyPresentElem*>::const_iterator iter=allSkyPresents.begin();iter!=allSkyPresents.end();++iter)
    {
        qDebug() << iter.key() << iter.value()->layer->getShortName();
        res.insert(iter.key(), iter.value()->layer);
    }
    return res;
}

StelSkyLayerP StelPresentMgr::getPresentImage(const QString& key) const
{
    if (allSkyPresents.contains(key))
        return allSkyPresents[key]->layer;
    return StelSkyLayerP();
}
void StelPresentMgr::showPresentImage(const QString& id, bool b)
{
    if (allSkyPresents.contains(id))
        if (allSkyPresents.value(id)!=NULL)
        {
            activePresentImageID = id;
            allSkyPresents[id]->layer->setFlagShow(b);
        }
}

void StelPresentMgr::finishedFadeForRemove()
{
    StelSkyLayer* layer= qobject_cast<StelSkyLayer*>(sender());
    QString k = keyForLayer(layer);
    if (!k.isEmpty())
        removePresentImage(k);
}

void StelPresentMgr::removeWithFade(const QString &id)
{
    if (allSkyPresents.contains(id))
        if (allSkyPresents.value(id)!=NULL)
        {
            activePresentImageID = id;
            connect(allSkyPresents[id]->layer.data(),SIGNAL(texFaderFinished()),this,SLOT(finishedFadeForRemove()));
            allSkyPresents[id]->layer->setFlagShow(false);
        }
}

void StelPresentMgr::setLayerProperties(const QString& id, const QString& command)
{
    //QMessageBox::information(0,id,command,0,0);
    QStringList data = command.split(";");
    if (data.count() == 9)
    {
        setLayerProperties(id,data[0],
                QString(data[1]).toDouble(),
                QString(data[2]).toDouble(),
                QString(data[3]).toDouble(),
                QString(data[4]).toDouble(),
                QString(data[5]).toDouble(),
                QString(data[6]).toDouble(),
                QString(data[7]).toDouble(),
                QString(data[8]).toInt());
    }
}

void StelPresentMgr::setLayerProperties(const QString& id, const QString& filename,
                                        double ra, double dec, double angSize, double rotation,
                                        double minRes, double maxBright,double aspect,bool isVideo)
{
    QString m_filename = filename;
    //#ifndef SHIRAPLAYER_PRE
    if(StelMainWindow::getInstance().getIsMultiprojector())
    {
        if (!StelMainWindow::getInstance().getIsServer())
        {
            QStringList lst = m_filename.split("/");
            QSettings* conf= StelApp::getInstance().getSettings();
            QString fmedia_path = conf->value("main/flat_media_path", "C:/").toString();
            m_filename =  fmedia_path +"/" +lst[lst.count()-1];
        }
    }
    //#endif

    ra= -1*ra;
    rotation =rotation+90.0;

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

    corners[0] = matPrecomp * Vec3f(0.f,-texSize,-texSize*aspect);
    corners[1] = matPrecomp * Vec3f(0.f,-texSize, texSize*aspect);
    corners[2] = matPrecomp * Vec3f(0.f, texSize,-texSize*aspect);
    corners[3] = matPrecomp * Vec3f(0.f, texSize, texSize*aspect);

    
    // convert back to ra/dec (radians)
    Vec3d cornersRaDec[4];
    for(int i=0; i<4; i++)
        StelUtils::rectToSphe(&cornersRaDec[i][0], &cornersRaDec[i][1], corners[i].toVec3d());
    

    double ra0 = cornersRaDec[0][0]*180./M_PI;  double dec0 = cornersRaDec[0][1]*180./M_PI;
    double ra1 = cornersRaDec[1][0]*180./M_PI;  double dec1 = cornersRaDec[1][1]*180./M_PI;
    double ra2 = cornersRaDec[3][0]*180./M_PI;  double dec2 = cornersRaDec[3][1]*180./M_PI;
    double ra3 = cornersRaDec[2][0]*180./M_PI;  double dec3 = cornersRaDec[2][1]*180./M_PI;

    
    QString path;
    try
    {
        if(m_filename!="")
        {
            /* bool enc = false;
            if (m_filename.endsWith(".encrypt"))
            {
                enc = true;
                m_filename = m_filename.remove(".encrypt");
            }*/
            path = StelFileMgr::findFile(m_filename);
            //if (enc) path = path + ".encrypt";
        }
        else
            path ="";
        
        QVariantMap vm;
        QVariantList l;
        QVariantList cl; // coordinates list for adding worldCoords and textureCoords
        QVariantList c;  // a list for a pair of coordinates
        QVariantList ol; // outer list - we want a structure 3 levels deep...
        vm["imageUrl"] = QVariant(path);
        vm["maxBrightness"] = QVariant(maxBright);
        vm["minResolution"] = QVariant(minRes);
        vm["shortName"] = QVariant(id);
        
        // textureCoords (define the ordering of worldCoords)
        cl.clear();
        ol.clear();
        if(!isVideo)
        {
            c.clear(); c.append(0); c.append(0); cl.append(QVariant(c));
            c.clear(); c.append(1); c.append(0); cl.append(QVariant(c));
            c.clear(); c.append(1); c.append(1); cl.append(QVariant(c));
            c.clear(); c.append(0); c.append(1); cl.append(QVariant(c));
        }
        else
        {
            c.clear(); c.append(0); c.append(1); cl.append(QVariant(c));
            c.clear(); c.append(1); c.append(1); cl.append(QVariant(c));
            c.clear(); c.append(1); c.append(0); cl.append(QVariant(c));
            c.clear(); c.append(0); c.append(0); cl.append(QVariant(c));
        }
        ol.append(QVariant(cl));
        vm["textureCoords"] = ol;
        
        // world coordinates
        cl.clear();
        ol.clear();
        
        c.clear(); c.append(ra0); c.append(dec0); cl.append(QVariant(c));
        c.clear(); c.append(ra1); c.append(dec1); cl.append(QVariant(c));
        c.clear(); c.append(ra2); c.append(dec2); cl.append(QVariant(c));
        c.clear(); c.append(ra3); c.append(dec3); cl.append(QVariant(c));
        
        ol.append(QVariant(cl));
        vm["worldCoords"] = ol;
        
        setLayerProperties(id,vm);
    }
    catch (std::runtime_error& e)
    {
        qWarning() << "Could not find image" << m_filename << ":" << e.what();
    }
}

void StelPresentMgr::setLayerProperties(const QString& id,QVariantMap vmNew)
{   
    if (allSkyPresents.contains(id))
        if (allSkyPresents.value(id)!=NULL)
        {
            StelPresentImageTile* cast = dynamic_cast<StelPresentImageTile*>(allSkyPresents[id]->layer.data());
            cast->loadFromQVariantMap(vmNew);
            activePresentImageID = id;
        }

}

void StelPresentMgr::setLayerdomeORsky(const QString& id,bool bscreen)
{
    if (allSkyPresents.contains(id))
        if (allSkyPresents.value(id)!=NULL)
        {
            activePresentImageID = id;
            allSkyPresents[id]->domeORsky = bscreen;
        }
}
void StelPresentMgr::LayerFaderfinished()
{
    if (allSkyPresents.contains(activePresentImageID))
        if (allSkyPresents.value(activePresentImageID)!=NULL)
        {
            if(allSkyPresents[activePresentImageID]->layer->willchangeProject)
            {
                allSkyPresents[activePresentImageID]->layer->willchangeProject = false;
                showPresentImage(activePresentImageID,true);
                setLayerdomeORsky(activePresentImageID,!allSkyPresents[activePresentImageID]->domeORsky);
            }
        }
}
GLuint StelPresentMgr::getLayerTexture(const QString& id)
{
    if (allSkyPresents.contains(id))
    {
        if (allSkyPresents.value(id)!=NULL)
        {
            StelPresentImageTile* cast = dynamic_cast<StelPresentImageTile*>(allSkyPresents[id]->layer.data());
            StelTextureSP texSP = cast->getStelTextureSP();
            StelTexture* stexture = dynamic_cast<StelTexture*>(texSP.data());

            return stexture->id;
        }
    }
    else
        return NULL;
}

void StelPresentMgr::SetLayerVideoSource(const QString& id, VideoSource* vsrc)
{
    if (allSkyPresents.contains(id))
    {
        if (allSkyPresents.value(id) != NULL)
        {
            activePresentImageID = id;
            StelPresentImageTile* cast = dynamic_cast<StelPresentImageTile*>(allSkyPresents[id]->layer.data());
            cast->SetVideoSource(vsrc);
        }
    }
}

VideoSource* StelPresentMgr::GetLayerVideoSource(const QString& id)
{    
    if (allSkyPresents.contains(id))
    {
        if (allSkyPresents.value(id) != NULL)
        {
            activePresentImageID = id;
            StelPresentImageTile* cast = dynamic_cast<StelPresentImageTile*>(allSkyPresents[id]->layer.data());
            return cast->GetVideoSource();
        }
    }
    else
        return NULL;
}


void StelPresentMgr::setPVideoContrast(int presentID, double value)
{
    if (GetLayerVideoSource(QString::number(presentID)))
        GetLayerVideoSource(QString::number(presentID))->setVideoContrast(value);
}
void StelPresentMgr::setPVideoBrightness(int presentID,double value)
{
    if (GetLayerVideoSource(QString::number(presentID)))
        GetLayerVideoSource(QString::number(presentID))->setVideoBrightness(value);
}
void StelPresentMgr::setPVideoSaturation(int presentID,double value)
{
    if (GetLayerVideoSource(QString::number(presentID)))
        GetLayerVideoSource(QString::number(presentID))->setVideoSaturation(value);
}

void StelPresentMgr::setMixWithSky(QString presentID, bool value)
{
    if (allSkyPresents.contains(presentID))
        if (allSkyPresents.value(presentID)!=NULL)
        {
            StelPresentImageTile* cast = dynamic_cast<StelPresentImageTile*>(allSkyPresents[presentID]->layer.data());
            cast->setMixWithSky(value);
        }
}

