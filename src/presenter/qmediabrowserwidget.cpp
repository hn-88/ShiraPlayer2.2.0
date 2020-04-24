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

#include <QtGui>
#include <QMessageBox>
#include "StelTranslator.hpp"

#include "qmediabrowserwidget.h"

QMediaBrowserWidget::QMediaBrowserWidget(QWidget *parent,QString mediapath)
    : QListWidget(parent),flat_media_path(mediapath)
{
    m_PieceSize = 75;

    setDragEnabled(true);
    setViewMode(QListView::IconMode);
    setIconSize(QSize(m_PieceSize, m_PieceSize));
    setSpacing(10);
    setAcceptDrops(true);
    setDropIndicatorShown(true);

    // TODO Klasör varmý kontrolü

    flat_media_path = flat_media_path+'/';
    QDir mediaDir(flat_media_path);
    QStringList filters;
    filters << "*.bmp" << "*.png" << "*.jpg";
    mediaDir.setNameFilters(filters);
    QStringList list = mediaDir.entryList(filters);

    for(int i=0;i<list.count();i++)
    {
        QPixmap px= openImage(flat_media_path + list[i]);
        if(px.width()>m_PieceSize )
        {
            int scalerate = px.width() / m_PieceSize ;
            px =px.scaled(px.width()/scalerate, px.height()/scalerate, Qt::IgnoreAspectRatio, Qt::FastTransformation);
        }
        addMediaImage(px, flat_media_path,list[i]);

    }

    filters.clear();
    filters << "*.avi" << "*.mpeg" << "*.mpg" << "*.mp4" << "*.wmv" << "*.mov" << "*.flv" << "*.vob"  ;
    mediaDir.setNameFilters(filters);
    list = mediaDir.entryList(filters);

    for(int i=0;i<list.count();i++)
    {
        addMediaImage(QPixmap(), flat_media_path,list[i]);
    }
}

QMediaBrowserWidget::~QMediaBrowserWidget()
{

}

void QMediaBrowserWidget::resizeEvent(QResizeEvent *event)
{
    sortItems();
}


QPixmap QMediaBrowserWidget::openImage(const QString &path)
{
    QString fileName = path;
    QPixmap newImage;

    if (!fileName.isEmpty()) {

        if (!newImage.load(fileName)) {
            QMessageBox::warning(this,
                                 q_("Open Image"),
                                 q_("The image file could not be loaded."),
                                 QMessageBox::Cancel);
            return newImage;
        }
    }
    return newImage;
}

void QMediaBrowserWidget::addMediaImage(QPixmap pixmap, QString path,QString caption)
{
    QListWidgetItem *pItem = new QListWidgetItem(this);
    pItem->setData(Qt::UserRole+1, path);
    pItem->setData(Qt::UserRole+2, caption);

    if (!pixmap.isNull())
    {
        pItem->setIcon(QIcon(pixmap));
        pItem->setData(Qt::UserRole, pixmap);
        pItem->setData(Qt::UserRole+3, false);
    }
    else
    {
        pItem->setIcon(QIcon(":/media/gui/presenter/media.png"));
        pItem->setData(Qt::UserRole, QPixmap(":/media/gui/presenter/media.png"));
        pItem->setData(Qt::UserRole+3, true); // video ise
    }

    pItem->setText(caption);
    pItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled );

}

void QMediaBrowserWidget::startDrag(Qt::DropActions /*supportedActions*/)
{
    QListWidgetItem *item = currentItem();

    QByteArray itemData;
    QDataStream dataStream(&itemData, QIODevice::WriteOnly);
    QPixmap pixmap = qvariant_cast<QPixmap>(item->data(Qt::UserRole));
    QString  path = item->data(Qt::UserRole+1).toString();
    QString  text = item->data(Qt::UserRole+2).toString();
    bool isVideo = item->data(Qt::UserRole+3).toBool();

    dataStream << pixmap << path << text << isVideo;

    QMimeData *mimeData = new QMimeData;
    mimeData->setData("image/x-shirabrowser", itemData);

    QDrag *drag = new QDrag(this);
    drag->setMimeData(mimeData);
    drag->setHotSpot(QPoint(pixmap.width()/2, pixmap.height()/2));
    drag->setPixmap(pixmap);

    if (drag->exec(Qt::MoveAction) == Qt::MoveAction)
        sortItems();
    //delete takeItem(row(item));
}

void QMediaBrowserWidget::dropEvent(QDropEvent *event)
{
    //  if (event->mimeData()->hasFormat("image/x-shirabrowser")) {
    //      QByteArray pieceData = event->mimeData()->data("image/x-shirabrowser");
    //      QDataStream dataStream(&pieceData, QIODevice::ReadOnly);
    //      QPixmap pixmap;
    //      QPoint location;
    //QString text;
    //      dataStream >> pixmap >> location >> text ;

    ////addMediaImage(pixmap, location,text);

    //      event->setDropAction(Qt::MoveAction);
    //      event->accept();
    //  } else
    //      event->ignore();
}

void QMediaBrowserWidget::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasFormat("image/x-shirabrowser"))
        event->accept();
    else
        event->ignore();
}

void QMediaBrowserWidget::dragMoveEvent(QDragMoveEvent *event)
{
    if (event->mimeData()->hasFormat("image/x-shirabrowser")) {
        event->setDropAction(Qt::MoveAction);
        event->accept();
    } else
        event->ignore();
}

