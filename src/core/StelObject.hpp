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

#ifndef _STELOBJECT_HPP_
#define _STELOBJECT_HPP_

#include <QString>
#include "VecMath.hpp"
#include "StelObjectType.hpp"
#include "StelRegionObject.hpp"

class StelNavigator;
class StelCore;

//! The base abstract class for sky objects used in Stellarium like Stars, Planets, Constellations etc...
//! Normally you should use StelObjectP instead of StelObject* which have by default the same behaviour,
//! but which can be added reference counting if needed.
//! @sa StelObjectP
class StelObject : public StelRegionObject
{
public:
    //! @enum InfoStringGroup used as named bitfield flags as specifiers to
    //! filter results of getInfoString. The precise definition of these should
    //! be documented in the getInfoString documentation for the derived classes
    //! for all specifiers which are defined in that derivative.
    enum InfoStringGroup
    {
        Name          = 0x00000001, //!< An object's name
        CatalogNumber = 0x00000002, //!< Catalog numbers
        Magnitude     = 0x00000004, //!< Magnitude related data
        RaDecJ2000    = 0x00000008, //!< The equatorial position (J2000 ref)
        RaDecOfDate   = 0x00000010, //!< The equatorial position (of date)
        AltAzi        = 0x00000020, //!< The position (Altitude/Azimuth)
        Distance      = 0x00000040, //!< Info about an object's distance
        Size          = 0x00000080, //!< Info about an object's size
        Extra1        = 0x00000100, //!< Derived class-specific extra fields
        Extra2        = 0x00000200, //!< Derived class-specific extra fields
        Extra3        = 0x00000400, //!< Derived class-specific extra fields
        PlainText     = 0x00000800, //!< Strip HTML tags from output
        HourAngle     = 0x00001000,  //!< The hour angle + DE (of date)
        AbsoluteMagnitude = 0x00002000  //!< The absolute magnitude
    };

    //! A pre-defined set of specifiers for the getInfoString flags argument to getInfoString
    static const InfoStringGroup AllInfo = (InfoStringGroup)(Name|CatalogNumber|Magnitude|RaDecJ2000|RaDecOfDate|AltAzi|Distance|Size|Extra1|Extra2|Extra3|HourAngle|AbsoluteMagnitude);
    //! A pre-defined set of specifiers for the getInfoString flags argument to getInfoString
    static const InfoStringGroup ShortInfo = (InfoStringGroup)(Name|CatalogNumber|Magnitude|RaDecJ2000);
    //ASAF
    static const InfoStringGroup NameInfo = (InfoStringGroup)(Name|PlainText);
    static const InfoStringGroup CatalogInfo = (InfoStringGroup)(CatalogNumber|PlainText);
    static const InfoStringGroup NameCat = (InfoStringGroup)(Name|CatalogNumber|PlainText);
    static const InfoStringGroup CatalogInfo2 = (InfoStringGroup)(CatalogNumber);

    virtual ~StelObject(void) {}

    //! Default implementation of the getRegion method.
    //! Calling this method on some object will cause an error if they need a valid StelNavigator instance to compute their position.
    //! Return the spatial region of the object.
    virtual SphericalRegionP getRegion() const {return SphericalRegionP(new SphericalPoint(getJ2000EquatorialPos(NULL)));}

    //! Write I18n information about the object in QString.
    //! @param core the StelCore object to use
    //! @param flags a set of InfoStringGroup flags which are used to
    //! filter the return value - including specified types of information
    //! and altering the output format.
    //! @return an HTML string containing information about the StelObject.
    virtual QString getInfoString(const StelCore *core, const InfoStringGroup& flags=StelObject::AllInfo) const = 0;

    //! Return object's type. It should be the name of the class.
    virtual QString getType(void) const = 0;

    //! Return object's name in english
    virtual QString getEnglishName(void) const = 0;

    //! Return translated object's name
    virtual QString getNameI18n(void) const = 0;

    //! Get observer-centered equatorial coordinates at equinox J2000
    virtual Vec3d getJ2000EquatorialPos(const StelNavigator *nav) const = 0;

    //! Get observer-centered equatorial coordinate at the current equinox
    //! The frame has it's Z axis at the planet's current rotation axis
    //! At time 2000-01-01 this frame is almost the same as J2000, but ONLY if the observer is on earth
    Vec3d getEquinoxEquatorialPos(const StelNavigator* nav) const;

    //! Get observer-centered hour angle + declination (at current equinox)
    //! The frame has its Z axis at the planet's current rotation axis
    Vec3d getSideralPos(const StelCore* core) const;

    //! Get observer-centered alt/az position
    //! The frame has it's Z axis at the zenith
    Vec3d getAltAzPos(const StelNavigator* nav) const;

    //! Get observer-centered alt/az position
    //! It is the automatic position, i.e. taking the refraction effect into account if atmosphere is on.
    //! The frame has it's Z axis at the zenith
    Vec3d getAltAzPosAuto(const StelCore* core) const;

    //! Return object's apparent V magnitude as seen from observer
    virtual float getVMagnitude(const StelNavigator *nav) const {return 99;}

    //! Return a priority value which is used to discriminate objects by priority
    //! As for magnitudes, the lower is the higher priority
    virtual float getSelectPriority(const StelNavigator *nav) const {return 99;}

    //! Get a color used to display info about the object
    virtual Vec3f getInfoColor() const {return Vec3f(1,1,1);}

    //! Return the best FOV in degree to use for a close view of the object
    virtual double getCloseViewFov(const StelNavigator *nav) const {return 10.;}

    //! Return the best FOV in degree to use for a global view of the object satellite system (if there are satellites)
    virtual double getSatellitesFov(const StelNavigator *nav) const {return -1.;}
    virtual double getParentSatellitesFov(const StelNavigator *nav) const {return -1.;}

    //! Return the angular radius of a circle containing the object as seen from the observer
    //! with the circle center assumed to be at getJ2000EquatorialPos().
    //! @return radius in degree. This value is the apparent angular size of the object, and is independent of the current FOV.
    virtual double getAngularSize(const StelCore* core) const = 0;

    //For stars
    unsigned char mag;
    unsigned char bV;
protected:

    //! Format the positional info string contain J2000/of date/altaz/hour angle positions for the object
    QString getPositionInfoString(const StelCore *core, const InfoStringGroup& flags) const;
    //! Apply post processing on the info string
    void postProcessInfoString(QString& str, const InfoStringGroup& flags) const;

};

#endif // _STELOBJECT_HPP_
