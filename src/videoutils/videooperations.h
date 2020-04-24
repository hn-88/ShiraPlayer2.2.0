#ifndef VIDEOOPERATIONS_H
#define VIDEOOPERATIONS_H

#include <QObject>
#include <QtOpenGL>

#include "videoutils/VideoFile.h"

class VideoOperations : public QObject
{
private:
    bool glSupportsExtension(QString extname);
    QStringList glSupportedExtensions();

public:

    VideoOperations();
    bool StartVideo(char* mFileName);
    bool StopVideo();
    bool PauseVideo(bool m_IsPause);
//    void Seek(double pos);
//    void Seek2(double pos);
//    double get_Clock();
//    double get_extClock();
//    void set_Clock(double clock);

//    GLuint** getData();
//    void *getScreenData();
//    int getWidth();
//    int getHeight();
//
//    bool isValid();
//    std::string getProperties(char* mFileName);
//    double getCurrentMoviePos();

    int start_w;
    int start_h;

//    void RefreshVideo();
    void set_Volume(int value);

    ///
    VideoFile *is;


};

#endif // VIDEOOPERATIONS_H
