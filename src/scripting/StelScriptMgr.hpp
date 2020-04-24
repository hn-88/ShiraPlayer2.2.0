/*
 * ShiraPlayer(TM)
 * Copyright (C) 2007 Fabien Chereau
 * 2009 Matthew Gates
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

#ifndef _STELSCRIPTMGR_HPP_
#define _STELSCRIPTMGR_HPP_

#include "VecMath.hpp"

#include <QObject>
#include <QtScript/QtScript>
#include <QStringList>
#include <QFile>
#include <QTime>

class StelMainScriptAPI;

#ifdef ENABLE_SCRIPT_CONSOLE
class ScriptConsole;
#endif

//! Manage scripting in Stellarium
class StelScriptMgr : public QObject
{
    Q_OBJECT

#ifdef ENABLE_SCRIPT_CONSOLE
    friend class ScriptConsole;
#endif

public:
    StelScriptMgr(QObject *parent=0);
    ~StelScriptMgr();

    QStringList getScriptList(void);

    //! Find out if a script is running
    //! @return true if a script is running, else false
    bool scriptIsRunning(void);
    //! Get the ID (filename) of the currently running script
    //! @return Empty string if no script is running, else the
    //! ID of the script which is running.
    QString runningScriptId(void);

    //ASAF
    bool runScriptNetwork(QString sScript);
    bool runScript2(const QString& fileName, const QString& includePath="");

    StelMainScriptAPI* getMainApi() { return mainAPI;}
public slots:
    //! Gets a single line name of the script.
    //! @param s the file name of the script whose name is to be returned.
    //! @return text following a comment with Name: at the start.  If no
    //! such comment is found, the file name will be returned.  If the file
    //! is not found or cannot be opened for some reason, an Empty string
    //! will be returned.
    const QString getName(const QString& s);

    //! Gets the name of the script Author
    //! @param s the file name of the script whose name is to be returned.
    //! @return text following a comment with Author: at the start.  If no
    //! such comment is found, "" is returned.  If the file
    //! is not found or cannot be opened for some reason, an Empty string
    //! will be returned.
    const QString getAuthor(const QString& s);

    //! Gets the licensing terms for the script
    //! @param s the file name of the script whose name is to be returned.
    //! @return text following a comment with License: at the start.  If no
    //! such comment is found, "" is returned.  If the file
    //! is not found or cannot be opened for some reason, an Empty string
    //! will be returned.
    const QString getLicense(const QString& s);

    //! Gets a description of the script.
    //! @param s the file name of the script whose name is to be returned.
    //! @return text following a comment with Description: at the start.
    //! The description is considered to be over when a line with no comment
    //! is found.  If no such comment is found, QString("") is returned.
    //! If the file is not found or cannot be opened for some reason, an
    //! Empty string will be returned.
    const QString getDescription(const QString& s);

    //! Run the script located at the given location
    //! @param fileName the location of the file containing the script.
    //! @param includePath the directory to use when searching for include files
    //! in the SSC preprocessor.  Usually this will be the same as the
    //! script file itself, but if you're running a generated script from
    //! a temp directory, but want to include a file from elsewhere, it
    //! can be usetul to set it to something else (e.g. in ScriptConsole).
    //! @return false if the named script could not be run, true otherwise
    bool runScript(const QString& fileName, const QString& includePath="");

    //! Stops any running script.
    //! @return false if no script was running, true otherwise.
    bool stopScript(void);

    void setScriptRate(double r);
    double getScriptRate(void);

    //! cause the emission of the scriptDebug signal.  This is so that functions in
    //! StelMainScriptAPI can explicitly send information to the ScriptConsole
    void debug(const QString& msg);

private slots:
    //! Called at the end of the running thread
    void scriptEnded();
signals:
    //! Notification when a script starts running
    void scriptRunning();
    //! Notification when a script has stopped running
    void scriptStopped();
    //! Notification of a script event - warnings, current execution line etc.
    void scriptDebug(const QString&);

private:
    // Utility functions for preprocessor
    QMap<QString, QString> mappify(const QStringList& args, bool lowerKey=false);
    bool strToBool(const QString& str);
    // Pre-processor functions
    bool preprocessScript(QFile& input, QFile& output, const QString& scriptDir);

#ifdef ENABLE_STRATOSCRIPT_COMPAT
    bool preprocessStratoScript(QFile& input, QFile& output, const QString& scriptDir);
#endif

    //! This function is for use with getName, getAuthor and getLicense.
    //! @param s the script id
    //! @param id the command line id, e.g. "Name"
    //! @param notFoundText the text to be returned if the key is not found
    //! @return the text following the id and : on a comment line near the top of
    //! the script file (i.e. before there is a non-comment line).
    const QString getHeaderSingleLineCommentText(const QString& s, const QString& id, const QString& notFoundText="");
    QScriptEngine engine;

    //! The thread in which scripts are run
    class StelScriptThread* thread;
    StelMainScriptAPI *mainAPI;
};

#endif // _STELSCRIPTMGR_HPP_
