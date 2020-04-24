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

#include "StelProjector.hpp"
#include "StelPainter.hpp"
#include "StelApp.hpp"
#include "StelCore.hpp"
#include "StelFileMgr.hpp"
#include "StelLocaleMgr.hpp"
#include "StelModuleMgr.hpp"
#include "StelGui.hpp"
#include "StelGuiItems.hpp"
#include "StelIniParser.hpp"
#include "StelVertexArray.hpp"
#include "AngleMeasure.hpp"

#include <QtOpenGL>
#include <QDebug>
#include <QTimer>
#include <QAction>
#include <QPixmap>
#include <QtNetwork>
#include <QSettings>
#include <QKeyEvent>
#include <QMouseEvent>
#include <cmath>

//! This method is the one called automatically by the StelModuleMgr just
//! after loading the dynamic library
StelModule* AngleMeasureStelPluginInterface::getStelModule() const
{
	return new AngleMeasure();
}

StelPluginInfo AngleMeasureStelPluginInterface::getPluginInfo() const
{
	// Allow to load the resources when used as a static plugin
	Q_INIT_RESOURCE(AngleMeasure);

	StelPluginInfo info;
	info.id = "AngleMeasure";
	info.displayedName = "Angle Measure";
	info.authors = "Matthew Gates";
	info.contact = "http://porpoisehead.net/";
	info.description = "Provides an angle measurement tool";
	return info;
}

Q_EXPORT_PLUGIN2(AngleMeasure, AngleMeasureStelPluginInterface)


AngleMeasure::AngleMeasure()
	: flagShowAngleMeasure(false), lineVisible(false), dragging(false),
	  angleText(""), flagUseDmsFormat(false), pxmapGlow(NULL), pxmapOnIcon(NULL),
	  pxmapOffIcon(NULL), toolbarButton(NULL)
{
	setObjectName("AngleMeasure");
	font.setPixelSize(16);

	messageTimer = new QTimer(this);
	messageTimer->setInterval(7000);
	messageTimer->setSingleShot(true);

	connect(messageTimer, SIGNAL(timeout()), this, SLOT(clearMessage()));

	QSettings* conf = StelApp::getInstance().getSettings();
	if (!conf->contains("AngleMeasure/angle_format_dms"))
	{
		// Create the "AngleMeasure" section and set default parameters
		conf->setValue("AngleMeasure/angle_format_dms", true);
		conf->setValue("AngleMeasure/text_color", "0,0.5,1");
		conf->setValue("AngleMeasure/line_color", "0,0.5,1");
	}

	flagUseDmsFormat = conf->value("options/angle_format_dms", false).toBool();
	textColor = StelUtils::strToVec3f(conf->value("options/text_color", "0,0.5,1").toString());
	lineColor = StelUtils::strToVec3f(conf->value("options/line_color", "0,0.5,1").toString());
}

AngleMeasure::~AngleMeasure()
{
}

//! Determine which "layer" the plagin's drawing will happen on.
double AngleMeasure::getCallOrder(StelModuleActionName actionName) const
{
	if (actionName==StelModule::ActionDraw)
		return StelApp::getInstance().getModuleMgr().getModule("LandscapeMgr")->getCallOrder(actionName)+10.;
	if (actionName==StelModule::ActionHandleMouseClicks)
		return -11;
	return 0;
}

void AngleMeasure::init()
{
	qDebug() << "AngleMeasure plugin - press control-A to toggle angle measure mode";
	startPoint.set(0.,0.,0.);
	endPoint.set(0.,0.,0.);
	perp1StartPoint.set(0.,0.,0.);
	perp1EndPoint.set(0.,0.,0.);
	perp2StartPoint.set(0.,0.,0.);
	perp2EndPoint.set(0.,0.,0.);

	// create action for enable/disable & hook up signals
	StelGui* gui = dynamic_cast<StelGui*>(StelApp::getInstance().getGui());
	Q_ASSERT(gui);
	gui->addGuiActions("actionShow_Angle_Measure", N_("Angle measure"), "Ctrl+A", "Plugin Key Bindings", true, false);
	gui->getGuiActions("actionShow_Angle_Measure")->setChecked(flagShowAngleMeasure);
	connect(gui->getGuiActions("actionShow_Angle_Measure"), SIGNAL(toggled(bool)), this, SLOT(enableAngleMeasure(bool)));

	// Add a toolbar button
	try
	{
		pxmapGlow = new QPixmap(":/graphicGui/gui/glow32x32.png");
		pxmapOnIcon = new QPixmap(":/angleMeasure/bt_anglemeasure_on.png");
		pxmapOffIcon = new QPixmap(":/angleMeasure/bt_anglemeasure_off.png");
		toolbarButton = new StelButton(NULL, *pxmapOnIcon, *pxmapOffIcon, *pxmapGlow, gui->getGuiActions("actionShow_Angle_Measure"));
		gui->getButtonBar()->addButton(toolbarButton, "065-pluginsGroup");
	}
	catch (std::runtime_error& e)
	{
		qWarning() << "WARNING: unable create toolbar button for AngleMeasure plugin: " << e.what();
	}
}

void AngleMeasure::update(double deltaTime)
{
	if (!messageFader && messageFader.getInterstate() <= 0.)
		return;

	messageFader.update((int)(deltaTime*1000));
}

//! Draw any parts on the screen which are for our module
void AngleMeasure::draw(StelCore* core)
{
	const StelProjectorP prj = core->getProjection(StelCore::FrameEquinoxEqu);
	StelPainter painter(prj);
	painter.setFont(font);

	if (lineVisible || flagShowAngleMeasure)
	{
		glDisable(GL_TEXTURE_2D);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_BLEND);
		glEnable(GL_LINE_SMOOTH);
	}

	if (lineVisible)
	{
		Vec3d xy;
		if (prj->project(perp1EndPoint,xy))
		{
			glColor3fv(textColor);
			painter.drawText(xy[0], xy[1], angleText, 0, 15, 15);
		}

		// main line is a great circle
		glColor3fv(lineColor);
		painter.drawGreatCircleArc(startPoint, endPoint, NULL);

		// End lines
		painter.drawGreatCircleArc(perp1StartPoint, perp1EndPoint, NULL);
		painter.drawGreatCircleArc(perp2StartPoint, perp2EndPoint, NULL);
	}

	if (messageFader.getInterstate() > 0.)
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glColor4f(textColor[0], textColor[1], textColor[2], messageFader.getInterstate());
		painter.drawText(83, 120, "Angle Tool Enabled - left drag to measure, left click to clear");
		painter.drawText(83, 95,  "right click to change end point only");
	}
}

void AngleMeasure::handleKeys(QKeyEvent* event)
{
		event->setAccepted(false);
}

void AngleMeasure::handleMouseClicks(class QMouseEvent* event)
{
	if (!flagShowAngleMeasure)
	{
		event->setAccepted(false);
		return;
	}

	if (event->type()==QEvent::MouseButtonPress && event->button()==Qt::LeftButton)
	{
		const StelProjectorP prj = StelApp::getInstance().getCore()->getProjection(StelCore::FrameEquinoxEqu);
		prj->unProject(event->x(),event->y(),startPoint);

		// first click reset the line... only draw it after we've dragged a little.
		if (!dragging)
		{
			lineVisible = false;
			endPoint = startPoint;
		}
		else
			lineVisible = true;

		dragging = true;
		calculateEnds();
		event->setAccepted(true);
		return;
	}
	else if (event->type()==QEvent::MouseButtonRelease && event->button()==Qt::LeftButton)
	{
		dragging = false;
		calculateEnds();
		event->setAccepted(true);
		return;
	}
	else if (event->type()==QEvent::MouseButtonPress && event->button()==Qt::RightButton)
	{
		const StelProjectorP prj = StelApp::getInstance().getCore()->getProjection(StelCore::FrameEquinoxEqu);
		prj->unProject(event->x(),event->y(),endPoint);
		calculateEnds();
		event->setAccepted(true);
		return;
	}
	event->setAccepted(false);
}

bool AngleMeasure::handleMouseMoves(int x, int y, Qt::MouseButtons b)
{
	if (dragging)
	{
		const StelProjectorP prj = StelApp::getInstance().getCore()->getProjection(StelCore::FrameEquinoxEqu);
		prj->unProject(x,y,endPoint);
		calculateEnds();
		lineVisible = true;
		return true;
	}
	else
		return false;
}

void AngleMeasure::calculateEnds(void)
{
		Vec3d v0 = endPoint - startPoint;
		Vec3d v1 = Vec3d(0,0,0) - startPoint;
		Vec3d p = v0 ^ v1;
		p *= 0.08;  // end width
		perp1StartPoint.set(startPoint[0]-p[0],startPoint[1]-p[1],startPoint[2]-p[2]);
		perp1EndPoint.set(startPoint[0]+p[0],startPoint[1]+p[1],startPoint[2]+p[2]);

		v1 = Vec3d(0,0,0) - endPoint;
		p = v0 ^ v1;
		p *= 0.08;  // end width
		perp2StartPoint.set(endPoint[0]-p[0],endPoint[1]-p[1],endPoint[2]-p[2]);
		perp2EndPoint.set(endPoint[0]+p[0],endPoint[1]+p[1],endPoint[2]+p[2]);

		unsigned int d, m;
		double s;
		bool sign;
		StelUtils::radToDms(startPoint.angle(endPoint), sign, d, m, s);
		if (flagUseDmsFormat)
			angleText = QString("%1d %2m %3s").arg(d).arg(m).arg(s, 0, 'f', 2);
		else
			angleText = QString("%1%2 %3' %4\"").arg(d).arg(QChar(0x00B0)).arg(m).arg(s, 0, 'f', 2);
}

void AngleMeasure::enableAngleMeasure(bool b)
{
	flagShowAngleMeasure = b;
	messageFader = b;
	if (b)
	{
		qDebug() << "AngleMeasure::enableAngleMeasure starting timer";
		messageTimer->start();
	}
}

void AngleMeasure::clearMessage()
{
	qDebug() << "AngleMeasure::clearMessage";
	messageFader = false;
}

