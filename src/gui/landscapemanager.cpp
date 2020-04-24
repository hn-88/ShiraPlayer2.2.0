/*
 * ShiraPlayer(TM)
 * Copyright (C) 2015 Asaf Yurdakul
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
#include <QListWidgetItem>

#include "landscapemanager.h"
#include "ui_landscapemanager.h"
#include "StelTranslator.hpp"
#include "LandscapeMgr.hpp"
#include "Landscape.hpp"
#include "StelModuleMgr.hpp"
#include "StelFileMgr.hpp"

landscapeManager::landscapeManager()
{
    ui = new Ui_landscapeManager;
}

landscapeManager::~landscapeManager()
{
    delete ui;
}

void landscapeManager::languageChanged()
{
    if (dialog)
    {
        ui->imgScreenShot->setText(q_("Screenshot"));
        LandscapeMgr* lmgr = GETSTELMODULE(LandscapeMgr);
        if (StelFileMgr::exists(StelFileMgr::getInstallationDir()+"/landscapes/"+lmgr->getCurrentLandscape()->getName()+"/screenshot.jpg"))
            ui->imgScreenShot->setPixmap(QPixmap(StelFileMgr::getInstallationDir()+"/landscapes/"+lmgr->getCurrentLandscape()->getName()+"/screenshot.jpg", 0, Qt::AutoColor));
        else
            ui->imgScreenShot->setPixmap(QPixmap());

        ui->label->setText(q_("Sample Screenshot"));

    }
}

void landscapeManager::createDialogContent()
{
    ui->setupUi(dialog);
    // Fill the landscape list
    QListWidget* l = ui->landscapesListWidget;
    l->blockSignals(true);
    l->clear();
    LandscapeMgr* lmgr = GETSTELMODULE(LandscapeMgr);
    l->addItems(lmgr->getAllLandscapeNames(false));
    l->setCurrentItem(l->findItems(lmgr->getCurrentLandscapeName(), Qt::MatchExactly).at(0));
    l->blockSignals(false);
    //ui->landscapeTextBrowser->setHtml(lmgr->getCurrentLandscapeHtmlDescription());

    connect(ui->landscapesListWidget, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(landscapeChanged(QListWidgetItem*)));

}

void landscapeManager::prepareAsChild()
{}

void landscapeManager::landscapeChanged(QListWidgetItem *item)
{
    LandscapeMgr* lmgr = GETSTELMODULE(LandscapeMgr);

    lmgr->doSetCurrentLandscapeName(item->text());
    if (StelFileMgr::exists(StelFileMgr::getInstallationDir()+"/landscapes/"+lmgr->getCurrentLandscape()->getName()+"/screenshot.jpg"))
        ui->imgScreenShot->setPixmap(QPixmap(StelFileMgr::getInstallationDir()+"/landscapes/"+lmgr->getCurrentLandscape()->getName()+"/screenshot.jpg", 0, Qt::AutoColor));
    else
        ui->imgScreenShot->setPixmap(QPixmap());
    //ui->landscapeTextBrowser->document()->setDefaultStyleSheet(QString(StelApp::getInstance().getCurrentStelStyle()->htmlStyleSheet));
    //ui->landscapeTextBrowser->setHtml(lmgr->getCurrentLandscapeHtmlDescription());
    //ui->useAsDefaultLandscapeCheckBox->setChecked(lmgr->getDefaultLandscapeID()==lmgr->getCurrentLandscapeID());
    //ui->useAsDefaultLandscapeCheckBox->setEnabled(lmgr->getDefaultLandscapeID()!=lmgr->getCurrentLandscapeID());

    //qDebug()<<StelFileMgr::getInstallationDir()+"/landscapes/"+lmgr->getCurrentLandscape()->getName()+"/screenshot.jpg";

    StelApp::getInstance().addNetworkCommand("name = LandscapeMgr.setCurrentLandscapeName('"+item->text()+"');");
}

void landscapeManager::retranslate()
{
    languageChanged();
}
