#ifndef VIDEOCLASS_H
#define VIDEOCLASS_H

#include <QObject>
#include <QElapsedTimer>
#include <QBuffer>
#include <QThread>
#include <QTimer>
#include <QSize>

#include "worker.h"
#include "audioclass.h"

class VideoClass : public QObject
{
    Q_OBJECT
public:
    explicit VideoClass(QObject *parent = 0);
    ~VideoClass();

    void OpenVideo(const QString &filename);
    void StartVideo();
    void StopVideo();
    void PauseVideo(bool pause, bool withaudio);
    void CloseVideo();
    void StopPullTimer() { timerPull->stop(); worker->qCbuffer.clear();}
    QSize GetVideoSize(){ return QSize(VW,VH);}
    double GetAspectRatio() { return (double)VW/(double)VH;}
    double GetVideoDuration() { return worker->getduration();}
    int64_t GetVideoEnd() { return worker->getvideoend();}
    QString GetFileName() { return _filename;}
    int GetCurrentFrameNumber(){ return currentFrame;}
    QString GetTimeStrFromFrame(int frmNumber);
    int GetVideFrameRate(){ return worker->getVideoRate();}
    void SeekByFrame(int frameNumber);

    QByteArray GetFirstFrame(){ return worker->PullFirstFrame();}
    bool IsPaused() { return worker->isPaused();}
    bool IsFinished() { return isfinished;}

    bool IsInternalTimer() { return internaltimer;}

    int GetFreeBufferCount() { return freeBufferCount;}
    void setFlatVideo(bool val){ flatVideo = val;}
    bool HasEmbedAudio();
private:
    QThread *thread;
    Worker *worker;
    int VW,VH;
    QTimer* timerPull;
    bool isfinished ;
    int currentFrame;
    QString _filename;

    int seekframeplus;
    bool seeking;
    int seekframe;

    QElapsedTimer tsync;
    qint64 lastElapsed;

    int prevAudioFrameNo;
    bool isEqualAudioVideo;
    bool internaltimer;

    AudioClass* audio;
    QString afilename;

    int freeBufferCount;
    bool flatVideo;
private slots:
    void on_pulltimer_setstatus(bool status);
    void on_currentFrame_changed(const QString &value);
    void on_value_bufferchanged(int value);
    void on_finished();
    void on_timerPull_timeout();
    void on_audio_trigger(double i);
protected slots:
    void toggle_audio();
signals:
    void currentFrameChanged(const QString &value);
    void valueBufferChanged(int value);
    void finished();
    void preparedBuffer(QByteArray frame);


};

#endif // VIDEOCLASS_H
