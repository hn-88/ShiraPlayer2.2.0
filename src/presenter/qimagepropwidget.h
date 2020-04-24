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


#ifndef QIMAGEPROPWIDGET_H
#define QIMAGEPROPWIDGET_H

#include <QWidget>
#include <QListWidget>
#include <QtXml/QtXml>

class Ui_QImagePropWidget;

#include "StelDialog.hpp"
#include "StelPresentMgr.hpp"
//todo
#include "presenter/VideoSource.h"
#include "videoutils/videoclass.h"

class QImagePropWidget : public QWidget
{
    Q_OBJECT

public:
    QImagePropWidget(QWidget *parent = 0,QString path ="",QString filename="",bool isVideo=false);
    ~QImagePropWidget();

    void setImage(QPixmap pix,QString caption);
    void setCaption(QString caption);
    void setCloseVisible(bool visible);
    //void setIsVideoProp(bool b);

    QDomElement domElement(const QString& name, QDomDocument& doc) const;
    void sendClinet();

    QWidget* getGroupBox();
    const QPixmap* getPixmap()const;
    QString getCaption() const;
    void languageChanged();
public slots:
    void RAsliderValueChange(int val);
    void DecsliderValueChange(int val);
    void RotsliderValueChange(int val);
    void SizesliderValueChange(int val);
    void domeORskyToggled(bool checked);
    void FadeToggled(bool checked);
    void btnCloseToggled(bool checked);
    void tabListItemChanged(QListWidgetItem* item,QListWidgetItem* previtem);
    void on_sliderSat_sliderMoved(int position);
    void on_sliderBrightness_sliderMoved(int position);
    void on_sliderContrast_sliderMoved(int);

    void sliderRelease();

    void StartAudio(bool checked);

private slots:
    void on_startButton_clicked(bool checked);
    void on_videoLoopButton_toggled(bool checked);
    void on_pauseButton_toggled(bool checked);

    void refreshTiming();
    void updateRefreshTimerState();
//    void on_frameSlider_sliderPressed();
//    void on_frameSlider_sliderReleased();
//    void on_frameSlider_sliderMoved (int v);

    void on_chMix_toggled(bool checked);

protected:
    StelPresentMgr* sPmgr;
    QString presentfilename;
    QString presentPath;
    int presentCon,presentBri,presentSat;
    float presentRA,presentDec,presentRotate,presentSize;
    bool presentdomeORsky, presentVisible;
    int presentID;
    double aspectratio;

    bool isVideo;
    //todo
    VideoSource* vsource;
    VideoClass* vfile;

    QTimer* refreshTimingTimer;

    QSharedMemory videoSharedMem;
    void loadFromSharedMem();

    bool loading;

    bool focused;
    QWidget* parentWidget;

private:
    Ui_QImagePropWidget* ui;

signals:
    void activationChangeFiltered();
};

#endif // QIMAGEPROPWIDGET_H
