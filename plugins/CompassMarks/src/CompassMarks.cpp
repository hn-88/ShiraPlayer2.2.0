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

#include "VecMath.hpp"
#include "StelProjector.hpp"
#include "StelPainter.hpp"
#include "StelApp.hpp"
#include "StelCore.hpp"
#include "StelLocaleMgr.hpp"
#include "StelModuleMgr.hpp"
#include "StelUtils.hpp"
#include "StelFileMgr.hpp"
#include "LandscapeMgr.hpp"
#include "CompassMarks.hpp"
#include "StelGui.hpp"
#include "StelGuiItems.hpp"
#include "StelIniParser.hpp"

#include <QtOpenGL>
#include <QAction>
#include <QDebug>
#include <QPixmap>
#include <QSettings>
#include <QKeyEvent>
#include <QtNetwork>
#include <cmath>

//! This method is the one called automatically by the StelModuleMgr just
//! after loading the dynamic library
StelModule* CompassMarksStelPluginInterface::getStelModule() const
{
	return new CompassMarks();
}

StelPluginInfo CompassMarksStelPluginInterface::getPluginInfo() const
{
	// Allow to load the resources when used as a static plugin
	Q_INIT_RESOURCE(CompassMarks);

	StelPluginInfo info;
	info.id = "CompassMarks";
	info.displayedName = "Compass Marks";
	info.authors = "Matthew Gates";
	info.contact = "http://porpoisehead.net/";
	info.description = "Displays compass bearing marks along the horizon";
	return info;
}

Q_EXPORT_PLUGIN2(CompassMarks, CompassMarksStelPluginInterface)


CompassMarks::CompassMarks()
	: markColor(1,1,1), pxmapGlow(NULL), pxmapOnIcon(NULL), pxmapOffIcon(NULL), toolbarButton(NULL)
{
	setObjectName("CompassMarks");

	QSettings* conf = StelApp::getInstance().getSettings();
	// Setup defaults if not present
	if (!conf->contains("CompassMarks/mark_color"))
		conf->setValue("CompassMarks/mark_color", "1,0,0");

	if (!conf->contains("CompassMarks/font_size"))
		conf->setValue("CompassMarks/font_size", 10);

	// Load settings from main config file
	markColor = StelUtils::strToVec3f(conf->value("CompassMarks/mark_color", "1,0,0").toString());
	font.setPixelSize(conf->value("CompassMarks/font_size", 10).toInt());
}

CompassMarks::~CompassMarks()
{
	// TODO (requires work in core API)
	// 1. Remove button from toolbar
	// 2. Remove action from GUI
	// 3. Delete GUI objects.  I'll leave this commented right now because
	// unloading (when implemented) might cause problems if we do it before we
	// can do parts 1 and 2.
	//if (pxmapGlow!=NULL)
	//	delete pxmapGlow;
	//if (pxmapOnIcon!=NULL)
	//	delete pxmapOnIcon;
	//if (pxmapOffIcon!=NULL)
	//	delete pxmapOffIcon;
	//if (toolbarButton!=NULL)
	//	delete toolbarButton;
}

//! Determine which "layer" the plugin's drawing will happen on.
double CompassMarks::getCallOrder(StelModuleActionName actionName) const
{
	if (actionName==StelModule::ActionDraw)
		return StelApp::getInstance().getModuleMgr().getModule("LandscapeMgr")->getCallOrder(actionName)+10.;
	return 0;
}

void CompassMarks::init()
{
	qDebug() << "CompassMarks plugin - press control-C to toggle compass marks";
	try
	{
		StelGui* gui = dynamic_cast<StelGui*>(StelApp::getInstance().getGui());
		pxmapGlow = new QPixmap(":/graphicGui/gui/glow32x32.png");
		pxmapOnIcon = new QPixmap(":/compassMarks/bt_compass_on.png");
		pxmapOffIcon = new QPixmap(":/compassMarks/bt_compass_off.png");

		gui->addGuiActions("actionShow_Compass_Marks", N_("Compass marks"), "Ctrl+C", "Plugin Key Bindings", true, false);
		gui->getGuiActions("actionShow_Compass_Marks")->setChecked(markFader);
		toolbarButton = new StelButton(NULL, *pxmapOnIcon, *pxmapOffIcon, *pxmapGlow, gui->getGuiActions("actionShow_Compass_Marks"));
		gui->getButtonBar()->addButton(toolbarButton, "065-pluginsGroup");
		connect(gui->getGuiActions("actionShow_Compass_Marks"), SIGNAL(toggled(bool)), this, SLOT(setCompassMarks(bool)));
		connect(gui->getGuiActions("actionShow_Cardinal_Points"), SIGNAL(toggled(bool)), this, SLOT(cardinalPointsChanged(bool)));
		cardinalPointsState = false;
	}
	catch (std::runtime_error& e)
	{
		qWarning() << "WARNING: unable create toolbar button for CompassMarks plugin: " << e.what();
	}
}

//! Draw any parts on the screen which are for our module
void CompassMarks::draw(StelCore* core)
{
	if (markFader.getInterstate() <= 0.0) { return; }

	Vec3f pos;
	Vec3d xy;
	Vec3d xy2;
	StelProjectorP prj = core->getProjection(StelCore::FrameAltAz);
	StelPainter painter(prj);
	painter.setFont(font);

	glColor4f(markColor[0],markColor[1],markColor[2], markFader.getInterstate());
	glDisable(GL_TEXTURE_2D);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
	glEnable(GL_LINE_SMOOTH);
	for(int i=0; i<360; i++)
	{
		float a = i*M_PI/180;
		float h = -0.002;
		if (i % 15 == 0)
		{
			h = -0.02;  // the size of the mark every 15 degrees

			// draw a label every 15 degrees
			pos.set(sin(a),cos(a), 0.f);
			QString s = QString("%1").arg((i+90)%360);
			float shiftx = painter.getFontMetrics().width(s) / 2.;
			float shifty = painter.getFontMetrics().height() / 2;
			if (prj->project(pos,xy)) painter.drawText(xy[0], xy[1], s, 0, -shiftx, shifty);
		}
		else if (i % 5 == 0)
		{
			h = -0.01;  // the size of the marking every 5 degrees
		}

		painter.drawGreatCircleArc(Vec3d(sin(a), cos(a), 0), Vec3d(sin(a), cos(a), h), NULL);
	}
	glDisable(GL_LINE_SMOOTH);
}

void CompassMarks::update(double deltaTime)
{
	markFader.update((int)(deltaTime*1000));
}

void CompassMarks::setCompassMarks(bool b)
{
	markFader = b;
	if (markFader)
	{
		// Save the display state of the cardinal points
		cardinalPointsState = GETSTELMODULE(LandscapeMgr)->getFlagCardinalsPoints();
	}
	else
	{
	}

	if(cardinalPointsState)
	{
		// Using QActions instead of directly calling
		// setFlagCardinalsPoints() in order to sync with the buttons
		dynamic_cast<StelGui*>(StelApp::getInstance().getGui())->getGuiActions("actionShow_Cardinal_Points")->trigger();
	}
}

void CompassMarks::cardinalPointsChanged(bool b)
{
	if(b)
	{
		// If the compass marks are displayed when cardinal points
		// are about to be shown, hide the compass marks
		if(markFader)
		{
			cardinalPointsState = false; // actionShow_Cardinal_Points should not be triggered again
			dynamic_cast<StelGui*>(StelApp::getInstance().getGui())->getGuiActions("actionShow_Compass_Marks")->trigger();
		}
	}
}
