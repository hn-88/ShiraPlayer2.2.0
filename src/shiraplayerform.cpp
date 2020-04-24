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

#include "shiraplayerform.hpp"
#include "StelMainGraphicsView.hpp"
#include "StelGui.hpp"
#include "StelApp.hpp"
#include "StelCore.hpp"
#include "ConfigurationDialog.hpp"
#include "servermanager.h"
#include "recordmanager.hpp"
#include "ViewDialog.hpp"
#include "ScriptConsole.hpp"
#include "HelpDialog.hpp"
#include "LocationDialog.hpp"
#include "DateTimeDialog.hpp"
#include "StelFileMgr.hpp"
#include "StelMainWindow.hpp"
#include "StelTranslator.hpp"
#include "shiraprojector.h"

// Initialize static variables
ShiraPlayerForm* ShiraPlayerForm::singleton = NULL;

ShiraPlayerForm::ShiraPlayerForm(QWidget* parent) :
    QGraphicsView(parent)
{
    Q_ASSERT(!singleton);
    singleton = this;

    // Avoid white background at init
    //setAttribute(Qt::WA_PaintOnScreen);
    setAttribute(Qt::WA_NoSystemBackground);
    setAttribute(Qt::WA_OpaquePaintEvent);
    setAutoFillBackground(true);
    setBackgroundRole(QPalette::Window);
    QPalette pal;
    pal.setColor(QPalette::Window, Qt::black);
    setPalette(pal);

    // Allows for precise FPS control
    setFrameShape(QFrame::NoFrame);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setFocusPolicy(Qt::StrongFocus);

    setScene(new QGraphicsScene());

    this->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(const QPoint&)),
            this, SLOT(ShowContextMenu(const QPoint&)));


    QPixmap pm = QPixmap(":/graphicGui/gui/logo.png");
    texLogo =  scene()->addPixmap(pm);
    texLogo->setPos(0, 0);

    //fitInView(QRect(0,0,this->width(),this->height()),Qt::KeepAspectRatio);

}

void ShiraPlayerForm::SettextLogoPos(int x,int y)
{
    texLogo->setPos(x, y);
}

void ShiraPlayerForm::ShowContextMenu(const QPoint& pos)
{
    // for most widgets
    QPoint globalPos = this->mapToGlobal(pos);
    // for QAbstractScrollArea and derived classes you would use:
    // QPoint globalPos = myWidget->viewport()->mapToGlobal(pos);

    QMenu myMenu;
    myMenu.addAction(q_("Window List"));
    myMenu.addSeparator();
    myMenu.addAction(q_("Media Manager"));
    //myMenu.addAction("Media Record Manager");
    myMenu.addAction(q_("Preview Window"));
    myMenu.addSeparator();
    myMenu.addAction(q_("Configration Window"));
    myMenu.addAction(q_("View Options Window"));
    //myMenu.addAction("Search Window");
    //myMenu.addAction("Date/Time Window");
    //myMenu.addAction("Location Window");
    myMenu.addAction(q_("Script Console"));
    if (StelMainWindow::getInstance().projdll->getPluginName()=="Multiprojector")
    {
        myMenu.addSeparator();
        myMenu.addAction(q_("Projector Settings"));
    }
    myMenu.addSeparator();
    myMenu.addAction(q_("Exit"));
    //myMenu.addAction("Help Window");

    QAction* selectedItem = myMenu.exec(globalPos);
    if (selectedItem)
    {
        if ( (selectedItem->text() == q_("Configration Window") ) ||
             (selectedItem->text() == q_("View Options Window") ) ||
             (selectedItem->text() == q_("Script Console")) )
        {
            if (StelApp::getInstance().getstartedFlyby() )
            {
                QMessageBox::warning(0,q_("Warning!"),q_("Please complete flyby operations first!"));
                return;
            }
        }

        StelGui* sgui = dynamic_cast<StelGui*>(StelApp::getInstance().getGui());
        QGraphicsProxyWidget* sproxy= NULL;
        if (selectedItem->text()==q_("Media Manager"))
            sproxy = sgui->servermanagerDialog.getProxy();
        else if (selectedItem->text()==q_("Media Record Manager"))
            sproxy =sgui->recordmanagerDialog.getProxy();
        else if (selectedItem->text()==q_("Preview Window"))
        {
            QWidget* w = StelMainWindow::getInstance().getPreviewWidget();
            w->setGeometry(globalPos.x(),globalPos.y(),w->width(),w->height());
        }
        else if (selectedItem->text()==q_("Configration Window"))
            sproxy =sgui->configurationDialog->getProxy();
        else if (selectedItem->text()==q_("View Options Window"))
            sproxy = sgui->viewDialog.getProxy();
        else if (selectedItem->text()==q_("Search Window"))
            sproxy= sgui->searchDialog.getProxy();
        else if (selectedItem->text()==q_("Date/Time Window"))
            sproxy = sgui->dateTimeDialog.getProxy();
        else if (selectedItem->text()==q_("Location Window"))
            sproxy = sgui->locationDialog.getProxy();
        else if (selectedItem->text()==q_("Script Console"))
            sproxy = sgui->scriptConsole.getProxy();
        else if (selectedItem->text()==q_("Help Window"))
            sproxy = sgui->helpDialog.getProxy();

        else if (selectedItem->text()==q_("Projector Settings"))
        {
            StelMainWindow::getInstance().projdll->ShowProjSettingsDialog();
        }

        else if (selectedItem->text()==q_("Exit"))
        {
            QMessageBox msgBox;
            msgBox.setWindowTitle(q_("Warning!"));
            msgBox.setText(q_("Are you sure want to quit?"));
            msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
            msgBox.setDefaultButton(QMessageBox::Save);
            msgBox.setWindowModality(Qt::WindowModality());
            msgBox.setIcon(QMessageBox::Warning);
            int ret = msgBox.exec();

            if(ret ==QMessageBox::Yes)
                StelApp::getInstance().getGui()->quitAll();
            return;
        }

        if(sproxy!=NULL)
        {
            sproxy->setPos(globalPos);
            sproxy->setVisible(true);

        }
    }
    else
    {
        // nothing was chosen
    }
}

ShiraPlayerForm::~ShiraPlayerForm()
{
}

void ShiraPlayerForm::resizeEvent(QResizeEvent* event)
{
    scene()->setSceneRect(QRect(QPoint(0, 0), event->size()));
    QGraphicsView::resizeEvent(event);
}
