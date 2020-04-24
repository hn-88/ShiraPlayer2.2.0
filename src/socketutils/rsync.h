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

#ifndef RSYNC_H
#define RSYNC_H

//#include <QFtp>
#include <QtNetwork/QUdpSocket>
#include <QProgressDialog>
#include <QtOpenGL/QtOpenGL>
#include <QPair>
#include <QList>

#include "StelMainGraphicsView.hpp"
#include "ScreenImageMgr.hpp"

#include "videoutils/videoclass.h"
#include "videoutils/audioclass.h"
//#include "presenter/VideoSource.h"


#include <QtNetwork/QTcpServer>
#include <QtNetwork/QTcpSocket>

//! Remote sync commands.
enum RSYNC_COMMAND
{
    RSYNC_COMMAND_INIT,         //!< Notify new connection.
    RSYNC_COMMAND_INIT_OFF,
    RSYNC_COMMAND_CHANNEL,      //!< Sync channel settings.
    RSYNC_COMMAND_END,          //!< Request to quit the program.
    RSYNC_COMMAND_SPHERICMIRROR,
    RSYNC_COMMAND_DISKVIEWPORT,
    RSYNC_COMMAND_SETGRAVITYLABELS,
    RSYNC_COMMAND_WARP_MODE,
    RSYNC_COMMAND_SCRIPT,
    RSYNC_COMMAND_LIVE_SCRIPT,
    RSYNC_COMMAND_START_GRAPH,
    RSYNC_COMMAND_STOP_GRAPH,
    RSYNC_COMMAND_STOP_AUDIO,
    RSYNC_COMMAND_PAUSE_GRAPH,
    RSYNC_COMMAND_SEEK_GRAPH,
    RSYNC_COMMAND_GRAPH_PERFORMANCE,
    RSYNC_COMMAND_FTP,
    RSYNC_COMMAND_FTP_UPDPRG,
    RSYNC_COMMAND_RESTARTAPP,
    RSYNC_COMMAND_VOLUME,
    RSYNC_COMMAND_SHOW_FRAME,
    RSYNC_COMMAND_SHOW_FRAME_DAYLIGHT,
    RSYNC_COMMAND_SHOW_CLEAR,
    RSYNC_COMMAND_SAVESET,
    RSYNC_COMMAND_SAVELOC,
    RSYNC_COMMAND_SHOWCHNAME,
    RSYNC_COMMAND_SHUTDOWN_PC,
    RSYNC_COMMAND_PROJECTOR,
    RSYNC_COMMAND_LOADPRESENTIMAGE,
    RSYNC_COMMAND_PLAYPRESENTVIDEO,
    RSYNC_COMMAND_REMOVEPRESENTVIDEO,
    RSYNC_COMMAND_PAUSEPRESENTVIDEO,
    RSYNC_COMMAND_RECORDVIDEO,
    RSYNC_COMMAND_STOPRECVIDEO,
    RSYNC_COMMAND_PAUSERECVIDEO,
    RSYNC_COMMAND_LOCKCLIENT,    
    RSYNC_COMMAND_SATELLITE_SHOW,
    RSYNC_COMMAND_SATELLITE_SHOW_LABEL,
    RSYNC_COMMAND_SATELLITE_FONTSIZE,
    RSYNC_COMMAND_DRAWFREE_LIVE,
    RSYNC_COMMAND_DRAWFREE_ADD,
    RSYNC_COMMAND_DRAWFREE_DELETE,
    RSYNC_COMMAND_DRAWFREE_DELETEALL,
    RSYNC_COMMAND_DRAWFREE_START,
    RSYNC_COMMAND_FINETUNE,
    RSYNC_COMMAND_FINETUNE_RESET,
    RSYNC_COMMAND_FINETUNE_SAVE,
    RSYNC_COMMAND_CONSOLE_GETSTELLA,
    RSYNC_COMMAND_CONSOLE_GETTIME,
    RSYNC_COMMAND_CONSOLE_GETLOCATION,
    RSYNC_COMMAND_CONSOLE_GETFULLDOMELIST,
    RSYNC_COMMAND_CONSOLE_GETTRACKTIME,
    RSYNC_COMMAND_CONSOLE_GETFISHEYEIMAGELIST,
    RSYNC_COMMAND_CONSOLE_GETONEIMAGE,
    RSYNC_COMMAND_CONSOLE_GETFLATLIST,
    RSYNC_COMMAND_CONSOLE_GETSCRIPTLIST,
    RSYNC_COMMAND_CONSOLE_RUNSCRIPT,
    RSYNC_COMMAND_CONSOLE_CLOSEALL,
    RSYNC_COMMAND_CONSOLE_GETPREVIEW,
    RSYNC_COMMAND_GRAPH_SKIPNEXTFRAME,
    RSYNC_COMMAND_FLYBY_SETPLANET,
    RSYNC_COMMAND_FLYBY_SETPOS,
    RSYNC_COMMAND_FLYBY_SETALTITUDE,
    RSYNC_COMMAND_FLYBY_SETHOME,
    RSYNC_COMMAND_FLYBY_SETLOC,
    RSYNC_COMMAND_MESS_ADD,
    RSYNC_COMMAND_MESS_REMOVE,
    RSYNC_COMMAND_MESS_REMOVEALL,
    RSYNC_COMMAND_MESS_GETLASTNAME,
    RSYNC_COMMAND_CONST_SELSINGLE,
    RSYNC_COMMAND_CONST_SELMULTI,
    RSYNC_COMMAND_CONST_ARTTOLINE,
    RSYNC_COMMAND_CONST_LINECOLOR,
    RSYNC_COMMAND_CONST_LINEWIDTH,
    RSYNC_COMMAND_STELLA_FRONTSKY,
    RSYNC_COMMAND_STELLA_STARTRAILS,
    RSYNC_COMMAND_STELLA_ISSTARMAGLIMIT,
    RSYNC_COMMAND_STELLA_STARMAGLIMITVAL,
    RSYNC_COMMAND_STELLA_HIDEALL,
    RSYNC_COMMAND_COMPASSPLUGIN_MARK,
    RSYNC_COMMAND_PLANETS_ZOOM,
    RSYNC_COMMAND_ILLUM,
    RSYNC_COMMAND_RESET_ALL,
    RSYNC_COMMAND_VIDEO_COLORS
};

class VideoSource;
class rsync : public QObject
{
    Q_OBJECT

public:
    rsync();
    void initServer();
    void sendChanges(RSYNC_COMMAND command, const QString& data);
    void sendOpenTerminals(const QString& MACAdress);
    void sendInitdata(bool state);

    void initClient();
    void saveCurrentViewOptions();

    void recvSettingsSharedMemory(QString dataShared);
    void CloseApplication(bool all);
private:
    //Serial Port commands
    //void sendSerial(QString msg);

    int savedProjectionType;

    QTcpServer *m_tcpserver;
    QTcpSocket *tcpsocket;
    void sendTcpData(RSYNC_COMMAND command, const QString &data);
    void sendTcpImage(RSYNC_COMMAND command, const QImage &image, const QString &type);

    unsigned int getProcessIdsByProcessName(const char* processName, QStringList &listOfPids);
    QImage decryptImage(QString filename);

    Vec3d oldViewDirection;
protected:
    bool m_bServer;             //!< True if this program is a server, false if it is a client.
    int m_portNo;               //!< Port number to connect.

    QUdpSocket * m_pUdpServer;  //!< Server object.
    QUdpSocket* m_pUdpClient;   //!< Client object.

    QString m_clientAddress;

    bool is_audio;
    bool withSound;
    bool only_audio;
    AudioClass* audiost;

    void Gecikme(QString strdt);

    QString sLandscape ;
    bool sLandscapeVisibleFlag;
    QSettings* conf;
    QString media_path;
    //QFtp *ftp;
    QFile *file;
    QString downfilename;
    QProgressDialog *progressDialog;
    void dowloadFtpFile(QString ftpserver,QString filename,QString user,QString pass,int mod);
    void downloadFile(int mod);

    bool old_isVideoLandscape;
    bool bFlagLandscape;
    int ftp_mod;
    QString sftp_server,sftp_prog_user,sftp_prog_pass;

    bool old_showPropGui ;

    QSharedMemory sharedMem;
    void LoadIntoSharedMomory(QString text);

    QTimer* startGraphTimer;
    QTimer* stopGraphTimer;
    QTimer* showFisheyeFrameTimer;
    QTimer* clearFramesTimer;
    QTimer* trackTimerConsole;
    QTimer* stellaHideTimer;
    QSharedMemory videoSharedMemConsole;

    QList<QPair<int,VideoSource*> > vPresentsource;
    QList<QPair<int,VideoClass*> > vPresentfile;

    void StartAudio(QString fileName,bool checked);
    ScreenImageMgr sImgr;

    //Console için Flyby
    int realtrackValue ;

    bool hideAllStella;

    QTimer* resetAllTimer;

public slots:
    void newConnection();

protected slots:

    void recvSettings();
    void displayNetworkError(QAbstractSocket::SocketError socketError);
    void ftpCommandFinished(int, bool error);
    void updateDataTransferProgress(qint64 readBytes, qint64 totalBytes);

    void recvSettingsTCP();
private slots:
    void doftpfinished();
    void startGraphBYTimer();
    void stopGraphBYTimer();
    void showFisheyeFrameBYTimer();
    void clearFramesBYTimer();
    void on_stellaHideTimer();
    void resetAllProc();

    QString loadFromSharedMemConsole();
    void trackConsoleTimer();
    void stopGraphProc();

signals:
    void ftpfinished();
};

#endif // RSYNC_H
