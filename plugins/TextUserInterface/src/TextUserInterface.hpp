/*
 * Copyright (C) 2009 Matthew Gates
 *
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
 */
 
#ifndef TEXTUSERINTERFACE_HPP_
#define _TEXTUSERINTERFACE_HPP_ 1

#include "StelModule.hpp"
#include "DummyDialog.hpp"
#include <QObject>
#include <QString>
#include <QFont>

class TuiNode;

//! This is an example of a plug-in which can be dynamically loaded into stellarium
class TextUserInterface : public StelModule
{
	Q_OBJECT
public:
	TextUserInterface();
	virtual ~TextUserInterface();
	
	///////////////////////////////////////////////////////////////////////////
	// Methods defined in the StelModule class
	virtual void init();
	virtual void update(double deltaTime) {;}
	virtual void draw(StelCore* core);
	virtual double getCallOrder(StelModuleActionName actionName) const;
	virtual void handleKeys(class QKeyEvent* event);

private slots:
	void setHomePlanet(QString planetName);
	void setAltitude(int altitude);
	void setLatitude(double latitude);
	void setLongitude(double longitude);
	void setStartupDateMode(QString mode);
	void setDateFormat(QString format);
	void setTimeFormat(QString format);
	void setSkyCulture(QString i18);
	void setAppLanguage(QString lang);
	void saveDefaultSettings(void);

private:
	DummyDialog dummyDialog;
	QFont font;
	bool tuiActive;
	TuiNode* currentNode;

	double getLatitude(void);
	double getLongitude(void);
};


#include "fixx11h.h"
#include <QObject>
#include "StelPluginInterface.hpp"

//! This class is used by Qt to manage a plug-in interface
class TextUserInterfaceStelPluginInterface : public QObject, public StelPluginInterface
{
	Q_OBJECT
	Q_INTERFACES(StelPluginInterface)
public:
	virtual StelModule* getStelModule() const;
	virtual StelPluginInfo getPluginInfo() const;
};

#endif /*_TEXTUSERINTERFACE_HPP_*/