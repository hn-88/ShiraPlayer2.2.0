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


#include "ViewDialog.hpp"
#include "ui_viewDialog.h"
#include "StelApp.hpp"
#include "StelCore.hpp"
#include "StelSkyCultureMgr.hpp"
#include "StelFileMgr.hpp"
#include "StelLocaleMgr.hpp"
#include "StelProjector.hpp"
#include "LandscapeMgr.hpp"
#include "StelModuleMgr.hpp"
#include "StarMgr.hpp"
#include "StelSkyDrawer.hpp"
#include "SolarSystem.hpp"
#include "NebulaMgr.hpp"
#include "MeteorMgr.hpp"
#include "GridLinesMgr.hpp"
#include "ConstellationMgr.hpp"
#include "StelStyle.hpp"
#include "StelSkyLayerMgr.hpp"
#include "StelGuiBase.hpp"

#include <QDebug>
#include <QFrame>
#include <QFile>
#include <QSettings>
#include <QTimer>
#include <QDialog>

//ASAF
#include "StelGui.hpp"
#include "StelMainWindow.hpp"
#include "StelMainGraphicsView.hpp"
#include "StelMovementMgr.hpp"
#include "StelNavigator.hpp"
#include "MilkyWay.hpp"
#include "warping/channel.h"

ViewDialog::ViewDialog()
{
    ui = new Ui_viewDialogForm;
}

ViewDialog::~ViewDialog()
{
    delete ui;
    ui=NULL;
}

void ViewDialog::languageChanged()
{
    if(dialog)
    {
        ui->stelWindowTitle->setText(q_("View"));
        QListWidgetItem *___qlistwidgetitem = ui->stackListWidget->item(0);
        ___qlistwidgetitem->setText(q_("Sky"));
        QListWidgetItem *___qlistwidgetitem1 = ui->stackListWidget->item(1);
        ___qlistwidgetitem1->setText(q_("Markings"));
        QListWidgetItem *___qlistwidgetitem2 = ui->stackListWidget->item(2);
        ___qlistwidgetitem2->setText(q_("Landscape"));
        QListWidgetItem *___qlistwidgetitem3 = ui->stackListWidget->item(3);
        ___qlistwidgetitem3->setText(q_("Starlore"));
        ui->starGroupBox->setTitle(q_("Stars"));
        ui->label_5->setText(q_("Absolute scale:"));
        ui->label_7->setText(q_("Relative scale:"));
        ui->starTwinkleCheckBox->setText(q_("Twinkle:"));

        ui->adaptationCheckbox->setToolTip(q_("Dim faint stars when a very bright object is visible"));

        ui->adaptationCheckbox->setText(q_("Dynamic eye adaptation"));
        ui->planetsGroupBox->setTitle(q_("Planets and satellites"));
        ui->showPlanetCheckBox->setText(q_("Show planets"));
        ui->planetMarkerCheckBox->setText(q_("Show planet markers"));
        ui->planetOrbitCheckBox->setText(q_("Show planet orbits"));
        ui->planetLightSpeedCheckBox->setText(q_("Simulate light speed"));
        ui->planetScaleMoonCheckBox->setText(q_("Scale Moon"));
        ui->planetScalePlanetCheckBox->setText(q_("Scale Planets-"));
        ui->Earth->setText(q_("Earth"));
        ui->label_10->setText(q_("Mercury"));
        ui->label_11->setText(q_("Venus"));
        ui->label_12->setText(q_("Mars"));
        ui->label_13->setText(q_("Jupiter"));
        ui->label_15->setText(q_("Saturn"));
        ui->label_16->setText(q_("Pluto"));
        ui->label_17->setText(q_("Neptun"));
        ui->label_18->setText(q_("Uranus"));
        ui->labelsGroupBox->setTitle(q_("Labels and Markers"));
        ui->starLabelCheckBox->setText(q_("Stars"));
        ui->nebulaLabelCheckBox->setText(q_("Nebulas"));
        ui->planetLabelCheckBox->setText(q_("Planets"));
        ui->shootingStarsGroupBox->setTitle(q_("Shooting Stars"));
        ui->label_14->setText(q_("Hourly zenith rate:"));
        ui->zhrNone->setText(q_("0"));
        ui->zhr10->setText(q_("10"));
        ui->zhr80->setText(q_("80"));
        ui->zhr10000->setText(q_("10000"));
        ui->zhr144000->setText(q_("144000"));
        ui->zhrLabel->setText(QString());
        ui->atmosphereGroupBox->setTitle(q_("Atmosphere"));
        ui->showAtmosphereCheckBox->setText(q_("Show atmosphere"));
        ui->label_4->setText(q_("Light pollution: "));
        ui->chMilkyway->setText(q_("Show milkyway"));
        ui->lblMikyway->setText(q_("Milkyway intensity"));
        ui->celestialSphereGroupBox->setTitle(q_("Celestial Sphere"));
        ui->showEquatorialGridCheckBox->setText(q_("Equatorial grid"));
        ui->showEquatorialJ2000GridCheckBox->setText(q_("Equatorial J2000 grid"));
        ui->showAzimuthalGridCheckBox->setText(q_("Azimuthal grid"));
        ui->showEquatorLineCheckBox->setText(q_("Equator line"));
        ui->showMeridianLineCheckBox->setText(q_("Meridian line"));
        ui->showEclipticLineCheckBox->setText(q_("Ecliptic line"));
        ui->showCardinalPointsCheckBox->setText(q_("Cardinal points"));
        ui->constellationGroupBox->setTitle(q_("Constellations"));
        ui->showConstellationLinesCheckBox->setText(q_("Show lines"));
        ui->showConstellationLabelsCheckBox->setText(q_("Show labels"));
        ui->showConstellationBoundariesCheckBox->setText(q_("Show boundaries"));
        ui->showConstellationArtCheckBox->setText(q_("Show art"));
        ui->label_6->setText(q_("Art brightness: "));
        ui->tuiGroupBox->setTitle(q_("Dome Show Informations"));
        ui->showDateTimeCheckBox->setText(q_("Date-Time"));
        ui->showLocationCheckBox->setText(q_("Location"));
        ui->showSelObjPropcheckBox->setText(q_("Selected Object Properties"));
        ui->setProjectionGroupBox->setTitle(q_("Projection"));
        ui->landscapeOptionsGroupBox->setTitle(q_("Options"));
        ui->showGroundCheckBox->setText(q_("Show ground"));
        ui->showFogCheckBox->setText(q_("Show fog"));
        ui->landscapePositionCheckBox->setText(q_("Use associated planet and position"));
        ui->useAsDefaultLandscapeCheckBox->setText(q_("Use this landscape as default"));
        ui->label_9->setText(q_("Video"));
        ui->lblPicture->setText(q_("Picture"));
        ui->starloreOptionsgroupBox->setTitle(q_("Options"));
        ui->useAsDefaultSkyCultureCheckBox->setText(q_("Use this sky culture as default"));
        ui->groupBox->setTitle(q_("Options"));
        ui->skyLayerEnableCheckBox->setText(q_("Visible"));
        ui->groupWarp->setTitle(q_("Warping Settings"));
        ui->groupBox_3->setTitle(QString());
        ui->btnsavewarp->setText(q_("Save Warp Settings"));
        ui->label->setText(q_("Mode"));
        ui->comboWarpMode->clear();
        ui->comboWarpMode->insertItems(0, QStringList()
         << q_("Show Mode")
         << q_("Distortion Warp Mode")
         << q_("Blend Warp Mode")
        );
        ui->btnResetWarp->setText(q_("Reset Warp"));
        ui->label_2->setText(q_("FOV:"));
        ui->label_20->setText(q_("Fov Width Rate(%0.1-1.0)"));
        ui->label_21->setText(q_("Fov Height Rate(%0.1-1.0)"));
        ui->checkConfigure->setText(q_("Configure Clients"));
        ui->groupChannel->setTitle(q_("Channel Settings"));
        ui->Chann->setText(q_("Channel as Hostname"));
        ui->bntAddChannel->setText(q_("Add"));
        ui->btnRemoveChannel->setText(q_("Remove"));
        ui->groupChannelProperties->setTitle(q_("Channel Properties"));
        ui->label_3->setText(q_("Width"));
        ui->label_8->setText(q_("Height"));
        ui->chShowChannelName->setText(QString());
        ui->label_19->setText(q_("Show Name"));
    }

    if (dialog)
    {
        //ui->retranslateUi(dialog);
        shootingStarsZHRChanged();
        populateLists();
    }
}

void ViewDialog::styleChanged()
{
    if (dialog)
    {
        populateLists();
    }
}

void ViewDialog::createDialogContent()
{
    ui->setupUi(dialog);

    // Set the Sky tab activated by default
    ui->stackedWidget->setCurrentIndex(0);
    ui->stackListWidget->setCurrentRow(0);

    //ui->viewTabWidget->removeTab(4);


    //ASAF
    ui->stackedWidget->removeWidget(ui->page_5);
    ui->stackListWidget->item(5)->setHidden(true);
    ui->stackListWidget->item(4)->setText("Warping");
    //

    connect(ui->closeStelWindow, SIGNAL(clicked()), this, SLOT(close()));

    populateLists();
    connect(ui->culturesListWidget, SIGNAL(currentTextChanged(const QString&)), this, SLOT(skyCultureChanged(const QString&)));
    connect(ui->projectionListWidget, SIGNAL(currentTextChanged(const QString&)), this, SLOT(projectionChanged(const QString&)));
    connect(ui->landscapesListWidget, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(landscapeChanged(QListWidgetItem*)));

    // Connect and initialize checkboxes and other widgets
    StelGuiBase* gui = StelApp::getInstance().getGui();
    // Stars section
    QAction* a;

    ui->starTwinkleCheckBox->setChecked(StelApp::getInstance().getCore()->getSkyDrawer()->getFlagTwinkle());
    connect(ui->starTwinkleCheckBox, SIGNAL(toggled(bool)), StelApp::getInstance().getCore()->getSkyDrawer(), SLOT(setFlagTwinkle(bool)));

    ui->starScaleRadiusDoubleSpinBox->setValue(StelApp::getInstance().getCore()->getSkyDrawer()->getAbsoluteStarScale());
    connect(ui->starScaleRadiusDoubleSpinBox, SIGNAL(valueChanged(double)), StelApp::getInstance().getCore()->getSkyDrawer(), SLOT(setAbsoluteStarScale(double)));

    ui->starRelativeScaleDoubleSpinBox->setValue(StelApp::getInstance().getCore()->getSkyDrawer()->getRelativeStarScale());
    connect(ui->starRelativeScaleDoubleSpinBox, SIGNAL(valueChanged(double)), StelApp::getInstance().getCore()->getSkyDrawer(), SLOT(setRelativeStarScale(double)));

    ui->starTwinkleAmountDoubleSpinBox->setValue(StelApp::getInstance().getCore()->getSkyDrawer()->getTwinkleAmount());
    connect(ui->starTwinkleAmountDoubleSpinBox, SIGNAL(valueChanged(double)), StelApp::getInstance().getCore()->getSkyDrawer(), SLOT(setTwinkleAmount(double)));

    ui->adaptationCheckbox->setChecked(StelApp::getInstance().getCore()->getSkyDrawer()->getFlagLuminanceAdaptation());
    connect(ui->adaptationCheckbox, SIGNAL(toggled(bool)), StelApp::getInstance().getCore()->getSkyDrawer(), SLOT(setFlagLuminanceAdaptation(bool)));

    // Planets section
    SolarSystem* ssmgr = GETSTELMODULE(SolarSystem);

    ui->showPlanetCheckBox->setChecked(ssmgr->getFlagPlanets());
    a = gui->getGuiActions("actionShow_Planets");
    connect(a, SIGNAL(toggled(bool)), ui->showPlanetCheckBox, SLOT(setChecked(bool)));
    connect(ui->showPlanetCheckBox,  SIGNAL(toggled(bool)), a, SLOT(setChecked(bool)));

    ui->planetMarkerCheckBox->setChecked(ssmgr->getFlagHints());
    connect(ui->planetMarkerCheckBox, SIGNAL(toggled(bool)), ssmgr, SLOT(setFlagHints(bool)));

    ui->planetScaleMoonCheckBox->setChecked(ssmgr->getFlagMoonScale());
    connect(ui->planetScaleMoonCheckBox, SIGNAL(toggled(bool)), ssmgr, SLOT(setFlagMoonScale(bool)));

    ui->planetOrbitCheckBox->setChecked(ssmgr->getFlagOrbits());
    connect(ui->planetOrbitCheckBox, SIGNAL(toggled(bool)), ssmgr, SLOT(setFlagOrbits(bool)));

    ui->planetLightSpeedCheckBox->setChecked(ssmgr->getFlagLightTravelTime());
    connect(ui->planetLightSpeedCheckBox, SIGNAL(toggled(bool)), ssmgr, SLOT(setFlagLightTravelTime(bool)));

    // Shooting stars section
    MeteorMgr* mmgr = GETSTELMODULE(MeteorMgr);
    Q_ASSERT(mmgr);
    switch(mmgr->getZHR())
    {
    case 0: ui->zhrNone->setChecked(true); break;
    case 80: ui->zhr80->setChecked(true); break;
    case 10000: ui->zhr10000->setChecked(true); break;
    case 144000: ui->zhr144000->setChecked(true); break;
    default: ui->zhr10->setChecked(true); break;
    }
    shootingStarsZHRChanged();
    connect(ui->zhrNone, SIGNAL(clicked()), this, SLOT(shootingStarsZHRChanged()));
    connect(ui->zhr10, SIGNAL(clicked()), this, SLOT(shootingStarsZHRChanged()));
    connect(ui->zhr80, SIGNAL(clicked()), this, SLOT(shootingStarsZHRChanged()));
    connect(ui->zhr10000, SIGNAL(clicked()), this, SLOT(shootingStarsZHRChanged()));
    connect(ui->zhr144000, SIGNAL(clicked()), this, SLOT(shootingStarsZHRChanged()));

    // Labels section    

    StarMgr* smgr = GETSTELMODULE(StarMgr);

    ui->starLabelCheckBox->setChecked(smgr->getFlagLabels());
    a = gui->getGuiActions("actionShow_Star_Labels");
    connect(a, SIGNAL(toggled(bool)), ui->starLabelCheckBox, SLOT(setChecked(bool)));
    connect(ui->starLabelCheckBox, SIGNAL(toggled(bool)), a, SLOT(setChecked(bool)));

    NebulaMgr* nmgr = GETSTELMODULE(NebulaMgr);
    ui->nebulaLabelCheckBox->setChecked(nmgr->getFlagHints());
    a = gui->getGuiActions("actionShow_Nebulas");
    connect(a, SIGNAL(toggled(bool)), ui->nebulaLabelCheckBox, SLOT(setChecked(bool)));
    connect(ui->nebulaLabelCheckBox, SIGNAL(toggled(bool)), a, SLOT(setChecked(bool)));

    ui->planetLabelCheckBox->setChecked(ssmgr->getFlagLabels());
    a = gui->getGuiActions("actionShow_Planets_Labels");
    connect(a, SIGNAL(toggled(bool)), ui->planetLabelCheckBox, SLOT(setChecked(bool)));
    connect(ui->planetLabelCheckBox, SIGNAL(toggled(bool)), a, SLOT(setChecked(bool)));

    ui->starsLabelsHorizontalSlider->setValue((int)(smgr->getLabelsAmount()*10.f));
    connect(ui->starsLabelsHorizontalSlider, SIGNAL(valueChanged(int)), this, SLOT(starsLabelsValueChanged(int)));
    ui->planetsLabelsHorizontalSlider->setValue((int)(ssmgr->getLabelsAmount()*10.f));
    connect(ui->planetsLabelsHorizontalSlider, SIGNAL(valueChanged(int)), this, SLOT(planetsLabelsValueChanged(int)));
    ui->nebulasLabelsHorizontalSlider->setValue((int)(nmgr->getHintsAmount()*10.f));
    connect(ui->nebulasLabelsHorizontalSlider, SIGNAL(valueChanged(int)), this, SLOT(nebulasLabelsValueChanged(int)));

    // Landscape section
    LandscapeMgr* lmgr = GETSTELMODULE(LandscapeMgr);
    ui->showGroundCheckBox->setChecked(lmgr->getFlagLandscape());
    a = gui->getGuiActions("actionShow_Ground");
    connect(a, SIGNAL(toggled(bool)), ui->showGroundCheckBox, SLOT(setChecked(bool)));
    connect(ui->showGroundCheckBox, SIGNAL(toggled(bool)), a, SLOT(setChecked(bool)));

    ui->showFogCheckBox->setChecked(lmgr->getFlagFog());
    connect(ui->showFogCheckBox, SIGNAL(toggled(bool)), lmgr, SLOT(setFlagFog(bool)));

    ui->showAtmosphereCheckBox->setChecked(lmgr->getFlagAtmosphere());
    a = gui->getGuiActions("actionShow_Atmosphere");
    connect(a, SIGNAL(toggled(bool)), ui->showAtmosphereCheckBox, SLOT(setChecked(bool)));
    connect(ui->showAtmosphereCheckBox, SIGNAL(toggled(bool)), a, SLOT(setChecked(bool)));

    ui->landscapePositionCheckBox->setChecked(lmgr->getFlagLandscapeSetsLocation());
    connect(ui->landscapePositionCheckBox, SIGNAL(toggled(bool)), lmgr, SLOT(setFlagLandscapeSetsLocation(bool)));

    ui->lightPollutionSpinBox->setValue(StelApp::getInstance().getCore()->getSkyDrawer()->getBortleScale());
    connect(ui->lightPollutionSpinBox, SIGNAL(valueChanged(int)), lmgr, SLOT(setAtmosphereBortleLightPollution(int)));
    connect(ui->lightPollutionSpinBox, SIGNAL(valueChanged(int)), StelApp::getInstance().getCore()->getSkyDrawer(), SLOT(setBortleScale(int)));

    ui->useAsDefaultLandscapeCheckBox->setChecked(lmgr->getCurrentLandscapeID()==lmgr->getDefaultLandscapeID());
    ui->useAsDefaultLandscapeCheckBox->setEnabled(lmgr->getCurrentLandscapeID()!=lmgr->getDefaultLandscapeID());
    connect(ui->useAsDefaultLandscapeCheckBox, SIGNAL(clicked()), this, SLOT(setCurrentLandscapeAsDefault()));

    // Grid and lines
    GridLinesMgr* glmgr = GETSTELMODULE(GridLinesMgr);
    ui->showEquatorLineCheckBox->setChecked(glmgr->getFlagEquatorLine());
    a = gui->getGuiActions("actionShow_Equator_Line");
    connect(a, SIGNAL(toggled(bool)), ui->showEquatorLineCheckBox, SLOT(setChecked(bool)));
    connect(ui->showEquatorLineCheckBox, SIGNAL(toggled(bool)), a, SLOT(setChecked(bool)));

    ui->showEclipticLineCheckBox->setChecked(glmgr->getFlagEclipticLine());
    a = gui->getGuiActions("actionShow_Ecliptic_Line");
    connect(a, SIGNAL(toggled(bool)), ui->showEclipticLineCheckBox, SLOT(setChecked(bool)));
    connect(ui->showEclipticLineCheckBox, SIGNAL(toggled(bool)), a, SLOT(setChecked(bool)));

    ui->showMeridianLineCheckBox->setChecked(glmgr->getFlagMeridianLine());
    a = gui->getGuiActions("actionShow_Meridian_Line");
    connect(a, SIGNAL(toggled(bool)), ui->showMeridianLineCheckBox, SLOT(setChecked(bool)));
    connect(ui->showMeridianLineCheckBox, SIGNAL(toggled(bool)), a, SLOT(setChecked(bool)));

    ui->showEquatorialGridCheckBox->setChecked(glmgr->getFlagEquatorGrid());
    a = gui->getGuiActions("actionShow_Equatorial_Grid");
    connect(a, SIGNAL(toggled(bool)), ui->showEquatorialGridCheckBox, SLOT(setChecked(bool)));
    connect(ui->showEquatorialGridCheckBox, SIGNAL(toggled(bool)), a, SLOT(setChecked(bool)));

    ui->showAzimuthalGridCheckBox->setChecked(glmgr->getFlagAzimuthalGrid());
    a = gui->getGuiActions("actionShow_Azimuthal_Grid");
    connect(a, SIGNAL(toggled(bool)), ui->showAzimuthalGridCheckBox, SLOT(setChecked(bool)));
    connect(ui->showAzimuthalGridCheckBox, SIGNAL(toggled(bool)), a, SLOT(setChecked(bool)));

    ui->showEquatorialJ2000GridCheckBox->setChecked(glmgr->getFlagEquatorJ2000Grid());
    a = gui->getGuiActions("actionShow_Equatorial_J2000_Grid");
    connect(a, SIGNAL(toggled(bool)), ui->showEquatorialJ2000GridCheckBox, SLOT(setChecked(bool)));
    connect(ui->showEquatorialJ2000GridCheckBox, SIGNAL(toggled(bool)), a, SLOT(setChecked(bool)));

    ui->showCardinalPointsCheckBox->setChecked(lmgr->getFlagCardinalsPoints());
    a = gui->getGuiActions("actionShow_Cardinal_Points");
    connect(a, SIGNAL(toggled(bool)), ui->showCardinalPointsCheckBox, SLOT(setChecked(bool)));
    connect(ui->showCardinalPointsCheckBox, SIGNAL(toggled(bool)), a, SLOT(setChecked(bool)));

    // Constellations
    ConstellationMgr* cmgr = GETSTELMODULE(ConstellationMgr);

    ui->showConstellationLinesCheckBox->setChecked(cmgr->getFlagLines());
    a = gui->getGuiActions("actionShow_Constellation_Lines");
    connect(a, SIGNAL(toggled(bool)), ui->showConstellationLinesCheckBox, SLOT(setChecked(bool)));
    connect(ui->showConstellationLinesCheckBox, SIGNAL(toggled(bool)), a, SLOT(setChecked(bool)));

    ui->showConstellationLabelsCheckBox->setChecked(cmgr->getFlagLabels());
    a = gui->getGuiActions("actionShow_Constellation_Labels");
    connect(a, SIGNAL(toggled(bool)), ui->showConstellationLabelsCheckBox, SLOT(setChecked(bool)));
    connect(ui->showConstellationLabelsCheckBox, SIGNAL(toggled(bool)), a, SLOT(setChecked(bool)));

    ui->showConstellationBoundariesCheckBox->setChecked(cmgr->getFlagBoundaries());
    a = gui->getGuiActions("actionShow_Constellation_Boundaries");
    connect(a, SIGNAL(toggled(bool)), ui->showConstellationBoundariesCheckBox, SLOT(setChecked(bool)));
    connect(ui->showConstellationBoundariesCheckBox, SIGNAL(toggled(bool)), a, SLOT(setChecked(bool)));

    ui->showConstellationArtCheckBox->setChecked(cmgr->getFlagArt());
    a = gui->getGuiActions("actionShow_Constellation_Art");
    connect(a, SIGNAL(toggled(bool)), ui->showConstellationArtCheckBox, SLOT(setChecked(bool)));
    connect(ui->showConstellationArtCheckBox, SIGNAL(toggled(bool)), a, SLOT(setChecked(bool)));

    ui->constellationArtBrightnessSpinBox->setValue(cmgr->getArtIntensity());
    connect(ui->constellationArtBrightnessSpinBox, SIGNAL(valueChanged(double)), cmgr, SLOT(setArtIntensity(double)));

    // Starlore
    connect(ui->useAsDefaultSkyCultureCheckBox, SIGNAL(clicked()), this, SLOT(setCurrentCultureAsDefault()));
    const bool b = StelApp::getInstance().getSkyCultureMgr().getCurrentSkyCultureID()==StelApp::getInstance().getSkyCultureMgr().getDefaultSkyCultureID();
    ui->useAsDefaultSkyCultureCheckBox->setChecked(b);
    ui->useAsDefaultSkyCultureCheckBox->setEnabled(!b);

    // Sky layers
    populateSkyLayersList();
    connect(this, SIGNAL(visibleChanged(bool)), this, SLOT(populateSkyLayersList()));
    connect(ui->skyLayerListWidget, SIGNAL(currentTextChanged(const QString&)), this, SLOT(skyLayersSelectionChanged(const QString&)));
    connect(ui->stackListWidget, SIGNAL(currentItemChanged(QListWidgetItem *, QListWidgetItem *)), this, SLOT(changePage(QListWidgetItem *, QListWidgetItem*)));
    connect(ui->skyLayerEnableCheckBox, SIGNAL(stateChanged(int)), this, SLOT(skyLayersEnabledChanged(int)));


    QTimer* refreshTimer = new QTimer(this);
    connect(refreshTimer, SIGNAL(timeout()), this, SLOT(updateFromProgram()));
    refreshTimer->start(200);

    //ASAF
    //Warping Settings
    connect(ui->comboWarpMode, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(warpModeChanged(const QString&)));
    connect(ui->btnsavewarp, SIGNAL(clicked()), this, SLOT(saveWarpSettings()));
    connect(ui->spboxFOV, SIGNAL(valueChanged(double)),this , SLOT(setcurrFOV(double)));
    connect(ui->spboxFWR, SIGNAL(valueChanged(double)),this , SLOT(on_change_FWR(double)));
    connect(ui->spboxFHR, SIGNAL(valueChanged(double)),this , SLOT(on_change_FHR(double)));

//#ifdef SHIRAPLAYER_PRE
    ui->stackListWidget->item(4)->setHidden(true);
    //ui->groupChannel->hide();
    //ui->groupChannelProperties->hide();
    //ui->channelSettings->hide();
    //ui->checkConfigure->setText("Configure");
    //ui->comboWarpMode->removeItem(2);
    //ui->fovSettings->hide();
    //ui->spboxFOV->setValue(StelApp::getInstance().getCore()->currentProjectorParams.fov);
//#else
    //Yeni distort - blend işlemi ile iptal edildi
    //ui->spboxFOV->setValue(StelApp::getInstance().getCore()->currentProjectorParams.fov);
//#endif

    connect(ui->bntAddChannel, SIGNAL(clicked()), this, SLOT(on_addButton_clicked()));
    connect(ui->btnRemoveChannel,SIGNAL(clicked()),this,SLOT(on_removeButton_clicked()));
    connect(ui->channelTable , SIGNAL(itemChanged(QTableWidgetItem*)), this, SLOT(on_channelTable_itemChanged(QTableWidgetItem*)));
    updateChannelGUI();

    ui->checkConfigure->setChecked(StelMainGraphicsView::getInstance().isClientConf);
    connect(ui->checkConfigure, SIGNAL(toggled(bool)), this, SLOT(on_checkConfig_checked(bool)));
    connect(ui->channelTable, SIGNAL(itemClicked(QTableWidgetItem*)), this, SLOT(on_channelList_select(QTableWidgetItem*)));

    connect(ui->btnResetWarp, SIGNAL(clicked()), this, SLOT(on_btnResetWarp_clicked()));

    connect(this,SIGNAL(visibleChanged(bool)),this, SLOT(on_visibleChanged(bool)));
    connect(this,SIGNAL(load(bool)),this,SLOT(on_load(bool)));

    connect(ui->spWidth, SIGNAL(valueChanged(int)),this , SLOT(on_value_Width(int)));
    connect(ui->spHeight, SIGNAL(valueChanged(int)),this , SLOT(on_value_Height(int)));

    ui->moonScaleSpinBox->setValue(ssmgr->getMoonScale());
    connect(ui->moonScaleSpinBox,SIGNAL(valueChanged(double)), ssmgr, SLOT(setMoonScale(double)));

    ui->planetScalePlanetCheckBox->setChecked(ssmgr->getFlagPlanetsScale());
    connect(ui->planetScalePlanetCheckBox, SIGNAL(toggled(bool)), ssmgr, SLOT(setFlagPlanetsScale(bool)));

    ui->mercurySpinBox->setValue(ssmgr->getMerkurScale());
    connect(ui->mercurySpinBox,SIGNAL(valueChanged(double)), ssmgr, SLOT(setMerkurScale(double)));

    ui->venusSpinBox->setValue(ssmgr->getVenusScale());
    connect(ui->venusSpinBox,SIGNAL(valueChanged(double)), ssmgr, SLOT(setVenusScale(double)));

    ui->marsSpinBox->setValue(ssmgr->getMarsScale());
    connect(ui->marsSpinBox,SIGNAL(valueChanged(double)), ssmgr, SLOT(setMarsScale(double)));

    ui->jupiterSpinBox->setValue(ssmgr->getJupiterScale());
    connect(ui->jupiterSpinBox,SIGNAL(valueChanged(double)), ssmgr, SLOT(setJupiterScale(double)));

    ui->saturnSpinBox->setValue(ssmgr->getSaturnScale());
    connect(ui->saturnSpinBox,SIGNAL(valueChanged(double)), ssmgr, SLOT(setSaturnScale(double)));

    ui->plutoSpinBox->setValue(ssmgr->getPlutoScale());
    connect(ui->plutoSpinBox,SIGNAL(valueChanged(double)), ssmgr, SLOT(setPlutoScale(double)));

    ui->neptunSpinBox->setValue(ssmgr->getNeptunScale());
    connect(ui->neptunSpinBox,SIGNAL(valueChanged(double)), ssmgr, SLOT(setNeptunScale(double)));

    ui->uranusSpinBox->setValue(ssmgr->getUranusScale());
    connect(ui->uranusSpinBox,SIGNAL(valueChanged(double)), ssmgr, SLOT(setUranusScale(double)));

    ui->earthSpinBox->setValue(ssmgr->getEarthScale());
    connect(ui->earthSpinBox,SIGNAL(valueChanged(double)), ssmgr, SLOT(setEarthScale(double)));

    connect(ui->chShowChannelName,SIGNAL(toggled(bool)),this,SLOT(on_setChNameShow(bool)));

    MilkyWay* milkyway = (MilkyWay*)GETSTELMODULE(MilkyWay);
    ui->chMilkyway->setChecked(milkyway->getFlagShow());
    connect(ui->chMilkyway,SIGNAL(toggled(bool)),milkyway,SLOT(setFlagShow(bool)));

    ui->spMilkyway->setValue(milkyway->getIntensity());
    connect(ui->spMilkyway,SIGNAL(valueChanged(double)),milkyway,SLOT(setIntensity(double)));

    ui->showDateTimeCheckBox->setChecked(StelApp::getInstance().getCore()->flagTuiDatetime);
    connect(ui->showDateTimeCheckBox,SIGNAL(toggled(bool)),this,SLOT(on_setDateTimeShow(bool)));
    ui->showLocationCheckBox->setChecked(StelApp::getInstance().getCore()->flagTuiLocation);
    connect(ui->showLocationCheckBox,SIGNAL(toggled(bool)),this,SLOT(on_setLocationShow(bool)));
    ui->showSelObjPropcheckBox->setChecked(StelApp::getInstance().showPropGui);
    connect(ui->showSelObjPropcheckBox,SIGNAL(toggled(bool)),this,SLOT(on_setPropGuiShow(bool)));

    //ui->planetsGroupBox->setHidden(true);
    ui->frame_10->setHidden(true);
    ui->frame_6->setHidden(true);

}
void ViewDialog::prepareAsChild()
{

}
void ViewDialog::populateLists()
{
    // Fill the culture list widget from the available list
    QListWidget* l = ui->culturesListWidget;
    l->blockSignals(true);
    l->clear();
    l->addItems(StelApp::getInstance().getSkyCultureMgr().getSkyCultureListI18());
    l->setCurrentItem(l->findItems(StelApp::getInstance().getSkyCultureMgr().getCurrentSkyCultureNameI18(), Qt::MatchExactly).at(0));
    l->blockSignals(false);
    updateSkyCultureText();

    const StelCore* core = StelApp::getInstance().getCore();

    // Fill the projection list
    l = ui->projectionListWidget;
    l->blockSignals(true);
    l->clear();
    const QStringList mappings = core->getAllProjectionTypeKeys();
    foreach (QString s, mappings)
    {
        l->addItem(core->projectionTypeKeyToNameI18n(s));
    }
    l->setCurrentItem(l->findItems(core->projectionTypeKeyToNameI18n(core->getCurrentProjectionTypeKey()), Qt::MatchExactly).at(0));
    l->blockSignals(false);
    ui->projectionTextBrowser->setHtml(core->getProjection(Mat4d())->getHtmlSummary());

    // Fill the landscape list
    l = ui->landscapesListWidget;
    l->blockSignals(true);
    l->clear();
    LandscapeMgr* lmgr = GETSTELMODULE(LandscapeMgr);
    l->addItems(lmgr->getAllLandscapeNames(false));
    l->setCurrentItem(l->findItems(lmgr->getCurrentLandscapeName(), Qt::MatchExactly).at(0));
    l->blockSignals(false);
    ui->landscapeTextBrowser->setHtml(lmgr->getCurrentLandscapeHtmlDescription());

    //ASAF
    // Fill the video landscape list
    l = ui->landscapesVideoListWidget;
    l->blockSignals(true);
    l->clear();
    //LandscapeMgr* lmgr = GETSTELMODULE(LandscapeMgr);
    l->addItems(lmgr->getAllLandscapeNames(true));
    //l->setCurrentItem(l->findItems(lmgr->getCurrentLandscapeName(), Qt::MatchExactly).at(0));
    l->blockSignals(false);
    //ui->landscapeTextBrowser->setHtml(lmgr->getCurrentLandscapeHtmlDescription());

    connect(ui->landscapesVideoListWidget, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(landscapeVideoChanged(QListWidgetItem*)));
}

void ViewDialog::populateSkyLayersList()
{
    ui->skyLayerListWidget->clear();
    StelSkyLayerMgr* skyLayerMgr = GETSTELMODULE(StelSkyLayerMgr);
    ui->skyLayerListWidget->addItems(skyLayerMgr->getAllKeys());
}

void ViewDialog::skyLayersSelectionChanged(const QString& s)
{
    StelSkyLayerMgr* skyLayerMgr = GETSTELMODULE(StelSkyLayerMgr);
    StelSkyLayerP l = skyLayerMgr->getSkyLayer(s);

    if (l.isNull())
        return;

    QString html = "<html><head></head><body>";
    html += "<h2>" + l->getShortName()+ "</h2>";
    html += "<p>" + l->getLayerDescriptionHtml() + "</p>";
    if (!l->getShortServerCredits().isEmpty())
        html += "<h3>" + q_("Contact") + ": " + l->getShortServerCredits() + "</h3>";
    html += "</body></html>";
    ui->skyLayerTextBrowser->setHtml(html);
    ui->skyLayerEnableCheckBox->setChecked(skyLayerMgr->getShowLayer(s));
}

void ViewDialog::skyLayersEnabledChanged(int state)
{
    StelSkyLayerMgr* skyLayerMgr = GETSTELMODULE(StelSkyLayerMgr);
    skyLayerMgr->showLayer(ui->skyLayerListWidget->currentItem()->text(), state);
}

void ViewDialog::skyCultureChanged(const QString& cultureName)
{
    StelApp::getInstance().getSkyCultureMgr().setCurrentSkyCultureNameI18(cultureName);
    updateSkyCultureText();
    const bool b = StelApp::getInstance().getSkyCultureMgr().getCurrentSkyCultureID()==StelApp::getInstance().getSkyCultureMgr().getDefaultSkyCultureID();
    ui->useAsDefaultSkyCultureCheckBox->setChecked(b);
    ui->useAsDefaultSkyCultureCheckBox->setEnabled(!b);
    //QMessageBox::information(0,0,StelApp::getInstance().getSkyCultureMgr().getCurrentSkyCultureID(),0,0);
    StelApp::getInstance().addNetworkCommand("core.setSkyCulture('"+StelApp::getInstance().getSkyCultureMgr().getCurrentSkyCultureID()+"');");
}

void ViewDialog::updateSkyCultureText()
{
    QString descPath;
    try
    {
        descPath = StelFileMgr::findFile("skycultures/" + StelApp::getInstance().getSkyCultureMgr().getCurrentSkyCultureID() + "/description."+StelApp::getInstance().getLocaleMgr().getAppLanguage()+".utf8");
    }
    catch (std::runtime_error& e)
    {
        try
        {
            descPath = StelFileMgr::findFile("skycultures/" + StelApp::getInstance().getSkyCultureMgr().getCurrentSkyCultureID() + "/description.en.utf8");
        }
        catch (std::runtime_error& e)
        {
            qWarning() << "WARNING: can't find description for skyculture" << StelApp::getInstance().getSkyCultureMgr().getCurrentSkyCultureID();
        }
    }

    QStringList searchPaths;
    try
    {
        searchPaths << StelFileMgr::findFile("skycultures/" + StelApp::getInstance().getSkyCultureMgr().getCurrentSkyCultureID());
    }
    catch (std::runtime_error& e) {}

    ui->skyCultureTextBrowser->setSearchPaths(searchPaths);
    ui->skyCultureTextBrowser->document()->setDefaultStyleSheet(QString(StelApp::getInstance().getCurrentStelStyle()->htmlStyleSheet));

    if (descPath.isEmpty())
    {
        ui->skyCultureTextBrowser->setHtml(q_("No description"));
    }
    else
    {
        QFile f(descPath);
        f.open(QIODevice::ReadOnly);
        ui->skyCultureTextBrowser->setHtml(QString::fromUtf8(f.readAll()));
    }
}

void ViewDialog::retranslate()
{
    languageChanged();
}

void ViewDialog::projectionChanged(const QString& projectionNameI18n)
{
    StelCore* core = StelApp::getInstance().getCore();
    core->setCurrentProjectionTypeKey(core->projectionNameI18nToTypeKey(projectionNameI18n));
    ui->projectionTextBrowser->document()->setDefaultStyleSheet(QString(StelApp::getInstance().getCurrentStelStyle()->htmlStyleSheet));
    ui->projectionTextBrowser->setHtml(core->getProjection(Mat4d())->getHtmlSummary());
}

void ViewDialog::landscapeChanged(QListWidgetItem* item)
{
    LandscapeMgr* lmgr = GETSTELMODULE(LandscapeMgr);

    //lmgr->setCurrentLandscapeName(item->text());
    lmgr->doSetCurrentLandscapeName(item->text());
    ui->landscapeTextBrowser->document()->setDefaultStyleSheet(QString(StelApp::getInstance().getCurrentStelStyle()->htmlStyleSheet));
    ui->landscapeTextBrowser->setHtml(lmgr->getCurrentLandscapeHtmlDescription());
    ui->useAsDefaultLandscapeCheckBox->setChecked(lmgr->getDefaultLandscapeID()==lmgr->getCurrentLandscapeID());
    ui->useAsDefaultLandscapeCheckBox->setEnabled(lmgr->getDefaultLandscapeID()!=lmgr->getCurrentLandscapeID());

    //ASAF
    ui->landscapesVideoListWidget->setCurrentRow(-1);
    StelApp::getInstance().addNetworkCommand("name = LandscapeMgr.setCurrentLandscapeName('"+item->text()+"');");

}

void ViewDialog::shootingStarsZHRChanged()
{
    MeteorMgr* mmgr = GETSTELMODULE(MeteorMgr);
    int zhr=-1;
    if (ui->zhrNone->isChecked())
    {
        mmgr->setFlagShow(false);
        zhr = 0;
    }
    else
    {
        mmgr->setFlagShow(true);
    }
    if (ui->zhr10->isChecked())
        zhr = 10;
    if (ui->zhr80->isChecked())
        zhr = 80;
    if (ui->zhr10000->isChecked())
        zhr = 10000;
    if (ui->zhr144000->isChecked())
        zhr = 144000;
    if (zhr!=mmgr->getZHR())
    {
        mmgr->setZHR(zhr);
    }
    switch (zhr)
    {
    case 0:
        ui->zhrLabel->setText("<small><i>"+q_("No shooting stars")+"</i></small>");
        break;
    case 10:
        ui->zhrLabel->setText("<small><i>"+q_("Normal rate")+"</i></small>");
        break;
    case 80:
        ui->zhrLabel->setText("<small><i>"+q_("Standard Perseids rate")+"</i></small>");
        break;
    case 10000:
        ui->zhrLabel->setText("<small><i>"+q_("Exceptional Leonid rate")+"</i></small>");
        break;
    case 144000:
        ui->zhrLabel->setText("<small><i>"+q_("Highest rate ever (1966 Leonids)")+"</i></small>");
        break;
    default:
        ui->zhrLabel->setText(QString("<small><i>")+"Error"+"</i></small>");
    }

    StelApp::getInstance().addNetworkCommand("name = MeteorMgr.setZHR('"+QString("%0").arg(zhr)+"');");

}

void ViewDialog::starsLabelsValueChanged(int v)
{
    StarMgr* smgr = GETSTELMODULE(StarMgr);
    float a= ((float)v)/10.f;
    smgr->setLabelsAmount(a);
}

void ViewDialog::setCurrentLandscapeAsDefault(void)
{
    LandscapeMgr* lmgr = GETSTELMODULE(LandscapeMgr);
    Q_ASSERT(lmgr);
    lmgr->setDefaultLandscapeID(lmgr->getCurrentLandscapeID());
    ui->useAsDefaultLandscapeCheckBox->setChecked(true);
    ui->useAsDefaultLandscapeCheckBox->setEnabled(false);

}

void ViewDialog::setCurrentCultureAsDefault(void)
{
    StelApp::getInstance().getSkyCultureMgr().setDefaultSkyCultureID(StelApp::getInstance().getSkyCultureMgr().getCurrentSkyCultureID());
    ui->useAsDefaultSkyCultureCheckBox->setChecked(true);
    ui->useAsDefaultSkyCultureCheckBox->setEnabled(false);
}

void ViewDialog::planetsLabelsValueChanged(int v)
{
    SolarSystem* ssmgr = GETSTELMODULE(SolarSystem);
    float a= ((float)v)/10.f;
    ssmgr->setLabelsAmount(a);
}

void ViewDialog::nebulasLabelsValueChanged(int v)
{
    NebulaMgr* nmgr = GETSTELMODULE(NebulaMgr);
    float a= ((float)v)/10.f;
    nmgr->setHintsAmount(a);
    nmgr->setLabelsAmount(a);
    QString script = "NebulaMgr.setHintsAmount("+QString("%0").arg(a)+");"+
                     "NebulaMgr.setLabelsAmount("+QString("%0").arg(a)+");";
    StelApp::getInstance().addNetworkCommand(script);
}

// Update the widget to make sure it is synchrone if a value was changed programmatically
void ViewDialog::updateFromProgram()
{
    if (!dialog->isVisible())
        return;

    // Check that the useAsDefaultSkyCultureCheckBox needs to be updated
    bool b = StelApp::getInstance().getSkyCultureMgr().getCurrentSkyCultureID()==StelApp::getInstance().getSkyCultureMgr().getDefaultSkyCultureID();
    if (b!=ui->useAsDefaultSkyCultureCheckBox->isChecked())
    {
        ui->useAsDefaultSkyCultureCheckBox->setChecked(b);
        ui->useAsDefaultSkyCultureCheckBox->setEnabled(!b);
    }

    LandscapeMgr* lmgr = GETSTELMODULE(LandscapeMgr);
    Q_ASSERT(lmgr);
    b = lmgr->getCurrentLandscapeID()==lmgr->getDefaultLandscapeID();
    if (b!=ui->useAsDefaultLandscapeCheckBox->isChecked())
    {
        ui->useAsDefaultLandscapeCheckBox->setChecked(b);
        ui->useAsDefaultLandscapeCheckBox->setEnabled(!b);
    }
}

void ViewDialog::changePage(QListWidgetItem *current, QListWidgetItem *previous)
{
    if (!current)
        current = previous;
    ui->stackedWidget->setCurrentIndex(ui->stackListWidget->row(current));
}


//ASAF
void ViewDialog::warpModeChanged(const QString& smode)
{
  //  Yeni Warping işlemiyle iptal edildi
  //  StelMainGraphicsView::getInstance().setwarpMode(smode);
  //  StelApp::getInstance().getRsync()->sendChanges(RSYNC_COMMAND_WARP_MODE,smode);
}

void ViewDialog::saveWarpSettings()
{
    //StelMainGraphicsView::getInstance().saveWarpSettings("warpsettings.xml");

    if(QMessageBox::information(0,q_("Warning!"),q_("Are you sure want to save All Channel settings?"),QMessageBox::Yes,QMessageBox::No)== QMessageBox::Yes)
    {
        QFile file("warpsettings.xml");
        if (file.open(QIODevice::WriteOnly|QIODevice::Truncate))
        {
            QTextStream out(&file);
            QDomDocument doc;

            QDomElement main = doc.createElement("ProjectorDistortion");
            main.setAttribute("version", 1.0);
            doc.appendChild(main);

            for (unsigned int i=0; i < StelMainGraphicsView::getInstance().m_pChannels.size(); ++i)
            {
                main.appendChild( StelMainGraphicsView::getInstance().m_pChannels[i]->domElement("Channel",doc,true));
            }

            doc.save(out, 4);
            file.flush();
            file.close();
        }
    }
}

void ViewDialog::setcurrFOV(double fov)
{
    StelApp::getInstance().getCore()->getMovementMgr()->zoomTo(fov, 0.2);
    StelApp::getInstance().getCore()->getMovementMgr()->setInitFov(fov);

    if (StelMainWindow::getInstance().getIsServer() && StelMainGraphicsView::getInstance().isClientConf)
    {
        if(StelMainGraphicsView::getInstance().m_pSelectedChannel)
        {
            StelMainGraphicsView::getInstance().m_pSelectedChannel->setinitFov(fov);
            StelMainGraphicsView::getInstance().m_pSelectedChannel->setinitPos(StelApp::getInstance().getCore()->getNavigator()->j2000ToAltAz(StelApp::getInstance().getCore()->getMovementMgr()->getViewDirectionJ2000()));

        }
        StelMainGraphicsView::getInstance().m_pSelectedChannel->updateClientData();

    }

}

void ViewDialog::on_addButton_clicked()
{
    Channel* pChannel = new Channel();

    pChannel->setinitFov(StelApp::getInstance().getCore()->getMovementMgr()->getInitFov());
    pChannel->setinitPos(StelApp::getInstance().getCore()->getMovementMgr()->getInitViewingDirection());
    //GETSTELMODULE(StelMovementMgr)->getInitViewingDirection();

    if (StelMainGraphicsView::getInstance().m_pSelectedChannel)
        pChannel = StelMainGraphicsView::getInstance().m_pSelectedChannel;
    StelMainGraphicsView::getInstance().m_pChannels.push_back(pChannel);

    StelMainGraphicsView::getInstance().m_pSelectedChannel = pChannel;

    ui->channelTable->insertRow(ui->channelTable->rowCount());
    ui->channelTable->setItem(ui->channelTable->rowCount(),0,new QTableWidgetItem(pChannel->getName()));

    ui->channelTable->resizeRowsToContents();
    ui->channelTable->setCurrentCell(ui->channelTable->rowCount()-1,0);
    updateChannelGUI();


}
void ViewDialog::on_removeButton_clicked()
{
    if(QMessageBox::information(0,q_("Warning!"),q_("Are you sure want to remove selected Channel?"),QMessageBox::Yes,QMessageBox::No)== QMessageBox::Yes)
    {
        unsigned int currentRow = ui->channelTable->currentRow();
        ui->channelTable->removeRow(currentRow);
        StelMainGraphicsView::getInstance().removeChannel(currentRow);

        updateChannelGUI();
    }
}

void ViewDialog::updateChannelGUI()
{
    unsigned int currentRow = ui->channelTable->currentRow();
    ui->channelTable->clear();
    ui->channelTable->setColumnCount(1);
    QStringList labels;
    labels << "        Name        "; // << "P" << "V" << "S";  // Name's Streach flag is ignored in Qt 4.1
    ui->channelTable->setHorizontalHeaderLabels(labels);
    //ui->channelTable->horizontalHeader()->setResizeMode(0, QHeaderView::Stretch);
    ui->channelTable->setShowGrid(false);
    for (int j=1; j<1; ++j) {
      //  ui->channelTable->horizontalHeader()->setResizeMode(j, QHeaderView::Custom);
    }
    ui->channelTable->resizeColumnsToContents();

    ui->channelTable->setRowCount( StelMainGraphicsView::getInstance().m_pChannels.size());

    for (unsigned int i=0; i< StelMainGraphicsView::getInstance().m_pChannels.size(); ++i)
    {
        ui->channelTable->setItem(i, 0, new QTableWidgetItem(StelMainGraphicsView::getInstance().m_pChannels[i]->getName()));
    }

    ui->channelTable->resizeRowsToContents();
    ui->channelTable->setCurrentCell(currentRow, 0);

}

void ViewDialog::updateChannelNamesGUI()
{
    for (unsigned int i = 0; i <  StelMainGraphicsView::getInstance().m_pChannels.size(); ++i)
        ui->channelTable->item(i, 0)->setText(StelMainGraphicsView::getInstance().m_pChannels[i]->getName());
}

void ViewDialog::on_channelTable_itemChanged(QTableWidgetItem* pItem)
{
    if (ui->channelTable->column(pItem) == 0)
    {
        if ( StelMainGraphicsView::getInstance().m_pChannels[ui->channelTable->row(pItem)]->getName() != pItem->text())
        {
            StelMainGraphicsView::getInstance().m_pChannels[ui->channelTable->row(pItem)]->setName(pItem->text());
        }
    }
}

void ViewDialog::on_checkConfig_checked(bool b)
{

    StelMainGraphicsView::getInstance().setClientConf(b);
    ui->groupChannel->setEnabled(b);
    ui->groupWarp->setEnabled(b);
    ui->groupChannelProperties->setEnabled(b);
    StelGui* gui = dynamic_cast<StelGui*>(StelApp::getInstance().getGui());
    gui->servermanagerDialog.setVisible(!b);
    if(b)
    {
        ui->channelTable->clear();
        ui->channelTable->clearContents();
        updateChannelGUI();

        StelApp::getInstance().getCore()->setMaskType(StelProjector::MaskNone);
    }
    else
    {
        StelApp::getInstance().getCore()->setMaskType(StelProjector::MaskDisk);
        QSettings* conf = StelApp::getInstance().getSettings();
        //StelMainWindow::getInstance().setGeometry(0,0,
        //                                          conf->value("video/screen_w", 800).toInt(),
        //                                         conf->value("video/screen_h", 600).toInt());
        StelApp::getInstance().getCore()->initWarpGL();
        gui->servermanagerDialog.getProxy()->setGeometry(QRect(0,0,gui->servermanagerDialog.getProxy()->geometry().width(),StelMainWindow::getInstance().height()));
        gui->servermanagerDialog.getProxy()->setPos(StelMainGraphicsView::getInstance().size().width()-gui->servermanagerDialog.getProxy()->geometry().width(),0.0);

        StelCore* core = StelApp::getInstance().getCore();
        core->setCurrentProjectionTypeKey("ProjectionFisheye");
        core->getMovementMgr()->zoomTo(180.0, 0.2);
        StelApp::getInstance().fov_height_rate = 1.0;
        StelApp::getInstance().fov_width_rate = 1.0;

    }

}
void ViewDialog::on_channelList_select(QTableWidgetItem* pItem)
{
    StelMainGraphicsView::getInstance().m_pSelectedChannel = StelMainGraphicsView::getInstance().m_pChannels[pItem->row()];

    StelApp::getInstance().getCore()->getMovementMgr()->zoomTo(StelMainGraphicsView::getInstance().m_pSelectedChannel->m_initfov, 0.2);
    StelApp::getInstance().getCore()->getMovementMgr()->setInitFov(StelMainGraphicsView::getInstance().m_pSelectedChannel->m_initfov);

    StelApp::getInstance().getCore()->getMovementMgr()->setViewDirectionJ2000(StelApp::getInstance().getCore()->getNavigator()->altAzToJ2000(StelMainGraphicsView::getInstance().m_pSelectedChannel->m_init_view_pos));

    ui->spWidth->setValue(StelMainGraphicsView::getInstance().m_pSelectedChannel->getWidth());
    ui->spHeight->setValue(StelMainGraphicsView::getInstance().m_pSelectedChannel->getHeight());

    //    StelMainWindow::getInstance().setGeometry(0,0,
    //                                              StelMainGraphicsView::getInstance().m_pSelectedChannel->getWidth(),
    //                                              StelMainGraphicsView::getInstance().m_pSelectedChannel->getHeight());
    //    StelApp::getInstance().getCore()->initWarpGL();

    ui->spboxFWR->setValue(StelMainGraphicsView::getInstance().m_pSelectedChannel->m_fwr);
    ui->spboxFHR->setValue(StelMainGraphicsView::getInstance().m_pSelectedChannel->m_fhr);
    ui->spboxFOV->setValue(StelMainGraphicsView::getInstance().m_pSelectedChannel->m_initfov);
    StelCore* core = StelApp::getInstance().getCore();
    core->setCurrentProjectionTypeKey("ProjectionStereographic");
    //core->setCurrentProjectionTypeKey("ProjectionPerspective");
    //core->setCurrentProjectionTypeKey("ProjectionOrthographic");
}

void ViewDialog::on_btnResetWarp_clicked()
{
    if(QMessageBox::information(0,q_("Warning!"),q_("Are you sure want to reset selected Warp setting?"),QMessageBox::Yes,QMessageBox::No)== QMessageBox::Yes)
    {
        StelMainGraphicsView::getInstance().m_pSelectedChannel->getCurrentWarp()->reset();
    }

}
void ViewDialog::on_visibleChanged(bool b)
{
    StelApp::getInstance().isNetCom = false;
}
void ViewDialog::on_load(bool b)
{
    StelApp::getInstance().isNetCom = true;
}

void ViewDialog::on_value_Width(int val)
{    
    if (StelMainWindow::getInstance().getIsServer() && StelMainGraphicsView::getInstance().isClientConf)
    {
        if(StelMainGraphicsView::getInstance().m_pSelectedChannel)
            StelMainGraphicsView::getInstance().m_pSelectedChannel->setWidth(val);
        StelMainGraphicsView::getInstance().m_pSelectedChannel->updateClientData();
    }
}

void ViewDialog::on_value_Height(int val)
{
    if (StelMainWindow::getInstance().getIsServer() && StelMainGraphicsView::getInstance().isClientConf)
    {
        if(StelMainGraphicsView::getInstance().m_pSelectedChannel)
            StelMainGraphicsView::getInstance().m_pSelectedChannel->setHeight(val);
        StelMainGraphicsView::getInstance().m_pSelectedChannel->updateClientData();
    }
}

void ViewDialog::landscapeVideoChanged(QListWidgetItem* item)
{
    if (!StelMainWindow::getInstance().getIsServer())
    {
        LandscapeMgr* lmgr = GETSTELMODULE(LandscapeMgr);
        lmgr->setCurrentLandscapeName(item->text(),false,true);
        ui->landscapeTextBrowser->document()->setDefaultStyleSheet(QString(StelApp::getInstance().getCurrentStelStyle()->htmlStyleSheet));
        ui->landscapeTextBrowser->setHtml(lmgr->getCurrentLandscapeHtmlDescription());
        ui->useAsDefaultLandscapeCheckBox->setChecked(lmgr->getDefaultLandscapeID()==lmgr->getCurrentLandscapeID());
        ui->useAsDefaultLandscapeCheckBox->setEnabled(lmgr->getDefaultLandscapeID()!=lmgr->getCurrentLandscapeID());

        ui->landscapesListWidget->setCurrentRow(-1);
    }
    //QMessageBox::critical(0,"",lmgr->getCurrentLandscapeID(),0,0);
    StelApp::getInstance().addNetworkCommand("name = LandscapeMgr.setCurrentLandscapeName('"+item->text()+"',false,true);");
}

void ViewDialog::on_setChNameShow(bool b)
{
    StelApp::getInstance().getCore()->showchannel_name = b;
    StelApp::getInstance().getRsync()->sendChanges(RSYNC_COMMAND_SHOWCHNAME,QString("%0").arg(b));
}

void ViewDialog::on_change_FWR(double fwr)
{
    StelApp::getInstance().fov_width_rate = fwr;
    if (StelMainWindow::getInstance().getIsServer() && StelMainGraphicsView::getInstance().isClientConf)
    {
        if(StelMainGraphicsView::getInstance().m_pSelectedChannel)
        {
            StelMainGraphicsView::getInstance().m_pSelectedChannel->set_FWR(fwr);
        }
        StelMainGraphicsView::getInstance().m_pSelectedChannel->updateClientData();
    }
}

void ViewDialog::on_change_FHR(double fhr)
{
    StelApp::getInstance().fov_height_rate = fhr;
    if (StelMainWindow::getInstance().getIsServer() && StelMainGraphicsView::getInstance().isClientConf)
    {
        if(StelMainGraphicsView::getInstance().m_pSelectedChannel)
        {
            StelMainGraphicsView::getInstance().m_pSelectedChannel->set_FHR(fhr);
        }
        StelMainGraphicsView::getInstance().m_pSelectedChannel->updateClientData();
    }
}

void ViewDialog::on_setDateTimeShow(bool b)
{
    StelApp::getInstance().getCore()->flagTuiDatetime = b;
    StelApp::getInstance().addNetworkCommand("core.showDateTime("+QString("%0").arg(b)+");");
}

void ViewDialog::on_setLocationShow(bool b)
{
    StelApp::getInstance().getCore()->flagTuiLocation = b;
    StelApp::getInstance().addNetworkCommand("core.showLocation("+QString("%0").arg(b)+");");
}

void ViewDialog::on_setPropGuiShow(bool b)
{
    StelApp::getInstance().showPropGui = b;
    StelApp::getInstance().addNetworkCommand("core.showProperties("+QString("%0").arg(b)+");");
}
