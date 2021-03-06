/*
 * ShiraPlayer(TM)
 * Copyright (C) 2009 Matthew Gates
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

#ifndef _SCRIPTCONSOLE_HPP_
#define _SCRIPTCONSOLE_HPP_

#include <QObject>
#include "StelDialog.hpp"

class Ui_scriptConsoleForm;
class StelScriptSyntaxHighlighter;

class ScriptConsole : public StelDialog
{
    Q_OBJECT
public:
    ScriptConsole();
    virtual ~ScriptConsole();
    void languageChanged();
    //! Notify that the application style changed
    void styleChanged();

public slots:
    void runScript();
    void loadScript();
    void saveScript();
    void clearButtonPressed();
    void preprocessScript();
    void scriptEnded();
    void appendLogLine(const QString& s);
    void includeBrowse();
    void quickRun(int idx);
    void rowColumnChanged();

private slots:
    void on_visibleChanged(bool b);
    void on_load(bool b);

protected:
    Ui_scriptConsoleForm* ui;

    //! Initialize the dialog widgets and connect the signals/slots
    virtual void createDialogContent();
    virtual void prepareAsChild();

private:
    QString getFileMask();
    StelScriptSyntaxHighlighter* highlighter;

public slots:
    void retranslate();
};

#endif // _SCRIPTCONSOLE_HPP_slots:

