/*
 * ShiraPlayer(TM)
 * Copyright (C) 2003 Fabien Chereau
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

#include "StelTranslator.hpp"

#include "StelProjector.hpp"
#include "StelProjectorClasses.hpp"


#include <QDebug>
#include <QString>

const QString StelProjector::maskTypeToString(StelProjectorMaskType type)
{
	if (type == MaskDisk )
		return "disk";
	else
		return "none";
}

StelProjector::StelProjectorMaskType StelProjector::stringToMaskType(const QString &s)
{
	if (s=="disk")
		return MaskDisk;
	return MaskNone;
}

void StelProjector::init(const StelProjectorParams& params)
{
	maskType = (StelProjectorMaskType)params.maskType;
	zNear = params.zNear;
	oneOverZNearMinusZFar = 1.f/(zNear-params.zFar);
	viewportXywh = params.viewportXywh;
	viewportCenter = params.viewportCenter;
	gravityLabels = params.gravityLabels;
	flipHorz = params.flipHorz ? -1.f : 1.f;
	flipVert = params.flipVert ? -1.f : 1.f;
	viewportFovDiameter = params.viewportFovDiameter;
	pixelPerRad = 0.5f * viewportFovDiameter / fovToViewScalingFactor(params.fov*(M_PI/360.f));

//        //ASAF
//        fov_height_rate = 1.0;
//        fov_width_rate = 1.0;
}

QString StelProjector::getHtmlSummary() const
{
	return QString("<h3>%1</h3><p>%2</p><b>%3</b>%4").arg(getNameI18()).arg(getDescriptionI18()).arg(q_("Maximum FOV: ")).arg(getMaxFov())+QChar(0x00B0);
}


/*************************************************************************
 Return a convex polygon on the sphere which includes the viewport in the 
 current frame
*************************************************************************/
SphericalRegionP StelProjector::getViewportConvexPolygon(float marginX, float marginY) const
{
	Vec3d e0, e1, e2, e3;
	const Vec4i& vp = viewportXywh;
	bool ok = unProject(vp[0]-marginX,vp[1]-marginY,e0);
	ok &= unProject(vp[0]+vp[2]+marginX,vp[1]-marginY,e1);
	ok &= unProject(vp[0]+vp[2]+marginX,vp[1]+vp[3]+marginY,e2);
	ok &= unProject(vp[0]-marginX,vp[1]+vp[3]+marginY,e3);
	if (!ok)
	{
		// Special case for handling degenerated cases, use full sky.
		//qDebug() << "!ok";
		return SphericalRegionP((SphericalRegion*)(new AllSkySphericalRegion()));
	}
	e0.normalize();
	e1.normalize();
	e2.normalize();
	e3.normalize();
	if (needGlFrontFaceCW())
	{
		Vec3d v = e0;
		e0 = e3;
		e3 = v;
		v = e1;
		e1 = e2;
		e2 = v;
	}
	const double d = e3*((e2-e3)^(e1-e3));
	if (d > 0)
	{
		SphericalConvexPolygon* res = new SphericalConvexPolygon(e0, e1, e2, e3);
		if (res->checkValid())
		{
			return SphericalRegionP(res);
		}
		qDebug() << "!valid";
		delete res;
	}
	//return SphericalRegionP((SphericalRegion*)(new AllSkySphericalRegion()));
	const SphericalCap& hp = getBoundingSphericalCap();
	return SphericalRegionP(new SphericalCap(hp));
}


// Return a Halfspace containing the whole viewport
SphericalCap StelProjector::getBoundingSphericalCap() const
{
        SphericalCap result;
        bool needNormalization = false;
//#ifndef SHIRAPLAYER_PRE

//        //ASAF fov_rate ler �arpan olarak eklendi
//        bool ok = unProject(viewportXywh[0]+0.5*viewportXywh[2]*StelApp::getInstance().fov_width_rate , viewportXywh[1]+0.5*viewportXywh[3]*StelApp::getInstance().fov_height_rate , result.n);
//        Q_ASSERT(ok);	// The central point should be at a valid position by definition
//        //Q_ASSERT(fabs(result.n.lengthSquared()-1.)<0.000001);
//        needNormalization = fabs(result.n.lengthSquared()-1.)>0.00000001;

//        // Now need to determine the aperture
//        Vec3d e0,e1,e2,e3,e4,e5;
//        const Vec4i& vp = viewportXywh;
//        //ASAF fov_rate ler b�len olarak eklendi
//        ok &= unProject(vp[0],vp[1],e0);
//        ok &= unProject(vp[0]+vp[2]/StelApp::getInstance().fov_width_rate,vp[1],e1);
//        ok &= unProject(vp[0]+vp[2]/StelApp::getInstance().fov_width_rate,vp[1]+vp[3]/ StelApp::getInstance().fov_height_rate,e2);
//        ok &= unProject(vp[0],vp[1]+vp[3]/ StelApp::getInstance().fov_height_rate,e3);
//        ok &= unProject(vp[0],vp[1]+vp[3]/ StelApp::getInstance().fov_height_rate/2,e4);
//        ok &= unProject(vp[0]+vp[2]/StelApp::getInstance().fov_width_rate,vp[1]+vp[3]/ StelApp::getInstance().fov_height_rate/2,e5);

//#else
        bool ok = unProject(viewportXywh[0]+0.5*viewportXywh[2], viewportXywh[1]+0.5*viewportXywh[3], result.n);
        Q_ASSERT(ok);     // The central point should be at a valid position by definition
        //Q_ASSERT(fabs(result.n.lengthSquared()-1.)<0.000001);
        needNormalization = fabs(result.n.lengthSquared()-1.)>0.00000001;

        // Now need to determine the aperture
        Vec3d e0,e1,e2,e3,e4,e5;
        const Vec4i& vp = viewportXywh;
        ok &= unProject(vp[0],vp[1],e0);
        ok &= unProject(vp[0]+vp[2],vp[1],e1);
        ok &= unProject(vp[0]+vp[2],vp[1]+vp[3],e2);
        ok &= unProject(vp[0],vp[1]+vp[3],e3);
        ok &= unProject(vp[0],vp[1]+vp[3]/2,e4);
        ok &= unProject(vp[0]+vp[2],vp[1]+vp[3]/2,e5);
//#endif
	if (!ok)
	{
		// Some points were in invalid positions, use full sky.
		result.d = -1.;
                result.n.set(1,0,0);
		return result;
	}
        if (needNormalization)
        {
                result.n.normalize();
                e0.normalize();
                e1.normalize();
                e2.normalize();
                e3.normalize();
                e4.normalize();
                e5.normalize();
        }

	result.d = result.n*e0;
	double h = result.n*e1;
	if (result.d > h)
		result.d=h;
	h = result.n*e2;
	if (result.d > h)
		result.d=h;
	h = result.n*e3;
	if (result.d > h)
		result.d=h;
	h = result.n*e4;
	if (result.d > h)
		result.d=h;
	h = result.n*e5;
	if (result.d > h)
		result.d=h;
	return result;
}

/*************************************************************************
 Project the vector v from the viewport frame into the current frame 
*************************************************************************/
bool StelProjector::unProject(double x, double y, Vec3d &v) const
{
	v[0] = flipHorz * (x - viewportCenter[0]) / pixelPerRad;
	v[1] = flipVert * (y - viewportCenter[1]) / pixelPerRad;
	v[2] = 0;
	const bool rval = backward(v);
	// Even when the reprojected point comes from a region of the screen,
	// where nothing is projected to (rval=false), we finish reprojecting.
	// This looks good for atmosphere rendering, and it helps avoiding
	// discontinuities when dragging around with the mouse.

	// We need no matrix inversion because we always work with orthogonal matrices (where the transposed is the inverse).
	//v.transfo4d(inverseModelViewMatrix);
	x = v[0] - modelViewMatrix.r[12];
	y = v[1] - modelViewMatrix.r[13];
	const double z = v[2] - modelViewMatrix.r[14];
	v[0] = modelViewMatrix.r[0]*x + modelViewMatrix.r[1]*y + modelViewMatrix.r[2]*z;
	v[1] = modelViewMatrix.r[4]*x + modelViewMatrix.r[5]*y + modelViewMatrix.r[6]*z;
	v[2] = modelViewMatrix.r[8]*x + modelViewMatrix.r[9]*y + modelViewMatrix.r[10]*z;

	return rval;
}

bool StelProjector::intersectViewportDiscontinuity(const SphericalCap& cap) const
{
        if (hasDiscontinuity()==false)
                return false;
        Vec3d v1(cap.n);
        //modelViewTransform->forward(v1); // ASAF 0.11.4 versiondan al�nanlardan geldi
        return intersectViewportDiscontinuityInternal(v1, cap.d);
}
