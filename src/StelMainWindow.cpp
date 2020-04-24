/*
 * ShiraPlayer(TM)
 * Copyright (C) 2002 Fabien Chereau
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

//#include "splashscreen.h"
#include "shiraplayerform.hpp"
#include "StelMainWindow.hpp"
#include <stdexcept>
#include "StelApp.hpp"
#include "StelMainGraphicsView.hpp"
#include "StelFileMgr.hpp"
#include "StelModuleMgr.hpp"
#include "StelTranslator.hpp"
//#include "StelOpenGL.hpp"
#include "StelGui.hpp"
#include "StelCore.hpp"
#include "warping/window.h"
#include "previewutil/previewform.hpp"
#include "ui_previewform.h"


#include <QThread>
#include <QSettings>
#include <QResizeEvent>
#include <QIcon>
#include <QDebug>
#include <QCoreApplication>
#include <QApplication>
#include <QHBoxLayout>
#include <QtOpenGL/QGLWidget>
#include <QDesktopWidget>
#include <QVBoxLayout>

#include <QtNetwork/QHostInfo>
#include <QStandardPaths>

#include "shiraprojector.h"
// Initialize static variables
StelMainWindow* StelMainWindow::singleton = NULL;

StelMainWindow::StelMainWindow() : QMainWindow(NULL),shiraSharedMem("ShiraPlayerSharedMemory")
{

    // Can't create 2 StelMainWindow instances
    Q_ASSERT(!singleton);
    singleton = this;

    setAttribute(Qt::WA_NoSystemBackground);

    setWindowIcon(QIcon(":/mainWindow/icon.bmp"));
    initTitleI18n();

    mainShiraPlayerForm = new ShiraPlayerForm(this);

    //mainGraphicsView = new StelMainGraphicsView(this);

    //mainShiraPlayerForm->setScene(mainGraphicsView->scene());

    //setCentralWidget(mainShiraPlayerForm);

}

// Update the translated title
void StelMainWindow::initTitleI18n()
{
    QString appNameI18n = q_("Shira Player %1").arg(StelUtils::getApplicationVersion());
    setWindowTitle(appNameI18n);
}

bool StelMainWindow::getIsPluginRegistered()
{
    return projdll->isRegistered();
}

void StelMainWindow::init(QSettings* conf,QStringList arg)
{
    QSettings settings("Sureyyasoft", "ShiraPlayer");
    settings.beginGroup("Licenses");
    dt = settings.value("record_date",QDateTime::currentDateTime()).toDate();
    uname =settings.value("user","").toString();
    lcode =settings.value("license_code","").toString();
    settings.endGroup();

    QLisansForm frm;
    if ( QString::compare(lcode,frm.KodOlustur(dt,uname),Qt::CaseInsensitive) == 0)
        StelMainWindow::getInstance().is_Licenced = true;
    else
        StelMainWindow::getInstance().is_Licenced = false;

#ifndef SHIRAPLAYER_PRE
    bool isServer = conf->value("main/flag_server",true).toBool();
    if (!isServer) arg<<"client";
#endif

    if (arg.contains("client"))
    {
        setIsServer(false);
        mainGraphicsView = new StelMainGraphicsView(this);
        setWindowFlags(
            #ifdef Q_OS_MAC
                    Qt::SubWindow |
            #else
                    Qt::Tool |
            #endif
                    Qt::FramelessWindowHint |
                    Qt::WindowSystemMenuHint |
                    Qt::WindowStaysOnTopHint
                    );
    }
    else
    {
        setIsServer(true);

        projdll = new Shiraprojector();
        if (projdll->getUseBuffer() && projdll->isEnabled())
        {
            setIsMultiprojector(true);
        }

        //Öncelikle yasaklanmýþ kullanýcýya ait dosya mevcut ve ayný kullanýcýmý lisnaslanmýþ?, mevcutsa lisansý sil
        if (isBanUserfromFile()) clearLicenseInf();

        //Server ise internetten log kaydý atýlacak, gerigelen veri okunacak ve lisans dosyasý modifiye edilecek.
        QString username = uname;
        if (username =="")
            username ="Unlicensed User";
        QString hostName = QHostInfo::localHostName();
        netMan = new QNetworkAccessManager(this);
        QString package = "";

        if (getIsMultiprojector())
            package = "PRO";

        QString url = QString("http://www.sureyyasoft.com/add_access.asp?"
                              "USERID=%0&USERNAME=%1&ENTERDATE=%2&"
                              "DESKTOPTABLET=%3&MACHINENAME=%4&VERSION=%5%6")
                .arg(lcode)
                .arg(username)
                .arg(QDateTime::currentDateTimeUtc().toString("dd.MM.yyyy hh:mm:ss"))
                .arg("D")
                .arg(hostName)
                .arg(PACKAGE_VERSION)
                .arg(package);

        netMan->get(QNetworkRequest(QUrl(url)));
        connect(netMan,SIGNAL(finished(QNetworkReply*)),this,SLOT(finished(QNetworkReply*)));
        //



        PreviewFormWidget = new PreviewForm(this);
        PreviewFormWidget->setVisible(false);// Önce gizleniyor
        mainGraphicsView = new StelMainGraphicsView(PreviewFormWidget);
        PreviewFormWidget->ui->viewLayout->addWidget(mainGraphicsView);

        m_splashScreen = new splashScreen();
        m_splashScreen->show();

    }


    //QMessageBox::information(0,QString::number(getIsServer()),"ok",0,0);
    setWindowFlags(Qt::FramelessWindowHint);

    //Splash için

    //    setGeometry((QApplication::desktop()->availableGeometry().width()-512)/2,
    //                (QApplication::desktop()->availableGeometry().height()-512)/2,
    //                512,
    //                512);


    //    mainGraphicsView->setGeometry(0,0,512,512);
    setGeometry(0,0,1,1);
    mainGraphicsView->setGeometry(0,0,1,1);

    //---

    show();
    // Process the event to make the window visible and create the openGL context.

    QCoreApplication::processEvents();
    mainGraphicsView->init(conf);
    //qDebug()<< "mainGraphicsView init edildi";

    if(!getIsServer())
    {
        StelApp::getInstance().getGui()->setVisible(StelApp::getInstance().showPropGui);
        //#ifdef SHIRAPLAYER_PRE
        if (!StelMainWindow::getInstance().getIsMultiprojector())
        {
            setGeometry(conf->value("video/screen_l", 800).toInt(),
                        conf->value("video/screen_t", 0).toInt(),
                        conf->value("video/screen_w", 800).toInt(),
                        conf->value("video/screen_h", 600).toInt());
        }
        //#else
        else
        {
            setGeometry(0,
                        0,
                        conf->value("video/screen_w", 800).toInt(),
                        conf->value("video/screen_h", 600).toInt());
        }
        //#endif
    }
    else
        StelApp::getInstance().getGui()->setVisible(true);

    //Client ise ShareMemory dinlemesine geçiliyor.
    if(!getIsServer())
    {
        //#ifdef SHIRAPLAYER_PRE
        if (!StelMainWindow::getInstance().getIsMultiprojector())
        {
            sharedMemoryTimer = new QTimer();
            sharedMemoryTimer->setSingleShot(false);
            sharedMemoryTimer->setInterval(100);
            connect(sharedMemoryTimer, SIGNAL(timeout()), this, SLOT(loadFromSharedMem()));
            sharedMemoryTimer->start();
        }
        //#endif
    }

    if(getIsServer())
    {
        StelApp::getInstance().isLiveMode = true;
    }

    isDiscreateTimeFirst = false;

}

void StelMainWindow::deinit()
{
    StelMainGraphicsView::getInstance().deinitGL();
}

// Alternate fullscreen mode/windowed mode if possible
void StelMainWindow::toggleFullScreen()
{
    // Toggle full screen
    if (getFullScreen())
    {
        setFullScreen(false);
    }
    else
    {
        setFullScreen(true);
    }
}

// Get whether fullscreen is activated or not
bool StelMainWindow::getFullScreen() const
{
    return windowState().testFlag(Qt::WindowFullScreen);
}

// Set whether fullscreen is activated or not.
// Don't use the showFullScreen() and showNormal() methods since they have an unexpected behaviour.
void StelMainWindow::setFullScreen(bool b)
{
    if (b)
        setWindowState(windowState() | Qt::WindowFullScreen );
    else
        setWindowState(windowState() & ~Qt::WindowFullScreen);
}

void StelMainWindow::closeEvent(QCloseEvent* event)
{
    event->ignore();
    //QCoreApplication::exit();
}

void StelMainWindow::loadFromSharedMem()
{
    if (!shiraSharedMem.attach())
    {
        return;
    }

    QBuffer buffer;
    QDataStream in(&buffer);
    QString text;

    shiraSharedMem.lock();
    buffer.setData((char*)shiraSharedMem.constData(), shiraSharedMem.size());
    buffer.open(QBuffer::ReadOnly);
    in >> text;


    QBuffer bufferClear;
    bufferClear.open(QBuffer::ReadWrite);
    QDataStream out(&bufferClear);
    out << "";
    int size = bufferClear.size();

    //--clear data---
    char *to = (char*)shiraSharedMem.data();
    const char *from = bufferClear.data().data();
    memcpy(to, from, qMin(shiraSharedMem.size(), size));
    //----

    shiraSharedMem.unlock();
    shiraSharedMem.detach();

    if(text != "")
        StelApp::getInstance().getRsync()->recvSettingsSharedMemory(text);
    //QMessageBox::information(0,"",text,0,0);

}

void StelMainWindow::finished(QNetworkReply *reply)
{
    QString result = reply->readAll();
    if ((result == "True") || (result == "true"))
    {
        //Internetten Yasaklanmýþ kullanýcý bilgisi döndü
        //Kullanýcý lisans bilgileri silinecek
        clearLicenseInf();

        //Bu lisans kodu yasaklanan lisans dosyasýna yazýlýcak
        writeBanFile();
    }
}


void StelMainWindow::writeBanFile()
{
    QString outputFilename = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation);
    outputFilename = outputFilename +"/tfosayyerus";

    QFile outputFile(outputFilename);
    outputFile.open(QIODevice::WriteOnly);

    /* Check it opened OK */
    if(!outputFile.isOpen()){
        //qDebug() << "- Error, unable to open" << outputFilename << "for output";
        return;
    }
    //qDebug() << "- output file:" << outputFilename << "for output";

    /* Point a QTextStream object at the file */
    QTextStream outStream(&outputFile);

    /* Write the line to the file */
    outStream << lcode;

    /* Close the file */
    outputFile.close();
}

void StelMainWindow::clearLicenseInf()
{
    QSettings settings("Sureyyasoft", "ShiraPlayer");
    settings.beginGroup("Licenses");
    settings.setValue("record_date","");
    settings.setValue("user","");
    settings.setValue("license_code","");
    settings.endGroup();
}

bool StelMainWindow::isBanUserfromFile()
{
    QString filename = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation);
    filename = filename +"/tfosayyerus";

    QFile readFile(filename);
    if (readFile.open(QIODevice::ReadOnly))
    {
        QTextStream in(&readFile);
        QString line = in.readLine();
        if ( line == lcode )
            return true;
    }
    return false;
}


