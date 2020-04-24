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


#include <QDir>

#include "StelApp.hpp"
#include "StelMainWindow.hpp"
#include "StelTranslator.hpp"
#include "videoutils/audioclass.h"
#include "socketutils/rsync.h"

#include "audiomanagerdialog.h"
#include "ui_audiomanagerdialog.h"

audioManagerDialog::audioManagerDialog(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::audioManagerDialog)
{
    ui->setupUi(this);
    conf = StelApp::getInstance().getSettings();
    media_path = conf->value("main/media_path", "C:/").toString();
    media_path = media_path+"/audio";
    currentState = Stopped;

    imagesFuldomeVideoShow_ = new QFutureWatcher<QImage>(this);
    connect(imagesFuldomeVideoShow_, SIGNAL(resultReadyAt(int)), SLOT(setFuldomeVideoItemInList(int)));
    connect(imagesFuldomeVideoShow_, SIGNAL(finished()), SLOT(finished()));
    ShowFulldomeVideoIconFile(media_path );

    ui->lblPath->setText(media_path);
    m_timer.setInterval(100);
    connect(&m_timer, SIGNAL(timeout()),this,SLOT(tracktime()));

    audioSt = &AudioClass::getInstance();
}

audioManagerDialog::~audioManagerDialog()
{
    delete ui;
}

void audioManagerDialog::languageChanged()
{
    ui->lblProperties->setText(q_("Properties"));
    ui->btnmStart->setText(q_("Start"));
    ui->btnmPause->setText(q_("Pause"));
    ui->btnmStop->setText(q_("Stop"));
    ui->lblStatus->setText(q_("Status"));
    ui->labelVolume->setText(q_("Volume"));
    ui->lblPathDesc->setText(q_("<html><head/><body><p><span style=\" font-size:9pt; color:#ff0000;\">Audio Files Path :</span></p></body></html>"));
    ui->chLoop->setText("Loop");
}

QFileInfoList audioManagerDialog::getFulldomeVideoListFiles(QString dirPath) const
{
    QDir dir(dirPath);
    dir.setFilter(QDir::Files | QDir::Hidden | QDir::NoSymLinks);
    dir.setSorting(QDir::Name );
    QStringList filters;
    filters << "*.wav" << "*.mp3" << "*.aiff" << "*.ogg" ;
    dir.setNameFilters(filters);

    return dir.entryInfoList(filters, QDir::Files);

}

void audioManagerDialog::setFuldomeVideoItemInList(int index)
{
    ui->listFilesWidget->item(index)->setIcon(QIcon(QPixmap::fromImage(imagesFuldomeVideoShow_->resultAt(index))));
}

void audioManagerDialog::WaitFutureFulldomeList()
{
    if (imagesFuldomeVideoShow_->isRunning())
    {
        imagesFuldomeVideoShow_->cancel();
        imagesFuldomeVideoShow_->waitForFinished();
    }
}

void audioManagerDialog::ShowFulldomeVideoIconFile(QString dirPath)
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
}

void audioManagerDialog::setMediaButtonDurum()
{
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

void audioManagerDialog::tracktime()
{
    if ((audioSt != NULL) && (!audioSt->isFinised()))
    {
        //qDebug()<<"end:"<<audioSt->getEndSn()<<" current:"<<audioSt->getCurrentFrameSn();
        ui->trackMedia->setMaximum(audioSt->getEndSn());
        ui->trackMedia->setValue(audioSt->getCurrentFrameSn());
        ui->labelProperties->setText(QString("Current Time: %1 \nTotal Time: %2 sn")
                                     .arg(audioSt->getCurrentClockStr())
                                     .arg(audioSt->getEndSnStr())
                                     );
    }

    if ((audioSt != NULL) && (audioSt->isFinised()))
        on_btnmStop_clicked();   
}

void audioManagerDialog::on_btnmStart_clicked()
{
    if(ui->listFilesWidget->selectedItems().count()==0)
    {
        QMessageBox::warning(0,q_("Warning"),q_("Please select a audio file!"),0,0);
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
    QFileInfo uDir(strfile);
    if (uDir.exists())
    {
        audioSt->Stop_Audio();
        StelApp::getInstance().getRsync()->sendChanges(RSYNC_COMMAND_STOP_AUDIO,"");
        //qDebug()<<"QString audio file:"<<strfile.toUtf8().data();
        audioSt->Load_Audio(strfile.toUtf8().data());
        ui->trackMedia->setMinimum(0);
        if (ui->chLoop->isChecked())
            audioSt->setLoop(1);
        else
            audioSt->setLoop(0);
        audioSt->Start_Audio();
        qDebug()<<"Audio start:"<< QDateTime().currentDateTime().toString("hh:mm:ss.zzz");
        m_timer.start();
        currentState = Running;
        setMediaButtonDurum();
    }
}

void audioManagerDialog::on_btnmStop_clicked()
{
    currentState = Stopped;
    setMediaButtonDurum();
    m_timer.stop();
    audioSt->Stop_Audio();
}

void audioManagerDialog::on_btnmPause_clicked()
{
    qDebug()<<"Audio state:"<<currentState;
    currentState==Running?ui->btnmPause->setText(q_("Continue")):ui->btnmPause->setText(q_("Pause"));
    currentState==Running?currentState=Paused:currentState=Running;

    setMediaButtonDurum();
    audioSt->toggle_pause();

}

void audioManagerDialog::on_trackMedia_sliderReleased()
{
    if (currentState == Stopped) return;

    double pos = (double)ui->trackMedia->value();
    audioSt->audioSeekToPositionSn(pos);
    m_timer.start();
}

void audioManagerDialog::on_trackMedia_sliderPressed()
{
    if (currentState == Stopped) return;
    m_timer.stop();
}

void audioManagerDialog::on_tbarSes_valueChanged(int val)
{
    audioSt->set_volume(val);
}
