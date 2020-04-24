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

#include <QtGui>
#include <QDebug>
#include <QDialog>
#include <QGraphicsProxyWidget>
#include <QObject>
#include <QMessageBox>
#include <QtNetwork/QtNetwork>
#include <QtConcurrent/QtConcurrent>

#include "StelModuleMgr.hpp"
#include "LandscapeMgr.hpp"
#include "Landscape.hpp"
#include "StelTexture.hpp"
#include "StelMovementMgr.hpp"
#include "StelTranslator.hpp"
#include "servermanager.h"
#include "ui_servermanager.h"
#include "ui_viewDialog.h"
#include "ViewDialog.hpp"

#include "StelMainGraphicsView.hpp"
#include "StelScriptMgr.hpp"
#include "StelCore.hpp"
#include "StelApp.hpp"
#include "StelGui.hpp"
#include "StelMainWindow.hpp"
#include "networkcommands.h"
#include "socketutils/rsync.h"

#include "socketutils/sntpclient.h"
#include "videoutils/audioclass.h"

#include "StelPresentMgr.hpp"

#include "presenter/qimagepropwidget.h"
#include "presenter/qmediabrowserwidget.h"
#include "presenter/qplayimagewidget.h"

#include "gui/stellamanager.hpp"
#include "videoutils/embedaudiowarning.h"
#include "shiraplayerform.hpp"

#include "config.h"
#include "stdexcept"

//#include "../plugins/ShiraVideoManager/src/ShiraVideoManager.hpp"


using namespace std;

const int WIDTH_ICON = 100;
const int HEIGHT_ICON = 90;
const int HEIGHT_ICON2 = 65;
const int WIDTH_IMAGE = 100;
const int HEIGHT_IMAGE = 65;

QMutex fulldomeListmutex;
servermanager::servermanager()
{
    ui = new Ui_servermanager;
}

servermanager::~servermanager()
{
    delete ui;
}

void servermanager::createDialogContent()
{
    ui->setupUi(dialog);
    stellaManager = new stellamanager(ui->page_6);
    ui->page6Layout->addWidget(stellaManager);

    flybyManager = new FlybyManager(ui->page_8);
    ui->page8Layout->addWidget(flybyManager);

    messierwindow = new messierWindow(ui->page_9);
    ui->page9Layout->addWidget(messierwindow);

    constwindow = new ConstellationWindow(ui->page_10);
    ui->page10Layout->addWidget(constwindow);

    audiomanager = new audioManagerDialog(ui->page_11);
    ui->page11Layout->addWidget(audiomanager);

    connect(ui->closeStelWindow, SIGNAL(clicked()), this, SLOT(close()));
    connect(ui->btnLiveMode, SIGNAL(clicked()), this, SLOT(btnliveModeClicked()));
    connect(ui->btnFadeEffect, SIGNAL(clicked()),this,SLOT(btnFadeEffectClicked()));
    connect(ui->btnApply,SIGNAL(clicked()),this,SLOT(btnApplyClicked()));
    connect(ui->listWidget, SIGNAL(itemSelectionChanged()), this, SLOT(favoritselectedChange()));
    connect(ui->btnApplyFavorits,SIGNAL(clicked()),this,SLOT(btnApplyFavoritsClicked()));

    connect(ui->btnFrameLoadtoTerminals,SIGNAL(clicked()),this,SLOT(on_btnLoadTerminal_clicked()));
    connect(ui->listFilesWidget, SIGNAL(itemSelectionChanged()), this, SLOT(on_mediaselectedChange()));
    connect(ui->listFrameFileWidget, SIGNAL(itemSelectionChanged()), this, SLOT(on_frameselectedChange()));
    connect(ui->btnmStart,SIGNAL(clicked()),this,SLOT(on_bntmStart_clicked()));
    connect(ui->btnmStop,SIGNAL(clicked()),this,SLOT(on_btnmStop_clicked()));
    connect(ui->btnmPause,SIGNAL(clicked()),this,SLOT(on_btnmPause_clicked()));
    connect(ui->tbarSes,SIGNAL(valueChanged(int)),this,SLOT(on_tbarSes_changed(int)));
    connect(ui->stackListWidget, SIGNAL(currentItemChanged(QListWidgetItem *, QListWidgetItem *)), this, SLOT(changePage(QListWidgetItem *, QListWidgetItem*)));
    connect(ui->listFrameFileWidget,SIGNAL(itemDoubleClicked(QListWidgetItem*)),this,SLOT(on_listframes_DoubleClicked(QListWidgetItem*)));
    connect(ui->btnClearFrame,SIGNAL(clicked()),this,SLOT(on_clearFrame_clicked()));
    connect(ui->btnTermUpdate,SIGNAL(clicked()),this,SLOT(on_TermUpdate_clicked()));
    connect(ui->btnTermRefresh,SIGNAL(clicked()),this,SLOT(on_TermRefresh_clicked()));
    connect(ui->btnTermClose,SIGNAL(clicked()),this,SLOT(on_TermClose_clicked()));
    connect(ui->btnTermOpen,SIGNAL(clicked()),this,SLOT(on_TermOpen_clicked()));
    connect(ui->btnproj_on,SIGNAL(clicked()),this,SLOT(on_projOn_clicked()));
    connect(ui->btnproj_off ,SIGNAL(clicked()),this,SLOT(on_projOff_clicked()));
    connect(ui->btnproj_blank,SIGNAL(clicked()),this,SLOT(on_projblank_clicked()));
    connect(ui->btnproj_display,SIGNAL(clicked()),this,SLOT(on_projdisplay_clicked()));
    connect(ui->btnRefresh,SIGNAL(clicked()),this,SLOT(btnRefreshList_clicked()));
    connect(ui->btnLisans,SIGNAL(clicked()),this,SLOT(btnLisansClick()));
    connect(ui->btnQuit,SIGNAL(clicked()),this,SLOT(on_btnQuit_clicked()));
    connect(ui->btnNightView,SIGNAL(clicked(bool)),this,SLOT(on_btnNightView_clicked(bool)));

    conf = StelApp::getInstance().getSettings();
    core = StelApp::getInstance().getCore();

    //Favorit List box i?eri?i
    QDir dir(qApp->applicationDirPath()+"/favorits");
    dir.setFilter(QDir::Files | QDir::Hidden | QDir::NoSymLinks);
    dir.setSorting(QDir::Size | QDir::Reversed);
    QStringList filters;
    filters << "*.fsc" ;
    dir.setNameFilters(filters);

    QFileInfoList list = dir.entryInfoList();
    for (int i = 0; i < list.size(); ++i)
    {
        QFileInfo fileInfo = list.at(i);
        QListWidgetItem *newItem = new QListWidgetItem(QIcon(),fileInfo.fileName(),ui->listWidget,QListWidgetItem::UserType);
    }
    ///

    //-------Style ayarlar?
    QMargins margins;
    margins.setLeft(0);
    margins.setTop(0);
    margins.setRight(0);
    margins.setBottom(0);

    ui->adminFrame->layout()->setContentsMargins(margins);
    ui->adminLayout->setContentsMargins(margins);
    ui->frame->layout()->setContentsMargins(margins);
    ui->MediaLayout->setContentsMargins(margins);
    //ui->frame->setStyleSheet("color:red");
    //------------

    //Media Liste i?eri?i-----
    media_path = conf->value("main/media_path", "C:/").toString();
    is_audio = conf->value("main/flag_audio","false").toBool();
    flat_media_path = conf->value("main/flat_media_path", "C:/").toString();

    //    QDir dirm(media_path);
    //    dirm.setFilter(QDir::Files | QDir::Hidden | QDir::NoSymLinks);
    //    dirm.setSorting(QDir::Size | QDir::Reversed);
    //    QStringList filtersm;
    //    filtersm << "*.avi" << "*.mpeg" << "*.mpg" << "*.mp4" << "*.wmv" << "*.mov" << "*.flv" << "*.vob"  ;
    //    dirm.setNameFilters(filtersm);
    //
    //    QFileInfoList listm = dirm.entryInfoList();
    //    for (int i = 0; i < listm.size(); ++i)
    //    {
    //        QFileInfo fileInfom = listm.at(i);
    //        QListWidgetItem *newItemm = new QListWidgetItem(QIcon(":/graphicGui/gui/tabicon-images.png"),fileInfom.fileName(),ui->listFilesWidget,QListWidgetItem::ItemType());
    //    }
    //
    imagesFuldomeVideoShow_ = new QFutureWatcher<QImage>(this);
    connect(imagesFuldomeVideoShow_, SIGNAL(resultReadyAt(int)), SLOT(setFuldomeVideoItemInList(int)));
    connect(imagesFuldomeVideoShow_, SIGNAL(finished()), SLOT(finished()));
    ShowFulldomeVideoIconFile(media_path );

    ui->lblPath->setText(media_path);

    //ui->listFilesWidget->sortItems(Qt::AscendingOrder);
    //ui->listFilesWidget->setIconSize(QSize(32, 32));
    //ui->listFilesWidget->setFlow(QListView::TopToBottom);
    ////-----

    //    //Frame Liste i?eri?i
    //    QDir dirf(media_path+"/panorama");
    //    dirf.setFilter(QDir::Files | QDir::Hidden | QDir::NoSymLinks);
    //    dirf.setSorting(QDir::Size | QDir::Reversed);
    //    filtersm.clear();
    //    filtersm << "*.jpg" << "*.jpeg" << "*.png" << "*.bmp" << "*.tiff" << "*.gif"  ;
    //    dirm.setNameFilters(filtersm);
    //
    //    listm = dirf.entryInfoList();
    //    for (int i = 0; i < listm.size(); ++i)
    //    {
    //        QFileInfo fileInfom = listm.at(i);
    //        //QListWidgetItem *newItemm = new QListWidgetItem(QIcon(":/graphicGui/gui/tabicon-landscape.png"),fileInfom.fileName(),ui->listFrameFileWidget,QListWidgetItem::UserType);
    //        QListWidgetItem *newItemm = new QListWidgetItem(QIcon(),fileInfom.fileName(),ui->listFrameFileWidget,QListWidgetItem::UserType);
    //    }
    imagesFisheyeShow_ = new QFutureWatcher<QImage>(this);
    connect(imagesFisheyeShow_, SIGNAL(resultReadyAt(int)), SLOT(setFisheyeImageItemInList(int)));
    connect(imagesFisheyeShow_, SIGNAL(finished()), SLOT(finished()));
    ShowImageIconFile(media_path+"/panorama");

    //ui->listFrameFileWidget->sortItems(Qt::AscendingOrder);

    //ui->listFrameFileWidget->setIconSize(QSize(32, 32));
    //ui->listFrameFileWidget->setFlow(QListView::TopToBottom);
    //----

    //core->ALLFader.setDuration(2000);
    //core->ALLFader = false;

    tSaniye =  conf->value("main/vd_start_command_time_delay",3).toInt();

    checkTimeServer();
    checkFtpServer();

    ui->lblTimeServer->setStyleSheet("background: rgb(255, 0, 0)");
    ui->lblFtpServer->setStyleSheet("background: rgb(255, 0, 0)");

    ui->stackedWidget->setCurrentIndex(0);
    ui->stackListWidget->setCurrentRow(0);

    connect(&StelMainGraphicsView::getInstance().getScriptMgr(), SIGNAL(scriptStopped()), this, SLOT(scriptEnded()));
    connect(ui->btnStopFavorit, SIGNAL(clicked()), this, SLOT(btnStopFavoritClicked()));

    //Admin ?zerllikleri gizleniyor
    //ui->page_5->hide();

    ui->stackListWidget->item(4)->setHidden(true);
    ui->lblFtpServer->hide();
    ui->lblTimeServer->hide();

    //#ifdef SHIRAPLAYER_PRE
    ui->btnLiveMode->hide();
    ui->btnApply->hide();

    ui->btnFrameLoadtoTerminals->hide();
    ui->Adminwidget->setVisible(false);
    //#endif

    m_timer.setInterval(100);
    connect(&m_timer, SIGNAL(timeout()),this,SLOT(tracktime()));

    //Presenter Stack
    setupPresenterWidgets();
    QObject::connect(mediaBrowserList, SIGNAL(currentItemChanged( QListWidgetItem*, QListWidgetItem*)),
                     this, SLOT(currentMediaItemChanged( QListWidgetItem*, QListWidgetItem*) ));

    //ui->page_4->hide();
    //ui->stackListWidget->item(3)->setHidden(true);
    ///

    ui->lisans->setStyleSheet("color: red;");
    ui->btnLisans->setStyleSheet("color: red;");
    QSettings settings("Sureyyasoft", "ShiraPlayer");
    settings.beginGroup("Licenses");
    QDate dt = settings.value("record_date",QDateTime::currentDateTime()).toDate();
    QString uname =settings.value("user","").toString();
    QString lcode =settings.value("license_code","").toString();
    settings.endGroup();

    if ( QString::compare(lcode,frm.KodOlustur(dt,uname),Qt::CaseInsensitive) == 0)
    {
        ui->btnLisans->setVisible(false);
        ui->lisans->setVisible(false);
        ui->registerLayout->removeWidget(ui->btnLisans);
        ui->registerLayout->removeWidget(ui->lisans);
        StelMainWindow::getInstance().is_Licenced = true;
    }
    else
    {
        frm.FormDoldur(uname,lcode);
        StelMainWindow::getInstance().is_Licenced = false;
    }

    //#ifdef SHIRAPLAYER_PRE
    if(!StelMainWindow::getInstance().getIsMultiprojector())
        videoSharedMem.setKey("videoSharedMemory");
    //#endif

    connect(ui->btnPLoad,SIGNAL(clicked()),this,SLOT(on_btnPLoad_clicked()));
    connect(ui->btnPSave,SIGNAL(clicked()),this,SLOT(on_btnPSave_clicked()));
    connect(ui->btnPNew,SIGNAL(clicked()),this,SLOT(on_btnPNew_clicked()));
    connect(ui->chDaylight,SIGNAL(clicked(bool)),this,SLOT(on_chDaylight_toggled(bool)));
    connect(ui->chShowFPS,SIGNAL(stateChanged(int)),this,SLOT(on_chShowFPS_stateChanged(int)));
    connect(ui->chpreWarped,SIGNAL(stateChanged(int)),this,SLOT(on_chpreWarped_stateChanged(int)));
    connect(ui->chCrossFade,SIGNAL(stateChanged(int)),this,SLOT(on_chCrossFade_stateChanged(int)));
    connect(ui->spFadeDuration,SIGNAL(valueChanged(int)),this,SLOT(on_spFadeDuration_valueChanged(int)));

    connect(ui->trackMedia,SIGNAL(sliderPressed()),this,SLOT(on_trackMedia_sliderPressed()));
    connect(ui->trackMedia,SIGNAL(sliderReleased()),this,SLOT(on_trackMedia_sliderReleased()));

    if(!StelApp::getInstance().isAdvancedMode)
        ui->stackListWidget->hide();
    ui->lblVersion->setText(ui->lblVersion->text()+PACKAGE_VERSION);

    currentState = Stopped;

    ui->stackedWidget->setCurrentIndex(5);

    tetiklemeTimer = new QTimer(this);
    tetiklemeTimer->setInterval(3000);
    tetiklemeTimer->setSingleShot(true);

    connect(tetiklemeTimer, SIGNAL(timeout()), this, SLOT(tetiklemeTimerProc()));

    connect(ui->btnSetFreeHand,SIGNAL(clicked()),this,SLOT(on_btnSetFreeHand_clicked()));
    connect(ui->btnDeleteFreeHand,SIGNAL(clicked()),this,SLOT(on_btnDeleteFreeHand_clicked()));
    connect(ui->btnClearFreeHand,SIGNAL(clicked()),this,SLOT(on_btnClearFreeHand_clicked()));
    connect(ui->btnSelectFreeHand,SIGNAL(clicked()),this,SLOT(on_btnSelectFreeHand_clicked()));

    //Free Hand Color Palette
    core->freeHandPen =  QColor(255, 0, 0, 127);
    colorpaletWidget = new TupColorPalette(core->freeHandPen.brush(),QSize(1600,700),0);
    ui->layoutColorChooser->addWidget(colorpaletWidget);
    connect(colorpaletWidget, SIGNAL(updateColor(const QColor)), this, SLOT(setCurrentColor(const QColor)));

    //FreeHand Stroke Size
    setSSLabelPanel();
    setSSBrushCanvas();
    setSSSlider();

    //FreeHand Opacity Size
    setOPLabelPanel();
    setOPBrushCanvas();
    setOPSlider();

    setCurrentColor(core->freeHandPen.color());
    sliderOP->setValue(92.0);

    //Fulldome video sets
    ui->chShowFPS->setChecked(conf->value("video/show_fps", false).toBool());
    ui->chpreWarped->setChecked(conf->value("video/prewarped", false).toBool());
    //Fisheye frame sets
    //ui->chCrossFade->setChecked(conf->value("video/crossfade", false).toBool()); CrossFade iptal edildi
    ui->chCrossFade->setChecked(false);
    ui->chCrossFade->hide();
    ui->chDaylight->setChecked(conf->value("video/frame_withdaylight",false).toBool());
    ui->spFadeDuration->setValue(conf->value("video/fadeduration",false).toInt());

    ui->chSound->setHidden(true);

    //ui->LocationBar->hide();

    //#ifndef SHIRAPLAYER_PRE
    if(StelMainWindow::getInstance().getIsMultiprojector())
        StelApp::getInstance().isLiveMode = true;
    //#endif

    showFisheyeFrameTimer = new QTimer(this);
    showFisheyeFrameTimer->setInterval(2000);
    showFisheyeFrameTimer->setSingleShot(true);
    connect(showFisheyeFrameTimer, SIGNAL(timeout()), this, SLOT(showFisheyeFrameBYTimer()));

    startGraphTimer = new QTimer(this);
    startGraphTimer->setInterval(2000);
    startGraphTimer->setSingleShot(true);
    connect(startGraphTimer, SIGNAL(timeout()), this, SLOT(startGraphBYTimer()));

    stopGraphTimer = new QTimer(this);
    stopGraphTimer->setInterval(2000);
    stopGraphTimer->setSingleShot(true);
    connect(stopGraphTimer, SIGNAL(timeout()), this, SLOT(stopGraphBYTimer()));

    clearFramesTimer = new QTimer(this);
    clearFramesTimer->setInterval(2000);
    clearFramesTimer->setSingleShot(true);
    connect(clearFramesTimer, SIGNAL(timeout()), this, SLOT(clearFramesBYTimer()));

    oldViewDirection = Vec3d(0,0,0);

    connect(ui->sliderBrightness,SIGNAL(valueChanged(int)),this,SLOT(on_videocolors_valueChanged(int)));
    connect(ui->sliderContrast,SIGNAL(valueChanged(int)),this,SLOT(on_videocolors_valueChanged(int)));
    connect(ui->sliderSaturation,SIGNAL(valueChanged(int)),this,SLOT(on_videocolors_valueChanged(int)));
    connect(ui->btnVideoColorReset,SIGNAL(clicked()),this,SLOT(on_btnVideoColorReset_clicked()));

}
void servermanager::setCurrentColor(const QColor color)
{
    core->freeHandPen = color;
    thickSSPreview->setColor(color);
    thickSSPreview->render(core->freeHandSize);
    thickSSPreview->update();
    sliderSSS->setColors(color,color);
    sliderSSS->update();

    thickOPPreview->setColor(color);
    thickOPPreview->render(core->freeHandOpacity);
    thickOPPreview->update();
    sliderOP->setColors(color,color);
    sliderOP->update();

}

void servermanager::retranslate()
{
    languageChanged();
}
void servermanager::setSSLabelPanel()
{
    SSSLabel = new QLabel(q_("Size: ") + QString::number(core->freeHandSize));
    int fontSize = 16;
    SSSLabel->setFont(QFont("Arial", fontSize, QFont::Normal));
    SSSLabel->setAlignment(Qt::AlignHCenter);
    ui->layoutStrokeSize->addWidget(SSSLabel);
}
void servermanager::setSSBrushCanvas()
{
    thickSSPreview = new TupPenPreviewCanvas(core->freeHandPen,
                                             core->freeHandOpacity);
    ui->layoutStrokeSize->addWidget(thickSSPreview);
}
void servermanager::setSSSlider()
{
    QPen p = core->freeHandPen;
    sliderSSS = new TupSlider(Qt::Horizontal,
                              TupSlider::Size,
                              p.color(),
                              p.color());

    sliderSSS->setBrushSettings(p.brush().style(),
                                core->freeHandOpacity);
    sliderSSS->setRange(1, 50);
    sliderSSS->setValue(core->freeHandSize);
    connect(sliderSSS, SIGNAL(valueChanged(int)), this, SLOT(modifyStrokeSize(int)));

    sliderSSSwidget = new QWidget;
    sliderSSSwidget->setFixedHeight(53);
    QBoxLayout *sliderLayout = new QHBoxLayout(sliderSSSwidget);
    //sliderLayout->setContentsMargins(0, 0, 0, 0);
    //sliderLayout->setSpacing(0);
    sliderLayout->addWidget(sliderSSS);

    ui->layoutStrokeSize->addWidget(sliderSSSwidget);
}
void servermanager::modifyStrokeSize(int value)
{
    core->freeHandSize = value;
    thickSSPreview->render(core->freeHandSize);
    SSSLabel->setText(q_("Size: ") + QString::number(core->freeHandSize));
    ui->grpStrokeSize->update();
}

void servermanager::setOPLabelPanel()
{
    double currentOpacity = core->freeHandOpacity;
    QString number = QString::number(currentOpacity);
    if (number.length() == 3)
        number = number + "0";

    if (number.compare("1") == 0)
        number = "1.00";

    OPLabel = new QLabel(q_("Opacity: ") + number);
    int fontSize = 16;

    OPLabel->setFont(QFont("Arial", fontSize, QFont::Normal));

    OPLabel->setAlignment(Qt::AlignHCenter);
    ui->layoutOpacitySize->addWidget(OPLabel);
}

void servermanager::setOPBrushCanvas()
{
    thickOPPreview = new TupPenPreviewCanvas(core->freeHandPen,
                                             core->freeHandOpacity);
    ui->layoutOpacitySize->addWidget(thickOPPreview);
}
void servermanager::setOPSlider()
{
    QPen p = core->freeHandPen;
    double currentOpacity = core->freeHandOpacity;
    sliderOP = new TupSlider(Qt::Horizontal, TupSlider::Opacity,
                             p.color(),
                             p.color());
    sliderOP->setRange(0, 100);
    sliderOP->setBrushSettings(p.brush().style(), currentOpacity);
    sliderOP->setValue(currentOpacity*100.0);
    connect(sliderOP, SIGNAL(valueChanged(int)), this, SLOT(modifyOpacitySize(int)));

    sliderOPwidget = new QWidget;
    sliderOPwidget->setFixedHeight(53);
    QBoxLayout *sliderlayout = new QHBoxLayout(sliderOPwidget);
    sliderlayout->addWidget(sliderOP);

    ui->layoutOpacitySize->addWidget(sliderOPwidget);
}

void servermanager::modifyOpacitySize(int value)
{
    core->freeHandOpacity = value / 100.0;

    if (core->freeHandOpacity == 0) {
        OPLabel->setText(q_("Opacity: 0.00"));
    } else if (core->freeHandOpacity == 1) {
        OPLabel->setText(q_("Opacity: 1.00"));
    } else {
        QString number = QString::number(core->freeHandOpacity );
        if (number.length() == 3)
            number = number + "0";
        OPLabel->setText(q_("Opacity: ") + number);
    }
    //qDebug()<<core->freeHandOpacity ;
    thickOPPreview->render(core->freeHandOpacity );
    ui->grpOpacitySize->update();
}

void servermanager::showFisheyeFrameBYTimer()
{
    QFileInfo thePath(filepath);
    if(thePath.exists())
    {
        StelCore* core = StelApp::getInstance().getCore();
        LandscapeMgr* lmgr = GETSTELMODULE(LandscapeMgr);
        lmgr->setFlagLandscape(false);

        //G?r?n?m g?ney ?nde olacak ?ekle getiriliyor
        if (StelApp::getInstance().returntoFrontSky)
            core->getMovementMgr()->setViewDirectionJ2000(core->getNavigator()->altAzToJ2000(StelUtils::strToVec3f("0.01,0,1").toVec3d()));

        sLandscape = lmgr->getCurrentLandscapeName();
        old_isVideoLandscape = lmgr->isVideoLandscape;
        lmgr->doSetCurrentLandscapetoFrame(filepath,sLandscape,ui->chDaylight->isChecked(),0);
        core->ALLFader = false;
        lmgr->setFlagLandscape(true);

    }
}

void servermanager::startGraphBYTimer()
{
    StelCore* core = StelApp::getInstance().getCore();
    LandscapeMgr* lmgr = GETSTELMODULE(LandscapeMgr);

    //?u anki landscape haf?zaya al?n?yor
    sLandscape = lmgr->getCurrentLandscapeName();
    old_isVideoLandscape = lmgr->isVideoLandscape;
    lmgr->isVideoLandscape = false;
    //if(lmgr->s_Name != "")
    //    lmgr->doClearLandscapeVideo();
    lmgr->setFlagLandscape(true);

    //Scripte ile Yap?lan hareket de?i?iklikleri geri al?n?yor.
    //StelMovementMgr* mmgr = GETSTELMODULE(StelMovementMgr);
    //mmgr->panView(-mmgr->rec_deltaAz,-mmgr->rec_deltaAlt);

    //G?r?n?m g?ney ?nde olacak ?ekle getiriliyor
    if (StelApp::getInstance().returntoFrontSky)
        core->getMovementMgr()->setViewDirectionJ2000(core->getNavigator()->altAzToJ2000(StelUtils::strToVec3f("0.01,0,1").toVec3d()));

    QFileInfo uDir(strfile);
    if (uDir.exists())
    {
        lmgr->s_Name = "";
        lmgr->doSetCurrentLandscapetoVideo(strfile);
        /*if ((is_audio) && (ui->chSound->isChecked()))
        {
            //todo
            //Load_Audio(strfile.toLatin1().data());
        }*/
    }

    /*if ((is_audio) && (ui->chSound->isChecked()))
    {
        if (uDir.exists())
        {
            //todo
            //Start_Audio(strfile.toLatin1().data());
            qDebug()<<"Audio start:"<< QDateTime().currentDateTime().toString("hh:mm:ss.zzz");
        }
    }*/

    //todo
    ui->progressBuffer->setMaximum(lmgr->vop_curr->GetVideFrameRate());
    connect(lmgr->vop_curr, SIGNAL(finished()), this, SLOT(on_btnmStop_clicked()));
    connect(lmgr->vop_curr,SIGNAL(valueBufferChanged(int)),ui->progressBuffer,SLOT(setValue(int)));
    lmgr->doStartVideo();
    qDebug()<<"Video start:"<< QDateTime().currentDateTime().toString("hh:mm:ss.zzz");
    //T?m ?izimler LandscapeMgr hari? kapatl?yor
    //StelApp::getInstance().isVideoMode = true;
    //old_showPropGui = StelApp::getInstance().showPropGui ;
    StelApp::getInstance().showPropGui = false;
    ///
    core->ALLFader = false;
    m_timer.start();
}

void servermanager::stopGraphBYTimer()
{
    StelCore* core = StelApp::getInstance().getCore();

    //G?r?n?m eski ?ekline getiriliyor
    //core->getMovementMgr()->setViewDirectionJ2000(oldViewDirection);
    if (StelApp::getInstance().returntoFrontSky)
        core->getMovementMgr()->moveToAltAz(oldViewDirection);
    oldViewDirection = Vec3d(0,0,0);

    LandscapeMgr* lmgr = GETSTELMODULE(LandscapeMgr);
    //StelApp::getInstance().isVideoMode = false;
    lmgr->doClearLandscapeVideo();
    lmgr->vop_curr->CloseVideo();

    lmgr->setCurrentLandscapeName(sLandscape,false,old_isVideoLandscape);
    lmgr->setFlagLandscape(sLandscapeVisibleFlag);

    StelMainGraphicsView::getInstance().getOpenGLWin()->repaint();

    core->ALLFader = false;
    m_timer.stop();
    ui->trackMedia->setValue(0);
}

void servermanager::clearFramesBYTimer()
{
    if(sLandscape != "")
    {
        StelCore* core = StelApp::getInstance().getCore();
        LandscapeMgr* lmgr = GETSTELMODULE(LandscapeMgr);

        //G?r?n?m eski ?ekline getiriliyor
        if (StelApp::getInstance().returntoFrontSky)
            core->getMovementMgr()->moveToAltAz(oldViewDirection);
        oldViewDirection = Vec3d(0,0,0);

        lmgr->doSetCurrentLandscapeName(sLandscape);
        //lmgr->setCurrentLandscapeName(sLandscape,false,old_isVideoLandscape);
        lmgr->setFlagLandscape(bFlagLandscape);
        sLandscape = "";
        core->ALLFader = false;

    }
}

void servermanager::on_videocolors_valueChanged(int val)
{
    StelApp::getInstance().getRsync()->sendChanges(RSYNC_COMMAND_VIDEO_COLORS,
                                                   QString("%1@%2@%3")
                                                   .arg(ui->sliderBrightness->value()/100.0)
                                                   .arg(ui->sliderContrast->value()/100.0)
                                                   .arg(ui->sliderSaturation->value()/100.0));

    LandscapeMgr* lmgr = GETSTELMODULE(LandscapeMgr);
    lmgr->setVideoBrightness(ui->sliderBrightness->value()/100.0);
    lmgr->setVideoContrast(ui->sliderContrast->value()/100.0);
    lmgr->setVideoSaturation(ui->sliderSaturation->value()/100.0);
}

void servermanager::on_btnVideoColorReset_clicked()
{
    ui->sliderBrightness->setValue(100);
    ui->sliderContrast->setValue(100);
    ui->sliderSaturation->setValue(100);
}

void servermanager::prepareAsChild(){}

//--FisheyeFrame Listesi ??in
void servermanager::WaitFuture()
{
    if (imagesFisheyeShow_->isRunning())
    {
        imagesFisheyeShow_->cancel();
        imagesFisheyeShow_->waitForFinished();
    }
}

void servermanager::setFuldomeVideoItemInList(int index)
{
    ui->listFilesWidget->item(index)->setIcon(QIcon(QPixmap::fromImage(imagesFuldomeVideoShow_->resultAt(index))));
}

//Fulldome listesi i?in
void servermanager::WaitFutureFulldomeList()
{
    if (imagesFuldomeVideoShow_->isRunning())
    {
        imagesFuldomeVideoShow_->cancel();
        imagesFuldomeVideoShow_->waitForFinished();
    }
}

void servermanager::setFisheyeImageItemInList(int index)
{
    ui->listFrameFileWidget->item(index)->setIcon(QIcon(QPixmap::fromImage(imagesFisheyeShow_->resultAt(index))));
}


QFileInfoList servermanager::getImageFisheyeListFiles(QString dirPath) const
{
    QDir dir(dirPath);
    dir.setFilter(QDir::Files | QDir::Hidden | QDir::NoSymLinks);
    dir.setSorting(QDir::Name);
    QStringList filters;
    filters <<"*.jpg" << "*.jpeg" << "*.png" << "*.bmp" << "*.tiff" << "*.gif" ;
    dir.setNameFilters(filters);

    return dir.entryInfoList(filters, QDir::Files);
}

QFileInfoList servermanager::getFulldomeVideoListFiles(QString dirPath) const
{
    QDir dir(dirPath);
    dir.setFilter(QDir::Files | QDir::Hidden | QDir::NoSymLinks);
    dir.setSorting(QDir::Name );
    QStringList filters;
    filters << "*.avi" << "*.mpeg" << "*.mpg" << "*.mp4" << "*.wmv" << "*.mov" << "*.flv" << "*.vob"  ;
    dir.setNameFilters(filters);

    return dir.entryInfoList(filters, QDir::Files);
}


void servermanager::ShowImageIconFile(QString dirPath)
{
    WaitFuture();
    ui->listFrameFileWidget->clear();
    QFileInfoList fileList_ = getImageFisheyeListFiles(dirPath);
    int count = fileList_.count();
    for (int i = 0; i < count; ++i)
    {
        QListWidgetItem *item = new QListWidgetItem;
        item->setIcon(QIcon(prepareIcon(fileList_[i])));
        item->setData(Qt::WhatsThisRole, QString(fileList_[i].filePath()));
        ui->listFrameFileWidget->addItem(item);
    }

    imagesFisheyeShow_->setFuture(QtConcurrent::mapped(fileList_, prepareImage));
}

void servermanager::ShowFulldomeVideoIconFile(QString dirPath)
{
    WaitFutureFulldomeList();
    ui->listFilesWidget->clear();
    QFileInfoList fileList_ = getFulldomeVideoListFiles(dirPath);
    int count = fileList_.count();
    for (int i = 0; i < count; ++i)
    {
        QListWidgetItem *item = new QListWidgetItem;
        //item->setIcon(QIcon(prepareIconVideo(fileList_[i])));
        item->setData(Qt::WhatsThisRole, QString(fileList_[i].filePath()));
        item->setText(QString(fileList_[i].fileName()));
        ui->listFilesWidget->addItem(item);
    }

    //imagesFuldomeVideoShow_->setFuture(QtConcurrent::mapped(fileList_, prepareVideoImage));
}

QPixmap prepareIcon(const QFileInfo &infoFile)
{
    QImage imageIcon(QLatin1String(":/mainWindow/gui/Image.png"));
    imageIcon = imageIcon.scaledToHeight(HEIGHT_IMAGE, Qt::SmoothTransformation);

    int image_width = imageIcon.width();
    int image_height = imageIcon.height();

    QRectF target((WIDTH_ICON - image_width) / 2, 0, image_width, image_height);
    QRectF source(0, 0, image_width, image_height);


    QPixmap pixDraw(QSize(WIDTH_ICON, HEIGHT_ICON));
    QPainter painter(&pixDraw);

    painter.setBrush(Qt::NoBrush);
    painter.fillRect(QRect(0, 0, WIDTH_ICON, HEIGHT_ICON),QColor(8, 31, 37));


    painter.setPen(Qt::black);
    painter.drawImage(target, imageIcon, source);
    painter.drawRect(target);


    painter.setPen(Qt::darkGray);

    QRect rect((WIDTH_ICON - image_width) / 2, image_height + 2, image_width, HEIGHT_ICON - image_height - 4);
    QFontMetrics font_metrics(painter.font());
    QString elideText = font_metrics.elidedText(infoFile.completeBaseName(), Qt::ElideRight, image_width);
    painter.drawText(rect, Qt::AlignCenter | Qt::ElideRight, elideText);

    return pixDraw;
}

QPixmap prepareIconVideo(const QFileInfo &infoFile)
{
    QImage imageIcon(QLatin1String(":/media/gui/presenter/fulldome.png"));
    imageIcon = imageIcon.scaledToHeight(HEIGHT_IMAGE, Qt::SmoothTransformation);

    int image_width = imageIcon.width();
    int image_height = imageIcon.height();

    QRectF target((WIDTH_ICON - image_width) / 2, 0, image_width, image_height);
    QRectF source(0, 0, image_width, image_height);


    QPixmap pixDraw(QSize(WIDTH_ICON, HEIGHT_ICON2));
    QPainter painter(&pixDraw);

    painter.setBrush(Qt::NoBrush);
    painter.fillRect(QRect(0, 0, WIDTH_ICON, HEIGHT_ICON2),QColor(8, 31, 37));


    painter.setPen(Qt::black);
    painter.drawImage(target, imageIcon, source);
    painter.drawRect(target);


    painter.setPen(Qt::darkGray);

    QRect rect((WIDTH_ICON - image_width) / 2, image_height + 2, image_width, HEIGHT_ICON2 - image_height - 4);
    QFontMetrics font_metrics(painter.font());
    QString elideText = font_metrics.elidedText(infoFile.completeBaseName(), Qt::ElideRight, image_width);
    painter.drawText(rect, Qt::AlignCenter | Qt::ElideRight, elideText);

    return pixDraw;
}

QImage prepareImage(const QFileInfo &infoFile)
{
    QImageReader imageReader(infoFile.filePath());
    QSize size;
    int image_width = WIDTH_IMAGE;
    int image_height = HEIGHT_IMAGE;

    if (imageReader.supportsOption(QImageIOHandler::Size))
    {
        size = imageReader.size();
        image_width = size.width();
        image_height = size.height();
    }

    double ratio = (double)image_width / (double)image_height;
    image_height = HEIGHT_IMAGE;
    image_width = ratio * image_height;

    imageReader.setScaledSize(QSize(image_width, image_height));
    QImage image = imageReader.read();



    if (image.isNull())
    {
        QImage imageIcon(QLatin1String(":/mainWindow/gui/Image.png"));
        image = imageIcon;
        image = image.scaledToHeight(HEIGHT_IMAGE, Qt::SmoothTransformation);

        image_width = image.width();
        image_height = image.height();
    }


    QRectF target((WIDTH_ICON - image_width) / 2, 0, image_width, image_height);
    QRectF source(0, 0, image_width, image_height);


    QImage imgDraw(QSize(WIDTH_ICON, HEIGHT_ICON), QImage::Format_RGB32);
    QPainter painter(&imgDraw);

    painter.setBrush(Qt::NoBrush);
    painter.fillRect(QRect(0, 0, WIDTH_ICON, HEIGHT_ICON), QColor(8, 31, 37));


    painter.setPen(Qt::black);
    painter.drawImage(target, image, source);
    painter.drawRect(target);

    painter.setPen(Qt::darkGray);

    QRect rect((WIDTH_ICON - WIDTH_IMAGE) / 2, image_height + 2, WIDTH_IMAGE, HEIGHT_ICON - image_height - 4);
    QFontMetrics font_metrics(painter.font());
    QString elideText = font_metrics.elidedText(infoFile.completeBaseName(), Qt::ElideRight, WIDTH_IMAGE);
    painter.drawText(rect, Qt::AlignCenter | Qt::ElideRight, elideText);

    return (imgDraw);
}

QImage prepareVideoImage(const QFileInfo &infoFile)
{
    int image_width = HEIGHT_IMAGE;
    int image_height = HEIGHT_IMAGE;

    fulldomeListmutex.lock();
    QImage image ;
    //    try
    //    {
    //        VideoFile* vop = new VideoFile();
    //        vop->useYUVData = false;
    //
    //        if (vop->open(infoFile.filePath(),0,0,false,PIX_FMT_RGB24))
    //        {
    //
    //            int movie_width = vop->getFrameWidth();
    //            int movie_height = vop->getFrameHeight();
    //            int fps = vop->getFrameRate();
    //
    //            QString codecname = vop->getCodecName();
    //            double duration = vop->getDuration();
    //            int64_t durationFrame = vop->getEnd();
    //
    //            vop->setMarkIn(durationFrame / 2);
    //            vop->fillFirstFrame();
    //
    //            //?n izleme Karesi
    //            const VideoPicture* pict= vop->getPictureAtIndex(-1);
    //            const AVPicture frm = pict->get_rgb();
    //            image = QImage(vop->getFrameWidth(),vop->getFrameHeight(),QImage::Format_RGB32);
    //
    //            unsigned char *src = (unsigned char *)frm.data[0];
    //            for (int y = 0; y < vop->getFrameHeight(); y++)
    //            {
    //                QRgb *scanLine = (QRgb *)image.scanLine(y);
    //                for (int x = 0; x < vop->getFrameWidth(); x++)
    //                {
    //                    scanLine[x] = qRgb(src[3*x], src[3*x+1], src[3*x+2]);
    //                }
    //                src += frm.linesize[0];
    //            }
    //
    //            image = image.scaled(image_width,image_height,Qt::KeepAspectRatio,Qt::FastTransformation);
    //            vop->stop();
    //            //vop->close();
    //            delete vop;
    //            vop = NULL;
    //        }
    //    }
    //    catch (std::runtime_error &e)
    //    {
    //        qWarning() << "Image List error " << e.what();
    //    }


    //    QString strlabel = QString("Width: %1 Height: %2 FPS: %3 Codec: %4 Duration: %5 sn ")
    //                       .arg(movie_width)
    //                       .arg(movie_height)
    //                       .arg(fps)
    //                       .arg(codecname)
    //                       .arg(duration);
    //
    //    ui->labelProperties->setText(strlabel);



    if (image.isNull())
    {
        QImage imageIcon(QLatin1String(":/media/gui/presenter/fulldome.png"));
        image = imageIcon;
        image = image.scaledToHeight(45, Qt::SmoothTransformation);

        image_width = image.width();
        image_height = image.height();
    }


    QRectF target((WIDTH_ICON - image_width) / 2, 0, image_width, image_height);
    QRectF source(0, 0, image_width, image_height);


    QImage imgDraw(QSize(WIDTH_ICON, HEIGHT_ICON2), QImage::Format_RGB32);
    QPainter painter(&imgDraw);

    painter.setBrush(Qt::NoBrush);
    painter.fillRect(QRect(0, 0, WIDTH_ICON, HEIGHT_ICON2), Qt::black);


    painter.setPen(Qt::black);
    painter.drawImage(target, image, source);
    painter.drawRect(target);

    painter.setPen(Qt::darkGray);

    QRect rect((WIDTH_ICON - WIDTH_IMAGE) / 2, image_height, WIDTH_IMAGE, HEIGHT_ICON2 - image_height - 4);
    QFontMetrics font_metrics(painter.font());
    QString elideText = font_metrics.elidedText(infoFile.completeBaseName(), Qt::ElideRight, WIDTH_IMAGE);
    painter.drawText(rect, Qt::AlignCenter | Qt::ElideRight, elideText);

    fulldomeListmutex.unlock();

    return (imgDraw);
}

///---

void servermanager::languageChanged()
{
    //if (dialog)
    //    ui->retranslateUi(dialog);
    if (dialog)
    {
        stellaManager->languageChanged();
        flybyManager->languageChanged();
        messierwindow->languageChanged();
        constwindow->languageChanged();
        audiomanager->languageChanged();
        ui->stelWindowTitle->setText(QString(q_("%0-Manager Window")).arg("Stellarium"));

        ui->lblVersion->setText(q_("ShiraPlayer Version :")+PACKAGE_VERSION);
        ui->lisans->setText(q_("UnLicensed Use.."));
        ui->btnLisans->setText(q_("Register"));
        ui->btnFadeEffect->setText(q_("Init - Fade On/Off"));
        ui->btnNightView->setText(q_("Night View"));
        ui->btnQuit->setText(q_("Quit"));
        ui->lblTimeServer->setText(q_("Time Server"));
        ui->lblFtpServer->setText(q_("FTP Server"));
        const bool __sortingEnabled = ui->stackListWidget->isSortingEnabled();
        ui->stackListWidget->setSortingEnabled(false);
        QListWidgetItem *___qlistwidgetitem = ui->stackListWidget->item(0);
        ___qlistwidgetitem->setText(q_("Script"));
        QListWidgetItem *___qlistwidgetitem1 = ui->stackListWidget->item(1);
        ___qlistwidgetitem1->setText(q_("Video"));
        QListWidgetItem *___qlistwidgetitem2 = ui->stackListWidget->item(2);
        ___qlistwidgetitem2->setText(q_("Fisheye Image"));
        QListWidgetItem *___qlistwidgetitem3 = ui->stackListWidget->item(3);
        ___qlistwidgetitem3->setText(q_("Flat Media"));
        QListWidgetItem *___qlistwidgetitem4 = ui->stackListWidget->item(4);
        ___qlistwidgetitem4->setText(q_("Admin"));
        QListWidgetItem *___qlistwidgetitem5 = ui->stackListWidget->item(5);
        ___qlistwidgetitem5->setText(q_("Stellarium"));
        ui->stackListWidget->setSortingEnabled(__sortingEnabled);
        ui->btnApply->setText(q_("Apply"));
        ui->btnLiveMode->setText(q_("Live Mode"));
        ui->lblFavorits->setText(q_("Favorites"));
        ui->btnApplyFavorits->setText(q_("Apply Favorites"));
        ui->btnStopFavorit->setText(q_("Stop Favorites"));
        ui->chShowFPS->setText(q_("Show Video FPS"));
        ui->chpreWarped->setText(q_("Prewarped"));
        ui->btnRefresh->setText(q_("Refresh List"));
        ui->lblProperties->setText(q_("Properties"));
        ui->btnmStart->setText(q_("Start"));
        ui->btnmPause->setText(q_("Pause"));
        ui->btnmStop->setText(q_("Stop"));
        ui->lblStatus->setText(q_("Status"));
        ui->labelVolume->setText(q_("Volume"));
        ui->label_5->setText(q_("Fade Time:"));
        ui->label_6->setText(q_("sec"));
        ui->chCrossFade->setText(q_("Cross Fade"));
        ui->chDaylight->setText(q_("Show with Daylight"));
        ui->btnClearFrame->setText(q_("Clear Frame"));
        ui->listFrameFileWidget->setToolTip(QString());
        ui->btnFrameLoadtoTerminals->setText(q_("Send Media to Terminals"));
        ui->btnPNew->setToolTip(q_("New Flat Media project file"));
        ui->btnPNew->setText(q_("New"));
        ui->btnPSave->setToolTip(q_("Save Flat Media project file"));
        ui->btnPSave->setText(q_("Save"));
        ui->btnPLoad->setToolTip(q_("Open Flat Media project file"));
        ui->btnPLoad->setText(q_("Load"));
        ui->label->setText(q_("File Name:"));
        ui->lblAdmin->setText(q_("Admin PCs and Projectors"));
        ui->grpProjector->setTitle(q_("Projector Controls"));
        ui->btnproj_on->setText(q_("Power On"));
        ui->btnproj_off->setText(q_("Power Off"));
        ui->btnproj_blank->setText(q_("Blank"));
        ui->btnproj_display->setText(q_("Display"));
        ui->groupBox->setTitle(q_("Client Computers"));
        ui->btnTermOpen->setText(q_("Power On All"));
        ui->btnTermRefresh->setText(q_("Refresh All"));
        ui->btnTermClose->setText(q_("Close All"));
        ui->btnTermUpdate->setText(q_("Update All"));
        ui->btnClearFreeHand->setText(q_("Clear\n""All"));
        ui->grpColorChhoser->setTitle(q_("Color Palette"));
        ui->grpStrokeSize->setTitle(q_("Stroke Size"));
        ui->grpOpacitySize->setTitle(q_("Opacity"));
        ui->lblPathDesc->setText(q_("<html><head/><body><p><span style=\" font-size:9pt; color:#ff0000;\">Video Files Path :</span></p></body></html>"));

        ui->lblSaturation->setText(q_("Saturation"));
        ui->lblBrightness->setText(q_("Brightness"));
        ui->lblContrast->setText(q_("Contrast"));
        ui->btnVideoColorReset->setText(q_("Reset"));
    }
}

void servermanager::styleChanged()
{
    // Nothing for now
}
void servermanager::btnRefreshList_clicked()
{
    //Media Liste i?eri?i-----
    media_path = conf->value("main/media_path", "C:/").toString();
    is_audio = conf->value("main/flag_audio","false").toBool();

    ShowFulldomeVideoIconFile(media_path);

    //     ui->listFilesWidget->clear();
    //    QDir dirm(media_path);
    //    dirm.setFilter(QDir::Files | QDir::Hidden | QDir::NoSymLinks);
    //    dirm.setSorting(QDir::Size | QDir::Reversed);
    //    QStringList filtersm;
    //    filtersm << "*.avi" << "*.mpeg" << "*.mpg" << "*.mp4" << "*.wmv" << "*.mov" << "*.flv" << "*.vob"  ;
    //    dirm.setNameFilters(filtersm);
    //
    //    QFileInfoList listm = dirm.entryInfoList();
    //    for (int i = 0; i < listm.size(); ++i)
    //    {
    //        QFileInfo fileInfom = listm.at(i);
    //        QListWidgetItem *newItemm = new QListWidgetItem(QIcon(":/graphicGui/gui/tabicon-images.png"),fileInfom.fileName(),ui->listFilesWidget,QListWidgetItem::UserType);
    //    }
    //    ui->listFilesWidget->sortItems(Qt::AscendingOrder);
    //    ui->listFilesWidget->setIconSize(QSize(32, 32));
    //    ui->listFilesWidget->setFlow(QListView::TopToBottom);
    ////-----
}
void servermanager::btnliveModeClicked()
{
    //#ifndef SHIRAPLAYER_PRE
    if(StelMainWindow::getInstance().getIsMultiprojector())
        StelApp::getInstance().isLiveMode = true;
    //#else
    else
        StelApp::getInstance().isLiveMode = ui->btnLiveMode->isChecked();
    //#endif
}

void servermanager::btnFadeEffectClicked()
{
    core->allFaderColor = Vec3f(0,0,0);
    core->ALLFader.setDuration(2000);
    core->ALLFader = ui->btnFadeEffect->isChecked();

    StelApp::getInstance().getRsync()->sendInitdata(ui->btnFadeEffect->isChecked());
}

void servermanager::btnApplyClicked()
{
    if(StelApp::getInstance().nc.toText()!="")
    {
        StelApp::getInstance().getRsync()->sendChanges(RSYNC_COMMAND_INIT_OFF, "fadeoff");
#ifdef Q_OS_WIN
        Sleep(2000);
#elif defined Q_OS_LINUX
        sleep(2);
#endif
        StelApp::getInstance().getRsync()->sendChanges(RSYNC_COMMAND_SCRIPT,StelApp::getInstance().nc.toText());
        StelApp::getInstance().nc.clear();
    }
}

void servermanager::favoritselectedChange()
{
    QString fileName = qApp->applicationDirPath()+"/favorits/"+ui->listWidget->selectedItems()[0]->text();
    QFile file(fileName);
    if (file.open(QIODevice::ReadOnly))
    {
        textScript = file.readAll();
        file.close();
    }
}

void servermanager::btnApplyFavoritsClicked()
{
    if(textScript != "")
    {
        //QMessageBox::information(0,"",textScript,0,0);
        StelApp::getInstance().getRsync()->sendChanges(RSYNC_COMMAND_LIVE_SCRIPT,textScript);
        StelMainGraphicsView::getInstance().getScriptMgr().runScriptNetwork(textScript);

        //StelScriptMgr& scriptMgr = StelMainGraphicsView::getInstance().getScriptMgr();
        //scriptMgr.runScript2(ui->listWidget->currentItem()->text());

        ui->btnApplyFavorits->setEnabled(false);
        ui->btnStopFavorit->setEnabled(true);
    }
}

void servermanager::changePage(QListWidgetItem *current, QListWidgetItem *previous)
{
    if (!current)
        current = previous;
    ui->stackedWidget->setCurrentIndex(ui->stackListWidget->row(current));
}

void image2Pixmap(QImage &img,QPixmap &pixmap)
{
    // Convert the QImage to a QPixmap for display
    pixmap = QPixmap(100,100);
    QPainter painter;
    painter.begin(&pixmap);
    QRectF target(0, 0, 100, 100);
    QRectF source(0.0, 0.0, img.width(), img.width());
    painter.drawImage(target,img,source);
    painter.end();
}
void servermanager::on_mediaselectedChange()
{
    //    QString strfile = media_path+"/"+ui->listFilesWidget->selectedItems()[0]->text();
    //    VideoFile* vop = new VideoFile();
    //
    //    vop->useYUVData = false;
    //
    //    vop->open(strfile,0,0,false,PIX_FMT_RGB24);
    //
    //    movie_width = vop->getFrameWidth();
    //    movie_height = vop->getFrameHeight();
    //    fps = vop->getFrameRate();
    //
    //    codecname = vop->getCodecName();
    //    duration = vop->getDuration();
    //    durationFrame = vop->getEnd();
    //
    //    //?n izleme Karesi
    //    const VideoPicture* pict= vop->getPictureAtIndex(durationFrame/2);
    //
    //    const AVPicture frm = pict->get_rgb();
    //
    //    QImage imageQt = QImage(vop->getFrameWidth(),vop->getFrameHeight(),QImage::Format_RGB32);
    //
    //    unsigned char *src = (unsigned char *)frm.data[0];
    //    for (int y = 0; y < vop->getFrameHeight(); y++)
    //    {
    //        QRgb *scanLine = (QRgb *)imageQt.scanLine(y);
    //        for (int x = 0; x < vop->getFrameWidth(); x++)
    //        {
    //            scanLine[x] = qRgb(src[3*x], src[3*x+1], src[3*x+2]);
    //        }
    //        src += frm.linesize[0];
    //    }
    //
    //    QPixmap p;
    //    image2Pixmap(imageQt,p);
    //    ui->listFilesWidget->selectedItems()[0]->setIcon(QIcon(p));
    //
    //    p = QPixmap();
    //    imageQt = QImage();
    //    //delete pict;
    //
    //    vop->stop();
    //    delete vop;
    //    vop = NULL;
    //
    //    QString strlabel = QString("Width: %1 Height: %2 FPS: %3 Codec: %4 Duration: %5 sn ")
    //                       .arg(movie_width)
    //                       .arg(movie_height)
    //                       .arg(fps)
    //                       .arg(codecname)
    //                       .arg(duration);
    //
    //    ui->labelProperties->setText(strlabel);

}

void servermanager::on_bntmStart_clicked()
{
    QString m_serverAddress;
    strfile = "";

    if(ui->listFilesWidget->selectedItems().count()==0)
    {
        QMessageBox::warning(0,q_("Warning"),q_("Please select a media file!"),0,0);
        setMediaButtonDurum();
        return;
    }

    strfile=ui->listFilesWidget->selectedItems()[0]->data(Qt::WhatsThisRole).toString();
    if (StelUtils::isUnicodeInclude(strfile))
    {
        QMessageBox::warning(0,"Warning!","File must be in ascii character path!");
        setMediaButtonDurum();
        return;
    }

    LandscapeMgr* lmgr = GETSTELMODULE(LandscapeMgr);
    lmgr->fromScriptVideo = false;
    if (StelMainWindow::getInstance().getIsMultiprojector())
    {
        //Server da baþlat


        core->showFPS = ui->chShowFPS->isChecked();
        core->allFaderColor = Vec3f(0,0,0);
        core->ALLFader.setDuration(2000);
        core->ALLFader = true;

        //?u anki landscape haf?zaya al?n?yor
        sLandscapeVisibleFlag = lmgr->getFlagLandscape();
        sLandscape = lmgr->getCurrentLandscapeName();
        old_isVideoLandscape = lmgr->isVideoLandscape;
        lmgr->isVideoLandscape = false;

        oldViewDirection = core->getNavigator()->j2000ToAltAz(core->getMovementMgr()->getViewDirectionJ2000());
        qDebug()<<"start oldViewDirection"<<oldViewDirection.toStringLonLat();
        startGraphTimer->start();
    }
    else
    {
        // Fisheye image gösteriliyorsa öncelikle preview ekranda kapatýlýyor.(Client ta göndermeye gerek yok
        if(sLandscape!="")
        {
            LandscapeMgr* lmgr = GETSTELMODULE(LandscapeMgr);
            lmgr->setCurrentLandscapeName(sLandscape,false,old_isVideoLandscape);
            sLandscape = "";
        }
    }


    QDateTime timedelay = QDateTime::currentDateTime().addSecs(tSaniye);

    //QMessageBox::information(0,"",QString("%0").arg(ui->chpreWarped->isChecked()),0,0);
    //Client ta Baþlat
    QString senddata= QString("%0;%1;%2;%3;%4;%5;%6;%7;%8;%9;%10")
            .arg(strfile)
            .arg(movie_width)
            .arg(movie_height)
            .arg(ui->tbarSes->value())
            .arg(timedelay.toString("dd.MM.yyyy hh:mm:ss"))
            .arg(m_serverAddress)
            .arg(ui->chpreWarped->isChecked())
            .arg(ui->chShowFPS->isChecked())
            .arg(true)
            .arg(false)
            .arg(ui->chSound->isChecked());

    StelApp::getInstance().getRsync()->sendChanges(RSYNC_COMMAND_START_GRAPH, senddata);
    //

    durationFrame = 100;
    ui->trackMedia->setMinimum(0);
    ui->trackMedia->setMaximum(durationFrame);
    if (!StelMainWindow::getInstance().getIsMultiprojector())
        m_timer.start();
    currentState = Running;
    setMediaButtonDurum();
    //ui->chUseShaders->setEnabled(false);
    //ui->chSkipNextFrame->setEnabled(false);


    /*strfile=ui->listFilesWidget->selectedItems()[0]->data(Qt::WhatsThisRole).toString();
    ShiraVideoManager* videoMan= GETSTELMODULE(ShiraVideoManager);
    videoMan->doLoadVideo(strfile);
    QThread::msleep(300);
    videoMan->doStartVideo();*/

}

bool IsstopClicked = false;
void servermanager::on_btnmStop_clicked()
{
    IsstopClicked = true;
    LandscapeMgr* lmgr = GETSTELMODULE(LandscapeMgr);
    //#ifndef SHIRAPLAYER_PRE
    //if (StelMainWindow::getInstance().getIsMultiprojector())
    //{
    //core->ALLFader = true;
    //}
    //#endif
    //m_timer.stop();

    //QDateTime timedelay = QDateTime::currentDateTime().addSecs(2);

    //Client
    StelApp::getInstance().getRsync()->sendChanges(RSYNC_COMMAND_STOP_GRAPH,"");

    //#ifndef SHIRAPLAYER_PRE
    if (StelMainWindow::getInstance().getIsMultiprojector())
    {
        //Server
        //#ifndef HAKONIWA
        //    Gecikme(timedelay.toString("dd.MM.yyyy hh:mm:ss"));
        //#endif
        ///
        //StelApp::getInstance().isVideoMode = false;
        /* core->ALLFader = false;

        lmgr->doClearLandscapeVideo();

        lmgr->setCurrentLandscapeName(sLandscape,false,old_isVideoLandscape);// doSetCurrentLandscapeName(sLandscape);*/
        core->allFaderColor = Vec3f(0,0,0);
        core->ALLFader.setDuration(2000);
        core->ALLFader = true;

        stopGraphTimer->start();
    }
    //#endif

    currentState = Stopped;
    setMediaButtonDurum();
    //ui->chUseShaders->setEnabled(true);
    //ui->chSkipNextFrame->setEnabled(true);

    //Client'? tetiklemek i?in
    tetiklemeTimer->start();

    if (StelMainWindow::getInstance().getIsMultiprojector())
    {
        //todo
        //if(is_audio)
        //    Stop_Audio();
        //todo
        lmgr->vop_curr->disconnect(SIGNAL(finished()));
    }

}

void servermanager::on_btnmPause_clicked()
{
    LandscapeMgr* lmgr = GETSTELMODULE(LandscapeMgr);
    lmgr->doPauseVideo(currentState==Running?true:false);

    StelApp::getInstance().getRsync()->sendChanges(RSYNC_COMMAND_PAUSE_GRAPH,currentState==Running?"1":"0");

    currentState==Running?ui->btnmPause->setText(q_("Continue")):ui->btnmPause->setText(q_("Pause"));

    currentState==Running?currentState=Paused:currentState=Running;
    setMediaButtonDurum();
    //todo
    //if(is_audio)
    //    toggle_pause();
}

void servermanager::tracktime()
{
    if (StelMainWindow::getInstance().getIsMultiprojector())
    {
        LandscapeMgr* lmgr = GETSTELMODULE(LandscapeMgr);
        if(lmgr->vop_curr!=NULL)
        {
            ui->trackMedia->setMaximum(lmgr->vop_curr->GetVideoDuration());
            ui->trackMedia->setValue(lmgr->vop_curr->GetCurrentFrameNumber());
            ui->labelProperties->setText(QString("Current Time: %1 \nTotal Time: %2")
                                         .arg(lmgr->vop_curr->GetTimeStrFromFrame(lmgr->vop_curr->GetCurrentFrameNumber()))
                                         .arg(lmgr->vop_curr->GetTimeStrFromFrame(lmgr->vop_curr->GetVideoDuration())));
        }
    }
    else
        loadFromSharedMem();

}

QString oldCommand ="";
QString currentVideoFrameRate ="";
void servermanager::loadFromSharedMem()
{
    if (!videoSharedMem.attach())
    {
        return;
    }

    QBuffer buffer;
    QDataStream in(&buffer);
    QString text;

    videoSharedMem.lock();
    buffer.setData((char*)videoSharedMem.constData(), videoSharedMem.size());
    buffer.open(QBuffer::ReadOnly);
    in >> text;


    QBuffer bufferClear;
    bufferClear.open(QBuffer::ReadWrite);
    QDataStream out(&bufferClear);
    out << "";
    int size = bufferClear.size();

    //--clear data---
    char *to = (char*)videoSharedMem.data();
    const char *from = bufferClear.data().data();
    memcpy(to, from, qMin(videoSharedMem.size(), size));
    //----

    videoSharedMem.unlock();
    videoSharedMem.detach();

    if(text != "")
    {
        QStringList datalist = text.split("@");
        // QMessageBox::information(0,"",datalist[3],0,0);
        if(datalist.count() == 6)
        {
            //qDebug()<<"Video geri Komut 0:"<<datalist[0];
            //if (oldCommand == "STOP")
            {
                if(datalist[0] == "STOP")
                {
                    oldCommand = "";
                    //QMessageBox::information(0,"",datalist[0],0,0);

                    if (!IsstopClicked)
                    {
                        qDebug()<<"oto stop click";
                        on_btnmStop_clicked();
                    }

                    IsstopClicked = false;
                    m_timer.stop();
                }
                else if(datalist[0] == "WARNING")
                {
                    EmbedAudioWarning* emb = new EmbedAudioWarning(0,datalist[1]);
                    emb->show();
                }
            }
            //if(datalist[0] == "STOP")
            //    oldCommand = datalist[0];

            ui->trackMedia->setMaximum(QString(datalist[3]).toInt());
            ui->trackMedia->setValue(QString(datalist[2]).toInt());
            currentVideoFrameRate = datalist[4];

            ui->labelProperties->setText(QString(q_("Current Time: %1 \nTotal Time: %2 Video FPS: %3"))
                                         .arg(datalist[0])
                    .arg(datalist[1])
                    .arg(datalist[4]));

            ui->progressBuffer->setMaximum(currentVideoFrameRate.toInt());
            ui->progressBuffer->setValue(datalist[5].toInt());

            //qDebug()<<"Free buffer count"<<datalist[5];
        }

    }

}
void servermanager::setMediaButtonDurum()
{
    //qDebug()<<currentState;
    if(currentState == Running)
    {
        ui->btnmStop->setEnabled(true);
        ui->btnmPause->setEnabled(true);
        ui->btnmStart->setEnabled(false);
    }
    else if ( currentState == Stopped)
    {
        ui->btnmStart->setEnabled(true);
        ui->btnmPause->setEnabled(false);
        ui->btnmStop->setEnabled(false);
    }
    ui->btnmStart->setChecked(currentState == Running);
    ui->btnmPause->setChecked(currentState == Paused);
}

void servermanager::Gecikme(QString strdt)
{
    while (QDateTime::currentDateTime().toString("dd.MM.yyyy hh:mm:ss")!= strdt)
    {
        //Gecikme
        QCoreApplication::processEvents(QEventLoop::AllEvents );
    }

}

bool servermanager::checkTimeServer()
{
    QHostInfo hostinfo = QHostInfo::fromName(QHostInfo::localHostName());
    QString m_serverAddress;

    for(int i = 0;i<hostinfo.addresses().count();i++)
    {
        QString str = hostinfo.addresses()[i].toString();
        if(!str.contains(':',Qt::CaseInsensitive))
        {
            m_serverAddress = str;
            break;
        }
    }

    sntpclient* s = new sntpclient(m_serverAddress,false);
    connect(s, SIGNAL(validserver()), this, SLOT(doValidTimeServer()));
    s->connectserver();

    return true;
}
void servermanager::doValidTimeServer()
{
    //QMessageBox::critical(0,"","",0,0);
    ui->lblTimeServer->setStyleSheet("background: rgb(0, 255, 0)");
}

bool servermanager::checkFtpServer()
{
    QHostInfo hostinfo = QHostInfo::fromName(QHostInfo::localHostName());
    QString m_serverAddress;

    for(int i = 0;i<hostinfo.addresses().count();i++)
    {
        QString str = hostinfo.addresses()[i].toString();
        if(!str.contains(':',Qt::CaseInsensitive))
        {
            m_serverAddress = str;
            break;
        }
    }
}

void servermanager::ftpCommandFinished(int i,bool error)
{
}

void servermanager::on_btnLoadTerminal_clicked()
{
    if((QMessageBox::information(0,q_("Information"),
                                 q_("Do you want to send media file to all terminals?"),
                                 QMessageBox::Yes,QMessageBox::No))== QMessageBox::No)
    {
        return;
    }

    switch (ui->stackedWidget->currentIndex() )
    {
    case 1:
        if(ui->listFilesWidget->selectedItems().count()==0)
        {
            QMessageBox::warning(0,q_("Warning"),q_("Please select a media file!"),0,0);
            return;
        }
        break;
    case 2:
        if(ui->listFrameFileWidget->selectedItems().count()==0)
        {
            QMessageBox::warning(0,q_("Warning"),q_("Please select a frame file!"),0,0);
            return;
        }
        break;
    }

    QHostInfo hostinfo = QHostInfo::fromName(QHostInfo::localHostName());
    QString m_serverAddress;

    for(int i = 0;i<hostinfo.addresses().count();i++)
    {
        QString str = hostinfo.addresses()[i].toString();
        if(!str.contains(':',Qt::CaseInsensitive))
        {
            m_serverAddress = str;
            break;
        }
    }

    //ftpserver,filename,user,pass

    QString filename;

    switch (ui->stackedWidget->currentIndex() )
    {
    case 1:
        filename = ui->listFilesWidget->selectedItems()[0]->text();
        break;
    case 2:
        filename = ui->listFrameFileWidget->selectedItems()[0]->text();
        break;
    }

    QString user = conf->value("main/ftpuser", "user").toString();
    QString pass = conf->value("main/ftppass", "pass").toString();

    StelApp::getInstance().getRsync()->sendChanges(RSYNC_COMMAND_FTP,
                                                   QString("%0").arg(ui->stackedWidget->currentIndex())+"@"+
                                                   m_serverAddress+"@"+
                                                   filename+"@"+
                                                   user+"@"+
                                                   pass);

}

void servermanager::on_tbarSes_changed(int val)
{
    if(is_audio)
    {
        //todo
        //set_volume(val);
        StelApp::getInstance().getRsync()->sendChanges(RSYNC_COMMAND_VOLUME,QString("%0").arg(val));
    }
}

void servermanager::on_frameselectedChange()
{

}

void servermanager::on_listframes_DoubleClicked(QListWidgetItem* item)
{
    //QMessageBox::information(0,0,"",ui->listFrameFileWidget->selectedItems()[0]->data(Qt::WhatsThisRole).toString(),0);

    if(currentState != Stopped)
        return;

    filepath = ui->listFrameFileWidget->selectedItems()[0]->data(Qt::WhatsThisRole).toString();

    if(filepath !="")
    {
        LandscapeMgr* lmgr = GETSTELMODULE(LandscapeMgr);
        //?u anki landscape haf?zaya al?n?yor
        sLandscape = lmgr->getCurrentLandscapeName();
        old_isVideoLandscape = lmgr->isVideoLandscape;
        if(lmgr->s_Name != "")
            lmgr->doClearLandscapeVideo();

        //QString framepath = media_path+"/panorama/"+ui->listFrameFileWidget->selectedItems()[0]->text();
        //if(StelMainWindow::getInstance().getIsMultiprojector())
        {
            core->allFaderColor = Vec3f(0,0,0);
            core->ALLFader.setDuration(ui->spFadeDuration->value()*1000);
            core->ALLFader = true;
            showFisheyeFrameTimer->setInterval(core->ALLFader.getDuration());
            if (oldViewDirection == Vec3d(0,0,0))
                oldViewDirection = core->getNavigator()->j2000ToAltAz(core->getMovementMgr()->getViewDirectionJ2000());
            //oldViewDirection = core->getMovementMgr()->getViewDirectionJ2000();
            showFisheyeFrameTimer->start();
        }
        //else
        //lmgr->doSetCurrentLandscapetoFrame(filepath,sLandscape,ui->chDaylight->isChecked(),1);

        StelApp::getInstance().getRsync()->sendChanges(RSYNC_COMMAND_SHOW_FRAME, filepath+";"+QString("%0;%1;%2").arg(ui->chDaylight->isChecked())
                                                       .arg(ui->chCrossFade->isChecked())
                                                       .arg(ui->spFadeDuration->value()));
    }
}
void servermanager::on_chDaylight_toggled(bool checked)
{
    //QMessageBox::information(0,"",QString("%0").arg(checked),0,0);
    LandscapeMgr* lmgr = GETSTELMODULE(LandscapeMgr);
    lmgr->setshowDaylight(checked);
    StelApp::getInstance().getRsync()->sendChanges(RSYNC_COMMAND_SHOW_FRAME_DAYLIGHT,QString("%0").arg(checked));
    conf->setValue("video/frame_withdaylight", checked);
}
void servermanager::on_clearFrame_clicked()
{
    if(currentState != Stopped)
        return;

    if(sLandscape!="")
    {
        StelGui* sgui = dynamic_cast<StelGui*>(StelApp::getInstance().getGui());

        /*LandscapeMgr* lmgr = GETSTELMODULE(LandscapeMgr);
        lmgr->doSetCurrentLandscapeName(sLandscape);*/

        bFlagLandscape = sgui->viewDialog.ui->showGroundCheckBox->isChecked();
        core->allFaderColor = Vec3f(0,0,0);
        core->ALLFader.setDuration(ui->spFadeDuration->value()*1000);
        core->ALLFader = true;
        clearFramesTimer->setInterval(core->ALLFader.getDuration());
        clearFramesTimer->start();

        StelApp::getInstance().getRsync()->sendChanges(RSYNC_COMMAND_SHOW_CLEAR,
                                                       QString("%0;%1").arg(ui->spFadeDuration->value())
                                                       .arg(sgui->viewDialog.ui->showGroundCheckBox->isChecked()));
        //sLandscape = "";
    }
}

void servermanager::on_TermUpdate_clicked()
{
    if((QMessageBox::information(0,q_("Information"),
                                 q_("Do you want to send program update files to all terminals?"),
                                 QMessageBox::Yes,QMessageBox::No))== QMessageBox::No)
    {
        return;
    }

    QHostInfo hostinfo = QHostInfo::fromName(QHostInfo::localHostName());
    QString m_serverAddress;

    for(int i = 0;i<hostinfo.addresses().count();i++)
    {
        QString str = hostinfo.addresses()[i].toString();
        if(!str.contains(':',Qt::CaseInsensitive))
        {
            m_serverAddress = str;
            break;
        }
    }


    //ftpserver,filename,user,pass

    //QString filename = "shiraplayer.exe";

    QString user = conf->value("main/ftp_upd_user", "user").toString();
    QString pass = conf->value("main/ftp_upd_pass", "pass").toString();

    StelApp::getInstance().getRsync()->sendChanges(RSYNC_COMMAND_FTP_UPDPRG,
                                                   m_serverAddress+"@"+
                                                   //filename+"@"+
                                                   user+"@"+
                                                   pass);

}
void servermanager::on_TermRefresh_clicked()
{
    if((QMessageBox::information(0,q_("Information"),
                                 q_("Do you want to restart program on all terminals?"),
                                 QMessageBox::Yes,QMessageBox::No))== QMessageBox::No)
    {
        return;
    }
    StelApp::getInstance().getRsync()->sendChanges(RSYNC_COMMAND_RESTARTAPP,"");

}

void servermanager::on_TermClose_clicked()
{
    if((QMessageBox::information(0,q_("Information"),
                                 q_("Do you want to shutdown all terminals?"),
                                 QMessageBox::Yes,QMessageBox::No))== QMessageBox::No)
    {
        return;
    }
    StelApp::getInstance().getRsync()->sendChanges(RSYNC_COMMAND_SHUTDOWN_PC,"");

}
void servermanager::on_TermOpen_clicked()
{
    for(int i=1;i<=10;i++)
    {
        StelApp::getInstance().getRsync()->sendOpenTerminals(conf->value("main/p"+QString("%0").arg(i)+"Mac","FFFFFFFFFFFF").toString());
    }
}

void servermanager::on_projOn_clicked()
{
    StelApp::getInstance().getRsync()->sendChanges(RSYNC_COMMAND_PROJECTOR,"ON");
}

void servermanager::on_projOff_clicked()
{
    if((QMessageBox::information(0,q_("Information"),
                                 q_("Do you want to shutdown all projectors?"),
                                 QMessageBox::Yes,QMessageBox::No))== QMessageBox::No)
    {
        return;
    }
    StelApp::getInstance().getRsync()->sendChanges(RSYNC_COMMAND_PROJECTOR,"OFF");
}

void servermanager::on_projblank_clicked()
{
    StelApp::getInstance().getRsync()->sendChanges(RSYNC_COMMAND_PROJECTOR,"BLANK");
}


void servermanager::on_projdisplay_clicked()
{
    StelApp::getInstance().getRsync()->sendChanges(RSYNC_COMMAND_PROJECTOR,"DISPLAY");
}

void servermanager::scriptEnded()
{
    ui->btnApplyFavorits->setEnabled(true);
    ui->btnStopFavorit->setEnabled(false);
}

void servermanager::btnStopFavoritClicked()
{
    StelMainGraphicsView::getInstance().getScriptMgr().stopScript();
    StelApp::getInstance().getRsync()->sendChanges(RSYNC_COMMAND_LIVE_SCRIPT,"");
}


void servermanager::setupPresenterWidgets()
{
    QGroupBox * lblCaption = new QGroupBox(ui->page_5);
    lblCaption->setTitle(q_("Flat Media Library : Drag and drop into the Active Images window"));
    QVBoxLayout * vLayout = new QVBoxLayout(lblCaption);
    vLayout->setMargin(0);

    defaultPropWidget = new QImagePropWidget(ui->page_5,flat_media_path);
    mediaBrowserList = new QMediaBrowserWidget(ui->page_5,flat_media_path);
    playImageWidget = new QPlayImageWidget(ui->page_5);
    //QLabel* lblFlatPath = new QLabel(ui->page_5);
    //lblFlatPath->setText("Flat Media Path: " +flat_media_path);
    //ui->preshorizontalLayout->addWidget(lblFlatPath);
    ui->preshorizontalLayout->addWidget(defaultPropWidget);
    ui->preshorizontalLayout->addWidget(lblCaption);

    vLayout->addWidget(mediaBrowserList);

    //ui->preshorizontalLayout->addWidget(mediaBrowserList);
    ui->preshorizontalLayout->addWidget(playImageWidget);

    defaultPropWidget->setCaption(q_("Default Display Properties"));
    defaultPropWidget->setCloseVisible(false);
    defaultPropWidget->hide();
}
void servermanager::currentMediaItemChanged( QListWidgetItem * current, QListWidgetItem * previous)
{
    defaultPropWidget->setImage(current->data(Qt::UserRole).value<QPixmap>(),current->data(Qt::UserRole+2).toString());
}
void servermanager::btnLisansClick()
{
    frm.setVisible(true);
    frm.getProxy()->setPos(this->getProxy()->pos());
}

void servermanager::on_btnQuit_clicked()
{
    QMessageBox msgBox;
    msgBox.setWindowTitle(q_("Warning!"));
    msgBox.setText(q_("Are you sure want to quit?"));
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    //msgBox.setDefaultButton(QMessageBox::Save);
    //msgBox.setWindowModality(Qt::WindowModality());
    msgBox.setWindowFlags(Qt::WindowStaysOnTopHint);
    msgBox.setIcon(QMessageBox::Warning);
    int ret = msgBox.exec();

    if(ret ==QMessageBox::Yes)
        StelApp::getInstance().getGui()->quitAll();
}
void servermanager::on_btnNightView_clicked(bool checked)
{
    StelApp::getInstance().setVisionModeNight(checked);
}

QPixmap servermanager::openImage(const QString &path)
{
    QString fileName = path;
    QPixmap newImage;

    if (!fileName.isEmpty()) {

        if (!newImage.load(fileName)) {
            return newImage;
        }
    }
    return newImage;
}
void servermanager::on_btnPLoad_clicked()
{
    presentFileName = QFileDialog::getOpenFileName(0,
                                                   q_("Presentation Files(*.xml)"));
    ui->textEditPresent->setText(presentFileName);
    if(presentFileName!="")
    {
        QFile file(presentFileName);
        if (file.open(QIODevice::ReadOnly))
        {
            QDomDocument doc;
            doc.setContent(&file);
            file.close();

            QDomElement main;
            main = doc.documentElement();
            if( doc.firstChild().nodeName() != "ShiraPlayerFlatPresentations")
            {
                QMessageBox::critical(0,q_("Error"),q_("Wrong ShiraPlayer Flat Present project file!"),0,0);
                return;
            }

            // Initialising channels
            QDomElement FlatItemelement = main.firstChildElement("FlatItem");
            while (!FlatItemelement.isNull())
            {
                QString str =initFromDOMElement(FlatItemelement);
                QStringList data = str.split("@");

                StelApp::getInstance().presentdomeORsky = QString(data[3]).toInt();
                StelApp::getInstance().presentRA =QString(data[4]).toInt();
                StelApp::getInstance().presentDec = QString(data[5]).toInt();
                StelApp::getInstance().presentSize = QString(data[6]).toInt()*100;
                StelApp::getInstance().presentRotate = QString(data[7]).toInt();
                StelApp::getInstance().presentVisible = QString(data[8]).toInt();
                StelApp::getInstance().presentCon = QString(data[10]).toInt();
                StelApp::getInstance().presentBri = QString(data[11]).toInt();
                StelApp::getInstance().presentSat = QString(data[12]).toInt();

                QImagePropWidget* imgpropWidget = new QImagePropWidget(playImageWidget->stackedWidget,data[1],data[0],QString(data[2]).toInt());

                //--image
                int m_PieceSize = 75;
                QPixmap px;
                if(QString(data[2]).toInt())
                    px = QIcon(":/media/gui/presenter/media.png").pixmap(96,96);
                else
                    px= openImage(data[1] + data[0]);

                if(px.width()>m_PieceSize )
                {
                    int scalerate = px.width() / m_PieceSize ;
                    px =px.scaled(px.width()/scalerate, px.height()/scalerate, Qt::IgnoreAspectRatio, Qt::FastTransformation);
                }
                imgpropWidget->setImage(px,data[0]);

                QListWidgetItem *item = new QListWidgetItem;
                item->setIcon(QIcon( QPixmap::fromImage(imgpropWidget->getPixmap()->toImage()) ));
                item->setData(Qt::DisplayRole, imgpropWidget->getCaption());
                item->setData(Qt::WhatsThisRole,playImageWidget->stackListWidget->count() );
                playImageWidget->stackListWidget->addItem(item);

                playImageWidget->stackedWidget->insertWidget(item->data(Qt::WhatsThisRole).toInt(),imgpropWidget);
                playImageWidget->stackedWidget->setCurrentIndex(item->data(Qt::WhatsThisRole).toInt());

                //imgpropWidget->on_sliderSat_sliderMoved(StelApp::getInstance().presentSat);
                FlatItemelement = FlatItemelement.nextSiblingElement("FlatItem");
                Sleep(200);
            }
        }

    }
}
QString servermanager::initFromDOMElement(const QDomElement& element)
{
    if (!element.isNull())
    {
        return QString("%1@%2@%3@%4@%5@%6@%7@%8@%9@%10@%11@%12@%13")
                .arg(element.attribute("filename"))
                .arg(element.attribute("path"))
                .arg(element.attribute("IsVideo"))
                .arg(element.attribute("domeORsky"))
                .arg(element.attribute("Alt"))
                .arg(element.attribute("Azi"))
                .arg(element.attribute("Size"))
                .arg(element.attribute("Rotate"))
                .arg(element.attribute("Visible"))
                .arg(element.attribute("IsVideo"))
                .arg(element.attribute("Contrast"))
                .arg(element.attribute("Brightness"))
                .arg(element.attribute("Saturation"));
    }
}
void servermanager::on_btnPSave_clicked()
{
    int count = playImageWidget->stackedWidget->count();
    if (count == 0) return;

    if(QMessageBox::information(0,q_("Warning!"),q_("Are you sure want to save available Presentations ?"),QMessageBox::Yes,QMessageBox::No)== QMessageBox::Yes)
    {
        if (presentFileName == "")
            presentFileName = qApp->applicationDirPath()+"\\untitled.xml";

        presentFileName = QFileDialog::getSaveFileName(0,
                                                       tr("Save Presentation File"),
                                                       presentFileName,
                                                       tr("Presentation Files(*.xml)"));
        ui->textEditPresent->setText(presentFileName);

        QFile file(presentFileName);
        if (file.open(QIODevice::WriteOnly|QIODevice::Truncate))
        {
            QTextStream out(&file);
            QDomDocument doc;

            QDomElement main = doc.createElement("ShiraPlayerFlatPresentations");
            main.setAttribute("version", 1.4);
            doc.appendChild(main);

            for (unsigned int i=0; i < playImageWidget->stackedWidget->count(); ++i)
            {

                QImagePropWidget* item = dynamic_cast <QImagePropWidget*>(playImageWidget->stackedWidget->widget(i));
                main.appendChild( item->domElement("FlatItem",doc));
            }

            doc.save(out, 4);
            file.flush();
            file.close();
        }
    }
}

void servermanager::on_textEditPresent_textChanged()
{
    presentFileName = ui->textEditPresent->text();
}

void servermanager::on_btnPNew_clicked()
{
    int count = playImageWidget->stackedWidget->count();

    if (count>0)
    {
        int ret = QMessageBox::warning(0,q_("Warning!"),q_("Active Images not saved.\nDo you want to save as a Presentation?"),QMessageBox::Save | QMessageBox::Discard
                                       | QMessageBox::Cancel,QMessageBox::Save);
        if (ret == QMessageBox::Save )
            on_btnPSave_clicked();
        else if (ret == QMessageBox::Cancel)
            return;
    }

    for (unsigned int i=0; i < count ; ++i)
    {
        QImagePropWidget* item = dynamic_cast <QImagePropWidget*>(playImageWidget->stackedWidget->widget(0));
        item->btnCloseToggled(false);
        Sleep(200);
    }

    ui->textEditPresent->setText("");
}

void servermanager::on_chShowFPS_stateChanged(int value)
{
    StelApp::getInstance().addNetworkCommand(QString("core.showFPS(%0);").arg(value));
    conf->setValue("video/show_fps", value);
}
void servermanager::on_chUseShaders_stateChanged(int value )
{
    StelApp::getInstance().getRsync()->sendChanges(RSYNC_COMMAND_GRAPH_PERFORMANCE,QString("%0").arg(value));
    conf->setValue("main/use_glshaders", value);
}

void servermanager::on_chSkipNextFrame_stateChanged(int value)
{
    StelApp::getInstance().getRsync()->sendChanges(RSYNC_COMMAND_GRAPH_SKIPNEXTFRAME,QString("%0").arg(value));
    conf->setValue("video/skipnextframe",value);
}

void servermanager::on_chpreWarped_stateChanged(int value)
{
    conf->setValue("video/prewarped", value);
}

void servermanager::on_chCrossFade_stateChanged(int value)
{
    conf->setValue("video/crossfade", value);
}

void servermanager::on_spFadeDuration_valueChanged(int value)
{
    conf->setValue("video/fadeduration", value);
}
void servermanager::setDateTimeGui(double newJd)
{
    stellaManager->dateTimeDialog.setDateTime(newJd);
}

void servermanager::on_trackMedia_sliderPressed()
{
    if (currentState == Stopped) return;
    m_timer.stop();
}

void servermanager::on_trackMedia_sliderReleased()
{
    if (currentState == Stopped) return;

    LandscapeMgr* lmgr = GETSTELMODULE(LandscapeMgr);

    if(StelMainWindow::getInstance().getIsMultiprojector())
    {
        int64_t pos = (int64_t)ui->trackMedia->value();


        if(lmgr->vop_curr)
            lmgr->vop_curr->SeekByFrame(pos);

        /*if(is_audio)
        {
            audioSeekToPosition(pos,currentVideoFrameRate.toDouble());
        }*/
    }
    else
        StelApp::getInstance().getRsync()->sendChanges(RSYNC_COMMAND_SEEK_GRAPH,
                                                       QString("%0@%1").arg(ui->trackMedia->value())
                                                       .arg(currentVideoFrameRate));

    m_timer.start();
}

void servermanager::tetiklemeTimerProc()
{
    StelApp::getInstance().addNetworkCommand(QString("core.showFPS(%0);").arg(core->showFPS));
    ui->trackMedia->setValue(0);
}

void servermanager::on_btnSetFreeHand_clicked()
{
    StelApp::getInstance().setAllowFreeHand(true);
    StelApp::getInstance().setAllowFreeHandDel(false);
    ui->btnDeleteFreeHand->setChecked(false);
    ui->btnSelectFreeHand->setChecked(false);

    ui->btnSetFreeHand->setChecked(true);
}
void servermanager::on_btnDeleteFreeHand_clicked()
{
    StelApp::getInstance().setAllowFreeHandDel(true);
    StelApp::getInstance().setAllowFreeHand(false);
    ui->btnSetFreeHand->setChecked(false);
    ui->btnSelectFreeHand->setChecked(false);

    ui->btnDeleteFreeHand->setChecked(true);
}
void servermanager::on_btnSelectFreeHand_clicked()
{
    StelApp::getInstance().setAllowFreeHandDel(false);
    StelApp::getInstance().setAllowFreeHand(false);
    ui->btnSetFreeHand->setChecked(false);
    ui->btnDeleteFreeHand->setChecked(false);

    ui->btnSelectFreeHand->setChecked(true);
}

void servermanager::on_btnClearFreeHand_clicked()
{
    QMessageBox msgBox;
    msgBox.setWindowTitle(q_("Warning!"));
    msgBox.setText(q_("Clear All?"));
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::Save);
    msgBox.setWindowModality(Qt::WindowModality());
    msgBox.setIcon(QMessageBox::Warning);
    int ret = msgBox.exec();

    if(ret ==QMessageBox::Yes)
    {
        core->freehandItems.clear();
        core->freehandPrevScreenItems.clear();
        StelApp::getInstance().getRsync()->sendChanges(RSYNC_COMMAND_DRAWFREE_DELETEALL,"");
    }

}
