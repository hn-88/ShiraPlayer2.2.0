#include "videoclass.h"
#include <QDebug>
#include <QFile>

#include "StelMainGraphicsView.hpp"
#include "StelApp.hpp"
#include "StelGui.hpp"
#include "StelMainWindow.hpp"

VideoClass::VideoClass(QObject *parent) :
    QObject(parent), isfinished(false)
{    
    audio = &AudioClass::getInstance();

    currentFrame = 0;
    seeking = false;
    lastElapsed = 0;
    prevAudioFrameNo = -999;
    isEqualAudioVideo = false;
    internaltimer = false;
    flatVideo = false;

    timerPull = new QTimer();
    timerPull->setTimerType(Qt::PreciseTimer);
    timerPull->setInterval( 1 );

    thread = new QThread();
    worker = new Worker();
    worker->moveToThread(thread);

    connect(worker, SIGNAL(workRequested()), thread, SLOT(start()));
    connect(thread, SIGNAL(started()), worker, SLOT(doWork()));
    connect(worker, SIGNAL(finished()), thread, SLOT(quit()), Qt::DirectConnection);
    connect(worker, SIGNAL(valueChanged(QString)), this,SLOT(on_currentFrame_changed(QString)));
    connect(worker, SIGNAL(valueBufferChanged(int)),this,SLOT(on_value_bufferchanged(int)));
    connect(worker, SIGNAL(finished()), this,SLOT(on_finished()));

    //Pull Frame timer
    connect(timerPull,SIGNAL(timeout()),this,SLOT(on_timerPull_timeout()));
    connect(worker,SIGNAL(pulltimerSetStatus(bool)),this,SLOT(on_pulltimer_setstatus(bool)));
    //
    //isfinished = true;
}

VideoClass::~VideoClass()
{
    worker->abort();
    thread->wait();
    qDebug()<<"Deleting thread and worker in Thread "<<this->QObject::thread()->currentThreadId();
    delete thread;
    delete worker;
}

void VideoClass::OpenVideo(const QString &filename)
{
    afilename ="";
    _filename = filename;
    worker->abort();
    //if (!thread->isRunning())
    thread->wait(); // If the thread is not running, this will immediately return.
    worker->openvideo(filename, VW, VH);
    currentFrame = 0;
    isfinished = false;
    prevAudioFrameNo = -999;
    isEqualAudioVideo = false;
}

void VideoClass::StartVideo()
{
    if (worker->isOpened())
    {
        timerPull->setInterval(1000.0 / worker->getVideoRate());
        worker->requestWork();
        //thread->setPriority(QThread::TimeCriticalPriority);
        //AudioClass* audio = dynamic_cast<AudioClass*>(this->parent());
        if( !audio->IsOpened() )
        {
            try
            {

                QString vfilename = GetFileName();
                afilename ="";

                //Waw veya mp3 dosyası ayrı var ise o çalıştırılacak
                QStringList strL = vfilename.split('.');
                if (strL.count() == 2)
                {
                    afilename = QString("%0.wav").arg(strL[0]);
                    if (!QFile::exists(afilename))
                    {
                        afilename = QString("%0.mp3").arg(strL[0]);
                        if (!QFile::exists(afilename))
                            afilename = vfilename;
                    }
                }
                if(audio->Load_Audio(afilename)>0)
                {
                    //audio->Start_Audio();
                    //qDebug()<<"audio started:"<<timer->elapsed();
                }
                if(!flatVideo)
                {
                    StelMainGraphicsView::getInstance().startLoopTimer(false);
                    if(StelMainWindow::getInstance().getIsServer())
                    {
                        StelGui* sgui = dynamic_cast<StelGui*>(StelApp::getInstance().getGui());
                        sgui->startAutoSave(false);
                    }
                    StelApp::getInstance().isVideoMode = true;
                }
            }
            catch (std::runtime_error &e)
            {
                qWarning() << "VideoClass::StartVideo error: " << e.what();
            }
        }

    }
}

void VideoClass::StopVideo()
{
    // if(!internaltimer)
    StopPullTimer();
    worker->stopT();
    thread->wait();
    if(!flatVideo)
    {
        QThread::msleep(50);
        worker->closevideo();
    }
    audio->Stop_Audio();
    //if(!internaltimer)
    timerPull->start();
    isfinished = true;
}

void VideoClass::PauseVideo(bool pause,bool withaudio)
{
    if (pause)
        worker->pauseT();
    else
        worker->resumeT();
    if (withaudio) toggle_audio();
}

void VideoClass::CloseVideo()
{
    try
    {
        worker->thread()->quit();
        worker->stopT();
        //worker->closevideo();
        seekframeplus = 0;
    }
    catch (std::runtime_error& e)
    {

    }

}

QString VideoClass::GetTimeStrFromFrame(int frmNumber)
{
    double time = (double) frmNumber / (double) GetVideFrameRate();
    int s = (int) time;
    time -= s;
    int h = s / 3600;
    int m = (s % 3600) / 60;
    s = (s % 3600) % 60;
    int ds = (int) (time * 100.0);
    return QString("%1h %2m %3.%4s").arg(h, 2).arg(m, 2, 10, QChar('0')).arg(s, 2, 10, QChar('0')).arg(ds, 2, 10, QChar('0'));
}

void VideoClass::SeekByFrame(int frameNumber)
{
    if(internaltimer)
    {
        double timepos = ( frameNumber / (double)worker->getVideoRate());
        seeking = true;
        worker->stream_seek( timepos ,frameNumber);
        seekframe = frameNumber;
        currentFrame = frameNumber;
    }
    else
    {
        tsync.start();
        double timepos = ( frameNumber / (double)GetVideFrameRate());
        seeking = true;
        worker->stream_seek( timepos ,frameNumber);
        seekframe = frameNumber;

    }

}

bool VideoClass::HasEmbedAudio()
{
    //if(internaltimer)
    //    return false;
    //qDebug()<<"video filename:"<<GetFileName();
    //qDebug()<<"audio filename:"<<afilename;
    if(worker->getAudioStreamIndex() == -1)
        return false;
    else
        return true;
}

void VideoClass::on_pulltimer_setstatus(bool status)
{
    if (status)
    {
        //AudioClass* audio = dynamic_cast<AudioClass*>(this->parent());

        if (seeking)
        {
            if (!internaltimer)
            {
                if(audio)
                {
                    if(audio->IsStarted())
                    {
                        //qDebug()<<"audio seek"<<currentFrame;
                        double sn = (seekframe / (double)GetVideFrameRate());
                        double addedSn = tsync.elapsed()/(double)1000.0;
                        audio->audioSeekToPositionSn(sn-addedSn);
                        currentFrame = seekframe-addedSn*GetVideFrameRate();
                    }
                }
            }
            seeking = false;
            if (internaltimer)
                timerPull->start();
        }
        else
        {
            if(audio)
            {
                if(audio->IsOpened())
                {
                    if (!audio->IsStarted())
                    {
                        internaltimer = false;
                        qDebug()<<"audio emitter started";
                        emit(preparedBuffer(0));
                        timerPull->stop();
                        connect(audio,SIGNAL(emitFunction(double)),this,SLOT(on_audio_trigger(double)));
                        audio->Start_Audio();
                    }
                }
                else
                {
                    if((!timerPull->isActive()) && (!isfinished))
                    {
                        internaltimer = true;
                        disconnect(audio,SIGNAL(emitFunction(double)),this,SLOT(on_audio_trigger(double)));
                        qDebug()<<"internal timer started";
                        timerPull->start();
                        timerPull->setTimerType(Qt::PreciseTimer);
                    }
                }

            }
        }
    }
    else
        timerPull->stop();
}

void VideoClass::on_currentFrame_changed(const QString &value)
{
    emit(currentFrameChanged(value));
}

void VideoClass::on_value_bufferchanged(int value)
{
    freeBufferCount = value;
    emit(valueBufferChanged(value));
}

void VideoClass::on_finished()
{
    /*StelMainGraphicsView::getInstance().startLoopTimer(true);
    StelApp::getInstance().isVideoMode = false;
    StelGui* sgui = dynamic_cast<StelGui*>(StelApp::getInstance().getGui());
    sgui->startAutoSave(true);*/
    qDebug()<<"video class ended";
    //isfinished = true;
    emit(finished());
}


void VideoClass::on_timerPull_timeout()
{
    if ((seeking) && (!worker->isPaused())) return;
    QElapsedTimer t; t.start();

    currentFrame++;
    QByteArray frame = GetFirstFrame();
    //if (IsPaused()) PauseVideo(false,false);
    worker->setWait(false);
    emit(currentFrameChanged(QString::number(currentFrame)));
    worker->LockWorker();
    emit(preparedBuffer(frame));
    //qDebug()<<"timer dan emit";
    worker->UnLockWorker();

    double newinterval = (1000.0 /GetVideFrameRate());
    newinterval = newinterval - t.nsecsElapsed()/ 1000000.0;
    if (newinterval < 1) newinterval = 1;
    timerPull->setInterval(newinterval);
    //qDebug()<<newinterval;

    if ((frame.isEmpty()) && isfinished)
        timerPull->stop();

}

void VideoClass::on_audio_trigger(double i)
{
    //qDebug()<<"audio triger:"<<i;
    if ((seeking) && (!worker->isWait())) return;
    int frameNo = (int)( i * GetVideFrameRate() );

    if( (  frameNo > 0 ) && ( currentFrame < frameNo )
            &&
        ( (!isEqualAudioVideo) || (frameNo != prevAudioFrameNo)) )
    {
        if( frameNo - prevAudioFrameNo <= 1)
        {
            QByteArray frame = GetFirstFrame();
            //if (IsPaused()) PauseVideo(false,false);
            worker->setWait(false);
            emit(currentFrameChanged(QString::number(currentFrame)));
            worker->LockWorker();
            emit(preparedBuffer(frame));
            worker->UnLockWorker();

            currentFrame++;
        }

        if (currentFrame == frameNo)
            isEqualAudioVideo = true;
        if ( frameNo - prevAudioFrameNo > 1)
            isEqualAudioVideo = false;
        //qDebug()<<"videoF:"<<currentFrame<<"audio F:"<<frameNo;
        prevAudioFrameNo = frameNo;
    }

}

void VideoClass::toggle_audio()
{
    if(audio->IsOpened())
        audio->toggle_pause();
}


