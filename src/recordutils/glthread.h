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

#ifndef MYTHREAD_H
#define MYTHREAD_H


#include <QThread>
#include <QMutex>
#include <QImage>

#include "QVideoEncoder.h"

class glthread : public QThread
{
    Q_OBJECT
public:
    glthread(QObject *parent = 0);

    void appendQIMGList(QImage img);

signals:
    void encodeImage(QImage img);

protected:
    void run();

private:
    QVector<QImage> qIV;
};



#endif
