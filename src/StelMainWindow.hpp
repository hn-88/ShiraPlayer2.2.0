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

#ifndef _STELMAINWINDOW_HPP_
#define _STELMAINWINDOW_HPP_

#include "previewutil/previewform.hpp"

#include <QMainWindow>
#include <QSettings>
#include <QSharedMemory>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QByteArray>

#include "gui/splashscreen.h"
//#include "shiraprojector.h"
class Shiraprojector;

//! @class StelMainWindow
//! Reimplement a QMainWindow for Stellarium.
//! It is the class in charge of switching betwee fullscreen or windowed mode.
class StelMainWindow : public QMainWindow
{
    Q_OBJECT
public:
    StelMainWindow();

    //! Get the StelMainWindow singleton instance.
    //! @return the StelMainWindow singleton instance
    static StelMainWindow& getInstance() {Q_ASSERT(singleton); return *singleton;}

    //! Performs various initialization including the init of the StelMainGraphicsView instance.
    void init(QSettings* settings,QStringList arg);

    void deinit();

    //! Set the application title for the current language.
    //! This is useful for e.g. chinese.
    void initTitleI18n();

    //! ASAF client/server state
    bool isServer;
    bool isDiscreateTimeFirst;
    bool getIsServer(){return isServer;}
    void setIsServer(bool status){isServer = status;}
    void setIsMultiprojector(bool val) { isMultiprojector = val;}
    bool getIsMultiprojector() { return isMultiprojector;}

    void setStellaHide(bool val) { isStellaHide = val;}
    bool getStellaHide(){ return isStellaHide;}

    bool getIsPluginRegistered();

    PreviewForm* getPreviewWidget(){return PreviewFormWidget;}

    //Lisans hakkýnda
    bool is_Licenced;

    QDate dt ;
    QString uname ;
    QString lcode ;
    void writeBanFile();
    void clearLicenseInf();
    bool isBanUserfromFile();

    //Projector Plugin
    Shiraprojector* projdll;
public slots:
    //! Alternate fullscreen mode/windowed mode if possible
    void toggleFullScreen();

    //! Get whether fullscreen is activated or not
    bool getFullScreen() const;
    //!	Set whether fullscreen is activated or not
    void setFullScreen(bool);

    void loadFromSharedMem();

    //Internetten gelen cevap iÃ§in
    void finished(QNetworkReply *reply);
protected:
    //! Reimplemented to delete openGL textures before the GLContext disappears
    virtual void closeEvent(QCloseEvent* event);

private:
    //! The StelMainWindow singleton
    static StelMainWindow* singleton;

    //QWidget* PreviewWidget;
    PreviewForm* PreviewFormWidget;
    splashScreen* m_splashScreen;

    class StelMainGraphicsView* mainGraphicsView;
    class ShiraPlayerForm* mainShiraPlayerForm;

    QSharedMemory shiraSharedMem;
    QTimer* sharedMemoryTimer;

    QNetworkAccessManager *netMan;

    //Main boolean Multiprojector
    bool isMultiprojector = false;

    bool isStellaHide = false;
};

#endif // _STELMAINWINDOW_HPP_
