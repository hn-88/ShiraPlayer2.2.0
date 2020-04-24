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
#ifndef _STELTEXTUREMGR_HPP_
#define _STELTEXTUREMGR_HPP_

#include <config.h>

#include <QtOpenGL/QtOpenGL>
#include "StelTexture.hpp"
#include <QObject>

//! @class StelTextureMgr
//! Manage textures loading.
//! It provides method for loading images in a separate thread.
class StelTextureMgr : QObject
{
public:
    StelTextureMgr();
    virtual ~StelTextureMgr();

    //! Initialize some variable from the openGL contex.
    //! Must be called after the creation of the GLContext.
    void init();

    //! Load an image from a file and create a new texture from it
    //! @param filename the texture file name, can be absolute path if starts with '/' otherwise
    //!    the file will be looked in stellarium standard textures directories.
    StelTextureSP createTexture(const QString& filename, const StelTexture::StelTextureParams& params=StelTexture::StelTextureParams());

    //! Load an image from a file and create a new texture from it in a new thread.
    //! @param url the texture file name or URL, can be absolute path if starts with '/' otherwise
    //!    the file will be looked in stellarium standard textures directories.
    //! @param fileExtension the file extension to assume. If not set the extension is determined from url
    //! @param lazyLoading define whether the texture should be actually loaded only when needed, i.e. when bind() is called the first time.
    StelTextureSP createTextureThread(QString url, const StelTexture::StelTextureParams& params=StelTexture::StelTextureParams(), const QString& fileExtension=QString(), bool lazyLoading=true);

    //ASAF
    StelTextureSP createTextureVideo(const StelTexture::StelTextureParams& params,bool isGray = false);//,int TEX_WIDTH,int TEX_HEIGHT);
    StelTextureSP createTextureVideo(const StelTexture::StelTextureParams& params,int TEX_WIDTH,int TEX_HEIGHT);
    StelTextureSP createTextureLogo(const StelTexture::StelTextureParams& params);
    StelTextureSP createTextureLogo(const StelTexture::StelTextureParams& params,QString path);
private:
    friend class StelTexture;

    //! Whether ARB_texture_non_power_of_two is supported on this card
    bool isNoPowerOfTwoAllowed;

};


#endif // _STELTEXTUREMGR_HPP_
