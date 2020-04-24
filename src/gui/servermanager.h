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

#ifndef _SERVERMANAGER_H_
#define _SERVERMANAGER_H_

#include <QObject>
//#include <QFtp>
#include <QtOpenGL/QtOpenGL>
#include "StelDialog.hpp"
#include <QtXml/QtXml>
#include <QFuture>

#include "videoutils/videoclass.h"
#include "presenter/qimagepropwidget.h"
#include "presenter/qmediabrowserwidget.h"
#include "presenter/qplayimagewidget.h"
#include "stellamanager.hpp"

#include "licenceutils/qlisansform.h"
#include "freehand/tupcolorpalette.h"
#include "freehand/tuppenpreviewcanvas.h"
#include "freehand/tupslider.h"

#include "flybyutil/flybymanager.h"
#include "catalogpages/messierwindow.h"
#include "catalogpages/constellationwindow.h"
#include "gui/audiomanagerdialog.h"

class Ui_servermanager;
class QListWidgetItem;

class servermanager : public StelDialog
{
    Q_OBJECT

public:
    enum PlayState
    {
        Stopped,
        Paused,
        Running
    };

    servermanager();
    ~servermanager();

    void languageChanged();
    //! Notify that the application style changed
    void styleChanged();

    QString getMediaPath(){return media_path;}
    QStringList listofextensions;

    QString initFromDOMElement(const QDomElement& element);

    Ui_servermanager *ui;
    stellamanager* stellaManager;
    FlybyManager* flybyManager;
    messierWindow* messierwindow;
    ConstellationWindow* constwindow;
    audioManagerDialog* audiomanager;
private:
    //! Initialize the dialog widgets and connect the signals/slots
    virtual void createDialogContent();
    virtual void prepareAsChild();

    QString sLandscape ;
    bool sLandscapeVisibleFlag;
    GLuint textID;

    QString textScript;
    QSettings* conf;
    StelCore* core;
    QString media_path;
    QString flat_media_path;
    QString strfile;

    int movie_width ;
    int movie_height ;
    int fps;
    QString codecname;
    double duration;
    int durationFrame;

    QTimer m_timer;

    PlayState currentState;

    //QFtp *ftp;

    void setMediaButtonDurum();

    void Gecikme(QString strdt);
    int tSaniye;

    bool checkTimeServer();
    bool checkFtpServer();

    bool is_audio;
    bool old_isVideoLandscape;

    QLisansForm frm;

    // Presenter Stack
    void setupPresenterWidgets();    
    QMediaBrowserWidget *mediaBrowserList;
    QPlayImageWidget *playImageWidget;
    QImagePropWidget *defaultPropWidget;

    //
    QSharedMemory videoSharedMem;
    void loadFromSharedMem();

    QString presentFileName;
    QPixmap openImage(const QString &path);

    QFutureWatcher<QImage> *imagesFisheyeShow_;
    QFileInfoList getImageFisheyeListFiles(QString dirPath) const;
    void WaitFuture();

    QFutureWatcher<QImage> *imagesFuldomeVideoShow_;
    QFileInfoList getFulldomeVideoListFiles(QString dirPath) const;
    void WaitFutureFulldomeList();

    void ShowImageIconFile(QString dirPath);
    void ShowFulldomeVideoIconFile(QString dirPath);

    QFileInfoList fileList_;

    QTimer* tetiklemeTimer;
    QTimer* showFisheyeFrameTimer;
    QTimer* startGraphTimer;
    QTimer* stopGraphTimer;
    QTimer* clearFramesTimer;

    TupColorPalette* colorpaletWidget;

    void setSSLabelPanel();
    void setSSBrushCanvas();
    void setSSSlider();
    QLabel *SSSLabel;
    TupPenPreviewCanvas *thickSSPreview;
    TupSlider *sliderSSS;
    QWidget *sliderSSSwidget;

    void setOPLabelPanel();
    void setOPBrushCanvas();
    void setOPSlider();
    QLabel *OPLabel;
    TupPenPreviewCanvas *thickOPPreview;
    TupSlider *sliderOP;
    QWidget *sliderOPwidget;

    QString filepath;
    bool bFlagLandscape;

    Vec3d oldViewDirection;
private slots:
    void btnRefreshList_clicked();

    void btnliveModeClicked();
    void btnFadeEffectClicked();
    void btnApplyClicked();
    void favoritselectedChange();
    void btnApplyFavoritsClicked();
    void btnStopFavoritClicked();

    void changePage(QListWidgetItem *current, QListWidgetItem *previous);
    void on_mediaselectedChange();
    void on_frameselectedChange();
    void on_bntmStart_clicked();
    void on_btnmStop_clicked();
    void on_btnmPause_clicked();
    void on_clearFrame_clicked();

    void on_btnLoadTerminal_clicked();

    void doValidTimeServer();

    void ftpCommandFinished(int i,bool error);

    void on_tbarSes_changed(int val);

    void on_listframes_DoubleClicked(QListWidgetItem* item);
    void on_chDaylight_toggled(bool checked);

    //Admin Controls
    void on_TermUpdate_clicked();
    void on_TermRefresh_clicked();
    void on_TermClose_clicked();
    void on_TermOpen_clicked();
    void on_projOn_clicked();
    void on_projOff_clicked();
    void on_projblank_clicked();
    void on_projdisplay_clicked();

    void btnLisansClick();

    void on_btnQuit_clicked();
    void on_btnNightView_clicked(bool checked);

    void on_btnPLoad_clicked();
    void on_btnPSave_clicked();
    void on_btnPNew_clicked();
    void on_textEditPresent_textChanged();

    void on_chShowFPS_stateChanged(int );
    void on_chUseShaders_stateChanged(int );
    void on_chSkipNextFrame_stateChanged(int);
    void on_chpreWarped_stateChanged(int );
    void on_chCrossFade_stateChanged(int);
    void on_spFadeDuration_valueChanged(int);

    void setFisheyeImageItemInList(int index);
    void setFuldomeVideoItemInList(int index);

    void on_trackMedia_sliderReleased();
    void on_trackMedia_sliderPressed();

    void tetiklemeTimerProc();
    
    void on_btnSetFreeHand_clicked();
    void on_btnDeleteFreeHand_clicked();
    void on_btnClearFreeHand_clicked();
    void on_btnSelectFreeHand_clicked();

    void modifyStrokeSize(int value);
    void modifyOpacitySize(int value);

    void showFisheyeFrameBYTimer();
    void startGraphBYTimer();
    void stopGraphBYTimer();
    void clearFramesBYTimer();

    void on_videocolors_valueChanged(int val);
    void on_btnVideoColorReset_clicked();

protected slots:
    void tracktime();

public slots:
    void scriptEnded();

    //! Presenter Slots
    void currentMediaItemChanged( QListWidgetItem * current, QListWidgetItem * previous);

    //! update the editing display with new JD.
    void setDateTimeGui(double newJd);

    //! Frehand set color
    void setCurrentColor(const QColor color);

public slots:
    void retranslate();
};

QPixmap prepareIcon(const QFileInfo &infoFile);
QPixmap prepareIconVideo(const QFileInfo &infoFile);
QImage prepareImage(const QFileInfo &infoFile);
QImage prepareVideoImage(const QFileInfo &infoFile);

#endif // _SERVERMANAGER_H
