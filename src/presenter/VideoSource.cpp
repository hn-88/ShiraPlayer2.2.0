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


#include "videoutils/videoclass.h"
#include "VideoSource.h"

#include "StelMainWindow.hpp"
#include "StelApp.hpp"
#include "StelTexture.hpp"
#include "StelTextureMgr.hpp"
#include "StelNavigator.hpp"

VideoSource::VideoSource(VideoClass *f, GLuint texture,QString presentID) :
    QObject(), is(f),textureIndex(texture)
{
    if (!is) return;
    try
    {
        connect(is, SIGNAL(preparedBuffer(QByteArray)), this, SLOT(updateFrame(QByteArray)));
        connect(is, SIGNAL(finished()), this, SLOT(doStopVideo()));

        mapTexY = StelApp::getInstance().getTextureManager().createTextureVideo(StelTexture::StelTextureParams(true),true);
        mapTexU = StelApp::getInstance().getTextureManager().createTextureVideo(StelTexture::StelTextureParams(true),true);
        mapTexV = StelApp::getInstance().getTextureManager().createTextureVideo(StelTexture::StelTextureParams(true),true);

        videoSharedMem.setKey("videopresentSharedMemory"+presentID);
        frameChanged = false;

        vbrightness = 1.0f;
        vcontrast = 1.0f;
        vsaturation = 1.0f;
    }
    catch (std::runtime_error& e)
    {
        qWarning() << "WARNING: unable create VideoSource " << e.what();
    }
}

VideoSource::~VideoSource()
{
    if (is)
        delete is;

    // free the OpenGL texture
    glDeleteTextures(1, &textureIndex);
}

void VideoSource::startShader()
{
    Q_ASSERT(videoShaderProgram);
    videoShaderProgram->bind();
}

bool VideoSource::isPlaying() const
{
    return !(is->IsFinished());
}

void VideoSource::play(bool on)
{
    if (on)
    {
        videoFinished = false;
        if(is->IsPaused())
        {
            is->SeekByFrame(0);
            is->PauseVideo(false,true);
        }
        else
            is->StartVideo();
    }
    else
    {
        is->PauseVideo(true,true);
    }
}

void VideoSource::stop()
{
    is->StopVideo();
}

bool VideoSource::isPaused() const
{
    return isPlaying() && is->IsPaused();
}

void VideoSource::pause(bool on)
{
    //if (on != isPaused())
    is->PauseVideo(on,true);
}

void VideoSource::init(QSize sizevideo)
{
    videoGeo = QSize(sizevideo.width(),sizevideo.height());
    videoShaderProgram= new QGLShaderProgram();
    QGLShader* fShader = new QGLShader(QGLShader::Fragment);
    if (!fShader->compileSourceCode(
                "#version 120\n"
                "uniform sampler2D t_texture_y;\n"
                "uniform sampler2D t_texture_u;\n"
                "uniform sampler2D t_texture_v;\n"
                "uniform int blackscreen = 1;\n"
                "uniform float opaque = 1.0;\n"
                "uniform float brightness = 1.0;\n"
                "uniform float contrast = 1.0;\n"
                "uniform float saturation = 1.0;\n"
                ""
                // For all settings: 1.0 = 100% 0.5=50% 1.5 = 150%\n"
                "vec3 ContrastSaturationBrightness(vec3 color, float brt, float sat, float con)\n"
                "{;\n"
                "    // Increase or decrease theese values to adjust r, g and b color channels seperately\n"
                "    const float AvgLumR = 0.5;\n"
                "    const float AvgLumG = 0.5;\n"
                "    const float AvgLumB = 0.5;\n"
                ""
                "    const vec3 LumCoeff = vec3(0.2125, 0.7154, 0.0721);\n"
                ""
                "    vec3 AvgLumin = vec3(AvgLumR, AvgLumG, AvgLumB);\n"
                "    vec3 brtColor = color * brt;\n"
                "    vec3 intensity = vec3(dot(brtColor, LumCoeff));\n"
                "    vec3 satColor = mix(intensity, brtColor, sat);\n"
                "    vec3 conColor = mix(AvgLumin, satColor, con);\n"
                "    return conColor;\n"
                "}\n"
                ""
                "void main(){\n"
                "float y,u,v,r,g,b;\n"
                "const float c1 = float(255.0/219.0);\n"
                "const float c2 = float(16.0/256.0);\n"
                "const float c3 = float(1.0/2.0);\n"
                "const float c4 = float(1596.0/1000.0);\n"
                "const float c5 = float(391.0/1000.0);\n"
                "const float c6 = float(813.0/1000.0);\n"
                "const float c7 = float(2018.0/1000.0);\n"
                "y = texture2D(t_texture_y, gl_TexCoord[0].xy).r;\n"
                "u = texture2D(t_texture_u, gl_TexCoord[0].xy).r;\n"
                "v = texture2D(t_texture_v, gl_TexCoord[0].xy).r;\n"
                "\n"
                "y = c1 * (y - c2);\n"
                "u = u - c3;\n"
                "v = v - c3;\n"
                "\n"
                "r = clamp(y + c4 * v, 0.0, 1.0);\n"
                "g = clamp(y - c5 * u - c6 * v, 0.0, 1.0);\n"
                "b = clamp(y + c7 * u, 0.0, 1.0);\n"
                "\n"
                "vec3 rgb = ContrastSaturationBrightness(vec3(r,g,b),brightness,saturation,contrast);"
                "\n"
                "gl_FragColor = vec4(rgb[0],rgb[1],rgb[2],opaque);\n"
                "if (blackscreen == 1)\n"
                "	gl_FragColor = vec4(0.0, 0.0, 0.0, 1.0);\n"
                "}\n"))
    {
        qWarning() << "Error while compiling fragment shader: " << fShader->log();
    }

    if (!fShader->log().isEmpty())
    {
        qWarning() << "Warnings while compiling fragment shader: " << fShader->log();
    }
    videoShaderProgram->addShader(fShader);
    if (!videoShaderProgram->link())
    {
        qWarning() << "Error while linking shader program: " << videoShaderProgram->log();
    }
    if (!videoShaderProgram->log().isEmpty())
    {
        qWarning() << "Warnings while linking shader: " << videoShaderProgram->log();
    }

    mapTexY->bind();
    glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D ( GL_TEXTURE_2D, 0, GL_LUMINANCE ,videoGeo.width(), videoGeo.height(), 0, GL_LUMINANCE, GL_UNSIGNED_BYTE,0 );

    mapTexU->bind();
    glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D ( GL_TEXTURE_2D, 0, GL_LUMINANCE, videoGeo.width() /2 ,videoGeo.height() /2, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE,0);

    mapTexV->bind();
    glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D ( GL_TEXTURE_2D, 0, GL_LUMINANCE, videoGeo.width() /2 ,videoGeo.height() /2, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, 0);

}

// only Rendering Manager can call this
void VideoSource::update(float opaqueVal, int blackscreen)
{
    // update texture
    if (frameChanged && !vp.isEmpty())
    {
        glEnable(GL_CULL_FACE);
        glDisable(GL_BLEND);
        int _idxU = videoGeo.width() * videoGeo.height();
        int _idxV = _idxU + (_idxU / 4);

        videoShaderProgram->setUniformValue("blackscreen",blackscreen);
        videoShaderProgram->setUniformValue("opaque",opaqueVal);
        videoShaderProgram->setUniformValue("brightness",vbrightness);
        videoShaderProgram->setUniformValue("contrast",vcontrast);
        videoShaderProgram->setUniformValue("saturation",vsaturation);

        mapTexU->bind(1);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        videoShaderProgram->setUniformValue("t_texture_u",1);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, videoGeo.width() /2, videoGeo.height() /2, GL_LUMINANCE, GL_UNSIGNED_BYTE, &vp.data()[_idxU]);

        mapTexV->bind(2);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        videoShaderProgram->setUniformValue("t_texture_v",2);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0,  videoGeo.width() /2, videoGeo.height() /2, GL_LUMINANCE, GL_UNSIGNED_BYTE,  &vp.data()[_idxV]);

        mapTexY->bind(0);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        videoShaderProgram->setUniformValue("t_texture_y",0);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, videoGeo.width(), videoGeo.height(), GL_LUMINANCE, GL_UNSIGNED_BYTE, vp.data());

        glEnable(GL_BLEND);
        glDisable(GL_CULL_FACE);
        frameChanged = false;


    }
}

void VideoSource::stopShader()
{
    videoShaderProgram->release();
}

void VideoSource::updateFrame(QByteArray buffer)
{
    frameChanged = true;
    vp = buffer;
    if(!StelMainWindow::getInstance().isServer)
    {
        int f_percent = (int) ( (double)( is->GetCurrentFrameNumber()) / (double)( is->GetVideoDuration()) * 1000.0) ;
        QString f_text = is->GetTimeStrFromFrame(is->GetCurrentFrameNumber());

        if((!this->isPaused()) && (this->isPlaying()))
            LoadIntoSharedMomory(QString("%1@%2").arg(f_percent).arg(f_text));
    }
}

void VideoSource::doStopVideo()
{
    if(is)
    {
        if(!videoFinished)
        {
            videoFinished = true;
        }
    }
}

void VideoSource::LoadIntoSharedMomory(QString text)
{
    if (videoSharedMem.isAttached())
    {
        videoSharedMem.detach();
    }
    if(text.length())
    {
        // load into shared memory
        QBuffer buffer;
        buffer.open(QBuffer::ReadWrite);
        QDataStream out(&buffer);
        out << text;
        int size = buffer.size();

        if (!videoSharedMem.create(size)) {
            return;
        }
        videoSharedMem.lock();
        char *to = (char*)videoSharedMem.data();
        const char *from = buffer.data().data();
        memcpy(to, from, qMin(videoSharedMem.size(), size));
        videoSharedMem.unlock();
    }

}
