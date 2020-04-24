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

#ifndef _CONFIGURATIONDIALOG_HPP_
#define _CONFIGURATIONDIALOG_HPP_

#include <QObject>
#include <QProgressBar>
#include <QtNetwork/QNetworkReply>
#include <QFile>
#include "StelDialog.hpp"

class Ui_configurationDialogForm;
class QSettings;
class QDataStream;
class QNetworkAccessManager;
class QListWidgetItem;
class StelGui;

class ConfigurationDialog : public StelDialog
{
    Q_OBJECT
public:
    ConfigurationDialog(StelGui* agui);
    virtual ~ConfigurationDialog();
    void languageChanged();
    //! Notify that the application style changed
    void styleChanged();
    void loadInitOptions();
protected:
    //! Initialize the dialog widgets and connect the signals/slots
    virtual void createDialogContent();
    virtual void prepareAsChild();
    Ui_configurationDialogForm* ui;

private:
    //! Contains the parsed content of the starsConfig.json file
    QVariantMap nextStarCatalogToDownload;
    //! Set the content of the "Star catalog updates" box
    void refreshStarCatalogButton();
    //! True when at least one star catalog has been downloaded successfully this session
    bool hasDownloadedStarCatalog;
    QNetworkReply* starCatalogDownloadReply;
    QFile* currentDownloadFile;
    QProgressBar* progressBar;
    QString userAgent;
private slots:
    void setNoSelectedInfo(void);
    void setAllSelectedInfo(void);
    void setBriefSelectedInfo(void);
    void languageChanged(const QString& languageCode);
    void setStartupTimeMode(void);
    void setDiskViewport(bool);
    void setSphericMirror(bool);
    void cursorTimeOutChanged();
    void cursorTimeOutChanged(double d) {cursorTimeOutChanged();}

    void newStarCatalogData();
    void downloadStars();
    void cancelDownload();
    void downloadFinished();
    void downloadError(QNetworkReply::NetworkError);

    //! Update the labels displaying the current default state
    void updateConfigLabels();

    //! open a file dialog to browse for a directory to save screenshots to
    //! if a directory is selected (i.e. dialog not cancelled), current
    //! value will be changed, but not saved to config file.
    void browseForScreenshotDir();
    void selectScreenshotDir(const QString& dir);

    //! Save the current viewing option including landscape, location and sky culture
    //! This doesn't include the current viewing direction, time and FOV since those
    //! have specific controls
    void saveCurrentViewOptions();

    //! Reset all stellarium options.
    //! This basically replaces the config.ini by the default one
    void setDefaultViewOptions();

    void populatePluginsList(void);
    void pluginsSelectionChanged(const QString&);
    void pluginConfigureCurrentSelection(void);
    void loadAtStartupChanged(int);

    //! The selection of script in the script list has changed
    //! Updates the script information panel
    void scriptSelectionChanged(const QString& s);

    //! Run the currently selected script from the script list.
    void runScriptClicked(void);
    //! Stop the currently running script.
    void stopScriptClicked(void);

    void aScriptIsRunning(void);
    void aScriptHasStopped(void);

    void populateScriptsList(void);
    void setFixedDateTimeToCurrent(void);

    void changePage(QListWidgetItem *current, QListWidgetItem *previous);

    void setFlagGravityLabels(bool b);

    //void setStellaMirror(bool b);
    //void setCustomMirror(bool b);

    void on_btnOpenMirrorData_clicked();
    void on_btnSelectFulldomePath_clicked();
    void on_btnSelectFlatPath_clicked();

    void on_chEnableTune_checked(bool);
    void on_resetWarp_Clicked();
    void on_btnSaveFineTune_Clicked();
    void on_btnDelFineTune_Clicked();

    void on_chInitialView_toggled(bool);
private:
    StelGui* gui;

    int savedProjectionType;

public slots:
    void retranslate();
    void saveCurrentOptions(bool autosave);
};

#endif // _CONFIGURATIONDIALOG_HPP_
