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

#ifndef _DATETIMEDIALOG_HPP_
#define _DATETIMEDIALOG_HPP_

#include <QObject>
#include "StelDialog.hpp"

class Ui_dateTimeDialogForm;

class DateTimeDialog : public StelDialog
{
    Q_OBJECT
public:
    DateTimeDialog();
    ~DateTimeDialog();
    Ui_dateTimeDialogForm* ui;
    double newJd();
    bool valid(int y, int m, int d, int h, int min, int s);
    void languageChanged();
    //! Notify that the application style changed
    void styleChanged();
public slots:
    //! update the editing display with new JD.
    void setDateTime(double newJd);

signals:
    //! signals that a new, valid JD is available.
    void dateTimeChanged(double newJd);

protected:
    //! Initialize the dialog widgets and connect the signals/slots
    virtual void createDialogContent();
    virtual void prepareAsChild();

private slots:
    //! year slider or dial changed
    void yearChanged(int ny);
    //! year slider or dial changed
    void monthChanged(int nm);
    //! year slider or dial changed
    void dayChanged(int nd);
    //! year slider or dial changed
    void hourChanged(int nh);
    //! year slider or dial changed
    void minuteChanged(int nm);
    //! year slider or dial changed
    void secondChanged(int ns);

    //ASAF
    void on_visibleChanged(bool b);
    void on_load(bool b);

    void on_combo_currentIndexChanged(int index);
    void on_btnStop_clicked();
    void on_btnForward_clicked();
    void on_btnRewind_clicked();
    void on_btnNow_clicked();

private:
    int year;
    int month;
    int day;
    int hour;
    int minute;
    int second;
    void pushToWidgets();

    //ASAF
    void setButtonDurum();

public slots:
    void retranslate();
};

#endif // _DATETIMEDIALOG_HPP_
