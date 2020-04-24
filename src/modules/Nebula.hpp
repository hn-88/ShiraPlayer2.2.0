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

#ifndef _NEBULA_HPP_
#define _NEBULA_HPP_

#include <QString>
#include "StelObject.hpp"
#include "StelTranslator.hpp"
#include "StelTextureTypes.hpp"

class StelPainter;
class QDataStream;

class Nebula : public StelObject
{
friend class NebulaMgr;
public:
	Nebula();
	~Nebula();

	//! Nebula support the following InfoStringGroup flags:
	//! - Name
	//! - CatalogNumber
	//! - Magnitude
	//! - RaDec
	//! - AltAzi
	//! - Distance
	//! - Size
	//! - Extra1 (contains the Nebula type, which might be "Galaxy", "Cluster" or similar)
	//! - PlainText
	//! @param core the Stelore object
	//! @param flags a set of InfoStringGroup items to include in the return value.
	//! @return a QString containing an HMTL encoded description of the Nebula.
	virtual QString getInfoString(const StelCore *core, const InfoStringGroup& flags) const;
	virtual QString getType(void) const {return "Nebula";}
	virtual Vec3d getJ2000EquatorialPos(const StelNavigator *nav) const {return XYZ;}
	virtual double getCloseViewFov(const StelNavigator * nav = NULL) const;
	virtual float getVMagnitude(const StelNavigator * nav = NULL) const {return mag;}
	virtual float getSelectPriority(const StelNavigator *nav) const;
	virtual Vec3f getInfoColor(void) const;
	virtual QString getNameI18n(void) const {return nameI18;}
	virtual QString getEnglishName(void) const {return englishName;}
	virtual double getAngularSize(const StelCore *core) const {return angularSize*0.5;}
	virtual SphericalRegionP getRegion() const {return pointRegion;}

	// Methods specific to Nebula
	void setLabelColor(const Vec3f& v) {labelColor = v;}
	void setCircleColor(const Vec3f& v) {circleColor = v;}

	//! Get the printable nebula Type.
	//! @return the nebula type code.
	QString getTypeString(void) const;

private:
	friend struct DrawNebulaFuncObject;
	
	//! @enum NebulaType Nebula types
	enum NebulaType
	{
		NebGx=0,     //!< Galaxy
		NebOc=1,     //!< Open star cluster
		NebGc=2,     //!< Globular star cluster, usually in the Milky Way Galaxy
		NebN=3,      //!< Bright emission or reflection nebula
		NebPn=4,     //!< Planetary nebula
		NebDn=5,     //!< ???
		NebIg=6,     //!< ???
		NebCn=7,     //!< Cluster associated with nebulosity
		NebUnknown=8 //!< Unknown type
	};

	//! Translate nebula name using the passed translator
    void translateName(StelTranslator& trans) {nameI18 = trans.qtranslate(englishName);}

	bool readNGC(char *record);
	void readNGC(QDataStream& in);
			
	void drawLabel(StelPainter& sPainter, float maxMagLabel);
	void drawHints(StelPainter& sPainter, float maxMagHints);

	unsigned int M_nb;              // Messier Catalog number
	unsigned int NGC_nb;            // New General Catalog number
	unsigned int IC_nb;             // Index Catalog number
    QString englishName;            // English name
	QString nameI18;                // Nebula name
	float mag;                      // Apparent magnitude
	float angularSize;              // Angular size in degree
	Vec3d XYZ;                      // Cartesian equatorial position
	Vec3d XY;                       // Store temporary 2D position
	NebulaType nType;

	SphericalRegionP pointRegion;

	static StelTextureSP texCircle;   // The symbolic circle texture
	static float hintsBrightness;

	static Vec3f labelColor, circleColor;
	static float circleScale;       // Define the scaling of the hints circle
};

#endif // _NEBULA_HPP_

