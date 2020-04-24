/*
 * ShiraPlayer(TM)
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

#ifndef _RECORDMANAGER_HPP
#define _RECORDMANAGER_HPP

#include "StelDialog.hpp"
#include "ScreenImageMgr.hpp"
#include "licenceutils/qlisansform.h"

class Ui_recordmanager;

class recordmanager : public StelDialog
{
    Q_OBJECT
public:
    recordmanager();    
    ~recordmanager();

    void languageChanged();
    //! Notify that the application style changed
    void styleChanged();
private:
    //! Initialize the dialog widgets and connect the signals/slots
    virtual void createDialogContent();
    virtual void prepareAsChild();
    Ui_recordmanager *ui;

    QString movieFileName;

    QLisansForm frm;
    ScreenImageMgr sImgr;
public slots:
    void retranslate();

private slots:
    void textFileChanged (QString text );
    void selectbtnClicked();
    void recButtonClick();
    void recstopButtonClick();
    void recpauseButtonClick();

    void btnLisansClick();

};

#endif // _RECORDMANAGER_HPP
