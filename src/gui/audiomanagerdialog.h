/*
 * ShiraPlayer(TM)
 * Copyright (C) 2015 Asaf Yurdakul
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

#ifndef AUDIOMANAGERDIALOG_H
#define AUDIOMANAGERDIALOG_H

#include <QWidget>
#include <QFutureWatcher>
#include <QFileInfoList>
#include <QSettings>
#include <QTimer>
//#include "videoutils/AudioFile.h"
#include "videoutils/audioclass.h"
class VideoState;

namespace Ui {
class audioManagerDialog;
}

class audioManagerDialog : public QWidget
{
    Q_OBJECT

public:
    enum PlayState
    {
        Stopped,
        Paused,
        Running
    };

    explicit audioManagerDialog(QWidget *parent = 0);
    ~audioManagerDialog();
    void languageChanged();

private:
    Ui::audioManagerDialog *ui;
    QString media_path;
    QString strfile;
    QSettings* conf;
    PlayState currentState;
    QTimer m_timer;
    AudioClass *audioSt;

    QFutureWatcher<QImage> *imagesFuldomeVideoShow_;
    QFileInfoList getFulldomeVideoListFiles(QString dirPath) const;
    void setFuldomeVideoItemInList(int index);
    void WaitFutureFulldomeList();

    void ShowFulldomeVideoIconFile(QString dirPath);
    void setMediaButtonDurum();

protected slots:
    void tracktime();

private slots:
    void on_btnmStart_clicked();
    void on_btnmStop_clicked();
    void on_btnmPause_clicked();

    void on_trackMedia_sliderReleased();
    void on_trackMedia_sliderPressed();
    void on_tbarSes_valueChanged(int val);



};

#endif // AUDIOMANAGERDIALOG_H
