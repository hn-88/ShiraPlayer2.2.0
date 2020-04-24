/*
 * ShiraPlayer(TM)
 * Copyright (C) 2009 Fabien Chereau
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

#include "StelGuiBase.hpp"
#include "StelAppGraphicsWidget.hpp"
#include "StelTranslator.hpp"
#include <QAction>


StelGuiBase::StelGuiBase() : stelAppGraphicsWidget(NULL)
{
}

void StelGuiBase::init(QGraphicsWidget* atopLevelGraphicsWidget, StelAppGraphicsWidget* astelAppGraphicsWidget)
{
	stelAppGraphicsWidget = astelAppGraphicsWidget;
}

void StelGuiBase::updateI18n()
{
	// Translate all action texts
	foreach (QObject* obj, stelAppGraphicsWidget->children())
	{
		QAction* a = qobject_cast<QAction*>(obj);
		if (a)
		{
			const QString& englishText = a->property("englishText").toString();
			if (!englishText.isEmpty())
			{
				a->setText(q_(englishText));
			}
		}
	}
}

// Note: "text" and "helpGroup" must be in English -- this method and the help
// dialog take care of translating them. Of course, they still have to be
// marked for translation using the N_() macro.
QAction* StelGuiBase::addGuiActions(const QString& actionName, const QString& text, const QString& shortCut, const QString& helpGroup, bool checkable, bool autoRepeat)
{
	QAction* a;
	a = new QAction(stelAppGraphicsWidget);
	a->setObjectName(actionName);
	a->setText(q_(text));
	QList<QKeySequence> shortcuts;
	QRegExp shortCutSplitRegEx(",(?!,|$)");
	QStringList shortcutStrings = shortCut.split(shortCutSplitRegEx);
	for (int i = 0; i < shortcutStrings.size(); ++i)
		shortcuts << QKeySequence(shortcutStrings.at(i).trimmed());

	a->setShortcuts(shortcuts);
	a->setCheckable(checkable);
	a->setAutoRepeat(autoRepeat);
	a->setProperty("englishText", QVariant(text));
	a->setShortcutContext(Qt::WidgetShortcut);
	stelAppGraphicsWidget->addAction(a);
	return a;
}

QAction* StelGuiBase::getGuiActions(const QString& actionName)
{
	QAction* a = stelAppGraphicsWidget->findChild<QAction*>(actionName);
	if (!a)
	{
		qWarning() << "Can't find action " << actionName;
		return NULL;
	}
	return a;
}
