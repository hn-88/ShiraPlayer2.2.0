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

#include "qplayimagewidget.h"
#include "QImagePropWidget.h"
#include "StelTranslator.hpp"
//#include "presenter/flowlayout.h"

QPlayImageWidget::QPlayImageWidget(QWidget *parent)
    : QWidget(parent)
{
    //    int m_widgetHeight = 200;
    //    int m_widgetWidth = 200;
    //
    //    setAcceptDrops(true);
    //    //setMinimumSize(m_widgetWidth, m_widgetHeight);
    //
    //    //QVBoxLayout * vLayout = new QVBoxLayout(this);
    //    QVBoxLayout * vLayout = new QVBoxLayout(this);
    //    QScrollArea *scrollArea = new QScrollArea(this);
    //    vLayout->setMargin(0);
    //    vLayout->addWidget(scrollArea);
    //
    //    //set up parameters for scroll area
    //    scrollArea->setWidgetResizable(false);
    //
    //    //create another widget with a QVBoxLayout
    //    scrollAreaVLayout = new QVBoxLayout();
    //    QWidget* scrollAreaWidgetContents = new QWidget();
    //    scrollAreaWidgetContents->setLayout(scrollAreaVLayout);
    //    scrollAreaVLayout->setSizeConstraint(QLayout::SetFixedSize);
    //
    //    //add scrolling widget to scroller
    //    scrollArea->setWidget(scrollAreaWidgetContents);
    //
    ////    scrollListWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    ////    scrollListWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    ////    scrollListWidget->setFlow(QListView::LeftToRight);
    ////    scrollListWidget->setWrapping(true);
    ////    scrollListWidget->setResizeMode(QListView::Adjust);
    ////    scrollListWidget->setMovement(QListView::Static);
    ////    scrollListWidget->setBatchSize(80);
    //
    //
    //    vLayout->addWidget(scrollListWidget);


    //-----

    int m_widgetHeight = 200;
    int m_widgetWidth = 200;
    //
    setAcceptDrops(true);
    setMinimumSize(m_widgetWidth, m_widgetHeight);
    //
    //    QVBoxLayout * vLayout = new QVBoxLayout(this);
    //    QScrollArea *scrollArea = new QScrollArea(this);
    //    vLayout->setMargin(0);
    //    vLayout->addWidget(scrollArea);
    //
    //    //set up parameters for scroll area
    //    scrollArea->setWidgetResizable(false);
    //
    //    //create another widget with a QVBoxLayout
    //    scrollAreaVLayout = new FlowLayout();
    //    QWidget* scrollAreaWidgetContents = new QWidget();
    //    scrollAreaWidgetContents->setLayout(scrollAreaVLayout);
    //    scrollAreaVLayout->setSizeConstraint(QLayout::SetFixedSize);
    //
    //    //add scrolling widget to scroller
    //    scrollArea->setWidget(scrollAreaWidgetContents);


    QVBoxLayout* vLayout = new QVBoxLayout(this);
    stackListWidget= new QListWidget(this);
    stackListWidget->setFlow(QListView::LeftToRight);
    stackListWidget->setIconSize(QSize(100,100));
    stackListWidget->setResizeMode(QListView::Adjust);
    stackListWidget->setBatchSize(80);
    stackListWidget->setMovement(QListView::Static);
    stackListWidget->setViewMode(QListView::IconMode);
    vLayout->setMargin(0);
    vLayout->addWidget(stackListWidget);

    stackedWidget = new QStackedWidget(this);
    vLayout->addWidget(stackedWidget);

    QGroupBox* grpBox = new QGroupBox(this);
    grpBox->setTitle(q_("Active Images"));
    grpBox->setLayout(vLayout);

    QVBoxLayout* vLayoutBox = new QVBoxLayout(this);
    vLayoutBox->setMargin(0);
    vLayoutBox->addWidget(grpBox);
    setLayout(vLayoutBox);

    //scrollAreaVLayout = new FlowLayout();
    //setLayout(scrollAreaVLayout);

    connect(stackListWidget, SIGNAL(currentItemChanged(QListWidgetItem *, QListWidgetItem *)), this, SLOT(changePage(QListWidgetItem *, QListWidgetItem*)));
    connect(stackedWidget,SIGNAL(widgetRemoved(int)),this,SLOT(widgetRemoved(int)));
}

QPlayImageWidget::~QPlayImageWidget()
{

}

void QPlayImageWidget::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasFormat("image/x-shirabrowser"))
        event->accept();
    else
        event->ignore();
}

void QPlayImageWidget::dragLeaveEvent(QDragLeaveEvent *event)
{
    QRect updateRect = highlightedRect;
    highlightedRect = QRect();
    update(updateRect);
    setStyleSheet( "");
    event->accept();
}


void QPlayImageWidget::dragMoveEvent(QDragMoveEvent *event)
{
    QRect updateRect = highlightedRect.united(targetSquare(event->pos()));

    if (event->mimeData()->hasFormat("image/x-shirabrowser"))
    {
        QString newStyleSheet = "*{  background-color: rgb(82, 68, 174);}";

        if (oldStyleSheet == newStyleSheet)
            oldStyleSheet = styleSheet();
        setStyleSheet(newStyleSheet);
        highlightedRect = targetSquare(event->pos());
        event->setDropAction(Qt::MoveAction);
        event->accept();
    } else {
        highlightedRect = QRect();
        event->ignore();
    }

    update(updateRect);
}

void QPlayImageWidget::dropEvent(QDropEvent *event)
{
    if (event->mimeData()->hasFormat("image/x-shirabrowser"))
    {
        QByteArray pieceData = event->mimeData()->data("image/x-shirabrowser");
        QDataStream dataStream(&pieceData, QIODevice::ReadOnly);
        QPixmap pixmap;
        QString path;
        QString filename;
        bool isVideo = false;

        dataStream >> pixmap >> path >>filename>>isVideo;

        if (isVideo)
        {
            if (StelUtils::isUnicodeInclude(filename))
            {
                QMessageBox::warning(0,"Warning!","File must be in ascii character path!");
                event->ignore();
                highlightedRect = QRect();
                setStyleSheet( "");
                return;
            }
        }

        QImagePropWidget* imgpropWidget = new QImagePropWidget(stackedWidget,path,filename,isVideo);
        imgpropWidget->setImage(pixmap,filename);
        imgpropWidget->setGeometry(0,0,120,120);

        //imgpropWidget->getGroupBox()->setStyleSheet("");
        //QWidget* w = imgpropWidget->getGroupBox();
        //w->set
        //w->setStyleSheet(styleSheet());
        //scrollAreaVLayout->addWidget(imgpropWidget);

        //        QListWidgetItem* item = new QListWidgetItem();
        //        scrollListWidget->addItem(item);
        //        item->setSizeHint(QSize(imgpropWidget->getGroupBox()->width(),imgpropWidget->getGroupBox()->height()));
        //        scrollListWidget->setItemWidget(item ,imgpropWidget->getGroupBox());

        //QMessageBox::information(0,"",oldStyleSheet,0,0);
        highlightedRect = QRect();
        setStyleSheet( "");


        QListWidgetItem *item = new QListWidgetItem;
        item->setIcon(QIcon( QPixmap::fromImage(imgpropWidget->getPixmap()->toImage()) ));
        item->setData(Qt::DisplayRole, imgpropWidget->getCaption());
        item->setData(Qt::WhatsThisRole,stackListWidget->count() );
        stackListWidget->addItem(item);


        stackedWidget->insertWidget(item->data(Qt::WhatsThisRole).toInt(),imgpropWidget);
        stackedWidget->setCurrentIndex(item->data(Qt::WhatsThisRole).toInt());

    } else {
        event->ignore();
        highlightedRect = QRect();
        setStyleSheet( "");
    }
}

void QPlayImageWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter;
    painter.begin(this);
    //painter.fillRect(event->rect(), Qt::white);

//    if (highlightedRect.isValid()) {
//        painter.setBrush(QColor("#ffcccc"));
//        painter.setPen(Qt::NoPen);
//        painter.drawRect(highlightedRect.adjusted(0, 0, -1, -1));
//    }

    //for (int i = 0; i < pieceRects.size(); ++i) {
    //    painter.drawPixmap(pieceRects[i], piecePixmaps[i]);
    //}
    painter.end();
}

void QPlayImageWidget::changePage(QListWidgetItem *current, QListWidgetItem *previous)
{
    if (!current)
        current = previous;
    stackedWidget->setCurrentIndex(current->data(Qt::WhatsThisRole).toInt());
}
void QPlayImageWidget::widgetRemoved(int index)
{
    delete stackListWidget->item(index);
    //QMessageBox::information(0,"",QString("%0").arg(index),0,0);
}
const QRect QPlayImageWidget::targetSquare(const QPoint &position) const
{
    return QRect(position.x(), position.y(), 120, 120);
}
