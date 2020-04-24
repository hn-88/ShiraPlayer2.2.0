/*
 * ShiraPlayer(TM)
 * Copyright (C) 2007 Matthew Gates
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

#include <QString>
#include <QTextBrowser>
#include <QVBoxLayout>
#include <QWidget>
#include <QFrame>
#include <QSettings>
#include <QResizeEvent>
#include <QSize>
#include <QMultiMap>
#include <QList>
#include <QSet>
#include <QPair>
#include <QtAlgorithms>
#include <QDebug>

#include "ui_helpDialogGui.h"

#include "HelpDialog.hpp"
#include "StelApp.hpp"
#include "StelFileMgr.hpp"
#include "StelLocaleMgr.hpp"
#include "StelStyle.hpp"
#include "StelLogger.hpp"

HelpDialog::HelpDialog()
{
    ui = new Ui_helpDialogForm;

    // Make some key and mouse bindings translatable. Keys starting with
    // "!NUMBER-" are made up; the number is there to keep the entries
    // sorted (at least relative to each other).
    specialKeys["Space"] = N_("Space");
    specialKeys["!01-arrows-and-left-drag"] = N_("Arrow keys & left mouse drag");
    specialKeys["!02-page-up/down"] = N_("Page Up/Down");
    specialKeys["!03-ctrl+up/down"] = N_("CTRL + Up/Down");
    specialKeys["!04-left-click"] = N_("Left click");
    specialKeys["!05-right-click"] = N_("Right click");
    specialKeys["!06-ctrl+left-click"] = N_("CTRL + Left click");

    // Add keys for those keys which do not have actions.
    QString group = N_("Movement and Selection");
    setKey(group, "", "!01-arrows-and-left-drag", N_("Pan view around the sky"));
    setKey(group, "", "!02-page-up/down", N_("Zoom in/out"));
    setKey(group, "", "!03-ctrl+up/down", N_("Zoom in/out"));
    setKey(group, "", "!04-left-click", N_("Select object"));
    setKey(group, "", "!05-right-click", N_("Clear selection"));
#ifdef Q_OS_MAC
    setKey(group, "", "!06-ctrl+left-click", N_("Clear selection"));
#endif

    group = N_("When a Script is Running");
    setKey(group, "", "J", N_("Slow down the script execution rate"));
    setKey(group, "", "L", N_("Speed up the script execution rate"));
    setKey(group, "", "K", N_("Set the normal script execution rate"));
}

HelpDialog::~HelpDialog()
{
    delete ui;
    ui = NULL;
}

void HelpDialog::languageChanged()
{
    if (dialog)
    {
        //ui->retranslateUi(dialog);

        ui->stelWindowTitle->setText(q_("Help"));
        ui->closeStelWindow->setText(QString());
        const bool __sortingEnabled = ui->stackListWidget->isSortingEnabled();
        ui->stackListWidget->setSortingEnabled(false);
        QListWidgetItem *___qlistwidgetitem = ui->stackListWidget->item(0);
        ___qlistwidgetitem->setText(q_("Help"));
        QListWidgetItem *___qlistwidgetitem1 = ui->stackListWidget->item(1);
        ___qlistwidgetitem1->setText(q_("About"));
        QListWidgetItem *___qlistwidgetitem2 = ui->stackListWidget->item(2);
        ___qlistwidgetitem2->setText(q_("Log"));
        ui->stackListWidget->setSortingEnabled(__sortingEnabled);
        ui->refreshButton->setText(q_("Refresh"));

        updateText();
    }
}

void HelpDialog::styleChanged()
{
    if (dialog)
    {
        updateText();
    }
}

void HelpDialog::createDialogContent()
{
    ui->setupUi(dialog);
    ui->stackedWidget->setCurrentIndex(0);
    ui->stackListWidget->setCurrentRow(0);
    connect(ui->closeStelWindow, SIGNAL(clicked()), this, SLOT(close()));

    updateText();

    ui->logPathLabel->setText(QString("%1/log.txt:").arg(StelFileMgr::getUserDir()));
    connect(ui->stackedWidget, SIGNAL(currentChanged(int)), this, SLOT(updateLog(int)));
    connect(ui->refreshButton, SIGNAL(clicked()), this, SLOT(refreshLog()));

    connect(ui->stackListWidget, SIGNAL(currentItemChanged(QListWidgetItem *, QListWidgetItem *)), this, SLOT(changePage(QListWidgetItem *, QListWidgetItem*)));

    //ui->stackListWidget->item(1)->setHidden(true);
}
void HelpDialog::prepareAsChild()
{
    ui->HelpBar->hide();
    updateText();
}
void HelpDialog::setHtmlText(QString htmlText)
{
    ui->helpBrowser->clear();
    ui->helpBrowser->document()->setDefaultStyleSheet(QString(StelApp::getInstance().getCurrentStelStyle()->htmlStyleSheet));
    ui->helpBrowser->insertHtml(htmlText);
    ui->helpBrowser->scrollToAnchor("top");
}

void HelpDialog::updateLog(int)
{
    if(ui->stackedWidget->currentWidget() == ui->page_3)
        refreshLog();
}

void HelpDialog::refreshLog()
{
    ui->logBrowser->setPlainText(StelLogger::getLog());
}

void HelpDialog::setKey(QString group, QString oldKey, QString newKey, QString description)
{
    // For adding keys like this, the choice of a QMultiMap seems like
    // madness.  However, when we update the text it does the grouping
    // for us... we have to live with ugliness in one of these functions
    // and it seems easier here.

    // For new key bindings we just insert and return
    if (oldKey.isEmpty())
    {
        keyData.insert(group, QPair<QString, QString>(newKey, description));
        // if (ui->helpBrowser!=NULL)
        // 	this->updateText();
        return;
    }

    // Else delete the old entry if we can find it, and then insert the
    // new entry.  Here's where the multimap makes us wince...
    QMultiMap<QString, QPair<QString, QString> >::iterator i = keyData.begin();
    while (i != keyData.end()) {
        QMultiMap<QString, QPair<QString, QString> >::iterator prev = i;
        ++i;
        if (prev.value().first == oldKey)
        {
            keyData.erase(prev);
        }
    }

    keyData.insert(group, QPair<QString, QString>(newKey, description));
    // if (ui->helpBrowser!=NULL)
    // 	this->updateText();
}

QString HelpDialog::getHeaderText(void)
{
    return "<html><head><title>" + QString(q_("Shiraplayer Help")).toHtmlEscaped() + "</title></head><body>\n"
            + "<h2>" + QString(q_("Keys")).toHtmlEscaped() + "</h2>\n";
}

QString HelpDialog::getFooterText(void)
{
    // Regexp to replace {text} with an HTML link.
    QRegExp a_rx = QRegExp("[{]([^{]*)[}]");

    QString footer = "<h2>" + QString(q_("www.sureyyasoft.com")).toHtmlEscaped() + "</h2>\n";

    //	QString footer = "<h2>" + q_("Further Reading")) + "</h2>\n";
    //	footer += q_("The following links are external web links, and will launch your web browser:\n"));
    //	footer += "<p><a href=\"http://stellarium.org/wiki/index.php/Category:User%27s_Guide\">" + q_("The Stellarium User Guide")) + "</a>";
    //
    //	footer += "<p>";
    //	// TRANSLATORS: The text between braces is the text of an HTML link.
    //	footer += q_("{Frequently Asked Questions} about Stellarium.  Answers too.")).replace(a_rx, "<a href=\"http://www.stellarium.org/wiki/index.php/FAQ\">\\1</a>");
    //	footer += "</p>\n";
    //
    //	footer += "<p>";
    //	// TRANSLATORS: The text between braces is the text of an HTML link.
    //	footer += q_("{The Stellarium Wiki} - General information.  You can also find user-contributed landscapes and scripts here.")).replace(a_rx, "<a href=\"http://stellarium.org/wiki/\">\\1</a>");
    //	footer += "</p>\n";
    //
    //	footer += "<p>";
    //	// TRANSLATORS: The text between braces is the text of an HTML link.
    //	footer += q_("{Support ticket system} - if you need help using Stellarium, post a support request here and we'll try to help.")).replace(a_rx, "<a href=\"http://answers.launchpad.net/stellarium/+addquestion\">\\1</a>");
    //	footer += "</p>\n";
    //
    //	footer += "<p>";
    //	// TRANSLATORS: The text between braces is the text of an HTML link.
    //	footer += q_("{Bug reporting and feature request system} - if something doesn't work properly or is missing and is not listed in the tracker, you can open bug reports here.")).replace(a_rx, "<a href=\"http://bugs.launchpad.net/stellarium/\">\\1</a>");
    //	footer += "</p>\n";
    //
    //	footer += "<p>";
    //	// TRANSLATORS: The text between braces is the text of an HTML link.
    //	footer += q_("{Forums} - discuss Stellarium with other users.")).replace(a_rx, "<a href=\"http://sourceforge.net/forum/forum.php?forum_id=278769\">\\1</a>");
    //	footer += "</p>\n";

    footer += "</body></html>\n";

    return footer;
}

void HelpDialog::updateText(void)
{
    // Here's how we will build the help text for the keys:
    // 1.  Get a unique list of groups by asking for the keys and then converting the
    //     resulting QList into a QSet.
    // 2   Converet back to a QList, sort and move the empty string to the end of the
    //     list if it is present (this is the miscellaneous group).
    // 3   Iterate over the QSet of groups names doing:
    // 3.1  add the group title
    // 3.2  Use QMultiMap::values(key) to get a list of QPair<QString, QString>
    //      which describe the key binding (QPair::first) and the help text for
    //      that key binding (QPair::second).
    // 3.3  Sort this list by the first value in the pait, courtesy of qSort and
    //      HelpDialog::helpItemSort
    // 3.4  Iterate over the sorted list adding key and description for each item

    newHtml = QString(getHeaderText());
    newHtml += "<table cellpadding=\"10%\">\n";

    QList<QString> groups = keyData.keys().toSet().toList(); // 1 + 2
    qSort(groups.begin(), groups.end(), HelpDialog::helpGroupSort);

    // 3
    QString lastGroup;  // to group "" and "Miscellaneous into one
    foreach (QString group, groups)
    {
        QString groupDescription = group;
        if (group.isEmpty())
            groupDescription = N_("Miscellaneous");

        if (lastGroup!=groupDescription)
        {
            // 3.1
            newHtml += "<tr></tr><tr><td><b><u>" + QString(q_(groupDescription)).toHtmlEscaped() + ":</u></b></td></tr>\n";
        }
        lastGroup = groupDescription;

        // 3.2
        QList< QPair<QString, QString> > keys = keyData.values(group);

        // 3.3
        qSort(keys.begin(), keys.end(), HelpDialog::helpItemSort);

        // 3.4
        for(int i=0; i<keys.size(); i++)
        {
            QString key = keys.at(i).first; // the string which holds the key, e.g. "F1"

            // For some keys we need to translate from th QT string to something
            // more readable
            QString specKey = specialKeys[key];
            if (!specKey.isEmpty())
                key = q_(specKey);

            // Finally, add HTML table data for the key as it's help text
            newHtml += "<tr><td><b>" + QString(key).toHtmlEscaped() + "</b></td>";
            newHtml += "<td>" + QString(q_( keys.at(i).second)).toHtmlEscaped() + "</td></tr>\n";
        }
    }

    newHtml += "</table>";
    newHtml += getFooterText();
    ui->helpBrowser->clear();
    ui->helpBrowser->document()->setDefaultStyleSheet(QString(StelApp::getInstance().getCurrentStelStyle()->htmlStyleSheet));
    ui->helpBrowser->insertHtml(newHtml);
    ui->helpBrowser->scrollToAnchor("top");


    // populate About tab
    QString newHtml1 = "<h1>" + StelUtils::getApplicationName() + "</h1>";
    // Note: this legal notice is not suitable for traslation
   //newHtml += QLatin1String("<img src=\"mypics://theCurrentPicture.png\" />")
    newHtml1 += "<h3>" + q_("ShiraPlayer is open source software based on Stellarium 0.10.2 version") + "</h3>";
    newHtml1 += "<h3>Copyright &copy; 2009-2015 Asaf Yurdakul</h3>";
    //newHtml1 += "<h3>Copyright &copy; 2000-2009 Stellarium Developers</h3>";
    newHtml1 += "<p>This program is free software; you can redistribute it and/or ";
    newHtml1 += "modify it under the terms of the GNU General Public License ";
    newHtml1 += "as published by the Free Software Foundation; either version 2 ";
    newHtml1 += "of the License, or (at your option) any later version.</p>";
    newHtml1 += "<p>This program is distributed in the hope that it will be useful, ";
    newHtml1 += "but WITHOUT ANY WARRANTY; without even the implied ";
    newHtml1 += "warranty of MERCHANTABILITY or FITNESS FOR A ";
    newHtml1 += "PARTICULAR PURPOSE.  See the GNU General Public ";
    newHtml1 += "License for more details.</p>";
    newHtml1 += "<p>You should have received a copy of the GNU General Public ";
    newHtml1 += "License along with this program; if not, write to:</p>";
    newHtml1 += "<pre>Free Software Foundation, Inc.\n";
    newHtml1 += "59 Temple Place - Suite 330\n";
    newHtml1 += "Boston, MA  02111-1307, USA.\n</pre>";
    newHtml1 += "<h3>" + q_("ShiraPlayer Developer") + "</h3><ul>";
    newHtml1 += "<li>" + q_("Project coordinator & lead developer,Graphic/other designer: %1").arg(QString("Asaf Yurdakul")) + "</li><ul>";
    //newHtml1 += "<p>" + q_("----------------------") + "</p><ul>";
   /* newHtml += "<h3>" + q_("Stellarium Developers") + "</h3><ul>";
    newHtml += "<li>" + q_("Project coordinator & lead developer: %1").arg(QString("Fabien Ch%1reau").arg(QChar(0x00E9))) + "</li>";
    newHtml += "<li>" + q_("Doc author/developer: %1").arg(QString("Matthew Gates")) + "</li>";
    newHtml += "<li>" + q_("Graphic/other designer: %1").arg(QString("Johan Meuris")) + "</li>";
    newHtml += "<li>" + q_("Developer: %1").arg(QString("Johannes Gajdosik")) + "</li>";
    newHtml += "<li>" + q_("Developer: %1").arg(QString("Rob Spearman")) + "</li>";
    newHtml += "<li>" + q_("OSX Developer: %1").arg(QString("Nigel Kerr")) + "</li>";
    newHtml += "<li>" + q_("Developer: %1").arg(QString("Andr%1s Mohari").arg(QChar(0x00E1))) + "</li>";
    newHtml += "<li>" + q_("Developer: %1").arg(QString("Mike Storm")) + "</li><ul><p>";*/
    ui->aboutBrowser->clear();
 //   ui->aboutBrowser->document()->setDefaultStyleSheet(QString(gui->getStelStyle().htmlStyleSheet));
    ui->aboutBrowser->insertHtml(newHtml1);
    ui->aboutBrowser->scrollToAnchor("top");
}

bool HelpDialog::helpItemSort(const QPair<QString, QString>& p1, const QPair<QString, QString>& p2)
{
    // To be 100% proper, we should sort F1 F2 F11 F12 in that order, although
    // right now we will get F1 F11 F12 F2.  However, at time of writing, no group
    // of keys has F1-F9, and one from F10-F12 in it, so it doesn't really matter.
    // -MNG 2008-06-01
    if (p1.first.split(",").at(0).size()!=p2.first.split(",").at(0).size())
        return p1.first.size() < p2.first.size();
    else
        return p1.first < p2.first;
}

bool HelpDialog::helpGroupSort(const QString& s1, const QString& s2)
{
    QString s1c = s1.toUpper();
    QString s2c = s2.toUpper();

    if (s1c=="" || s1c==QString(N_("Miscellaneous")).toUpper())
        s1c = "ZZZ" + s1c;
    if (s2c=="" || s2c==QString(N_("Miscellaneous")).toUpper())
        s2c = "ZZZ" + s2c;
    if (s1c=="DEBUG")
        s1c = "ZZZZ" + s1c;
    if (s2c=="DEBUG")
        s2c = "ZZZZ" + s2c;

    return s1c < s2c;
}

void HelpDialog::retranslate()
{
    languageChanged();
}

void HelpDialog::changePage(QListWidgetItem *current, QListWidgetItem *previous)
{
    if (!current)
        current = previous;
    ui->stackedWidget->setCurrentIndex(ui->stackListWidget->row(current));
}