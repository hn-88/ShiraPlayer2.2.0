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

#include "StelMainScriptAPIProxy.hpp"

#include "StelApp.hpp"
#include "StelCore.hpp"
#include "StelProjector.hpp"
#include "StelMainWindow.hpp"

#include "StelModuleMgr.hpp"
#include "StelFileMgr.hpp"
#include "LandscapeMgr.hpp"

void StelMainScriptAPIProxy::setDiskViewport(bool b)
{
    if (b)
        StelApp::getInstance().getCore()->setMaskType(StelProjector::MaskDisk);
    else
        StelApp::getInstance().getCore()->setMaskType(StelProjector::MaskNone);
}

void StelMainScriptAPIProxy::loadVideo(const QString& filename)
{
    if ((!StelMainWindow::getInstance().getIsMultiprojector()) &&
            (StelMainWindow::getInstance().getIsServer())) return;

    //QSettings* conf = StelApp::getInstance().getSettings();
    //QString media_path = conf->value("main/media_path", "C:/").toString();

    try
    {
        //path = StelFileMgr::findFile(media_path+"/" + filename);
        path = StelFileMgr::findFile(filename);
    }
    catch(std::runtime_error& e)
    {
        qWarning() << "cannot find video file" << filename << ":" << e.what();
        isloaded = false;
        return;
    }

    //Þu anki landscape hafýzaya alýnýyor
    LandscapeMgr* lmgr = GETSTELMODULE(LandscapeMgr);
    sLandscape = lmgr->getCurrentLandscapeName();
    old_isVideoLandscape = lmgr->isVideoLandscape;

    if(lmgr->s_Name != "")
        lmgr->doClearLandscapeVideo();

    lmgr->s_Name = ""; //Onemli
    lmgr->isVideoLandscape = false;

    lmgr->doSetCurrentLandscapetoVideo(path);
    isloaded = true;
    //lmgr->getCurrentLandscapeID();
    //Burasý bir þekilde yazýlmalý
    //QObject::connect(lmgr->vop_curr, SIGNAL(endofVideo()), this, SLOT(on_btnmStop_clicked()));
}
void StelMainScriptAPIProxy::playVideo(bool is_Audio)
{
    if (!isloaded) return;

    if ((!StelMainWindow::getInstance().getIsMultiprojector()) &&
            (StelMainWindow::getInstance().getIsServer())) return;
    LandscapeMgr* lmgr = GETSTELMODULE(LandscapeMgr);
    lmgr->doStartVideo();
    lmgr->fromScriptVideo = true;
    //Tüm çizimler LandscapeMgr hariç kapatlýyor
    StelApp::getInstance().isVideoMode = true;
    if(is_Audio)
    {
        b_isaudio = is_Audio;
    }
}

void StelMainScriptAPIProxy::pauseVideo(bool pause)
{
    if (!isloaded) return;
    if ((!StelMainWindow::getInstance().getIsMultiprojector()) &&
            (StelMainWindow::getInstance().getIsServer())) return;
    LandscapeMgr* lmgr = GETSTELMODULE(LandscapeMgr);
    lmgr->doPauseVideo(pause);

}

void StelMainScriptAPIProxy::stopVideo()
{
    if (!isloaded) return;
    if ((!StelMainWindow::getInstance().getIsMultiprojector()) &&
            (StelMainWindow::getInstance().getIsServer())) return;
    LandscapeMgr* lmgr = GETSTELMODULE(LandscapeMgr);
    lmgr->doClearLandscapeVideo();

    lmgr->setCurrentLandscapeName(sLandscape,false,old_isVideoLandscape);

}

void StelMainScriptAPIProxy::playAudio(const QString& filename)
{
    if(!StelMainWindow::getInstance().getIsServer())return;
    QSettings* conf = StelApp::getInstance().getSettings();
    QString media_path = conf->value("main/media_path", "C:/").toString();
    QString patha;
    try
    {
        if (StelFileMgr::exists(media_path+"/audio/" + filename))
            patha= StelFileMgr::findFile(media_path+"/audio/" + filename);
        else
        {
            patha= StelFileMgr::findFile(filename);
        }        

        audio->Load_Audio(patha.toLatin1().data());
        audio->Start_Audio();
    }
    catch(std::runtime_error& e)
    {
        qWarning() << "cannot play audio" << filename << ":" << e.what();
        return;
    }
}

void StelMainScriptAPIProxy::stopAudio()
{
    if (!StelMainWindow::getInstance().getIsServer()) return;
    audio->Stop_Audio();
}

void StelMainScriptAPIProxy::prepareFlyBy()
{
    StelApp::getInstance().prepareFlyBy();
}

void StelMainScriptAPIProxy::setFlyBy(const QString &planetFlyByname, bool isInner)
{
    StelApp::getInstance().setFlyBy(planetFlyByname,0,isInner);
}

void StelMainScriptAPIProxy::goHome()
{
    StelApp::getInstance().goHome();

}

