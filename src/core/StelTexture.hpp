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
#ifndef _STELTEXTURE_HPP_
#define _STELTEXTURE_HPP_

#include "StelTextureTypes.hpp"



#include <QObject>
#include <QImage>
//#include <QtOpenGL/QtOpenGL>

class QMutex;
class QSemaphore;
class QFile;

#ifndef GL_CLAMP_TO_EDGE
#define GL_CLAMP_TO_EDGE 0x812F
#endif

//! @class StelTexture
//! Base texture class. For creating an instance, use StelTextureMgr::createTexture() and StelTextureMgr::createTextureThread()
//! @sa StelTextureSP
class StelTexture : public QObject
{
	Q_OBJECT

public:
	//! Contains the parameters defining how a texture is created.
	struct StelTextureParams
	{
        StelTextureParams(bool qgenerateMipmaps=false, GLint afiltering=GL_LINEAR, GLint awrapMode=GL_CLAMP_TO_EDGE, bool qfilterMipmaps=false) :
				generateMipmaps(qgenerateMipmaps),
                filterMipmaps(qfilterMipmaps),
                filtering(afiltering),
				wrapMode(awrapMode) {;}
		//! Define if mipmaps must be created.
		bool generateMipmaps;
		//! Define the scaling filter to use. Must be one of GL_NEAREST or GL_LINEAR
        bool filterMipmaps;
        //! Define the scaling filter to use. Must be one of GL_NEAREST or GL_LINEAR
        GLint filtering;
		//! Define the wrapping mode to use. Must be one of GL_CLAMP_TO_EDGE, or GL_REPEAT.
		GLint wrapMode;
	};

	//! Destructor
	virtual ~StelTexture();

	//! Bind the texture so that it can be used for openGL drawing (calls glBindTexture).
	//! If the texture is lazyly loaded, this starts the loading and return false immediately.
	//! @return true if the binding successfully occured, false if the texture is not yet loaded.
    bool bind(int slot=0);

	//! Return whether the texture can be binded, i.e. it is fully loaded
	bool canBind() const {return id!=0;}

	//! Return the width and heigth of the texture in pixels
	bool getDimensions(int &width, int &height);

    //! Returns whether the texture has an alpha channel (GL_RGBA or GL_LUMINANCE_ALPHA format)
    //! This only returns valid information after the texture is fully loaded.
    bool hasAlphaChannel() const { return alphaChannel ; }

	//! Get the error message which caused the texture loading to fail
	//! @return the human friendly error message or empty string if no errors occured
	const QString& getErrorMessage() const {return errorMessage;}

	//! Return the full path to the image file.
	//! If the texture was downloaded from a remote location, this function return the full URL.
	const QString& getFullPath() const {return fullPath;}

	//! Return whether the image is currently being loaded
	bool isLoading() const {return isLoadingImage && !canBind();}

	//! Load the texture already in the RAM to the openGL memory
	//! This function uses openGL routines and must be called in the main thread
	//! @return false if an error occured
	bool glLoad();

signals:
	//! Emitted when the texture is ready to be bind(), i.e. when downloaded, imageLoading and	glLoading is over
	//! or when an error occured and the texture will never be available
	//! In case of error, you can query what the problem was by calling getErrorMessage()
	//! @param error is equal to true if an error occured while loading the texture
	void loadingProcessFinished(bool error);

private slots:
	//! Called when the download for the texture file terminated
	void downloadFinished();

	//! Called when the file loading thread has terminated
	void fileLoadFinished();

private:
	friend class StelTextureMgr;
	friend class ImageLoadThread;

	//! Private constructor
	StelTexture();

	//! Loading of the image data.
	//! This method is thread safe
	//! @return false if an error occured
	bool imageLoad();

	//! This method should be called if the texture loading failed for any reasons
	//! @param errorMessage the human friendly error message
	void reportError(const QString& errorMessage);

	StelTextureParams loadParams;

	//! Used to download remote files if needed
	class QNetworkReply* httpReply;

	//! Used to load in thread
	class ImageLoadThread* loadThread;

	//! Define if the texture was already downloaded if it was a remote one
	bool downloaded;
	//! Define whether the image is already loading
	bool isLoadingImage;

	//! The URL where to download the file
	QString fullPath;

	//! The data that was loaded from http
	QByteArray downloadedData;    

    QImage decryptImage(QString filename);
public:
	QImage qImage;

	//! Used ony when creating temporary file
	QString fileExtension;

	//! True when something when wrong in the loading process
	bool errorOccured;

    //! True if this texture contains an alpha channel
    bool alphaChannel;

	//! Human friendly error message if loading failed
	QString errorMessage;

	//! OpenGL id
	GLuint id;

	///////////////////////////////////////////////////////////////////////////
	// Attributes protected by the Mutex
	//! Mutex used to protect all the attributes below
	QMutex* mutex;

	//! Cached average luminance
	float avgLuminance;

	GLsizei width;	//! Texture image width
	GLsizei height;	//! Texture image height

	//! Fix a limit to the number of maximum simultaneous loading threads
	static QSemaphore* maxLoadThreadSemaphore;
};


#endif // _STELTEXTURE_HPP_
