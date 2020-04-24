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

#include "Dialog.hpp"
#include "ConfigurationDialog.hpp"
#include "StelMainGraphicsView.hpp"
#include "StelMainWindow.hpp"
#include "ui_configurationDialog.h"
#include "StelAppGraphicsWidget.hpp"
#include "StelApp.hpp"
#include "StelFileMgr.hpp"
#include "StelCore.hpp"
#include "StelLocaleMgr.hpp"
#include "StelProjector.hpp"
#include "StelNavigator.hpp"
#include "StelCore.hpp"
#include "StelMovementMgr.hpp"
#include "StelModuleMgr.hpp"
#include "StelSkyDrawer.hpp"
#include "StelGui.hpp"
#include "StelGuiItems.hpp"
#include "StelLocation.hpp"
#include "LandscapeMgr.hpp"
#include "StelSkyCultureMgr.hpp"
#include "SolarSystem.hpp"
#include "MeteorMgr.hpp"
#include "MilkyWay.hpp"
#include "ConstellationMgr.hpp"
#include "StarMgr.hpp"
#include "NebulaMgr.hpp"
#include "GridLinesMgr.hpp"
#include "StelScriptMgr.hpp"
#include "LabelMgr.hpp"
#include "ScreenImageMgr.hpp"
#include "SkyGui.hpp"
#include "StelJsonParser.hpp"
#include "StelIniParser.hpp"

#include <QSettings>
#include <QDebug>
#include <QFile>
#include <QFileDialog>

//ASAF
#include "socketutils/rsync.h"

ConfigurationDialog::ConfigurationDialog(StelGui* agui) :
    starCatalogDownloadReply(NULL),
    currentDownloadFile(NULL),
    progressBar(NULL),
    gui(agui)
{
    ui = new Ui_configurationDialogForm;
    hasDownloadedStarCatalog = false;
    QString platform = StelUtils::getOperatingSystemInfo();
    userAgent = QString("Stellarium/%1 (%2)").arg(10.6).arg(platform);
}

ConfigurationDialog::~ConfigurationDialog()
{
    delete ui;
}

void ConfigurationDialog::languageChanged()
{    
    //if (dialog)
    //  ui->retranslateUi(dialog);
    if (dialog)
    {
        ui->stelWindowTitle->setText(q_("Configuration"));
        ui->groupBox_3->setTitle(q_("Settings"));
        ui->label_7->setText(q_("Program Language"));
        ui->label_3->setText(q_("Fulldome File Path :"));
        ui->label_4->setText(q_("Flat Media Path :"));
        ui->label_8->setText(q_("Projector Screen "));
        ui->Left->setText(q_("Left :"));
        ui->label_5->setText(q_("Top :"));
        ui->Left_2->setText(q_("Width :"));
        ui->label_6->setText(q_("Height :"));
        ui->label_9->setText(q_("Preview Screen"));
        ui->Left_3->setText(q_("Width :"));
        ui->label_10->setText(q_("Height :"));
        ui->label_11->setText(q_("Console Screen"));
        ui->label_12->setText(q_("Height :"));
        ui->groupBox_4->setTitle(q_("Selected object information"));
        ui->noSelectedInfoRadio->setToolTip(q_("Display no information"));
        ui->noSelectedInfoRadio->setText(q_("None"));
        ui->briefSelectedInfoRadio->setToolTip(q_("Display less information"));
        ui->briefSelectedInfoRadio->setText(q_("Short"));
        ui->allSelectedInfoRadio->setToolTip(q_("Display all information available"));
        ui->allSelectedInfoRadio->setText(q_("All available"));
        ui->groupBox_6->setTitle(q_("Default options"));
        ui->setViewingOptionAsDefaultPushButton->setWhatsThis(q_("Save the settings you've changed this session to be the same the next time you start Stellarium"));
        ui->setViewingOptionAsDefaultPushButton->setText(q_("Save settings"));
        ui->restoreDefaultsButton->setWhatsThis(q_("Restore the default settings that came with Stellarium"));
        ui->restoreDefaultsButton->setText(q_("Restore defaults"));
        ui->saveDefaultOptionsLabel->setText(q_("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
                                                "<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">\n"
                                                "p, li { white-space: pre-wrap; }\n"
                                                "</style></head><body style=\" font-family:'MS Shell Dlg 2'; font-size:8.25pt; font-weight:400; font-style:normal;\">\n"
                                                "<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-size:8pt;\">Restoring default settings requires a restart of ShiraPlayer. Saving all the current options for use at next startup.</span></p></body></html>"));
        ui->startupFOVLabel->setStatusTip(q_("The width of your view when Stellarium starts"));
        ui->startupFOVLabel->setText(q_("Startup FOV: XX"));
        ui->startupDirectionOfViewlabel->setStatusTip(q_("The direction you're looking when Stellarium starts"));
        ui->startupDirectionOfViewlabel->setText(q_("Startup direction of view: xxxx"));
        ui->groupBox_5->setTitle(q_("Control"));
        ui->enableMouseNavigationCheckBox->setToolTip(q_("Allow mouse to pan (drag) and zoom (mousewheel)"));
        ui->enableMouseNavigationCheckBox->setText(q_("Enable mouse navigation"));
        ui->enableKeysNavigationCheckBox->setToolTip(q_("Allow keyboard to pan and zoom"));
        ui->enableKeysNavigationCheckBox->setText(q_("Enable keyboard navigation"));
        ui->groupBox->setTitle(q_("Startup date and time"));
        ui->systemTimeRadio->setToolTip(q_("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
                                           "<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">\n"
                                           "p, li { white-space: pre-wrap; }\n"
                                           "</style></head><body style=\" font-family:'MS Shell Dlg 2'; font-size:8.25pt; font-weight:400; font-style:normal;\">\n"
                                           "<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-size:8pt;\">Starts ShiraPlayer at system clock date and time</span></p></body></html>"));
        ui->systemTimeRadio->setText(q_("System date and time"));
        ui->todayRadio->setToolTip(q_("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
                                      "<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">\n"
                                      "p, li { white-space: pre-wrap; }\n"
                                      "</style></head><body style=\" font-family:'MS Shell Dlg 2'; font-size:8.25pt; font-weight:400; font-style:normal;\">\n"
                                      "<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-size:8pt;\">Sets the simulation time to the next instance of this time of day when ShiraPlayer starts</span></p></body></html>"));
        ui->todayRadio->setText(q_("System date at:"));
        ui->fixedTimeRadio->setToolTip(q_("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
                                          "<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">\n"
                                          "p, li { white-space: pre-wrap; }\n"
                                          "</style></head><body style=\" font-family:'MS Shell Dlg 2'; font-size:8.25pt; font-weight:400; font-style:normal;\">\n"
                                          "<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-size:8pt;\">Use a specific date and time when ShiraPlayer starts up</span></p></body></html>"));
        ui->fixedTimeRadio->setText(q_("Other:"));
        ui->fixedDateTimeCurrentButton->setText(q_("use current"));
        ui->groupBox_9->setTitle(q_("Other"));
        ui->showFlipButtonsCheckbox->setToolTip(q_("Toggle vertical and horizontal image flip buttons."));
        ui->showFlipButtonsCheckbox->setText(q_("Show flip buttons"));
        ui->mouseTimeoutCheckbox->setToolTip(q_("Hides the mouse cursor when inactive"));
        ui->mouseTimeoutCheckbox->setText(q_("Mouse cursor timeout (seconds):"));
        ui->contentBox->setTitle(q_("Star catalog updates"));
        ui->getStarsButton->setToolTip(q_("Click here to start downloading"));
        //ui->getStarsButton->setText(q_("Get catalog x of y"));
        ui->getStarsButton->setDescription(q_("Download this file to view even more stars"));
        //ui->downloadLabel->setText(q_("xxx"));
        ui->downloadRetryButton->setToolTip(q_("Restart the download"));
        ui->downloadRetryButton->setText(q_("Retry"));
        ui->downloadCancelButton->setToolTip(q_("Stop the download. You can always restart it later"));
        ui->downloadCancelButton->setText(q_("Cancel"));
        ui->groupBox_7->setTitle(q_("Planetarium options"));
        ui->groupBox_10->setTitle(q_("Spherical mirror distortion"));
        ui->rdStellaMirror->setText(q_("Stellarium base distortion"));
        ui->rdCustomMirror->setText(q_("Use Custom data file "));
        ui->Filename->setText(q_("Custom File Name"));
        ui->btnOpenMirrorData->setToolTip(q_("Open Flat Media project file"));
        ui->btnOpenMirrorData->setText(QString());
        ui->label_2->setText(q_("Custom data texture Mirror"));
        ui->chVertical->setText(q_("Vertical"));
        ui->chHorizontal->setText(q_("Horizontal"));
        ui->sphericMirrorCheckbox->setToolTip(q_("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
                                                 "<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">\n"
                                                 "p, li { white-space: pre-wrap; }\n"
                                                 "</style></head><body style=\" font-family:'MS Shell Dlg 2'; font-size:8.25pt; font-weight:400; font-style:normal;\">\n"
                                                 "<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-size:8pt;\">Spheric mirror distortion is used when projecting ShiraPlayer onto a spheric mirror for low-cost planetarium systems.</span></p></body></html>"));
        ui->sphericMirrorCheckbox->setText(q_("Apply Spheric mirror distortion"));
        ui->groupBoxFineTune->setTitle(q_("Fine-tune the distortion"));
        ui->chEnableTune->setText(q_("Configuration"));
        ui->btnResetWarp->setText(q_("Reset"));
        ui->btnSaveFineTune->setText(q_("Save"));
        ui->btnDelFineTune->setText(q_("Delete"));
        ui->autoZoomResetsDirectionCheckbox->setToolTip(q_("When enabled, the \"auto zoom out\" key will also set the initial viewing direction"));
        ui->autoZoomResetsDirectionCheckbox->setText(q_("Auto zoom out returns to initial direction of view"));
        ui->gravityLabelCheckbox->setToolTip(q_("Align labels with the horizon"));
        ui->gravityLabelCheckbox->setText(q_("Gravity labels"));
        ui->diskViewportCheckbox->setToolTip(q_("Mask out everything outside a central circle in the main view"));
        ui->diskViewportCheckbox->setText(q_("Disc viewport"));
        ui->selectSingleConstellationButton->setStatusTip(q_("Hide other constellations when you click one"));
        ui->selectSingleConstellationButton->setText(q_("Select single constellation"));
        ui->grpScreenShots->setTitle(q_("Screenshots"));
        ui->invertScreenShotColorsCheckBox->setText(q_("Invert colors"));
        ui->label->setText(q_("Screenshot Directory"));
        ui->screenshotBrowseButton->setText(QString());
        ui->groupBox_8->setTitle(q_("Options"));
        ui->scriptStatusLabel->setText(QString());
        ui->closeWindowAtScriptRunCheckbox->setText(q_("Close window when script runs"));
        ui->runScriptButton->setToolTip(q_("Run the selected script"));
        ui->runScriptButton->setText(QString());
        ui->stopScriptButton->setToolTip(q_("Stop a running script"));
        ui->stopScriptButton->setText(QString());
        ui->pluginsGroupBox->setTitle(q_("Options"));
        ui->pluginLoadAtStartupCheckBox->setText(q_("Load at startup"));
        ui->pluginConfigureButton->setText(q_("configure"));

        QListWidgetItem *___qlistwidgetitem = ui->stackListWidget->item(0);
        ___qlistwidgetitem->setText(q_("Main"));
        QListWidgetItem *___qlistwidgetitem1 = ui->stackListWidget->item(1);
        ___qlistwidgetitem1->setText(q_("Navigation"));
        QListWidgetItem *___qlistwidgetitem2 = ui->stackListWidget->item(2);
        ___qlistwidgetitem2->setText(q_("Tools"));
        QListWidgetItem *___qlistwidgetitem3 = ui->stackListWidget->item(3);
        ___qlistwidgetitem3->setText(q_("Scripts"));
        QListWidgetItem *___qlistwidgetitem4 = ui->stackListWidget->item(4);
        ___qlistwidgetitem4->setText(q_("Plugins"));
    }
}

void ConfigurationDialog::styleChanged()
{
    // Nothing for now
}

void ConfigurationDialog::createDialogContent()
{
    const StelProjectorP proj = StelApp::getInstance().getCore()->getProjection(Mat4d());
    StelNavigator* nav = StelApp::getInstance().getCore()->getNavigator();
    StelMovementMgr* mvmgr = GETSTELMODULE(StelMovementMgr);

    ui->setupUi(dialog);

    // Set the main tab activated by default
    ui->configurationStackedWidget->setCurrentIndex(0);
    ui->stackListWidget->setCurrentRow(0);

    connect(ui->closeStelWindow, SIGNAL(clicked()), this, SLOT(close()));

    // Main tab
    // Fill the language list widget from the available list
    QString appLang = StelApp::getInstance().getLocaleMgr().getAppLanguage();
    QComboBox* cb = ui->programLanguageComboBox;
    cb->clear();
    cb->addItems(StelTranslator::globalTranslator.getAvailableLanguagesNamesNative(StelFileMgr::getLocaleDir()));
    QString l2 = StelTranslator::iso639_1CodeToNativeName(appLang);
    int lt = cb->findText(l2, Qt::MatchExactly);
    if (lt == -1 && appLang.contains('_'))
    {
        l2 = appLang.left(appLang.indexOf('_'));
        l2=StelTranslator::iso639_1CodeToNativeName(l2);
        lt = cb->findText(l2, Qt::MatchExactly);
    }
    if (lt!=-1)
        cb->setCurrentIndex(lt);
    connect(cb, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(languageChanged(const QString&)));

    connect(ui->getStarsButton, SIGNAL(clicked()), this, SLOT(downloadStars()));
    connect(ui->downloadCancelButton, SIGNAL(clicked()), this, SLOT(cancelDownload()));
    connect(ui->downloadRetryButton, SIGNAL(clicked()), this, SLOT(downloadStars()));
    refreshStarCatalogButton();

    // Selected object info
    if (gui->getInfoTextFilters() == (StelObject::InfoStringGroup)0)
        ui->noSelectedInfoRadio->setChecked(true);
    else if (gui->getInfoTextFilters() == StelObject::InfoStringGroup(StelObject::ShortInfo))
        ui->briefSelectedInfoRadio->setChecked(true);
    else
        ui->allSelectedInfoRadio->setChecked(true);
    connect(ui->noSelectedInfoRadio, SIGNAL(released()), this, SLOT(setNoSelectedInfo()));
    connect(ui->allSelectedInfoRadio, SIGNAL(released()), this, SLOT(setAllSelectedInfo()));
    connect(ui->briefSelectedInfoRadio, SIGNAL(released()), this, SLOT(setBriefSelectedInfo()));

    // Navigation tab
    // Startup time
    if (nav->getStartupTimeMode()=="actual")
        ui->systemTimeRadio->setChecked(true);
    else if (nav->getStartupTimeMode()=="today")
        ui->todayRadio->setChecked(true);
    else
        ui->fixedTimeRadio->setChecked(true);
    connect(ui->systemTimeRadio, SIGNAL(clicked(bool)), this, SLOT(setStartupTimeMode()));
    connect(ui->todayRadio, SIGNAL(clicked(bool)), this, SLOT(setStartupTimeMode()));
    connect(ui->fixedTimeRadio, SIGNAL(clicked(bool)), this, SLOT(setStartupTimeMode()));

    ui->todayTimeSpinBox->setTime(nav->getInitTodayTime());
    connect(ui->todayTimeSpinBox, SIGNAL(timeChanged(QTime)), nav, SLOT(setInitTodayTime(QTime)));
    ui->fixedDateTimeEdit->setDateTime(StelUtils::jdToQDateTime(nav->getPresetSkyTime()));
    connect(ui->fixedDateTimeEdit, SIGNAL(dateTimeChanged(QDateTime)), nav, SLOT(setPresetSkyTime(QDateTime)));

    ui->enableKeysNavigationCheckBox->setChecked(mvmgr->getFlagEnableMoveKeys() || mvmgr->getFlagEnableZoomKeys());
    ui->enableMouseNavigationCheckBox->setChecked(mvmgr->getFlagEnableMouseNavigation());
    connect(ui->enableKeysNavigationCheckBox, SIGNAL(toggled(bool)), mvmgr, SLOT(setFlagEnableMoveKeys(bool)));
    connect(ui->enableKeysNavigationCheckBox, SIGNAL(toggled(bool)), mvmgr, SLOT(setFlagEnableZoomKeys(bool)));
    connect(ui->enableMouseNavigationCheckBox, SIGNAL(toggled(bool)), mvmgr, SLOT(setFlagEnableMouseNavigation(bool)));
    connect(ui->fixedDateTimeCurrentButton, SIGNAL(clicked()), this, SLOT(setFixedDateTimeToCurrent()));

    QSettings* conf = StelApp::getInstance().getSettings();
    ui->sphericMirrorCheckbox->setChecked(conf->value("video/viewport_effect", "none").toString() == "sphericMirrorDistorter");

    ui->chApplyFine->setChecked(conf->value("video/apply_finetune",false).toBool());

    // Tools tab
    ConstellationMgr* cmgr = GETSTELMODULE(ConstellationMgr);
    Q_ASSERT(cmgr);
    connect(ui->sphericMirrorCheckbox, SIGNAL(toggled(bool)), this, SLOT(setSphericMirror(bool)));
    ui->txtCustomDataFile->setEnabled(!ui->sphericMirrorCheckbox->isChecked());
    ui->btnOpenMirrorData->setEnabled(!ui->sphericMirrorCheckbox->isChecked());
    ui->rdStellaMirror->setEnabled(!ui->sphericMirrorCheckbox->isChecked());
    ui->rdCustomMirror->setEnabled(!ui->sphericMirrorCheckbox->isChecked());
    ui->chVertical->setEnabled(!ui->sphericMirrorCheckbox->isChecked());
    ui->chHorizontal->setEnabled(!ui->sphericMirrorCheckbox->isChecked());
    connect(ui->btnOpenMirrorData, SIGNAL(clicked()),this,SLOT(on_btnOpenMirrorData_clicked()));
    //--Server için özel oldu
    //ASAF

    QString custom_distortion_file = conf->value("spheric_mirror/custom_distortion_file","").toString();
    ui->txtCustomDataFile->setText(custom_distortion_file);
    ui->rdCustomMirror->setChecked(conf->value("spheric_mirror/use_custom_data",false).toBool());
    ui->chHorizontal->setChecked(conf->value("spheric_mirror/custom_dist_horz_mirror",false).toBool());
    ui->chVertical->setChecked(conf->value("spheric_mirror/custom_dist_vert_mirror",false).toBool());

    //--
    ui->gravityLabelCheckbox->setChecked(proj->getFlagGravityLabels());
    connect(ui->gravityLabelCheckbox, SIGNAL(toggled(bool)), this, SLOT(setFlagGravityLabels(bool)));
    ui->selectSingleConstellationButton->setChecked(cmgr->getFlagIsolateSelected());
    connect(ui->selectSingleConstellationButton, SIGNAL(toggled(bool)), cmgr, SLOT(setFlagIsolateSelected(bool)));
    ui->diskViewportCheckbox->setChecked(proj->getMaskType() == StelProjector::MaskDisk);
    connect(ui->diskViewportCheckbox, SIGNAL(toggled(bool)), this, SLOT(setDiskViewport(bool)));
    ui->autoZoomResetsDirectionCheckbox->setChecked(mvmgr->getFlagAutoZoomOutResetsDirection());
    connect(ui->autoZoomResetsDirectionCheckbox, SIGNAL(toggled(bool)), mvmgr, SLOT(setFlagAutoZoomOutResetsDirection(bool)));

    ui->showFlipButtonsCheckbox->setChecked(gui->getFlagShowFlipButtons());
    connect(ui->showFlipButtonsCheckbox, SIGNAL(toggled(bool)), gui, SLOT(setFlagShowFlipButtons(bool)));

    ui->mouseTimeoutCheckbox->setChecked(StelMainGraphicsView::getInstance().getFlagCursorTimeout());
    ui->mouseTimeoutSpinBox->setValue(StelMainGraphicsView::getInstance().getCursorTimeout());
    connect(ui->mouseTimeoutCheckbox, SIGNAL(clicked()), this, SLOT(cursorTimeOutChanged()));
    connect(ui->mouseTimeoutCheckbox, SIGNAL(toggled(bool)), this, SLOT(cursorTimeOutChanged()));
    connect(ui->mouseTimeoutSpinBox, SIGNAL(valueChanged(double)), this, SLOT(cursorTimeOutChanged(double)));

    connect(ui->setViewingOptionAsDefaultPushButton, SIGNAL(clicked()), this, SLOT(saveCurrentViewOptions()));
    connect(ui->restoreDefaultsButton, SIGNAL(clicked()), this, SLOT(setDefaultViewOptions()));

    ui->screenshotDirEdit->setText(StelFileMgr::getScreenshotDir());
    connect(ui->screenshotDirEdit, SIGNAL(textChanged(QString)), this, SLOT(selectScreenshotDir(QString)));
    connect(ui->screenshotBrowseButton, SIGNAL(clicked()), this, SLOT(browseForScreenshotDir()));

    ui->invertScreenShotColorsCheckBox->setChecked(StelMainGraphicsView::getInstance().getFlagInvertScreenShotColors());
    connect(ui->invertScreenShotColorsCheckBox, SIGNAL(toggled(bool)), &StelMainGraphicsView::getInstance(), SLOT(setFlagInvertScreenShotColors(bool)));

    // script tab controls
    StelScriptMgr& scriptMgr = StelMainGraphicsView::getInstance().getScriptMgr();
    connect(ui->scriptListWidget, SIGNAL(currentTextChanged(const QString&)), this, SLOT(scriptSelectionChanged(const QString&)));
    connect(ui->runScriptButton, SIGNAL(clicked()), this, SLOT(runScriptClicked()));
    connect(ui->stopScriptButton, SIGNAL(clicked()), this, SLOT(stopScriptClicked()));
    if (scriptMgr.scriptIsRunning())
        aScriptIsRunning();
    else
        aScriptHasStopped();
    connect(&scriptMgr, SIGNAL(scriptRunning()), this, SLOT(aScriptIsRunning()));
    connect(&scriptMgr, SIGNAL(scriptStopped()), this, SLOT(aScriptHasStopped()));
    ui->scriptListWidget->setSortingEnabled(true);
    populateScriptsList();
    connect(this, SIGNAL(visibleChanged(bool)), this, SLOT(populateScriptsList()));

    // plugins control
    connect(ui->pluginsListWidget, SIGNAL(currentTextChanged(const QString&)), this, SLOT(pluginsSelectionChanged(const QString&)));
    connect(ui->pluginLoadAtStartupCheckBox, SIGNAL(stateChanged(int)), this, SLOT(loadAtStartupChanged(int)));
    connect(ui->pluginConfigureButton, SIGNAL(clicked()), this, SLOT(pluginConfigureCurrentSelection()));
    populatePluginsList();


    connect(ui->stackListWidget, SIGNAL(currentItemChanged(QListWidgetItem *, QListWidgetItem *)), this, SLOT(changePage(QListWidgetItem *, QListWidgetItem*)));
    updateConfigLabels();

    ui->grpScreenShots->hide();


    ui->txtFulldomePath->setText(conf->value("main/media_path","C:/").toString());
    ui->txtFlatPath->setText(conf->value("main/flat_media_path", "C:/").toString());
    ui->spScreenLeft->setValue(conf->value("video/screen_l", 720).toInt());
    ui->spScreenTop->setValue(conf->value("video/screen_t").toInt());
    ui->spScreenWidth->setValue(conf->value("video/screen_w", 800).toInt());
    ui->spScreenHeight->setValue(conf->value("video/screen_h", 600).toInt());
    ui->spPreviewWidth->setValue(conf->value("video/preview_w",512).toInt());
    ui->spPreviewHeight->setValue(conf->value("video/preview_h",600).toInt());
    ui->spConsoleHeight->setValue(conf->value("video/console_h",720).toInt());

    connect(ui->btnSelectFulldomePath, SIGNAL(clicked()),this,SLOT(on_btnSelectFulldomePath_clicked()));
    connect(ui->btnSelectFlatPath, SIGNAL(clicked()),this,SLOT(on_btnSelectFlatPath_clicked()));


    //gizlendi
    ui->groupBox_9->hide();
    ui->grpScreenShots->hide();

    connect(ui->chEnableTune,SIGNAL(toggled(bool)),this,SLOT(on_chEnableTune_checked(bool)));
    connect(ui->btnResetWarp, SIGNAL(clicked()), this, SLOT(on_resetWarp_Clicked()));
    connect(ui->btnSaveFineTune,SIGNAL(clicked()),this,SLOT(on_btnSaveFineTune_Clicked()));
    connect(ui->btnDelFineTune,SIGNAL(clicked()),this,SLOT(on_btnDelFineTune_Clicked()));

    ui->chInitialView->setChecked(conf->value("navigation/flag_start_initview",false).toBool());
    connect(ui->chInitialView,SIGNAL(toggled(bool)),this,SLOT(on_chInitialView_toggled(bool)));

}
void ConfigurationDialog::prepareAsChild()
{
    ui->LocationBar->hide();
}
void ConfigurationDialog::languageChanged(const QString& langName)
{
    QString code = StelTranslator::nativeNameToIso639_1Code(langName);
    StelApp::getInstance().getLocaleMgr().setAppLanguage(code);
    StelApp::getInstance().getLocaleMgr().setSkyLanguage(code);
    updateConfigLabels();
    StelMainWindow::getInstance().initTitleI18n();

}

void ConfigurationDialog::setStartupTimeMode(void)
{
    StelNavigator* nav = StelApp::getInstance().getCore()->getNavigator();
    Q_ASSERT(nav);

    if (ui->systemTimeRadio->isChecked())
        StelApp::getInstance().getCore()->getNavigator()->setStartupTimeMode("actual");
    else if (ui->todayRadio->isChecked())
        StelApp::getInstance().getCore()->getNavigator()->setStartupTimeMode("today");
    else
        StelApp::getInstance().getCore()->getNavigator()->setStartupTimeMode("preset");

    nav->setInitTodayTime(ui->todayTimeSpinBox->time());
    nav->setPresetSkyTime(ui->fixedDateTimeEdit->dateTime());
}

void ConfigurationDialog::setDiskViewport(bool b)
{
    if (b)
        StelApp::getInstance().getCore()->setMaskType(StelProjector::MaskDisk);
    else
        StelApp::getInstance().getCore()->setMaskType(StelProjector::MaskNone);

    //Client
    StelApp::getInstance().getRsync()->sendChanges(RSYNC_COMMAND_DISKVIEWPORT,QString("%1").arg(b));
    //
}

void ConfigurationDialog::setSphericMirror(bool b)
{
    if(!StelMainWindow::getInstance().isServer)
    {
        StelCore* core = StelApp::getInstance().getCore();
        if (b)
        {
            savedProjectionType = core->getCurrentProjectionType();
            core->setCurrentProjectionType(StelCore::ProjectionFisheye);
            StelMainGraphicsView::getInstance().getStelAppGraphicsWidget()->setViewportEffect("sphericMirrorDistorter");
        }
        else
        {
            core->setCurrentProjectionType((StelCore::ProjectionType)savedProjectionType);
            StelMainGraphicsView::getInstance().getStelAppGraphicsWidget()->setViewportEffect("none");
        }
    }
    else
    {
        ui->txtCustomDataFile->setEnabled(!b);
        ui->btnOpenMirrorData->setEnabled(!b);
        ui->rdStellaMirror->setEnabled(!b);
        ui->rdCustomMirror->setEnabled(!b);
        ui->chVertical->setEnabled(!b);
        ui->chHorizontal->setEnabled(!b);

        //Send Client command
        StelApp::getInstance().getRsync()->sendChanges(RSYNC_COMMAND_SPHERICMIRROR,QString("%1@%2@%3@%4@%5").arg(b)
                                                       .arg(ui->rdCustomMirror->isChecked())
                                                       .arg(ui->txtCustomDataFile->text())
                                                       .arg(ui->chHorizontal->isChecked())
                                                       .arg(ui->chVertical->isChecked()));
        //
        //

        QSettings* conf = StelApp::getInstance().getSettings();
        if(b)
            conf->setValue("video/viewport_effect","sphericMirrorDistorter");
        else
            conf->setValue("video/viewport_effect","none");
        conf->setValue("spheric_mirror/custom_distortion_file", ui->txtCustomDataFile->text());
        conf->setValue("spheric_mirror/use_custom_data", ui->rdCustomMirror->isChecked());
        conf->setValue("spheric_mirror/custom_dist_horz_mirror", ui->chHorizontal->isChecked());
        conf->setValue("spheric_mirror/custom_dist_vert_mirror", ui->chVertical->isChecked());

    }
}
void ConfigurationDialog::setFlagGravityLabels(bool b)
{
    StelApp::getInstance().getCore()->setFlagGravityLabels(b);
    //Client
    StelApp::getInstance().getRsync()->sendChanges(RSYNC_COMMAND_SETGRAVITYLABELS,QString("%0").arg(b));
    //
}
void ConfigurationDialog::setNoSelectedInfo(void)
{
    gui->setInfoTextFilters(StelObject::InfoStringGroup(0));
}

void ConfigurationDialog::setAllSelectedInfo(void)
{
    gui->setInfoTextFilters(StelObject::InfoStringGroup(StelObject::AllInfo));
}

void ConfigurationDialog::setBriefSelectedInfo(void)
{
    gui->setInfoTextFilters(StelObject::InfoStringGroup(StelObject::ShortInfo));
}

void ConfigurationDialog::cursorTimeOutChanged()
{
    StelMainGraphicsView::getInstance().setFlagCursorTimeout(ui->mouseTimeoutCheckbox->isChecked());
    StelMainGraphicsView::getInstance().setCursorTimeout(ui->mouseTimeoutSpinBox->value());
}

void ConfigurationDialog::browseForScreenshotDir()
{
    QString oldScreenshorDir = StelFileMgr::getScreenshotDir();
    QString newScreenshotDir = QFileDialog::getExistingDirectory(NULL, q_("Select screenshot directory"), oldScreenshorDir, QFileDialog::ShowDirsOnly);

    if (!newScreenshotDir.isEmpty()) {
        // remove trailing slash
        if (newScreenshotDir.right(1) == "/")
            newScreenshotDir = newScreenshotDir.left(newScreenshotDir.length()-1);

        ui->screenshotDirEdit->setText(newScreenshotDir);
    }
}

void ConfigurationDialog::selectScreenshotDir(const QString& dir)
{
    try
    {
        StelFileMgr::setScreenshotDir(dir);
    }
    catch (std::runtime_error& e)
    {
        // nop
        // this will happen when people are only half way through typing dirs
    }
}

// Save the current viewing option including landscape, location and sky culture
// This doesn't include the current viewing direction, time and FOV since those have specific controls
void ConfigurationDialog::saveCurrentViewOptions()
{
    QMessageBox msgBox;
    msgBox.setWindowTitle(q_("Warning!"));
    msgBox.setText(q_("Do you want to save all view options?"));
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setWindowFlags(Qt::WindowStaysOnTopHint);
    msgBox.setIcon(QMessageBox::Warning);
    int ret = msgBox.exec();

    if(ret ==QMessageBox::Yes)
    {
        saveCurrentOptions(false);
        updateConfigLabels();
    }
}
void ConfigurationDialog::loadInitOptions()
{
    StelApp::getInstance().LoadInitOptions();
}

void ConfigurationDialog::saveCurrentOptions(bool autosave )
{
    QSettings* conf = StelApp::getInstance().getSettings();
    Q_ASSERT(conf);

    if (autosave)
    {
        QHash<QString, QVariant> hash;
        const QStringList keys = conf->allKeys();
        Q_FOREACH(QString key, keys) {
          hash[key] = conf->value(key);
        }

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
                qFatal("Could not create auto save configuration file %s.", qPrintable(configName));
            }
        }
        conf = new QSettings(configFileFullPath, StelIniFormat, NULL);
        //Öncelikle Normal config.ini dosyasýnýn içeriði kopyalanacak
        Q_FOREACH(QString key, keys) {
          conf->setValue(key,hash[key]);
        }
    }

    if ((StelMainWindow::getInstance().getIsServer()) && (!autosave))
        StelApp::getInstance().getRsync()->sendChanges(RSYNC_COMMAND_SAVESET,"");


    LandscapeMgr* lmgr = GETSTELMODULE(LandscapeMgr);
    Q_ASSERT(lmgr);
    SolarSystem* ssmgr = GETSTELMODULE(SolarSystem);
    Q_ASSERT(ssmgr);
    MeteorMgr* mmgr = GETSTELMODULE(MeteorMgr);
    Q_ASSERT(mmgr);
    StelSkyDrawer* skyd = StelApp::getInstance().getCore()->getSkyDrawer();
    Q_ASSERT(skyd);
    ConstellationMgr* cmgr = GETSTELMODULE(ConstellationMgr);
    Q_ASSERT(cmgr);
    StarMgr* smgr = GETSTELMODULE(StarMgr);
    Q_ASSERT(smgr);
    NebulaMgr* nmgr = GETSTELMODULE(NebulaMgr);
    Q_ASSERT(nmgr);
    GridLinesMgr* glmgr = GETSTELMODULE(GridLinesMgr);
    Q_ASSERT(glmgr);
    StelMovementMgr* mvmgr = GETSTELMODULE(StelMovementMgr);
    Q_ASSERT(mvmgr);
    StelNavigator* nav = StelApp::getInstance().getCore()->getNavigator();
    Q_ASSERT(nav);
    const StelProjectorP proj = StelApp::getInstance().getCore()->getProjection(Mat4d());
    Q_ASSERT(proj);
    MilkyWay* mway = GETSTELMODULE(MilkyWay);
    Q_ASSERT(mway);

    // view dialog / sky tab settings
    conf->setValue("stars/absolute_scale", skyd->getAbsoluteStarScale());
    conf->setValue("stars/relative_scale", skyd->getRelativeStarScale());
    conf->setValue("stars/flag_star_twinkle", skyd->getFlagTwinkle());
    conf->setValue("stars/star_twinkle_amount", skyd->getTwinkleAmount());
    conf->setValue("viewing/use_luminance_adaptation", skyd->getFlagLuminanceAdaptation());
    conf->setValue("astro/flag_planets", ssmgr->getFlagPlanets());
    conf->setValue("astro/flag_planets_hints", ssmgr->getFlagHints());
    conf->setValue("astro/flag_planets_orbits", ssmgr->getFlagOrbits());
    conf->setValue("astro/flag_light_travel_time", ssmgr->getFlagLightTravelTime());
    conf->setValue("viewing/flag_moon_scaled", ssmgr->getFlagMoonScale());
    conf->setValue("astro/meteor_rate", mmgr->getZHR());
    conf->setValue("astro/milky_way_intensity",mway->getIntensity());
    conf->setValue("astro/flag_milky_way",mway->getFlagShow());


    // view dialog / markings tab settings
    conf->setValue("viewing/flag_azimuthal_grid", glmgr->getFlagAzimuthalGrid());
    conf->setValue("viewing/flag_equatorial_grid", glmgr->getFlagEquatorGrid());
    conf->setValue("viewing/flag_equator_line", glmgr->getFlagEquatorLine());
    conf->setValue("viewing/flag_ecliptic_line", glmgr->getFlagEclipticLine());
    conf->setValue("viewing/flag_meridian_line", glmgr->getFlagMeridianLine());
    conf->setValue("viewing/flag_equatorial_J2000_grid", glmgr->getFlagEquatorJ2000Grid());
    conf->setValue("viewing/flag_cardinal_points", lmgr->getFlagCardinalsPoints());
    conf->setValue("viewing/flag_constellation_drawing", cmgr->getFlagLines());
    conf->setValue("viewing/flag_constellation_name", cmgr->getFlagLabels());
    conf->setValue("viewing/flag_constellation_boundaries", cmgr->getFlagBoundaries());
    conf->setValue("viewing/flag_constellation_art", cmgr->getFlagArt());
    conf->setValue("viewing/flag_constellation_isolate_selected", cmgr->getFlagIsolateSelected());
    conf->setValue("viewing/constellation_art_intensity", cmgr->getArtIntensity());
    conf->setValue("astro/flag_star_name", smgr->getFlagLabels());
    conf->setValue("astro/flag_star_selected_name", smgr->getFlagSelectedLabel());
    conf->setValue("stars/labels_amount", smgr->getLabelsAmount());
    conf->setValue("astro/flag_planets_labels", ssmgr->getFlagLabels());
    conf->setValue("astro/flag_planet_selected_label", ssmgr->getFlagSelectedLabel());

    conf->setValue("astro/labels_amount", ssmgr->getLabelsAmount());
    conf->setValue("astro/nebula_hints_amount", nmgr->getHintsAmount());
    conf->setValue("astro/flag_nebula_name", nmgr->getFlagHints());
    conf->setValue("projection/type", StelApp::getInstance().getCore()->getCurrentProjectionTypeKey());

    // view dialog / landscape tab settings
    lmgr->setDefaultLandscapeID(lmgr->getCurrentLandscapeID());
    conf->setValue("landscape/flag_landscape_sets_location", lmgr->getFlagLandscapeSetsLocation());
    conf->setValue("landscape/flag_landscape", lmgr->getFlagLandscape());
    conf->setValue("landscape/flag_atmosphere", lmgr->getFlagAtmosphere());
    conf->setValue("landscape/flag_fog", lmgr->getFlagFog());
    conf->setValue("stars/init_bortle_scale", StelApp::getInstance().getCore()->getSkyDrawer()->getBortleScale());

    // view dialog / starlore tab
    StelApp::getInstance().getSkyCultureMgr().setDefaultSkyCultureID(StelApp::getInstance().getSkyCultureMgr().getCurrentSkyCultureID());

    // Save default location
    if (!autosave)
        nav->setDefaultLocationID(nav->getCurrentLocation().getID());

    // configuration dialog / main tab
    QString langName = StelApp::getInstance().getLocaleMgr().getAppLanguage();
    conf->setValue("localization/app_locale", StelTranslator::nativeNameToIso639_1Code(langName));
    langName = StelApp::getInstance().getLocaleMgr().getSkyLanguage();
    conf->setValue("localization/sky_locale", StelTranslator::nativeNameToIso639_1Code(langName));

    if (gui->getInfoTextFilters() == (StelObject::InfoStringGroup)0)
        conf->setValue("gui/selected_object_info", "none");
    else if (gui->getInfoTextFilters() == StelObject::InfoStringGroup(StelObject::ShortInfo))
        conf->setValue("gui/selected_object_info", "short");
    else
        conf->setValue("gui/selected_object_info", "all");

    // toolbar auto-hide status
    conf->setValue("gui/auto_hide_horizontal_toolbar", gui->getAutoHideHorizontalButtonBar());
    conf->setValue("gui/auto_hide_vertical_toolbar", gui->getAutoHideVerticalButtonBar());

    if (!autosave)
    {
        mvmgr->setInitFov(mvmgr->getCurrentFov());
        mvmgr->setInitViewDirectionToCurrent();
    }

    // configuration dialog / navigation tab
    conf->setValue("navigation/flag_enable_zoom_keys", mvmgr->getFlagEnableZoomKeys());
    conf->setValue("navigation/flag_enable_mouse_navigation", mvmgr->getFlagEnableMouseNavigation());
    conf->setValue("navigation/flag_enable_move_keys", mvmgr->getFlagEnableMoveKeys());
    conf->setValue("navigation/startup_time_mode", nav->getStartupTimeMode());
    conf->setValue("navigation/today_time", nav->getInitTodayTime());
    conf->setValue("navigation/preset_sky_time", nav->getPresetSkyTime());
    conf->setValue("navigation/init_fov", mvmgr->getInitFov());
    if (mvmgr->getMountMode() == StelMovementMgr::MountAltAzimuthal)
        conf->setValue("navigation/viewing_mode", "horizon");
    else
        conf->setValue("navigation/viewing_mode", "equator");


    // configuration dialog / tools tab
    conf->setValue("gui/flag_show_flip_buttons", gui->getFlagShowFlipButtons());
    //conf->setValue("video/viewport_effect", StelMainGraphicsView::getInstance().getStelAppGraphicsWidget()->getViewportEffect());
    conf->setValue("projection/viewport", StelProjector::maskTypeToString(proj->getMaskType()));
    conf->setValue("viewing/flag_gravity_labels", proj->getFlagGravityLabels());
    conf->setValue("navigation/auto_zoom_out_resets_direction", mvmgr->getFlagAutoZoomOutResetsDirection());
    conf->setValue("gui/flag_mouse_cursor_timeout", StelMainGraphicsView::getInstance().getFlagCursorTimeout());
    conf->setValue("gui/mouse_cursor_timeout", StelMainGraphicsView::getInstance().getCursorTimeout());

    conf->setValue("main/screenshot_dir", StelFileMgr::getScreenshotDir());
    conf->setValue("main/invert_screenshots_colors", StelMainGraphicsView::getInstance().getFlagInvertScreenShotColors());

    // full screen and window size
    conf->setValue("video/fullscreen", StelMainWindow::getInstance().getFullScreen());
    if (!StelMainWindow::getInstance().getFullScreen())
    {
        //#ifndef SHIRAPLAYER_PRE
        if(StelMainWindow::getInstance().getIsMultiprojector())
        {
            conf->setValue("video/screen_w", StelMainWindow::getInstance().size().width());
            conf->setValue("video/screen_h", StelMainWindow::getInstance().size().height());
        }
        //#endif
    }

    // clear the restore defaults flag if it is set.
    conf->setValue("main/restore_defaults", false);

    //#ifdef SHIRAPLAYER_PRE
    conf->setValue("main/media_path",ui->txtFulldomePath->text());
    conf->setValue("main/flat_media_path", ui->txtFlatPath->text());
    conf->setValue("video/screen_l", ui->spScreenLeft->value());
    conf->setValue("video/screen_t", ui->spScreenTop->value());
    conf->setValue("video/screen_w", ui->spScreenWidth->value());
    conf->setValue("video/screen_h", ui->spScreenHeight->value());
    conf->setValue("video/preview_w",ui->spPreviewWidth->value());
    conf->setValue("video/preview_h",ui->spPreviewHeight->value());
    conf->setValue("video/console_h",ui->spConsoleHeight->value());
    conf->setValue("video/apply_finetune", ui->chApplyFine->isChecked());
    if(ui->sphericMirrorCheckbox->isChecked())
        conf->setValue("video/viewport_effect","sphericMirrorDistorter");
    else
        conf->setValue("video/viewport_effect","none");
    //#endif

    conf->setValue("viewing/flag_night",StelApp::getInstance().getVisionModeNight());

    const QList<StelModuleMgr::PluginDescriptor> pluginsList = StelApp::getInstance().getModuleMgr().getPluginsList();
    foreach (const StelModuleMgr::PluginDescriptor& desc, pluginsList)
    {
        if (desc.info.id == "Satellites")
        {
            if (desc.loaded)
            {
                conf->setValue("Satellites/show_satellites",
                               StelApp::getInstance().getGui()->getGuiActions("actionShow_Satellite_Show")->isChecked());
            }
        }
    }

    //auto-save için ek kayýtlar
    if (autosave)
    {
        conf->setValue("navigation/flag_start_initview",true);        
        conf->setValue("navigation/startup_time_mode", "preset");
        conf->setValue("navigation/preset_sky_time",
                       StelApp::getInstance().getCore()->getNavigator()->getJDay()+
                       (StelUtils::getGMTShiftFromQT(StelUtils::getJDFromSystem()) * JD_HOUR));
    }
}

void ConfigurationDialog::updateConfigLabels()
{
    ui->startupFOVLabel->hide();
    ui->startupDirectionOfViewlabel->hide();

    //ui->startupFOVLabel->setText(q_("Startup FOV: %1%2").arg(StelApp::getInstance().getCore()->getMovementMgr()->getCurrentFov()).arg(QChar(0x00B0)));

    //double az, alt;
    //const Vec3d& v = GETSTELMODULE(StelMovementMgr)->getInitViewingDirection();
    //StelUtils::rectToSphe(&az, &alt, v);
    //az = 3.*M_PI - az;  // N is zero, E is 90 degrees
    //if (az > M_PI*2)
    //    az -= M_PI*2;
    //ui->startupDirectionOfViewlabel->setText(q_("Startup direction of view Az/Alt: %1/%2").arg(StelUtils::radToDmsStr(az), StelUtils::radToDmsStr(alt)));
}

void ConfigurationDialog::setDefaultViewOptions()
{
    QMessageBox msgBox;
    msgBox.setWindowTitle(q_("Warning!"));
    msgBox.setText(q_("Do you want to restore default all view options?"));
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setWindowFlags(Qt::WindowStaysOnTopHint);
    msgBox.setIcon(QMessageBox::Warning);
    int ret = msgBox.exec();

    if(ret ==QMessageBox::Yes)
    {
        QSettings* conf = StelApp::getInstance().getSettings();
        Q_ASSERT(conf);

        conf->setValue("main/restore_defaults", true);
    }
}

void ConfigurationDialog::populatePluginsList()
{
    int prevSel = ui->pluginsListWidget->currentRow();
    ui->pluginsListWidget->clear();
    const QList<StelModuleMgr::PluginDescriptor> pluginsList = StelApp::getInstance().getModuleMgr().getPluginsList();
    foreach (const StelModuleMgr::PluginDescriptor& desc, pluginsList)
    {
        ui->pluginsListWidget->addItem(desc.info.displayedName);
    }
    // If we had a valid previous selection (i.e. not first time we populate), restore it
    if (prevSel >= 0 && prevSel < ui->pluginsListWidget->count())
        ui->pluginsListWidget->setCurrentRow(prevSel);
    else
        ui->pluginsListWidget->setCurrentRow(0);
}

void ConfigurationDialog::pluginsSelectionChanged(const QString& s)
{
    const QList<StelModuleMgr::PluginDescriptor> pluginsList = StelApp::getInstance().getModuleMgr().getPluginsList();
    foreach (const StelModuleMgr::PluginDescriptor& desc, pluginsList)
    {
        if (s==desc.info.displayedName)
        {
            QString html = "<html><head></head><body>";
            html += "<h2>" + desc.info.displayedName + "</h2>";
            html += "<h3>" + q_("Authors") + ": " + desc.info.authors + "</h3>";
            QString d = desc.info.description;
            d.replace("\n", "<br />");
            html += "<p>" + d + "</p>";
            html += "<h3>" + q_("Contact") + ": " + desc.info.contact + "</h3>";
            html += "</body></html>";
            ui->pluginsInfoBrowser->setHtml(html);
            ui->pluginLoadAtStartupCheckBox->setChecked(desc.loadAtStartup);
            StelModule* pmod = StelApp::getInstance().getModuleMgr().getModule(desc.info.id, true);
            if (pmod != NULL)
                ui->pluginConfigureButton->setEnabled(pmod->configureGui(false));
            else
                ui->pluginConfigureButton->setEnabled(false);
            return;
        }
    }
}

void ConfigurationDialog::pluginConfigureCurrentSelection(void)
{
    QString s = ui->pluginsListWidget->currentItem()->text();
    if (s.isEmpty() || s=="")
        return;

    const QList<StelModuleMgr::PluginDescriptor> pluginsList = StelApp::getInstance().getModuleMgr().getPluginsList();
    foreach (const StelModuleMgr::PluginDescriptor& desc, pluginsList)
    {
        if (s==desc.info.displayedName)
        {
            StelModule* pmod = StelApp::getInstance().getModuleMgr().getModule(desc.info.id);
            if (pmod != NULL)
            {
                pmod->configureGui(true);
            }
            return;
        }
    }
}

void ConfigurationDialog::loadAtStartupChanged(int state)
{
    if (ui->pluginsListWidget->count() <= 0)
        return;
    QString name = ui->pluginsListWidget->currentItem()->text();
    QString key;
    QList<StelModuleMgr::PluginDescriptor> pluginsList = StelApp::getInstance().getModuleMgr().getPluginsList();
    foreach (const StelModuleMgr::PluginDescriptor& desc, pluginsList)
    {
        if (desc.info.displayedName==name)
            key = desc.info.id;
    }
    if (!key.isEmpty())
        StelApp::getInstance().getModuleMgr().setPluginLoadAtStartup(key, state==Qt::Checked);
}

void ConfigurationDialog::populateScriptsList(void)
{
    int prevSel = ui->scriptListWidget->currentRow();
    StelScriptMgr& scriptMgr = StelMainGraphicsView::getInstance().getScriptMgr();
    ui->scriptListWidget->clear();
    ui->scriptListWidget->addItems(scriptMgr.getScriptList());
    // If we had a valid previous selection (i.e. not first time we populate), restore it
    if (prevSel >= 0 && prevSel < ui->scriptListWidget->count())
        ui->scriptListWidget->setCurrentRow(prevSel);
    else
        ui->scriptListWidget->setCurrentRow(0);
}

void ConfigurationDialog::scriptSelectionChanged(const QString& s)
{
    if (s.isEmpty())
        return;
    StelScriptMgr& scriptMgr = StelMainGraphicsView::getInstance().getScriptMgr();
    //ui->scriptInfoBrowser->document()->setDefaultStyleSheet(QString(StelApp::getInstance().getCurrentStelStyle()->htmlStyleSheet));
    QString html = "<html><head></head><body>";
    html += "<h2>" + scriptMgr.getName(s) + "</h2>";
    html += "<h3>" + q_("Author") + ": " + scriptMgr.getAuthor(s) + "</h3>";
    html += "<h3>" + q_("License") + ": " + scriptMgr.getLicense(s) + "</h3>";
    QString d = scriptMgr.getDescription(s);
    d.replace("\n", "<br />");
    html += "<p>" + d + "</p>";
    html += "</body></html>";
    ui->scriptInfoBrowser->setHtml(html);
}

void ConfigurationDialog::runScriptClicked(void)
{
    if (ui->closeWindowAtScriptRunCheckbox->isChecked())
        this->close();

    StelScriptMgr& scriptMgr = StelMainGraphicsView::getInstance().getScriptMgr();
    if (ui->scriptListWidget->currentItem())
    {
        ui->runScriptButton->setEnabled(false);
        ui->stopScriptButton->setEnabled(true);
        scriptMgr.runScript(ui->scriptListWidget->currentItem()->text());
    }
}

void ConfigurationDialog::stopScriptClicked(void)
{
    GETSTELMODULE(LabelMgr)->deleteAllLabels();
    GETSTELMODULE(ScreenImageMgr)->deleteAllImages();
    StelMainGraphicsView::getInstance().getScriptMgr().stopScript();
}

void ConfigurationDialog::aScriptIsRunning(void)
{
    ui->scriptStatusLabel->setText(q_("Running script: ") + StelMainGraphicsView::getInstance().getScriptMgr().runningScriptId());
    ui->runScriptButton->setEnabled(false);
    ui->stopScriptButton->setEnabled(true);
}

void ConfigurationDialog::aScriptHasStopped(void)
{
    ui->scriptStatusLabel->setText(q_("Running script: [none]"));
    ui->runScriptButton->setEnabled(true);
    ui->stopScriptButton->setEnabled(false);
}


void ConfigurationDialog::setFixedDateTimeToCurrent(void)
{
    ui->fixedDateTimeEdit->setDateTime(StelUtils::jdToQDateTime(StelApp::getInstance().getCore()->getNavigator()->getJDay()));
    ui->fixedTimeRadio->setChecked(true);
    setStartupTimeMode();
}

void ConfigurationDialog::changePage(QListWidgetItem *current, QListWidgetItem *previous)
{
    if (!current)
        current = previous;
    ui->configurationStackedWidget->setCurrentIndex(ui->stackListWidget->row(current));
}


void ConfigurationDialog::refreshStarCatalogButton()
{
    const QVariantList& catalogConfig = GETSTELMODULE(StarMgr)->getCatalogsDescription();
    nextStarCatalogToDownload.clear();
    int idx=0;
    foreach (const QVariant& catV, catalogConfig)
    {
        ++idx;
        const QVariantMap& m = catV.toMap();
        const bool checked = m.value("checked").toBool();
        if (checked)
            continue;
        nextStarCatalogToDownload=m;
        break;
    }

    ui->downloadCancelButton->setVisible(false);
    ui->downloadRetryButton->setVisible(false);

    if (idx == catalogConfig.size() && !hasDownloadedStarCatalog)//The size is 9; for "stars8", idx is 9
    {
        ui->getStarsButton->setVisible(false);
        ui->downloadLabel->setText(q_("Finished downloading all star catalogs!"));
        //BM: Doesn't this message duplicate the one below?
        //This one should be something like "All available star catalogs are installed."
        return;
    }

    ui->getStarsButton->setEnabled(true);
    if (!nextStarCatalogToDownload.isEmpty())
    {
        ui->getStarsButton->setText(q_("Get catalog %1 of %2").arg(idx).arg(catalogConfig.size()));
        const QVariantList& magRange = nextStarCatalogToDownload.value("magRange").toList();
        try
        {
            ui->downloadLabel->setText(q_("Download size: %1MB\nStar count: %2 Million\nMagnitude range: %3 - %4")
                                       .arg(nextStarCatalogToDownload.value("sizeMb").toString())
                                       .arg(nextStarCatalogToDownload.value("count").toString())
                                       .arg(magRange.at(0).toString())
                                       .arg(magRange.at(1).toString()));

        }
        catch (std::runtime_error& e)
        {
            qWarning() << "WARNING: unable StarCatalog " << e.what();
        }
        ui->getStarsButton->setVisible(true);
    }
    else
    {
        ui->downloadLabel->setText(q_("Finished downloading new star catalogs!\nRestart ShiraPlayer to display them."));
        ui->getStarsButton->setVisible(false);
    }
}

void ConfigurationDialog::cancelDownload(void)
{
    Q_ASSERT(currentDownloadFile);
    Q_ASSERT(starCatalogDownloadReply);
    qWarning() << "Aborting download";
    starCatalogDownloadReply->abort();
}

void ConfigurationDialog::newStarCatalogData()
{
    Q_ASSERT(currentDownloadFile);
    Q_ASSERT(starCatalogDownloadReply);
    Q_ASSERT(progressBar);

    int size = starCatalogDownloadReply->bytesAvailable();
    progressBar->setValue((float)progressBar->value()+(float)size/1024);
    QByteArray ba = starCatalogDownloadReply->read(size);


    if(ba.indexOf("Found")==-1)
        currentDownloadFile->write(ba);
}

void ConfigurationDialog::downloadStars()
{
    Q_ASSERT(!nextStarCatalogToDownload.isEmpty());
    Q_ASSERT(starCatalogDownloadReply==NULL);
    Q_ASSERT(currentDownloadFile==NULL);
    Q_ASSERT(progressBar==NULL);

    QString path = StelFileMgr::getUserDir()+QString("/stars/default/")+nextStarCatalogToDownload.value("fileName").toString();
    currentDownloadFile = new QFile(path);
    if (!currentDownloadFile->open(QIODevice::WriteOnly))
    {
        qWarning() << "Can't open a writable file for storing new star catalog: " << path;
        currentDownloadFile->deleteLater();
        currentDownloadFile = NULL;
        ui->downloadLabel->setText(q_("Error downloading %1:\n%2").arg(nextStarCatalogToDownload.value("id").toString()).arg(QString("Can't open a writable file for storing new star catalog: %1").arg(path)));
        ui->downloadRetryButton->setVisible(true);
        return;
    }

    ui->downloadLabel->setText(q_("Downloading %1...\n(You can close this window.)").arg(nextStarCatalogToDownload.value("id").toString()));
    ui->downloadCancelButton->setVisible(true);
    ui->downloadRetryButton->setVisible(false);
    ui->getStarsButton->setVisible(true);
    ui->getStarsButton->setEnabled(false);

    QNetworkRequest req(nextStarCatalogToDownload.value("url").toString());
    req.setAttribute(QNetworkRequest::CacheSaveControlAttribute, false);
    req.setAttribute(QNetworkRequest::RedirectionTargetAttribute, false);
    req.setRawHeader("User-Agent", userAgent.toLatin1());
    starCatalogDownloadReply = StelApp::getInstance().getNetworkAccessManager()->get(req);
    starCatalogDownloadReply->setReadBufferSize(1024*1024*2);
    //connect(starCatalogDownloadReply, SIGNAL(readyRead()), this, SLOT(newStarCatalogData()));
    connect(starCatalogDownloadReply, SIGNAL(finished()), this, SLOT(downloadFinished()));
    connect(starCatalogDownloadReply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(downloadError(QNetworkReply::NetworkError)));

    progressBar  = new QProgressBar();
    progressBar->setFixedHeight(15);
    progressBar->setTextVisible(true);
    progressBar->setValue(0);
    ui->frmProgress->layout()->addWidget(progressBar);
    /*progressBar = StelApp::getInstance().getGui()->addProgressBar();*/
    progressBar->setValue(0);
    progressBar->setMaximum(nextStarCatalogToDownload.value("sizeMb").toDouble()*1024);
    progressBar->setVisible(true);
    progressBar->setFormat(QString("%1: %p%").arg(nextStarCatalogToDownload.value("id").toString()));
}

void ConfigurationDialog::downloadError(QNetworkReply::NetworkError code)
{
    Q_ASSERT(currentDownloadFile);
    Q_ASSERT(starCatalogDownloadReply);

    qWarning() << "Error downloading file" << starCatalogDownloadReply->url() << ": " << starCatalogDownloadReply->errorString();
    ui->downloadLabel->setText(q_("Error downloading %1:\n%2")
                               .arg(nextStarCatalogToDownload.value("id").toString())
                               .arg(starCatalogDownloadReply->errorString()));
    ui->downloadCancelButton->setVisible(false);
    ui->downloadRetryButton->setVisible(true);
    ui->getStarsButton->setVisible(false);
    ui->getStarsButton->setEnabled(true);
}

void ConfigurationDialog::downloadFinished()
{
    Q_ASSERT(currentDownloadFile);
    Q_ASSERT(starCatalogDownloadReply);
    Q_ASSERT(progressBar);

    if (starCatalogDownloadReply->error()!=QNetworkReply::NoError)
    {
        starCatalogDownloadReply->deleteLater();
        starCatalogDownloadReply = NULL;
        currentDownloadFile->close();
        currentDownloadFile->deleteLater();
        currentDownloadFile = NULL;
        progressBar->deleteLater();
        progressBar=NULL;
        return;
    }

    Q_ASSERT(starCatalogDownloadReply->bytesAvailable()==0);

    const QVariant& redirect = starCatalogDownloadReply->attribute(QNetworkRequest::RedirectionTargetAttribute);
    if (!redirect.isNull())
    {
        // We got a redirection, we need to follow
        starCatalogDownloadReply->deleteLater();
        QNetworkRequest req(redirect.toUrl());
        req.setAttribute(QNetworkRequest::CacheSaveControlAttribute, false);
        req.setAttribute(QNetworkRequest::RedirectionTargetAttribute, false);
        req.setRawHeader("User-Agent", userAgent.toLatin1());
        starCatalogDownloadReply = StelApp::getInstance().getNetworkAccessManager()->get(req);
        starCatalogDownloadReply->setReadBufferSize(1024*1024*2);
        connect(starCatalogDownloadReply, SIGNAL(readyRead()), this, SLOT(newStarCatalogData()));
        connect(starCatalogDownloadReply, SIGNAL(finished()), this, SLOT(downloadFinished()));
        connect(starCatalogDownloadReply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(downloadError(QNetworkReply::NetworkError)));
        return;
    }

    currentDownloadFile->close();
    currentDownloadFile->deleteLater();
    currentDownloadFile = NULL;
    starCatalogDownloadReply->deleteLater();
    starCatalogDownloadReply = NULL;
    progressBar->deleteLater();
    progressBar=NULL;

    ui->downloadLabel->setText(q_("Verifying file integrity..."));
    if (GETSTELMODULE(StarMgr)->checkAndLoadCatalog(nextStarCatalogToDownload)==false)
    {
        ui->getStarsButton->setVisible(false);
        ui->downloadLabel->setText(q_("Error downloading %1:\nFile is corrupted.").arg(nextStarCatalogToDownload.value("id").toString()));
        ui->downloadCancelButton->setVisible(false);
        ui->downloadRetryButton->setVisible(true);
    }
    else
    {
        hasDownloadedStarCatalog = true;
    }

    refreshStarCatalogButton();
}

void ConfigurationDialog::on_btnOpenMirrorData_clicked()
{
    QString customDataFileName = QFileDialog::getOpenFileName(0,
                                                              tr("Select Custom Data File"),
                                                              "",
                                                              tr("Data Files(*.data *.*)"));
    ui->txtCustomDataFile->setText(customDataFileName);
}

void ConfigurationDialog::on_btnSelectFulldomePath_clicked()
{
    QString dir = QFileDialog::getExistingDirectory(0, tr("Open Directory"),
                                                    "C:/",
                                                    QFileDialog::ShowDirsOnly
                                                    | QFileDialog::DontResolveSymlinks);
    ui->txtFulldomePath->setText(dir);
}

void ConfigurationDialog::on_btnSelectFlatPath_clicked()
{
    QString dir = QFileDialog::getExistingDirectory(0, tr("Open Directory"),
                                                    "C:/",
                                                    QFileDialog::ShowDirsOnly
                                                    | QFileDialog::DontResolveSymlinks);
    ui->txtFlatPath->setText(dir);

}

void ConfigurationDialog::on_chEnableTune_checked(bool val)
{
    ui->btnResetWarp->setEnabled(val);
    ui->btnSaveFineTune->setEnabled(val);
    ui->btnDelFineTune->setEnabled(val);

    StelApp::getInstance().getRsync()->sendChanges(RSYNC_COMMAND_FINETUNE,QString("%0").arg(val));
}
void ConfigurationDialog::on_resetWarp_Clicked()
{
    QMessageBox msgBox;
    msgBox.setWindowTitle(q_("Warning!"));
    msgBox.setText(q_("Reset Fine-Tune settings?"));
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::Save);
    msgBox.setWindowModality(Qt::WindowModality());
    msgBox.setIcon(QMessageBox::Warning);
    int ret = msgBox.exec();

    if(ret ==QMessageBox::Yes)
    {
        StelApp::getInstance().getRsync()->sendChanges(RSYNC_COMMAND_FINETUNE_RESET,"");
    }
}
void ConfigurationDialog::on_btnSaveFineTune_Clicked()
{
    QMessageBox msgBox;
    msgBox.setWindowTitle(q_("Warning!"));
    msgBox.setText(q_("Save Fine-Tune settings?"));
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::Save);
    msgBox.setWindowModality(Qt::WindowModality());
    msgBox.setIcon(QMessageBox::Warning);
    int ret = msgBox.exec();

    if(ret ==QMessageBox::Yes)
    {
        StelApp::getInstance().getRsync()->sendChanges(RSYNC_COMMAND_FINETUNE_SAVE,"");
    }
}
void ConfigurationDialog::on_btnDelFineTune_Clicked()
{
    QMessageBox msgBox;
    msgBox.setWindowTitle(q_("Warning!"));
    msgBox.setText(q_("Delete Fine-Tune settings?"));
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::Save);
    msgBox.setWindowModality(Qt::WindowModality());
    msgBox.setIcon(QMessageBox::Warning);
    int ret = msgBox.exec();

    if(ret ==QMessageBox::Yes)
    {
        QFile file("warpsettings.xml");
        if (file.remove())
            StelApp::getInstance().getRsync()->sendChanges(RSYNC_COMMAND_FINETUNE_RESET,"");
    }
}

void ConfigurationDialog::on_chInitialView_toggled(bool)
{
    QSettings* conf = StelApp::getInstance().getSettings();
    conf->setValue("navigation/flag_start_initview",ui->chInitialView->isChecked());
}

void ConfigurationDialog::retranslate()
{
    languageChanged();

}
