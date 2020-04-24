/*
 * ShiraPlayer(TM)
 * Copyright (C) 2008 Fabien Chereau
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

#include "StelApp.hpp"
#include "StelDialog.hpp"
#include "StelMainGraphicsView.hpp"
#include "StelMainWindow.hpp"
#include "StelTranslator.hpp"
#include "shiraplayerform.hpp"

#include <QDebug>
#include <QDialog>
#include <QGraphicsProxyWidget>
#include <QStyleOptionGraphicsItem>

class CustomProxy : public QGraphicsProxyWidget
{
public:
    CustomProxy(QGraphicsItem *parent = 0, Qt::WindowFlags wFlags = 0) : QGraphicsProxyWidget(parent, wFlags)
    {
        setFocusPolicy(Qt::StrongFocus);
        //setWindowFlags(windowFlags() | Qt::CustomizeWindowHint);
       // setWindowFlags(Qt::CustomizeWindowHint);
        setWindowFlags(Qt::Window | Qt::CustomizeWindowHint | Qt::WindowTitleHint);
    }
    //! Reimplement this method to add windows decorations. Currently there are invisible 2 px decorations
    void paintWindowFrame(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
    {
        //QStyleOptionTitleBar bar;
        //initStyleOption(&bar);
        //bar.subControls = QStyle::SC_TitleBarLabel;
        //qWarning() << style()->subControlRect(QStyle::CC_TitleBar, &bar, QStyle::SC_TitleBarCloseButton);
        //QGraphicsProxyWidget::paintWindowFrame(painter, option, widget);
    }
protected:

    virtual bool event(QEvent* event)
    {
        if (event->type()==QEvent::WindowDeactivate)
        {
            widget()->setWindowOpacity(0.9); // Eskisi 0.4 idi
        }
        if (event->type()==QEvent::WindowActivate)
        {
            widget()->setWindowOpacity(1.0);
        }
        return QGraphicsProxyWidget::event(event);
    }
};

StelDialog::StelDialog() : dialog(NULL)
{
}

StelDialog::~StelDialog()
{
}


void StelDialog::close()
{
    setVisible(false);
    StelMainGraphicsView::getInstance().scene()->setActiveWindow(0);
    ((QGraphicsWidget*)StelMainGraphicsView::getInstance().getStelAppGraphicsWidget())->setFocus(Qt::OtherFocusReason);
}

bool StelDialog::visible() const
{
    return dialog!=NULL && dialog->isVisible();
}

void StelDialog::setVisible(bool v)
{
    if (v)
    {
        if (StelApp::getInstance().getstartedFlyby() )
        {
           QMessageBox::warning(0,q_("Warning!"),q_("Please complete flyby operations first!"));
           return;
        }

        //QSize screenSize = StelMainGraphicsView::getInstance().size();
        QSize screenSize = ShiraPlayerForm::getInstance().size();
        if (dialog)
        {
            dialog->show();
            //StelMainGraphicsView::getInstance().scene()->setActiveWindow(proxy);
            ShiraPlayerForm::getInstance().scene()->setActiveWindow(proxy);
            // If the main window has been resized, it is possible the dialog
            // will be off screen.  Check for this and move it to a visible
            // position if necessary
            QPointF newPos = proxy->pos();
            if (newPos.x()>=screenSize.width())
                newPos.setX(screenSize.width() - dialog->size().width());
            if (newPos.y()>=screenSize.height())
                newPos.setY(screenSize.height() - dialog->size().height());
            if (newPos != dialog->pos())
                proxy->setPos(newPos);

            proxy->setFocus();
            //ASAF
            emit load(true);
            return;
        }
        dialog = new QDialog(NULL);

        //dialog->setAttribute(Qt::WA_OpaquePaintEvent, true);
        connect(dialog, SIGNAL(rejected()), this, SLOT(close()));
        createDialogContent();

        proxy = new CustomProxy(NULL, Qt::Tool);
        proxy->setWidget(dialog);
        QRectF bound = proxy->boundingRect();

        // centre with dialog according to current window size.
        proxy->setPos((int)((screenSize.width()-bound.width())/2), (int)((screenSize.height()-bound.height())/2));
        //StelMainGraphicsView::getInstance().scene()->addItem(proxy);
        ShiraPlayerForm::getInstance().scene()->addItem(proxy);
        proxy->setWindowFrameMargins(2,0,2,2);

        // The caching is buggy on all plateforms with Qt 4.5.2
        proxy->setCacheMode(QGraphicsItem::ItemCoordinateCache);

        proxy->setZValue(100);

        ShiraPlayerForm::getInstance().scene()->setActiveWindow(proxy);
        proxy->setFocus();
        //ASAF
        emit load(true);
    }
    else
    {
        dialog->hide();
        emit visibleChanged(false);
        //proxy->clearFocus();
        ShiraPlayerForm::getInstance().scene()->setActiveWindow(0);
    }
}

void StelDialog::setVisibleAsChild(QWidget* parent,bool v)
{
    if (!dialog)
    {
        dialog = new QWidget(parent);
        createDialogContent();
        dialog->setParent(parent);
        prepareAsChild();
    }
    else
    {
        dialog->setParent(parent);
    }
    if (!v)
        dialog->hide();
    else
        dialog->show();

}
