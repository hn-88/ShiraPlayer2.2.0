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

#include "rsync.h"
#include <QtNetwork/QtNetwork>
#include <QtNetwork/QUdpSocket>
#include <QMessageBox>
#include <QtXml/QtXml>
#include <QtNetwork/QHostAddress>
#include <windows.h>
#include <psapi.h>

#include "StelMainGraphicsView.hpp"
#include "StelApp.hpp"
#include "StelCore.hpp"
#include "StelMovementMgr.hpp"
#include "StelScriptMgr.hpp"
#include "StelNavigator.hpp"
#include "StelMainWindow.hpp"
#include "LandscapeMgr.hpp"
#include "StelModuleMgr.hpp"
#include "StelGui.hpp"
#include "StelAppGraphicsWidget.hpp"
#include "StelUtils.hpp"
#include "StelLocaleMgr.hpp"
#include "StelObjectMgr.hpp"
#include "StelLocationMgr.hpp"
#include "StelSkyDrawer.hpp"
#include "SolarSystem.hpp"
#include "NebulaMgr.hpp"
#include "MeteorMgr.hpp"
#include "GridLinesMgr.hpp"
#include "ConstellationMgr.hpp"
#include "StelStyle.hpp"
#include "StarMgr.hpp"
#include "MilkyWay.hpp"
#include "GridLinesMgr.hpp"
#include "Constellation.hpp"
#include "StelGui.hpp"
#include "StelGuiItems.hpp"
#include "StelSkyCultureMgr.hpp"
#include "../plugins/Satellites/src/Satellites.hpp"
#include "../plugins/CompassMarks/src/CompassMarks.hpp"
#include "StelMainScriptAPI.hpp"

#include "socketutils/sntpclient.h"
#include "videoutils/audioclass.h"

#include "encrypt/cBinExt.h"
#include "StelFileMgr.hpp"

//#include <qextserialport.h>
class Satellites;


//! Default port number.
#define DEFAULT_PORT_NO 55555

QString strfile="";// =media_path+"/"+datalist[0];
int savedprewarped = 0;
int showWithDaylight = 0;
QString savedViewportEffect = "none";

BOOL MySystemShutdown()
{
    HANDLE hToken;
    TOKEN_PRIVILEGES tkp;

    // Get a token for this process.

    if (!OpenProcessToken(GetCurrentProcess(),
                          TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
        return( FALSE );

    // Get the LUID for the shutdown privilege.

    LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME,
                         &tkp.Privileges[0].Luid);

    tkp.PrivilegeCount = 1;  // one privilege to set
    tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    // Get the shutdown privilege for this process.

    AdjustTokenPrivileges(hToken, FALSE, &tkp, 0,
                          (PTOKEN_PRIVILEGES)NULL, 0);

    if (GetLastError() != ERROR_SUCCESS)
        return FALSE;

    // Shut down the system and force all applications to close.

    if (!ExitWindowsEx(EWX_SHUTDOWN | EWX_FORCE,0))
        return FALSE;

    //shutdown was successful
    return TRUE;
}


class BlockWriter
{
public:
    BlockWriter(QIODevice *io)
    {
        buffer.open(QIODevice::WriteOnly);
        this->io = io;
        _stream.setVersion(QDataStream::Qt_4_8);
        _stream.setDevice(&buffer);

        // Placeholder for the size. We will get the value
        // at the end.
        _stream << quint64(0);
    }

    ~BlockWriter()
    {
        // Write the real size.
        _stream.device()->seek(0);
        _stream << (quint64) buffer.size();

        // Flush to the device.
        io->write(buffer.buffer());
    }

    QDataStream &stream()
    {
        return _stream;
    }

private:
    QBuffer buffer;
    QDataStream _stream;
    QIODevice *io;
};

rsync::rsync()
{
    startGraphTimer = new QTimer(this);
    startGraphTimer->setInterval(2000);
    startGraphTimer->setSingleShot(true);
    connect(startGraphTimer, SIGNAL(timeout()), this, SLOT(startGraphBYTimer()));

    stopGraphTimer = new QTimer(this);
    stopGraphTimer->setInterval(2000);
    stopGraphTimer->setSingleShot(true);
    connect(stopGraphTimer, SIGNAL(timeout()), this, SLOT(stopGraphBYTimer()));

    showFisheyeFrameTimer = new QTimer(this);
    showFisheyeFrameTimer->setInterval(2000);
    showFisheyeFrameTimer->setSingleShot(true);
    connect(showFisheyeFrameTimer, SIGNAL(timeout()), this, SLOT(showFisheyeFrameBYTimer()));

    clearFramesTimer = new QTimer(this);
    clearFramesTimer->setInterval(2000);
    clearFramesTimer->setSingleShot(true);
    connect(clearFramesTimer, SIGNAL(timeout()), this, SLOT(clearFramesBYTimer()));

    trackTimerConsole = new QTimer(this);
    trackTimerConsole->setInterval(500);
    connect(trackTimerConsole, SIGNAL(timeout()), this, SLOT(trackConsoleTimer()));

    stellaHideTimer = new QTimer(this);
    stellaHideTimer->setInterval(2000);
    stellaHideTimer->setSingleShot(true);
    connect(stellaHideTimer, SIGNAL(timeout()), this, SLOT(on_stellaHideTimer()));

    resetAllTimer = new QTimer(this);
    resetAllTimer->setInterval(2000);
    resetAllTimer->setSingleShot(true);
    connect(resetAllTimer,SIGNAL(timeout()),this,SLOT(resetAllProc()));


    savedProjectionType = 3;

    //#ifdef SHIRAPLAYER_PRE
    if(!StelMainWindow::getInstance().getIsMultiprojector())
    {
        sharedMem.setKey("ShiraPlayerSharedMemory");
        videoSharedMemConsole.setKey("videoSharedMemory");
    }
    //#else
    //    m_pUdpServer = NULL;
    //    m_pUdpClient = NULL;
    //    m_portNo = DEFAULT_PORT_NO;
    //#endif

    //Console için Flyby
    realtrackValue = 2;

    oldViewDirection = Vec3d(0,0,0);

    audiost = &AudioClass::getInstance();
}

void rsync::initServer()
{
    //#ifndef SHIRAPLAYER_PRE
    //    qDebug()<<"UDP service started port:"<<m_portNo;
    //    m_bServer = true;
    //    m_pUdpServer = new QUdpSocket(this);
    //#endif
    if(StelMainWindow::getInstance().getIsMultiprojector())
    {
        //Android tablet baðlantýsý için çoklu projektör aktif ise client olarak çalýþmasý saðlandý
        initClient();
    }
}

void rsync::sendChanges(RSYNC_COMMAND command, const QString& data)
{
    //#ifdef SHIRAPLAYER_PRE
    if(!StelMainWindow::getInstance().getIsMultiprojector())
    {
        LoadIntoSharedMomory(QString::number(command)+"@"+data);
        return;
    }
    //#else
    //    QByteArray datagram = QByteArray::number(command) +"@"+ data.toLatin1();
    //    m_pUdpServer->writeDatagram(datagram.data(), datagram.size(), QHostAddress::Broadcast, m_portNo);
    //#endif
}
void rsync::sendOpenTerminals(const QString& MACAdress)
{
#ifdef SHIRAPLAYER_PRE
    return;
#endif

    QByteArray mac = QByteArray::fromHex(MACAdress.toLatin1());

    QByteArray packet;
    packet.resize(17 * 6);

    for (int i = 0; i < 6; i++)
        packet[i] = 0xFF;

    for (int i = 1; i <= 16; i++)
        for (int j = 0; j < 6; j++)
            packet[i * 6 + j] = mac[j];

    m_pUdpServer->writeDatagram(packet.data(), packet.size(), QHostAddress::Broadcast, m_portNo);
    //QMessageBox::information(0,"",QString("%0").arg(packet.data()),0,0);
}

void rsync::initClient()
{
    //#ifdef SHIRAPLAYER_PRE
    //if(!StelMainWindow::getInstance().getIsMultiprojector())
    {
        m_clientAddress = QHostInfo::localHostName();

        //ShiraPlayer Console
        m_tcpserver = new QTcpServer(this);
        connect(m_tcpserver,SIGNAL(newConnection()),this,SLOT(newConnection()));

        if (!m_tcpserver->listen(QHostAddress::Any,DEFAULT_PORT_NO))
        {
            qDebug()<<"ShiraPlayerConsole listening could not start!";
        }
        else
        {
            qDebug()<<"ShiraPlayerConsole listening started!";
            QTcpSocket *socket = new QTcpSocket(this);
        }
    }
    //#else
    //    m_pUdpClient = new QUdpSocket(this);
    //    m_pUdpClient->setReadBufferSize(0);
    //    m_pUdpClient ->bind(m_portNo);
    //    //m_pUdpClient ->bind(QHostAddress::Broadcast, m_portNo);

    //    connect(m_pUdpClient, SIGNAL(readyRead()), this, SLOT(recvSettings()));
    //    connect(m_pUdpClient, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(displayNetworkError(QAbstractSocket::SocketError)));
    //    connect(this,SIGNAL(ftpfinished()),this,SLOT(doftpfinished()));

    //    m_clientAddress = QHostInfo::localHostName();
    //    qDebug()<<"Client UDP listen started port:"<< m_portNo;
    //#endif
    //vop = new VideoOperations();
    conf = StelApp::getInstance().getSettings();
    media_path = conf->value("main/media_path", "C:/").toString();
    is_audio = conf->value("main/flag_audio","false").toBool();
    only_audio = conf->value("main/only_audio","false").toBool();

    if(!StelMainWindow::getInstance().getIsMultiprojector())
    {
        //Record anýnda gösterilecek watermarklar
        if(!StelMainWindow::getInstance().is_Licenced)
        {
            int w =conf->value("video/screen_w", 800).toInt();//StelMainGraphicsView::getInstance().width();
            int h = conf->value("video/screen_h", 600).toInt();//StelMainGraphicsView::getInstance().height();
            sImgr.createScreenImage("logo", ":/mainWindow/gui/logo_ekran.png",
                                    w/2-150, h/2+110, 1.0, false, 1.0, 1.0,true);
            sImgr.createScreenImage("logo1", ":/mainWindow/gui/logo_ekran_45.png",
                                    w/2-400, h/2+25, 1.0, false, 1.0, 1.0,true);
            sImgr.createScreenImage("logo2", ":/mainWindow/gui/logo_ekran_45_2.png",
                                    w/2, h/2+25, 1.0, false, 1.0, 1.0,true);
            sImgr.createScreenImage("logo3", ":/mainWindow/gui/logo_ekran_180.png",
                                    w/2-150, h/2-350, 1.0, false, 1.0, 1.0,true);
        }

        StelMovementMgr* mmgr = GETSTELMODULE(StelMovementMgr);
        mmgr->setFlagEnableMoveKeys(false);
        mmgr->setFlagEnableMouseNavigation(false);
        mmgr->setFlagEnableZoomKeys(false);
    }

}

void rsync::recvSettings()
{
    StelCore* core = StelApp::getInstance().getCore();

    QByteArray datagram;
    int command;
    QStringList datalist;
    QString data;
    while(m_pUdpClient->hasPendingDatagrams())
    {
        datagram.resize(m_pUdpClient->pendingDatagramSize());
        m_pUdpClient->readDatagram(datagram.data(), datagram.size());

        datalist = QString(datagram.data()).split("@");

        command = QString(datalist[0]).toInt();
        data    = datalist[1];
        qDebug()<<"Received Command: "<< command<<"@"<<data;

        switch (command)
        {
        case RSYNC_COMMAND_INIT:
        {
            //QMessageBox::critical(0,"",data,0,0);
            QDomDocument doc;
            doc.setContent(data);
            QDomElement channel = doc.firstChildElement("Channel");
            QString channelName = channel.attribute("name");

            if(m_clientAddress == channelName)
            {
                //StelMainGraphicsView &mainView =  StelMainGraphicsView::getInstance();
                StelCore* core = StelApp::getInstance().getCore();

                StelMainGraphicsView::getInstance().m_pSelectedChannel->initFromDOMElement(channel);

                core->getMovementMgr()->setInitFov(StelMainGraphicsView::getInstance().m_pSelectedChannel->m_initfov);
                core->getMovementMgr()->zoomTo(StelMainGraphicsView::getInstance().m_pSelectedChannel->m_initfov, 0.2);

                core->getMovementMgr()->setViewDirectionJ2000(core->getNavigator()->altAzToJ2000(StelMainGraphicsView::getInstance().m_pSelectedChannel->m_init_view_pos));

                int w = StelMainGraphicsView::getInstance().m_pSelectedChannel->getWidth();
                int h = StelMainGraphicsView::getInstance().m_pSelectedChannel->getHeight();
                if (w == 0)
                    w = 800;
                if (h == 0)
                    h = 600;
                StelMainGraphicsView::getInstance().setGeometry(0,0,w,h);
                core->initWarpGL();
                StelApp::getInstance().set_fwr(StelMainGraphicsView::getInstance().m_pSelectedChannel->m_fwr);
                StelApp::getInstance().set_fhr(StelMainGraphicsView::getInstance().m_pSelectedChannel->m_fhr);
                core->setCurrentProjectionTypeKey("ProjectionStereographic");
                //core->setCurrentProjectionTypeKey("ProjectionPerspective");
                //core->setCurrentProjectionTypeKey("ProjectionOrthographic");

                //Zamaný eþitle
                //QMessageBox::critical(0,"",datalist[2],0,0);
                sntpclient* s = new sntpclient(datalist[2],true);
                s->connectserver();
                Gecikme(datalist[3]);
                core->allFaderColor = Vec3f(0,0,0);
                core->ALLFader.setDuration(2000);
                core->ALLFader = false;
            }

            break;
        }
        case RSYNC_COMMAND_INIT_OFF:
        {
            StelApp::getInstance().getCore()->allFaderColor = Vec3f(0,0,0);
            StelApp::getInstance().getCore()->ALLFader.setDuration(2000);
            StelApp::getInstance().getCore()->ALLFader = true;
            break;
        }
        case RSYNC_COMMAND_SCRIPT:
        {
            StelApp::getInstance().getCore()->allFaderColor = Vec3f(0,0,0);
            StelMainGraphicsView::getInstance().getScriptMgr().runScriptNetwork(data);
            StelApp::getInstance().getCore()->ALLFader.setDuration(1000);
            StelApp::getInstance().getCore()->ALLFader = false;
            break;
        }
        case RSYNC_COMMAND_LIVE_SCRIPT:
        {
            if(data!="")
            {
                //QMessageBox::critical(0,"",data,0,0);
                StelScriptMgr& scriptMgr = StelMainGraphicsView::getInstance().getScriptMgr();
                if (!scriptMgr.scriptIsRunning())
                    scriptMgr.runScriptNetwork(data);
            }
            else
            {
                StelMainGraphicsView::getInstance().getScriptMgr().stopScript();
            }
            break;
        }
        case RSYNC_COMMAND_CHANNEL:
        {
            QDomDocument doc;
            doc.setContent(data);
            QDomElement channel = doc.firstChildElement("Channel");
            QString channelName = channel.attribute("name");

            if(m_clientAddress == channelName)
            {
                StelMainGraphicsView::getInstance().m_pSelectedChannel->initFromDOMElement(channel);
                StelApp::getInstance().getCore()->getMovementMgr()->setInitFov(StelMainGraphicsView::getInstance().m_pSelectedChannel->m_initfov);
                StelApp::getInstance().getCore()->getMovementMgr()->zoomTo(StelMainGraphicsView::getInstance().m_pSelectedChannel->m_initfov, 0.2);

                StelApp::getInstance().getCore()->getMovementMgr()->setViewDirectionJ2000(StelApp::getInstance().getCore()->getNavigator()->altAzToJ2000(StelMainGraphicsView::getInstance().m_pSelectedChannel->m_init_view_pos));
                StelApp::getInstance().set_fwr(StelMainGraphicsView::getInstance().m_pSelectedChannel->m_fwr);
                StelApp::getInstance().set_fhr(StelMainGraphicsView::getInstance().m_pSelectedChannel->m_fhr);
            }
            break;
        }
        case RSYNC_COMMAND_WARP_MODE:
        {
            StelMainGraphicsView::getInstance().setwarpMode(data);
            break;
        }

        case RSYNC_COMMAND_START_GRAPH:
        {
            datalist = QString(data).split(";");
            //                .arg(ui->listFilesWidget->selectedItems()[0]->text())
            //                .arg(movie_width)
            //                .arg(movie_height)
            //                .arg(ui->tbarSes->value())
            //                .arg(timedelay.toString("dd.MM.yyyy hh:mm:ss"))
            //                .arg(m_serverAddress);
            //                .arg(ui->chpreWarped->isChecked());
            //                .arg(ui->chShowFPS->isChecked()));
            //                .arg(ui->chUseShaders->isChecked());
            //                .arg(ui->chsSkipNextFrame->isChecked();
            //                .arg(ui->chSound->isChecked());

            strfile = datalist[0];
            QFileInfo uDir(strfile);

            StelApp::getInstance().setUseGLShaders(datalist[8].toInt());
            LandscapeMgr* lmgr = GETSTELMODULE(LandscapeMgr);
            //todo
            /*if(lmgr->vop_curr)
            {
                lmgr->vop_curr->useYUVData = datalist[8].toInt();
                lmgr->vop_curr->skipNextFrame = datalist[9].toInt();
            }*/
            QString ispreWarped = datalist[6];
            if (ispreWarped == "1")
                savedprewarped = 1;

            QString showFPS = datalist[7];
            if (showFPS == "1")
                core->showFPS = true;
            else
                core->showFPS = false;

            withSound = datalist[10].toInt();
            if(!only_audio)
            {
                core->allFaderColor = Vec3f(0,0,0);
                core->ALLFader.setDuration(2000);
                core->ALLFader = true;

                //Þu anki landscape hafýzaya alýnýyor
                sLandscapeVisibleFlag = lmgr->getFlagLandscape();
                sLandscape = lmgr->getCurrentLandscapeName();
                old_isVideoLandscape = lmgr->isVideoLandscape;
                lmgr->isVideoLandscape = false;

                //Görünüm kaydediliyor
                oldViewDirection = core->getMovementMgr()->getViewDirectionJ2000();

#ifndef SHIRAPLAYER_PRE
                if(lmgr->s_Name != "")
                    lmgr->doClearLandscapeVideo();

                //Client da baþlat
                //Scripte ile Yapýlan hareket deðiþiklikleri geri alýnýyor.
                StelMovementMgr* mmgr = GETSTELMODULE(StelMovementMgr);
                mmgr->panView(-mmgr->rec_deltaAz,-mmgr->rec_deltaAlt);

                if (uDir.exists())
                {
                    lmgr->s_Name = "";
                    lmgr->doSetCurrentLandscapetoVideo(strfile);
                    lmgr->setFlagLandscape(true);
                }
                lmgr->doStartVideo();
            }
#else
                startGraphTimer->start();
            }
#endif

#ifndef SHIRAPLAYER_PRE
            if(is_audio)
                if (uDir.exists())
                    Start_Audio(strfile.toLatin1().data());


            Gecikme(datalist[4]);

            if(!only_audio)
            {
                //Tüm çizimler LandscapeMgr hariç kapatlýyor
                StelApp::getInstance().isVideoMode = true;
                old_showPropGui = StelApp::getInstance().showPropGui ;
                StelApp::getInstance().showPropGui = false;
                ///
                core->ALLFader = false;
            }

#endif

            break;
        }
        case RSYNC_COMMAND_STOP_GRAPH:
        {
            stopGraphProc();
            break;
        }
        case RSYNC_COMMAND_STOP_AUDIO:
        {
            audiost->Stop_Audio();
            break;
        }
        case RSYNC_COMMAND_PAUSE_GRAPH:
        {
            if(!only_audio)
            {
                LandscapeMgr* lmgr = GETSTELMODULE(LandscapeMgr);
                if(lmgr->vop_curr)
                    lmgr->doPauseVideo(data.toInt());
            }
            break;
        }
        case RSYNC_COMMAND_SEEK_GRAPH:
        {
            int64_t pos = (int64_t)data.toLong();
            if(!only_audio)
            {
                LandscapeMgr* lmgr = GETSTELMODULE(LandscapeMgr);
                if(lmgr->vop_curr)
                    lmgr->vop_curr->SeekByFrame(pos);
            }

            break;
        }
        case RSYNC_COMMAND_SHOW_FRAME:
        {

            datalist = QString(data).split(";");
            LandscapeMgr* lmgr = GETSTELMODULE(LandscapeMgr);
            if(datalist[0] != "")
            {
                QStringList lst = datalist[0].split("/");
                QString framepath = media_path+"/panorama/"+ lst[lst.count()-1];
                //qDebug()<<"Frame Path"<<framepath;
                QFileInfo thePath(framepath);
                if (datalist[2] == "1")
                {
                    if(thePath.exists())
                    {
                        lmgr->setFlagLandscape(false);
                        sLandscape = lmgr->getCurrentLandscapeName();
                        old_isVideoLandscape = lmgr->isVideoLandscape;
                        lmgr->doSetCurrentLandscapetoFrame(framepath,sLandscape,datalist[1].toInt(),datalist[3].toInt());
                        lmgr->setFlagLandscape(true);
                    }
                }
                else
                {
                    if(thePath.exists())
                    {
                        core->allFaderColor = Vec3f(0,0,0);
                        core->ALLFader.setDuration(datalist[3].toInt()*1000);
                        core->ALLFader = true;
                        showFisheyeFrameTimer->setInterval(core->ALLFader.getDuration());
                        if (oldViewDirection == Vec3d(0,0,0))
                            oldViewDirection = core->getMovementMgr()->getViewDirectionJ2000();
                        showFisheyeFrameTimer->start();
                        strfile = framepath;
                        showWithDaylight =datalist[1].toInt();
                    }
                }
            }

            break;
        }
        case RSYNC_COMMAND_SHOW_CLEAR:
        {
            if(sLandscape != "")
            {
                LandscapeMgr* lmgr = GETSTELMODULE(LandscapeMgr);
                lmgr->doSetCurrentLandscapeName(sLandscape);
                //lmgr->setCurrentLandscapeName(sLandscape,false,old_isVideoLandscape);
            }
            break;
        }
        case RSYNC_COMMAND_FTP:
        {
            int tab = datalist[1].toInt();
            dowloadFtpFile(datalist[2],datalist[3],datalist[4],datalist[5],tab);
            break;
        }
        case RSYNC_COMMAND_FTP_UPDPRG:
        {
            QString filename = "shiraplayer.exe";
            //libstelmain dowload etmek için gerekli
            sftp_server = datalist[1];
            sftp_prog_user = datalist[2];
            sftp_prog_pass = datalist[3];

            dowloadFtpFile(sftp_server ,filename,sftp_prog_user,sftp_prog_pass,3);
            break;
        }
        case RSYNC_COMMAND_RESTARTAPP:
        {
            QProcess *process = new QProcess(this);
            //QMessageBox::critical(0,"",qApp->applicationName(),0,0);
            process->startDetached(qApp->applicationName());
            qApp->exit(0);
            break;
        }
        case RSYNC_COMMAND_SAVESET:
        {
            saveCurrentViewOptions();
            break;
        }
        case RSYNC_COMMAND_SAVELOC:
        {
            StelLocation loc; //= locationFromFields();

            loc.planetName = datalist[1];
            loc.name = datalist[2];
            loc.latitude = QString("%0").arg(datalist[3]).toFloat();
            loc.longitude = QString("%0").arg(datalist[4]).toFloat();
            loc.altitude = QString("%0").arg(datalist[5]).toInt();
            loc.country = datalist[6];

            StelApp::getInstance().getLocationMgr().saveUserLocation(loc);

            break;
        }
        case RSYNC_COMMAND_SHOWCHNAME:
        {
            StelCore* core = StelApp::getInstance().getCore();
            core->showchannel_name = data.toInt();
            break;
        }
        case RSYNC_COMMAND_SHUTDOWN_PC:
        {
            MySystemShutdown();
            break;
        }
        case RSYNC_COMMAND_VOLUME:
        {
            //todo
            /*if(is_audio)
            {
                set_volume(data.toInt());
            }*/
            break;
        }
        case RSYNC_COMMAND_PROJECTOR:
        {
            //QMessageBox::critical(0,"",data,0,0);
            //                if (data == "ON")
            //                    sendSerial("PWR1");
            //                else if (data == "OFF")
            //                    sendSerial("PWR0");
            //                else if (data == "BLANK")
            //                    sendSerial("BLK1");
            //                else if (data == "DISPLAY")
            //                    sendSerial("BLK0");

            break;
        }

        case RSYNC_COMMAND_LOADPRESENTIMAGE:
        {
            // QMessageBox::critical(0,"",datalist[1],0,0);
            QStringList pData = data.split(";");
            StelPresentMgr* sPmgr = GETSTELMODULE(StelPresentMgr);
            QStringList lst = pData[1].split("/");
            QString fmedia_path = conf->value("main/flat_media_path", "C:/").toString();
            QString myfmedia_path = fmedia_path +"/"+lst[lst.count()-1];
            //qDebug()<<"Flat File Name"<<myfmedia_path;
            if(QString(pData[11]).toInt())
                myfmedia_path = "";
            sPmgr->loadPresentImage(pData[0],
                    myfmedia_path,
                    QString(pData[2]).toDouble(),
                    QString(pData[3]).toDouble(),
                    QString(pData[4]).toDouble(),
                    QString(pData[5]).toDouble(),
                    QString(pData[6]).toDouble(),
                    QString(pData[7]).toDouble(),
                    QString(pData[8]).toInt(),
                    QString(pData[9]).toInt(),
                    QString(pData[10]).toDouble());

            if(QString(pData[11]).toInt()) //isVideo
            {
                lst = pData[12].split("/");
                QString fvideo_path = fmedia_path +"/"+lst[lst.count()-1];
                qDebug()<<"Video Flat File Name"<<fvideo_path;
                //QMessageBox::information(0,"",pData[12],0,0);
                StelApp::getInstance().setUseGLShaders(false);

                VideoClass* vfile= new VideoClass(this);
                //vfile->useYUVData = false;
                //vfile->setMarkIn(0);
                //vfile->setOptionRestartToMarkIn(false);
                vfile->setFlatVideo(true);
                vfile->OpenVideo(fvideo_path);//pData[12]); //presentPath+presentfilename

                vPresentfile.append(qMakePair(QString(pData[0]).toInt(),vfile));
            }

            if(pData.length()>12) // Mix with Sky
               sPmgr->setMixWithSky(pData[0],QString(pData[12]).toInt());

            break;
        }
        case RSYNC_COMMAND_PLAYPRESENTVIDEO:
        {
            QStringList pData = data.split(";");
            StelPresentMgr* sPmgr = GETSTELMODULE(StelPresentMgr);
            int presentID = QString(pData[0]).toInt();
            bool checked = QString(pData[1]).toInt();
            bool loop = QString(pData[2]).toInt();
            bool found = false;
            int foundPSID = 0;
            int foundPFID = 0;

            for (int i=0;i<vPresentsource.count();i++)
            {
                if(vPresentsource[i].first == presentID )
                {
                    found = true;
                    foundPSID = i;
                    break;
                }
            }

            for (int i=0;i<vPresentfile.count();i++)
            {
                if(vPresentfile[i].first == presentID )
                {
                    foundPFID = i;
                    break;
                }
            }

            if (!found)
            {
                VideoSource* videsource = new VideoSource(vPresentfile[foundPFID].second,sPmgr->getLayerTexture(QString::number(presentID)),QString::number(presentID));
                videsource->init(vPresentfile[foundPFID].second->GetVideoSize());
                sPmgr->SetLayerVideoSource(QString::number(presentID),videsource);
                vPresentsource.append(qMakePair(presentID,videsource));
                //vPresentfile[vPresentfile.count()-1].second->seekToPosition(0);
                //vPresentfile[vPresentfile.count()-1].second->setLoop(loop);
                vPresentsource[vPresentsource.count()-1].second->play(checked);
                //StartAudio(vPresentfile[vPresentfile.count()-1].second->getFileName(),checked);
            }
            else
            {
                //vPresentfile[foundPFID].second->seekToPosition(0);
                //vPresentfile[foundPFID].second->setLoop(loop);
                vPresentsource[foundPSID].second->play(checked);
                //StartAudio(vPresentfile[foundPSID].second->getFileName(),checked);
            }

            break;
        }
        case RSYNC_COMMAND_REMOVEPRESENTVIDEO:
        {
            StelPresentMgr* sPmgr = GETSTELMODULE(StelPresentMgr);
            int presentID = QString(data).toInt();
            bool found = false;
            int foundID = 0;

            for (int i=0;i<vPresentsource.count();i++)
            {
                if(vPresentsource[i].first == presentID )
                {
                    found = true;
                    foundID = i;
                    break;
                }
            }
            if (found)
            {
                vPresentsource[foundID].second->stop();// play(false);
                //StartAudio(vPresentfile[foundID].second->getFileName(),false);

                vPresentfile.removeAt(foundID);
                vPresentsource.removeAt(foundID);

                sPmgr->removePresentImage(QString::number(presentID));
            }

            break;
        }
        case RSYNC_COMMAND_PAUSEPRESENTVIDEO:
        {
            QStringList pData = data.split(";");
            int presentID = QString(pData[0]).toInt();
            bool checked = QString(pData[1]).toInt();
            bool found = false;
            int foundID = 0;

            for (int i=0;i<vPresentsource.count();i++)
            {
                if(vPresentsource[i].first == presentID )
                {
                    found = true;
                    foundID = i;
                    break;
                }
            }
            if (found)
            {
                vPresentsource[foundID].second->pause(checked);
                //toggle_pause();
            }
            break;
        }
        case RSYNC_COMMAND_RECORDVIDEO:
        {
            //QMessageBox::information(0,datalist[1],datalist[2],0,0);
            if(!StelMainWindow::getInstance().is_Licenced)
            {
                sImgr.setImageAlpha("logo", 0.5);
                sImgr.setImageAlpha("logo1", 0.5);
                sImgr.setImageAlpha("logo2", 0.5);
                sImgr.setImageAlpha("logo3", 0.5);
                sImgr.showImage("logo",true);
                sImgr.showImage("logo1",true);
                sImgr.showImage("logo2",true);
                sImgr.showImage("logo3",true);
            }
            StelMainGraphicsView::getInstance().startRecordFile(datalist[1],QString(datalist[2]).toInt());
            break;
        }
        case RSYNC_COMMAND_STOPRECVIDEO:
        {
            StelMainGraphicsView::getInstance().stopRecordFile();
            if(!StelMainWindow::getInstance().is_Licenced)
            {
                sImgr.showImage("logo",false);
                sImgr.showImage("logo1",false);
                sImgr.showImage("logo2",false);
                sImgr.showImage("logo3",false);
            }
            break;
        }
        case RSYNC_COMMAND_PAUSERECVIDEO:
        {
            StelMainGraphicsView::getInstance().pauseRecord(QString(datalist[1]).toInt());
            break;
        }
        case RSYNC_COMMAND_SPHERICMIRROR:
        {
            if (QString(datalist[1]).toInt() == 1)
            {
                StelApp::getInstance().setspUseCustomdata(QString(datalist[2]).toInt());
                StelApp::getInstance().setcustomDistortDataFile(datalist[3]);
                StelApp::getInstance().setDistortHorzMirror(QString(datalist[4]).toInt());
                StelApp::getInstance().setDistortVertMirror(QString(datalist[5]).toInt());

                //savedProjectionType = core->getCurrentProjectionType();
                //core->setCurrentProjectionType(StelCore::ProjectionFisheye);
                StelMainGraphicsView::getInstance().getStelAppGraphicsWidget()->setViewportEffect("sphericMirrorDistorter");
                core->currentProjectorParams.gravityLabels = conf->value("viewing/flag_gravity_labels").toBool();

            }
            else
            {
                //core->setCurrentProjectionType((StelCore::ProjectionType)savedProjectionType);
                //core->setCurrentProjectionType(StelCore::ProjectionFisheye);
                StelMainGraphicsView::getInstance().getStelAppGraphicsWidget()->setViewportEffect("none");
                core->currentProjectorParams.gravityLabels = conf->value("viewing/flag_gravity_labels").toBool();
            }
            break;
        }
        case RSYNC_COMMAND_DISKVIEWPORT:
        {
            if (QString(datalist[1]).toInt() == 1)
                StelApp::getInstance().getCore()->setMaskType(StelProjector::MaskDisk);
            else
                StelApp::getInstance().getCore()->setMaskType(StelProjector::MaskNone);
            break;
        }
        case RSYNC_COMMAND_SETGRAVITYLABELS:
        {
            if (QString(datalist[1]).toInt() == 1)
                StelApp::getInstance().getCore()->setFlagGravityLabels(true);
            else
                StelApp::getInstance().getCore()->setFlagGravityLabels(false);
            break;
        }
        case RSYNC_COMMAND_LOCKCLIENT:
        {
            StelMovementMgr* mmgr = GETSTELMODULE(StelMovementMgr);
            mmgr->setFlagEnableMoveKeys(QString(datalist[1]).toInt());
            mmgr->setFlagEnableMouseNavigation(QString(datalist[1]).toInt());
            mmgr->setFlagEnableZoomKeys(QString(datalist[1]).toInt());
            break;
        }
        case RSYNC_COMMAND_SATELLITE_SHOW_LABEL:
        {
            //QMessageBox::information(0,"",datalist[1],0,0);
            StelApp::getInstance().getGui()->getGuiActions("actionShow_Satellite_Labels")->setChecked(datalist[1].toInt());

            //Satellites* sat = GETSTELMODULE(Satellites);
            //sat->setFlagLabels(data.toInt());
            //Satellites::getInstance().setFlagLabels(data.toInt());
            break;
        }
        case RSYNC_COMMAND_SATELLITE_SHOW:
        {
            StelApp::getInstance().getGui()->getGuiActions("actionShow_Satellite_Show")->setChecked(datalist[1].toInt());
            //QMessageBox::information(0,"",data,0,0);
            //GETSTELMODULE(Satellites)->updateTLEs();
            //Satellites* s= GETSTELMODULE(Satellites);//->setFlagShow(data.toInt());
            //s->setTest(data.toInt());
            //GETSTELMODULE(Satellites)->setFlagLabels(data.toInt());
            break;
        }
        case RSYNC_COMMAND_SATELLITE_FONTSIZE:
        {
            //QMessageBox::information(0,"",datalist[1],0,0);
            Satellites* s= GETSTELMODULE(Satellites);//->setFlagShow(data.toInt());
            s->setLabelFontSize(datalist[1].toInt());
            break;
        }
        case RSYNC_COMMAND_COMPASSPLUGIN_MARK:
        {
            bool val = datalist[1].toInt();
            CompassMarks* c = GETSTELMODULE(CompassMarks);
            c->setCompassMarks(val);
            LandscapeMgr* lmgr = (LandscapeMgr*)GETSTELMODULE(LandscapeMgr);
            lmgr->setFlagCardinalsPoints(!val);
            //StelApp::getInstance().getGui()->getGuiActions("actionShow_Compass_Marks")->setChecked(val);
            //StelApp::getInstance().getGui()->getGuiActions("actionShow_Cardinal_Points")->setChecked(!val);
            break;
        }

        case RSYNC_COMMAND_DRAWFREE_ADD:
        {
            double viewportRes = conf->value("projection/viewport_res",2048 ).toDouble(); // Default deðer 2048 kabul edildi.

            double wServer = QString(datalist[2]).toDouble();
            double hServer = QString(datalist[3]).toDouble();
            double wClient = viewportRes;//StelMainGraphicsView::getInstance().viewport()->width();
            double hClient = viewportRes;//StelMainGraphicsView::getInstance().viewport()->height();

            //qDebug()<<"Freehand params:"<<wServer << hServer << wClient << hClient;


            int penSize =  QString(datalist[5]).toInt();
            double opacity = QString(datalist[6]).toDouble();
            QStringList pRGB = datalist[4].split("-");
            QPen pen = QPen(QColor(QString(pRGB[0]).toInt(),
                            QString(pRGB[1]).toInt(),
                    QString(pRGB[2]).toInt()));
            QVector<QPointF> points ;
            QStringList pPoints = datalist[1].split("-");
            for (int i = 1 ; i < pPoints.count(); i++)
            {
                if (pPoints[i]!="")
                {
                    QStringList pPoint =  pPoints[i].split(";");
                    double x = wClient / 2.0f - (wServer / 2.0f - QString(pPoint[1]).toDouble())* hClient / hServer;
                    double y = hClient* QString(pPoint[0]).toDouble() / hServer;
                    points.append(QPointF(x,y));
                }
            }
            penSize = hClient* penSize / hServer;
            QPolygonF polygons = QPolygonF(points);
            QPainterPath path = QPainterPath();
            path.addPolygon(polygons);
            StelAppGraphicsWidget::getInstance().smoothPath(path, 4);
            StelApp::getInstance().getCore()->freehandItems.append(freehandItemPtr( new freehandItem(path,
                                                                                                     pen,
                                                                                                     penSize,
                                                                                                     opacity)));
            break;
        }
        case RSYNC_COMMAND_DRAWFREE_DELETE:
        {
            int index =  QString(datalist[1]).toInt();
            core->freehandItems.erase(&StelApp::getInstance().getCore()->freehandItems[index]);
            break;
        }
        case RSYNC_COMMAND_DRAWFREE_DELETEALL:
        {
            core->freehandItems.clear();
            break;
        }
        case RSYNC_COMMAND_FINETUNE:
        {
            core->initWarpGL();
            int enable =  QString(datalist[1]).toInt();
            if (enable == 1)
            {
                StelMainGraphicsView::getInstance().setwarpMode("Distortion Warp Mode");

                Channel* pChannel = new Channel();

                pChannel->setinitFov(StelApp::getInstance().getCore()->getMovementMgr()->getInitFov());
                pChannel->setinitPos(StelApp::getInstance().getCore()->getMovementMgr()->getInitViewingDirection());
                pChannel->setWidth(StelMainGraphicsView::getInstance().viewport()->width());
                pChannel->setHeight(StelMainGraphicsView::getInstance().viewport()->height());

                if (StelMainGraphicsView::getInstance().m_pSelectedChannel)
                    pChannel = StelMainGraphicsView::getInstance().m_pSelectedChannel;
                StelMainGraphicsView::getInstance().m_pChannels.push_back(pChannel);

                StelMainGraphicsView::getInstance().m_pSelectedChannel = pChannel;
            }
            else
            {
                StelMainGraphicsView::getInstance().setwarpMode("Show Mode");
                StelMainGraphicsView::getInstance().setClientConf(false);
            }
            break;
        }
        case RSYNC_COMMAND_FINETUNE_SAVE:
        {
            QFile file("warpsettings.xml");
            if (file.open(QIODevice::WriteOnly|QIODevice::Truncate))
            {
                QTextStream out(&file);
                QDomDocument doc;

                QDomElement main = doc.createElement("ProjectorDistortion");
                main.setAttribute("version", 1.0);
                doc.appendChild(main);

                for (unsigned int i=0; i < StelMainGraphicsView::getInstance().m_pChannels.size(); ++i)
                {
                    main.appendChild( StelMainGraphicsView::getInstance().m_pChannels[i]->domElement("Channel",doc,false));
                }

                doc.save(out, 4);
                file.flush();
                file.close();
            }

            break;
        }
        case RSYNC_COMMAND_FINETUNE_RESET:
        {
            StelMainGraphicsView::getInstance().m_pSelectedChannel->getCurrentWarp()->reset();
            break;
        }
        case RSYNC_COMMAND_FLYBY_SETPLANET:
        {
            QString flybyPlanet = QString(datalist[1]);
            bool isInner = QString(datalist[2]).toInt();
            double value = 0;
            StelApp::getInstance().setFlyBy(flybyPlanet,value,isInner);

            break;
        }
        case RSYNC_COMMAND_FLYBY_SETPOS:
        {
            StelLocation loc = StelApp::getInstance().getCore()->getNavigator()->getCurrentLocation();
            loc.longitude = QString(datalist[1]).toFloat();
            loc.latitude = QString(datalist[2]).toFloat();
            double duration = QString(datalist[3]).toDouble();
            StelApp::getInstance().getCore()->getNavigator()->moveObserverTo(loc,duration, duration,true);
            break;
        }
        case RSYNC_COMMAND_FLYBY_SETALTITUDE:
        {
            QString planetName = QString(datalist[1]);
            SolarSystem* ssmgr = GETSTELMODULE(SolarSystem);
            PlanetP p = ssmgr->searchByEnglishNameForFlyby(planetName);
            if (p != NULL)
            {
                double newRadius = QString(datalist[2]).toDouble();
                p.data()->setRadiusforFader(newRadius);
                p.data()->setRadiusFader(true);
                StelMovementMgr* mvmgr = GETSTELMODULE(StelMovementMgr);
                Vec3d aim = Vec3d(QString(datalist[3]).toDouble(),
                        QString(datalist[4]).toDouble(),
                        QString(datalist[5]).toDouble());

                mvmgr->moveToJ2000(StelApp::getInstance().getCore()->getNavigator()->altAzToJ2000(aim), 1);
            }
            break;
        }
        case RSYNC_COMMAND_FLYBY_SETHOME:
        {
            StelApp::getInstance().goHome();
            break;
        }
        case RSYNC_COMMAND_FLYBY_SETLOC:
        {
            StelLocation loc = StelApp::getInstance().getCore()->getNavigator()->getCurrentLocation();

            loc.latitude = datalist[1].toDouble();
            loc.longitude = datalist[2].toDouble();

            StelApp::getInstance().getCore()->getNavigator()->moveObserverTo(loc, 0.);

            break;
        }
        case RSYNC_COMMAND_MESS_ADD:
        {
            QString messier = QString(datalist[1]);
            int iwidth = datalist[2].toInt();
            int iheight = datalist[3].toInt();
            QString btnName = datalist[4];
            StelApp::getInstance().addMessierObject(messier,
                                                    iwidth,
                                                    iheight,
                                                    btnName);
            break;
        }
        case RSYNC_COMMAND_MESS_REMOVE:
        {
            QString messier = QString(datalist[1]);
            StelApp::getInstance().removeMessierObject(messier);
            break;
        }
        case RSYNC_COMMAND_MESS_REMOVEALL:
        {
            StelPresentMgr* sPmgr = GETSTELMODULE(StelPresentMgr);
            foreach(messierimagePtr m, StelApp::getInstance().messierList)
            {
                sPmgr->removeWithFade(m.data()->id);
            }
            StelApp::getInstance().messierList.clear();
            break;
        }
        case RSYNC_COMMAND_PLANETS_ZOOM:
        {
            QString objname= datalist[1];
            int zoomin = datalist[2].toInt();
            StelApp::getInstance().startPlanetZoom(objname,zoomin);
            break;
        }
        case RSYNC_COMMAND_ILLUM:
        {
            int checked = datalist[1].toInt();
            QString color= datalist[2];
            if (checked)
            {
                if (color == "R")
                    StelApp::getInstance().getCore()->allFaderColor.set(1,0,0);
                else if (color == "G")
                    StelApp::getInstance().getCore()->allFaderColor.set(0,1,0);
                else if (color == "B")
                    StelApp::getInstance().getCore()->allFaderColor.set(0,0,1);
                else if (color == "W")
                    StelApp::getInstance().getCore()->allFaderColor.set(1,1,1);

                StelApp::getInstance().getCore()->ALLFader.setDuration(2000);
                StelApp::getInstance().getCore()->ALLFader = true;
            }
            else
            {
                StelApp::getInstance().getCore()->ALLFader.setDuration(2000);
                StelApp::getInstance().getCore()->ALLFader = false;
            }
            break;
        }
        case RSYNC_COMMAND_RESET_ALL:
        {
            StelApp::getInstance().getCore()->allFaderColor.set(0,0,0);
            StelApp::getInstance().getCore()->ALLFader.setDuration(2000);
            StelApp::getInstance().getCore()->ALLFader = true;

            resetAllTimer->start();
            break;
        }
        case RSYNC_COMMAND_VIDEO_COLORS:
        {
            LandscapeMgr* lmgr = GETSTELMODULE(LandscapeMgr);
            double brightness = datalist[1].toDouble();
            double contrast = datalist[2].toDouble();
            double saturation = datalist[3].toDouble();
            lmgr->setVideoBrightness(brightness);
            lmgr->setVideoContrast(contrast);
            lmgr->setVideoSaturation(saturation);
            break;
        }
        case RSYNC_COMMAND_END:
        {
            qApp->closeAllWindows();
            break;
        }
        }
    }
}

void rsync::displayNetworkError(QAbstractSocket::SocketError socketError)
{

    switch (socketError) {
    case QAbstractSocket::RemoteHostClosedError:
        break;
    case QAbstractSocket::HostNotFoundError:
        QMessageBox::information(NULL, qApp->applicationName(),
                                 tr("The host was not found. Please check the "
                                    "host name and port settings."));
        break;
    case QAbstractSocket::ConnectionRefusedError:
        QMessageBox::information(NULL, qApp->applicationName(),
                                 tr("The connection was refused by the peer. "
                                    "Make sure the Multichannnel Projection Designer server is running, "
                                    "and check that the host name and port settings are correct."));
        break;
    default:
        QMessageBox::information(NULL, qApp->applicationName(),
                                 tr("The following error occurred: %1.")
                                 .arg(m_pUdpClient->errorString()));
    }

}

QString getIpAdress()
{
    QList<QNetworkInterface> ifaces = QNetworkInterface::allInterfaces();
    bool result = false;

    for (int i = 0; i < ifaces.count(); i++)
    {
        QNetworkInterface iface = ifaces.at(i);
        if ( iface.flags().testFlag(QNetworkInterface::IsUp)
             && !iface.flags().testFlag(QNetworkInterface::IsLoopBack) )
        {
            // this loop is important
            for (int j=0; j<iface.addressEntries().count(); j++)
            {
                if(!iface.addressEntries().at(j).ip().toString().contains(':',Qt::CaseInsensitive))
                    return iface.addressEntries().at(j).ip().toString();
                //QMessageBox::critical(0,"2",iface.addressEntries().at(j).ip().toString()+"-"+iface.addressEntries().at(j).netmask().toString(),0,0);

            }
        }

    }
}

void rsync::sendInitdata(bool state)
{
#ifdef SHIRAPLAYER_PRE      
    return;
#endif

    if( state)
    {

        //QHostInfo hostinfo = QHostInfo::fromName(QHostInfo::localHostName());
        QString m_serverAddress = getIpAdress();

        //        for(int i = 0;i<hostinfo.addresses().count();i++)
        //        {
        //            QString str = hostinfo.addresses()[i].toString();
        //            if(!str.contains(':',Qt::CaseInsensitive))
        //            {
        //                m_serverAddress = str;
        //                break;
        //                //QMessageBox::critical(0,"",m_serverAddress,0,0);
        //            }
        //        }
        //QMessageBox::critical(0,"",m_serverAddress,0,0);
        QDomDocument doc;
        QDateTime timedelay = QDateTime::currentDateTime().addSecs(2);

        for (unsigned int i=0; i< StelMainGraphicsView::getInstance().m_pChannels.size(); ++i)
        {
            doc.appendChild(StelMainGraphicsView::getInstance().m_pChannels[i]->domElement("Channel",doc,true));
            StelApp::getInstance().getRsync()->sendChanges(RSYNC_COMMAND_INIT, doc.toString(0)+"@"+m_serverAddress+"@"+timedelay.toString("dd.MM.yyyy hh:mm:ss"));
            doc.clear();
#ifdef Q_OS_WIN
            Sleep(50);
#elif defined Q_OS_LINUX
            usleep(1);
#endif
        }
    }
    else
        StelApp::getInstance().getRsync()->sendChanges(RSYNC_COMMAND_INIT_OFF, "fadeoff");

}

void rsync::Gecikme(QString strdt)
{
    while (QDateTime::currentDateTime().toString("dd.MM.yyyy hh:mm:ss")!= strdt)
    {
        //Gecikme
        QCoreApplication::processEvents(QEventLoop::AllEvents );
    }
}

void rsync::dowloadFtpFile(QString ftpserver,QString filename,QString user,QString pass,int mod)
{
    //    ftp = new QFtp(this);
    //    connect(ftp, SIGNAL(commandFinished(int,bool)),this, SLOT(ftpCommandFinished(int,bool)));
    //    connect(ftp, SIGNAL(dataTransferProgress(qint64,qint64)), this, SLOT(updateDataTransferProgress(qint64,qint64)));

    //    downfilename = filename;

    //    ftp->connectToHost(ftpserver, 21);
    //    ftp->login(user,pass);
    //    downloadFile(mod);
}

void rsync::ftpCommandFinished(int, bool error)
{
    //    if (ftp->currentCommand() == QFtp::ConnectToHost) {
    //        if (error) {
    //            QMessageBox::information(0, tr("FTP"),
    //                                     tr("Unable to connect to the FTP server "
    //                                        "at %1. Please check that the host "
    //                                        "name is correct."));
    //            return;
    //        }
    //    }
    //    if (ftp->currentCommand() == QFtp::Login)
    //    {
    //        if (error) {
    //            QMessageBox::information(0, tr("FTP"),tr("Unable to connect to login FTP server"),0,0);
    //        }
    //    }
    //    if (ftp->currentCommand() == QFtp::Get) {
    //        if (error) {
    //            QMessageBox::information(0, tr("FTP"),tr("Canceled download of %1.")
    //                                     .arg(file->fileName(),0,0));
    //            file->close();
    //            file->remove();
    //        } else {
    //            file->close();
    //        }
    //        delete file;
    //        progressDialog->hide();
    //        emit(ftpfinished());
    //    }
}

void rsync::downloadFile(int mod)
{
    //    if(ftp_mod != 4)
    //        ftp_mod = mod;
    //    conf = StelApp::getInstance().getSettings();
    //    QString down_path;
    //    if(mod == 2)
    //        down_path = conf->value("main/media_path", "C:/").toString()+"/panorama";
    //    else if(mod == 1)
    //        down_path = conf->value("main/media_path", "C:/").toString();
    //    else if(mod == 3)
    //        down_path = qApp->applicationDirPath();

    //    if (mod == 3)
    //    {
    //        QStringList strlist = downfilename.split(".");
    //        file = new QFile(down_path +"/"+strlist[0]+"_update."+strlist[1]);
    //    }
    //    else
    //        file = new QFile(down_path +"/"+downfilename);

    //    if (!file->open(QIODevice::WriteOnly)) {
    //        QMessageBox::information(0, tr("FTP"),
    //                                 tr("Unable to save the file %1: %2.")
    //                                 .arg(downfilename).arg(file->errorString()));
    //        delete file;
    //        return;
    //    }

    //    if (mod == 2)
    //        ftp->get("panorama/"+downfilename, file);
    //    else
    //        ftp->get(downfilename, file);


    //    progressDialog->setLabelText(tr("Downloading %1...").arg(downfilename));
    //    progressDialog->exec();

}

void rsync::updateDataTransferProgress(qint64 readBytes, qint64 totalBytes)
{
    progressDialog->setMaximum(totalBytes);
    progressDialog->setValue(readBytes);
}

void rsync::doftpfinished()
{    
    if(ftp_mod == 3)
    {
        //QMessageBox::critical(0,0,"","Bitti",0,0);
        QString filename = "libstelMain.dll";
        ftp_mod = 4;
        dowloadFtpFile(sftp_server,filename,sftp_prog_user,sftp_prog_pass,3);
    }
    else if (ftp_mod == 4)
    {
        QProcess *process = new QProcess(this);
        //QMessageBox::critical(0,"",qApp->applicationDirPath()+"/UpdateCacarium.exe",0,0);
        process->start(qApp->applicationDirPath()+"/UpdateCacarium.exe");
        qApp->exit(0);
    }
}

void rsync::saveCurrentViewOptions()
{
    QSettings* conf = StelApp::getInstance().getSettings();
    Q_ASSERT(conf);

    LandscapeMgr* lmgr = (LandscapeMgr*)GETSTELMODULE(LandscapeMgr);
    Q_ASSERT(lmgr);
    SolarSystem* ssmgr = (SolarSystem*)GETSTELMODULE(SolarSystem);
    Q_ASSERT(ssmgr);
    MeteorMgr* mmgr = (MeteorMgr*)GETSTELMODULE(MeteorMgr);
    Q_ASSERT(mmgr);
    StelSkyDrawer* skyd = StelApp::getInstance().getCore()->getSkyDrawer();
    Q_ASSERT(skyd);
    ConstellationMgr* cmgr = (ConstellationMgr*)GETSTELMODULE(ConstellationMgr);
    Q_ASSERT(cmgr);
    StarMgr* smgr = (StarMgr*)GETSTELMODULE(StarMgr);
    Q_ASSERT(smgr);
    NebulaMgr* nmgr = (NebulaMgr*)GETSTELMODULE(NebulaMgr);
    Q_ASSERT(nmgr);
    GridLinesMgr* glmgr = (GridLinesMgr*)GETSTELMODULE(GridLinesMgr);
    Q_ASSERT(glmgr);
    StelGui* gui = (StelGui*)GETSTELMODULE(StelGui);
    Q_ASSERT(gui);
    StelMovementMgr* mvmgr = (StelMovementMgr*)GETSTELMODULE(StelMovementMgr);
    Q_ASSERT(mvmgr);
    StelNavigator* nav = StelApp::getInstance().getCore()->getNavigator();
    Q_ASSERT(nav);
    //StelProjector* proj = StelApp::getInstance().getCore()->currentProjectorParams;
    //Q_ASSERT(proj);

    MilkyWay* milkyway = (MilkyWay*)GETSTELMODULE(MilkyWay);
    Q_ASSERT(milkyway);

    // view dialog / sky tab settings
    conf->setValue("stars/absolute_scale", skyd->getAbsoluteStarScale());
    conf->setValue("stars/relative_scale", skyd->getRelativeStarScale());
    conf->setValue("stars/flag_star_twinkle", skyd->getFlagTwinkle());
    conf->setValue("stars/star_twinkle_amount", skyd->getTwinkleAmount());
    conf->setValue("viewing/use_luminance_adaptation", skyd->getFlagLuminanceAdaptation());
    conf->setValue("astro/flag_planets", ssmgr->getFlagPlanets());
    conf->setValue("astro/flag_planets_hints", ssmgr->getFlagHints());
    conf->setValue("astro/flag_planets_orbits", ssmgr->getFlagOrbits());
    conf->setValue("astro/flag_light_travel_time", ssmgr->getFlagLightTravelTime());
    conf->setValue("viewing/flag_moon_scaled", ssmgr->getFlagMoonScale());
    conf->setValue("viewing/moon_scale", ssmgr->getMoonScale());
    conf->setValue("viewing/flag_planets_scaled", ssmgr->getFlagPlanetsScale());
    conf->setValue("viewing/merkur_scale", ssmgr->getMerkurScale());
    conf->setValue("viewing/venus_scale", ssmgr->getVenusScale());
    conf->setValue("viewing/mars_scale", ssmgr->getMarsScale());
    conf->setValue("viewing/jupiter_scale", ssmgr->getJupiterScale());
    conf->setValue("viewing/saturn_scale", ssmgr->getSaturnScale());
    conf->setValue("viewing/uranus_scale", ssmgr->getUranusScale());
    conf->setValue("viewing/neptun_scale", ssmgr->getNeptunScale());
    conf->setValue("viewing/pluto_scale", ssmgr->getPlutoScale());

    conf->setValue("astro/meteor_rate", mmgr->getZHR());

    // view dialog / markings tab settings
    conf->setValue("viewing/flag_azimutal_grid", glmgr->getFlagAzimuthalGrid());
    conf->setValue("viewing/flag_equatorial_grid", glmgr->getFlagEquatorGrid());
    conf->setValue("viewing/flag_equator_line", glmgr->getFlagEquatorLine());
    conf->setValue("viewing/flag_ecliptic_line", glmgr->getFlagEclipticLine());
    conf->setValue("viewing/flag_meridian_line", glmgr->getFlagMeridianLine());
    conf->setValue("viewing/flag_equatorial_J2000_grid", glmgr->getFlagEquatorJ2000Grid());
    conf->setValue("viewing/flag_cardinal_points", lmgr->getFlagCardinalsPoints());
    conf->setValue("viewing/flag_constellation_drawing", cmgr->getFlagLines());
    conf->setValue("viewing/flag_constellation_name", cmgr->getFlagLabels());
    conf->setValue("viewing/flag_constellation_boundaries", cmgr->getFlagBoundaries());
    conf->setValue("viewing/flag_constellation_art", cmgr->getFlagArt());
    conf->setValue("viewing/flag_constellation_isolate_selected",cmgr->getFlagIsolateSelected());
    conf->setValue("viewing/constellation_art_intensity", cmgr->getArtIntensity());
    conf->setValue("astro/flag_star_name", smgr->getFlagLabels());
    conf->setValue("stars/labels_amount", smgr->getLabelsAmount());
    conf->setValue("astro/flag_planets_labels", ssmgr->getFlagLabels());
    conf->setValue("astro/labels_amount", ssmgr->getLabelsAmount());
    conf->setValue("astro/nebula_hints_amount", nmgr->getHintsAmount());
    conf->setValue("astro/flag_nebula_name", nmgr->getFlagHints());
    conf->setValue("astro/milky_way_intensity",milkyway->getIntensity());
    //	conf->setValue("projection/type", StelApp::getInstance().getCore()->getProjection()->getCurrentMapping().getId());

    // view dialog / landscape tab settings
    lmgr->setDefaultLandscapeID(lmgr->getCurrentLandscapeID());
    conf->setValue("landscape/flag_landscape_sets_location", lmgr->getFlagLandscapeSetsLocation());
    conf->setValue("landscape/flag_landscape", lmgr->getFlagLandscape());
    conf->setValue("landscape/flag_atmosphere", lmgr->getFlagAtmosphere());
    conf->setValue("landscape/flag_fog", lmgr->getFlagFog());
    conf->setValue("stars/init_bortle_scale", StelApp::getInstance().getCore()->getSkyDrawer()->getBortleScale());

    // view dialog / starlore tab
    StelApp::getInstance().getSkyCultureMgr().setDefaultSkyCultureID(StelApp::getInstance().getSkyCultureMgr().getCurrentSkyCultureID());

    // Save default location
    StelApp::getInstance().getCore()->getNavigator()->setDefaultLocationID(StelApp::getInstance().getCore()->getNavigator()->getCurrentLocation().getID());
    //
    //        // configuration dialog / main tab
    //        /*
    //        QString langName = StelApp::getInstance().getLocaleMgr().getAppLanguage();
    //        conf->setValue("localization/app_locale", Translator::nativeNameToIso639_1Code(langName));
    //
    //        if (gui->getInfoPanel()->getInfoTextFilters() == (StelObject::InfoStringGroup)0)
    //                conf->setValue("gui/selected_object_info", "none");
    //        else if (gui->getInfoPanel()->getInfoTextFilters() == StelObject::InfoStringGroup(StelObject::ShortInfo))
    //                conf->setValue("gui/selected_object_info", "short");
    //        else
    //                conf->setValue("gui/selected_object_info", "all");
    //        */
    //
    //        // configuration dialog / navigation tab
    //        conf->setValue("navigation/flag_enable_zoom_keys", mvmgr->getFlagEnableZoomKeys());
    //        conf->setValue("navigation/flag_enable_mouse_navigation", mvmgr->getFlagEnableMouseNavigation());
    //        conf->setValue("navigation/flag_enable_move_keys", mvmgr->getFlagEnableMoveKeys());
    //        conf->setValue("navigation/startup_time_mode", nav->getStartupTimeMode());
    //        conf->setValue("navigation/today_time", nav->getInitTodayTime());
    //        conf->setValue("navigation/preset_sky_time", nav->getPresetSkyTime());
    ////        conf->setValue("navigation/init_fov", proj->getInitFov());

    // configuration dialog / tools tab
    //        conf->setValue("gui/flag_show_flip_buttons", gui->getFlagShowFlipButtons());
    //	conf->setValue("video/distorter", StelAppGraphicsScene::getInstance().getViewPortDistorterType());
    //	conf->setValue("projection/viewport", Projector::maskTypeToString(proj->getMaskType()));
    //	conf->setValue("viewing/flag_gravity_labels", proj->getFlagGravityLabels());
    //        conf->setValue("navigation/auto_zoom_out_resets_direction", mvmgr->getFlagAutoZoomOutResetsDirection());
    //	conf->setValue("gui/flag_mouse_cursor_timeout", StelAppGraphicsScene::getInstance().getFlagCursorTimeout());
    //	conf->setValue("gui/mouse_cursor_timeout", StelAppGraphicsScene::getInstance().getCursorTimeout());

    //	StelApp::getInstance().getCore()->getProjection()->setInitFov(StelApp::getInstance().getCore()->getProjection()->getFov());
    //	StelApp::getInstance().getCore()->getNavigation()->setInitViewDirectionToCurrent();

    //	// full screen and window size
    //	conf->setValue("video/fullscreen", StelMainWindow::getInstance().getFullScreen());
    //	if (!StelMainWindow::getInstance().getFullScreen())
    //	{
    //		conf->setValue("video/screen_w", StelMainWindow::getInstance().size().width());
    //		conf->setValue("video/screen_h", StelMainWindow::getInstance().size().height());
    //	}
    //
    //	updateConfigLabels();
}


// Seri port ile ilgili kodlar
//------------------------------------------
//static char *bratestrings[]={"50", "75", "110","134","150","200","300","600","1200","1800","2400","4800","9600","14400","19200","38400","56000","57600","76800","115200","128000","256000"};
//static char *flowstrings[]={"off", "hardware", "xonxoff"};
//static char *paritystrings[]={"none", "odd", "even","mark","space"};
//static char *dbstrings[]={"5","6","7","8"};
//static char *sbstrings[]={"1","1.5","2"};


//QString brateToStr(BaudRateType l){
//    return QString(bratestrings[(int)l]);
//}
//BaudRateType strTobRate(QString str){
//    int pos=-1;
//    while(bratestrings[++pos]!=0){
//        if(str==bratestrings[pos])
//            return ((BaudRateType)pos);
//    }
//    return BAUD19200; // return some default here
//}


//QString flowToStr(FlowType l){
//    return QString(flowstrings[(int)l]);
//}
//FlowType strToflow(QString str){
//    int pos=-1;
//    while(flowstrings[++pos]!=0){
//        if(str==flowstrings[pos])
//            return ((FlowType)pos);
//    }
//    return FLOW_OFF; // return some default here
//}

//QString parityToStr(ParityType l){
//    return QString(paritystrings[(int)l]);
//}
//ParityType strToparity(QString str){
//    int pos=-1;
//    while(paritystrings[++pos]!=0){
//        if(str==paritystrings[pos])
//            return ((ParityType)pos);
//    }
//    return PAR_NONE; // return some default here
//}

//QString dbToStr(DataBitsType l){
//    return QString(dbstrings[(int)l]);
//}
//DataBitsType strTodb(QString str){
//    int pos=-1;
//    while(dbstrings[++pos]!=0){
//        if(str==dbstrings[pos])
//            return ((DataBitsType)pos);
//    }
//    return DATA_8; // return some default here
//}

//QString sbToStr(StopBitsType l){
//    return QString(sbstrings[(int)l]);
//}
//StopBitsType strTosb(QString str){
//    int pos=-1;
//    while(sbstrings[++pos]!=0){
//        if(str==sbstrings[pos])
//            return ((StopBitsType)pos);
//    }
//    return STOP_1; // return some default here
//}


//void rsync::sendSerial(QString msg)
//{
//    msg  = "("+ msg +")";


//    BaudRateType btype =  strTobRate(conf->value("main/seriport_brate","19200").toString());
//    FlowType ftype = strToflow(conf->value("main/seriport_flow","off").toString());
//    ParityType ptype = strToparity(conf->value("main/seriport_parity","none").toString());
//    DataBitsType dbtype = strTodb(conf->value("main/seriport_databits","8").toString());
//    StopBitsType sbtype = strTosb(conf->value("main/seriport_stopbits","1").toString());


//    //QMessageBox::information(0,"Bilgi",msg+" Mesaj Gönderilecek",0,0);
//    QextSerialPort *serialPort= new QextSerialPort(conf->value("main/seriport_com","false").toString());

//    serialPort->setBaudRate(btype);
//    serialPort->setFlowControl(ftype);
//    serialPort->setParity(ptype);
//    serialPort->setDataBits(dbtype);
//    serialPort->setStopBits(sbtype);

//    if(!serialPort->open(QIODevice::ReadWrite))
//    {
//        QMessageBox::warning(0,"Uyarý",serialPort->errorString(),0,0);
//        return;
//    }
//    serialPort->write(msg.toLatin1().data());
//    serialPort->close();
//    delete serialPort;
//    serialPort = NULL;
//    //QMessageBox::information(0,"Bilgi","Mesaj Gönderildi",0,0);
//}

//----------------------------------------------------------------

//--- Shared Memory Komutlarý

void rsync::LoadIntoSharedMomory(QString text)
{
    if (sharedMem.isAttached())
    {
        sharedMem.detach();
    }
    if(text.length())
    {
        // load into shared memory
        QBuffer buffer;
        buffer.open(QBuffer::ReadWrite);
        QDataStream out(&buffer);
        out << text;
        int size = buffer.size();

        if (!sharedMem.create(size)) {
            return;
        }
        sharedMem.lock();
        char *to = (char*)sharedMem.data();
        const char *from = buffer.data().data();
        memcpy(to, from, qMin(sharedMem.size(), size));
        sharedMem.unlock();
    }

}

void rsync::recvSettingsSharedMemory(QString dataShared)
{
    StelCore* core = StelApp::getInstance().getCore();

    int command;
    QStringList datalist;
    QString data;

    datalist = dataShared.split("@");

    command = QString(datalist[0]).toInt();
    data    = datalist[1];
    //QMessageBox::critical(0,"client",dataShared,0,0);
    qWarning()<<"Client Command: "<< dataShared;

    switch (command)
    {
    case RSYNC_COMMAND_INIT:
    {
        //QMessageBox::critical(0,"",data,0,0);
        core->allFaderColor = Vec3f(0,0,0);
        core->ALLFader.setDuration(2000);
        core->ALLFader = false;


        QDomDocument doc;
        doc.setContent(data);
        QDomElement channel = doc.firstChildElement("Channel");
        QString channelName = channel.attribute("name");

        if(m_clientAddress == channelName)
        {

            StelMainGraphicsView::getInstance().m_pSelectedChannel->initFromDOMElement(channel);

            core->getMovementMgr()->setInitFov(StelMainGraphicsView::getInstance().m_pSelectedChannel->m_initfov);
            core->getMovementMgr()->zoomTo(StelMainGraphicsView::getInstance().m_pSelectedChannel->m_initfov, 0.2);

            core->getMovementMgr()->setViewDirectionJ2000(core->getNavigator()->altAzToJ2000(StelMainGraphicsView::getInstance().m_pSelectedChannel->m_init_view_pos));

            int w = StelMainGraphicsView::getInstance().m_pSelectedChannel->getWidth();
            int h = StelMainGraphicsView::getInstance().m_pSelectedChannel->getHeight();
            if (w == 0)
                w = 800;
            if (h == 0)
                h = 600;
            StelMainGraphicsView::getInstance().setGeometry(0,0,w,h);
            core->initWarpGL();
            StelApp::getInstance().set_fwr(StelMainGraphicsView::getInstance().m_pSelectedChannel->m_fwr);
            StelApp::getInstance().set_fhr(StelMainGraphicsView::getInstance().m_pSelectedChannel->m_fhr);
            core->setCurrentProjectionTypeKey("ProjectionStereographic");

        }

        break;
    }
    case RSYNC_COMMAND_INIT_OFF:
    {
        core->allFaderColor = Vec3f(0,0,0);
        core->ALLFader.setDuration(2000);
        core->ALLFader = true;
        break;
    }
    case RSYNC_COMMAND_SCRIPT:
    {
        //QMessageBox::critical(0,"client",data,0,0);
        StelMainGraphicsView::getInstance().getScriptMgr().runScriptNetwork(data);
        if (datalist[2] == "2")
        {
            core->ALLFader.setDuration(1000);
            core->ALLFader = false;
        }
        break;
    }
    case RSYNC_COMMAND_LIVE_SCRIPT:
    {
        if(data!="")
        {
            //QMessageBox::critical(0,"",data,0,0);
            StelScriptMgr& scriptMgr = StelMainGraphicsView::getInstance().getScriptMgr();
            if (!scriptMgr.scriptIsRunning())
                scriptMgr.runScriptNetwork(data);
        }
        else
        {
            StelMainGraphicsView::getInstance().getScriptMgr().stopScript();
        }
        break;
    }
    case RSYNC_COMMAND_CHANNEL:
    {
        QDomDocument doc;
        doc.setContent(data);
        QDomElement channel = doc.firstChildElement("Channel");
        QString channelName = channel.attribute("name");

        if(m_clientAddress == channelName)
        {
            StelMainGraphicsView::getInstance().m_pSelectedChannel->initFromDOMElement(channel);
            core->getMovementMgr()->setInitFov(StelMainGraphicsView::getInstance().m_pSelectedChannel->m_initfov);
            core->getMovementMgr()->zoomTo(StelMainGraphicsView::getInstance().m_pSelectedChannel->m_initfov, 0.2);

            core->getMovementMgr()->setViewDirectionJ2000(StelApp::getInstance().getCore()->getNavigator()->altAzToJ2000(StelMainGraphicsView::getInstance().m_pSelectedChannel->m_init_view_pos));
            StelApp::getInstance().set_fwr(StelMainGraphicsView::getInstance().m_pSelectedChannel->m_fwr);
            StelApp::getInstance().set_fhr(StelMainGraphicsView::getInstance().m_pSelectedChannel->m_fhr);
        }
        break;
    }
    case RSYNC_COMMAND_WARP_MODE:
    {
        StelMainGraphicsView::getInstance().setwarpMode(data);
        break;
    }

    case RSYNC_COMMAND_START_GRAPH:
    {
        //QMessageBox::critical(0,"",data,0,0);

        datalist = QString(data).split(";");
        //                .arg(ui->listFilesWidget->selectedItems()[0]->text())
        //                .arg(movie_width)
        //                .arg(movie_height)
        //                .arg(ui->tbarSes->value())
        //                .arg(timedelay.toString("dd.MM.yyyy hh:mm:ss"))
        //                .arg(m_serverAddress);
        //                .arg(ui->chpreWarped->isChecked());
        //                .arg(ui->chShowFPS->isChecked()));
        //                .arg(ui->chUseShaders->isChecked());
        //                .arg(ui->chsSkipNextFrame->isChecked();

        strfile = datalist[0];
        QFileInfo uDir(strfile);
#ifndef SHIRAPLAYER_PRE
        //Zaman eþitle
        //QMessageBox::critical(0,"",datalist[5],0,0);
        sntpclient* s = new sntpclient(datalist[5],true);
        s->connectserver();
        ///
#endif
        StelApp::getInstance().setUseGLShaders(datalist[8].toInt());
        LandscapeMgr* lmgr = GETSTELMODULE(LandscapeMgr);
        //todo
        /*
        if(lmgr->vop_curr)
        {
            lmgr->vop_curr->useYUVData = datalist[8].toInt();
            lmgr->vop_curr->skipNextFrame = datalist[9].toInt();
        }*/
        QString ispreWarped = datalist[6];
        if (ispreWarped == "1")
            savedprewarped = 1;

        QString showFPS = datalist[7];
        if (showFPS == "1")
            core->showFPS = true;
        else
            core->showFPS = false;

        withSound = datalist[10].toInt();

        if(!only_audio)
        {
            core->allFaderColor = Vec3f(0,0,0);
            core->ALLFader.setDuration(2000);
            core->ALLFader = true;

            //Þu anki landscape hafýzaya alýnýyor
            sLandscapeVisibleFlag = lmgr->getFlagLandscape();
            sLandscape = lmgr->getCurrentLandscapeName();
            old_isVideoLandscape = lmgr->isVideoLandscape;
            lmgr->isVideoLandscape = false;

            //Görünüm kaydediliyor
            //oldViewDirection = core->getMovementMgr()->getViewDirectionJ2000();
            oldViewDirection = core->getNavigator()->j2000ToAltAz(core->getMovementMgr()->getViewDirectionJ2000());
            //qDebug()<<"recorded frontsky:"<<oldViewDirection.toStringLonLat();
#ifndef SHIRAPLAYER_PRE
            if(lmgr->s_Name != "")
                lmgr->doClearLandscapeVideo();

            //Client da baþlat
            //Scripte ile Yapýlan hareket deðiþiklikleri geri alýnýyor.
            StelMovementMgr* mmgr = GETSTELMODULE(StelMovementMgr);
            mmgr->panView(-mmgr->rec_deltaAz,-mmgr->rec_deltaAlt);

            if (uDir.exists())
            {
                lmgr->s_Name = "";
                lmgr->doSetCurrentLandscapetoVideo(strfile);
                lmgr->setFlagLandscape(true);
            }
            lmgr->doStartVideo();
        }
#else
            startGraphTimer->start();
        }
#endif

#ifndef SHIRAPLAYER_PRE
        if(is_audio)
            if (uDir.exists())
                Start_Audio(strfile.toLatin1().data());


        Gecikme(datalist[4]);

        if(!only_audio)
        {
            //Tüm çizimler LandscapeMgr hariç kapatlýyor
            StelApp::getInstance().isVideoMode = true;
            old_showPropGui = StelApp::getInstance().showPropGui ;
            StelApp::getInstance().showPropGui = false;
            ///
            core->ALLFader = false;
        }

#endif
        break;
    }
    case RSYNC_COMMAND_STOP_AUDIO:
    {
        audiost->Stop_Audio();
        break;
    }
    case RSYNC_COMMAND_STOP_GRAPH:
    {
        stopGraphProc();
        break;
    }
    case RSYNC_COMMAND_PAUSE_GRAPH:
    {
        if(!only_audio)
        {
            LandscapeMgr* lmgr = GETSTELMODULE(LandscapeMgr);
            if(lmgr->vop_curr)
                lmgr->doPauseVideo(data.toInt());
        }
        //if(is_audio)
        //    toggle_pause();
        break;
    }
    case RSYNC_COMMAND_SEEK_GRAPH:
    {
        int64_t pos = (int64_t)data.toLong();
        double framerate = QString(datalist[2]).toDouble();
        LandscapeMgr* lmgr = GETSTELMODULE(LandscapeMgr);

        if(!only_audio)
        {
            if(lmgr->vop_curr)
                lmgr->vop_curr->SeekByFrame(pos);
        }

        /*if(is_audio)
        {
            //Burasý yazýlacak
            audioSeekToPosition(pos,framerate);
        }*/
        break;
    }
    case RSYNC_COMMAND_GRAPH_PERFORMANCE:
    {
        StelApp::getInstance().setUseGLShaders(data.toInt());
        LandscapeMgr* lmgr = GETSTELMODULE(LandscapeMgr);
        //todo
        /*
        if(lmgr->vop_curr)
            lmgr->vop_curr->useYUVData = data.toInt();
        */
        break;
    }
    case RSYNC_COMMAND_SHOW_FRAME:
    {
        datalist = QString(data).split(";");
        LandscapeMgr* lmgr = GETSTELMODULE(LandscapeMgr);
        if(datalist[0] != "")
        {
            if (datalist[2] == "1")
            {
                QString framepath = datalist[0];
                QFileInfo thePath(framepath);
                if(thePath.exists())
                {
                    lmgr->setFlagLandscape(false);
                    sLandscape = lmgr->getCurrentLandscapeName();
                    old_isVideoLandscape = lmgr->isVideoLandscape;
                    lmgr->doSetCurrentLandscapetoFrame(framepath,sLandscape,datalist[1].toInt(),datalist[3].toInt());
                    lmgr->setFlagLandscape(true);
                }
            }
            else
            {
                core->allFaderColor = Vec3f(0,0,0);
                core->ALLFader.setDuration(datalist[3].toInt()*1000);
                core->ALLFader = true;
                showFisheyeFrameTimer->setInterval(core->ALLFader.getDuration());
                if (oldViewDirection == Vec3d(0,0,0))
                    oldViewDirection = core->getMovementMgr()->getViewDirectionJ2000();
                showFisheyeFrameTimer->start();
                strfile = datalist[0];
                showWithDaylight =datalist[1].toInt();
            }

        }
        break;
    }
    case RSYNC_COMMAND_SHOW_FRAME_DAYLIGHT:
    {
        if(data !="")
        {
            LandscapeMgr* lmgr = GETSTELMODULE(LandscapeMgr);
            lmgr->setshowDaylight(data.toInt());
        }
        break;
    }
    case RSYNC_COMMAND_SHOW_CLEAR:
    {
        datalist = QString(data).split(";");
        bFlagLandscape = datalist[1].toInt();
        core->allFaderColor = Vec3f(0,0,0);
        core->ALLFader.setDuration(datalist[0].toInt()*1000);
        core->ALLFader = true;
        clearFramesTimer->setInterval(core->ALLFader.getDuration());
        clearFramesTimer->start();

        break;
    }
    case RSYNC_COMMAND_FTP:
    {
        int tab = datalist[1].toInt();
        dowloadFtpFile(datalist[2],datalist[3],datalist[4],datalist[5],tab);
        break;
    }
    case RSYNC_COMMAND_FTP_UPDPRG:
    {
        QString filename = "shiraplayer.exe";
        //libstelmain dowload etmek için gerekli
        sftp_server = datalist[1];
        sftp_prog_user = datalist[2];
        sftp_prog_pass = datalist[3];

        dowloadFtpFile(sftp_server ,filename,sftp_prog_user,sftp_prog_pass,3);
        break;
    }
    case RSYNC_COMMAND_RESTARTAPP:
    {
        QProcess *process = new QProcess(this);
        //QMessageBox::critical(0,"",qApp->applicationName(),0,0);
        process->startDetached(qApp->applicationName());
        qApp->exit(0);
        break;
    }
    case RSYNC_COMMAND_SAVESET:
    {
        saveCurrentViewOptions();
        break;
    }
    case RSYNC_COMMAND_SAVELOC:
    {
        StelLocation loc; //= locationFromFields();

        loc.planetName = datalist[1];
        loc.name = datalist[2];
        loc.latitude = QString("%0").arg(datalist[3]).toFloat();
        loc.longitude = QString("%0").arg(datalist[4]).toFloat();
        loc.altitude = QString("%0").arg(datalist[5]).toInt();
        loc.country = datalist[6];

        StelApp::getInstance().getLocationMgr().saveUserLocation(loc);

        break;
    }
    case RSYNC_COMMAND_SHOWCHNAME:
    {
        core->showchannel_name = data.toInt();
        break;
    }
    case RSYNC_COMMAND_SHUTDOWN_PC:
    {
        MySystemShutdown();
        break;
    }
    case RSYNC_COMMAND_VOLUME:
    {
        if(is_audio)
        {
            //todo
            //set_volume(data.toInt());
        }
        break;
    }
    case RSYNC_COMMAND_PROJECTOR:
    {
        //QMessageBox::critical(0,"",data,0,0);
        //        if (data == "ON")
        //            sendSerial("PWR1");
        //        else if (data == "OFF")
        //            sendSerial("PWR0");
        //        else if (data == "BLANK")
        //            sendSerial("BLK1");
        //        else if (data == "DISPLAY")
        //            sendSerial("BLK0");

        break;
    }

    case RSYNC_COMMAND_LOADPRESENTIMAGE:
    {
        // QMessageBox::critical(0,"",datalist[1],0,0);
        QStringList pData = data.split(";");
        StelPresentMgr* sPmgr = GETSTELMODULE(StelPresentMgr);
        sPmgr->loadPresentImage(pData[0],pData[1],
                QString(pData[2]).toDouble(),
                QString(pData[3]).toDouble(),
                QString(pData[4]).toDouble(),
                QString(pData[5]).toDouble(),
                QString(pData[6]).toDouble(),
                QString(pData[7]).toDouble(),
                QString(pData[8]).toInt(),
                QString(pData[9]).toInt(),
                QString(pData[10]).toDouble());

        if(QString(pData[11]).toInt()) //isVideo
        {
            //QMessageBox::information(0,"",pData[12],0,0);
            StelApp::getInstance().setUseGLShaders(false);

            VideoClass* vfile= new VideoClass(this);
            //vfile->useYUVData = false;
            //vfile->setMarkIn(0);
            //vfile->setOptionRestartToMarkIn(false);
            vfile->setFlatVideo(true);
            vfile->OpenVideo(pData[12]); //presentPath+presentfilename

            vPresentfile.append(qMakePair(QString(pData[0]).toInt(),vfile));
        }
        if(pData.length()>12) // Mix with Sky
           sPmgr->setMixWithSky(pData[0],QString(pData[12]).toInt());

        break;
    }
    case RSYNC_COMMAND_PLAYPRESENTVIDEO:
    {
        QStringList pData = data.split(";");
        StelPresentMgr* sPmgr = GETSTELMODULE(StelPresentMgr);
        int presentID = QString(pData[0]).toInt();
        bool checked = QString(pData[1]).toInt();
        bool loop = QString(pData[2]).toInt();
        bool found = false;
        int foundPSID = 0;
        int foundPFID = 0;

        for (int i=0;i<vPresentsource.count();i++)
        {
            if(vPresentsource[i].first == presentID )
            {
                found = true;
                foundPSID = i;
                break;
            }
        }

        for (int i=0;i<vPresentfile.count();i++)
        {
            if(vPresentfile[i].first == presentID )
            {
                foundPFID = i;
                break;
            }
        }

        if (!found)
        {
            VideoSource* videsource = new VideoSource(vPresentfile[foundPFID].second,sPmgr->getLayerTexture(QString::number(presentID)),QString::number(presentID));
            videsource->init(vPresentfile[foundPFID].second->GetVideoSize());
            sPmgr->SetLayerVideoSource(QString::number(presentID),videsource);
            vPresentsource.append(qMakePair(presentID,videsource));
            //vPresentfile[vPresentfile.count()-1].second->seekToPosition(0);
            //vPresentfile[vPresentfile.count()-1].second->setLoop(loop);
            vPresentsource[vPresentsource.count()-1].second->play(checked);
            //StartAudio(vPresentfile[vPresentfile.count()-1].second->getFileName(),checked);
        }
        else
        {
            //vPresentfile[foundPFID].second->seekToPosition(0);
            //vPresentfile[foundPFID].second->setLoop(loop);
            vPresentsource[foundPSID].second->play(checked);
            //StartAudio(vPresentfile[foundPSID].second->getFileName(),checked);
        }

        break;
    }
    case RSYNC_COMMAND_REMOVEPRESENTVIDEO:
    {
        StelPresentMgr* sPmgr = GETSTELMODULE(StelPresentMgr);
        int presentID = QString(data).toInt();
        bool found = false;
        int foundID = 0;

        for (int i=0;i<vPresentsource.count();i++)
        {
            if(vPresentsource[i].first == presentID )
            {
                found = true;
                foundID = i;
                break;
            }
        }
        if (found)
        {
            vPresentsource[foundID].second->stop();// play(false);

            vPresentfile.removeAt(foundID);
            vPresentsource.removeAt(foundID);

            sPmgr->removePresentImage(QString::number(presentID));
        }

        break;
    }
    case RSYNC_COMMAND_PAUSEPRESENTVIDEO:
    {
        QStringList pData = data.split(";");
        int presentID = QString(pData[0]).toInt();
        bool checked = QString(pData[1]).toInt();
        bool found = false;
        int foundID = 0;

        for (int i=0;i<vPresentsource.count();i++)
        {
            if(vPresentsource[i].first == presentID )
            {
                found = true;
                foundID = i;
                break;
            }
        }
        if (found)
        {
            vPresentsource[foundID].second->pause(checked);
            //toggle_pause();
        }
        break;
    }
    case RSYNC_COMMAND_RECORDVIDEO:
    {
        //QMessageBox::information(0,datalist[1],datalist[2],0,0);
        if(!StelMainWindow::getInstance().is_Licenced)
        {
            sImgr.setImageAlpha("logo", 0.5);
            sImgr.setImageAlpha("logo1", 0.5);
            sImgr.setImageAlpha("logo2", 0.5);
            sImgr.setImageAlpha("logo3", 0.5);
            sImgr.showImage("logo",true);
            sImgr.showImage("logo1",true);
            sImgr.showImage("logo2",true);
            sImgr.showImage("logo3",true);
        }
        StelMainGraphicsView::getInstance().startRecordFile(datalist[1],QString(datalist[2]).toInt());
        break;
    }
    case RSYNC_COMMAND_STOPRECVIDEO:
    {
        StelMainGraphicsView::getInstance().stopRecordFile();
        if(!StelMainWindow::getInstance().is_Licenced)
        {
            sImgr.showImage("logo",false);
            sImgr.showImage("logo1",false);
            sImgr.showImage("logo2",false);
            sImgr.showImage("logo3",false);
        }
        break;
    }
    case RSYNC_COMMAND_PAUSERECVIDEO:
    {
        StelMainGraphicsView::getInstance().pauseRecord(QString(datalist[1]).toInt());
        break;
    }
    case RSYNC_COMMAND_SPHERICMIRROR:
    {
        if (QString(datalist[1]).toInt() == 1)
        {
            StelApp::getInstance().setspUseCustomdata(QString(datalist[2]).toInt());
            StelApp::getInstance().setcustomDistortDataFile(datalist[3]);
            StelApp::getInstance().setDistortHorzMirror(QString(datalist[4]).toInt());
            StelApp::getInstance().setDistortVertMirror(QString(datalist[5]).toInt());

            //savedProjectionType = core->getCurrentProjectionType();
            //core->setCurrentProjectionType(StelCore::ProjectionFisheye);
            StelMainGraphicsView::getInstance().getStelAppGraphicsWidget()->setViewportEffect("sphericMirrorDistorter");
            core->currentProjectorParams.gravityLabels = conf->value("viewing/flag_gravity_labels").toBool();

        }
        else
        {
            //core->setCurrentProjectionType((StelCore::ProjectionType)savedProjectionType);
            //core->setCurrentProjectionType(StelCore::ProjectionFisheye);
            StelMainGraphicsView::getInstance().getStelAppGraphicsWidget()->setViewportEffect("none");
            core->currentProjectorParams.gravityLabels = conf->value("viewing/flag_gravity_labels").toBool();
        }
        break;
    }
    case RSYNC_COMMAND_DISKVIEWPORT:
    {
        if (QString(datalist[1]).toInt() == 1)
            StelApp::getInstance().getCore()->setMaskType(StelProjector::MaskDisk);
        else
            StelApp::getInstance().getCore()->setMaskType(StelProjector::MaskNone);
        break;
    }
    case RSYNC_COMMAND_SETGRAVITYLABELS:
    {
        if (QString(datalist[1]).toInt() == 1)
            StelApp::getInstance().getCore()->setFlagGravityLabels(true);
        else
            StelApp::getInstance().getCore()->setFlagGravityLabels(false);
        break;
    }
    case RSYNC_COMMAND_LOCKCLIENT:
    {
        StelMovementMgr* mmgr = GETSTELMODULE(StelMovementMgr);
        mmgr->setFlagEnableMoveKeys(QString(datalist[1]).toInt());
        mmgr->setFlagEnableMouseNavigation(QString(datalist[1]).toInt());
        mmgr->setFlagEnableZoomKeys(QString(datalist[1]).toInt());
        break;
    }
    case RSYNC_COMMAND_SATELLITE_SHOW_LABEL:
    {
        //QMessageBox::information(0,"",datalist[1],0,0);
        StelApp::getInstance().getGui()->getGuiActions("actionShow_Satellite_Labels")->setChecked(datalist[1].toInt());
        
        //Satellites* sat = GETSTELMODULE(Satellites);
        //sat->setFlagLabels(data.toInt());
        //Satellites::getInstance().setFlagLabels(data.toInt());
        break;
    }
    case RSYNC_COMMAND_SATELLITE_SHOW:
    {
        StelApp::getInstance().getGui()->getGuiActions("actionShow_Satellite_Show")->setChecked(datalist[1].toInt());
        //QMessageBox::information(0,"",data,0,0);
        //GETSTELMODULE(Satellites)->updateTLEs();
        //Satellites* s= GETSTELMODULE(Satellites);//->setFlagShow(data.toInt());
        //s->setTest(data.toInt());
        //GETSTELMODULE(Satellites)->setFlagLabels(data.toInt());
        break;
    }
    case RSYNC_COMMAND_SATELLITE_FONTSIZE:
    {
        //QMessageBox::information(0,"",datalist[1],0,0);
        Satellites* s= GETSTELMODULE(Satellites);//->setFlagShow(data.toInt());
        s->setLabelFontSize(datalist[1].toInt());
        break;
    }
    case RSYNC_COMMAND_COMPASSPLUGIN_MARK:
    {
        bool val = datalist[1].toInt();
        CompassMarks* c = GETSTELMODULE(CompassMarks);
        c->setCompassMarks(val);
        LandscapeMgr* lmgr = (LandscapeMgr*)GETSTELMODULE(LandscapeMgr);
        lmgr->setFlagCardinalsPoints(!val);
        //StelApp::getInstance().getGui()->getGuiActions("actionShow_Compass_Marks")->setChecked(val);
        //StelApp::getInstance().getGui()->getGuiActions("actionShow_Cardinal_Points")->setChecked(!val);
        break;
    }
    case RSYNC_COMMAND_DRAWFREE_ADD:
    {
        double wServer = QString(datalist[2]).toDouble();
        double hServer = QString(datalist[3]).toDouble();
        double wClient = StelMainGraphicsView::getInstance().viewport()->width();
        double hClient = StelMainGraphicsView::getInstance().viewport()->height();
        int penSize =  QString(datalist[5]).toInt();
        double opacity = QString(datalist[6]).toDouble();
        QStringList pRGB = datalist[4].split("-");
        QPen pen = QPen(QColor(QString(pRGB[0]).toInt(),
                        QString(pRGB[1]).toInt(),
                QString(pRGB[2]).toInt()));
        QVector<QPointF> points ;
        QStringList pPoints = datalist[1].split("-");
        for (int i = 1 ; i < pPoints.count(); i++)
        {
            if (pPoints[i]!="")
            {
                QStringList pPoint =  pPoints[i].split(";");
                double x = wClient / 2.0f - (wServer / 2.0f - QString(pPoint[1]).toDouble())* hClient / hServer;
                double y = hClient* QString(pPoint[0]).toDouble() / hServer;
                points.append(QPointF(x,y));
            }
        }
        penSize = hClient* penSize / hServer;
        QPolygonF polygons = QPolygonF(points);
        QPainterPath path = QPainterPath();
        path.addPolygon(polygons);
        StelAppGraphicsWidget::getInstance().smoothPath(path, 4);
        StelApp::getInstance().getCore()->freehandItems.append(freehandItemPtr( new freehandItem(path,
                                                                                                 pen,
                                                                                                 penSize,
                                                                                                 opacity)));
        break;
    }
    case RSYNC_COMMAND_DRAWFREE_DELETE:
    {
        int index =  QString(datalist[1]).toInt();
        core->freehandItems.erase(&StelApp::getInstance().getCore()->freehandItems[index]);
        break;
    }
    case RSYNC_COMMAND_DRAWFREE_DELETEALL:
    {
        core->freehandItems.clear();
        break;
    }
    case RSYNC_COMMAND_FINETUNE:
    {
        core->initWarpGL();
        int enable =  QString(datalist[1]).toInt();
        if (enable == 1)
        {
            StelMainGraphicsView::getInstance().setwarpMode("Distortion Warp Mode");

            Channel* pChannel = new Channel();

            pChannel->setinitFov(StelApp::getInstance().getCore()->getMovementMgr()->getInitFov());
            pChannel->setinitPos(StelApp::getInstance().getCore()->getMovementMgr()->getInitViewingDirection());
            pChannel->setWidth(StelMainGraphicsView::getInstance().viewport()->width());
            pChannel->setHeight(StelMainGraphicsView::getInstance().viewport()->height());

            if (StelMainGraphicsView::getInstance().m_pSelectedChannel)
                pChannel = StelMainGraphicsView::getInstance().m_pSelectedChannel;
            StelMainGraphicsView::getInstance().m_pChannels.push_back(pChannel);

            StelMainGraphicsView::getInstance().m_pSelectedChannel = pChannel;
        }
        else
        {
            StelMainGraphicsView::getInstance().setwarpMode("Show Mode");
            StelMainGraphicsView::getInstance().setClientConf(false);
        }
        break;
    }
    case RSYNC_COMMAND_FINETUNE_SAVE:
    {
        QFile file("warpsettings.xml");
        if (file.open(QIODevice::WriteOnly|QIODevice::Truncate))
        {
            QTextStream out(&file);
            QDomDocument doc;

            QDomElement main = doc.createElement("ProjectorDistortion");
            main.setAttribute("version", 1.0);
            doc.appendChild(main);

            for (unsigned int i=0; i < StelMainGraphicsView::getInstance().m_pChannels.size(); ++i)
            {
                main.appendChild( StelMainGraphicsView::getInstance().m_pChannels[i]->domElement("Channel",doc,false));
            }

            doc.save(out, 4);
            file.flush();
            file.close();
        }

        break;
    }
    case RSYNC_COMMAND_FINETUNE_RESET:
    {
        StelMainGraphicsView::getInstance().m_pSelectedChannel->getCurrentWarp()->reset();
        break;
    }
    case RSYNC_COMMAND_FLYBY_SETPLANET:
    {
        QString flybyPlanet = QString(datalist[1]);
        bool isInner = QString(datalist[2]).toInt();
        double value = 0;
        StelApp::getInstance().setFlyBy(flybyPlanet,value,isInner);

        break;
    }
    case RSYNC_COMMAND_FLYBY_SETPOS:
    {
        StelLocation loc = StelApp::getInstance().getCore()->getNavigator()->getCurrentLocation();
        loc.longitude = QString(datalist[1]).toFloat();
        loc.latitude = QString(datalist[2]).toFloat();
        double duration = QString(datalist[3]).toDouble();
        StelApp::getInstance().getCore()->getNavigator()->moveObserverTo(loc,duration, duration,true);
        break;
    }
    case RSYNC_COMMAND_FLYBY_SETALTITUDE:
    {
        QString planetName = QString(datalist[1]);
        SolarSystem* ssmgr = GETSTELMODULE(SolarSystem);
        PlanetP p = ssmgr->searchByEnglishNameForFlyby(planetName);
        if (p != NULL)
        {
            double newRadius = QString(datalist[2]).toDouble();
            p.data()->setRadiusforFader(newRadius);
            p.data()->setRadiusFader(true);
            StelMovementMgr* mvmgr = GETSTELMODULE(StelMovementMgr);
            Vec3d aim = Vec3d(QString(datalist[3]).toDouble(),
                    QString(datalist[4]).toDouble(),
                    QString(datalist[5]).toDouble());

            mvmgr->moveToJ2000(StelApp::getInstance().getCore()->getNavigator()->altAzToJ2000(aim), 1);
        }
        break;
    }
    case RSYNC_COMMAND_FLYBY_SETHOME:
    {
        StelApp::getInstance().goHome();
        break;
    }
    case RSYNC_COMMAND_FLYBY_SETLOC:
    {
        StelLocation loc = StelApp::getInstance().getCore()->getNavigator()->getCurrentLocation();

        loc.latitude = datalist[1].toDouble();
        loc.longitude = datalist[2].toDouble();

        StelApp::getInstance().getCore()->getNavigator()->moveObserverTo(loc, 0.);

        break;
    }
    case RSYNC_COMMAND_MESS_ADD:
    {
        QString messier = QString(datalist[1]);
        int iwidth = datalist[2].toInt();
        int iheight = datalist[3].toInt();
        QString btnName = datalist[4];
        StelApp::getInstance().addMessierObject(messier,
                                                iwidth,
                                                iheight,
                                                btnName);
        break;
    }
    case RSYNC_COMMAND_MESS_REMOVE:
    {
        QString messier = QString(datalist[1]);
        StelApp::getInstance().removeMessierObject(messier);
        break;
    }
    case RSYNC_COMMAND_MESS_REMOVEALL:
    {
        StelPresentMgr* sPmgr = GETSTELMODULE(StelPresentMgr);
        foreach(messierimagePtr m, StelApp::getInstance().messierList)
        {
            sPmgr->removeWithFade(m.data()->id);
        }
        StelApp::getInstance().messierList.clear();
        break;
    }
    case RSYNC_COMMAND_CONST_SELSINGLE:
    {
        StelApp::getInstance().usingConstModule = true;
        ConstellationMgr * cmgr = GETSTELMODULE(ConstellationMgr);
        cmgr->setFlagIsolateSelected(true);

        QString nameArt = QString(datalist[1]);
        int isLine = QString(datalist[2]).toInt();
        int IsExistMultiCheck = QString(datalist[3]).toInt();
        int val = QString(datalist[4]).toInt();
        Constellation* co = dynamic_cast<Constellation*>(cmgr->searchByName(nameArt).data());
        if(co)
        {
            if (IsExistMultiCheck) cmgr->clearAllConst();
            if(val) cmgr->setSelectedConst(co);
            else cmgr->unsetSelectedConstWithoutAll(co);
            if(isLine)
            {
                co->setFlagArt(false);
                co->setFlagLines(val); //
            }
            else
            {
                co->setFlagLines(false);
                co->setFlagArt(val);//
            }
        }
        break;
    }
    case RSYNC_COMMAND_CONST_SELMULTI:
    {
        StelApp::getInstance().usingConstModule = true;
        ConstellationMgr * cmgr = GETSTELMODULE(ConstellationMgr);
        cmgr->setFlagIsolateSelected(true);
        QString multitype = datalist[1];
        int isLine = QString(datalist[2]).toInt();
        int val = QString(datalist[3]).toInt();
        int isName = QString(datalist[4]).toInt();

        if (multitype != "Cmd7")
            cmgr->clearAllConst();

        if(multitype == "Cmd0")
            cmgr->ShowSpringConst(val,isLine,isName);
        else if(multitype == "Cmd1")
            cmgr->ShowSummerConst(val,isLine,isName);
        else if(multitype == "Cmd2")
            cmgr->ShowAutumnConst(val,isLine,isName);
        else if(multitype == "Cmd3")
            cmgr->ShowWinterConst(val,isLine,isName);
        else if(multitype == "Cmd5")
            cmgr->ShowHideZodiac(val,isLine,isName);
        else if(multitype == "Cmd6")
            cmgr->ShowAllConst(val,isLine,isName);
        else if(multitype == "Cmd7")
            cmgr->ShowSelectedLabels(isName);

        break;
    }
    case RSYNC_COMMAND_CONST_ARTTOLINE:
    {
        StelApp::getInstance().usingConstModule = true;
        ConstellationMgr * cmgr = GETSTELMODULE(ConstellationMgr);
        cmgr->setFlagIsolateSelected(true);
        int val = QString(datalist[1]).toInt();
        cmgr->setFlagArt(!val);
        cmgr->setFlagLines(val);

        break;
    }
    case RSYNC_COMMAND_CONST_LINECOLOR:
    {
        StelApp::getInstance().usingConstModule = true;
        ConstellationMgr * cmgr = GETSTELMODULE(ConstellationMgr);
        cmgr->setFlagIsolateSelected(true);
        double r = QString(datalist[1]).toDouble();
        double g = QString(datalist[2]).toDouble();
        double b = QString(datalist[3]).toDouble();
        cmgr->setLinesColor(Vec3f(r,g,b));
        break;
    }
    case RSYNC_COMMAND_CONST_LINEWIDTH:
    {
        StelApp::getInstance().usingConstModule = true;
        ConstellationMgr * cmgr = GETSTELMODULE(ConstellationMgr);
        cmgr->setFlagIsolateSelected(true);
        int val = QString(datalist[1]).toInt();
        cmgr->setLinesWidth(val);
        break;
    }
    case RSYNC_COMMAND_STELLA_FRONTSKY:
    {
        StelCore* core = StelApp::getInstance().getCore();
        QString frontSky = datalist[1];
        double trackTime = QString(datalist[2]).toDouble();
        core->getMovementMgr()->setFrontViewDirection(frontSky,trackTime);
        break;
    }
    case RSYNC_COMMAND_STELLA_STARTRAILS:
    {
        int checked = datalist[1].toInt();
        int flagmaglevel = datalist[2].toInt();
        double magval = datalist[3].toDouble();

        StarMgr* smgr = GETSTELMODULE(StarMgr);
        smgr->setFlagTrails(checked);
        smgr->setFlagMagLevel(flagmaglevel);
        smgr->setlimitMagValue(magval);

        break;
    }
    case RSYNC_COMMAND_STELLA_STARMAGLIMITVAL:
    {
        double magval = datalist[1].toDouble();
        StarMgr* smgr = GETSTELMODULE(StarMgr);
        smgr->setlimitMagValue(magval);
        break;
    }
    case RSYNC_COMMAND_STELLA_ISSTARMAGLIMIT:
    {
        int flagmaglevel = datalist[1].toInt();
        double magval = datalist[2].toDouble();
        StarMgr* smgr = GETSTELMODULE(StarMgr);
        smgr->setFlagMagLevel(flagmaglevel);
        smgr->setlimitMagValue(magval);

        break;
    }
    case RSYNC_COMMAND_STELLA_HIDEALL:
    {
        core->allFaderColor = Vec3f(0,0,0);
        core->ALLFader.setDuration(2000);
        core->ALLFader = true;
        hideAllStella= QString(datalist[1]).toInt();
        stellaHideTimer->start();
        break;
    }
    case RSYNC_COMMAND_PLANETS_ZOOM:
    {
        QString objname= datalist[1];
        int zoomin = datalist[2].toInt();
        StelApp::getInstance().startPlanetZoom(objname,zoomin);
        //qDebug()<<objname<<zoomin;
        break;
    }
    case RSYNC_COMMAND_ILLUM:
    {
        int checked = datalist[1].toInt();
        QString color= datalist[2];
        if (checked)
        {
            if (color == "R")
                StelApp::getInstance().getCore()->allFaderColor.set(1,0,0);
            else if (color == "G")
                StelApp::getInstance().getCore()->allFaderColor.set(0,1,0);
            else if (color == "B")
                StelApp::getInstance().getCore()->allFaderColor.set(0,0,1);
            else if (color == "W")
                StelApp::getInstance().getCore()->allFaderColor.set(1,1,1);

            StelApp::getInstance().getCore()->ALLFader.setDuration(2000);
            StelApp::getInstance().getCore()->ALLFader = true;
        }
        else
        {
            StelApp::getInstance().getCore()->ALLFader.setDuration(2000);
            StelApp::getInstance().getCore()->ALLFader = false;
        }
        break;
    }
    case RSYNC_COMMAND_RESET_ALL:
    {
        StelApp::getInstance().getCore()->allFaderColor.set(0,0,0);
        StelApp::getInstance().getCore()->ALLFader.setDuration(2000);
        StelApp::getInstance().getCore()->ALLFader = true;

        resetAllTimer->start();
        break;
    }
    case RSYNC_COMMAND_VIDEO_COLORS:
    {
        LandscapeMgr* lmgr = GETSTELMODULE(LandscapeMgr);
        double brightness = datalist[1].toDouble();
        double contrast = datalist[2].toDouble();
        double saturation = datalist[3].toDouble();
        lmgr->setVideoBrightness(brightness);
        lmgr->setVideoContrast(contrast);
        lmgr->setVideoSaturation(saturation);
        break;
    }
    case RSYNC_COMMAND_END:
    {
        qApp->closeAllWindows();
        break;
    }
    }

}

void rsync::showFisheyeFrameBYTimer()
{
    //QString framepath = media_path+"/panorama/"+datalist[0];
    QString framepath = strfile;
    QFileInfo thePath(framepath);
    if(thePath.exists())
    {
        StelCore* core = StelApp::getInstance().getCore();
        LandscapeMgr* lmgr = GETSTELMODULE(LandscapeMgr);
        lmgr->setFlagLandscape(false);

        //Görünüm güney önde olacak þekle getiriliyor
        if (StelApp::getInstance().returntoFrontSky)
            core->getMovementMgr()->setViewDirectionJ2000(core->getNavigator()->altAzToJ2000(StelUtils::strToVec3f("0.01,0,1").toVec3d()));

        sLandscape = lmgr->getCurrentLandscapeName();
        old_isVideoLandscape = lmgr->isVideoLandscape;
        lmgr->doSetCurrentLandscapetoFrame(framepath,sLandscape,showWithDaylight,0);
        core->ALLFader = false;
        lmgr->setFlagLandscape(true);

    }

}

void rsync::clearFramesBYTimer()
{
    if(sLandscape != "")
    {
        StelCore* core = StelApp::getInstance().getCore();
        LandscapeMgr* lmgr = GETSTELMODULE(LandscapeMgr);

        //Görünüm eski þekline getiriliyor
        if (StelApp::getInstance().returntoFrontSky)
            core->getMovementMgr()->setViewDirectionJ2000(oldViewDirection);
        oldViewDirection = Vec3d(0,0,0);

        lmgr->doSetCurrentLandscapeName(sLandscape);
        //lmgr->setCurrentLandscapeName(sLandscape,false,old_isVideoLandscape);
        lmgr->setFlagLandscape(bFlagLandscape);
        sLandscape = "";
        core->ALLFader = false;
    }

}

void rsync::on_stellaHideTimer()
{
    StelMainWindow::getInstance().setStellaHide(hideAllStella);
    StelApp::getInstance().getCore()->ALLFader = false;
}

QString rsync::loadFromSharedMemConsole()
{
    if (!videoSharedMemConsole.attach())
    {
        return "";
    }

    QBuffer buffer;
    QDataStream in(&buffer);
    QString text;

    videoSharedMemConsole.lock();
    buffer.setData((char*)videoSharedMemConsole.constData(), videoSharedMemConsole.size());
    buffer.open(QBuffer::ReadOnly);
    in >> text;


    QBuffer bufferClear;
    bufferClear.open(QBuffer::ReadWrite);
    QDataStream out(&bufferClear);
    out << "";
    int size = bufferClear.size();

    //--clear data---
    char *to = (char*)videoSharedMemConsole.data();
    const char *from = bufferClear.data().data();
    memcpy(to, from, qMin(videoSharedMemConsole.size(), size));
    //----

    videoSharedMemConsole.unlock();
    videoSharedMemConsole.detach();

    return text ;

}

void rsync::trackConsoleTimer()
{
    QString text = loadFromSharedMemConsole();
    if( text != "")
    {
        QString data = text.replace("@","|");
        sendTcpData(RSYNC_COMMAND_CONSOLE_GETTRACKTIME ,data);
    }
}

void rsync::stopGraphProc()
{
    StelCore* core = StelApp::getInstance().getCore();
    if(!only_audio)
    {
        core->allFaderColor = Vec3f(0,0,0);
        core->ALLFader.setDuration(2000);
        core->ALLFader = true;
        stopGraphTimer->start();
    }
}

void rsync::stopGraphBYTimer()
{
    //qDebug()<<"video stop edildi";
    StelCore* core = StelApp::getInstance().getCore();
    //    core->ALLFader.setDuration(2000);
    //    core->ALLFader = true;

    //Görünüm eski þekline getiriliyor
    if (StelApp::getInstance().returntoFrontSky)
        core->getMovementMgr()->moveToAltAz(oldViewDirection);
    oldViewDirection = Vec3d(0,0,0);
    //---

    LandscapeMgr* lmgr = GETSTELMODULE(LandscapeMgr);

    //StelApp::getInstance().isVideoMode = false;
    lmgr->doClearLandscapeVideo();
    lmgr->setCurrentLandscapeName(sLandscape,false,old_isVideoLandscape);
    lmgr->setFlagLandscape(sLandscapeVisibleFlag);

    //Hareket deðiþikliði tekrar uygulanýyor.
    //StelMovementMgr* mmgr = GETSTELMODULE(StelMovementMgr);
    //mmgr->panView(mmgr->rec_deltaAz,mmgr->rec_deltaAlt);
    //
    StelApp::getInstance().showPropGui = old_showPropGui;

    if(savedprewarped == 1)
    {
        savedprewarped = 0;
        core->setCurrentProjectionType(StelCore::ProjectionFisheye);
        StelMainGraphicsView::getInstance().getStelAppGraphicsWidget()->setViewportEffect(savedViewportEffect);
        core->currentProjectorParams.gravityLabels = conf->value("viewing/flag_gravity_labels").toBool();
    }

    StelMainGraphicsView::getInstance().getOpenGLWin()->repaint();

    core->ALLFader = false;

}

void rsync::startGraphBYTimer()
{
    StelCore* core = StelApp::getInstance().getCore();
    LandscapeMgr* lmgr = GETSTELMODULE(LandscapeMgr);
    lmgr->fromScriptVideo = false;
    if(!only_audio)
    {
        //Þu anki landscape hafýzaya alýnýyor
        sLandscape = lmgr->getCurrentLandscapeName();
        old_isVideoLandscape = lmgr->isVideoLandscape;
        lmgr->isVideoLandscape = false;
        //if(lmgr->s_Name != "")
        //    lmgr->doClearLandscapeVideo();
        lmgr->setFlagLandscape(true);
        //Görünüm güney önde olacak þekle getiriliyor
        if (StelApp::getInstance().returntoFrontSky)
            core->getMovementMgr()->setViewDirectionJ2000(core->getNavigator()->altAzToJ2000(StelUtils::strToVec3f("0.01,0,1").toVec3d()));

    }
    //Client da baþlat
    //Scripte ile Yapýlan hareket deðiþiklikleri geri alýnýyor.
    //   StelMovementMgr* mmgr = GETSTELMODULE(StelMovementMgr);
    //   mmgr->panView(-mmgr->rec_deltaAz,-mmgr->rec_deltaAlt);


    QFileInfo uDir(strfile);
    if (uDir.exists())
    {
        lmgr->s_Name = "";
        lmgr->doSetCurrentLandscapetoVideo(strfile);
        //todo
        /*
        if ((is_audio) && (withSound ))
        {
            Load_Audio(strfile.toLatin1().data());
        }*/
    }

    if (savedprewarped  == 1)
    {
        savedViewportEffect = StelMainGraphicsView::getInstance().getStelAppGraphicsWidget()->getViewportEffect();
        core->setCurrentProjectionType(StelCore::ProjectionFisheye);
        StelMainGraphicsView::getInstance().getStelAppGraphicsWidget()->setViewportEffect("none");
    }

    if ((is_audio) && (withSound ))
    {
        //todo
        /*
        if (uDir.exists())
        {
            Start_Audio(strfile.toLatin1().data());
            qDebug()<<"Audio start:"<< QDateTime().currentDateTime().toString("hh:mm:ss.zzz");
        }*/
    }

    if(!only_audio)
    {
        //QObject::connect(lmgr->vop_curr, SIGNAL(endofVideo()), this, SLOT(stopGraphProc()));
        lmgr->doStartVideo();
        qDebug()<<"Video started:"<< QDateTime().currentDateTime().toString("hh:mm:ss.zzz");
        //Tüm çizimler LandscapeMgr hariç kapatlýyor
        //StelApp::getInstance().isVideoMode = true;
        old_showPropGui = StelApp::getInstance().showPropGui ;
        StelApp::getInstance().showPropGui = false;
        ///
        core->ALLFader = false;
    }

}

void rsync::StartAudio(QString fileName,bool checked)
{
    //todo
    /*
    if(checked)
    {
        Stop_Audio();
        Load_Audio(QString(fileName).toLatin1().data());
        Start_Audio(QString(fileName).toLatin1().data());
    }
    else
        Stop_Audio();*/
}

void rsync::resetAllProc()
{
    StelApp::getInstance().LoadInitOptions();
    StelApp::getInstance().getCore()->ALLFader = false;
}


//----
//****
//****
//****
//ShiraPlayer Console Commands
void rsync::newConnection()
{
    tcpsocket = m_tcpserver->nextPendingConnection();
    // BlockWriter(tcpsocket).stream() << QDir("C:/").entryList();
    // tcpsocket->flush();
    connect(tcpsocket,SIGNAL(readyRead()),this,SLOT(recvSettingsTCP()));
    StelApp::getInstance().getCore()->isConnectedFromTablet = true;

}

void rsync::sendTcpData(RSYNC_COMMAND command,const QString &data)
{
    BlockWriter(tcpsocket).stream() << QString::number(command)+"@"+data;
    tcpsocket->flush();
}

void rsync::sendTcpImage(RSYNC_COMMAND command, const QImage &image, const QString &type)
{
    QByteArray ba;
    QBuffer buffer(&ba);
    image.save(&buffer, "JPG");

    BlockWriter(tcpsocket).stream() << QString::number(command)+"@"+buffer.buffer().toHex()+"@"+type;
    tcpsocket->flush();
    //qDebug()<<QString::number(command)+"@"+Params+"@" +buffer.buffer().toHex();
}

void rsync::recvSettingsTCP()
{
    StelCore* core = StelApp::getInstance().getCore();

    int command;
    QStringList datalist;
    QString data;

    QString strData = QString(tcpsocket->readAll());
    datalist = strData.split("@");

    command = QString(datalist[0]).toInt();
    data    = datalist[1];

    //qWarning()<<"Client Command: "<< strData;
    switch (command)
    {
    case RSYNC_COMMAND_INIT:
    {
        //QMessageBox::critical(0,"",data,0,0);
        core->allFaderColor = Vec3f(0,0,0);
        core->ALLFader.setDuration(2000);
        core->ALLFader = false;


        QDomDocument doc;
        doc.setContent(data);
        QDomElement channel = doc.firstChildElement("Channel");
        QString channelName = channel.attribute("name");

        if(m_clientAddress == channelName)
        {
            StelMainGraphicsView::getInstance().m_pSelectedChannel->initFromDOMElement(channel);

            core->getMovementMgr()->setInitFov(StelMainGraphicsView::getInstance().m_pSelectedChannel->m_initfov);
            core->getMovementMgr()->zoomTo(StelMainGraphicsView::getInstance().m_pSelectedChannel->m_initfov, 0.2);

            core->getMovementMgr()->setViewDirectionJ2000(core->getNavigator()->altAzToJ2000(StelMainGraphicsView::getInstance().m_pSelectedChannel->m_init_view_pos));

            int w = StelMainGraphicsView::getInstance().m_pSelectedChannel->getWidth();
            int h = StelMainGraphicsView::getInstance().m_pSelectedChannel->getHeight();
            if (w == 0) w = 800;
            if (h == 0) h = 600;
            StelMainGraphicsView::getInstance().setGeometry(0,0,w,h);
            core->initWarpGL();
            StelApp::getInstance().set_fwr(StelMainGraphicsView::getInstance().m_pSelectedChannel->m_fwr);
            StelApp::getInstance().set_fhr(StelMainGraphicsView::getInstance().m_pSelectedChannel->m_fhr);
            core->setCurrentProjectionTypeKey("ProjectionStereographic");
        }

        break;
    }
    case RSYNC_COMMAND_INIT_OFF:
    {
        core->allFaderColor = Vec3f(0,0,0);
        core->ALLFader.setDuration(2000);
        core->ALLFader = true;
        break;
    }
    case RSYNC_COMMAND_SCRIPT:
    {
        //QMessageBox::critical(0,"client",data,0,0);
        StelMainGraphicsView::getInstance().getScriptMgr().runScriptNetwork(data);
        if (datalist.count() == 3)
            if (datalist[2] == "2")
            {
                core->ALLFader.setDuration(1000);
                core->ALLFader = false;
            }
        break;
    }
    case RSYNC_COMMAND_LIVE_SCRIPT:
    {
        if(data!="")
        {
            //QMessageBox::critical(0,"",data,0,0);
            StelScriptMgr& scriptMgr = StelMainGraphicsView::getInstance().getScriptMgr();
            if (!scriptMgr.scriptIsRunning())
                scriptMgr.runScriptNetwork(data);
        }
        else
        {
            StelMainGraphicsView::getInstance().getScriptMgr().stopScript();
        }
        break;
    }
    case RSYNC_COMMAND_CHANNEL:
    {
        QDomDocument doc;
        doc.setContent(data);
        QDomElement channel = doc.firstChildElement("Channel");
        QString channelName = channel.attribute("name");

        if(m_clientAddress == channelName)
        {
            StelMainGraphicsView::getInstance().m_pSelectedChannel->initFromDOMElement(channel);
            core->getMovementMgr()->setInitFov(StelMainGraphicsView::getInstance().m_pSelectedChannel->m_initfov);
            core->getMovementMgr()->zoomTo(StelMainGraphicsView::getInstance().m_pSelectedChannel->m_initfov, 0.2);

            core->getMovementMgr()->setViewDirectionJ2000(StelApp::getInstance().getCore()->getNavigator()->altAzToJ2000(StelMainGraphicsView::getInstance().m_pSelectedChannel->m_init_view_pos));
            StelApp::getInstance().set_fwr(StelMainGraphicsView::getInstance().m_pSelectedChannel->m_fwr);
            StelApp::getInstance().set_fhr(StelMainGraphicsView::getInstance().m_pSelectedChannel->m_fhr);
        }
        break;
    }
    case RSYNC_COMMAND_WARP_MODE:
    {
        StelMainGraphicsView::getInstance().setwarpMode(data);
        break;
    }

    case RSYNC_COMMAND_START_GRAPH:
    {
        //QMessageBox::critical(0,"",data,0,0);

        datalist = QString(data).split(";");
        //                .arg(ui->listFilesWidget->selectedItems()[0]->text())
        //                .arg(movie_width)
        //                .arg(movie_height)
        //                .arg(ui->tbarSes->value())
        //                .arg(timedelay.toString("dd.MM.yyyy hh:mm:ss"))
        //                .arg(m_serverAddress);
        //                .arg(ui->chpreWarped->isChecked());
        //                .arg(ui->chShowFPS->isChecked()));
        //                .arg(ui->chUseShaders->isChecked());
        strfile = media_path +"/" + datalist[0];
        QFileInfo uDir(strfile);
#ifndef SHIRAPLAYER_PRE
        //Zaman eþitle
        //QMessageBox::critical(0,"",datalist[5],0,0);
        sntpclient* s = new sntpclient(datalist[5],true);
        s->connectserver();
        ///
#endif
        StelApp::getInstance().setUseGLShaders(datalist[8].toInt());
        LandscapeMgr* lmgr = GETSTELMODULE(LandscapeMgr);
        //todo
        /*
        if(lmgr->vop_curr)
            lmgr->vop_curr->useYUVData = datalist[8].toInt();*/

        QString ispreWarped = datalist[6];
        if (ispreWarped == "1")
            savedprewarped = 1;

        QString showFPS = datalist[7];
        if (showFPS == "1")
            core->showFPS = true;
        else
            core->showFPS = false;

        if (datalist.count()>10)
            withSound = datalist[10].toInt();
        else
            withSound = true;

        if(!only_audio)
        {
            core->allFaderColor = Vec3f(0,0,0);
            core->ALLFader.setDuration(2000);
            core->ALLFader = true;

            //Þu anki landscape hafýzaya alýnýyor
            sLandscapeVisibleFlag = lmgr->getFlagLandscape();
            sLandscape = lmgr->getCurrentLandscapeName();
            old_isVideoLandscape = lmgr->isVideoLandscape;
            lmgr->isVideoLandscape = false;

            //Görünüm kaydediliyor
            //oldViewDirection = core->getMovementMgr()->getViewDirectionJ2000();
            oldViewDirection = core->getNavigator()->j2000ToAltAz(core->getMovementMgr()->getViewDirectionJ2000());


#ifndef SHIRAPLAYER_PRE
            if(lmgr->s_Name != "")
                lmgr->doClearLandscapeVideo();

            //Client da baþlat
            //Scripte ile Yapýlan hareket deðiþiklikleri geri alýnýyor.
            StelMovementMgr* mmgr = GETSTELMODULE(StelMovementMgr);
            mmgr->panView(-mmgr->rec_deltaAz,-mmgr->rec_deltaAlt);

            if (uDir.exists())
            {
                lmgr->s_Name = "";
                lmgr->doSetCurrentLandscapetoVideo(strfile);
                lmgr->setFlagLandscape(true);
            }
            lmgr->doStartVideo();
        }
#else
            startGraphTimer->start();
            trackTimerConsole->start(); // Android Console için tracktime
        }
#endif

#ifndef SHIRAPLAYER_PRE
        if(is_audio)
            if (uDir.exists())
                Start_Audio(strfile.toLatin1().data());


        Gecikme(datalist[4]);

        if(!only_audio)
        {
            //Tüm çizimler LandscapeMgr hariç kapatlýyor
            StelApp::getInstance().isVideoMode = true;
            old_showPropGui = StelApp::getInstance().showPropGui ;
            StelApp::getInstance().showPropGui = false;
            ///
            core->ALLFader = false;
        }

#endif
        break;
    }
    case RSYNC_COMMAND_STOP_AUDIO:
    {
        audiost->Stop_Audio();
        break;
    }
    case RSYNC_COMMAND_STOP_GRAPH:
    {
        stopGraphProc();
        break;
    }
    case RSYNC_COMMAND_PAUSE_GRAPH:
    {
        if(!only_audio)
        {
            LandscapeMgr* lmgr = GETSTELMODULE(LandscapeMgr);
            if(lmgr->vop_curr)
                lmgr->doPauseVideo(data.toInt());
        }
        //if(is_audio)
        //    toggle_pause();
        break;
    }
    case RSYNC_COMMAND_SEEK_GRAPH:
    {
        int64_t pos = (int64_t)data.toLong();
        double framerate = QString(datalist[2]).toDouble();
        LandscapeMgr* lmgr = GETSTELMODULE(LandscapeMgr);

        if(!only_audio)
        {
            if(lmgr->vop_curr)
                lmgr->vop_curr->SeekByFrame(pos);
        }

        /*if(is_audio)
        {
            //Burasý yazýlacak
            audioSeekToPosition(pos,framerate);
        }*/
        break;
    }
    case RSYNC_COMMAND_GRAPH_PERFORMANCE:
    {
        StelApp::getInstance().setUseGLShaders(data.toInt());
        LandscapeMgr* lmgr = GETSTELMODULE(LandscapeMgr);
        //todo
        /*if(lmgr->vop_curr)
            lmgr->vop_curr->useYUVData = data.toInt();*/
        break;
    }
    case RSYNC_COMMAND_SHOW_FRAME:
    {
        media_path = conf->value("main/media_path", "C:/").toString();
        media_path = media_path+"/panorama";

        datalist = QString(data).split(";");
        LandscapeMgr* lmgr = GETSTELMODULE(LandscapeMgr);
        if(datalist[0] != "")
        {
            if (datalist[2] == "1")
            {
                QString framepath =media_path +"/"+ datalist[0];
                QFileInfo thePath(framepath);
                if(thePath.exists())
                {
                    lmgr->setFlagLandscape(false);
                    sLandscape = lmgr->getCurrentLandscapeName();
                    old_isVideoLandscape = lmgr->isVideoLandscape;
                    lmgr->doSetCurrentLandscapetoFrame(framepath,sLandscape,datalist[1].toInt(),datalist[3].toInt());
                    lmgr->setFlagLandscape(true);
                }
            }
            else
            {
                core->allFaderColor = Vec3f(0,0,0);
                core->ALLFader.setDuration(datalist[3].toInt()*1000);
                core->ALLFader = true;
                showFisheyeFrameTimer->setInterval(core->ALLFader.getDuration());
                if (oldViewDirection == Vec3d(0,0,0))
                    oldViewDirection = core->getMovementMgr()->getViewDirectionJ2000();
                showFisheyeFrameTimer->start();
                strfile = media_path +"/"+datalist[0];
                showWithDaylight =datalist[1].toInt();
            }

        }
        break;
    }
    case RSYNC_COMMAND_SHOW_FRAME_DAYLIGHT:
    {
        if(data !="")
        {
            LandscapeMgr* lmgr = GETSTELMODULE(LandscapeMgr);
            lmgr->setshowDaylight(data.toInt());
        }
        break;
    }
    case RSYNC_COMMAND_SHOW_CLEAR:
    {
        LandscapeMgr* lmgr = GETSTELMODULE(LandscapeMgr);

        datalist = QString(data).split(";");
        bFlagLandscape = lmgr->getFlagLandscape();
        core->allFaderColor = Vec3f(0,0,0);
        core->ALLFader.setDuration(datalist[0].toInt()*1000);
        core->ALLFader = true;
        clearFramesTimer->setInterval(core->ALLFader.getDuration());
        clearFramesTimer->start();

        break;
    }
    case RSYNC_COMMAND_FTP:
    {
        int tab = datalist[1].toInt();
        dowloadFtpFile(datalist[2],datalist[3],datalist[4],datalist[5],tab);
        break;
    }
    case RSYNC_COMMAND_FTP_UPDPRG:
    {
        QString filename = "shiraplayer.exe";
        //libstelmain dowload etmek için gerekli
        sftp_server = datalist[1];
        sftp_prog_user = datalist[2];
        sftp_prog_pass = datalist[3];

        dowloadFtpFile(sftp_server ,filename,sftp_prog_user,sftp_prog_pass,3);
        break;
    }
    case RSYNC_COMMAND_RESTARTAPP:
    {
        QProcess *process = new QProcess(this);
        //QMessageBox::critical(0,"",qApp->applicationName(),0,0);
        process->startDetached(qApp->applicationName());
        qApp->exit(0);
        break;
    }
    case RSYNC_COMMAND_SAVESET:
    {
        saveCurrentViewOptions();
        break;
    }
    case RSYNC_COMMAND_SAVELOC:
    {
        StelLocation loc; //= locationFromFields();

        loc.planetName = datalist[1];
        loc.name = datalist[2];
        loc.latitude = QString("%0").arg(datalist[3]).toFloat();
        loc.longitude = QString("%0").arg(datalist[4]).toFloat();
        loc.altitude = QString("%0").arg(datalist[5]).toInt();
        loc.country = datalist[6];

        StelApp::getInstance().getLocationMgr().saveUserLocation(loc);

        break;
    }
    case RSYNC_COMMAND_SHOWCHNAME:
    {
        core->showchannel_name = data.toInt();
        break;
    }
    case RSYNC_COMMAND_SHUTDOWN_PC:
    {
        MySystemShutdown();
        break;
    }
    case RSYNC_COMMAND_VOLUME:
    {
        //todo
        /*
        if(is_audio)
        {
            set_volume(data.toInt());
        }*/
        break;
    }
    case RSYNC_COMMAND_PROJECTOR:
    {
        //QMessageBox::critical(0,"",data,0,0);
        //        if (data == "ON")
        //            sendSerial("PWR1");
        //        else if (data == "OFF")
        //            sendSerial("PWR0");
        //        else if (data == "BLANK")
        //            sendSerial("BLK1");
        //        else if (data == "DISPLAY")
        //            sendSerial("BLK0");

        break;
    }

    case RSYNC_COMMAND_LOADPRESENTIMAGE:
    {
        // QMessageBox::critical(0,"",datalist[1],0,0);
        QStringList pData = data.split(";");
        StelPresentMgr* sPmgr = GETSTELMODULE(StelPresentMgr);

        if(QString(pData[11]).toInt() == 0) //isImage
        {
            //qDebug()<<"ImagePath"<<pData[12];
            QPixmap ppx(pData[12]);
            double aspectratio = (double)ppx.width()/(double)ppx.height();
            ppx = QPixmap();

            sPmgr->loadPresentImage(pData[0],pData[1],
                    QString(pData[2]).toDouble(),
                    QString(pData[3]).toDouble(),
                    QString(pData[4]).toDouble(),
                    QString(pData[5]).toDouble(),
                    QString(pData[6]).toDouble(),
                    QString(pData[7]).toDouble(),
                    QString(pData[8]).toInt(),
                    QString(pData[9]).toInt(),
                    aspectratio);
            sendTcpData(RSYNC_COMMAND(command), QString("%0@%1").arg(pData[0])
                    .arg(aspectratio));
        }
        else //isVideo
        {

            //QMessageBox::information(0,"",pData[12],0,0);
            StelApp::getInstance().setUseGLShaders(false);

            VideoClass* vfile= new VideoClass(this);
            //vfile->useYUVData = false;
            //vfile->setMarkIn(0);
            //vfile->setOptionRestartToMarkIn(false);
            vfile->setFlatVideo(true);
            vfile->OpenVideo(pData[12]); //presentPath+presentfilename

            double aspectratio =vfile->GetAspectRatio();
            sPmgr->loadPresentImage(pData[0],pData[1],
                    QString(pData[2]).toDouble(),
                    QString(pData[3]).toDouble(),
                    QString(pData[4]).toDouble(),
                    QString(pData[5]).toDouble(),
                    QString(pData[6]).toDouble(),
                    QString(pData[7]).toDouble(),
                    QString(pData[8]).toInt(),
                    QString(pData[9]).toInt(),
                    aspectratio);

            vPresentfile.append(qMakePair(QString(pData[0]).toInt(),vfile));
            sendTcpData(RSYNC_COMMAND(command), QString("%0@%1").arg(pData[0])
                    .arg(aspectratio));

        }
        if(pData.length()>12) // Mix with Sky
           sPmgr->setMixWithSky(pData[0],QString(pData[12]).toInt());

        break;
    }
    case RSYNC_COMMAND_PLAYPRESENTVIDEO:
    {
        QStringList pData = data.split(";");
        StelPresentMgr* sPmgr = GETSTELMODULE(StelPresentMgr);
        int presentID = QString(pData[0]).toInt();
        bool checked = QString(pData[1]).toInt();
        bool loop = QString(pData[2]).toInt();
        bool found = false;
        int foundPSID = 0;
        int foundPFID = 0;

        for (int i=0;i<vPresentsource.count();i++)
        {
            if(vPresentsource[i].first == presentID )
            {
                found = true;
                foundPSID = i;
                break;
            }
        }

        for (int i=0;i<vPresentfile.count();i++)
        {
            if(vPresentfile[i].first == presentID )
            {
                foundPFID = i;
                break;
            }
        }

        if (!found)
        {
            VideoSource* videsource = new VideoSource(vPresentfile[foundPFID].second,sPmgr->getLayerTexture(QString::number(presentID)),QString::number(presentID));
            videsource->init(vPresentfile[foundPFID].second->GetVideoSize());
            sPmgr->SetLayerVideoSource(QString::number(presentID),videsource);
            vPresentsource.append(qMakePair(presentID,videsource));
            //vPresentfile[vPresentfile.count()-1].second->seekToPosition(0);
            //vPresentfile[vPresentfile.count()-1].second->setLoop(loop);
            vPresentsource[vPresentsource.count()-1].second->play(checked);
            //StartAudio(vPresentfile[vPresentfile.count()-1].second->getFileName(),checked);
        }
        else
        {
            //vPresentfile[foundPFID].second->seekToPosition(0);
            //vPresentfile[foundPFID].second->setLoop(loop);
            vPresentsource[foundPSID].second->play(checked);
            //StartAudio(vPresentfile[foundPSID].second->getFileName(),checked);
        }

        break;
    }
    case RSYNC_COMMAND_REMOVEPRESENTVIDEO:
    {
        StelPresentMgr* sPmgr = GETSTELMODULE(StelPresentMgr);
        int presentID = QString(data).toInt();
        bool found = false;
        int foundID = 0;

        for (int i=0;i<vPresentsource.count();i++)
        {
            if(vPresentsource[i].first == presentID )
            {
                found = true;
                foundID = i;
                break;
            }
        }
        if (found)
        {
            vPresentsource[foundID].second->stop();//play(false);
            //StartAudio(vPresentfile[foundID].second->getFileName(),false);

            vPresentfile.removeAt(foundID);
            vPresentsource.removeAt(foundID);

            sPmgr->removePresentImage(QString::number(presentID));
        }

        break;
    }
    case RSYNC_COMMAND_PAUSEPRESENTVIDEO:
    {
        QStringList pData = data.split(";");
        int presentID = QString(pData[0]).toInt();
        bool checked = QString(pData[1]).toInt();
        bool found = false;
        int foundID = 0;

        for (int i=0;i<vPresentsource.count();i++)
        {
            if(vPresentsource[i].first == presentID )
            {
                found = true;
                foundID = i;
                break;
            }
        }
        if (found)
        {
            vPresentsource[foundID].second->pause(checked);
            //toggle_pause();
        }
        break;
    }
    case RSYNC_COMMAND_RECORDVIDEO:
    {
        //QMessageBox::information(0,datalist[1],datalist[2],0,0);
        if(!StelMainWindow::getInstance().is_Licenced)
        {
            sImgr.setImageAlpha("logo", 0.5);
            sImgr.setImageAlpha("logo1", 0.5);
            sImgr.setImageAlpha("logo2", 0.5);
            sImgr.setImageAlpha("logo3", 0.5);
            sImgr.showImage("logo",true);
            sImgr.showImage("logo1",true);
            sImgr.showImage("logo2",true);
            sImgr.showImage("logo3",true);
        }
        StelMainGraphicsView::getInstance().startRecordFile(datalist[1],QString(datalist[2]).toInt());
        break;
    }
    case RSYNC_COMMAND_STOPRECVIDEO:
    {
        StelMainGraphicsView::getInstance().stopRecordFile();
        if(!StelMainWindow::getInstance().is_Licenced)
        {
            sImgr.showImage("logo",false);
            sImgr.showImage("logo1",false);
            sImgr.showImage("logo2",false);
            sImgr.showImage("logo3",false);
        }
        break;
    }
    case RSYNC_COMMAND_PAUSERECVIDEO:
    {
        StelMainGraphicsView::getInstance().pauseRecord(QString(datalist[1]).toInt());
        break;
    }
    case RSYNC_COMMAND_SPHERICMIRROR:
    {
        if (QString(datalist[1]).toInt() == 1)
        {
            StelApp::getInstance().setspUseCustomdata(QString(datalist[2]).toInt());
            StelApp::getInstance().setcustomDistortDataFile(datalist[3]);
            StelApp::getInstance().setDistortHorzMirror(QString(datalist[4]).toInt());
            StelApp::getInstance().setDistortVertMirror(QString(datalist[5]).toInt());

            savedProjectionType = core->getCurrentProjectionType();
            core->setCurrentProjectionType(StelCore::ProjectionFisheye);
            StelMainGraphicsView::getInstance().getStelAppGraphicsWidget()->setViewportEffect("sphericMirrorDistorter");
            core->currentProjectorParams.gravityLabels = conf->value("viewing/flag_gravity_labels").toBool();

        }
        else
        {
            //core->setCurrentProjectionType((StelCore::ProjectionType)savedProjectionType);
            core->setCurrentProjectionType(StelCore::ProjectionFisheye);
            StelMainGraphicsView::getInstance().getStelAppGraphicsWidget()->setViewportEffect("none");
            core->currentProjectorParams.gravityLabels = conf->value("viewing/flag_gravity_labels").toBool();
        }
        break;
    }
    case RSYNC_COMMAND_DISKVIEWPORT:
    {
        if (QString(datalist[1]).toInt() == 1)
            StelApp::getInstance().getCore()->setMaskType(StelProjector::MaskDisk);
        else
            StelApp::getInstance().getCore()->setMaskType(StelProjector::MaskNone);
        break;
    }
    case RSYNC_COMMAND_SETGRAVITYLABELS:
    {
        if (QString(datalist[1]).toInt() == 1)
            StelApp::getInstance().getCore()->setFlagGravityLabels(true);
        else
            StelApp::getInstance().getCore()->setFlagGravityLabels(false);
        break;
    }
    case RSYNC_COMMAND_LOCKCLIENT:
    {
        StelMovementMgr* mmgr = GETSTELMODULE(StelMovementMgr);
        mmgr->setFlagEnableMoveKeys(QString(datalist[1]).toInt());
        mmgr->setFlagEnableMouseNavigation(QString(datalist[1]).toInt());
        mmgr->setFlagEnableZoomKeys(QString(datalist[1]).toInt());
        break;
    }
    case RSYNC_COMMAND_SATELLITE_SHOW_LABEL:
    {
        //QMessageBox::information(0,"",datalist[1],0,0);
        StelApp::getInstance().getGui()->getGuiActions("actionShow_Satellite_Labels")->setChecked(datalist[1].toInt());

        //Satellites* sat = GETSTELMODULE(Satellites);
        //sat->setFlagLabels(data.toInt());
        //Satellites::getInstance().setFlagLabels(data.toInt());
        break;
    }
    case RSYNC_COMMAND_SATELLITE_SHOW:
    {
        StelApp::getInstance().getGui()->getGuiActions("actionShow_Satellite_Show")->setChecked(datalist[1].toInt());
        //QMessageBox::information(0,"",data,0,0);
        //GETSTELMODULE(Satellites)->updateTLEs();
        //Satellites* s= GETSTELMODULE(Satellites);//->setFlagShow(data.toInt());
        //s->setTest(data.toInt());
        //GETSTELMODULE(Satellites)->setFlagLabels(data.toInt());
        break;
    }
    case RSYNC_COMMAND_SATELLITE_FONTSIZE:
    {
        //QMessageBox::information(0,"",datalist[1],0,0);
        Satellites* s= GETSTELMODULE(Satellites);//->setFlagShow(data.toInt());
        s->setLabelFontSize(datalist[1].toInt());
        break;
    }
    case RSYNC_COMMAND_COMPASSPLUGIN_MARK:
    {
        bool val = datalist[1].toInt();
        CompassMarks* c = GETSTELMODULE(CompassMarks);
        c->setCompassMarks(val);
        LandscapeMgr* lmgr = (LandscapeMgr*)GETSTELMODULE(LandscapeMgr);
        lmgr->setFlagCardinalsPoints(!val);
        //StelApp::getInstance().getGui()->getGuiActions("actionShow_Compass_Marks")->setChecked(val);
        //StelApp::getInstance().getGui()->getGuiActions("actionShow_Cardinal_Points")->setChecked(!val);
        break;
    }
    case RSYNC_COMMAND_DRAWFREE_ADD:
    {
        double wServer = QString(datalist[2]).toDouble();
        double hServer = QString(datalist[3]).toDouble();
        double wClient = StelMainGraphicsView::getInstance().viewport()->width();
        double hClient = StelMainGraphicsView::getInstance().viewport()->height();
        int penSize =  QString(datalist[5]).toInt();
        double opacity = QString(datalist[6]).toDouble();
        QStringList pRGB = datalist[4].split("-");
        QPen pen = QPen(QColor( QString(pRGB[0]).toInt(),
                        QString(pRGB[1]).toInt(),
                QString(pRGB[2]).toInt()));
        QVector<QPointF> points ;
        QStringList pPoints = datalist[1].split("-");
        for (int i = 1 ; i < pPoints.count(); i++)
        {
            if (pPoints[i]!="")
            {
                QStringList pPoint =  pPoints[i].split(";");
                double x = wClient / 2.0f - (wServer / 2.0f - QString(pPoint[1]).toDouble())* hClient / hServer;
                double y = hClient* QString(pPoint[0]).toDouble() / hServer;
                points.append(QPointF(x,y));
            }
        }
        penSize = hClient* penSize / hServer;
        QPolygonF polygons = QPolygonF(points);
        QPainterPath path = QPainterPath();
        path.addPolygon(polygons);
        StelAppGraphicsWidget::getInstance().smoothPath(path, 4);
        StelApp::getInstance().getCore()->freehandItems.append(freehandItemPtr( new freehandItem(path,
                                                                                                 pen,
                                                                                                 penSize,
                                                                                                 opacity)));
        break;
    }
    case RSYNC_COMMAND_DRAWFREE_DELETE:
    {
        int index =  QString(datalist[1]).toInt();
        core->freehandItems.erase(&StelApp::getInstance().getCore()->freehandItems[index]);
        break;
    }
    case RSYNC_COMMAND_DRAWFREE_DELETEALL:
    {
        core->freehandItems.clear();
        break;
    }
    case RSYNC_COMMAND_FINETUNE:
    {
        core->initWarpGL();
        int enable =  QString(datalist[1]).toInt();
        if (enable == 1)
        {
            StelMainGraphicsView::getInstance().setwarpMode("Distortion Warp Mode");

            Channel* pChannel = new Channel();

            pChannel->setinitFov(StelApp::getInstance().getCore()->getMovementMgr()->getInitFov());
            pChannel->setinitPos(StelApp::getInstance().getCore()->getMovementMgr()->getInitViewingDirection());
            pChannel->setWidth(StelMainGraphicsView::getInstance().viewport()->width());
            pChannel->setHeight(StelMainGraphicsView::getInstance().viewport()->height());

            if (StelMainGraphicsView::getInstance().m_pSelectedChannel)
                pChannel = StelMainGraphicsView::getInstance().m_pSelectedChannel;
            StelMainGraphicsView::getInstance().m_pChannels.push_back(pChannel);

            StelMainGraphicsView::getInstance().m_pSelectedChannel = pChannel;
        }
        else
        {
            StelMainGraphicsView::getInstance().setwarpMode("Show Mode");
            StelMainGraphicsView::getInstance().setClientConf(false);
        }
        break;
    }
    case RSYNC_COMMAND_FINETUNE_SAVE:
    {
        QFile file("warpsettings.xml");
        if (file.open(QIODevice::WriteOnly|QIODevice::Truncate))
        {
            QTextStream out(&file);
            QDomDocument doc;

            QDomElement main = doc.createElement("ProjectorDistortion");
            main.setAttribute("version", 1.0);
            doc.appendChild(main);

            for (unsigned int i=0; i < StelMainGraphicsView::getInstance().m_pChannels.size(); ++i)
            {
                main.appendChild( StelMainGraphicsView::getInstance().m_pChannels[i]->domElement("Channel",doc,false));
            }

            doc.save(out, 4);
            file.flush();
            file.close();
        }

        break;
    }
    case RSYNC_COMMAND_FINETUNE_RESET:
    {
        StelMainGraphicsView::getInstance().m_pSelectedChannel->getCurrentWarp()->reset();
        break;
    }
    case RSYNC_COMMAND_CONSOLE_GETSTELLA:
    {
        QString uName,lcode ="";
        bool closeDesktop =  QString(datalist[1]).toInt();
        StelApp::getInstance().getCore()->isConnectedFromTabletFreeVersion = datalist[2].toInt();

        if (closeDesktop)
            CloseApplication(false);

        if(StelMainWindow::getInstance().is_Licenced)
        {
            uName = StelMainWindow::getInstance().uname;
            lcode = StelMainWindow::getInstance().lcode;
        }
        else
        {
            uName = "UnLicensed User";
            lcode = "";
        }

        //Stellarium basic durum kodlar? haz?rlan?p gönderilecek
        GridLinesMgr* glmgr = GETSTELMODULE(GridLinesMgr);
        ConstellationMgr* cmgr = GETSTELMODULE(ConstellationMgr);
        SolarSystem* ssmgr = GETSTELMODULE(SolarSystem);
        NebulaMgr* nmgr = GETSTELMODULE(NebulaMgr);
        StarMgr* smgr = GETSTELMODULE(StarMgr);
        Satellites* sat= GETSTELMODULE(Satellites);
        LandscapeMgr* lmgr = GETSTELMODULE(LandscapeMgr);

        bool satellitesEnabled = false;
        const QList<StelModuleMgr::PluginDescriptor> pluginsList = StelApp::getInstance().getModuleMgr().getPluginsList();
        foreach (const StelModuleMgr::PluginDescriptor& desc, pluginsList)
        {
            if (desc.info.id == "Satellites")
            {
                satellitesEnabled = desc.loaded;
            }
        }

        QString data = QString("%0;%1;%2;%3;%4;%5;%6;%7;%8;%9;%10;%11;%12;%13;%14;%15;%16;%17;%18")
                .arg(glmgr->getFlagEquatorGrid())
                .arg(glmgr->getFlagAzimuthalGrid())
                .arg(glmgr->getFlagEclipticLine())
                .arg(glmgr->getFlagMeridianLine())
                .arg(cmgr->getFlagLines())
                .arg(cmgr->getFlagLabels())
                .arg(cmgr->getFlagArt())
                .arg(cmgr->getFlagBoundaries())
                .arg(ssmgr->getFlagPlanets())
                .arg(nmgr->getFlagHints())
                .arg(smgr->getFlagLabels())
                .arg(lmgr->getFlagAtmosphere())
                .arg(lmgr->getFlagLandscape())
                .arg(lmgr->getFlagFog())
                .arg(conf->value("Satellites/show_satellites", false).toBool())
                .arg(satellitesEnabled)
                .arg(StelMainWindow::getInstance().is_Licenced)
                .arg(uName)
                .arg(lcode);

        sendTcpData(RSYNC_COMMAND(command),data);
        break;
    }
    case RSYNC_COMMAND_CONSOLE_GETTIME:
    {
        double jd = StelApp::getInstance().getCore()->getNavigator()->getJDay();
        double gmtshift = StelApp::getInstance().getLocaleMgr().getGMTShift(jd)/24.0;
        double timerate= core->getNavigator()->getTimeRate();

        QString data = QString("%0;%1;%2").arg(QString::number(jd, 'g', 16))
                .arg(gmtshift)
                .arg(timerate);
        sendTcpData(RSYNC_COMMAND(command),data);
        break;
    }
    case RSYNC_COMMAND_CONSOLE_GETLOCATION:
    {
        StelLocation loc= StelApp::getInstance().getCore()->getNavigator()->getCurrentLocation();
        QString data =
                QString("%0;%1;%2;%3;%4;%5;%6;%7;%8;%9")
                .arg(loc.altitude)
                .arg(loc.country)
                .arg(loc.getID())
                .arg(loc.isUserLocation)
                .arg(loc.landscapeKey)
                .arg(loc.latitude)
                .arg(loc.longitude)
                .arg(loc.name)
                .arg(loc.planetName)
                .arg(loc.state);

        sendTcpData(RSYNC_COMMAND(command),data);
        break;
    }
    case RSYNC_COMMAND_CONSOLE_GETFULLDOMELIST:
    {
        QString data ="";
        //Media Liste içeriði-----
        media_path = conf->value("main/media_path", "C:/").toString();

        QDir dirm(media_path);
        dirm.setFilter(QDir::Files | QDir::Hidden | QDir::NoSymLinks);
        //dirm.setSorting(QDir::Size | QDir::Reversed);
        dirm.setSorting(QDir::Name | QDir::Reversed);
        QStringList filtersm;
        filtersm << "*.avi" << "*.mpeg" << "*.mpg" << "*.mp4" << "*.wmv" << "*.mov" << "*.flv" << "*.vob"  ;
        dirm.setNameFilters(filtersm);
        QFileInfoList listm = dirm.entryInfoList();

        for (int i = 0; i < listm.size(); ++i)
        {
            QFileInfo fileInfom = listm.at(i);
            data = fileInfom.fileName()+ ";" + data;
        }

        data = QString("%0|%1|%2").arg(data)
                .arg(StelApp::getInstance().getUseGLShaders())
                .arg(conf->value("video/prewarped", false).toBool());
        sendTcpData(RSYNC_COMMAND(command),data);
        break;
    }
    case RSYNC_COMMAND_CONSOLE_GETFISHEYEIMAGELIST:
    {
        QString data ="";
        //Media Liste içeriði-----
        media_path = conf->value("main/media_path", "C:/").toString();
        media_path = media_path+"/panorama";

        QDir dirm(media_path);
        dirm.setFilter(QDir::Files | QDir::Hidden | QDir::NoSymLinks);
        dirm.setSorting(QDir::Name | QDir::Reversed);
        QStringList filtersm;
        filtersm <<"*.jpg" << "*.jpeg" << "*.png" << "*.bmp" << "*.tiff" << "*.gif" ;
        dirm.setNameFilters(filtersm);
        QFileInfoList listm = dirm.entryInfoList();

        for (int i = 0; i < listm.size(); ++i)
        {
            QFileInfo fileInfom = listm.at(i);
            data = fileInfom.fileName()+ ";" + data;
        }

        data = QString("%0|%1").arg(data)
                .arg(conf->value("video/frame_withdaylight",false).toBool());
        sendTcpData(RSYNC_COMMAND(command),data);
        break;

    }
    case RSYNC_COMMAND_CONSOLE_GETFLATLIST:
    {
        QString data ="";
        QString fmedia_path = conf->value("main/flat_media_path", "C:/").toString();

        QDir dirm(fmedia_path);
        dirm.setFilter(QDir::Files | QDir::Hidden | QDir::NoSymLinks);
        dirm.setSorting(QDir::Name | QDir::Reversed);
        QStringList filtersm;

        filtersm << "*.avi" << "*.mpeg" << "*.mpg" << "*.mp4" << "*.wmv" << "*.mov" << "*.flv" << "*.vob"  ;
        dirm.setNameFilters(filtersm);
        QFileInfoList listm  = dirm.entryInfoList();

        for (int i = 0; i < listm.size(); ++i)
        {
            QFileInfo fileInfom = listm.at(i);
            data = fileInfom.fileName()+"|"+"video"+ ";" + data;
        }

        filtersm.clear();
        filtersm << "*.bmp" << "*.png" << "*.jpg";
        dirm.setNameFilters(filtersm);
        listm = dirm.entryInfoList();

        for (int i = 0; i < listm.size(); ++i)
        {
            QFileInfo fileInfom = listm.at(i);
            data = fileInfom.fileName()+"|"+"image"+ ";" + data;
        }

        sendTcpData(RSYNC_COMMAND(command), fmedia_path + "@"+ data);

        break;
    }
    case RSYNC_COMMAND_CONSOLE_GETONEIMAGE:
    {
        media_path ="";
        if (datalist[2] == "panorama")
        {
            media_path = conf->value("main/media_path", "C:/").toString();
            media_path = media_path+"/panorama";
        }
        else if (datalist[2] == "flat")
        {
            media_path =  conf->value("main/flat_media_path", "C:/").toString();
        }
        QString path = media_path+ "/"+ datalist[1];
        if (0 == access(path.toLatin1(), 0))
        {
            QImageReader imageReader(path);
            QSize size;
            int image_width = 96;
            int image_height = 96;

            imageReader.setScaledSize(QSize(image_width, image_height));
            QImage image = imageReader.read();
            image.setText("name",datalist[1]);
            image.setText("path",media_path);
            sendTcpImage(RSYNC_COMMAND(command),image,datalist[2]);
        }
        else
            qDebug()<<"File Not exists: (For send to Console )"+ datalist[1];

        break;
    }
    case RSYNC_COMMAND_CONSOLE_GETSCRIPTLIST:
    {
        QString data ="";
        //Favorit List box içeriði
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
            data = fileInfo.fileName()+ ";" + data;
        }
        sendTcpData(RSYNC_COMMAND(command),data);
        break;
    }
    case RSYNC_COMMAND_CONSOLE_RUNSCRIPT:
    {
        QString textScript ="";
        QString fileName = qApp->applicationDirPath()+"/favorits/"+datalist[1];
        QFile file(fileName);
        if (file.open(QIODevice::ReadOnly))
        {
            textScript = file.readAll();
            file.close();
        }
        if (textScript != "")
            StelMainGraphicsView::getInstance().getScriptMgr().runScriptNetwork(textScript);
        break;
    }
    case RSYNC_COMMAND_CONSOLE_GETPREVIEW:
    {
        QImage img = StelMainGraphicsView::getInstance().getOpenGLWin()->grabFrameBuffer();
        img = img.scaled(512,512);
        sendTcpImage(RSYNC_COMMAND(command),img,"preview");
        break;
    }
    case RSYNC_COMMAND_CONSOLE_CLOSEALL:
    {
        CloseApplication(true);
        break;
    }
    case RSYNC_COMMAND_FLYBY_SETPLANET:
    {
        QString flybyPlanet = QString(datalist[1]);
        bool isInner = QString(datalist[2]).toInt();
        bool showOrbit = QString(datalist[3]).toInt();
        double value = 0;
        StelApp::getInstance().setFlyBy(flybyPlanet,value,isInner);
        SolarSystem* ssmgr = GETSTELMODULE(SolarSystem);
        ssmgr->setFlagOrbits(showOrbit);
        break;
    }
    case RSYNC_COMMAND_FLYBY_SETPOS:
    {
        double velocitySlider = QString(datalist[1]).toFloat();

        if (realtrackValue < 0)
            realtrackValue = -1 * velocitySlider;
        else
            realtrackValue = velocitySlider;

        double trackValue = realtrackValue;
        StelLocation loc = StelApp::getInstance().getCore()->getNavigator()->getCurrentLocation();

        if( loc.latitude > 90.0 )  { realtrackValue = -1 * realtrackValue ;trackValue = realtrackValue; }
        if( loc.latitude < -90.0 ) { realtrackValue = -1 * realtrackValue ;trackValue = realtrackValue; }

        loc.longitude = loc.longitude + qAbs(trackValue);
        loc.latitude = loc.latitude + realtrackValue / 10.0;
        //StelApp::getInstance().getCore()->getNavigator()->moveObserverTo(loc, 2.0, 2.0,true);

        //calcVelocity();

        //StelLocation loc = StelApp::getInstance().getCore()->getNavigator()->getCurrentLocation();
        //loc.longitude = QString(datalist[1]).toFloat();
        //loc.latitude = QString(datalist[2]).toFloat();
        double duration = 2.0 ;//QString(datalist[3]).toDouble();
        StelApp::getInstance().getCore()->getNavigator()->moveObserverTo(loc,duration, duration,true);
        break;
    }
    case RSYNC_COMMAND_FLYBY_SETALTITUDE:
    {
        QString planetName = QString(datalist[1]);
        double val = QString(datalist[2]).toDouble();
        double max = QString(datalist[3]).toDouble();

        StelApp::getInstance().setAltitudebyConsole(planetName,val,max);
        break;
    }
    case RSYNC_COMMAND_FLYBY_SETHOME:
    {
        StelApp::getInstance().goHome();
        break;
    }
    case RSYNC_COMMAND_FLYBY_SETLOC:
    {
        StelLocation loc = StelApp::getInstance().getCore()->getNavigator()->getCurrentLocation();

        loc.latitude = datalist[1].toDouble();
        loc.longitude = datalist[2].toDouble();

        StelApp::getInstance().getCore()->getNavigator()->moveObserverTo(loc, 0.);

        break;
    }
    case RSYNC_COMMAND_MESS_ADD:
    {
        QString messier = QString(datalist[1]);
        QString btnName = datalist[2];
        bool move = QString(datalist[3]).toInt();

        QString strFile = StelFileMgr::getInstallationDir()+"/catalogs/messiers/"+messier;
        QImage img = decryptImage(strFile);
        int iwidth = img.width();
        int iheight = img.height();
        img = QImage();

        if(move)
        {
            StelObjectMgr* objectMgr = GETSTELMODULE(StelObjectMgr);
            objectMgr->findAndSelectI18n(messier);
            const QList<StelObjectP> newSelected = objectMgr->getSelectedObject();
            if (!newSelected.empty())
            {
                StelMovementMgr* mvmgr = GETSTELMODULE(StelMovementMgr);
                mvmgr->moveToObject(newSelected[0], mvmgr->getAutoMoveDuration());

                //---
                const Vec3d& current = newSelected[0]->getAltAzPos(StelApp::getInstance().getCore()->getNavigator());
                double alt, azi;
                StelUtils::rectToSphe(&azi, &alt, current);
                alt = (alt)*180/M_PI; // convert to degrees from radians
                azi = std::fmod((((azi)*180/M_PI)*-1)+180., 360.);

                Sleep(100);

                StelApp::getInstance().addNetworkCommand(
                            "core.selectObjectByName(\""+messier+"\",true);\n"+
                            "core.moveToAltAzi("+QString("%0").arg(alt)+","+QString("%0").arg(azi)+",2);");
                //--
                Sleep(100);
            }

        }

        messierimage* miLast =StelApp::getInstance().addMessierObject(messier,iwidth, iheight, btnName);

        if(miLast)
            sendTcpData(RSYNC_COMMAND(RSYNC_COMMAND_MESS_GETLASTNAME),miLast->btnName);

        break;
    }
    case RSYNC_COMMAND_MESS_REMOVE:
    {
        QString messier = QString(datalist[1]);
        StelApp::getInstance().removeMessierObject(messier);
        break;
    }
    case RSYNC_COMMAND_MESS_REMOVEALL:
    {
        StelPresentMgr* sPmgr = GETSTELMODULE(StelPresentMgr);
        foreach(messierimagePtr m, StelApp::getInstance().messierList)
        {
            sPmgr->removeWithFade(m.data()->id);
        }
        StelApp::getInstance().messierList.clear();
        break;
    }
    case RSYNC_COMMAND_PLANETS_ZOOM:
    {
        QString objname= datalist[1];
        int zoomin = datalist[2].toInt();
        StelApp::getInstance().startPlanetZoom(objname,zoomin);
        break;
    }
    case RSYNC_COMMAND_ILLUM:
    {
        int checked = datalist[1].toInt();
        QString color= datalist[2];
        if (checked)
        {
            if (color == "R")
                StelApp::getInstance().getCore()->allFaderColor.set(1,0,0);
            else if (color == "G")
                StelApp::getInstance().getCore()->allFaderColor.set(0,1,0);
            else if (color == "B")
                StelApp::getInstance().getCore()->allFaderColor.set(0,0,1);
            else if (color == "W")
                StelApp::getInstance().getCore()->allFaderColor.set(1,1,1);

            StelApp::getInstance().getCore()->ALLFader.setDuration(2000);
            StelApp::getInstance().getCore()->ALLFader = true;
        }
        else
        {
            StelApp::getInstance().getCore()->ALLFader.setDuration(2000);
            StelApp::getInstance().getCore()->ALLFader = false;
        }
        break;
    }
    case RSYNC_COMMAND_RESET_ALL:
    {
        StelApp::getInstance().getCore()->allFaderColor.set(0,0,0);
        StelApp::getInstance().getCore()->ALLFader.setDuration(2000);
        StelApp::getInstance().getCore()->ALLFader = true;

        resetAllTimer->start();
        break;
    }
    case RSYNC_COMMAND_VIDEO_COLORS:
    {
        LandscapeMgr* lmgr = GETSTELMODULE(LandscapeMgr);
        double brightness = datalist[1].toDouble();
        double contrast = datalist[2].toDouble();
        double saturation = datalist[3].toDouble();
        lmgr->setVideoBrightness(brightness);
        lmgr->setVideoContrast(contrast);
        lmgr->setVideoSaturation(saturation);
        break;
    }

    case RSYNC_COMMAND_END:
    {
        qApp->closeAllWindows();
        break;
    }

    }

}

unsigned int rsync::getProcessIdsByProcessName(const char* processName, QStringList &listOfPids)
{
    // Clear content of returned list of PIDS
    listOfPids.clear();

#if defined(Q_OS_WIN)
    // Get the list of process identifiers.
    DWORD aProcesses[1024], cbNeeded, cProcesses;
    unsigned int i;

    if (!EnumProcesses(aProcesses, sizeof(aProcesses), &cbNeeded))
    {
        return 0;
    }

    // Calculate how many process identifiers were returned.
    cProcesses = cbNeeded / sizeof(DWORD);

    // Search for a matching name for each process
    for (i = 0; i < cProcesses; i++)
    {
        if (aProcesses[i] != 0)
        {
            char szProcessName[MAX_PATH] = {0};

            DWORD processID = aProcesses[i];

            // Get a handle to the process.
            HANDLE hProcess = OpenProcess( PROCESS_QUERY_INFORMATION |
                                           PROCESS_VM_READ,
                                           FALSE, processID);

            // Get the process name
            if (NULL != hProcess)
            {
                HMODULE hMod;
                DWORD cbNeeded;

                if (EnumProcessModules(hProcess, &hMod, sizeof(hMod), &cbNeeded))
                {
                    GetModuleBaseNameA(hProcess, hMod, szProcessName, sizeof(szProcessName)/sizeof(char));
                }

                // Release the handle to the process.
                CloseHandle(hProcess);

                if (*szProcessName != 0 && strcmp(processName, szProcessName) == 0)
                {
                    listOfPids.append(QString::number(processID));
                }
            }
        }
    }

    return listOfPids.count();

#else

    // Run pgrep, which looks through the currently running processses and lists the process IDs
    // which match the selection criteria to stdout.
    QProcess process;
    process.start("pgrep",  QStringList() << processName);
    process.waitForReadyRead();

    QByteArray bytes = process.readAllStandardOutput();

    process.terminate();
    process.waitForFinished();
    process.kill();

    // Output is something like "2472\n2323" for multiple instances
    if (bytes.isEmpty())
        return 0;

    // Remove trailing CR
    if (bytes.endsWith("\n"))
        bytes.resize(bytes.size() - 1);

    listOfPids = QString(bytes).split("\n");
    return listOfPids.count();

#endif
}


void rsync::CloseApplication(bool all)
{
    QStringList exePIDlist;
    getProcessIdsByProcessName("shiraplayer.exe",exePIDlist);
    foreach(QString exePID,exePIDlist)
    {
#if defined(Q_OS_WIN)
        if ( (exePID != QString("%0").arg(QCoreApplication::applicationPid())) ||
             all )
        {
            QString cmdString = QString("taskkill /f /pid %1").arg(exePID);
            system(cmdString.toStdString().c_str());
        }
#endif
    }
}

QImage rsync::decryptImage(QString filename)
{
    cBinExt* m_BinExt = new cBinExt;

    m_BinExt->SetBinPath(filename.toStdString() );
    m_BinExt->DoBufferFile();

    m_BinExt->IsAttachmentPresent();
    QPixmap img = m_BinExt->ExtractImage(filename.toLocal8Bit().constData(),
                                         QString("14531975").toStdString());

    if(m_BinExt != NULL)
    {
        delete m_BinExt;
        m_BinExt = NULL;
    }
    return img.toImage();
}



