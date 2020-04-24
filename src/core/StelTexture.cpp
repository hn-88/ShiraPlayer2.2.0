/*
 * ShiraPlayer(TM)
 * Copyright (C) 2009 Fabien Chereau
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

#include <cstdlib>
#include "StelTextureMgr.hpp"
#include "StelTexture.hpp"
#include "glues.h"
#include "StelFileMgr.hpp"
#include "StelApp.hpp"
#include "StelUtils.hpp"
#include "StelMainGraphicsView.hpp"
#include "StelSkyCultureMgr.hpp"
#include "encrypt/cBinExt.h"

#include "StelOpenGL.hpp"
#include <QThread>
#include <QMutexLocker>
#include <QSemaphore>
#include <QImageReader>
#include <QDir>
#include <QFile>
#include <QTemporaryFile>
#include <QSize>
#include <QtNetwork/QHttpPart>
#include <QDebug>
#include <QUrl>
#include <QImage>
#include <QtNetwork/QNetworkReply>
//#include <QtOpenGL/QGLWidget>

// Initialize statics
QSemaphore* StelTexture::maxLoadThreadSemaphore = new QSemaphore(5);


//  Class used to load an image and set the texture parameters in a thread
class ImageLoadThread : public QThread
{
public:
    ImageLoadThread(StelTexture* tex) : QThread((QObject*)tex), texture(tex) {;}
    virtual void run();
private:
    StelTexture* texture;
};

void ImageLoadThread::run()
{
    StelTexture::maxLoadThreadSemaphore->acquire(1);
    texture->imageLoad();
    StelTexture::maxLoadThreadSemaphore->release(1);
}

StelTexture::StelTexture() : httpReply(NULL), loadThread(NULL), downloaded(false), isLoadingImage(false),
    errorOccured(false),alphaChannel(false), id(0), avgLuminance(-1.f)
{
    mutex = new QMutex();
    width = -1;
    height = -1;
}

StelTexture::~StelTexture()
{
    if (httpReply || (loadThread && loadThread->isRunning()))
    {
        reportError("Aborted (texture deleted)");
    }

    if (httpReply)
    {
        // HTTP is still doing something for this texture. We abort it.
        httpReply->abort();
        delete httpReply;
        httpReply = NULL;
    }

    if (loadThread && loadThread->isRunning())
    {
        // The thread is currently running, it needs to be properly stopped
        loadThread->terminate();
        loadThread->wait(500);
    }

    if (id!=0)
    {
        if (glIsTexture(id)==GL_FALSE)
        {
            qDebug() << "WARNING: in StelTexture::~StelTexture() tried to delete invalid texture with ID=" << id << " Current GL ERROR status is " << glGetError();
        }
        else
        {
            StelMainGraphicsView::getInstance().getOpenGLWin()->deleteTexture(id);
        }
        id = 0;
    }
    delete mutex;
    mutex = NULL;
}

/*************************************************************************
 This method should be called if the texture loading failed for any reasons
 *************************************************************************/
void StelTexture::reportError(const QString& aerrorMessage)
{
    errorOccured = true;
    errorMessage = aerrorMessage;
    // Report failure of texture loading
    emit(loadingProcessFinished(true));
}

QImage StelTexture::decryptImage(QString filename)
{
    cBinExt* m_BinExt = new cBinExt;

    m_BinExt->SetBinPath(filename.toStdString() );
    m_BinExt->DoBufferFile();

    m_BinExt->IsAttachmentPresent();
    QPixmap img = m_BinExt->ExtractImage(filename.toLocal8Bit().constData(),
                                         QString("14531975").toStdString());

    if(m_BinExt != NULL)
    {
        delete m_BinExt;
        m_BinExt = NULL;
    }
    return img.toImage();

}

/*************************************************************************
 Bind the texture so that it can be used for openGL drawing (calls glBindTexture)
 *************************************************************************/
bool StelTexture::bind(int slot)
{
    if (id!=0)
    {
//        // The texture is already fully loaded, just bind and return true;
//        StelApp::makeMainGLContextCurrent();
//#ifdef USE_OPENGL_ES2
//        glActiveTexture(GL_TEXTURE0);
//#endif
//        glBindTexture(GL_TEXTURE_2D, id);
//        return true;

        // The texture is already fully loaded, just bind and return true;
        glActiveTexture(GL_TEXTURE0 + slot);
        glBindTexture(GL_TEXTURE_2D, id);
        return true;
    }
    if (errorOccured)
        return false;

    // The texture is not yet fully loaded
    if (downloaded==false && httpReply==NULL && fullPath.startsWith("http://"))
    {
        // We need to start download
        QNetworkRequest req = QNetworkRequest(QUrl(fullPath));
        // Define that preference should be given to cached files (no etag checks)
        req.setAttribute(QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::PreferCache);
        req.setRawHeader("User-Agent", StelUtils::getApplicationName().toLatin1());
        httpReply = StelApp::getInstance().getNetworkAccessManager()->get(req);
        connect(httpReply, SIGNAL(finished()), this, SLOT(downloadFinished()));
        return false;
    }

    // From this point we assume that fullPath is valid
    // Start loading the image in a thread and return imediately
    if (!isLoadingImage && downloaded==true)
    {
        isLoadingImage = true;
        loadThread = new ImageLoadThread(this);
        connect(loadThread, SIGNAL(finished()), this, SLOT(fileLoadFinished()));
        loadThread->start(QThread::LowestPriority);
    }
    return false;

}


/*************************************************************************
 Called when the download for the texture file terminated
*************************************************************************/
void StelTexture::downloadFinished()
{
    downloadedData = httpReply->readAll();
    downloaded=true;
    if (httpReply->error()!=QNetworkReply::NoError || errorOccured)
    {
        if (httpReply->error()!=QNetworkReply::OperationCanceledError)
            qWarning() << "Texture download failed for " + fullPath+ ": " + httpReply->errorString();
        errorOccured = true;
    }
    httpReply->deleteLater();
    httpReply=NULL;
}

/*************************************************************************
 Called when the file loading thread has terminated
*************************************************************************/
void StelTexture::fileLoadFinished()
{
    glLoad();
}

/*************************************************************************
 Return the width and heigth of the texture in pixels
*************************************************************************/
bool StelTexture::getDimensions(int &awidth, int &aheight)
{
    if (width<0 || height<0)
    {
        //ASAF
        if ((fullPath.contains("skycultures")) && (StelApp::getInstance().getSkyCultureMgr().getSkyCulture().b_encrypt))
        {
            qImage = decryptImage(fullPath);

            width = qImage.width();
            height = qImage.height();
        }
        else
        {
            // Try to get the size from the file without loading data
            QImageReader im(fullPath);
            if (!im.canRead())
            {
                return false;
            }
            QSize size = im.size();
            width = size.width();
            height = size.height();
        }
    }
    awidth = width;
    aheight = height;
    return true;
}

// Load the image data
bool StelTexture::imageLoad()
{
    if (downloadedData.isEmpty())
    {
        // Load the data from the file
        QMutexLocker lock(mutex);
        //ASAF
        if ((fullPath.contains("skycultures")) && (StelApp::getInstance().getSkyCultureMgr().getSkyCulture().b_encrypt))
        {
            // getDimensions da y�kleme yap�ld�.
        }
        else if(fullPath.contains("messiers"))
        {
            // getDimensions da y�kleme yap�ld�.
            qImage = decryptImage(fullPath);
        }
        else
        {
            qImage = QImage(fullPath);
        }
        lock.relock();
    }
    else
    {
        qImage = QImage::fromData(downloadedData);
        // Release the memory
        downloadedData = QByteArray();
    }
    return !qImage.isNull();
}

// Actually load the texture to openGL memory
bool StelTexture::glLoad()
{
    if (qImage.isNull())
    {
        errorOccured = true;
        reportError("Unknown error");
        return false;
    }

#ifdef USE_OPENGL_ES2
    glActiveTexture(GL_TEXTURE0);
#endif

    QGLContext::BindOptions opt = QGLContext::InvertedYBindOption;
    if (loadParams.filtering==GL_LINEAR)
        opt |= QGLContext::LinearFilteringBindOption;

    // Mipmap seems to be pretty buggy on windows..
#ifndef Q_OS_WIN
    if (loadParams.generateMipmaps==true)
        opt |= QGLContext::MipmapBindOption;
#endif

    GLint glformat;
    if (qImage.isGrayscale())
    {
        glformat = qImage.hasAlphaChannel() ? GL_LUMINANCE_ALPHA : GL_LUMINANCE;
    }
    else if (qImage.hasAlphaChannel())
    {
        glformat = GL_RGBA;
    }
    else
        glformat = GL_RGB;
    id = StelMainGraphicsView::getInstance().getOpenGLWin()->bindTexture(qImage, GL_TEXTURE_2D, glformat, opt);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, loadParams.wrapMode);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, loadParams.wrapMode);
    if (loadParams.generateMipmaps)
    {
      /*  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, loadParams.filterMipmaps ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR_MIPMAP_NEAREST);
        glGenerateMipmap(GL_TEXTURE_2D);*/
    }

    StelApp::makeMainGLContextCurrent();

    // Release shared memory
    qImage = QImage();

    // Report success of texture loading
    emit(loadingProcessFinished(false));
    return true;


}
