/*
 * ShiraPlayer(TM)
 * Copyright (C) 2012 Asaf Yurdakul
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

#ifndef VIDEOSOURCE_H_
#define VIDEOSOURCE_H_

#include <QObject>
#include <QDebug>
#include <QtOpenGL/QtOpenGL>

#include "videoutils/videoclass.h"
#include "StelTextureTypes.hpp"
#include "StelCore.hpp"
#include "StelPainter.hpp"

class VideoSource : public QObject {

    Q_OBJECT

public:
    VideoSource(VideoClass *f, GLuint texture,QString presentID ="");
    ~VideoSource();

    GLuint textureIndex;
    bool frameChanged;

    void startShader();
    void update(float opaqueVal, int blackscreen);
    void stopShader();
    //bool isPlayable() const;
    bool isPlaying() const;
    bool isPaused() const;

    inline VideoClass *getVideoFile() const { return is; }

    int getFrameWidth() const { return is->GetVideoSize().width(); }
    int getFrameHeight() const { return is->GetVideoSize().height(); }

    double getStorageAspectRatio() const { return is->GetAspectRatio(); }

    inline void requestUpdate() {
        frameChanged = true;
    }

    QSize videoGeo;
public Q_SLOTS:
    void play(bool on);
    void stop();
    void pause(bool on);
    void init(QSize sizevideo);
    void updateFrame (QByteArray buffer);
    void doStopVideo();

    // OpenGL access to the texture index
    inline GLuint getTextureIndex() {
        return textureIndex;
    }

    void setVideoBrightness(double val){  vbrightness = (float)val; }
    void setVideoContrast(double val){ vcontrast = (float)val;}
    void setVideoSaturation(double val){ vsaturation = (float)val;}

private:

    VideoClass *is;
    //VideoPicture copy;
    QByteArray vp;
    bool videoFinished;

    float vbrightness;
    float vcontrast;
    float vsaturation;

    StelTextureSP mapTexY;
    StelTextureSP mapTexU;
    StelTextureSP mapTexV;
    QGLShaderProgram* videoShaderProgram;

    QSharedMemory videoSharedMem;
    void LoadIntoSharedMomory(QString text);
};

#endif /* VIDEOSOURCE_H_ */
