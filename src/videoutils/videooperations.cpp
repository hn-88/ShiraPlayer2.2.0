//#include "ffplayMovie.h"
#include "videooperations.h"
#include "StelMainGraphicsView.hpp"

#include <QtOpenGL>

VideoOperations::VideoOperations()
{
}

bool VideoOperations::StartVideo(char* mFileName)
{
    //Start_Movie(mFileName);

    //is = new VideoFile(this, true, SWS_FAST_BILINEAR);
    is = new VideoFile(this);
    is->open(mFileName);
    is->setLoop(false);
//    is->start();

    return true;
}
bool VideoOperations::StopVideo()
{
    //Stop_Movie();
    is->stop();
    is->close();
    //is = NULL;
    //delete is;

    return true;
}

bool VideoOperations::PauseVideo(bool m_IsPause)
{
    //toggle_pause();
//    if(is)
//       is->pause(m_IsPause);

    return true;
}
//
//GLuint** VideoOperations::getData()
//{
////    if(getVideoState())
////    {
////        if( getVideoState()->width != 0)
////        {
////            if (get_show_refreshed())
////            {
////                video_refresh_timer(getVideoState());
////                set_show_refreshed(false);
////                return (GLuint **)get_currFrame()->data[0];
////            }
////        }
////    }
////    return NULL;
//
//}
//
//bool VideoOperations::isValid()
//{
////    if(getVideoState())
////        if( getVideoState()->width != 0)
////            return true;
////    else
////        return false;
//}
//int VideoOperations::getWidth()
//{
////    if(getVideoState()!=NULL)
////        return getVideoState()->width;
////    else
////        return 0;
//}
//
//int VideoOperations::getHeight()
//{
////    if(getVideoState()!=NULL)
////        return getVideoState()->height;
////    else
////        return 0;
//}
//
//std::string VideoOperations::getProperties(char* mFileName)
//{
////    return get_properties_str(mFileName);
//}
//
//double VideoOperations::getCurrentMoviePos()
//{
////    if(getVideoState()!=NULL)
////        return getVideoState()->video_clock;
////    else
////        return 0;
//}
//
//void VideoOperations::Seek(double pos)
//{
////    if(getVideoState()!=NULL)
////        return;
////    double posL = get_master_clock(getVideoState());
////    posL += pos;
////    stream_seek(getVideoState(), (int64_t)(posL * AV_TIME_BASE), (int64_t)(posL * AV_TIME_BASE), 0);
//}
//void VideoOperations::Seek2(double pos)
//{
////    if(getVideoState()!=NULL)
////        return;
////    stream_seek(getVideoState(), (int64_t)(pos * AV_TIME_BASE), (int64_t)(pos * AV_TIME_BASE), 0);
//}
//void VideoOperations::set_Clock(double clock)
//{
//    //set_VideoClock(getVideoState(),clock); ///IPTAL
//}
//
//double VideoOperations::get_Clock()
//{
////    if(getVideoState()!=NULL)
////        return get_master_clock(getVideoState());
////    else
////        return 0;
//}
//double VideoOperations::get_extClock()
//{
////    if(getVideoState()!=NULL)
////        return get_external_clock(getVideoState());
////    else
////        return 0;
//}
//void *VideoOperations::getScreenData()
//{
////    if(getSDLScreen()!=NULL)
////        return getSDLScreen()->pixels;
////    else
////        return NULL;
//}
//void VideoOperations::RefreshVideo()
//{
////    if(getVideoState()!=NULL)
////        video_refresh_timer(getVideoState());
//}
void VideoOperations::set_Volume(int value)
{
    //set_volume(value);
}
