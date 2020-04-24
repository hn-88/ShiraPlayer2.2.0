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
#include "StelTextureMgr.hpp"
#include "StelFileMgr.hpp"
#include "StelApp.hpp"
#include "StelMainGraphicsView.hpp"
#include "StelUtils.hpp"

#include <QtNetwork/QHttpPart>
#include <QFileInfo>
#include <QFile>
#include <QDebug>
#include <QThread>
#include <QSettings>
#include <QtOpenGL/QGLFormat>
#include <cstdlib>


StelTextureMgr::StelTextureMgr()
{
}

StelTextureMgr::~StelTextureMgr()
{
}

void StelTextureMgr::init()
{
    StelApp::getInstance().makeMainGLContextCurrent();
    isNoPowerOfTwoAllowed = QGLFormat::openGLVersionFlags().testFlag(QGLFormat::OpenGL_Version_2_0) || QGLFormat::openGLVersionFlags().testFlag(QGLFormat::OpenGL_ES_Version_2_0);
}


StelTextureSP StelTextureMgr::createTexture(const QString& afilename, const StelTexture::StelTextureParams& params)
{
    if (afilename.isEmpty())
        return StelTextureSP();

    StelTextureSP tex = StelTextureSP(new StelTexture());
    try
    {
        tex->fullPath = StelFileMgr::findFile(afilename);
    }
    catch (std::runtime_error er)
    {
        qWarning() << "WARNING : Can't find texture file-1 " << afilename << ": " << er.what() << endl;
        tex->errorOccured = true;
        return StelTextureSP();
    }

#ifdef USE_OPENGL_ES2
    // Allow to replace the texures by compressed .pvr versions using GPU decompression.
    // This saves memory and increases rendering speed.
    QString pvrVersion = tex->fullPath;
    pvrVersion.replace(".png", ".pvr");
    if (StelFileMgr::exists(pvrVersion))
        tex->fullPath = pvrVersion;
#endif

    if (tex->fullPath.endsWith(".pvr"))
    {
        // Load compressed textures using Qt wrapper.
        tex->loadParams = params;
        tex->downloaded = true;

        tex->id = StelMainGraphicsView::getInstance().getOpenGLWin()->bindTexture(tex->fullPath);
        // For some reasons only LINEAR seems to work
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, tex->loadParams.wrapMode);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, tex->loadParams.wrapMode);
        return tex;
    }
    else
    {
        tex->qImage = QImage(tex->fullPath);
        if (tex->qImage.isNull())
            return StelTextureSP();
        tex->loadParams = params;
        tex->downloaded = true;

        if (tex->glLoad())
            return tex;
        else
            return StelTextureSP();
    }
}


StelTextureSP StelTextureMgr::createTextureThread(QString url, const StelTexture::StelTextureParams& params, const QString& fileExtension, bool lazyLoading)
{
    if (url.isEmpty())
        return StelTextureSP();

    StelTextureSP tex = StelTextureSP(new StelTexture());
    tex->loadParams = params;
    if (!url.startsWith("http://"))
    {
        // Assume a local file
        try
        {
            bool enc = false;
            if (url.endsWith(".encrypt"))
            {
                enc = true;
                url = url.remove(".encrypt");
            }
            tex->fullPath  = StelFileMgr::findFile(url);
           // if (enc) tex->fullPath  = tex->fullPath  + ".encrypt";
        }
        catch (std::runtime_error e)
        {
            try
            {
                tex->fullPath = StelFileMgr::findFile("textures/" + url);
            }
            catch (std::runtime_error er)
            {
                qWarning() << "WARNING : Can't find texture file-2 " << url << ": " << er.what() << endl;
                tex->errorOccured = true;
                return StelTextureSP();
            }
        }
        tex->downloaded = true;
    }
    else
    {
        tex->fullPath = url;
        if (fileExtension.isEmpty())
        {
            const int idx = url.lastIndexOf('.');
            if (idx!=-1)
                tex->fileExtension = url.right(url.size()-idx-1);
        }
    }
    if (!fileExtension.isEmpty())
        tex->fileExtension = fileExtension;
    if (!lazyLoading)
        tex->bind();
    return tex;
}
//ASAF
StelTextureSP StelTextureMgr::createTextureVideo(const StelTexture::StelTextureParams& params,bool isGray)
{
    StelTextureSP tex = StelTextureSP(new StelTexture());
    //tex->qImage = QImage(TEX_WIDTH,TEX_HEIGHT,QImage::Format_RGB888);
    if (isGray)
        tex->qImage = QImage(1,1,QImage::Format_Indexed8);
    else
        tex->qImage = QImage(1,1,QImage::Format_RGB888);
    tex->qImage.fill(24);

    if (tex->qImage.isNull())
        return StelTextureSP();
    tex->loadParams = params;
    tex->loadParams.generateMipmaps = false;
    tex->downloaded = true;

    if (tex->glLoad())
        return tex;
    else
        return StelTextureSP();
}
StelTextureSP StelTextureMgr::createTextureVideo(const StelTexture::StelTextureParams& params,int TEX_WIDTH,int TEX_HEIGHT)
{
    StelTextureSP tex = StelTextureSP(new StelTexture());
    tex->qImage = QImage(TEX_WIDTH,TEX_HEIGHT,QImage::Format_RGB888);
    //tex->qImage = QImage(1,1,QImage::Format_RGB888);
    tex->qImage.fill(24);

    if (tex->qImage.isNull())
        return StelTextureSP();
    tex->loadParams = params;
    tex->downloaded = true;

    if (tex->glLoad())
        return tex;
    else
        return StelTextureSP();
}
StelTextureSP StelTextureMgr::createTextureLogo(const StelTexture::StelTextureParams& params)
{
    StelTextureSP tex = StelTextureSP(new StelTexture());
    tex->qImage = QImage( ":/mainWindow/gui/logo_ekranflat.png" );

    if (tex->qImage.isNull())
        return StelTextureSP();
    tex->loadParams = params;
    tex->downloaded = true;

    if (tex->glLoad())
        return tex;
    else
        return StelTextureSP();
}
StelTextureSP StelTextureMgr::createTextureLogo(const StelTexture::StelTextureParams& params,QString path)
{
    StelTextureSP tex = StelTextureSP(new StelTexture());
    tex->qImage = QImage(path);

    if (tex->qImage.isNull())
        return StelTextureSP();
    tex->loadParams = params;
    tex->downloaded = true;

    if (tex->glLoad())
        return tex;
    else
        return StelTextureSP();
}
