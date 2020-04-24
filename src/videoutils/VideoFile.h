
/*
 *  ShiraPlayer(TM)
 *  Created on: Jul 10, 2009 Author: bh
 *
 *  This file is part of GLMixer.
 *
 *   GLMixer is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   GLMixer is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with GLMixer.  If not, see <http://www.gnu.org/licenses/>.
 *
 *   Copyright 2009, 2012 Bruno Herbelin
 *   Copyright (C) 2011 Asaf Yurdakul
 */

#ifndef VIDEOFILE_H_
#define VIDEOFILE_H_

extern "C" {
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
}

#include <QObject>
#include <QMutex>
#include <QWaitCondition>
#include <QThread>
#include <QTimer>
#include <QImage>

/**
 * Dimension of the queue of VideoPictures in a VideoFile
 */
#define VIDEO_PICTURE_QUEUE_SIZE 4
/**
 * Portion of a movie to jump by (seek) when calling seekForward() or seekBackward() on a VideoFile.
 * (e.g. (0.05 * duration of the movie) = a jump by 5% of the movie)
 */
#define SEEK_STEP 0.1
/**
 * During decoding, the thread sleep for a little while in case there is an error or nothing to do.
 * 100 miliseconds is the ffplay default. The lower the more busy the CPU will be.
 */
#define SLEEP_DELAY 100

/**
 * Frames of a VideoFile are decoded and converted to VideoPictures.
 * A VideoPicture is accessed from the VideoFile::getPictureAtIndex() method.
 *
 * The VideoPicture RGB buffer can then be used for OpenGL texturing.
 * Here is an example code:
 *
 *    <pre>
 *    const VideoPicture *vp = is->getPictureAtIndex(newtexture);
 *    if (vp->isAllocated()) {
 *        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB,  vp->getWidth(),
 *                     vp->getHeight(), 0, GL_RGB, GL_UNSIGNED_BYTE,
 *                     vp->getBuffer() );
 *    }
 *    </pre>
 */
class VideoPicture {
    friend class VideoFile;
    AVPicture rgb, *oldframe;
    int width, height;
    bool allocated, usePalette;
    double pts;
    enum PixelFormat pixelformat;
    SwsContext *img_convert_ctx_filtering;
public:

    VideoPicture();
    ~VideoPicture();
    /**
     * Re-fills the Picture using the last filled AVFrame * given;
     * this also re-applies the filter.
     */
    void refilter() const;
    /**
     * Allocate the frame (w x h pixels) and resets to black.
     *
     * The pixel format is usually PIX_FMT_RGB24 or PIX_FMT_RGBA for OpenGL texturing, but depends
     * on the format requested when instanciating the VideoFile.
     *
     * @param w Width of the frame
     * @param h Height of the frame
     * @param format Internal pixel format of the buffer. PIX_FMT_RGB24 (by default), PIX_FMT_RGBA if there is alpha channel
     * @return true on success.
     */
    bool allocate(SwsContext *img_convert_ctx, int w, int h, enum PixelFormat format = PIX_FMT_RGB24, bool palettized = false);
    /**
     * Fills the rgb buffer of this Video Picture with the content of the ffmpeg AVFrame given.
     * If pFrame is not given, it fills the Picture with the formerly given one.
     *
     * This is done with the ffmpeg software conversion method sws_scale using the conversion context provided
     * during allocation (SwsContext *).
     *
     * If the video picture uses a color palette (allocated with palettized = true), then the
     * copy of pixels is done accordingly (slower).
     *
     * Finally, the timestamp given is kept into the Video Picture for later use.
     */
    void fill(AVPicture *pFrame, double timestamp = 0.0);

    /**
     * Get a pointer to the buffer containing the frame.
     *
     * If the buffer is allocated in the PIX_FMT_RGB24 pixel format, it is
     * a Width x Height x 3 array of unsigned bytes (char or uint8_t) .
     * This buffer is directly applicable as a GL_RGB OpenGL texture.
     *
     * If the buffer is allocated in the PIX_FMT_RGBA pixel format, it is
     * a Width x Height x 4 array of unsigned bytes (char or uint8_t) .
     * This buffer is directly applicable as a GL_RGBA OpenGL texture.
     *
     * @return pointer to an array of unsigned bytes (char or uint8_t)
     */
    inline char *getBuffer() const {
        return (allocated ? (char*) rgb.data[0] : NULL);
    }
    /**
     * Get the width of the picture.
     *
     * @return Width of the pixture in pixels.
     */
    inline int getWidth() const {
        return width;
    }
    /**
     * Get the height of the picture.
     *
     * @return Height of the pixture in pixels.
     */
    inline int getHeight() const {
        return height;
    }
    /**
     * Get aspect ratio width / height
     *
     * @return aspect ratio as a double
     */
    inline double getAspectRatio() const {
        return (double)width / (double)height;
    }
    /**
     * Tells if the picture was allocated and contains a frame in the buffer.
     *
     * @return true if there is a picture available.
     */
    inline bool isAllocated() const {
        return (allocated && (rgb.data != NULL));
    }
    /**
     * Creates and saves a .ppm image file with the current buffer (if full).
     */
    void saveToPPM(QString filename = "videoPicture.ppm") const;
    /**
     * Internal pixel format of the buffer.
     *
     * This is usually PIX_FMT_RGB24 or PIX_FMT_RGBA for OpenGL texturing, but depends
     * on the pixel format requested when instanciating the VideoFile.
     *
     * Pixel format should be tested before applying texture, e.g.:
     *    <pre>
     *     if ( vp->getFormat() == PIX_FMT_RGBA)
     *           glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,  vp->getWidth(),
     *                      vp->getHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE,
     *                      vp->getBuffer() );
     *    </pre>
     *
     * @return PIX_FMT_RGB24 by default, PIX_FMT_RGBA if there is alpha channel
     */

    AVPicture get_rgb()const { return rgb; }

    enum PixelFormat getFormat() const {
         return pixelformat;
    }

    bool useYUVData;
    bool skipNextFrame;
};

class ParsingThread;
class DecodingThread;

/**
 *  A VideoFile holds the ffmpeg video decoding and conversion processes required to read a
 *  movie and obtain each frame for further display. No sound information is decoded. This
 *  class is designed to be used for video mixing, not a regular video player.
 *
 *  Once instanciated, a VideoFile is suppoesed to read a movie by calling open().
 *  The file can be a video (e.g. avi, mov) or a picture (e.g. jpg, png). All ffmpeg formats
 *  and Codecs are accepted (call displayFormatsCodecsInformation() to get a list).
 *
 *  If a picture file was opened, a frameReady() QT event is sent with the argument (-1) to inform that the picture
 *  was loaded; the -1 index is a special case for the pre-loaded frame of a video (before
 *  reading the full movie), and is also used for pictures files (jpg, png, tif, etc.).
 *  So, to display the loaded picture, call getPictureAtIndex(-1) and display the given VideoPicture.
 *
 *  If a movie file was opened,  a frameReady() QT event is sent with the argument (-1) to
 *  inform that a preview frame was pre-loaded and the VideoFile is ready to start (call start() or play( true )).
 *  When started, a VideoFile will create two internal threads, one for decoding the movie
 *  and one for converting the frames into VideoPicture.
 *  The internal parrallel process of decoding & conversion will emit a frameReady()
 *  QT event each time a VideoPicture is produced. To display those pictures, connect
 *  this signal to a slot taking the integer index as argument for getPictureAtIndex(int).
 *  The picture index are in the range [0..VIDEO_PICTURE_QUEUE_SIZE]. See VideoPicture class for more details.
 *
 *  When playing, the VideoFile can seek for another time in the movie (no effect if movie is stopped).
 *  You can jump to a given position in frame (seekToPosition()), or jump relatively to the
 *  current play position (seekBySeconds(), seekByFrames(), seekForward(), etc.). However,
 *  be careful because calling one of those methods will only *request* a jump for *next* frame;
 *  all is fine if the video is playing, but this means that if the VideoFile was paused nothing
 *  will happen immediately; you have to unpause the video to actually jump and see the frame.
 *  This seems annoying but it is easy to program with qt events (just connect to the frameReady()
 *  event and pause again when recieving the event) and it guarantees the continuity of
 *  the decoding process after seeking.
 *
 *  To interrupt the video, you can either pause it (call pause()) or stop it (call stop() or
 *  play( false )). When paused, the processes are still running, waiting to restart exactly at
 *  the frame it was interrupted. When stopped, all activity is ended and the VideoFile will
 *  have to be started again. Note that by default, the behavior is to restart where the movie
 *  was stopped but that this is NOT equivalent to pause (see setOptionRestartToMarkIn(bool) ).
 *
 *  In addition, a VideoFile can use two marks, IN and OUT, defining the beginning and the end
 *  of the exerpt to play. Marks can be given when opening the file, but they can be defined
 *  and modified at any time (see setMarkIn(), setMarkOut()). This is particularly useful to
 *  repetitively show an excerpt of a movie when combined with the loop mode (setLoop()).
 *
 *  This code was inpired by the C/SDL tutorial by Martin Bohme (http://www.dranger.com/ffmpeg/)
 *  and by the C/SDL code of ffplay provided with ffmpeg (http://ffmpeg.org/)
 */
class VideoFile: public QObject {
Q_OBJECT

    friend class ParsingThread;
    friend class DecodingThread;

public:

    /**
     * Set of video playback speeds available.
     * See setPlaySpeed(int) for details.
     */
    typedef enum {
        SPEED_QUARTER = 0, SPEED_THIRD, SPEED_HALF, SPEED_NORMAL, SPEED_DOUBLE, SPEED_TRIPLE
    } PlaySpeed;
    /**
     *  Constructor of a VideoFile.
     *
     *  By default, a VideoFile should be instanciated with only its QObject parent.
     *
     *  Set the generatePowerOfTwo parameter to true in order to approximate the original dimensions of the
     *  movie to the closest power of two values.
     *  (this could be needed for old graphic cards which do not support the GL_EXT_texture_non_power_of_two extension).
     *
     *  The swsConversionQuality parameter is to decide for the scaling algorithm.
     *  In order of speed (inverse order of quality), swsConversionQuality can be :
     *  SWS_POINT, SWS_FAST_BILINEAR, SWS_BILINEAR, SWS_BICUBLIN, SWS_BICUBIC, SWS_SPLINE or SWS_SINC
     *  If no scaling is needed (generatePowerOfTwo false and no custom dimensions), use SWS_POINT for best performance.
     *  See libswscale/swscale.h for details.
     *
     *  The width and height parameters should be used only if your application requires frames of special dimensions.
     *  (If the generatePowerOfTwo parameter is true, these dimensions will be approximated to the closest power of two values).
     *
     *  PERFORMANCE REMARK; as the size conversion is done by ffmpeg in software (lib swscale) and is rather slow,
     *  use the generatePowerOfTwo or the custom dimensions only if needed.
     *
     *  @param parent QObject parent class.
     *  @param generatePowerOfTwo True to request a resize of the frames to the closest power of two dimensions.
     *  @param swsConversionQuality SWS_POINT, SWS_FAST_BILINEAR, SWS_BILINEAR, SWS_BICUBLIN, SWS_BICUBIC, SWS_SPLINE or SWS_SINC
     *  @param destinationWidth Width of the VideoPicture to produce; leave at 0 for auto detection from the file resolution.
     *  @param destinationHeight Height of the VideoPicture to produce; leave at 0 for auto detection from the file resolution.
     */
    VideoFile(QObject *parent = 0,  bool generatePowerOfTwo = false,
                int swsConversionQuality = 0, int destinationWidth = 0, int destinationHeight = 0);
    /**
     * Destructor.
     *
     * Automatically calls stop() and waits for the threads to end before clearing memory.
     */
    ~VideoFile();
    /**
     * Get a VideoPicture from the internal queue of pictures.
     *
     * Calling getPictureAtIndex with a valid index (typically given by a frameReady(int) event) returns the current picture
     * of the movie to display. The index of the pictures which can be obtained with this function are in the range [0 .. VIDEO_PICTURE_QUEUE_SIZE ].
     *
     * Calling  getPictureAtIndex with an invalid index (e.g. -1) returns the special VideoPicture previously pre-loaded.
     * This special VideoPicture can be:
     *
     * -  the only frame of a picture VideoFile or the first frame of the movie (just after loading the file).
     *
     * -  the frame at the Mark IN  after having stopped the video if setOptionRestartToMarkIn() is active.
     *
     * -  a back frame if the setOptionRevertToBlackWhenStop() is active.
     *
     *
     * It returns a const pointer to make sure you don't mess with it! You can only use the public methods of VideoPicture to read the picture.
     *
     * @param index Index of the picture to read; this should be used when recieving a frameReady(int) event and using the given index.
     * @return Const pointer to a VideoPicture; you cannot and should not modify the content of the VideoPicture. Just use it to read the buffer.
     */
    const VideoPicture *getPictureAtIndex(int index) const;
    /**
     * Test if a file was open for this VideoFile.
     *
     * @return true if a file was open, false otherwise.
     */
    bool isOpen() const {
        return (pFormatCtx != NULL);
    }
    /**
     * Test if running (playing).
     *
     * @return true if running (start() or play(true) was called).
     */
    inline bool isRunning() const {
        return !quit;
    }
    /**
     * Test if paused.
     *
     * @return true if paused.
     */
    inline bool isPaused() const {
        return pause_video;
    }
    /**
     * Test if in loop mode.
     *
     * When loop mode is active, the playback will restart at MarkIn when arriving at MarkOut.
     *
     * @return true if in loop mode.
     */
    inline bool isLoop() const {
        return loop_video;
    }
    /**
     *  Get the name of the file opened in this VideoFile.
     *
     *  @return const pointer to a char array containing the string file name. String is empty if file was not opened.
     */
    inline QString getFileName() const {
        return filename;
    }
    /**
     *  Get the name of the codec of the file opened.
     *
     *  @return FFmpeg name of the codec (e.g. mpeg4, h264, etc.). String is empty if file was not opened.
     */
    inline QString getCodecName() const {
        return codecname;
    }
    /**
     *  Get the name of the pixel format of the frames of the file opened.
     *
     *  @return FFmpeg name of the pixel format. String is empty if file was not opened.
     */
    QString getPixelFormatName(PixelFormat ffmpegPixelFormat = PIX_FMT_NONE) const ;
    /**
     *  Get if the frames are converted to power-of-two dimensions.
     *
     *  @return true if the video file was created with the 'power of two' option.
     */
    inline bool getPowerOfTwoConversion() const {
        return powerOfTwo;
    }
    /**
     *  Get the width of the produced frames.
     *
     *  Dimensions of frames were either specified when creating the VideoFile, either read from the file when opened.
     *
     *  @return Width of frames in pixels.
     */
    inline int getFrameWidth() const {
        return targetWidth;
    }
    /**
     *  Get the height of the produced frames.
     *
     *  Dimensions of frames were either specified when creating the VideoFile, either read from the file when opened.
     *
     *  @return Height of frames in pixels.
     */
    inline int getFrameHeight() const {
        return targetHeight;
    }
    /**
     *  Get the width of the frames contained in the stream.
     *
     *  @return Width of frames in pixels.
     */
    inline int getStreamFrameWidth() const {
        if (video_st)
            return video_st->codec->width;
        else
            return targetWidth;
    }
    /**
     *  Get the height of the frames contained in the stream.
     *
     *  @return Height of frames in pixels.
     */
    inline int getStreamFrameHeight() const {
        if (video_st)
            return video_st->codec->height;
        else
            return targetWidth;
    }
    /**
     * Get the aspect ratio of the video stream.
     *
     * @return Aspect ratio of the video.
     */
    double getStreamAspectRatio() const;
    /**
     * Get the frame rate of the movie file opened.
     *
     * NB: the frame rate is computed from avcodec information, which is not always correct as
     * some codecs have non-constant frame rates.
     *
     * @return frame rate in Hertz (frames per second), 0 if no video is opened.
     */
    double getFrameRate() const;
    /**
     * Get the duration of the VideoFile in seconds.
     *
     * @return Duration of the stream in seconds.
     */
    double getDuration() const;
    /**
     * Get the time of the first picture in VideoFile.
     *
     * @return Time stamp of the first frame of the stream, in stream time base (usually frame number).
     */
    int64_t getBegin() const;
    /**
     * Get the time of the last picture in VideoFile.
     *
     * @return Time stamp of the last frame, in stream time base (usually frame number).
     */
    int64_t getEnd() const;
    /**
     * Get the time of the current frame.
     *
     * @return frame Time stamp of the current frame, in stream time base (usually frame number).
     */
    int64_t getCurrentFrameTime() const;
    /**
     * Get the time when the IN mark was set
     *
     * @return Mark IN time, in stream time base (usually frame number).
     */
    inline int64_t getMarkIn() const {
        return mark_in;
    }
    /**
     * Set the IN mark time ; this is the time in the video where the playback will restart or loop.
     *
     * @param t Mark IN time, in stream time base (usually frame number).
     */
    void setMarkIn(int64_t t);
    /**
     * Get the time when the OUT mark was set
     *
     * @return Mark OUT time, in stream time base (usually frame number).
     */
    inline int64_t getMarkOut() const {
        return mark_out;
    }
    /**
     * Set the OUT mark time ; this is the time in the video at which the playback will end (pause) or loop.
     *
     * @param t Mark OUT time, in stream time base (usually frame number).
     */
    void setMarkOut(int64_t t);
    /**
     * Requests a seek (jump) into the video to the time t.
     *
     * Does nothing if the process is not running (started) or already seeking.
     *
     * @param t Time where to jump to, in stream time base (usually frame number). t shall be > 0 and < getEnd().
     */
    void seekToPosition(int64_t t);
    /**
     * Requests a seek (jump) into the video by a given amount of seconds.
     *
     * Does nothing if the process is not running (started) or already seeking.
     *
     * @param ss Amount of time to jump; if ss > 0 it seeks forward. if ss < 0, it seeks backward.
     */
    void seekBySeconds(double ss);
    /**
     * Requests a seek (jump) into the video by a given amount of frames.
     *
     * Does nothing if the process is not running (started) or already seeking.
     *
     * @param ss Amount of frames to jump; if ss > 0 it seeks forward. if ss < 0, it seeks backward.
     */
    void seekByFrames(int64_t ss);
    /**
     * Gives a string for human readable display of time (hh:mm:ss.ms) from a stream time-base value (frames).
     *
     * @param t a time in stream time base (usually frame number)
     * @return a string in 'hh:mm:ss.ms' format
     */
    QString getTimeFromFrame(int64_t t) const;
    /**
     * Gives a string for human readable display of frame (e.g. "frame: 125") from a stream time-base value (frames).
     *
     * @param t a time in stream time base (usually frame number)
     * @return a string in 'frame: f' format
     */
    QString getExactFrameFromFrame(int64_t t) const;
    /**
     * Gives a value in stream time-base (frames) from a given string in a human readable time format (hh:mm:ss.ms).
     *
     * @return a time in stream time base (usually frame number) if the parameter was ok, -1 otherwise.
     * @param t a string in 'hh:mm:ss.ms' format
     */
    int64_t getFrameFromTime(QString t) const;
    /**
     * Displays a dialog window (QDialog) listing the formats and video codecs supported for reading.
     *
     * @param iconfile Name of the file to put as icon of the window
     */
    static void displayFormatsCodecsInformation(QString iconfile);
    /**
     * Returns the strength of the brightness filter applied on the video
     * @return value between [-100, 100]
     */
    int getBrightness();
    /**
     * Returns the strength of the contrast filter applied on the video
     * @return value between [-100, 100]
     */
    int getContrast();
    /**
     * Returns the strength of the saturation filter applied on the video
     * @return value between [-100, 100]
     */
    int getSaturation();

    //ASAF
    /**
     * Gives number of sn from a stream time-base value (frames).
     *
     * @param t a time in stream time base (usually frame number)
     * @return a int
     */
    int getTimeSnFromFrame(int64_t t) const;
    int64_t getSeekPos() const { return video_st->nb_frames; }

    bool useYUVData;
    bool skipNextFrame;
    void fillFirstFrame();

    //
Q_SIGNALS:
    /**
     * Signal emmited when a new VideoPicture is ready;
     *
     * @param id the argument is the id of the VideoPicture to read.
     */
    void frameReady(int id);
    /**
     * Signal emmited when started or stopped;
     *
     * @param run the argument is true for running, and false for not running.
     */
    void running(bool run);
    /**
     * Signal emmited when video is paused;
     *
     * @param p the argument is true for paused, and false for playing.
     */
    void paused(bool p);
    /**
     * Signal emited when a mark (IN or OUT) has been moved.
     */
    void markingChanged();
    /**
     * Signal emmited when the pre-filtering has changed (brightness, contrast or saturation)
     *
     */
    void prefilteringChanged();

    //ASAF
    void endofVideo();


public Q_SLOTS:
    /**
     * Opens the file and reads first frame.
     *
     * Emits a frameReady signal to display this first frame (this is useful for 1 frame files like jpg or png).
     *
     * @param file Full path of the file to open
     * @param markIn Position of the mark IN where to start.
     * @param markOut Position of the mark OUT where to stop.
     * @return true on success
     */
    bool open(QString file, int64_t markIn = 0, int64_t markOut = 0, bool ignoreAlphaChannel = false, PixelFormat mytargetPixelFormat= PIX_FMT_NONE );
    /**
     *
     */
    void close();
    /**
     * Starts the decoding-conversion process.
     * Does nothing if the process was already started.
     *
     * Before starting, the processes are reseted to play the movie from the beginning (a paused(false) signal may be emited).
     *
     * Emits a frameReady signal at every frame.
     *
     * Emits an error message if no file was openned.
     *
     * Emits running(true) on success.
     */
    void start();
    /**
     * Stops the decoding-conversion process.
     * Does nothing if the process was not started.
     *
     * Once stopped, the process is reseted in order to restart from beginning next time.
     * (NB: call pause(true) if you want to temporarily interrupt the playback).
     *
     * Emits a frameReady signal to display the first frame.
     *
     * Emits an error message if no file was openned.
     *
     * Emits running(false) on success.
     */
    void stop();
    /**
     * Starts or stop the reading of the video.
     * Executes start() if argument it true, stop() otherwise.
     *
     * Emits the signals accordingly (see start() and stop()).
     *
     * @param startorstop whereas it should start of stop.
     */
    void play(bool startorstop);
    /**
     * Pauses / resumes the reading of the video.
     * Does nothing if the process is not running (started) or already paused.
     *
     * When paused, the processes are still running but just not going to the next frame.
     * (NB: Call stop() if you want to interrupt the playback more permanently).
     *
     * Emits a pause(bool) signal.
     *
     * @param pause If argument is true, pauses the reading of the video, if it is false, resumes play.
     */
    void pause(bool pause);
    /**
     *  Set the IN mark to the movie first frame.
     */
    inline void resetMarkIn() {
        setMarkIn(getBegin());
    }
    /**
     * Set the OUT mark to the movie last frame.
     */
    inline void resetMarkOut() {
        setMarkOut(getEnd());
    }
    /**
     * Set the IN mark to the current frame.
     */
    inline void setMarkIn() {
        setMarkIn(getCurrentFrameTime());
    }
    /**
     * Set the OUT mark to the current frame.
     */
    inline void setMarkOut() {
        setMarkOut(getCurrentFrameTime());
    }
    /**
     * Sets the reading to loop at the end or not.
     * When loop mode is active, the playback will restart at MarkIn when arriving at MarkOut.
     * Else, playing will pause after the last frame.
     *
     * @param loop activate the loop mode if true,
     */
    void setLoop(bool loop) {
        loop_video = loop;
    }
    /**
     * Sets the playing speed factor from the list of available playback speeds available in the PlaySpeed type.
     * NB: the argument is an integer for Qt compatibility with signals/slots, but it is used as a type PlaySpeed.
     *
     * This option alters the time stamp of the next frame decoded; this is not exactly the next frame to display as there is
     * a small buffer queue between the decoding and display threads, but the effect is almost instantaneous (few frames later).
     *
     * If speed is set to less than 1 (SPEED_QUARTER, SPEED_THIRD, SPEED_HALF), the presentation time stamp of the frame
     * is delayed and the frames will be displayed later than normal. If speed is set to more than 1 (SPEED_DOUBLE, SPEED_TRIPLE) the
     * time stamp is reduced and the frame will be displayed earlier than normal.
     *
     * However, this implementation of the playing speed is still not fully correct :(... The problem is that the delay or
     * acceleration seems to accumulate after a period out of normal play speed. Although the playback speed always end
     * up to be the one requested, it sometimes takes little time to reach it...
     *
     * @param playspeed SPEED_QUARTER = 0, SPEED_THIRD = 1, SPEED_HALF = 2, SPEED_NORMAL = 3, SPEED_DOUBLE = 4, SPEED_TRIPLE = 5
     */
    void setPlaySpeed(int playspeed);
    /**
     * Gets the playing speed factor from the list of available playback speeds available in the PlaySpeed type.
     * NB: the argument is an integer for Qt compatibility with signals/slots, but it is used as a type PlaySpeed.
     *
     * @return SPEED_QUARTER = 0, SPEED_THIRD = 1, SPEED_HALF = 2, SPEED_NORMAL = 3, SPEED_DOUBLE = 4, SPEED_TRIPLE = 5
     */
    int getPlaySpeed();
    /**
     * Seek backward of SEEK_STEP percent of the movie duration.
     * Does nothing if the process is not running (started).
     */
    inline void seekBackward() {
        seekByFrames(-SEEK_STEP * getEnd());
    }
    /**
     * Seek forward of SEEK_STEP percent of the movie duration.
     * Does nothing if the process is not running (started).
     */
    inline void seekForward() {
        seekByFrames(SEEK_STEP * getEnd());
    }
    /**
     *  Seek backward to the movie first frame.
     * Does nothing if the process is not running (started).
     */
    inline void seekBegin() {
        seekToPosition(getBegin());
    }
    /**
     * Sets the "allow dirty seek" option.
     *
     * This option is a parameter of ffmpeg seek; when dirty seek is allowed, seek to position will allow
     * any frame to be accepted (AVSEEK_FLAG_ANY) and displayed. However, with files using frame difference
     * for compressed encoding, this means that only a partial information is available, and that the frame
     * is incomplete, aka dirty.
     *
     * This option tries to compensate for some cases where seeking into a file causes jumps to 'real' key frames
     * at times which are quite far from the target seekToPosition time (the default behavior is to find the closest
     * full frame to display). Use it carefully!
     *
     * @param dirty true to activate the option.
     */
    void setOptionAllowDirtySeek(bool dirty);
    /**
     * Gets the "allow dirty seek" option.
     *
     * @return true if the option is active.
     */
    inline bool getOptionAllowDirtySeek() {
        return seek_any;
    }
    /**
     * Sets the "restart to Mark IN" option.
     *
     * When this option is active, stopping a VideoFile will jump back to the IN mark, ready to restart from there.
     *
     * @param on true to activate the option.
     */
    inline void setOptionRestartToMarkIn(bool on) {
        restart_where_stopped = !on;
    }
    /**
     * Gets the "restart to Mark IN" option.
     *
     * @return true if the option is active.
     */
    inline bool getOptionRestartToMarkIn() {
        return !restart_where_stopped;
    }
    /**
     * Sets the "revert to black" option.
     *
     * When this option AND the "restart to Mark IN" option are active, stopping a VideoFile will show a black frame
     * instead of the begining frame.
     *
     * @param black true to activate the option.
     */
    void setOptionRevertToBlackWhenStop(bool black);
    /**
     * Gets the "revert to black" option.
     *
     * @return true if the option is active.
     */
    inline bool getOptionRevertToBlackWhenStop() {
        return (resetPicture == &blackPicture);
    }

    /**
     * Specify the strength of the brightness filter applied on the video
     * @param b value between [-100, 100], 0 for no effect (original colors)
     */
    void setBrightness(int b);
    /**
     * Specify the strength of the contrast filter applied on the video
     * @param c value between [-100, 100], 0 for no effect (original colors)
     */
    void setContrast(int c);
    /**
     * Specify the strength of the saturation filter applied on the video.
     * @param s value between [-100, 100], 0 for no effect (original colors)
     */
    void setSaturation(int s);
    /**
     *
     */
    bool pixelFormatHasAlphaChannel() const;
    inline bool ignoresAlphaChannel() const { return ignoreAlpha; }
    /**
     *
     */
    bool isInterlaced() const { return deinterlacing_buffer != 0; }

protected Q_SLOTS:
        /**
         * Slot called from an internal timer synchronized on the video time code.
         */
        void video_refresh_timer();
    /**
     * Slot called when a thread ends prematurely (error handling).
     */
    void thread_terminated();

protected:

    /**
     * Packets are queued during the decoding process and unqueued in the conversion process.
     * This class is just the abstract data type needed for that.
     */
    class PacketQueue {
        AVPacketList *first_pkt, *last_pkt;
        static AVPacket *flush_pkt;
        int nb_packets;
        int size;
        QMutex *mutex;
        QWaitCondition *cond;

    public:
        PacketQueue();
        ~PacketQueue();

        bool get(AVPacket *pkt, bool block);
        bool put(AVPacket *pkt);
        bool flush();
        bool isFlush(AVPacket *pkt) const;
        bool isFull() const;
        inline bool isEmpty() const { return size == 0; }
        inline int getSize() const { return size; }
    };

    // internal methods
    double get_clock();
    void reset();
    void fill_first_frame(bool);
    int stream_component_open(AVFormatContext *);
    double synchronize_video(AVFrame *src_frame, double pts);
    void queue_picture(AVFrame *pFrame, double pts);
    static int roundPowerOfTwo(int v);

    // Video and general information
    QString filename;
    QString codecname;

    // Video file, streams and packets
    AVFormatContext *pFormatCtx;
    AVStream *video_st;
    SwsContext *img_convert_ctx;
    SwsFilter *filter;
    int videoStream;
    PacketQueue videoq;
    bool ignoreAlpha;
    uint8_t *deinterlacing_buffer;
    AVPicture deinterlacing_picture;

    // seeking management
    bool seek_req, seek_backward, seek_any;
    int64_t seek_pos;
    int64_t mark_in;
    int64_t mark_out;
    int64_t mark_stop;

    // time management
    double frame_timer;
    double frame_last_pts;
    double frame_last_delay;
    double video_clock;
    double video_current_pts;
    int64_t video_current_pts_time;

    QTimer *ptimer;
    double play_speed;
    double min_frame_delay, max_frame_delay;

    // picture queue management
    VideoPicture pictq[VIDEO_PICTURE_QUEUE_SIZE];
    int pictq_size, pictq_rindex, pictq_windex;
    bool pictq_skip;
    QMutex *pictq_mutex;
    QWaitCondition *pictq_cond;
    bool powerOfTwo;
    int targetWidth, targetHeight;
    int conversionAlgorithm;
    VideoPicture firstPicture, blackPicture;
    AVFrame *firstFrame;
    VideoPicture *resetPicture;

    // Threads and execution manangement
    ParsingThread *parse_tid;
    DecodingThread *decod_tid;
    bool quit;
    bool loop_video;
    bool pause_video;
    bool pause_video_last;
    bool restart_where_stopped;

    // ffmpeg util
    static bool ffmpegregistered;

};


#endif /* VIDEOFILE_H_ */
