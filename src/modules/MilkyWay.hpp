/*
 * ShiraPlayer(TM)
 * Copyright (C) 2002 Fabien Chereau
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

#ifndef _MILKYWAY_HPP_
#define _MILKYWAY_HPP_

#include "StelModule.hpp"
#include "VecMath.hpp"
#include "StelTextureTypes.hpp"
#include "StelApp.hpp"

//! @class MilkyWay 
//! Manages the displaying of the Milky Way.
class MilkyWay : public StelModule
{
	Q_OBJECT

public:
	MilkyWay();
	virtual ~MilkyWay();
	
	///////////////////////////////////////////////////////////////////////////
	// Methods defined in the StelModule class
	//! Initialize the class.  Here we load the texture for the Milky Way and 
	//! get the display settings from application settings, namely the flag which
	//! determines if the Milky Way is displayed or not, and the intensity setting.
	virtual void init();

	//! Draw the Milky Way.
	virtual void draw(StelCore* core);
	
	//! Update and time-dependent state.  Updates the fade level while the 
	//! Milky way rendering is being changed from on to off or off to on.
	virtual void update(double deltaTime);
	
	//! Does nothing in the MilkyWay module.
	virtual void updateI18n() {;}
	
	//! Does nothing in the MilkyWay module.
	//! @param skyCultureDir the name of the directory containing the sky culture to use.
	virtual void updateSkyCulture(const QString& skyCultureDir) {;}
	
	//! Used to determine the order in which the various modules are drawn.
	virtual double getCallOrder(StelModuleActionName actionName) const {return 1.;}
	
	///////////////////////////////////////////////////////////////////////////////////////
	// Setter and getters
public slots:
	//! Get Milky Way intensity.
	float getIntensity() const {return intensity;}
	//! Set Milky Way intensity.
        void setIntensity(double aintensity) {
            intensity = aintensity;
            StelApp::getInstance().addNetworkCommand("MilkyWayMgr.setIntensity("+ QString("%0").arg(aintensity) +");");
        }
	
	//! Get the color used for rendering the milky way
	Vec3f getColor() const {return color;}
	//! Sets the color to use for rendering the milky way
	void setColor(const Vec3f& c) {color=c;}
	
	//! Sets whether to show the Milky Way
	void setFlagShow(bool b);
	//! Gets whether the Milky Way is displayed
	bool getFlagShow(void) const;
	
private:
	float radius;
	StelTextureSP tex;
	Vec3f color;
        double intensity;
	class LinearFader* fader;
};

#endif // _MILKYWAY_HPP_
