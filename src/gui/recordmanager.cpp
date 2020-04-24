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

#include <QFileDialog>
#include <QMessageBox>

#include "StelApp.hpp"
#include "StelCore.hpp"
#include "StelGui.hpp"
#include "StelFileMgr.hpp"
#include "StelMainWindow.hpp"
#include "StelTranslator.hpp"
#include "recordmanager.hpp"
#include "ui_recordmanager.h"

#include "licenceutils/qlisansform.h"

recordmanager::recordmanager()
{
    ui = new Ui_recordmanager;
}
recordmanager::~recordmanager()
{
    delete ui;
}
void recordmanager::createDialogContent()
{
    ui->setupUi(dialog);
    connect(ui->closeStelWindow, SIGNAL(clicked()), this, SLOT(close()));
    connect(ui->btnSelectFile, SIGNAL(clicked()),this,SLOT(selectbtnClicked()));
    connect(ui->recButton, SIGNAL(clicked()),this,SLOT(recButtonClick()));
    connect(ui->recstopButton, SIGNAL(clicked()),this, SLOT(recstopButtonClick()));
    connect(ui->lineEdit,SIGNAL(textChanged(QString)),SLOT(textFileChanged(QString)));
    connect(ui->recpauseButton,SIGNAL(clicked()),this,SLOT(recpauseButtonClick()));

    connect(ui->btnLisans,SIGNAL(clicked()),this,SLOT(btnLisansClick()));

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
//        ui->btnLisans->setVisible(false);
//        ui->lisans->setVisible(false);
//        ui->registerLayout->removeWidget(ui->btnLisans);
//        ui->registerLayout->removeWidget(ui->lisans);
        ui->frameLisans->setVisible(false);
        StelMainWindow::getInstance().is_Licenced = true;
    }
    else
    {
        frm.FormDoldur(uname,lcode);
        StelMainWindow::getInstance().is_Licenced = false;
    }
    if(!StelMainWindow::getInstance().is_Licenced)
    {
        int w =StelMainGraphicsView::getInstance().width();
        int h = StelMainGraphicsView::getInstance().height();
        sImgr.createScreenImage("logo", ":/mainWindow/gui/logo_ekran.png",
                                            w/2-150, h/2+110, 1.0, false, 1.0, 1.0,true);
        sImgr.createScreenImage("logo1", ":/mainWindow/gui/logo_ekran_45.png",
                                            w/2-400, h/2+25, 1.0, false, 1.0, 1.0,true);
        sImgr.createScreenImage("logo2", ":/mainWindow/gui/logo_ekran_45_2.png",
                                            w/2, h/2+25, 1.0, false, 1.0, 1.0,true);
        sImgr.createScreenImage("logo3", ":/mainWindow/gui/logo_ekran_180.png",
                                            w/2-150, h/2-350, 1.0, false, 1.0, 1.0,true);
    }
}
void recordmanager::prepareAsChild()
{
    ui->LocationBar->hide();
}

void recordmanager::retranslate()
{
    languageChanged();
}
void recordmanager::languageChanged()
{
    if (dialog)
    {
        //ui->retranslateUi(dialog);
        ui->stelWindowTitle->setText(q_("Record Window"));
        ui->groupBoxProp->setTitle(q_("Properties"));
        ui->label->setText(q_("File Name:"));
        ui->btnSelectFile->setText(q_("Select File"));
        ui->label_2->setText(q_("FPS : "));
        ui->lblStatus->setText(q_("Stopped"));
        ui->recButton->setText(q_("Rec"));
        ui->recpauseButton->setText(q_("Pause"));
        ui->recstopButton->setText(q_("Stop"));
        ui->lisans->setText(q_("UnLicensed Use.."));
        ui->btnLisans->setText(q_("Register Module"));
    }
}

void recordmanager::styleChanged()
{
    // Nothing for now
}

void recordmanager::selectbtnClicked()
{
    movieFileName = QFileDialog::getSaveFileName(0,
                                                 tr("Save Movie"), 
                                                 qApp->applicationDirPath()+"\\untitled.avi", 
                                                 tr("Movie Files(*.avi *.mpeg *.mp4)"));
    ui->lineEdit->setText(movieFileName);

}

void recordmanager::textFileChanged (QString text )
{
    movieFileName = text;
}
void recordmanager::recButtonClick()
{
    int FPS = ui->spinBoxFPS->value();

    if(StelFileMgr::exists(movieFileName))
    {
        if (QMessageBox::warning(0,q_("Warning!"),q_("This file is exist.Do you want to overwrite?"),QMessageBox::Yes,QMessageBox::No) == QMessageBox::No)
            return;
    }

    if(StelFileMgr::isDirectory(movieFileName) )
    {
        QMessageBox::warning(0,q_("Error"),q_("Please select a file to record! Record is cancelled."),0,0);
        return;
    }

    if(movieFileName == "" )
    {
        QMessageBox::warning(0,q_("Error"),q_("Please select a file to record! Record is cancelled."),0,0);
        return;
    }
    StelApp::getInstance().getRsync()->sendChanges(RSYNC_COMMAND_RECORDVIDEO,QString("%1@%2").arg(movieFileName).arg(FPS));

    //Server da iptal oldu
    //StelMainGraphicsView::getInstance().startRecordFile(movieFileName ,FPS );

    ui->recButton->setEnabled(false);
    ui->recButton->setStyleSheet(" QPushButton { background-color: red; border: none; }");
    ui->groupBoxProp->setEnabled(false);
    ui->lblStatusIcon->setPixmap(QPixmap(":/mainWindow/gui/rec_started.png"));
    ui->lblStatus->setText("Recording");

//Server da iptal oldu
//    if(!StelApp::getInstance().getCore()->is_Licenced)
//    {
//        sImgr.setImageAlpha("logo", 0.5);
//        sImgr.setImageAlpha("logo1", 0.5);
//        sImgr.setImageAlpha("logo2", 0.5);
//        sImgr.setImageAlpha("logo3", 0.5);
//        sImgr.showImage("logo",true);
//        sImgr.showImage("logo1",true);
//        sImgr.showImage("logo2",true);
//        sImgr.showImage("logo3",true);
//    }
}
void recordmanager::recstopButtonClick()
{
    //Server da iptal oldu
    //StelMainGraphicsView::getInstance().stopRecordFile();
    ui->recButton->setEnabled(true);
    ui->groupBoxProp->setEnabled(true);
    ui->lblStatusIcon->setPixmap(QPixmap(":/mainWindow/gui/rec_stopped.png"));
    ui->lblStatus->setText("Stopped");
    ui->recButton->setStyleSheet(StelApp::getInstance().getCurrentStelStyle()->qtStyleSheet);
    ui->recpauseButton->setStyleSheet(StelApp::getInstance().getCurrentStelStyle()->qtStyleSheet);

    StelApp::getInstance().getRsync()->sendChanges(RSYNC_COMMAND_STOPRECVIDEO,"");

//Server da iptal oldu
//    if(!StelMainWindow::getInstance().is_Licenced)
//    {
//        sImgr.showImage("logo",false);
//        sImgr.showImage("logo1",false);
//        sImgr.showImage("logo2",false);
//        sImgr.showImage("logo3",false);
//    }

}
void recordmanager::recpauseButtonClick()
{

    //if(StelMainGraphicsView::getInstance().b_save)
    if(ui->lblStatus->text()=="Recording")
    {        
        //if(StelMainGraphicsView::getInstance().pauseRecord(true))
        StelApp::getInstance().getRsync()->sendChanges(RSYNC_COMMAND_PAUSERECVIDEO,"1");
        {
            ui->recpauseButton->setStyleSheet(" QPushButton { background-color: yellow; border: none; }");
            ui->lblStatus->setText("Paused");
            ui->lblStatusIcon->setPixmap(QPixmap(":/mainWindow/gui/rec_paused.png"));
        }
    }
    else
    {
        //if(StelMainGraphicsView::getInstance().pauseRecord(false))
        StelApp::getInstance().getRsync()->sendChanges(RSYNC_COMMAND_PAUSERECVIDEO,"0");
        {
            ui->recpauseButton->setStyleSheet(StelApp::getInstance().getCurrentStelStyle()->qtStyleSheet);
            ui->lblStatus->setText("Recording");
            ui->lblStatusIcon->setPixmap(QPixmap(":/mainWindow/gui/rec_started.png"));
        }
    }
}

void recordmanager::btnLisansClick()
{
    frm.setVisible(true);
    frm.getProxy()->setPos(this->getProxy()->pos());
}
