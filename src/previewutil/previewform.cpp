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

#include <QMessageBox>

#include "previewform.hpp"
#include "ui_previewform.h"

#include "StelApp.hpp"
#include "StelGuiBase.hpp"
#include "StelGui.hpp"
#include "StelCore.hpp"
#include "SkyGui.hpp"
#include "StelTranslator.hpp"
#include "StelFileMgr.hpp"

#include "ui_servermanager.h"
#include "StelMainWindow.hpp"

#include "StelAppGraphicsWidget.hpp"

PreviewForm::PreviewForm(QWidget *parent) :
    QWidget(parent)
{
    ui = new Ui_PreviewForm;
    ui->setupUi(this);
    this->setFocusPolicy(Qt::StrongFocus);
    connect(ui->widgetInform,SIGNAL(visibilityChanged(bool)),this,SLOT(on_widgetInform_visibilityChanged(bool)));
    connect(ui->widgetControls,SIGNAL(visibilityChanged(bool)),this,SLOT(on_widgetControls_visibilityChanged(bool)));

    stellaHideTimer = new QTimer(this);
    stellaHideTimer->setInterval(2000);
    stellaHideTimer->setSingleShot(true);
    connect(stellaHideTimer, SIGNAL(timeout()), this, SLOT(on_stellaHideTimer()));

    resetAllTimer = new QTimer(this);
    resetAllTimer->setInterval(2000);
    resetAllTimer->setSingleShot(true);
    connect(resetAllTimer,SIGNAL(timeout()),this,SLOT(resetAllProc()));
}

PreviewForm::~PreviewForm()
{
    delete ui;
}

void PreviewForm::showEvent ( QShowEvent * event )
{
    if (!StelApp::getInstance().isAdvancedMode)
        ui->widgetAdvanced->hide();
    QSettings* conf = StelApp::getInstance().getSettings();
    ui->btnFadeEffect->blockSignals(true);
    ui->btnFadeEffect->setChecked(conf->value("gui/initial_hide","false").toBool());
    ui->btnFadeEffect->blockSignals(false);
}
void PreviewForm::changeEvent(QEvent *e)
{
    QWidget::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}
void PreviewForm::focusInEvent(QFocusEvent *e)
{
    QWidget::focusInEvent(e);
    StelMainGraphicsView::getInstance().setFocus();
}

void PreviewForm::languageChanged()
{
    //ui->retranslateUi(this);

    ui->stelWindowTitle->setText(q_("Control and Preview Window"));
    ui->groupBox->setTitle(q_("Selected Object Information"));
    ui->btnRealTime->setText(q_("Real Time"));
    ui->btnFadeEffect->setText(q_("Hide All"));
    ui->btnApply->setText(q_("Apply"));
    ui->chFadeEfect->setText(q_("With Fade Effect"));
    ui->btnLock->setText(q_("Lock \n"
                            "Mouse-Key\n"
                            "Client"));
    ui->btnNightView->setText(q_("Night View"));
    ui->btnQuit->setText(q_("Exit"));
    ui->simpleExit->setText(q_("Exit"));
    ui->simpleStellarium->setText(q_("Stellarium"));
    ui->simpleFreeHand->setText(q_("Sky Writer"));
    ui->simpleFisheyeImage->setText(q_("Fisheye"));
    ui->simpleScripts->setText(q_("Advanced\nScripts"));

    ui->simpleFlatMedia->setText(q_("Flat Media"));

    ui->simpleFlyby->setText(q_("Flyby"));

    ui->simpleMovies->setText(q_("Video"));
    ui->simpleMessier->setText(q_("Messiers"));
    ui->simpleConst->setText(q_("Const."));
    ui->btnInform->setToolTip(q_("Show/Hide Information Panel"));
    ui->btnControls->setToolTip(q_("Show/Hide Control Panel"));
    ui->btnStellaHide->setText(q_("Hide Sky"));
    ui->simpleAudio->setText(q_("Audio"));
    ui->btnResetAll->setText(q_("Reset All"));

    ui->testButton->hide();
}
void PreviewForm::on_btnQuit_clicked()
{
    QMessageBox msgBox;
    msgBox.setWindowTitle(q_("Warning!"));
    msgBox.setText(q_("Are you sure want to quit?"));
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    //msgBox.setDefaultButton(QMessageBox::Save);
    msgBox.setWindowModality(Qt::WindowModality());
    //msgBox.setIcon(QMessageBox::Warning);
    msgBox.setWindowFlags(Qt::WindowStaysOnTopHint);

    if(msgBox.exec() ==QMessageBox::Yes)
        StelApp::getInstance().getGui()->quitAll();

}

void PreviewForm::on_btnFadeEffect_toggled(bool toggled)
{
    //Program�n crash olmas� i�in eklendi-kald�r�lacak
    //rsync* r = NULL;
    //r->initClient();
    //

    //#ifndef SHIRAPLAYER_PRE
    //if(StelMainWindow::getInstance().getIsMultiprojector())
    {
        StelApp::getInstance().getCore()->allFaderColor = Vec3f(0,0,0);
        StelApp::getInstance().getCore()->ALLFader.setDuration(2000);
        StelApp::getInstance().getCore()->ALLFader = ui->btnFadeEffect->isChecked();
        StelApp::getInstance().getRsync()->sendInitdata(ui->btnFadeEffect->isChecked());
        StelMainGraphicsView::getInstance().setFocus();
    }
    //#else
    //else
    {
        StelApp::getInstance().addNetworkCommand(QString("core.SetFade(%0);").arg(toggled));
        //        if(!toggled)
        //            StelApp::getInstance().getRsync()->sendChanges(RSYNC_COMMAND_INIT, "fadeoff");
        //        else
        //            StelApp::getInstance().getRsync()->sendChanges(RSYNC_COMMAND_INIT_OFF, "fadeon");
    }

    //#endif
}

void PreviewForm::on_btnNightView_clicked(bool checked)
{
    StelApp::getInstance().setVisionModeNight(checked);
    StelMainGraphicsView::getInstance().setFocus();
}

void PreviewForm::on_btnRealTime_clicked(bool checked)
{
#ifndef SHIRAPLAYER_PRE
    StelApp::getInstance().isLiveMode = true;
#else
    StelApp::getInstance().isLiveMode = checked;
#endif;
    StelMainGraphicsView::getInstance().setFocus();
}

void PreviewForm::on_btnApply_clicked()
{
    //QMessageBox::critical(0,"server",StelApp::getInstance().nc.toText(),0,0);
    if(StelApp::getInstance().nc.toText()!="")
    {
        if (ui->chFadeEfect->checkState())
        {
            StelApp::getInstance().getRsync()->sendChanges(RSYNC_COMMAND_INIT_OFF, "fadeoff");
#ifdef Q_OS_WIN
            Sleep(2000);
#elif defined Q_OS_LINUX
            sleep(2);
#endif
        }
        //QMessageBox::critical(0,"server",StelApp::getInstance().nc.toText()+"@"+QString::number(ui->chFadeEfect->checkState()),0,0);
        StelApp::getInstance().getRsync()->sendChanges(RSYNC_COMMAND_SCRIPT,StelApp::getInstance().nc.toText()+"@"+QString::number(ui->chFadeEfect->checkState()));
        StelApp::getInstance().nc.clear();
        StelMainGraphicsView::getInstance().setFocus();
    }
}

void PreviewForm::on_btnLock_clicked(bool checked)
{    
    StelApp::getInstance().getRsync()->sendChanges(RSYNC_COMMAND_LOCKCLIENT,QString("%1").arg(!checked));
    StelMainGraphicsView::getInstance().setFocus();
}

void PreviewForm::on_btnStellaHide_toggled(bool toggled)
{
    StelApp::getInstance().getCore()->allFaderColor = Vec3f(0,0,0);
    StelApp::getInstance().getCore()->ALLFader.setDuration(2000);
    StelApp::getInstance().getCore()->ALLFader = true;
    //    StelApp::getInstance().getRsync()->sendChanges(RSYNC_COMMAND_STELLA_HIDEALL,
    //                                                   QString("%1").arg(toggled));
    QString str = QString("core.setStellaFade(%0);").arg(toggled);
    StelApp::getInstance().addNetworkCommand(str);
    stellaHideTimer->start();

    ui->simpleConst->setEnabled(!toggled);
    ui->simpleFlyby->setEnabled(!toggled);
    ui->simpleStellarium->setEnabled(!toggled);
    ui->simpleScripts->setEnabled(!toggled);
}

void PreviewForm::on_btnResetAll_clicked()
{
    QMessageBox msgBox;
    msgBox.setWindowTitle(q_("Warning!"));
    msgBox.setText(q_("Do you want to return back to initial settings?"));
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setWindowModality(Qt::WindowModality());
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.setWindowFlags(Qt::WindowStaysOnTopHint);

    if(msgBox.exec() ==QMessageBox::Yes)
    {
        StelApp::getInstance().getCore()->allFaderColor.set(0,0,0);
        StelApp::getInstance().getCore()->ALLFader.setDuration(2000);
        StelApp::getInstance().getCore()->ALLFader = true;

        resetAllTimer->start();
        StelApp::getInstance().getRsync()->sendChanges(RSYNC_COMMAND_RESET_ALL,"");

    }
}

void PreviewForm::setInfoText(QString text)
{
    ui->infolabel->setText(text);
}

void PreviewForm::on_simpleMovies_clicked()
{
    if (StelApp::getInstance().getstartedFlyby() )
    {
        QMessageBox::warning(0,q_("Warning!"),q_("Please complete flyby operations first!"));
        QPushButton* button = (QPushButton*)(sender());
        button->setChecked(false);
        return;
    }
    setButtonChecks(ui->simpleMovies);
}
void PreviewForm::on_simpleStellarium_clicked()
{
    setButtonChecks(ui->simpleStellarium);

}
void PreviewForm::on_simpleFisheyeImage_clicked()
{
    if (StelApp::getInstance().getstartedFlyby() )
    {
        QMessageBox::warning(0,q_("Warning!"),q_("Please complete flyby operations first!"));
        QPushButton* button = (QPushButton*)(sender());
        button->setChecked(false);
        return;
    }
    setButtonChecks(ui->simpleFisheyeImage);
}
void PreviewForm::on_simpleFlatMedia_clicked()
{
    setButtonChecks(ui->simpleFlatMedia);
}
void PreviewForm::on_simpleScripts_clicked()
{
    if (StelApp::getInstance().getstartedFlyby() )
    {
        QMessageBox::warning(0,q_("Warning!"),q_("Please complete flyby operations first!"));
        QPushButton* button = (QPushButton*)(sender());
        button->setChecked(false);
        return;
    }
    setButtonChecks(ui->simpleScripts);
}
void PreviewForm::on_simpleFreeHand_clicked()
{
    setButtonChecks(ui->simpleFreeHand);
}
void PreviewForm::on_simpleFlyby_clicked()
{
    setButtonChecks(ui->simpleFlyby);
}
void PreviewForm::on_simpleMessier_clicked()
{
    setButtonChecks(ui->simpleMessier);
}

void PreviewForm::on_simpleConst_clicked()
{
    setButtonChecks(ui->simpleConst);
}

void PreviewForm::on_simpleAudio_clicked()
{
    setButtonChecks(ui->simpleAudio);
}

void PreviewForm::on_btnInform_clicked()
{
    if(ui->widgetInform->isVisible())
        ui->widgetInform->close();
    else
    {
        ui->widgetInform->setFloating(false);
        ui->widgetInform->show();
    }
}

void PreviewForm::on_btnControls_clicked()
{
    if(ui->widgetControls->isVisible())
        ui->widgetControls->close();
    else
    {
        ui->widgetControls->setFloating(false);
        ui->widgetControls->show();
    }

}

void PreviewForm::on_widgetInform_visibilityChanged(bool visible)
{
    QToolTip::showText( ui->btnInform->mapToGlobal( QPoint( 0, 0 ) ), ui->btnInform->toolTip() );
}

void PreviewForm::on_widgetControls_visibilityChanged(bool visible)
{
    QToolTip::showText( ui->btnControls->mapToGlobal( QPoint( 0, 0 ) ), ui->btnControls->toolTip() );
}

void PreviewForm::on_stellaHideTimer()
{
    StelMainWindow::getInstance().setStellaHide(ui->btnStellaHide->isChecked());
    StelApp::getInstance().getCore()->ALLFader = false;
}

void PreviewForm::resetAllProc()
{
    StelGui* sgui = dynamic_cast<StelGui*>(StelApp::getInstance().getGui());
    sgui->configurationDialog->loadInitOptions();
    StelApp::getInstance().getCore()->ALLFader = false;
    sgui->servermanagerDialog.stellaManager->loadButtonStatus();
}

void PreviewForm::on_testButton_clicked()
{
    //StelAppGraphicsWidget::getInstance().startSaveFrame(true);
}
void PreviewForm::on_simpleExit_clicked()
{
    QMessageBox msgBox;
    msgBox.setWindowTitle(q_("Warning!"));
    msgBox.setText(q_("Are you sure want to quit?"));
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setWindowModality(Qt::WindowModality());
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.setWindowFlags(Qt::WindowStaysOnTopHint);

    if(msgBox.exec() ==QMessageBox::Yes)
    {
        //autosave file varsa silinecek
        QString configName = "autosaveconfig.ini";
        QString configFileFullPath ="";
        try
        {
            configFileFullPath = StelFileMgr::findFile(configName, StelFileMgr::Flags(StelFileMgr::Writable|StelFileMgr::File));
        }
        catch (std::runtime_error& e)
        {
            try
            {
                configFileFullPath = StelFileMgr::findFile(configName, StelFileMgr::New);
            }
            catch (std::runtime_error& e)
            {
            }
        }
        if (StelFileMgr::exists(configFileFullPath))
        {
            qDebug()<<configFileFullPath<<" file exist, deleting...";
            QFile(configFileFullPath).remove();
        }
        else
            qDebug()<<configFileFullPath<<" file not found";
        //---

        StelApp::getInstance().getGui()->quitAll();
    }
}
void PreviewForm::setButtonChecks(QPushButton* sender)
{
    StelGui* sgui = dynamic_cast<StelGui*>(StelApp::getInstance().getGui());
    sgui->servermanagerDialog.getProxy()->setVisible(true);
    QString caseTab = sender->accessibleName();

    sgui->servermanagerDialog.ui->btnSelectFreeHand->setChecked(true);
    sgui->servermanagerDialog.ui->btnDeleteFreeHand->setChecked(false);
    sgui->servermanagerDialog.ui->btnSetFreeHand->setChecked(false);
    StelApp::getInstance().setAllowFreeHandDel(false);
    StelApp::getInstance().setAllowFreeHand(false);

    if (caseTab == "B1")
        sgui->servermanagerDialog.ui->stackedWidget->setCurrentIndex(5);
    else if (caseTab == "B2")
        sgui->servermanagerDialog.ui->stackedWidget->setCurrentIndex(1);
    else if (caseTab == "B3")
        sgui->servermanagerDialog.ui->stackedWidget->setCurrentIndex(2);
    else if (caseTab == "B4")
        sgui->servermanagerDialog.ui->stackedWidget->setCurrentIndex(3);
    else if (caseTab == "B5")
        sgui->servermanagerDialog.ui->stackedWidget->setCurrentIndex(0);
    else if (caseTab == "B6")
        sgui->servermanagerDialog.ui->stackedWidget->setCurrentIndex(6);
    else if (caseTab == "B7")
        sgui->servermanagerDialog.ui->stackedWidget->setCurrentIndex(7);
    else if (caseTab == "B8")
        sgui->servermanagerDialog.ui->stackedWidget->setCurrentIndex(8);
    else if (caseTab == "B9")
        sgui->servermanagerDialog.ui->stackedWidget->setCurrentIndex(9);
    else if (caseTab == "B10")
        sgui->servermanagerDialog.ui->stackedWidget->setCurrentIndex(10);

    sgui->servermanagerDialog.ui->stelWindowTitle->setText(QString(q_("%0-Manager Window")).arg(sender->text().replace("\n"," ")));

    ui->simpleStellarium->setChecked(false);
    ui->simpleMovies->setChecked(false);
    ui->simpleFisheyeImage->setChecked(false);
    ui->simpleFlatMedia->setChecked(false);
    ui->simpleScripts->setChecked(false);
    ui->simpleFreeHand->setChecked(false);
    ui->simpleFlyby->setChecked(false);
    ui->simpleMessier->setChecked(false);
    ui->simpleConst->setChecked(false);
    ui->simpleAudio->setChecked(false);

    sender->setChecked(true);
}
bool clicked = false;
void PreviewForm::mousePressEvent(QMouseEvent *event)
{
    mpos = event->pos();
    clicked = true;
}

void PreviewForm::mouseMoveEvent(QMouseEvent *event)
{
    if (clicked && (event->buttons() && Qt::LeftButton)) {
        QPoint diff = event->pos() - mpos;
        QPoint newpos = this->pos() + diff;

        this->move(newpos);
    }
}

void PreviewForm::mouseReleaseEvent(QMouseEvent *event)
{
    clicked = false;
}
void PreviewForm::setInformationBoxHeight(int value)
{
    ui->widgetInform->setGeometry(0,0,ui->widgetInform->width(),value);
}

