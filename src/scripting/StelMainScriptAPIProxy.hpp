/*
 * ShiraPlayer(TM)
 * Copyright (C) 2009 Matthew Gates
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
#ifndef _STELMAINSCRIPTAPIPROXY_HPP_
#define _STELMAINSCRIPTAPIPROXY_HPP_

#include <QObject>
#include "videoutils/audioclass.h"

//! @class StelMainScriptAPIProxy
//! Because the core API runs in a different thread to the main program, 
//! direct function calls to some classes can cause problems - especially 
//! when images must be loaded, or other non-atomic operations are involved.
//!
//! This class acts as a proxy - running in the Main thread.  Connect signals
//! from the StelMainScriptAPI to the instance of this class running in the
//! main thread and let the slots do the work which is not possible within
//! StelMainScriptAPI itself.
//! 
//! Please follow the following convention:
//! member in StelMainScriptAPI:      someSlot(...)
//! signal in StelMainScriptAPI:      requestSomeSlot(...)
//! member in StelMainScriptAPIProxy: someSlot(...)
//!
//! The dis-advantage of this method is that there is no way to get a return
//! value.  This is because of how the signal/slot mechanism works.
class StelMainScriptAPIProxy : public QObject
{
    Q_OBJECT

public:
    StelMainScriptAPIProxy(QObject* parent=0) : QObject(parent) {
        audio = &AudioClass::getInstance();
    }
    ~StelMainScriptAPIProxy() {;}

private:

    QString sLandscape ;
    bool old_isVideoLandscape;
    QString path;
    bool b_isaudio;

    AudioClass* audio;
    bool isloaded;

public slots:
    void setDiskViewport(bool b);

    //About media player
    void loadVideo(const QString& filename);
    void playVideo(bool is_Audio);
    void pauseVideo(bool pause);
    void stopVideo();
    void playAudio(const QString& filename);
    void stopAudio();
    //About FlyBy
    void prepareFlyBy();
    void setFlyBy(const QString &planetFlyByname, bool isInner);
    void goHome();
};

#endif // _STELMAINSCRIPTAPIPROXY_HPP_

