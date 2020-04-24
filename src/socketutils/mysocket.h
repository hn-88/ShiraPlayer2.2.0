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

#ifndef _MYSOCKET_H_
#define _MYSOCKET_H_


#include "StelApp.hpp"
#include "StelObject.hpp"

#include <QtNetwork/QTcpServer>
#include <QWidget>
#include <QtGui>
#include <QDateTime>

class mysocket : public QObject
{
    Q_OBJECT
public:
    mysocket(StelApp* app);
	~mysocket();
    //mysocket(QWidget *parent = 0);
    QTcpServer* tcpServer;
    QTcpSocket* sck;
    StelApp* s_app;
    void mesajlari_uygula(QString msg);
    void sendMessage(const QString &message);

    double newJd();
    void setDateTime(double newJd);
    StelObjectP selectObject(QString name);
    void gotoObject(QString name);
    void saveCurrentViewOptions();
    void Gecikme(QString strdt);

public slots:
    void DoConnect();
    void readIncomingData();
private:
	int year;
	int month;
	int day;
	int hour;
	int minute;
	int second;

	bool b_dataSend;
	int tsaniye;
};
#endif
