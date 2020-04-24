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


#ifndef SNTPCLIENT_H
#define SNTPCLIENT_H

#include <Qt>
#include <QtNetwork/QtNetwork>

class sntpclient: public QObject
{
    Q_OBJECT
public:
    sntpclient(QString timeserver,bool set);
    bool result;
    void connectserver();
private:
    QUdpSocket* udpsocket;
    bool settime;
    QString m_timeserver;

    void SetClientSystemTime(QDateTime dt);

private slots:
    void connectsucess();
    void readingDataGrams();

signals:
    void validserver(void);
};

#endif // SNTPCLIENT_H
