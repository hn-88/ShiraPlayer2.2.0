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

#ifndef __QVideoEncoder_H
#define __QVideoEncoder_H


#include <stdint.h>

#include <QIODevice>
#include <QFile>
#include <QImage>

extern "C" {

#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavutil/mathematics.h"
#include "libavutil/opt.h"
#include "libavutil/rational.h"
#include "libavutil/avstring.h"
#include "libswscale/swscale.h"
}

class QVideoEncoder
{
   protected:
      unsigned Width,Height;
      unsigned Bitrate;
      unsigned Gop;
      bool ok;

      // FFmpeg stuff
      AVFormatContext *pFormatCtx;
      AVOutputFormat *pOutputFormat;
	  AVCodecContext *pCodecCtx;
      AVStream *pVideoStream;
      AVCodec *pCodec;
      // Frame data
      AVFrame *ppicture;
      uint8_t *picture_buf;
      // Compressed data
      int outbuf_size;
      uint8_t* outbuf;
      // Conversion
      SwsContext *img_convert_ctx;
      // Packet
      AVPacket pkt;

      QString fileName;

      unsigned getWidth();
      unsigned getHeight();
      bool isSizeValid();

      void initVars();
      bool initCodec();

      // Alloc/free the output buffer
      bool initOutputBuf();
      void freeOutputBuf();

      // Alloc/free a frame
      bool initFrame();
      void freeFrame();

      // Frame conversion
      bool convertImage(const QImage &img);
      bool convertImage_sws(const QImage &img);



   public:
      QVideoEncoder();
      virtual ~QVideoEncoder();

      bool createFile(QString filename,unsigned width,unsigned height,unsigned bitrate,unsigned gop,unsigned fps=25);
      virtual bool close();

      virtual int encodeImage(const QImage &);
      virtual bool isOk();  

};




#endif // QVideoEncoder_H
