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
#ifndef _VIEWDIALOG_HPP_
#define _VIEWDIALOG_HPP_

#include <QObject>
#include "StelDialog.hpp"
//ASAF
#include <QTableWidget>
#include "warping/channel.h"

class Ui_viewDialogForm;
class QListWidgetItem;

class ViewDialog : public StelDialog
{
    Q_OBJECT
public:
    ViewDialog();
    virtual ~ViewDialog();
    void languageChanged();
    //! Notify that the application style changed
    void styleChanged();
    Ui_viewDialogForm* ui;
protected:

    //! Initialize the dialog widgets and connect the signals/slots
    virtual void createDialogContent();
    virtual void prepareAsChild();
private slots:
    void populateLists();
    void skyCultureChanged(const QString& cultureName);
    void projectionChanged(const QString& projectionName);
    void landscapeChanged(QListWidgetItem* item);
    void shootingStarsZHRChanged();
    void planetsLabelsValueChanged(int);
    void nebulasLabelsValueChanged(int);
    void starsLabelsValueChanged(int);
    void setCurrentLandscapeAsDefault(void);
    void setCurrentCultureAsDefault(void);
    //! Update the widget to make sure it is synchrone if a value was changed programmatically
    //! This function should be called repeatidly with e.g. a timer
    void updateFromProgram();

    void populateSkyLayersList();
    void skyLayersSelectionChanged(const QString&);
    void skyLayersEnabledChanged(int);

    void changePage(QListWidgetItem *current, QListWidgetItem *previous);

    //ASAF
    void warpModeChanged(const QString& smode);
    void saveWarpSettings();
    void setcurrFOV(double fov);
    void on_change_FWR(double fwr);
    void on_change_FHR(double fhr);

    void on_addButton_clicked();
    void on_removeButton_clicked();
    void on_channelTable_itemChanged(QTableWidgetItem* pItem);
    void on_checkConfig_checked(bool b);
    void on_channelList_select(QTableWidgetItem* pItem);
    void on_btnResetWarp_clicked();
    void on_visibleChanged(bool b);
    void on_load(bool b);
    void on_setChNameShow(bool b);

    void updateChannelGUI();
    void updateChannelNamesGUI();

    void on_value_Width(int val);
    void on_value_Height(int val);

    void landscapeVideoChanged(QListWidgetItem* item);

    //void setmoonScaleAmount(double val);

    void on_setDateTimeShow(bool b);
    void on_setLocationShow(bool b);
    void on_setPropGuiShow(bool b);

private:
    void updateSkyCultureText();
public slots:
    void retranslate();
};

#endif // _VIEWDIALOG_HPP_
