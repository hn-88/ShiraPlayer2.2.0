/*
 * ShiraPlayer(TM)
 * Copyright (C) 2008 Nigel Kerr
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

#include "Dialog.hpp"
#include "DateTimeDialog.hpp"
#include "StelApp.hpp"
#include "StelCore.hpp"
#include "StelLocaleMgr.hpp"
#include "StelNavigator.hpp"
#include "ui_dateTimeDialogGui.h"
#include "StelMainWindow.hpp"
#include "StelMovementMgr.hpp"

#include <QDebug>
#include <QFrame>
#include <QLineEdit>

DateTimeDialog::DateTimeDialog() :
        year(0),
        month(0),
        day(0),
        hour(0),
        minute(0),
        second(0)
{
    ui = new Ui_dateTimeDialogForm;
}

DateTimeDialog::~DateTimeDialog()
{
    delete ui;
    ui=NULL;
}

void DateTimeDialog::createDialogContent()
{
    ui->setupUi(dialog);
    double jd = StelApp::getInstance().getCore()->getNavigator()->getJDay();
    setDateTime(jd + (StelApp::getInstance().getLocaleMgr().getGMTShift(jd)/24.0)); // UTC -> local tz

    connect(ui->closeStelWindow, SIGNAL(clicked()), this, SLOT(close()));

    connect(ui->spinner_year, SIGNAL(valueChanged(int)), this, SLOT(yearChanged(int)));
    connect(ui->spinner_month, SIGNAL(valueChanged(int)), this, SLOT(monthChanged(int)));
    connect(ui->spinner_day, SIGNAL(valueChanged(int)), this, SLOT(dayChanged(int)));
    connect(ui->spinner_hour, SIGNAL(valueChanged(int)), this, SLOT(hourChanged(int)));
    connect(ui->spinner_minute, SIGNAL(valueChanged(int)), this, SLOT(minuteChanged(int)));
    connect(ui->spinner_second, SIGNAL(valueChanged(int)), this, SLOT(secondChanged(int)));

    connect(this, SIGNAL(dateTimeChanged(double)), StelApp::getInstance().getCore()->getNavigator(), SLOT(setJDay(double)));

    //ASAF
    connect(this,SIGNAL(visibleChanged(bool)),this, SLOT(on_visibleChanged(bool)));
    connect(this,SIGNAL(load(bool)),this,SLOT(on_load(bool)));

    connect(ui->comboBox,SIGNAL(currentIndexChanged(int)),this,SLOT(on_combo_currentIndexChanged(int)));
    connect(ui->btnStop,SIGNAL(clicked()),this,SLOT(on_btnStop_clicked()));
    connect(ui->btnForward,SIGNAL(clicked()),this,SLOT(on_btnForward_clicked()));
    connect(ui->btnRewind,SIGNAL(clicked()),this,SLOT(on_btnRewind_clicked()));
    connect(ui->btnNow,SIGNAL(clicked()),this,SLOT(on_btnNow_clicked()));

    setButtonDurum();

}
void DateTimeDialog::prepareAsChild()
{
    ui->DateTimeBar->hide();
}

//! take in values, adjust for calendrical correctness if needed, and push to
//! the widgets and signals
bool DateTimeDialog::valid(int y, int m, int d, int h, int min, int s)
{
    int dy, dm, dd, dh, dmin, ds;

    if ( ! StelUtils::changeDateTimeForRollover(y, m, d, h, min, s, &dy, &dm, &dd, &dh, &dmin, &ds) )
    {
        dy = y;
        dm = m;
        dd = d;
        dh = h;
        dmin = min;
        ds = s;
    }

    year = dy;
    month = dm;
    day = dd;
    hour = dh;
    minute = dmin;
    second = ds;
    pushToWidgets();
    emit dateTimeChanged(newJd());
    //ASAF
    //QMessageBox::information(0,"","",0,0);
    StelApp::getInstance().isTimeCommand = true;
    StelApp::getInstance().addNetworkCommand(QString("core.setDate('%0-%1-%2T%3:%4:%5','local');").arg(year, 4, 10, QLatin1Char('0')).
                                             arg(month,2, 10, QLatin1Char('0')).
                                             arg(day,2, 10, QLatin1Char('0')).
                                             arg(hour,2, 10, QLatin1Char('0')).
                                             arg(minute,2, 10, QLatin1Char('0')).
                                             arg(second,2, 10, QLatin1Char('0')));
    StelApp::getInstance().isTimeCommand = false;
    return true;
}

void DateTimeDialog::languageChanged()
{
    //if (dialog)
    //    ui->retranslateUi(dialog);
    if (dialog)
    {
        ui->stelWindowTitle->setText(q_("Date and Time"));
        ui->closeStelWindow->setText(QString());
        ui->label->setText(q_("/"));
        ui->label_2->setText(q_("/"));
        ui->label_3->setText(q_(":"));
        ui->label_4->setText(q_(":"));
        ui->spinner_second->setPrefix(QString());
        ui->btnRewind->setText(QString());
        ui->btnStop->setText(QString());
        ui->btnForward->setText(QString());
        ui->btnNow->setText(QString());
        ui->label_5->setText(q_("Flow Rate"));
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

    }
}

void DateTimeDialog::styleChanged()
{
    // Nothing for now
}

/************************************************************************
 year slider or dial changed
************************************************************************/

void DateTimeDialog::yearChanged(int newyear)
{
    if ( year != newyear ) {
	valid( newyear, month, day, hour, minute, second );
    }
}
void DateTimeDialog::monthChanged(int newmonth)
{
    if ( month != newmonth ) {
	valid( year, newmonth, day, hour, minute, second );
    }
}
void DateTimeDialog::dayChanged(int newday)
{
    if ( day != newday ) {
	valid( year, month, newday, hour, minute, second );
    }
}
void DateTimeDialog::hourChanged(int newhour)
{
    if ( hour != newhour ) {
	valid( year, month, day, newhour, minute, second );
    }
}
void DateTimeDialog::minuteChanged(int newminute)
{
    if ( minute != newminute ) {
	valid( year, month, day, hour, newminute, second );
    }
}
void DateTimeDialog::secondChanged(int newsecond)
{
    if ( second != newsecond ) {
	valid( year, month, day, hour, minute, newsecond );
    }
}

double DateTimeDialog::newJd()
{
    double jd;
    StelUtils::getJDFromDate(&jd,year, month, day, hour, minute, second);
    jd -= (StelApp::getInstance().getLocaleMgr().getGMTShift(jd)/24.0); // local tz -> UTC
    return jd;
}

void DateTimeDialog::pushToWidgets()
{
    ui->spinner_year->setValue(year);
    ui->spinner_month->setValue(month);
    ui->spinner_day->setValue(day);
    ui->spinner_hour->setValue(hour);
    ui->spinner_minute->setValue(minute);
    ui->spinner_second->setValue(second);
}

/************************************************************************
Send newJd to spinner_*
 ************************************************************************/
void DateTimeDialog::setDateTime(double newJd)
{

    ui->dateTimeBox->setEnabled(!StelApp::getInstance().getCore()->getMovementMgr()->getflagAutoMove());
    ui->DateTimeTrack->setEnabled(!StelApp::getInstance().getCore()->getMovementMgr()->getflagAutoMove());

    newJd += (StelApp::getInstance().getLocaleMgr().getGMTShift(newJd)/24.0); // UTC -> local tz
    StelUtils::getDateFromJulianDay(newJd, &year, &month, &day);
    StelUtils::getTimeFromJulianDay(newJd, &hour, &minute, &second);
    pushToWidgets();
}

void DateTimeDialog::on_visibleChanged(bool b)
{
    StelApp::getInstance().isNetCom = false;
}
void DateTimeDialog::on_load(bool b)
{
    StelApp::getInstance().isNetCom = true;
}

void DateTimeDialog::on_combo_currentIndexChanged(int index)
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

    setButtonDurum();

//    if(StelMainWindow::getInstance().getIsServer())
//    {
//        StelApp::getInstance().getCore()->getNavigator()->firstPass = false;
//        if (StelApp::getInstance().isDiscreteTimeSteps )
//            StelMainWindow::getInstance().isDiscreateTimeFirst = true;
//        else
//            StelMainWindow::getInstance().isDiscreateTimeFirst = false;
//    }
}

void DateTimeDialog::on_btnStop_clicked()
{
    StelApp::getInstance().m_timedirection = 0.;
    on_combo_currentIndexChanged(ui->comboBox->currentIndex());
    setButtonDurum();
}
void DateTimeDialog::on_btnForward_clicked()
{    
    StelApp::getInstance().m_timedirection = 1.;
    on_combo_currentIndexChanged(ui->comboBox->currentIndex());
}
void DateTimeDialog::on_btnRewind_clicked()
{
    StelApp::getInstance().m_timedirection = -1.;
    on_combo_currentIndexChanged(ui->comboBox->currentIndex());
}
void DateTimeDialog::on_btnNow_clicked()
{
    StelApp::getInstance().getCore()->getNavigator()->setTimeNow();
    StelApp::getInstance().isTimeCommand = true;
    StelApp::getInstance().addNetworkCommand("core.setTimeNow();");
    StelApp::getInstance().isTimeCommand = false;
}
void DateTimeDialog::setButtonDurum()
{
    if(StelApp::getInstance().m_timedirection == 0.)
    {
        ui->btnForward->setChecked(false);
        ui->btnRewind->setChecked(false);
        ui->btnStop->setChecked(true);
        return;
    }
    if(StelApp::getInstance().m_timedirection == -1.0)
    {
        ui->btnForward->setChecked(false);
        ui->btnRewind->setChecked(true);
        ui->btnStop->setChecked(false);
        return;
    }
    if(StelApp::getInstance().m_timedirection == 1.0)
    {
        ui->btnForward->setChecked(true);
        ui->btnRewind->setChecked(false);
        ui->btnStop->setChecked(false);
        return;
    }
}

void DateTimeDialog::retranslate()
{
    languageChanged();
}
