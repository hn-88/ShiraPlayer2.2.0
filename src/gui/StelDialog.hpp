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

#ifndef _STELDIALOG_HPP_
#define _STELDIALOG_HPP_

#include <QObject>
#include <QGraphicsProxyWidget>

//! @class StelDialog
//! Base class for all the GUI windows in shiraplayer
class StelDialog : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool visible READ visible WRITE setVisible NOTIFY visibleChanged)
public:
    StelDialog();
    virtual ~StelDialog();
    //! Retranslate the content of the dialog
    virtual void languageChanged()=0;

    bool visible() const;
    QGraphicsProxyWidget* getProxy(){return (QGraphicsProxyWidget*)proxy;}
    QWidget* getDialog(){ return dialog;}

public slots:
    virtual void retranslate() = 0;

    void setVisible(bool);
    void setVisibleAsChild(QWidget* parent,bool v);
    void close();
signals:
    void visibleChanged(bool);
    void load(bool);
protected:
    //! Initialize the dialog widgets and connect the signals/slots
    virtual void createDialogContent()=0;
    virtual void prepareAsChild()=0;

    //! The main dialog
    QWidget* dialog;
    class CustomProxy* proxy;

};

#endif // _STELDIALOG_HPP_
