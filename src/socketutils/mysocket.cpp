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

#include <QMessageBox>
#include <QWidget>
#include <QtNetwork/QTcpServer>
#include <QtNetwork/QTcpSocket>
#include <QDateTime>

#include "socketutils/mysocket.h"
#include "StelCore.hpp"
#include "StelApp.hpp"
#include "StelMovementMgr.hpp"
#include "StelUtils.hpp"
#include "StelNavigator.hpp"
#include "StelLocaleMgr.hpp"
#include "StelObjectMgr.hpp"
#include "StelModuleMgr.hpp"
#include "StelLocationMgr.hpp"
#include "StelSkyDrawer.hpp"
#include "LandscapeMgr.hpp"
#include "SolarSystem.hpp"
#include "NebulaMgr.hpp"
#include "MeteorMgr.hpp"
#include "GridLinesMgr.hpp"
#include "ConstellationMgr.hpp"
#include "StelStyle.hpp"
#include "StarMgr.hpp"
#include "MilkyWay.hpp"
#include "GridLinesMgr.hpp"
#include "ConstellationMgr.hpp"
#include "Constellation.hpp"
#include "LandscapeMgr.hpp"
#include "StelGui.hpp"
#include "StelGuiItems.hpp"
#include "StelSkyCultureMgr.hpp"

mysocket::mysocket(StelApp* app)
{
    tcpServer = new QTcpServer();
    if (!tcpServer->listen(QHostAddress::Any,8001))
    {
        QMessageBox::critical(0, "Hata",
                                 "Unable to start the server",
                                 tcpServer->errorString());
    }

    connect(tcpServer,SIGNAL(newConnection ()),this,SLOT(DoConnect()));
    s_app=app;

    year=0;
	month=0;
	day=0;
	hour=0;
	minute=0;
	second=0;

}
mysocket::~mysocket()
{}

void mysocket::DoConnect()
{
    while(tcpServer->hasPendingConnections())
    {
        sck = tcpServer->nextPendingConnection();
        QObject::connect(sck, SIGNAL(readyRead()), this, SLOT(readIncomingData()));
    }

}
void mysocket::readIncomingData()
{
    QTextStream in(sck);
    QString string="";
    in >> string;

    this->mesajlari_uygula(string);
}

void mysocket::mesajlari_uygula(QString msg)
{
    //QMessageBox::information(0, "Stellarium",msg,"ok");
    StelCore* core=s_app->getCore();

    if (msg.startsWith("299"))
    {
        QStringList strList;
        strList=msg.split(":");

        int c_index=strList[1].toInt();
        if (c_index==0)
        {
            b_dataSend=true;
        }
        else
        {
            b_dataSend=false;
        }
    }
    //---Hareketler
    if (msg=="300")
    {
        core->movementMgr->turnLeft(true);
    }
    if (msg=="301")
    {
        core->movementMgr->turnLeft(false);
    }
    if (msg=="302")
    {
        core->movementMgr->turnRight(true);
    }
    if (msg=="303")
    {
        core->movementMgr->turnRight(false);
    }
    if (msg.startsWith("304"))
    {
        QStringList strList;
        strList=msg.split(":");
        core->movementMgr->autoZoomIn(strList[1].toDouble());
    }
    if (msg.startsWith("305"))
    {
        QStringList strList;
        strList=msg.split(":");
        core->movementMgr->autoZoomOut(strList[1].toDouble());
    }
    if (msg=="306")
    {
        core->movementMgr->zoomIn(true);
    }
    if (msg=="307")
    {
        core->movementMgr->zoomIn(false);
    }
    if (msg=="308")
    {
        core->movementMgr->zoomOut(true);
    }
    if (msg=="309")
    {
        core->movementMgr->zoomOut(false);
    }


    //------
    //---Zaman deðiþimi
    if(msg.startsWith("330"))
    {
        QStringList strList;
        strList=msg.split(":");
        try
        {
            bool ok;
            year=strList[1].toInt(&ok, 10);
            month=strList[2].toInt(&ok, 10);
            day=strList[3].toInt(&ok, 10);
            hour=strList[4].toInt(&ok, 10);
            minute=strList[5].toInt(&ok, 10);
            second=strList[6].toInt(&ok, 10);
            core->navigation->setJDay(newJd());
        }
        catch ( ... )
        {
            QMessageBox::critical(0, "ShiraPlayer","hata oluþtu"," ");
        }
    }

    if(msg.startsWith("331"))
    {
        QStringList strList;
        strList=msg.split(":");

        Gecikme(strList[1]);
        core->navigation->increaseTimeSpeed();

    }
    if(msg=="332")
    {
        core->navigation->decreaseTimeSpeed();
    }
    if(msg=="333")
    {
        core->navigation->setRealTimeSpeed();
    }
    if(msg=="334")
    {
        core->navigation->setTimeNow();
    }
    if(msg=="335")
    {
        double jd = StelApp::getInstance().getCore()->getNavigator()->getJDay();
        setDateTime(jd + (StelApp::getInstance().getLocaleMgr().getGMTShift(jd)/24.0)); // UTC -> local tz
        QString str=QString(":335:%1:%2:%3:%4:%5:%6").arg(year).arg(month).arg(day).arg(hour).arg(minute).arg(second);

        sendMessage(str);
    }
    //----------
    //--Nesne arama
    if(msg.startsWith("340"))
    {
        QStringList strList;
        strList=msg.split(":");

        QStringList matches = StelApp::getInstance().getStelObjectMgr().listMatchingObjectsI18n(strList[1], 5);
        QString str = matches.join(":");
        sendMessage(":340:"+str);
    }
    if(msg.startsWith("341"))
    {
        QStringList strList;
        strList=msg.split(":");
        StelObjectP selo=selectObject(strList[1]);
        if(selo->getType()=="Planet")
        {
            Planet* pl = dynamic_cast<Planet*>(selo.data());
            sendMessage(":341:"+QString("%1").arg(pl->getSphereScale()));
        }
    }
    if(msg.startsWith("342"))
    {
        QStringList strList;
        strList=msg.split(":");
        QString str=strList[1];
        str.replace(QString("="), QString(" "));
        gotoObject(str);
    }
    //------
    //--Fov ve initview ayarlarý
    if(msg.startsWith("350"))
    {
        core->currentProjectorParams.fov=180.0;// getProjection()->setFov(180.0);
        StelApp::getInstance().getStelObjectMgr().unSelect();
    }
    if(msg=="351")
    {
        QSettings* conf = StelApp::getInstance().getSettings();
        Vec3d initViewPos;
        initViewPos = StelUtils::strToVec3f(conf->value("navigation/init_view_pos").toString()).toVec3d();
        core->movementMgr->setViewDirectionJ2000(initViewPos);

        //core->navigation-> setLocalVision(initViewPos);


        int screen_count=conf->value("video/screen_count",0).toInt();
        int screen_index=conf->value("video/screen_index",0).toInt();


        StelMovementMgr* mvmgr=(StelMovementMgr*)GETSTELMODULE(StelMovementMgr);

        if (screen_count==3)
        {
            if(screen_index==1)
            {
                mvmgr->panView(-2*M_PI/3,0.0);
            }
            else if(screen_index==2)
            {
                mvmgr->panView(0.0,0.0);
            }
            else if(screen_index==3)
            {
                mvmgr->panView(2*M_PI/3,0.0);
            }
        }

    }
    ///---
    //---Location ayarlarý
    if(msg.startsWith("360"))
    {

        QStringList strList;
        strList=msg.split(":");

        QStringList locations;
        locations=StelApp::getInstance().getLocationMgr().getModelAll()->stringList();

        QRegExp rx(strList[1]);
        //rx.setPatternSyntax(QRegExp::Wildcard);
        rx.setCaseSensitivity(Qt::CaseInsensitive);

        QStringList strFinded=locations.filter(rx);

        QString str = strFinded.join(":");
        sendMessage("360:"+str);
    }

    if(msg.startsWith("361"))
    {
        QStringList strList;
        strList=msg.split(":");
        QString str=strList[1];
        str.replace(QString("="), QString(" "));

        StelLocation loc = StelApp::getInstance().getLocationMgr().locationForSmallString(str);
        StelApp::getInstance().getCore()->getNavigator()->moveObserverTo(loc, 0.);
    }

    if(msg.startsWith("362"))
    {
        StelLocation loc = StelApp::getInstance().getCore()->getNavigator()->getCurrentLocation();
        QString str=QString("362:%1:%2").arg(loc.longitude).arg(loc.latitude);
        sendMessage(str);
    }
    if(msg.startsWith("363"))
    {
        QStringList strList;
        strList=msg.split(":");
        StelLocation loc = StelApp::getInstance().getCore()->getNavigator()->getCurrentLocation();
        loc.latitude=strList[1].toDouble();
        loc.longitude=strList[2].toDouble();
        StelApp::getInstance().getCore()->getNavigator()->moveObserverTo(loc, 0.);
    }
    if(msg=="364")
    {
        QSettings* conf = StelApp::getInstance().getSettings();
        StelLocation loc = StelApp::getInstance().getLocationMgr().locationForSmallString(conf->value("init_location/location").toString());
        StelApp::getInstance().getCore()->getNavigator()->moveObserverTo(loc, 0.);
        QString str=QString("362:%1:%2").arg(loc.longitude).arg(loc.latitude);
        sendMessage(str);
    }
    //---
    // ---View/ Sky
    if(msg=="370")
    {
        LandscapeMgr* lmgr = (LandscapeMgr*)GETSTELMODULE(LandscapeMgr);
        SolarSystem* ssmgr = (SolarSystem*)GETSTELMODULE(SolarSystem);
        StarMgr* smgr = (StarMgr*)GETSTELMODULE(StarMgr);
        NebulaMgr* nmgr = (NebulaMgr*)GETSTELMODULE(NebulaMgr);
        MeteorMgr* mmgr = (MeteorMgr*)GETSTELMODULE(MeteorMgr);
        MilkyWay* milkyway = (MilkyWay*)GETSTELMODULE(MilkyWay);

        double absl=StelApp::getInstance().getCore()->getSkyDrawer()->getAbsoluteStarScale();
        double rsl=StelApp::getInstance().getCore()->getSkyDrawer()->getRelativeStarScale();
        bool btwk=StelApp::getInstance().getCore()->getSkyDrawer()->getFlagTwinkle();
        double twka=StelApp::getInstance().getCore()->getSkyDrawer()->getTwinkleAmount();
        bool fatm=lmgr->getFlagAtmosphere();
        double poll=StelApp::getInstance().getCore()->getSkyDrawer()->getBortleScale();
        bool splanet=ssmgr->getFlagPlanets();
        bool splaneto=ssmgr->getFlagOrbits();
        bool sstar=smgr->getFlagLabels();
        bool snebula=nmgr->getFlagHints();
        bool splanetsl=ssmgr->getFlagLabels();
        int sastroid=mmgr->getZHR();
        bool smilk=milkyway->getFlagShow();
        int imilk=milkyway->getIntensity();
        bool sscale=ssmgr->getFlagMoonScale();
        int  iscale=ssmgr->getMoonScale();

        QString str=QString("370:%1:%2:%3:%4:%5:%6:%7:%8:%9:%10:%11:%12:%13:%14:%15:%16")
                                        .arg(absl)
                                        .arg(rsl)
                                        .arg(btwk)
                                        .arg(twka)
                                        .arg(fatm)
                                        .arg(poll)
                                        .arg(splanet)
                                        .arg(splaneto)
                                        .arg(sstar)
                                        .arg(snebula)
                                        .arg(splanetsl)
                                        .arg(sastroid)
                                        .arg(smilk)
                                        .arg(imilk)
                                        .arg(sscale)
                                        .arg(iscale);

        sendMessage(str);


    }
    if(msg.startsWith("371"))
    {

        QStringList strList;
        strList=msg.split(":");

        LandscapeMgr* lmgr = (LandscapeMgr*)GETSTELMODULE(LandscapeMgr);
        SolarSystem* ssmgr = (SolarSystem*)GETSTELMODULE(SolarSystem);
        StarMgr* smgr = (StarMgr*)GETSTELMODULE(StarMgr);
        NebulaMgr* nmgr = (NebulaMgr*)GETSTELMODULE(NebulaMgr);
        MeteorMgr* mmgr = (MeteorMgr*)GETSTELMODULE(MeteorMgr);
        MilkyWay* milkyway = (MilkyWay*)GETSTELMODULE(MilkyWay);

        StelApp::getInstance().getCore()->getSkyDrawer()->setAbsoluteStarScale(strList[1].toDouble());
        StelApp::getInstance().getCore()->getSkyDrawer()->setRelativeStarScale(strList[2].toDouble());
        StelApp::getInstance().getCore()->getSkyDrawer()->setFlagTwinkle(strList[3].toInt());
        StelApp::getInstance().getCore()->getSkyDrawer()->setTwinkleAmount(strList[4].toDouble());
        lmgr->setFlagAtmosphere(strList[5].toInt());
        StelApp::getInstance().getCore()->getSkyDrawer()->setBortleScale(strList[6].toInt());
        ssmgr->setFlagPlanets(strList[7].toInt());
        ssmgr->setFlagOrbits(strList[8].toInt());
        smgr->setFlagLabels(strList[9].toInt());
        nmgr->setFlagHints(strList[10].toInt());
        ssmgr->setFlagLabels(strList[11].toInt());
        mmgr->setZHR(strList[12].toInt());
        mmgr->setFlagShow(strList[12].toInt());
        milkyway->setFlagShow(strList[13].toInt());
        milkyway->setIntensity(strList[14].toInt());
        //ssmgr->setFlagMoonScale(strList[15].toInt());
        //ssmgr->setMoonScale(strList[16].toInt());

        const QList<StelObjectP> pSel=StelApp::getInstance().getStelObjectMgr().getSelectedObject();
        if(pSel.count()>0)
        {
            if(pSel[0]->getType()=="Planet")
            {
                Planet* pl = dynamic_cast<Planet*>(pSel[0].data());
                pl->setSphereScale(strList[15].toInt());
                if(pl->getRings())
                    pl->getRings()->setRingScale(strList[15].toInt());
            }
        }

    }
    //---
    // ---View/ markings
    if(msg=="372")
    {
        GridLinesMgr* glmgr = (GridLinesMgr*)GETSTELMODULE(GridLinesMgr);
        LandscapeMgr* lmgr = (LandscapeMgr*)GETSTELMODULE(LandscapeMgr);
        ConstellationMgr* cmgr = (ConstellationMgr*)GETSTELMODULE(ConstellationMgr);

        bool eqgrid=glmgr->getFlagEquatorGrid();
        bool eqjgrid=glmgr->getFlagEquatorJ2000Grid();
        bool eqagrid=glmgr->getFlagAzimuthalGrid();
        bool eqline=glmgr->getFlagEquatorLine();
        bool merline=glmgr->getFlagMeridianLine();
        bool eqpline=glmgr->getFlagEclipticLine();
        bool carpnt=lmgr->getFlagCardinalsPoints();

        bool conline=cmgr->getFlagLines();
        bool conlabel=cmgr->getFlagLabels();
        bool conboun=cmgr->getFlagBoundaries();
        bool conart=cmgr->getFlagArt();
        double conartI=cmgr->getArtIntensity();

        QString str=QString("372:%1:%2:%3:%4:%5:%6:%7:%8:%9:%10:%11:%12")
                            .arg(eqgrid)
                            .arg(eqjgrid)
                            .arg(eqagrid)
                            .arg(eqline)
                            .arg(merline)
                            .arg(eqpline)
                            .arg(carpnt)
                            .arg(conline)
                            .arg(conlabel)
                            .arg(conboun)
                            .arg(conart)
                            .arg(conartI);
        sendMessage(str);
    }
    if(msg.startsWith("373"))
    {
        QStringList strList;
        strList=msg.split(":");

        GridLinesMgr* glmgr = (GridLinesMgr*)GETSTELMODULE(GridLinesMgr);
        LandscapeMgr* lmgr = (LandscapeMgr*)GETSTELMODULE(LandscapeMgr);
        ConstellationMgr* cmgr = (ConstellationMgr*)GETSTELMODULE(ConstellationMgr);

        glmgr->setFlagEquatorGrid(strList[1].toInt());
        glmgr->setFlagEquatorJ2000Grid(strList[2].toInt());
        glmgr->setFlagAzimuthalGrid(strList[3].toInt());
        glmgr->setFlagEquatorLine(strList[4].toInt());
        glmgr->setFlagMeridianLine(strList[5].toInt());
        glmgr->setFlagEclipticLine(strList[6].toInt());
        lmgr->setFlagCardinalsPoints(strList[7].toInt());

        cmgr->setFlagLines(strList[8].toInt());
        cmgr->setFlagLabels(strList[9].toInt());
        cmgr->setFlagBoundaries(strList[10].toInt());
        cmgr->setFlagArt(strList[11].toInt());
        cmgr->setArtIntensity(strList[12].toDouble());

    }
    ///---

    if(msg.startsWith("380"))
    {
        QStringList strList;
        strList=msg.split(":");
        QString str=strList[1];
        str.replace(QString("="), QString(" "));

        ConstellationMgr* cmgr = (ConstellationMgr*)GETSTELMODULE(ConstellationMgr);
        cmgr->ShowHideArt(str,strList[2].toInt());
    }
    if(msg=="381")
    {
        ConstellationMgr* cmgr = (ConstellationMgr*)GETSTELMODULE(ConstellationMgr);
        QStringList strList=cmgr->GetAllNameI18n();
        strList.sort();
        QString str=strList.join(":");
        sendMessage("381:"+str);
    }
    if(msg.startsWith("382"))
    {
       // QMessageBox::information(0, "Stellarium",msg,"ok");
        QStringList strList;
        strList=msg.split(":");
        ConstellationMgr* cmgr = (ConstellationMgr*)GETSTELMODULE(ConstellationMgr);
        cmgr->ShowHideZodiac(strList[1].toInt(),false,false);
    }
    if(msg=="374")
    {
        LandscapeMgr* lmgr = (LandscapeMgr*)GETSTELMODULE(LandscapeMgr);
        bool blan=lmgr->getFlagLandscape();
        bool bfog=lmgr->getFlagFog();
        bool bdef=lmgr->getDefaultLandscapeID()==lmgr->getCurrentLandscapeID();
        QStringList sLan=lmgr->getAllLandscapeNames(false);
        QString strLan=sLan.join(":");
        QString idef=lmgr->getDefaultLandscapeID();

        QString str=QString("374:%1:%2:%3:%4:%5")
                            .arg(blan)
                            .arg(bfog)
                            .arg(bdef)
                            .arg(idef)
                            .arg(strLan);
        sendMessage(str);

    }
    if(msg.startsWith("375"))
    {
        QStringList strList;
        strList=msg.split(":");
        LandscapeMgr* lmgr = (LandscapeMgr*)GETSTELMODULE(LandscapeMgr);
        lmgr->setFlagLandscape(strList[1].toInt());
        lmgr->setFlagFog(strList[2].toInt());
        if (strList[3].toInt()==1)
        {
            lmgr->setDefaultLandscapeID(lmgr->getCurrentLandscapeID());
        }

    }
    if(msg.startsWith("376"))
    {
        QStringList strList;
        strList=msg.split(":");
        LandscapeMgr* lmgr = (LandscapeMgr*)GETSTELMODULE(LandscapeMgr);
    	QString str=strList[1];
        str.replace(QString("="), QString(" "));
        lmgr->setCurrentLandscapeName(str);
    }
    if(msg=="400")
    {
        saveCurrentViewOptions();
    }

    if(msg.startsWith("401"))
    {
        //QDateTime dt=QDateTime::currentDateTime().addSecs(5);
        //Gecikme(dt);
        QStringList strList;
        strList=msg.split(":");
        tsaniye=strList[1].toInt();
        //QMessageBox::information(0, "Stellarium",strList[1],"ok");
    }
    if(msg.startsWith("402"))
    {
        QStringList strList;
        strList=msg.split(":");

        StelNavigator* nav = StelApp::getInstance().getCore()->getNavigator();
        Q_ASSERT(nav);
        //Vec3d localVision=nav->getLocalVision();
        //QString dirStr = QString("%1,%2,%3").arg(localVision[0]).arg(localVision[1]).arg(localVision[2]);
        //QMessageBox::information(0, "Stellarium",dirStr ,"ok");
        Vec3d initViewPos;
        initViewPos = StelUtils::strToVec3f(strList[1]).toVec3d();
        //nav->setLocalVision(initViewPos);
        core->movementMgr->setViewDirectionJ2000(initViewPos);
    }
    if(msg.startsWith("403"))
    {
       /*
        QStringList strList;
        strList=msg.split(":");

        MovementMgr* mvmgr = (MovementMgr*)GETSTELMODULE("MovementMgr");
        mvmgr->panView(strList[1].toDouble(),0.0);
        */
    }

    if(msg.startsWith("411"))
    {
        QStringList strList;
        strList=msg.split(":");

        StelCore* core=s_app->getCore();

        Gecikme(strList[1]);

        core->ALLFader.setDuration(strList[3].toInt());
        if(strList[2].toInt()==0)
        {
            core->ALLFader=false;
        }
        else
        {
            core->ALLFader=true;
        }
    }
    if(msg.startsWith("TEST"))
    {
        const QList<StelObjectP> pSel=StelApp::getInstance().getStelObjectMgr().getSelectedObject();
        if(pSel[0]->getType()=="Planet")
        {
            Planet* pl = dynamic_cast<Planet*>(pSel[0].data());
            pl->setSphereScale(2000);
        }
    }
}

double mysocket::newJd()
{
  double jd;
  StelUtils::getJDFromDate(&jd,year, month, day, hour, minute, second);
  jd -= (StelApp::getInstance().getLocaleMgr().getGMTShift(jd)/24.0); // local tz -> UTC
  return jd;
}
void mysocket::setDateTime(double newJd)
{
  StelUtils::getDateFromJulianDay(newJd, &year, &month, &day);
  StelUtils::getTimeFromJulianDay(newJd, &hour, &minute, &second);
}
StelObjectP mysocket::selectObject(QString name)
{
        if (name=="") return NULL;
	else if (StelApp::getInstance().getStelObjectMgr().findAndSelectI18n(name))
	{
		//MovementMgr* mvmgr = (MovementMgr*)GETSTELMODULE("MovementMgr");
		const QList<StelObjectP> newSelected = StelApp::getInstance().getStelObjectMgr().getSelectedObject();
                return newSelected[0];
	}
}
void mysocket::gotoObject(QString name)
{
	if (name=="") return;
	else if (StelApp::getInstance().getStelObjectMgr().findAndSelectI18n(name))
	{
                StelMovementMgr* mvmgr = (StelMovementMgr*)GETSTELMODULE(StelMovementMgr);
		const QList<StelObjectP> newSelected = StelApp::getInstance().getStelObjectMgr().getSelectedObject();
		if (!newSelected.empty())
		{
                        mvmgr->moveToObject(newSelected[0],mvmgr->getAutoMoveDuration(),1);
			mvmgr->setFlagTracking(false);
		}
	}



}

void mysocket::sendMessage(const QString &message)
{
    if (!sck->flush())
    {
        //sck->write(message.toLatin1().data());
        if (b_dataSend)
        {
            QString string = message;
            QTextCodec *codec = QTextCodec::codecForName("ISO 8859-9");
            QByteArray encodedString = codec->fromUnicode(string);
            sck->write(encodedString);
        }
    }
}

void mysocket::saveCurrentViewOptions()
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

	// configuration dialog / main tab
        /*
        QString langName = StelApp::getInstance().getLocaleMgr().getAppLanguage();
        conf->setValue("localization/app_locale", Translator::nativeNameToIso639_1Code(langName));

	if (gui->getInfoPanel()->getInfoTextFilters() == (StelObject::InfoStringGroup)0)
		conf->setValue("gui/selected_object_info", "none");
	else if (gui->getInfoPanel()->getInfoTextFilters() == StelObject::InfoStringGroup(StelObject::ShortInfo))
		conf->setValue("gui/selected_object_info", "short");
	else
		conf->setValue("gui/selected_object_info", "all");
        */

	// configuration dialog / navigation tab
	conf->setValue("navigation/flag_enable_zoom_keys", mvmgr->getFlagEnableZoomKeys());
	conf->setValue("navigation/flag_enable_mouse_navigation", mvmgr->getFlagEnableMouseNavigation());
	conf->setValue("navigation/flag_enable_move_keys", mvmgr->getFlagEnableMoveKeys());
	conf->setValue("navigation/startup_time_mode", nav->getStartupTimeMode());
	conf->setValue("navigation/today_time", nav->getInitTodayTime());
	conf->setValue("navigation/preset_sky_time", nav->getPresetSkyTime());
//        conf->setValue("navigation/init_fov", proj->getInitFov());

	// configuration dialog / tools tab
	conf->setValue("gui/flag_show_flip_buttons", gui->getFlagShowFlipButtons());
//	conf->setValue("video/distorter", StelAppGraphicsScene::getInstance().getViewPortDistorterType());
//	conf->setValue("projection/viewport", Projector::maskTypeToString(proj->getMaskType()));
//	conf->setValue("viewing/flag_gravity_labels", proj->getFlagGravityLabels());
	conf->setValue("navigation/auto_zoom_out_resets_direction", mvmgr->getFlagAutoZoomOutResetsDirection());
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
void mysocket::Gecikme(QString strdt)
{
    strdt.replace(QString(","), QString(" "));
    strdt.replace(QString("-"), QString(":"));

    while (QDateTime::currentDateTime().toString("dd.MM.yyyy hh:mm:ss")!= strdt)
    {
      //Gecikme
      QCoreApplication::processEvents(QEventLoop::AllEvents );
    }

}
