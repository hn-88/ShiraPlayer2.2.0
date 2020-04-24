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

#include "sntpclient.h"
#include <QtNetwork/QtNetwork>

#include "QMessageBox"

#ifdef Q_OS_WIN32
    #include <windows.h>
#endif

sntpclient::sntpclient(QString timeserver,bool set)
{
    settime = set;
    result = false;
    m_timeserver = timeserver;
    udpsocket=new QUdpSocket(this);
    connect(udpsocket,SIGNAL(connected()),this,SLOT(connectsucess()));
    connect(udpsocket,SIGNAL(readyRead()),this,SLOT(readingDataGrams()));
}

void sntpclient::connectserver()
{
    udpsocket->connectToHost(m_timeserver,123);
}
void sntpclient::connectsucess()
{
    qint8 LI=0;
    qint8 VN=3;
    qint8 MODE=3;
    qint8 STRATUM=0;
    qint8 POLL=4;
    qint8 PREC=-6;
    QDateTime Epoch(QDate(1900, 1, 1));
    qint32 second=quint32(Epoch.secsTo(QDateTime::currentDateTime()));

    qint32 temp=0;
    QByteArray timeRequest(48, 0);
    timeRequest[0]=(LI <<6) | (VN <<3) | (MODE);
    timeRequest[1]=STRATUM;
    timeRequest[2]=POLL;
    timeRequest[3]=PREC & 0xff;
    timeRequest[5]=1;
    timeRequest[9]=1;
    timeRequest[40]=(temp=(second&0xff000000)>>24);
    temp=0;
    timeRequest[41]=(temp=(second&0x00ff0000)>>16);
    temp=0;
    timeRequest[42]=(temp=(second&0x0000ff00)>>8);
    temp=0;
    timeRequest[43]=((second&0x000000ff));
    udpsocket->flush();
    udpsocket->write(timeRequest);
    udpsocket->flush();
}

void sntpclient::readingDataGrams()
{
    QByteArray newTime;
    QDateTime Epoch(QDate(1900, 1, 1));
    QDateTime unixStart(QDate(1970, 1, 1));
    do
    {
        newTime.resize(udpsocket->pendingDatagramSize());
        udpsocket->read(newTime.data(), newTime.size());
    }while(udpsocket->hasPendingDatagrams());

    emit(validserver());

    QByteArray TransmitTimeStamp ;
    TransmitTimeStamp=newTime.right(8);
    quint32 seconds=TransmitTimeStamp[0];
    quint8 temp=0;
    for(int j=1;j<=3;j++)
    {
        seconds=seconds<<8;
        temp=TransmitTimeStamp[j];
        seconds=seconds+temp;
    }
    QDateTime time;
    time.setTime_t(seconds-Epoch.secsTo(unixStart));

    //QMessageBox::critical(0,"",time.toString(),0,0);

    this->udpsocket->disconnectFromHost();
    this->udpsocket->close();
    this->udpsocket = NULL;

    if(settime)
    {
        SetClientSystemTime(time);
    }


}

void sntpclient::SetClientSystemTime(QDateTime dt)
{
    SYSTEMTIME systime;
    systime.wYear = dt.date().year();
    systime.wMonth = dt.date().month();
    systime.wDay = dt.date().day();
    systime.wHour = dt.time().hour();
    systime.wMinute = dt.time().minute();
    systime.wSecond = dt.time().second();
    systime.wMilliseconds = dt.time().msec();

    SetLocalTime(&systime);
}
