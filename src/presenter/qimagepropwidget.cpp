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
#include "ui_qimagepropwidget.h"
#include "qimagepropwidget.h"
#include "StelMainWindow.hpp"
#include "StelApp.hpp"
#include "StelModuleMgr.hpp"
#include "StelModule.hpp"
#include "StelPresentMgr.hpp"
#include "StelTranslator.hpp"
#include "videoutils/audioclass.h"

QImagePropWidget::QImagePropWidget(QWidget *parent,QString path,QString filename,bool aisVideo)
    : QWidget(parent),presentfilename(filename),isVideo(aisVideo)
{
    parentWidget = parent;
    //todo
    vsource = NULL;
    vfile = NULL;

    ui = new Ui_QImagePropWidget;
    ui->setupUi(this);

    sPmgr = GETSTELMODULE(StelPresentMgr);
    if(presentfilename != "")
    {
        presentRA = StelApp::getInstance().presentRA;
        presentDec =StelApp::getInstance().presentDec;
        presentRotate = StelApp::getInstance().presentRotate;
        presentSize =StelApp::getInstance().presentSize;
        presentdomeORsky = StelApp::getInstance().presentdomeORsky;
        presentVisible= StelApp::getInstance().presentVisible;
        presentCon = StelApp::getInstance().presentCon;
        presentBri = StelApp::getInstance().presentBri;
        presentSat = StelApp::getInstance().presentSat;

        ui->RASlider->setValue(presentRA);
        ui->decSlider->setValue(presentDec);
        ui->rotSlider->setValue(presentRotate);
        ui->sizeSlider->setValue(presentSize/100);
        ui->rdOnDome->setChecked(presentdomeORsky);
        ui->rdOnSky->setChecked(!presentdomeORsky);
        ui->btnFade->setChecked(presentVisible);

        ui->sliderContrast->setValue(presentCon);
        ui->sliderBrightness->setValue(presentBri);
        ui->sliderSat->setValue(presentSat);

        presentPath = path;
        StelApp::getInstance().presentID++;
        presentID = StelApp::getInstance().presentID;

        if(isVideo)
        {
            StelApp::getInstance().setUseGLShaders(false);
            //todo
            vfile= new VideoClass();
            vfile->setFlatVideo(true);
            //vfile->useYUVData = false;
            //vfile->setMarkIn(0);
            //vfile->setOptionRestartToMarkIn(false);
            vfile->OpenVideo(presentPath+presentfilename);

            aspectratio = vfile->GetAspectRatio();
            sPmgr->loadPresentImage(QString::number(presentID),"",presentRA,presentDec,
                                    presentSize,presentRotate,2.5,14.0,presentVisible,presentdomeORsky,aspectratio);

            sPmgr->setMixWithSky(QString::number(presentID),ui->chMix->isChecked());

            QString str = QString("%1;%2;%3;%4;%5;%6;%7;%8;%9;%10;%11;%12;%13;%14")
                    .arg(presentID)
                    .arg("")
                    .arg(presentRA)
                    .arg(presentDec)
                    .arg(presentSize)
                    .arg(presentRotate)
                    .arg(2.5)
                    .arg(14.0)
                    .arg(false)
                    .arg(presentdomeORsky)
                    .arg(aspectratio)
                    .arg(isVideo)
                    .arg(presentPath+presentfilename)
                    .arg(ui->chMix->isChecked());
            StelApp::getInstance().getRsync()->sendChanges(RSYNC_COMMAND_LOADPRESENTIMAGE,str);
            if(presentVisible)
            {
                Sleep(100);
                str = "core.showPresentImage('"+QString::number(presentID)+"',"+QString("%1").arg(presentVisible)+");";
                StelApp::getInstance().addNetworkCommand(str);
            }

            refreshTimingTimer = new QTimer(this);
            Q_CHECK_PTR(refreshTimingTimer);
            refreshTimingTimer->setInterval(150);
            QObject::connect(refreshTimingTimer, SIGNAL(timeout()), this, SLOT(refreshTiming()));
            //QObject::connect(vfile, SIGNAL(running(bool)), this, SLOT(updateRefreshTimerState()));
            if(!StelMainWindow::getInstance().getIsMultiprojector())
                if(StelMainWindow::getInstance().isServer)
                    videoSharedMem.setKey("videopresentSharedMemory"+QString::number(presentID));
        }
        else
        {
            QPixmap ppx(presentPath+ presentfilename);
            aspectratio = (double)ppx.width()/(double)ppx.height();
            ppx = QPixmap();
            sPmgr->loadPresentImage(QString::number(presentID),presentPath+ presentfilename,presentRA,presentDec,
                                    presentSize,presentRotate,2.5,14.0,presentVisible,presentdomeORsky,aspectratio);

            sPmgr->setMixWithSky(QString::number(presentID),ui->chMix->isChecked());

            QString str = QString("%1;%2;%3;%4;%5;%6;%7;%8;%9;%10;%11;%12;%13;%14")
                    .arg(presentID)
                    .arg(presentPath+ presentfilename)
                    .arg(presentRA)
                    .arg(presentDec)
                    .arg(presentSize)
                    .arg(presentRotate)
                    .arg(2.5)
                    .arg(14.0)
                    .arg(false)
                    .arg(presentdomeORsky)
                    .arg(aspectratio)
                    .arg(isVideo)
                    .arg(presentPath+presentfilename)
                    .arg(ui->chMix->isChecked());
            StelApp::getInstance().getRsync()->sendChanges(RSYNC_COMMAND_LOADPRESENTIMAGE,str);
            if(presentVisible)
            {
                Sleep(100);
                str = "core.showPresentImage('"+QString::number(presentID)+"',"+QString("%1").arg(presentVisible)+");";
                StelApp::getInstance().addNetworkCommand(str);
            }

        }

        ui->listTab->setVisible(isVideo);
        ui->lblFlatPath->setVisible(false);
        ui->labelPath->setVisible(false);

        ///tekrar eden kodlar
        connect(ui->RASlider,SIGNAL(valueChanged(int)),this,SLOT(RAsliderValueChange(int)));
        connect(ui->decSlider,SIGNAL(valueChanged(int)),this,SLOT(DecsliderValueChange(int)));
        connect(ui->rotSlider,SIGNAL(valueChanged(int)),this,SLOT(RotsliderValueChange(int)));
        connect(ui->sizeSlider,SIGNAL(valueChanged(int)),this,SLOT(SizesliderValueChange(int)));
        connect(ui->rdOnDome,SIGNAL(toggled(bool)),this,SLOT(domeORskyToggled(bool)));
        connect(ui->rdOnSky,SIGNAL(toggled(bool)),this,SLOT(domeORskyToggled(bool)));
        connect(ui->btnFade,SIGNAL(toggled(bool)),this,SLOT(FadeToggled(bool)));
        connect(ui->btnClose,SIGNAL(clicked(bool)),this,SLOT(btnCloseToggled(bool)));
        connect(ui->listTab,SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)),this,SLOT(tabListItemChanged(QListWidgetItem*,QListWidgetItem*)));

        ui->stackedWidget->setCurrentIndex(0);
        ui->listTab->setCurrentRow(0);

        //connect(ui->startButton,SIGNAL(clicked(bool)),this,SLOT(PlayVideo(bool)));
        //connect(ui->btnAudio,SIGNAL(clicked(bool)),this,SLOT(StartAudio(bool)));
        //connect(ui->btnAudio,SIGNAL(clicked(bool)),this,SLOT(Test(bool)));

        //simdilik Kapalýlar
        ui->markInButton->setVisible(false);
        ui->markOutButton->setVisible(false);
        ui->markInSlider->setVisible(false);
        ui->markOutSlider->setVisible(false);
        ui->resetMarkInButton->setVisible(false);
        ui->resetMarkOutButton->setVisible(false);

        connect(ui->RASlider,SIGNAL(sliderReleased()),this,SLOT(sliderRelease()));
        connect(ui->decSlider,SIGNAL(sliderReleased()),this,SLOT(sliderRelease()));
        connect(ui->rotSlider,SIGNAL(sliderReleased()),this,SLOT(sliderRelease()));
        connect(ui->sizeSlider,SIGNAL(sliderReleased()),this,SLOT(sliderRelease()));

    }
    else
    {
        ui->listTab->setVisible(false);
        ui->lblFlatPath->setVisible(true);
        ui->lblFlatPath->setText(path);
        ui->labelPath->setVisible(true);

        StelApp::getInstance().presentRA =ui->RASlider->value();
        StelApp::getInstance().presentDec = ui->decSlider->value();
        StelApp::getInstance().presentRotate = ui->rotSlider->value();
        StelApp::getInstance().presentSize = ui->sizeSlider->value()*100;
        StelApp::getInstance().presentdomeORsky = ui->rdOnDome->isChecked();
        StelApp::getInstance().presentVisible = ui->btnFade->isChecked();

        //tekrar eden kodlar
        connect(ui->RASlider,SIGNAL(valueChanged(int)),this,SLOT(RAsliderValueChange(int)));
        connect(ui->decSlider,SIGNAL(valueChanged(int)),this,SLOT(DecsliderValueChange(int)));
        connect(ui->rotSlider,SIGNAL(valueChanged(int)),this,SLOT(RotsliderValueChange(int)));
        connect(ui->sizeSlider,SIGNAL(valueChanged(int)),this,SLOT(SizesliderValueChange(int)));
        connect(ui->rdOnDome,SIGNAL(toggled(bool)),this,SLOT(domeORskyToggled(bool)));
        connect(ui->rdOnSky,SIGNAL(toggled(bool)),this,SLOT(domeORskyToggled(bool)));
        connect(ui->btnFade,SIGNAL(toggled(bool)),this,SLOT(FadeToggled(bool)));
        connect(ui->btnClose,SIGNAL(clicked(bool)),this,SLOT(btnCloseToggled(bool)));
        connect(ui->listTab,SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)),this,SLOT(tabListItemChanged(QListWidgetItem*,QListWidgetItem*)));

        ui->stackedWidget->setCurrentIndex(0);
        ui->listTab->setCurrentRow(0);

        //connect(ui->startButton,SIGNAL(clicked(bool)),this,SLOT(PlayVideo(bool)));
        //connect(ui->btnAudio,SIGNAL(clicked(bool)),this,SLOT(StartAudio(bool)));
        //connect(ui->btnAudio,SIGNAL(clicked(bool)),this,SLOT(Test(bool)));

        //simdilik Kapalýlar
        ui->markInButton->setVisible(false);
        ui->markOutButton->setVisible(false);
        ui->markInSlider->setVisible(false);
        ui->markOutSlider->setVisible(false);
        ui->resetMarkInButton->setVisible(false);
        ui->resetMarkOutButton->setVisible(false);

        connect(ui->RASlider,SIGNAL(sliderReleased()),this,SLOT(sliderRelease()));
        connect(ui->decSlider,SIGNAL(sliderReleased()),this,SLOT(sliderRelease()));
        connect(ui->rotSlider,SIGNAL(sliderReleased()),this,SLOT(sliderRelease()));
        connect(ui->sizeSlider,SIGNAL(sliderReleased()),this,SLOT(sliderRelease()));

    }

    ui->videoLoopButton->setHidden(true);

    languageChanged();

}
QWidget* QImagePropWidget::getGroupBox()
{
    return ui->groupBoxFlat;
}
void QImagePropWidget::sliderRelease()
{
    sendClinet();
}
void QImagePropWidget::RAsliderValueChange(int val)
{
    if(presentfilename == "")
        StelApp::getInstance().presentRA = val;
    else
    {
        presentRA = val;
        sPmgr->setLayerProperties(QString::number(presentID), presentPath+ presentfilename,presentRA,presentDec,presentSize,presentRotate,2.5,14.0,aspectratio,isVideo);
        // sendClinet();
    }
}
void QImagePropWidget::DecsliderValueChange(int val)
{
    if(presentfilename == "")
        StelApp::getInstance().presentDec= val;
    else
    {
        presentDec = val;
        sPmgr->setLayerProperties(QString::number(presentID), presentPath+ presentfilename,presentRA,presentDec,presentSize,presentRotate,2.5,14.0,aspectratio,isVideo);
        //sendClinet();
    }
}
void QImagePropWidget::RotsliderValueChange(int val)
{
    if(presentfilename == "")
        StelApp::getInstance().presentRotate= val;
    else
    {
        presentRotate = val;
        sPmgr->setLayerProperties(QString::number(presentID), presentPath+ presentfilename,presentRA,presentDec,presentSize,presentRotate,2.5,14.0,aspectratio,isVideo);
        // sendClinet();
    }
}
void QImagePropWidget::SizesliderValueChange(int val)
{
    if(presentfilename == "")
        StelApp::getInstance().presentSize= val *100;
    else
    {
        presentSize = val*100;
        sPmgr->setLayerProperties(QString::number(presentID), presentPath+ presentfilename,presentRA,presentDec,presentSize,presentRotate,2.5,14.0,aspectratio,isVideo);
        // sendClinet();
    }
}
void QImagePropWidget::domeORskyToggled(bool checked)
{
    if(presentfilename == "")
    {
        if(ui->rdOnDome->isChecked())
            StelApp::getInstance().presentdomeORsky = true;
        else
            StelApp::getInstance().presentdomeORsky = false;
    }
    else
    {
        sPmgr->showPresentImage(QString::number(presentID),false);
        sPmgr->getPresentImage(QString::number(presentID))->willchangeProject= true;
        //sPmgr->setLayerdomeORsky(QString::number(presentID),ui->rdOnDome->isChecked());Olay tamamlandýðýnda deðiþtirilecek

        QString str = QString("core.setLayerdomeORsky('%1',%2);")
                .arg(presentID)
                .arg(ui->rdOnDome->isChecked());

        //QMessageBox::information(0,"",str,0,0);
        StelApp::getInstance().addNetworkCommand(str);
    }
}
void QImagePropWidget::FadeToggled(bool checked)
{
    if(presentfilename == "")
        StelApp::getInstance().presentVisible=checked;
    else
    {
        sPmgr->showPresentImage(QString::number(presentID),checked);
        QString str = "core.showPresentImage('"+QString::number(presentID)+"',"+QString("%1").arg(checked)+");";
        //QMessageBox::information(0,"",str,0,0);
        StelApp::getInstance().addNetworkCommand(str);
    }
}

void QImagePropWidget::btnCloseToggled(bool checked)
{
    sPmgr->activePresentImageID = QString::number(presentID);
    if (ui->btnFade->isChecked())
    {
        sPmgr->showPresentImage(QString::number(presentID),false);
        QString str = "core.showPresentImage('"+QString::number(presentID)+"',"+QString("%1").arg(false)+");";
        StelApp::getInstance().addNetworkCommand(str);
    }

    //todo
    if(vsource != NULL)
    {
        //Stop_Audio();
        vsource->stop();
        vsource= NULL;
        vfile = NULL;
        delete vfile;
        delete vsource;
        sPmgr->removePresentImage(QString::number(presentID));

        //Client
        StelApp::getInstance().getRsync()->sendChanges(RSYNC_COMMAND_REMOVEPRESENTVIDEO,QString("%1").arg(presentID));
    }

    dynamic_cast<QStackedWidget*>(parentWidget)->removeWidget(this);


}
QImagePropWidget::~QImagePropWidget()
{
    delete refreshTimingTimer;
}
void QImagePropWidget::setImage(QPixmap pix,QString caption)
{
    ui->imgPicture->setPixmap(pix);
    ui->imgFileName->setText(caption);
}
void QImagePropWidget::setCaption(QString caption)
{
    ui->groupBoxCaption->setTitle(caption);
}
void QImagePropWidget::setCloseVisible(bool visible)
{
    ui->btnClose->setVisible(visible);
}

void QImagePropWidget::tabListItemChanged(QListWidgetItem* item,QListWidgetItem* previtem)
{
    ui->stackedWidget->setCurrentIndex(ui->listTab->currentRow());
}

void QImagePropWidget::StartAudio(bool checked)
{
    /*if(vsource!= NULL)
    {
        if(checked)
        {
            Stop_Audio();
            Load_Audio(QString(presentPath+presentfilename).toLatin1().data());
            Start_Audio(QString(presentPath+presentfilename).toLatin1().data());
        }
        else
            Stop_Audio();
    }*/
}
void QImagePropWidget::refreshTiming()
{
    if (ui->frameSlider->value() == ui->frameSlider->maximum()) return;
    if(!StelMainWindow::getInstance().getIsMultiprojector())
        loadFromSharedMem();
    else
    {
        int f_percent = (int) ( (double)( vfile->GetCurrentFrameNumber()) / (double)( vfile->GetVideoDuration()) * 1000.0) ;
        ui->frameSlider->setValue(f_percent);
        ui->timeLineEdit->setText( vfile->GetTimeStrFromFrame(vfile->GetCurrentFrameNumber()) );
    }
}
void QImagePropWidget::updateRefreshTimerState(){

}

void QImagePropWidget::on_chMix_toggled(bool checked)
{
    if(presentfilename == "")
        sPmgr->setMixWithSky(QString::number(presentID),checked);
    else
    {
        sPmgr->setMixWithSky(QString::number(presentID),checked);
        QString str = "core.mixPresentImage('"+QString::number(presentID)+"',"+QString("%1").arg(checked)+");";
        StelApp::getInstance().addNetworkCommand(str);

    }
}

void QImagePropWidget::on_sliderContrast_sliderMoved(int position)
{
    if (vsource)
    {
        vsource->setVideoContrast(position/100.0);
        QString str = QString("core.setPVideoContrast(%1,%2);").arg(presentID).arg(position/100.0);
        StelApp::getInstance().addNetworkCommand(str);
    }
}

void QImagePropWidget::on_sliderBrightness_sliderMoved(int position)
{
    if (vsource)
    {
        vsource->setVideoBrightness(position/100.0);
        QString str = QString("core.setPVideoBrightness(%1,%2);").arg(presentID).arg(position/100.0);
        StelApp::getInstance().addNetworkCommand(str);
    }
}

void QImagePropWidget::on_sliderSat_sliderMoved(int position)
{
    if (vsource)
    {
        vsource->setVideoSaturation(position/100.0);
        QString str = QString("core.setPVideoSaturation(%1,%2);").arg(presentID).arg(position/100.0);
        StelApp::getInstance().addNetworkCommand(str);
    }
}

void QImagePropWidget::on_pauseButton_toggled(bool checked)
{
    if (vsource)
    {
        vsource->pause(checked);
        //Client
        QString str = QString("%1;%2")
                .arg(presentID)
                .arg(checked);
        StelApp::getInstance().getRsync()->sendChanges(RSYNC_COMMAND_PAUSEPRESENTVIDEO,str);
    }
}

void QImagePropWidget::on_videoLoopButton_toggled(bool checked)
{
    //todo
    /*if (vfile)
        vfile->setLoop(checked);*/
}


void QImagePropWidget::on_startButton_clicked(bool checked)
{
    //todo
    if(vsource == NULL)
    {
        vsource= new VideoSource(vfile,sPmgr->getLayerTexture(QString::number(presentID)));
        vsource->init(vfile->GetVideoSize());
        sPmgr->SetLayerVideoSource(QString::number(presentID),vsource);
    }
    if(vsource!= NULL)
    {
        //vfile->seekToPosition(0);
        //#ifndef SHIRAPLAYER_PRE
        if(StelMainWindow::getInstance().getIsMultiprojector())
        {
            //PRO deðilse Server tarafýnda video ve audio çalýþtýrýlmýyor
            vsource->play(checked);
            //StartAudio(checked);
        }
        //#endif
        if (checked)
        {
            refreshTimingTimer->start();
            ui->startButton->setText("Stop");
            ui->startButton->setIcon(QIcon(":/media/gui/presenter/media-playback-stop.png"));
            ui->videoLoopButton->setEnabled(false);
        }
        else
        {
            refreshTimingTimer->stop();
            ui->frameSlider->setValue(0);
            ui->timeLineEdit->setText( "" );
            ui->startButton->setText("Play");
            ui->startButton->setIcon(QIcon(":/media/gui/presenter/media-playback-start.png"));
            ui->videoLoopButton->setEnabled(true);
        }

        //Client
        QString str = QString("%1;%2;%3")
                .arg(presentID)
                .arg(checked)
                .arg(ui->videoLoopButton->isChecked());
        StelApp::getInstance().getRsync()->sendChanges(RSYNC_COMMAND_PLAYPRESENTVIDEO,str);

    }
}

void QImagePropWidget::sendClinet()
{
    QString str = QString("core.setPresentProperties('%1','%2;%3;%4;%5;%6;%7;%8;%9;%10');")
            .arg(presentID)
            .arg(presentPath+ presentfilename)
            .arg(presentRA)
            .arg(presentDec)
            .arg(presentSize)
            .arg(presentRotate)
            .arg(2.5)
            .arg(14.0)
            .arg(aspectratio)
            .arg(isVideo);
    //QMessageBox::information(0,"",str,0,0);
    StelApp::getInstance().addNetworkCommand(str);

}

void QImagePropWidget::loadFromSharedMem()
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
        if(datalist.count()==2)
        {
            ui->frameSlider->setValue(QString(datalist[0]).toInt());
            ui->timeLineEdit->setText(datalist[1]);
        }

    }
}
QDomElement QImagePropWidget::domElement(const QString& name, QDomDocument& doc) const
{
    QDomElement de = doc.createElement(name);
    de.setAttribute("name", QString("%1").arg(presentID));
    de.setAttribute("filename",QString("%1").arg(presentfilename));
    de.setAttribute("path",QString("%1").arg(presentPath));
    de.setAttribute("domeORsky",QString("%1").arg(presentdomeORsky));
    de.setAttribute("Alt",QString("%1").arg(ui->RASlider->value()));
    de.setAttribute("Azi",QString("%1").arg(ui->decSlider->value()));
    de.setAttribute("Size",QString("%1").arg(ui->sizeSlider->value()));
    de.setAttribute("Rotate",QString("%1").arg(ui->rotSlider->value()));
    de.setAttribute("Visible",QString("%1").arg(ui->btnFade->isChecked()));
    de.setAttribute("IsVideo",QString("%1").arg(isVideo));
    de.setAttribute("Contrast",QString("%1").arg(ui->sliderContrast->value()));
    de.setAttribute("Brightness",QString("%1").arg(ui->sliderBrightness->value()));
    de.setAttribute("Saturation",QString("%1").arg(ui->sliderSat->value()));
    return de;
}

const QPixmap* QImagePropWidget::getPixmap() const
{
    return ui->imgPicture->pixmap();
}
QString QImagePropWidget::getCaption() const
{
    return ui->imgFileName->text();
}

void QImagePropWidget::languageChanged()
{
    const bool __sortingEnabled = ui->listTab->isSortingEnabled();
    ui->listTab->setSortingEnabled(false);
    QListWidgetItem *___qlistwidgetitem = ui->listTab->item(0);
    ___qlistwidgetitem->setText(q_("Display Properties"));
    QListWidgetItem *___qlistwidgetitem1 = ui->listTab->item(1);
    ___qlistwidgetitem1->setText(q_("Video Control"));
    ui->listTab->setSortingEnabled(__sortingEnabled);
    ui->imgPicture->setText(q_("image"));
    ui->imgFileName->setText(q_("File Name"));
    ui->groupBoxCaption->setTitle(q_("Properties"));
    ui->label->setText(q_("Resize %"));
    ui->label_2->setText(q_("Azimuth"));
    ui->label_3->setText(q_("Altitude"));
    ui->btnFade->setText(q_("Fade In/Out"));
    ui->rdOnSky->setText(q_("On Sky"));
    ui->rdOnDome->setText(q_("On Dome"));
    ui->Rotate->setText(q_("Rotate"));
    ui->lblFlatPath->setText(q_("lblFlatPath"));
    ui->labelPath->setText(q_("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
                              "<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">\n"
                              "p, li { white-space: pre-wrap; }\n"
                              "</style></head><body style=\" font-family:'MS Shell Dlg 2'; font-size:8.25pt; font-weight:400; font-style:normal;\">\n"
                              "<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-size:8pt; font-weight:600;\">Flat Path :</span></p></body></html>"));
    ui->vcontrolDockGroupBoxControls->setTitle(q_("Video Properties"));
    ui->label_4->setText(q_("Contrast"));
    ui->Brightness->setText(q_("Brightness"));
    ui->label_6->setText(q_("Saturation"));
    ui->timeLineEdit->setToolTip(q_("Current frame time"));
    ui->markInButton->setToolTip(q_("Sets the IN mark at the current time"));
    ui->markInButton->setText(q_("Mark IN"));
    ui->resetMarkInButton->setToolTip(q_("Resets the IN mark at the start of file."));
    ui->markOutButton->setToolTip(q_("Sets the OUT mark at the current time"));
    ui->markOutButton->setText(q_("Mark OUT"));
    ui->resetMarkOutButton->setToolTip(q_("Reset the OUT mark at the end of file."));
    ui->frameSlider->setToolTip(q_("Time line"));
    ui->markInSlider->setToolTip(q_("Placement of the mark IN"));
    ui->markOutSlider->setToolTip(q_("Placement of the mark OUT"));
    ui->startButton->setToolTip(q_("Start / stops updating"));
    ui->startButton->setStatusTip(q_("Starts / stops updating"));
    ui->startButton->setText(q_("Play"));
    ui->pauseButton->setToolTip(q_("Pause/resume play"));
    ui->pauseButton->setStatusTip(q_("Pause/resume play"));
    ui->pauseButton->setText(q_("Pause"));
    ui->videoLoopButton->setToolTip(q_("Loop to mark-in playback"));
    ui->videoLoopButton->setText(q_("..."));
}
