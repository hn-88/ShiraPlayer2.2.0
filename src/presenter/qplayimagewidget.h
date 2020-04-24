/*
 * ShiraPlayer(TM)
 * Copyright (C) 2012 Asaf Yurdakul
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

#ifndef QPLAYIMAGEWIDGET_H
#define QPLAYIMAGEWIDGET_H

#include <QWidget>
#include <QtGui>
#include <QListWidget>
#include <QStackedWidget>

#include "StelDialog.hpp"
#include "presenter/flowlayout.h"

class QPlayImageWidget : public QWidget
{
    Q_OBJECT

public:
    QPlayImageWidget(QWidget *parent);
    ~QPlayImageWidget();

    FlowLayout * scrollAreaVLayout;

    QListWidget* stackListWidget;
    QStackedWidget* stackedWidget;
protected:
    void dragEnterEvent(QDragEnterEvent *event);
    void dragLeaveEvent(QDragLeaveEvent *event);
    void dragMoveEvent(QDragMoveEvent *event);
    void dropEvent(QDropEvent *event);
    void paintEvent(QPaintEvent *event);
private:
    const QRect targetSquare(const QPoint &position) const;
    QRect highlightedRect;
    QString oldStyleSheet;

private slots:
    void changePage(QListWidgetItem *current, QListWidgetItem *previous);
    void widgetRemoved ( int index );

};

#endif // QPLAYIMAGEWIDGET_H
