#include "flybymanager.h"
#include "ui_flybymanager.h"

#include "StelLocation.hpp"
#include "StelApp.hpp"
#include "StelNavigator.hpp"
#include "StelCore.hpp"
#include "StelObjectMgr.hpp"
#include "SolarSystem.hpp"
#include "StelModuleMgr.hpp"
#include "StelUtils.hpp"
#include "StelMovementMgr.hpp"
#include "StelGui.hpp"
#include "StelTranslator.hpp"

FlybyManager::FlybyManager(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FlybyManager)
{
    ui->setupUi(this);
    realtrackValue = 2;
    timerFlyby = new QTimer();
    timerFlyby->setInterval(1000);
    connect(timerFlyby,SIGNAL(timeout()),this,SLOT(on_flyby_tmeout()));

    connect(ui->btnSun,SIGNAL(clicked()),this,SLOT(on_btnEarth_clicked()));
    connect(ui->btnMercury,SIGNAL(clicked()),this,SLOT(on_btnEarth_clicked()));
    connect(ui->btnVenus,SIGNAL(clicked()),this,SLOT(on_btnEarth_clicked()));
    connect(ui->btnMoon,SIGNAL(clicked()),this,SLOT(on_btnEarth_clicked()));
    connect(ui->btnMars,SIGNAL(clicked()),this,SLOT(on_btnEarth_clicked()));
    connect(ui->btnJupiter,SIGNAL(clicked()),this,SLOT(on_btnEarth_clicked()));
    connect(ui->btnSaturn,SIGNAL(clicked()),this,SLOT(on_btnEarth_clicked()));
    connect(ui->btnUranus,SIGNAL(clicked()),this,SLOT(on_btnEarth_clicked()));
    connect(ui->btnNeptune,SIGNAL(clicked()),this,SLOT(on_btnEarth_clicked()));
    connect(ui->btnPluto,SIGNAL(clicked()),this,SLOT(on_btnEarth_clicked()));
    connect(ui->btnSolarSystem,SIGNAL(clicked()),this,SLOT(on_btnEarth_clicked()));
    connect(ui->btnHome,SIGNAL(clicked()),this,SLOT(on_btnEarth_clicked()));


    SolarSystem* ssmgr = GETSTELMODULE(SolarSystem);

    ui->btnOrbit->setChecked(ssmgr->getFlagOrbits());

    ui->btnAtm->setChecked(ssmgr->getFlagFlybyAtm());
    connect(ui->btnAtm , SIGNAL(toggled(bool)), ssmgr, SLOT(setFlagFlybyAtm(bool)));

    QAction* a;
    StelGuiBase* gui = StelApp::getInstance().getGui();
    ui->btnAxis->setChecked(ssmgr->getFlagAxises());
    a = gui->getGuiActions("actionShow_Planets_Axis");
    connect(a, SIGNAL(toggled(bool)), ui->btnAxis, SLOT(setChecked(bool)));
    connect(ui->btnAxis, SIGNAL(toggled(bool)), a, SLOT(setChecked(bool)));

    ui->btnEquline->setChecked(ssmgr->getFlagEquline());
    a = gui->getGuiActions("actionShow_Planets_Equator");
    connect(a, SIGNAL(toggled(bool)), ui->btnEquline, SLOT(setChecked(bool)));
    connect(ui->btnEquline, SIGNAL(toggled(bool)), a, SLOT(setChecked(bool)));
}

FlybyManager::~FlybyManager()
{
    disconnect();
    delete ui;
}

void FlybyManager::languageChanged()
{
    //ui->retranslateUi();
    ui->btnStart->setText(q_("Start Tracking"));
    ui->lblVelocity->setText(q_("Velocity :"));
    ui->velocitySlider->setStyleSheet(q_("font: 14pt \"MS Shell Dlg 2\";"));
    ui->lblAltitude->setText(q_("Altitude :"));
    ui->btnOrbit->setText(q_("Show Orbits"));
    ui->btnAtm->setText(q_("Show Atm"));
    ui->btnAxis->setText(q_("Planet Axis"));
    ui->btnEquline->setText(q_("Equator Line"));
    ui->label->setText(q_("Date Time\n""Flow Rate:"));
    ui->comboBox->clear();
    ui->comboBox->insertItems(0, QStringList()
     << q_("Time Flow")
     << q_("  1x(Real Time)")
     << q_("  30x")
     << q_("  300x")
     << q_("  3000x")
     << q_("  30000x")
     << q_("Discrete Time Steps")
     << q_("  seconds")
     << q_("  minutes")
     << q_("  hours")
     << q_("  days")
     << q_("  years")
    );
    ui->rdOuter->setText(q_("Outer Planets"));
    ui->rdInner->setText(q_("Inner Planets"));
    ui->btnSolarSystem->setAccessibleName(q_("FLYBY-SS"));
    ui->btnSolarSystem->setText(q_("Solar System"));
    ui->btnHome->setText(q_("Prepare for flyby"));
    ui->btnJupiter->setAccessibleName(q_("FLYBY-Jupiter"));
    ui->btnJupiter->setText(q_("Jupiter"));
    ui->btnNeptune->setAccessibleName(q_("FLYBY-Neptune"));
    ui->btnNeptune->setText(q_("Neptune"));
    ui->btnMars->setAccessibleName(q_("FLYBY-Mars"));
    ui->btnMars->setText(q_("Mars"));
    ui->btnSaturn->setAccessibleName(q_("FLYBY-Saturn"));
    ui->btnSaturn->setText(q_("Saturn"));
    ui->btnSun->setAccessibleName(q_("FLYBY-Sun"));
    ui->btnSun->setText(q_("Sun"));
    ui->btnVenus->setAccessibleName(q_("FLYBY-Venus"));
    ui->btnVenus->setText(q_("Venus"));
    ui->btnMoon->setAccessibleName(q_("FLYBY-Moon"));
    ui->btnMoon->setText(q_("Moon"));
    ui->btnEarth->setAccessibleName(q_("FLYBY-Earth"));
    ui->btnEarth->setText(q_("Earth"));
    ui->btnUranus->setAccessibleName(q_("FLYBY-Uranus"));
    ui->btnUranus->setText(q_("Uranus"));
    ui->btnMercury->setAccessibleName(q_("FLYBY-Mercury"));
    ui->btnMercury->setText(q_("Mercury"));
    ui->btnPluto->setAccessibleName(q_("FLYBY-Pluto"));
    ui->btnPluto->setText(q_("Pluto"));

}

void FlybyManager::on_altitudeSlider_sliderReleased()
{
    if (selectedPlanet == NULL) return;

    PlanetP p = selectedPlanet;
    PlanetP pmain = p.data()->getFlybyPlanet();
    double newRadius = p.data()->getflybyFactor() * pmain.data()->getRadius() + ui->altitudeSlider->value() / AU ;
    p.data()->setRadiusforFader(newRadius);
    p.data()->setRadiusFader(true);
    ui->lblAltitude->setText(QString("Altitude: %0 km").arg( ((p.data()->getflybyFactor() -1) * pmain.data()->getRadius() * AU )+ ui->altitudeSlider->value()));

    double slope = 1.;
    if(p->getEnglishName()=="FLYBY-SS")
        slope = (- M_PI_4) +( -1 * ui->altitudeSlider->value() * M_PI_4 * (44.0 / 45.0)  / ui->altitudeSlider->maximum());
    else
        slope = -1 * ui->altitudeSlider->value() * M_PI_2 * (88.0 / 90.0)  / ui->altitudeSlider->maximum();
    StelMovementMgr* mvmgr = GETSTELMODULE(StelMovementMgr);
    Vec3d aim;
    StelUtils::spheToRect(M_PI_2,slope,aim);
    mvmgr->moveToJ2000(StelApp::getInstance().getCore()->getNavigator()->altAzToJ2000(aim), 1);

    calcVelocity();

    StelApp::getInstance().getRsync()->sendChanges(RSYNC_COMMAND_FLYBY_SETALTITUDE,
                                                   QString("%0@%1@%2@%3@%4").arg(p->getEnglishName())
                                                   .arg(newRadius)
                                                   .arg(aim.v[0])
                                                   .arg(aim.v[1])
                                                   .arg(aim.v[2]));
}


void FlybyManager::on_btnStart_clicked()
{
    if (selectedPlanet == NULL) return;

    if (timerFlyby->isActive())
    {
        timerFlyby->stop();
        ui->btnStart->setText(q_("Start Tracking"));
    }
    else
    {
        timerFlyby->start();
        ui->btnStart->setText(q_("Stop"));
    }

    PlanetP p = selectedPlanet;
    PlanetP pmain = p.data()->getFlybyPlanet();
    double newRadius = p.data()->getflybyFactor() * pmain.data()->getRadius() + ui->altitudeSlider->value() / AU ;
    p.data()->setRadiusforFader(newRadius);
    p.data()->setRadiusFader(true);
    ui->lblAltitude->setText(QString(q_("Altitude: %0 km")).arg( ((p.data()->getflybyFactor() -1) * pmain.data()->getRadius() * AU )+ ui->altitudeSlider->value()));

    calcVelocity();
}

void FlybyManager::on_flyby_tmeout()
{
    double trackValue = realtrackValue;
    StelLocation loc = StelApp::getInstance().getCore()->getNavigator()->getCurrentLocation();

    if( loc.latitude > 90.0 )  { realtrackValue = -1 * realtrackValue ;trackValue = realtrackValue; }
    if( loc.latitude < -90.0 ) { realtrackValue = -1 * realtrackValue ;trackValue = realtrackValue; }

    loc.longitude = loc.longitude + qAbs(trackValue);
    loc.latitude = loc.latitude + realtrackValue / 10.0;
    StelApp::getInstance().getCore()->getNavigator()->moveObserverTo(loc, 2.0, 2.0,true);
    //ui->lblVelocity->setText(QString("%0\n%1").arg(loc.latitude).arg(loc.longitude));
    calcVelocity();
    StelApp::getInstance().getRsync()->sendChanges(RSYNC_COMMAND_FLYBY_SETPOS,
                                                   QString("%0@%1@%2").arg(loc.longitude)
                                                   .arg(loc.latitude)
                                                   .arg(2.0) );

}

void FlybyManager::calcVelocity()
{
    if (selectedPlanet == NULL) return;
    double velocity = 0;
    if (timerFlyby->isActive())
        velocity = qAbs(2.0 * M_PI * selectedPlanet.data()->getRadius() * AU * realtrackValue /360 );
    ui->lblVelocity->setText(QString(q_("Velocity: %0 km/s")).arg(QString::number(velocity, 'f', 2 )));

    //ui->lblVelocity->setText(QString("lat: %0 lon: %1").arg(StelApp::getInstance().getCore()->getNavigator()->getCurrentLocation().latitude)
    //                         .arg(StelApp::getInstance().getCore()->getNavigator()->getCurrentLocation().longitude));

}

void FlybyManager::setButtons(QToolButton *sender)
{
    ui->btnSun->setChecked(false);
    ui->btnMercury->setChecked(false);
    ui->btnVenus->setChecked(false);
    ui->btnEarth->setChecked(false);
    ui->btnMoon->setChecked(false);
    ui->btnMars->setChecked(false);
    ui->btnJupiter->setChecked(false);
    ui->btnSaturn->setChecked(false);
    ui->btnUranus->setChecked(false);
    ui->btnNeptune->setChecked(false);
    ui->btnPluto->setChecked(false);
    ui->btnSolarSystem->setChecked(false);
    ui->btnHome->setChecked(false);

    sender->setChecked(true);

}


void FlybyManager::on_btnEarth_clicked()
{
    QToolButton* button = (QToolButton*)(sender());
    if (timerFlyby->isActive())
    {
        timerFlyby->stop();
        ui->btnStart->setText(q_("Start"));
    }
    if (button == ui->btnHome)
    {
        if (button->text() == q_("Prepare for flyby"))
        {

            StelGui* sgui = dynamic_cast<StelGui*>(StelApp::getInstance().getGui());
            sgui->configurationDialog->getProxy()->close();
            sgui->viewDialog.getProxy()->close();
            sgui->scriptConsole.getProxy()->close();

            StelApp::getInstance().prepareFlyBy();
            ui->controlsFrame->setEnabled(true);
            ui->planetsFrame->setEnabled(true);
            ui->ssFrame->setEnabled(true);
            button->setIcon(QIcon(":/gui/planets/gui/planets/home.png"));
            button->setText(q_("Go Home"));
            //client tarafý
            StelApp::getInstance().addNetworkCommand("core.clear('spaceship');");
            ui->widgetControls->setEnabled(true);
            ui->chFullLight->setEnabled(true);
        }
        else
        {
            StelApp::getInstance().goHome();
            ui->controlsFrame->setEnabled(false);
            ui->planetsFrame->setEnabled(false);
            ui->ssFrame->setEnabled(false);
            ui->chFullLight->setEnabled(false);
            button->setIcon(QIcon(":/gui/planets/gui/planets/spaceship.png"));
            button->setText(q_("Prepare for flyby"));

            //Client tarafý
            StelApp::getInstance().addNetworkCommand("core.clear('returnback');");
            Sleep(100);
            StelApp::getInstance().getRsync()->sendChanges(RSYNC_COMMAND_FLYBY_SETHOME,"");
            //
            ui->widgetControls->setEnabled(false);
            ui->comboBox->blockSignals(true);
            ui->comboBox->setCurrentIndex(1);
            ui->comboBox->blockSignals(false);
        }
        setButtons(button);
        return;
    }
    double value = 0;
    StelApp::getInstance().setFlyBy(button->accessibleName(),value,ui->rdInner->isChecked());

    ui->lblAltitude->setText(QString(q_("Altitude: %0 km")).arg( value ));
    ui->altitudeSlider->setValue(value );
    SolarSystem* ssmgr = GETSTELMODULE(SolarSystem);
    PlanetP p = ssmgr->searchByEnglishNameForFlyby(button->accessibleName());
    selectedPlanet = p;

    ui->widgetControls->setEnabled(true);
    calcVelocity();

    // Sadece Solar System seÃ§ili iken orbitleri gÃ¶ster
    if (button == ui->btnSolarSystem)
    {
        ssmgr->setFlagOrbits(ui->btnOrbit->isChecked());
        Sleep(100);
    }
    else
    {
        if (ssmgr->getFlagOrbits())
        {
            ssmgr->setFlagOrbits(false);
            Sleep(100);
        }
    }

    StelApp::getInstance().getRsync()->sendChanges(RSYNC_COMMAND_FLYBY_SETPLANET,
                                                   QString("%0@%1").arg(button->accessibleName())
                                                                   .arg(ui->rdInner->isChecked()));
    setButtons(button);

}

void FlybyManager::on_velocitySlider_sliderReleased()
{
    if (realtrackValue < 0)
        realtrackValue = -1 * ui->velocitySlider->value();
    else
        realtrackValue = ui->velocitySlider->value();

    calcVelocity();
}


void FlybyManager::on_comboBox_currentIndexChanged(int index)
{
    double s;// = StelApp::getInstance().getCore()->getNavigator()->getTimeRate();
    StelApp::getInstance().isDiscreteTimeSteps = false;
    //StelApp::getInstance().addNetworkCommand("core.setDiscreteTime('stop');");

    switch (index)
    {
    case 0:
    {
        ui->comboBox->setCurrentIndex(1);
        break;
    }
    case 1:
    {
        s = StelApp::getInstance().m_timedirection*JD_SECOND;
        StelApp::getInstance().getCore()->getNavigator()->setTimeRate(s);
        StelApp::getInstance().isTimeCommand = true;
        StelApp::getInstance().addNetworkCommand(QString("core.setDiscreteTime('stop');core.setTimeRate(%0);").arg(StelApp::getInstance().m_timedirection*1));
        StelApp::getInstance().isTimeCommand = false;
        break ;
    }
    case 2:
    {
        s = StelApp::getInstance().m_timedirection*30.*JD_SECOND;
        StelApp::getInstance().getCore()->getNavigator()->setTimeRate(s);
        StelApp::getInstance().isTimeCommand = true;
        StelApp::getInstance().addNetworkCommand(QString("core.setDiscreteTime('stop');core.setTimeRate(%0);").arg(StelApp::getInstance().m_timedirection*30));
        StelApp::getInstance().isTimeCommand = false;
        break ;
    }
    case 3:
    {
        s = StelApp::getInstance().m_timedirection*300.*JD_SECOND;
        StelApp::getInstance().getCore()->getNavigator()->setTimeRate(s);
        StelApp::getInstance().isTimeCommand = true;
        StelApp::getInstance().addNetworkCommand(QString("core.setDiscreteTime('stop');core.setTimeRate(%0);").arg(StelApp::getInstance().m_timedirection*300));
        StelApp::getInstance().isTimeCommand = false;
        break ;
    }
    case 4:
    {
        s = StelApp::getInstance().m_timedirection*3000.*JD_SECOND;
        StelApp::getInstance().getCore()->getNavigator()->setTimeRate(s);
        StelApp::getInstance().isTimeCommand = true;
        StelApp::getInstance().addNetworkCommand(QString("core.setDiscreteTime('stop');core.setTimeRate(%0);").arg(StelApp::getInstance().m_timedirection*3000));
        StelApp::getInstance().isTimeCommand = false;
        break ;
    }
    case 5:
    {
        s = StelApp::getInstance().m_timedirection*30000.*JD_SECOND;
        StelApp::getInstance().getCore()->getNavigator()->setTimeRate(s);
        //QMessageBox::critical(0,"",QString("core.setTimeRate(%0);").arg(s),0,0);
        StelApp::getInstance().isTimeCommand = true;
        StelApp::getInstance().addNetworkCommand(QString("core.setDiscreteTime('stop');core.setTimeRate(%0);").arg(StelApp::getInstance().m_timedirection*30000));
        StelApp::getInstance().isTimeCommand = false;
        break ;
    }
    case 6:
    {
        ui->comboBox->setCurrentIndex(7);
        break;
    }
    case 7:
    {
        StelApp::getInstance().m_discrete = StelApp::DiscreteTimeStepsSeconds;
        StelApp::getInstance().isDiscreteTimeSteps = true;
        StelApp::getInstance().isTimeCommand = true;
        StelApp::getInstance().addNetworkCommand("core.setDiscreteTime('seconds');");
        StelApp::getInstance().isTimeCommand = false;
        break;
    }
    case 8:
    {
        StelApp::getInstance().m_discrete = StelApp::DiscreteTimeStepsMinutes;
        StelApp::getInstance().isDiscreteTimeSteps = true;
        StelApp::getInstance().isTimeCommand = true;
        StelApp::getInstance().addNetworkCommand("core.setDiscreteTime('minutes');");
        StelApp::getInstance().isTimeCommand = false;
        break;
    }
    case 9:
    {
        StelApp::getInstance().m_discrete = StelApp::DiscreteTimeStepsHours;
        StelApp::getInstance().isDiscreteTimeSteps = true;
        StelApp::getInstance().isTimeCommand = true;
        StelApp::getInstance().addNetworkCommand("core.setDiscreteTime('hours');");
        StelApp::getInstance().isTimeCommand = false;
        break;
    }
    case 10:
    {
        StelApp::getInstance().m_discrete = StelApp::DiscreteTimeStepsDays;
        StelApp::getInstance().isDiscreteTimeSteps = true;
        StelApp::getInstance().isTimeCommand = true;
        StelApp::getInstance().addNetworkCommand("core.setDiscreteTime('days');");
        StelApp::getInstance().isTimeCommand = false;
        break;
    }
    case 11:
    {
        StelApp::getInstance().m_discrete = StelApp::DiscreteTimeStepsYears;
        StelApp::getInstance().isDiscreteTimeSteps = true;
        StelApp::getInstance().isTimeCommand = true;
        StelApp::getInstance().addNetworkCommand("core.setDiscreteTime('years');");
        StelApp::getInstance().isTimeCommand = false;
        break;
    }
    }
}

void FlybyManager::on_btnOrbit_toggled(bool checked)
{
    if (!ui->btnSolarSystem->isChecked()) return;
    SolarSystem* ssmgr = GETSTELMODULE(SolarSystem);
    ssmgr->setFlagOrbits(checked);
}

void FlybyManager::on_chFullLight_toggled(bool checked)
{
    SolarSystem* ssmgr = GETSTELMODULE(SolarSystem);
    ssmgr->setFlagLighting(!checked);
    QString str =  QString("SolarSystem.setFlagLighting(%0);").arg(!checked);
    StelApp::getInstance().addNetworkCommand(str);
}

//void FlybyManager::on_btnAxis_toggled(bool checked)
//{
   // SolarSystem* ssmgr = GETSTELMODULE(SolarSystem);
   // ssmgr->setFlagAxises(checked);
//}

//void FlybyManager::on_btnEquline_toggled(bool checked)
//{
   // SolarSystem* ssmgr = GETSTELMODULE(SolarSystem);
   // ssmgr->setFlagEquline(checked);
//}
