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
#include <QPushButton>
#include <QMessageBox>

#include "planetszoomdialog.h"
#include "ui_planetszoomdialog.h"
#include "StelTranslator.hpp"
#include "StelApp.hpp"

PlanetsZoomDialog::PlanetsZoomDialog()
{
    ui = new Ui_PlanetsZoomDialog;

}

PlanetsZoomDialog::~PlanetsZoomDialog()
{
    delete ui;
}

void PlanetsZoomDialog::languageChanged()
{
    if (dialog)
    {
        ui->btnPluto->setText(q_("Pluto"));
        ui->btnUranus->setText(q_("Uranus"));
        ui->btnNeptune->setText(q_("Neptune"));
        ui->btnMars->setText(q_("Mars"));
        ui->btnSaturn->setText(q_("Saturn"));
        ui->btnJupiter->setText(q_("Jupiter"));
        ui->btnVenus->setText(q_("Venus"));
        ui->btnMercury->setText(q_("Mercury"));
        ui->btnMoon->setText(q_("Moon"));
    }
}

void PlanetsZoomDialog::createDialogContent()
{
    ui->setupUi(dialog);
    connect(ui->btnJupiter,SIGNAL(clicked()),this,SLOT(buttonClick()));
    connect(ui->btnMars,SIGNAL(clicked()),this,SLOT(buttonClick()));
    connect(ui->btnMercury,SIGNAL(clicked()),this,SLOT(buttonClick()));
    connect(ui->btnMoon,SIGNAL(clicked()),this,SLOT(buttonClick()));
    connect(ui->btnNeptune,SIGNAL(clicked()),this,SLOT(buttonClick()));
    connect(ui->btnPluto,SIGNAL(clicked()),this,SLOT(buttonClick()));
    connect(ui->btnSaturn,SIGNAL(clicked()),this,SLOT(buttonClick()));
    connect(ui->btnUranus,SIGNAL(clicked()),this,SLOT(buttonClick()));
    connect(ui->btnVenus,SIGNAL(clicked()),this,SLOT(buttonClick()));
    ui->btnMoon->setHidden(true);
}

void PlanetsZoomDialog::prepareAsChild()
{}

void PlanetsZoomDialog::retranslate()
{
    languageChanged();
}

void PlanetsZoomDialog::buttonClick()
{
    QToolButton* button = qobject_cast<QToolButton*>(sender());
    //QMessageBox::about(0,"",button->objectName());
    StelApp::getInstance().startPlanetZoom(button->objectName(),button->isChecked());
}
