
#include "stellamanager.hpp"
#include "ui_stellamanager.h"
#include "ui_dateTimeDialogGui.h"
#include "shiraplayerform.hpp"
#include "StelGui.hpp"
#include "StelApp.hpp"
#include "StelModuleMgr.hpp"
#include "GridLinesMgr.hpp"
#include "ConstellationMgr.hpp"
#include "StarMgr.hpp"
#include "SolarSystem.hpp"
#include "NebulaMgr.hpp"
#include "StelMovementMgr.hpp"
#include "LandscapeMgr.hpp"
#include "StelTranslator.hpp"
#include "StelNavigator.hpp"
#include "../plugins/Satellites/src/Satellites.hpp"

#include <QMessageBox>
class Ui_dateTimeDialogForm;

stellamanager::stellamanager(QWidget *parent) :
    QWidget(parent)
{
    ui = new Ui_StellaManagerForm;
    ui->setupUi(this);
    this->setFocusPolicy(Qt::StrongFocus);

    setButtonChecks(ui->btnDateTime);

    loadButtonStatusTimer = new QTimer(this);
    loadButtonStatusTimer->setInterval(1000);
    loadButtonStatusTimer->setSingleShot(true);
    connect(loadButtonStatusTimer, SIGNAL(timeout()), this, SLOT(loadButtonStatus()));
    loadButtonStatusTimer->start();

    connect(ui->horizontalSlider,SIGNAL(sliderMoved(int)),this,SLOT(on_horizontalSlider_moved(int)));
    //loadButtonStatus();

    trackTime = 5;
}

stellamanager::~stellamanager()
{

}

void stellamanager::languageChanged()
{    
    //ui->retranslateUi(this);

    locationDialog.languageChanged();
    helpDialog.languageChanged();
    dateTimeDialog.languageChanged();
    searchDialog.languageChanged();
    recordmanagerDialog.languageChanged();
    planetszoomdialog.languageChanged();
    landscapemanager.languageChanged();
    illimunationdialog.languageChanged();

    ui->btnEquitorialGrid->setText(q_("Equatorial\n"  "Grid"));
    ui->btnAzimuthalGrid->setText(q_("Azimuthal\n""Grid"));
    ui->btnEclipticLine->setText(q_("Ecliptic\n""Line"));
    ui->btnMeridianLine->setText(q_("Meridian\n""Line"));
    ui->btnConstellationLines->setText(q_("Constellation\n""Lines"));
    ui->btnConstellationLabels->setText(q_("Constellation\n""Labels"));
    ui->btnConstellationArts->setText(q_("Constellation\n""Arts"));
    ui->btnConstellationBoundaries->setText(q_("Constellation \n""Boundaries"));
    ui->btnPlanets->setText(q_("Planets"));
    ui->btnNebulae->setText(q_("Nebulae"));
    ui->btnStarLabels->setText(q_("Star Labels"));
    ui->btnSatllites->setText(q_("Satellites"));
    ui->btnAtmosphere->setText(q_("Atmosphere"));
    ui->btnGround->setText(q_("Ground"));
    ui->btnFog->setText(q_("Fog"));
    ui->btnDateTime->setAccessibleName(q_("B1"));
    ui->btnDateTime->setText(q_("Date Time"));
    ui->btnSearch->setAccessibleName(q_("B2"));
    ui->btnSearch->setText(q_("Search"));
    ui->btnGoto->setAccessibleName(q_("B3"));
    ui->btnGoto->setText(q_("Go To"));
    ui->btnLocation->setAccessibleName(q_("B4"));
    ui->btnLocation->setText(q_("Location"));
    ui->btnRecord->setAccessibleName(q_("B5"));
    ui->btnRecord->setText(q_("Record"));
    ui->btnNightView->setText(q_("Night View"));
    ui->btnHelp->setAccessibleName(q_("B6"));
    ui->btnHelp->setText(q_("Help"));
    ui->btnSouth->setText(q_("South"));
    ui->btnNorth->setText(q_("North"));
    ui->btnWest->setText(q_("West"));
    ui->btnEast->setText(q_("East"));
    ui->lblFront->setText(q_("Front Sky"));
    ui->btnStarTrails->setText(q_("Star Trails"));
    ui->btnStarMag->setText(q_("Star Mag. Limit"));
    ui->labelMag->setText(q_("Mag. >="));
    ui->btnCardinal->setText(q_("Compass\n""Points"));
    ui->btnPlanetZoom->setText(q_("Planet Zoom"));
    ui->chSelectedStar->setText(q_("Selected Label"));
    ui->chSelectedPlanet->setText(q_("Selected Label"));
    ui->btnLandscape->setText(q_("Landscapes"));
    ui->btnIlluminate->setText(q_("Illumination"));
}

void stellamanager::showEvent(QShowEvent *event)
{
    ui->wfistRow->setEnabled( !StelApp::getInstance().getstartedFlyby() );
    //ui->wSecondRow->setEnabled(!StelApp::getInstance().getstartedFlyby() );
    ui->wThirdRow->setEnabled( !StelApp::getInstance().getstartedFlyby() );
    ui->wFourthRow->setEnabled(!StelApp::getInstance().getstartedFlyby() );
    ui->btnLocation->setEnabled(!StelApp::getInstance().getstartedFlyby() );
    locationDialog.getDialog()->setEnabled(!StelApp::getInstance().getstartedFlyby());
    planetszoomdialog.getDialog()->setEnabled(!StelApp::getInstance().getstartedFlyby());
    landscapemanager.getDialog()->setEnabled(!StelApp::getInstance().getstartedFlyby());
}

void stellamanager::on_btnDateTime_clicked()
{
    setButtonChecks(ui->btnDateTime);
}
void stellamanager::on_btnSearch_clicked()
{
    setButtonChecks(ui->btnSearch);
}
void stellamanager::on_btnLocation_clicked()
{
    setButtonChecks(ui->btnLocation);
}
void stellamanager::on_btnRecord_clicked()
{
    setButtonChecks(ui->btnRecord);
}
void stellamanager::on_btnHelp_clicked()
{
    setButtonChecks(ui->btnHelp);
}
void stellamanager::on_btnPlanetZoom_clicked()
{
    setButtonChecks(ui->btnPlanetZoom);
}
void stellamanager::on_btnLandscape_clicked()
{
    setButtonChecks(ui->btnLandscape);
}

void stellamanager::on_btnIlluminate_clicked()
{
   setButtonChecks(ui->btnIlluminate);
}

void stellamanager::setButtonChecks(QPushButton* sender)
{
    //StelGui* sgui = dynamic_cast<StelGui*>(StelApp::getInstance().getGui());
    //sgui->servermanagerDialog.getProxy()->setVisible(true);
    QString caseTab = sender->accessibleName();
    //
    dateTimeDialog.setVisibleAsChild(ui->widgetStella,false);
    locationDialog.setVisibleAsChild(ui->widgetStella,false);
    helpDialog.setVisibleAsChild(ui->widgetStella,false);
    dateTimeDialog.setVisibleAsChild(ui->widgetStella,false);
    searchDialog.setVisibleAsChild(ui->widgetStella,false);
    recordmanagerDialog.setVisibleAsChild(ui->widgetStella,false);
    planetszoomdialog.setVisibleAsChild(ui->widgetStella,false);
    landscapemanager.setVisibleAsChild(ui->widgetStella,false);
    illimunationdialog.setVisibleAsChild(ui->widgetStella,false);
    if (caseTab == "B1")
    {
        dateTimeDialog.setVisibleAsChild(ui->widgetStella,true);
        ui->stellaLayout->addWidget(dateTimeDialog.getDialog());
    }
    else if (caseTab == "B2")
    {
        searchDialog.setVisibleAsChild(ui->widgetStella,true);
        ui->stellaLayout->addWidget(searchDialog.getDialog());
    }
    else if (caseTab == "B4")
    {
        locationDialog.setVisibleAsChild(ui->widgetStella,true);
        ui->stellaLayout->addWidget(locationDialog.getDialog());
    }
    else if (caseTab == "B5")
    {
        recordmanagerDialog.setVisibleAsChild(ui->widgetStella,true);
        ui->stellaLayout->addWidget(recordmanagerDialog.getDialog());
    }
    else if (caseTab == "B6")
    {
        helpDialog.setVisibleAsChild(ui->widgetStella,true);
        ui->stellaLayout->addWidget(helpDialog.getDialog());

        StelGui* sgui = dynamic_cast<StelGui*>(StelApp::getInstance().getGui());
        helpDialog.setHtmlText(sgui->helpDialog.getHtmlText());
    }
    else if(caseTab == "B7")
    {
        planetszoomdialog.setVisibleAsChild(ui->widgetStella,true);
        ui->stellaLayout->addWidget(planetszoomdialog.getDialog());
    }
    else if(caseTab == "B8")
    {
        landscapemanager.setVisibleAsChild(ui->widgetStella,true);
        ui->stellaLayout->addWidget(landscapemanager.getDialog());
    }
    else if(caseTab == "B9")
    {
        illimunationdialog.setVisibleAsChild(ui->widgetStella,true);
        ui->stellaLayout->addWidget(illimunationdialog.getDialog());
    }

    ui->btnDateTime->setChecked(false);
    //ui->btnGoto->setChecked(false);
    ui->btnHelp->setChecked(false);
    ui->btnLocation->setChecked(false);
    ui->btnRecord->setChecked(false);
    ui->btnSearch->setChecked(false);
    ui->btnRecord->setChecked(false);
    ui->btnPlanetZoom->setChecked(false);
    ui->btnLandscape->setChecked(false);
    ui->btnIlluminate->setChecked(false);
    sender->setChecked(true);
}

void stellamanager::loadButtonStatus()
{
    QAction* a;
    StelGuiBase* gui = StelApp::getInstance().getGui();

    GridLinesMgr* glmgr = GETSTELMODULE(GridLinesMgr);
    ConstellationMgr* cmgr = GETSTELMODULE(ConstellationMgr);
    SolarSystem* ssmgr = GETSTELMODULE(SolarSystem);
    NebulaMgr* nmgr = GETSTELMODULE(NebulaMgr);
    StarMgr* smgr = GETSTELMODULE(StarMgr);
    Satellites* sat= GETSTELMODULE(Satellites);
    LandscapeMgr* lmgr = GETSTELMODULE(LandscapeMgr);

    ui->btnEquitorialGrid->setChecked(glmgr->getFlagEquatorGrid());
    a = gui->getGuiActions("actionShow_Equatorial_Grid");
    connect(a, SIGNAL(toggled(bool)), ui->btnEquitorialGrid, SLOT(setChecked(bool)));
    connect(ui->btnEquitorialGrid, SIGNAL(toggled(bool)), a, SLOT(setChecked(bool)));

    ui->btnAzimuthalGrid->setChecked(glmgr->getFlagAzimuthalGrid());
    a = gui->getGuiActions("actionShow_Azimuthal_Grid");
    connect(a, SIGNAL(toggled(bool)), ui->btnAzimuthalGrid, SLOT(setChecked(bool)));
    connect(ui->btnAzimuthalGrid, SIGNAL(toggled(bool)), a, SLOT(setChecked(bool)));

    ui->btnEclipticLine->setChecked(glmgr->getFlagEclipticLine());
    a = gui->getGuiActions("actionShow_Ecliptic_Line");
    connect(a, SIGNAL(toggled(bool)), ui->btnEclipticLine, SLOT(setChecked(bool)));
    connect(ui->btnEclipticLine, SIGNAL(toggled(bool)), a, SLOT(setChecked(bool)));

    ui->btnMeridianLine->setChecked(glmgr->getFlagMeridianLine());
    a = gui->getGuiActions("actionShow_Meridian_Line");
    connect(a, SIGNAL(toggled(bool)), ui->btnMeridianLine, SLOT(setChecked(bool)));
    connect(ui->btnMeridianLine, SIGNAL(toggled(bool)), a, SLOT(setChecked(bool)));

    ui->btnConstellationLines->setChecked(cmgr->getFlagLines());
    a = gui->getGuiActions("actionShow_Constellation_Lines");
    connect(a, SIGNAL(toggled(bool)), ui->btnConstellationLines, SLOT(setChecked(bool)));
    connect(ui->btnConstellationLines, SIGNAL(toggled(bool)), a, SLOT(setChecked(bool)));

    ui->btnConstellationLabels->setChecked(cmgr->getFlagLabels());
    a = gui->getGuiActions("actionShow_Constellation_Labels");
    connect(a, SIGNAL(toggled(bool)), ui->btnConstellationLabels, SLOT(setChecked(bool)));
    connect(ui->btnConstellationLabels, SIGNAL(toggled(bool)), a, SLOT(setChecked(bool)));

    ui->btnConstellationBoundaries->setChecked(cmgr->getFlagBoundaries());
    a = gui->getGuiActions("actionShow_Constellation_Boundaries");
    connect(a, SIGNAL(toggled(bool)), ui->btnConstellationBoundaries, SLOT(setChecked(bool)));
    connect(ui->btnConstellationBoundaries, SIGNAL(toggled(bool)), a, SLOT(setChecked(bool)));

    ui->btnConstellationArts->setChecked(cmgr->getFlagArt());
    a = gui->getGuiActions("actionShow_Constellation_Art");
    connect(a, SIGNAL(toggled(bool)), ui->btnConstellationArts, SLOT(setChecked(bool)));
    connect(ui->btnConstellationArts, SIGNAL(toggled(bool)), a, SLOT(setChecked(bool)));

    ui->btnPlanets->setChecked(ssmgr->getFlagPlanets());
    a = gui->getGuiActions("actionShow_Planets");
    connect(a, SIGNAL(toggled(bool)), ui->btnPlanets, SLOT(setChecked(bool)));
    connect(ui->btnPlanets, SIGNAL(toggled(bool)), a, SLOT(setChecked(bool)));

    ui->chSelectedPlanet->setChecked(ssmgr->getFlagSelectedLabel());
    a = gui->getGuiActions("actionShow_PlanetSelected_Label");
    connect(a, SIGNAL(toggled(bool)), ui->chSelectedPlanet, SLOT(setChecked(bool)));
    connect(ui->chSelectedPlanet, SIGNAL(toggled(bool)), a, SLOT(setChecked(bool)));

    ui->btnNebulae->setChecked(nmgr->getFlagHints());
    a = gui->getGuiActions("actionShow_Nebulas");
    connect(a, SIGNAL(toggled(bool)), ui->btnNebulae, SLOT(setChecked(bool)));
    connect(ui->btnNebulae, SIGNAL(toggled(bool)), a, SLOT(setChecked(bool)));

    ui->btnStarLabels->setChecked(smgr->getFlagLabels());
    a = gui->getGuiActions("actionShow_Star_Labels");
    connect(a, SIGNAL(toggled(bool)), ui->btnStarLabels, SLOT(setChecked(bool)));
    connect(ui->btnStarLabels, SIGNAL(toggled(bool)), a, SLOT(setChecked(bool)));

    ui->chSelectedStar->setChecked(smgr->getFlagSelectedLabel());
    a = gui->getGuiActions("actionShow_StarSelected_Labels");
    connect(a, SIGNAL(toggled(bool)), ui->chSelectedStar, SLOT(setChecked(bool)));
    connect(ui->chSelectedStar, SIGNAL(toggled(bool)), a, SLOT(setChecked(bool)));

    ui->btnAtmosphere->setChecked(lmgr->getFlagAtmosphere());
    a = gui->getGuiActions("actionShow_Atmosphere");
    connect(a, SIGNAL(toggled(bool)), ui->btnAtmosphere, SLOT(setChecked(bool)));
    connect(ui->btnAtmosphere, SIGNAL(toggled(bool)), a, SLOT(setChecked(bool)));

    ui->btnGround->setChecked(lmgr->getFlagLandscape());
    a = gui->getGuiActions("actionShow_Ground");
    connect(a, SIGNAL(toggled(bool)), ui->btnGround, SLOT(setChecked(bool)));
    connect(ui->btnGround, SIGNAL(toggled(bool)), a, SLOT(setChecked(bool)));

    ui->btnFog->setChecked(lmgr->getFlagFog());
    a = gui->getGuiActions("actionShow_Fog");
    connect(a, SIGNAL(toggled(bool)), ui->btnFog, SLOT(setChecked(bool)));
    connect(ui->btnFog, SIGNAL(toggled(bool)), a, SLOT(setChecked(bool)));

    ui->btnCardinal->setChecked(lmgr->getFlagCardinalsPoints());
    a = gui->getGuiActions("actionShow_Cardinal_Points");
    connect(a,SIGNAL(toggled(bool)),ui->btnCardinal,SLOT(setChecked(bool)));
    connect(ui->btnCardinal,SIGNAL(toggled(bool)),a,SLOT(setChecked(bool)));

    QSettings* conf = StelApp::getInstance().getSettings();
    ui->btnSatllites->setChecked(conf->value("Satellites/show_satellites", false).toBool());
    on_btnSatllites_toggled(conf->value("Satellites/show_satellites", false).toBool());

    const QList<StelModuleMgr::PluginDescriptor> pluginsList = StelApp::getInstance().getModuleMgr().getPluginsList();
    foreach (const StelModuleMgr::PluginDescriptor& desc, pluginsList)
    {
        if (desc.info.id == "Satellites")
        {
            ui->btnSatllites->setEnabled(desc.loaded);
            if (!ui->btnSatllites->isEnabled())
                ui->btnSatllites->setStyleSheet("QPushButton { background-color: rgb(0, 0, 0);}");
        }
    }

}

void stellamanager::on_btnSatllites_toggled(bool b)
{
    const QList<StelModuleMgr::PluginDescriptor> pluginsList = StelApp::getInstance().getModuleMgr().getPluginsList();
    foreach (const StelModuleMgr::PluginDescriptor& desc, pluginsList)
    {
        if (desc.info.id == "Satellites")
        {
            if (desc.loaded)
            {
                StelApp::getInstance().getGui()->getGuiActions("actionShow_Satellite_Show")->setChecked(b);
            }
            else
            {
                // QMessageBox::warning(0,"Warning","Please select Satellites module 'Load at Startup' from Configration window'",0,0);
            }
        }
    }
}

void stellamanager::on_btnGoto_toggled(bool b)
{
    StelMovementMgr* mmgr = GETSTELMODULE(StelMovementMgr);
    if (b) { mmgr->autoZoomIn(3.0);}
    else { mmgr->autoZoomOut(3.0,true);}
}

void stellamanager::on_btnNightView_toggled(bool checked)
{
    StelApp::getInstance().setVisionModeNight(checked);
    StelMainGraphicsView::getInstance().setFocus();
}

void stellamanager::on_btnStarTrails_toggled(bool checked)
{
    StarMgr* smgr = GETSTELMODULE(StarMgr);
    smgr->setFlagTrails(checked);
    StelApp::getInstance().getRsync()->sendChanges(RSYNC_COMMAND_STELLA_STARTRAILS,
                                                   QString("%1@%2@%3")
                                                   .arg(smgr->getFlagTrails())
                                                   .arg(smgr->getFlagMagLevel())
                                                   .arg(ui->horizontalSlider->value() / 10.0));


    Sleep(200);

    if (checked)
        dateTimeDialog.ui->comboBox->setCurrentIndex(2);
    else
        dateTimeDialog.ui->comboBox->setCurrentIndex(1);
}

void stellamanager::on_horizontalSlider_moved(int value)
{
    StarMgr* smgr = GETSTELMODULE(StarMgr);
    double realVal = double(value / 10.0);
    QString s;
    s = s.setNum(realVal, 'g', 2);
    ui->lblMag->setText(s);
    smgr->setlimitMagValue(ui->horizontalSlider->value() / 10.0);
    StelApp::getInstance().getRsync()->sendChanges(RSYNC_COMMAND_STELLA_STARMAGLIMITVAL,
                                                   QString("%0")
                                                   .arg(ui->horizontalSlider->value() / 10.0));
}

void stellamanager::on_btnStarMag_toggled(bool checked)
{
    StarMgr* smgr = GETSTELMODULE(StarMgr);
    smgr->setlimitMagValue(ui->horizontalSlider->value() / 10.0);
    smgr->setFlagMagLevel(checked);
    StelApp::getInstance().getRsync()->sendChanges(RSYNC_COMMAND_STELLA_ISSTARMAGLIMIT,
                                                   QString("%0@%1")
                                                   .arg(checked)
                                                   .arg(ui->horizontalSlider->value() / 10.0));
}

void stellamanager::on_btnNorth_clicked()
{
    StelApp::getInstance().getCore()->getMovementMgr()->setFrontViewDirection("N",trackTime);
    StelApp::getInstance().getRsync()->sendChanges(RSYNC_COMMAND_STELLA_FRONTSKY,
                                                   QString("%0@%1")
                                                   .arg("N")
                                                   .arg(trackTime));
}

void stellamanager::on_btnSouth_clicked()
{
    StelApp::getInstance().getCore()->getMovementMgr()->setFrontViewDirection("S",trackTime);
    StelApp::getInstance().getRsync()->sendChanges(RSYNC_COMMAND_STELLA_FRONTSKY,
                                                   QString("%0@%1")
                                                   .arg("S")
                                                   .arg(trackTime));
}

void stellamanager::on_btnWest_clicked()
{
    StelApp::getInstance().getCore()->getMovementMgr()->setFrontViewDirection("W",trackTime);
    StelApp::getInstance().getRsync()->sendChanges(RSYNC_COMMAND_STELLA_FRONTSKY,
                                                   QString("%0@%1")
                                                   .arg("W")
                                                   .arg(trackTime));
}

void stellamanager::on_btnEast_clicked()
{
    StelApp::getInstance().getCore()->getMovementMgr()->setFrontViewDirection("E",trackTime);
    StelApp::getInstance().getRsync()->sendChanges(RSYNC_COMMAND_STELLA_FRONTSKY,
                                                   QString("%0@%1")
                                                   .arg("E")
                                                   .arg(trackTime));
}

void stellamanager::retranslate()
{
    languageChanged();
}

