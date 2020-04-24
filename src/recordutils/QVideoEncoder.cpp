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

#include <QPainter>
#include "QVideoEncoder.h"
#include "ffmpeg.h"



/******************************************************************************
*******************************************************************************
* QVideoEncoder   QVideoEncoder   QVideoEncoder   QVideoEncoder   QVideoEncoder
*******************************************************************************
******************************************************************************/


/******************************************************************************
* PUBLIC   PUBLIC   PUBLIC   PUBLIC   PUBLIC   PUBLIC   PUBLIC   PUBLIC   PUBLIC
******************************************************************************/

/**
  gop: maximal interval in frames between keyframes
**/
#include <QMessageBox>
#include <QDebug>

QVideoEncoder::QVideoEncoder()
{
    initVars();
    initCodec();
}

QVideoEncoder::~QVideoEncoder()
{
    close();
    /*if(Initmodefile)
   {
      writeFooter();
      Outdev->close();
      delete Outdev;
      Outdev=0;
   }
   else
   {
      // nothing to do
   }*/
}

bool QVideoEncoder::createFile(QString fileName,unsigned width,unsigned height,unsigned bitrate,unsigned gop,unsigned fps)
{
    // If we had an open video, close it.
    close();

    Width=width;
    Height=height;
    Gop=gop;
    Bitrate=bitrate;

    if(!isSizeValid())
    {
        qWarning()<<"Record Manager-"<<"Invalid size\n";
        return false;
    }

    pOutputFormat = av_guess_format(NULL, fileName.toStdString().c_str(), NULL);
    if (!pOutputFormat) {
        qWarning()<<"Record Manager-"<<"Could not deduce output format from file extension: using MPEG.\n";
        pOutputFormat = av_guess_format("mpeg", NULL, NULL);
        if (pOutputFormat == NULL)
        {
           qWarning()<<"Record Manager-"<<"Could not guess output format";
           return false;
        }
    }

    pFormatCtx=avformat_alloc_context();
    if(!pFormatCtx)
    {
        qWarning()<<"Record Manager-"<<"Error allocating format context\n";
        return false;
    }
    pFormatCtx->oformat = pOutputFormat;
    //   snprintf(pFormatCtx->filename, sizeof(pFormatCtx->filename), "%s", fileName.toStdString().c_str());


    // Add the video stream

    pVideoStream = avformat_new_stream(pFormatCtx,0);
    if(!pVideoStream )
    {
        qWarning()<<"Record Manager-"<<"Could not allocate stream\n";
        return false;
    }


    pCodecCtx=pVideoStream->codec;
    pCodecCtx->codec_id = pOutputFormat->video_codec;
    pCodecCtx->codec_type = AVMEDIA_TYPE_VIDEO;

    pCodecCtx->bit_rate = Bitrate;
    pCodecCtx->width = getWidth();
    pCodecCtx->height = getHeight();
    pCodecCtx->time_base.den = fps;
    pCodecCtx->time_base.num = 1;
    pCodecCtx->gop_size = Gop;
    pCodecCtx->pix_fmt = PIX_FMT_YUV420P;
    if (fps < 15)
        pCodecCtx->bit_rate_tolerance = Bitrate/5;

    //deprecated
    //avcodec_thread_init(pCodecCtx, 10);
    pCodecCtx->thread_count = 10;

    //if (c->codec_id == CODEC_ID_MPEG2VIDEO)
    //{
    //c->max_b_frames = 2;  // just for testing, we also add B frames
    //}

    // some formats want stream headers to be separate
    if(pFormatCtx->oformat->flags & AVFMT_GLOBALHEADER)
        pCodecCtx->flags |= CODEC_FLAG_GLOBAL_HEADER;


   /* if (av_set_parameters(pFormatCtx, NULL) < 0)
    {
        //printf("Invalid output format parameters\n");
        qWarning()<<"Record Manager-"<<"Invalid output format parameters\n";
        return false;
    }*/

    av_dump_format(pFormatCtx, 0, fileName.toStdString().c_str(), 1);

    // open_video
    // find the video encoder
    pCodec = avcodec_find_encoder(pCodecCtx->codec_id);
    if (!pCodec)
    {
        //printf("codec not found\n");
        qWarning()<<"Record Manager-"<<"codec not found\n";
        return false;
    }
    // open the codec
    if (avcodec_open2(pCodecCtx, pCodec,NULL) < 0)
    {
        //printf("could not open codec\n");
        qWarning()<<"Record Manager-"<<"could not open codec\n";
        return false;
    }

    // Allocate memory for output
    if(!initOutputBuf())
    {
        //printf("Can't allocate memory for output bitstream\n");
        qWarning()<<"Record Manager-"<<"Can't allocate memory for output bitstream\n";
        return false;
    }

    // Allocate the YUV frame
    if(!initFrame())
    {
        //printf("Can't init frame\n");
        qWarning()<<"Record Manager-"<<"Can't init frame\n";
        return false;
    }

    if (avio_open(&pFormatCtx->pb, fileName.toStdString().c_str(), AVIO_FLAG_WRITE) < 0)
    {
        //printf( "Could not open '%s'\n", fileName.toStdString().c_str());
        qWarning()<<"Record Manager-"<<"Could not open '%s'\n", fileName.toStdString().c_str();
        return false;
    }

    avformat_write_header(pFormatCtx,NULL);

    ok=true;

    return true;
}

/**
   \brief Completes writing the stream, closes it, release resources.
**/
bool QVideoEncoder::close()
{
    if(!isOk())
        return false;

    av_write_trailer(pFormatCtx);

    // close_video

    avcodec_close(pVideoStream->codec);
    freeFrame();
    freeOutputBuf();


    /* free the streams */

    for(int i = 0; i < pFormatCtx->nb_streams; i++)
    {
        av_freep(&pFormatCtx->streams[i]->codec);
        av_freep(&pFormatCtx->streams[i]);
    }

    // Close file
    avio_close(pFormatCtx->pb);

    // Free the stream
    av_free(pFormatCtx);

    initVars();
    return true;
}


/**
   \brief Encode one frame

   The frame must be of the same size as specifie
**/
int QVideoEncoder::encodeImage(const QImage &img)
{
    if(!isOk())
        return -1;

    //convertImage(img);       // Custom conversion routine
    convertImage_sws(img);     // SWS conversion

    int out_size = avcodec_encode_video(pCodecCtx,outbuf,outbuf_size,ppicture);
    //printf("Frame size: %d\n",out_size);
    if (out_size > 0)
    {

        av_init_packet(&pkt);

        //if (pCodecCtx->coded_frame->pts != AV_NOPTS_VALUE)
        if (pCodecCtx->coded_frame->pts != (0x8000000000000000LL))
            pkt.pts= av_rescale_q(pCodecCtx->coded_frame->pts, pCodecCtx->time_base, pVideoStream->time_base);
        if(pCodecCtx->coded_frame->key_frame)
            pkt.flags |= AV_PKT_FLAG_KEY;

        pkt.stream_index= pVideoStream->index;
        pkt.data= outbuf;
        pkt.size= out_size;
        int ret = av_interleaved_write_frame(pFormatCtx, &pkt);
        //printf("Wrote %d\n",ret);
        if(ret<0)
            return -1;
    }
    return out_size;
}


/******************************************************************************
* INTERNAL   INTERNAL   INTERNAL   INTERNAL   INTERNAL   INTERNAL   INTERNAL
******************************************************************************/

void QVideoEncoder::initVars()
{
    ok=false;
    pFormatCtx=0;
    pOutputFormat=0;
    pCodecCtx=0;
    pVideoStream=0;
    pCodec=0;
    ppicture=0;
    outbuf=0;
    picture_buf=0;
    img_convert_ctx=0;
}


/**
   \brief Register the codecs
**/
bool QVideoEncoder::initCodec()
{
    av_register_all();
    avcodec_register_all();

    //printf("License: %s\n",avformat_license());
    //printf("AVCodec version %d\n",avformat_version());
    //printf("AVFormat configuration: %s\n",avformat_configuration());
    //qDebug()<<"Record Manager-"<<QString("License: %0\n").arg(avformat_license());
    //qDebug()<<"Record Manager-"<<QString("AVCodec version %0\n").arg(avformat_version());
    //qDebug()<<"Record Manager-"<<QString("AVFormat configuration: %0\n").arg(avformat_configuration());
    return true;
}


/**
  Ensures sizes are some reasonable multiples
**/
bool QVideoEncoder::isSizeValid()
{
//    if(getWidth()%8)
//        return false;
//    if(getHeight()%8)
//        return false;
    return true;
}

unsigned QVideoEncoder::getWidth()
{
    return Width;
}
unsigned QVideoEncoder::getHeight()
{
    return Height;
}
bool QVideoEncoder::isOk()
{
    return ok;
}

/**
  Allocate memory for the compressed bitstream
**/
bool QVideoEncoder::initOutputBuf()
{
    outbuf_size = getWidth()*getHeight()*3;        // Some extremely generous memory allocation for the encoded frame.
    outbuf = new uint8_t[outbuf_size];
    if(outbuf==0)
        return false;
    return true;
}
/**
  Free memory for the compressed bitstream
**/
void QVideoEncoder::freeOutputBuf()
{
    if(outbuf)
    {
        delete[] outbuf;
        outbuf=0;
    }
}

bool QVideoEncoder::initFrame()
{
    ppicture = avcodec_alloc_frame();
    if(ppicture==0)
        return false;

    int size = avpicture_get_size(pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height);
    picture_buf = new uint8_t[size];
    if(picture_buf==0)
    {
        av_free(ppicture);
        ppicture=0;
        return false;
    }

    // Setup the planes
    avpicture_fill((AVPicture *)ppicture, picture_buf,pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height);

    return true;
}
void QVideoEncoder::freeFrame()
{
    if(picture_buf)
    {
        delete[] picture_buf;
        picture_buf=0;
    }
    if(ppicture)
    {
        av_free(ppicture);
        ppicture=0;
    }
}

/**
  \brief Convert the QImage to the internal YUV format

  Custom conversion - not very optimized.

**/

bool QVideoEncoder::convertImage(const QImage &img)
{
    // Check if the image matches the size
    if(img.width()!=getWidth() || img.height()!=getHeight())
    {
        //printf("Wrong image size!\n");
        qDebug()<<"Record Manager-"<<"Wrong image size!\n";
        return false;
    }
    if(img.format()!=QImage::Format_RGB32	&& img.format() != QImage::Format_ARGB32)
    {
       // printf("Wrong image format\n");
        qDebug()<<"Record Manager-"<<"Wrong image format\n";
        return false;
    }

    // RGB32 to YUV420

    int size = getWidth()*getHeight();
    // Y
    for(unsigned y=0;y<getHeight();y++)
    {

        unsigned char *s = (unsigned char*)img.scanLine(y);
        unsigned char *d = (unsigned char*)&picture_buf[y*getWidth()];
        //printf("Line %d. d: %p. picture_buf: %p\n",y,d,picture_buf);

        for(unsigned x=0;x<getWidth();x++)
        {
            unsigned int r=s[2];
            unsigned int g=s[1];
            unsigned int b=s[0];

            unsigned Y = (r*2104 + g*4130 + b*802 + 4096 + 131072) >> 13;
            if(Y>235) Y=235;

            *d = Y;

            d+=1;
            s+=4;
        }
    }

    // U,V
    for(unsigned y=0;y<getHeight();y+=2)
    {
        unsigned char *s = (unsigned char*)img.scanLine(y);
        unsigned int ss = img.bytesPerLine();
        unsigned char *d = (unsigned char*)&picture_buf[size+y/2*getWidth()/2];

        //printf("Line %d. d: %p. picture_buf: %p\n",y,d,picture_buf);

        for(unsigned x=0;x<getWidth();x+=2)
        {
            // Cr = 128 + 1/256 * ( 112.439 * R'd -  94.154 * G'd -  18.285 * B'd)
            // Cb = 128 + 1/256 * (- 37.945 * R'd -  74.494 * G'd + 112.439 * B'd)

            // Get the average RGB in a 2x2 block
            int r=(s[2] + s[6] + s[ss+2] + s[ss+6] + 2) >> 2;
            int g=(s[1] + s[5] + s[ss+1] + s[ss+5] + 2) >> 2;
            int b=(s[0] + s[4] + s[ss+0] + s[ss+4] + 2) >> 2;

            int Cb = (-1214*r - 2384*g + 3598*b + 4096 + 1048576)>>13;
            if(Cb<16)
                Cb=16;
            if(Cb>240)
                Cb=240;

            int Cr = (3598*r - 3013*g - 585*b + 4096 + 1048576)>>13;
            if(Cr<16)
                Cr=16;
            if(Cr>240)
                Cr=240;

            *d = Cb;
            *(d+size/4) = Cr;

            d+=1;
            s+=8;
        }
    }
    return true;    
}

/**
  \brief Convert the QImage to the internal YUV format

  SWS conversion

   Caution: the QImage is allocated by QT without guarantee about the alignment and bytes per lines.
   It *should* be okay as we make sure the image is a multiple of many bytes (8 or 16)...
   ... however it is not guaranteed that sws_scale won't at some point require more bytes per line.
   We keep the custom conversion for that case.

**/

bool QVideoEncoder::convertImage_sws(const QImage &img)
{
    // Check if the image matches the size
    if(img.width()!=getWidth() || img.height()!=getHeight())
    {
        //printf("Wrong image size!\n");
        qDebug()<<"Record Manager-"<<"Wrong image size!\n";
        return false;
    }
    if(img.format()!=QImage::Format_RGB32	&& img.format() != QImage::Format_ARGB32)
    {
        //printf("Wrong image format\n");
        qDebug()<<"Record Manager-"<<"Wrong image size!\n";
        return false;
    }

    img_convert_ctx = sws_getCachedContext(img_convert_ctx,getWidth(),getHeight(),PIX_FMT_BGRA,getWidth(),getHeight(),PIX_FMT_YUV420P,SWS_BICUBIC, NULL, NULL, NULL);
    //img_convert_ctx = sws_getCachedContext(img_convert_ctx,getWidth(),getHeight(),PIX_FMT_BGRA,getWidth(),getHeight(),PIX_FMT_YUV420P,SWS_FAST_BILINEAR, NULL, NULL, NULL);
    if (img_convert_ctx == NULL)
    {
        //printf("Cannot initialize the conversion context\n");
        qDebug()<<"Record Manager-"<<"Cannot initialize the conversion context\n";
        return false;
    }

    uint8_t *srcplanes[3];
    srcplanes[0]=(uint8_t*)img.bits();
    srcplanes[1]=0;
    srcplanes[2]=0;

    int srcstride[3];
    srcstride[0]=img.bytesPerLine();
    srcstride[1]=0;
    srcstride[2]=0;

    sws_scale(img_convert_ctx, srcplanes, srcstride,0, getHeight(), ppicture->data, ppicture->linesize);

    return true;
}

