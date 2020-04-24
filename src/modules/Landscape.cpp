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

#include "Landscape.hpp"
#include "StelApp.hpp"
#include "StelTextureMgr.hpp"
#include "StelFileMgr.hpp"
#include "StelIniParser.hpp"
#include "StelLocation.hpp"
#include "StelCore.hpp"
#include "StelPainter.hpp"
#include "StelMainGraphicsView.hpp"

#include <QDebug>
#include <QSettings>
#include <QVarLengthArray>

//#ifdef WIN32
//#include <GL/glext.h>
//PFNGLACTIVETEXTUREPROC pGlActiveTexture = NULL;
//#define glActiveTexture pGlActiveTexture
//#endif //WIN32

GLuint pboIds2[2];  // IDs of PBO
long data_size2;

Landscape::Landscape(float _radius) : radius(_radius), skyBrightness(1.), angleRotateZOffset(0.)
{
    validLandscape = 0;
}

Landscape::~Landscape()
{
}


// Load attributes common to all landscapes
void Landscape::loadCommon(const QSettings& landscapeIni, const QString& landscapeId)
{
    name = landscapeIni.value("landscape/name").toString();
    author = landscapeIni.value("landscape/author").toString();
    description = landscapeIni.value("landscape/description").toString();
    if (name.isEmpty())
    {
        qWarning() << "No valid landscape definition found for landscape ID "
                   << landscapeId << ". No landscape in use." << endl;
        validLandscape = 0;
        return;
    }
    else
    {
        validLandscape = 1;
    }

    // Optional data
    // Patch GZ:
    if (landscapeIni.contains("landscape/tesselate_rows"))
        rows = landscapeIni.value("landscape/tesselate_rows").toInt();
    else rows=20;
    if (landscapeIni.contains("landscape/tesselate_cols"))
        cols = landscapeIni.value("landscape/tesselate_cols").toInt();
    else cols=40;


    if (landscapeIni.contains("location/planet"))
        location.planetName = landscapeIni.value("location/planet").toString();
    else
        location.planetName = "Earth";
    if (landscapeIni.contains("location/altitude"))
        location.altitude = landscapeIni.value("location/altitude").toInt();
    if (landscapeIni.contains("location/latitude"))
        location.latitude = StelUtils::getDecAngle(landscapeIni.value("location/latitude").toString())*180./M_PI;
    if (landscapeIni.contains("location/longitude"))
        location.longitude = StelUtils::getDecAngle(landscapeIni.value("location/longitude").toString())*180./M_PI;
    if (landscapeIni.contains("location/country"))
        location.country = landscapeIni.value("location/country").toString();
    if (landscapeIni.contains("location/state"))
        location.state = landscapeIni.value("location/state").toString();
    if (landscapeIni.contains("location/name"))
        location.name = landscapeIni.value("location/name").toString();
    else
        location.name = name;
    location.landscapeKey = name;
}

const QString Landscape::getTexturePath(const QString& basename, const QString& landscapeId)
{
    // look in the landscape directory first, and if not found default to global textures directory
    QString path;
    try
    {
        path = StelFileMgr::findFile("landscapes/" + landscapeId + "/" + basename);
        return path;
    }
    catch (std::runtime_error& e)
    {
        path = StelFileMgr::findFile("textures/" + basename);
        return path;
    }
}

LandscapeOldStyle::LandscapeOldStyle(float _radius) : Landscape(_radius), sideTexs(NULL), sides(NULL), tanMode(false), calibrated(false)
{}

LandscapeOldStyle::~LandscapeOldStyle()
{
    if (sideTexs)
    {
        delete [] sideTexs;
        sideTexs = NULL;
    }

    if (sides) delete [] sides;
}

void LandscapeOldStyle::load(const QSettings& landscapeIni, const QString& landscapeId)
{
    // TODO: put values into hash and call create method to consolidate code
    loadCommon(landscapeIni, landscapeId);
    // Patch GZ:
    if (landscapeIni.contains("landscape/tesselate_rows"))
        rows = landscapeIni.value("landscape/tesselate_rows").toInt();
    else rows=8;
    if (landscapeIni.contains("landscape/tesselate_cols"))
        cols = landscapeIni.value("landscape/tesselate_cols").toInt();
    else cols=16;

    QString type = landscapeIni.value("landscape/type").toString();
    if(type != "old_style")
    {
        qWarning() << "Landscape type mismatch for landscape " << landscapeId
                   << ", expected old_style, found " << type << ".  No landscape in use.";
        validLandscape = 0;
        return;
    }

    // Load sides textures
    nbSideTexs = landscapeIni.value("landscape/nbsidetex", 0).toInt();
    sideTexs = new StelTextureSP[nbSideTexs];
    for (int i=0;i<nbSideTexs;++i)
    {
        QString tmp = QString("tex%1").arg(i);
        sideTexs[i] = StelApp::getInstance().getTextureManager().createTexture(getTexturePath(landscapeIni.value(QString("landscape/")+tmp).toString(), landscapeId));
    }

    // Init sides parameters
    nbSide = landscapeIni.value("landscape/nbside", 0).toInt();
    sides = new landscapeTexCoord[nbSide];
    QString s;
    int texnum;
    float a,b,c,d;
    for (int i=0;i<nbSide;++i)
    {
        QString tmp = QString("side%1").arg(i);
        s = landscapeIni.value(QString("landscape/")+tmp).toString();
        sscanf(s.toLocal8Bit(),"tex%d:%f:%f:%f:%f",&texnum,&a,&b,&c,&d);
        sides[i].tex = sideTexs[texnum];
        sides[i].texCoords[0] = a;
        sides[i].texCoords[1] = b;
        sides[i].texCoords[2] = c;
        sides[i].texCoords[3] = d;
        // qDebug("%f %f %f %f\n",a,b,c,d);
    }

    nbDecorRepeat = landscapeIni.value("landscape/nb_decor_repeat", 1).toInt();

    groundTex = StelApp::getInstance().getTextureManager().createTexture(getTexturePath(landscapeIni.value("landscape/groundtex").toString(), landscapeId), StelTexture::StelTextureParams(true));
    s = landscapeIni.value("landscape/ground").toString();
    sscanf(s.toLocal8Bit(),"groundtex:%f:%f:%f:%f",&a,&b,&c,&d);
    groundTexCoord.tex = groundTex;
    groundTexCoord.texCoords[0] = a;
    groundTexCoord.texCoords[1] = b;
    groundTexCoord.texCoords[2] = c;
    groundTexCoord.texCoords[3] = d;

    fogTex = StelApp::getInstance().getTextureManager().createTexture(getTexturePath(landscapeIni.value("landscape/fogtex").toString(), landscapeId), StelTexture::StelTextureParams(true, GL_LINEAR, GL_REPEAT));
    s = landscapeIni.value("landscape/fog").toString();
    sscanf(s.toLocal8Bit(),"fogtex:%f:%f:%f:%f",&a,&b,&c,&d);
    fogTexCoord.tex = fogTex;
    fogTexCoord.texCoords[0] = a;
    fogTexCoord.texCoords[1] = b;
    fogTexCoord.texCoords[2] = c;
    fogTexCoord.texCoords[3] = d;

    fogAltAngle        = landscapeIni.value("landscape/fog_alt_angle", 0.).toFloat();
    fogAngleShift      = landscapeIni.value("landscape/fog_angle_shift", 0.).toFloat();
    decorAltAngle      = landscapeIni.value("landscape/decor_alt_angle", 0.).toFloat();
    decorAngleShift    = landscapeIni.value("landscape/decor_angle_shift", 0.).toFloat();
    angleRotateZ       = landscapeIni.value("landscape/decor_angle_rotatez", 0.).toFloat();
    groundAngleShift   = landscapeIni.value("landscape/ground_angle_shift", 0.).toFloat();
    groundAngleRotateZ = landscapeIni.value("landscape/ground_angle_rotatez", 0.).toFloat();
    drawGroundFirst    = landscapeIni.value("landscape/draw_ground_first", 0).toInt();
    tanMode            = landscapeIni.value("landscape/tan_mode", false).toBool();
    calibrated         = landscapeIni.value("landscape/calibrated", false).toBool();
}


// create from a hash of parameters (no ini file needed)
void LandscapeOldStyle::create(bool _fullpath, QMap<QString, QString> param)
{
    name = param["name"];
    validLandscape = 1;  // assume valid if got here

    // Load sides textures
    nbSideTexs = param["nbsidetex"].toInt();
    sideTexs = new StelTextureSP[nbSideTexs];

    char tmp[255];
    //StelApp::getInstance().getTextureManager().setMipmapsMode(true);
    //StelApp::getInstance().getTextureManager().setMagFilter(GL_NEAREST);
    for (int i=0;i<nbSideTexs;++i)
    {
        sprintf(tmp,"tex%d",i);
        sideTexs[i] = StelApp::getInstance().getTextureManager().createTexture(param["path"] + param[tmp], StelTexture::StelTextureParams(true));
    }

    // Init sides parameters
    nbSide = param["nbside"].toInt();
    sides = new landscapeTexCoord[nbSide];
    QString s;
    int texnum;
    float a,b,c,d;
    for (int i=0;i<nbSide;++i)
    {
        sprintf(tmp,"side%d",i);
        s = param[tmp];
        sscanf(s.toUtf8().constData(),"tex%d:%f:%f:%f:%f",&texnum,&a,&b,&c,&d);
        sides[i].tex = sideTexs[texnum];
        sides[i].texCoords[0] = a;
        sides[i].texCoords[1] = b;
        sides[i].texCoords[2] = c;
        sides[i].texCoords[3] = d;
        //qDebug("%f %f %f %f\n",a,b,c,d);
    }

    bool ok;
    nbDecorRepeat = param["nb_decor_repeat"].toInt(&ok);

    if (!ok)
        nbDecorRepeat = 1;

    groundTex = StelApp::getInstance().getTextureManager().createTexture(param["path"] + param["groundtex"], StelTexture::StelTextureParams(true));
    s = param["ground"];
    sscanf(s.toUtf8().constData(),"groundtex:%f:%f:%f:%f",&a,&b,&c,&d);
    groundTexCoord.tex = groundTex;
    groundTexCoord.texCoords[0] = a;
    groundTexCoord.texCoords[1] = b;
    groundTexCoord.texCoords[2] = c;
    groundTexCoord.texCoords[3] = d;

    fogTex = StelApp::getInstance().getTextureManager().createTexture(param["path"] + param["fogtex"], StelTexture::StelTextureParams(true, GL_LINEAR, GL_REPEAT));
    s = param["fog"];
    sscanf(s.toUtf8().constData(),"fogtex:%f:%f:%f:%f",&a,&b,&c,&d);
    fogTexCoord.tex = fogTex;
    fogTexCoord.texCoords[0] = a;
    fogTexCoord.texCoords[1] = b;
    fogTexCoord.texCoords[2] = c;
    fogTexCoord.texCoords[3] = d;

    fogAltAngle        = param["fog_alt_angle"].toDouble();
    fogAngleShift      = param["fog_angle_shift"].toDouble();
    decorAltAngle      = param["decor_alt_angle"].toDouble();
    decorAngleShift    = param["decor_angle_shift"].toDouble();
    angleRotateZ       = param["decor_angle_rotatez"].toDouble();
    groundAngleShift   = param["ground_angle_shift"].toDouble();
    groundAngleRotateZ = param["ground_angle_rotatez"].toDouble();
    drawGroundFirst    = param["draw_ground_first"].toInt();
}

void LandscapeOldStyle::draw(StelCore* core)
{
    StelPainter painter(core->getProjection(StelCore::FrameAltAz));
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    painter.enableTexture2d(true);
    glEnable(GL_CULL_FACE);

    if (!validLandscape)
        return;
    if (drawGroundFirst)
        drawGround(core, painter);
    drawDecor(core, painter);
    if (!drawGroundFirst)
        drawGround(core, painter);
    drawFog(core, painter);
}


// Draw the horizon fog
void LandscapeOldStyle::drawFog(StelCore* core, StelPainter& sPainter) const
{
    if (!fogFader.getInterstate())
        return;

    const float vpos = (tanMode||calibrated) ? radius*std::tan(fogAngleShift*M_PI/180.) : radius*std::sin(fogAngleShift*M_PI/180.);
    sPainter.setProjector(core->getProjection(core->getNavigator()->getAltAzModelViewMat() * Mat4d::translation(Vec3d(0.,0.,vpos))));
    //sPainter.setProjector(core->getProjection(core->getNavigator()->getAltAzModelViewMat() * Mat4d::scaling(Vec3d(5.,5.,5))));


    glBlendFunc(GL_ONE, GL_ONE);
    //const float nightModeFilter = StelApp::getInstance().getVisionModeNight() ? 0.f : 1.f;
    const float nightModeFilter = 1.f;
    sPainter.setColor(fogFader.getInterstate()*(0.1f+0.1f*skyBrightness),
                      fogFader.getInterstate()*(0.1f+0.1f*skyBrightness)*nightModeFilter,
                      fogFader.getInterstate()*(0.1f+0.1f*skyBrightness)*nightModeFilter);
    fogTex->bind();
    const float height = (tanMode||calibrated) ? radius*std::tan(fogAltAngle*M_PI/180.) : radius*std::sin(fogAltAngle*M_PI/180.);
    sPainter.sCylinder(radius, height, 64, 1);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    //qDebug()<<radius;
}

// Draw the mountains with a few pieces of texture
void LandscapeOldStyle::drawDecor(StelCore* core, StelPainter& sPainter) const
{

    // Patched by Georg Zotti: I located an undocumented switch tan_mode, maybe tan_mode=true means cylindrical panorama projection.
    // anyway, the old code makes unfortunately no sense.
    // I added a switch "calibrated" for the ini file. If true, it works as this landscape apparently was originally intended.
    // So I corrected the texture coordinates so that decorAltAngle is the total angle, decorAngleShift the lower angle,
    // and the texture in between is correctly stretched.
    // TODO: (1) Replace fog cylinder by similar texture, which could be painted as image layer in Photoshop/Gimp.
    //       (2) Implement calibrated && tan_mode
    sPainter.setProjector(core->getProjection(StelCore::FrameAltAz));

    if (!landFader.getInterstate())
        return;
    //const float nightModeFilter = StelApp::getInstance().getVisionModeNight() ? 0.f : 1.f;
    const float nightModeFilter = 1.f;
    sPainter.setColor(skyBrightness, skyBrightness*nightModeFilter, skyBrightness*nightModeFilter, landFader.getInterstate());
    static const int stacks = (calibrated ? 16 : 8); // GZ: 8->16, I need better precision.
    // make slices_per_side=(3<<K) so that the innermost polygon of the
    // fandisk becomes a triangle:
    int slices_per_side = 3*64/(nbDecorRepeat*nbSide);
    if (slices_per_side<=0) slices_per_side = 1;
    const double z0 =
            calibrated ?
                // GZ: For calibrated, we use z=decorAngleShift...(decorAltAngle-decorAngleShift), but we must compute the tan in the loop.
                decorAngleShift :
                (tanMode ?
                     radius * std::tan(decorAngleShift*M_PI/180.0) :
                     radius * std::sin(decorAngleShift*M_PI/180.0));
    // GZ: The old formula is completely meaningless for photos with opening angle >90,
    // and most likely also not what was intended for other images.
    // Note that GZ fills this value with a different meaning!
    const double d_z = calibrated ? decorAltAngle/stacks : (tanMode ? radius*std::tan(decorAltAngle*M_PI/180.0)/stacks : radius*std::sin(decorAltAngle*M_PI/180.0)/stacks);

    // N.B. GZ had doubles for the next 5, but floats may well be enough.
    const float alpha = 2.f*M_PI/(nbDecorRepeat*nbSide*slices_per_side);
    const float ca = std::cos(alpha);
    const float sa = std::sin(alpha);
    float y0 = radius*std::cos((angleRotateZ+angleRotateZOffset)*M_PI/180.0);
    float x0 = radius*std::sin((angleRotateZ+angleRotateZOffset)*M_PI/180.0);

    static QVector<Vec2f> texCoordsArray;
    static QVector<Vec3d> vertexArray;
    static QVector<unsigned int> indiceArray;
    for (int n=0;n<nbDecorRepeat;n++)
    {
        for (int i=0;i<nbSide;i++)
        {
            texCoordsArray.resize(0);
            vertexArray.resize(0);
            indiceArray.resize(0);
            sides[i].tex->bind();
            float tx0 = sides[i].texCoords[0];
            const float d_tx0 = (sides[i].texCoords[2]-sides[i].texCoords[0]) / slices_per_side;
            const float d_ty = (sides[i].texCoords[3]-sides[i].texCoords[1]) / stacks;
            for (int j=0;j<slices_per_side;j++)
            {
                const float y1 = y0*ca - x0*sa;
                const float x1 = y0*sa + x0*ca;
                const float tx1 = tx0 + d_tx0;
                float z = z0;
                float ty0 = sides[i].texCoords[1];
                for (int k=0;k<=stacks*2;k+=2)
                {
                    texCoordsArray << Vec2f(tx0, ty0) << Vec2f(tx1, ty0);
                    if (calibrated){
                        double tanZ=radius * std::tan(z*M_PI/180.0);
                        vertexArray << Vec3d(x0, y0, tanZ) << Vec3d(x1, y1, tanZ);
                    }else{
                        vertexArray << Vec3d(x0, y0, z) << Vec3d(x1, y1, z);
                    }
                    z += d_z;
                    ty0 += d_ty;
                }
                unsigned int offset = j*(stacks+1)*2;
                for (int k = 2;k<stacks*2+2;k+=2)
                {
                    indiceArray << offset+k-2 << offset+k-1 << offset+k;
                    indiceArray << offset+k << offset+k-1 << offset+k+1;
                }
                y0 = y1;
                x0 = x1;
                tx0 = tx1;
            }
            StelVertexArray array(vertexArray, StelVertexArray::Triangles, texCoordsArray, indiceArray);
            sPainter.drawSphericalTriangles(array, true, NULL, false);
        }
    }
}


// Draw the ground
void LandscapeOldStyle::drawGround(StelCore* core, StelPainter& sPainter) const
{
    if (!landFader.getInterstate())
        return;
    const StelNavigator* nav = core->getNavigator();
    const float vshift = (tanMode || calibrated) ?
                radius*std::tan(groundAngleShift*M_PI/180.) :
                radius*std::sin(groundAngleShift*M_PI/180.);
    Mat4d mat = nav->getAltAzModelViewMat() * Mat4d::zrotation((groundAngleRotateZ-angleRotateZOffset)*M_PI/180.f) * Mat4d::translation(Vec3d(0,0,vshift));
    sPainter.setProjector(core->getProjection(mat));
    //float nightModeFilter = StelApp::getInstance().getVisionModeNight() ? 0.f : 1.f;
    float nightModeFilter = 1.f;
    sPainter.setColor(skyBrightness, skyBrightness*nightModeFilter, skyBrightness*nightModeFilter, landFader.getInterstate());
    groundTex->bind();
    // make slices_per_side=(3<<K) so that the innermost polygon of the
    // fandisk becomes a triangle:
    int slices_per_side = 3*64/(nbDecorRepeat*nbSide);
    if (slices_per_side<=0) slices_per_side = 1;

    // draw a fan disk instead of a ordinary disk to that the inner slices
    // are not so slender. When they are too slender, culling errors occur
    // in cylinder projection mode.
    int slices_inside = nbSide*slices_per_side*nbDecorRepeat;
    int level = 0;
    while ((slices_inside&1)==0 && slices_inside > 4)
    {
        ++level;
        slices_inside>>=1;
    }
    sPainter.sFanDisk(radius,slices_inside,level);
}

LandscapeFisheye::LandscapeFisheye(float _radius) : Landscape(_radius)
{}

LandscapeFisheye::~LandscapeFisheye()
{
}

void LandscapeFisheye::load(const QSettings& landscapeIni, const QString& landscapeId)
{
    loadCommon(landscapeIni, landscapeId);

    type = landscapeIni.value("landscape/type").toString();
    if(type != "fisheye")
    {
        qWarning() << "Landscape type mismatch for landscape "<< landscapeId << ", expected fisheye, found " << type << ".  No landscape in use.\n";
        validLandscape = 0;
        return;
    }
    create(name, 0, getTexturePath(landscapeIni.value("landscape/maptex").toString(), landscapeId),
           landscapeIni.value("landscape/texturefov", 360).toFloat(),
           landscapeIni.value("landscape/angle_rotatez", 0.).toFloat());
}
//ASAF

void LandscapeFisheye::loadFrame(QString fileFrame,QString oldName)
{
    type = "fisheyeFrame";
    validLandscape = 1;
    rows=40;//20;
    cols=80;//40;

    double rotatez = 0;
    create(oldName, 0, fileFrame,-180,rotatez);
}
void LandscapeFisheye::setShowDaylight(bool value)
{
    _isShowDaylight = value;
}

void LandscapeFisheye::setFaderDuration(int duration)
{
    _faderDuration = duration;
    landFader.setDuration(duration*1000);
}
//

// create a fisheye landscape from basic parameters (no ini file needed)
void LandscapeFisheye::create(const QString _name, bool _fullpath, const QString& _maptex,
                              double _texturefov, double _angleRotateZ)
{
    // qDebug() << _name << " " << _fullpath << " " << _maptex << " " << _texturefov;
    validLandscape = 1;  // assume ok...
    name = _name;
    mapTex = StelApp::getInstance().getTextureManager().createTexture(_maptex, StelTexture::StelTextureParams(true));
    texFov = _texturefov*M_PI/180.;
    angleRotateZ = _angleRotateZ*M_PI/180.;
    _faderDuration = 1;
    landFader.setDuration(_faderDuration*1000);

}


void LandscapeFisheye::draw(StelCore* core)
{
    if(!validLandscape) return;
    if(!landFader.getInterstate()) return;

    StelNavigator* nav = core->getNavigator();
    const StelProjectorP prj = core->getProjection(nav->getAltAzModelViewMat() * Mat4d::zrotation(-(angleRotateZ+(angleRotateZOffset*2*M_PI/360.))));
    StelPainter sPainter(prj);

    // Normal transparency mode
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    //float nightModeFilter = StelApp::getInstance().getVisionModeNight() ? 0. : 1.;
    float nightModeFilter =  1.;
    if (_isShowDaylight)
        sPainter.setColor(skyBrightness,
                          skyBrightness*nightModeFilter,
                          skyBrightness*nightModeFilter,
                          landFader.getInterstate());
    else
        sPainter.setColor(255,
                          255,
                          255,
                          landFader.getInterstate());

    glEnable(GL_CULL_FACE);
    sPainter.enableTexture2d(true);
    glEnable(GL_BLEND);
    mapTex->bind();
    // Patch GZ: (40,20)->(cols,rows)
    sPainter.sSphereMap(radius,cols,rows,texFov,1);

    glDisable(GL_CULL_FACE);
}


// spherical panoramas

LandscapeSpherical::LandscapeSpherical(float _radius) : Landscape(_radius)
{}

LandscapeSpherical::~LandscapeSpherical()
{
}

void LandscapeSpherical::load(const QSettings& landscapeIni, const QString& landscapeId)
{
    loadCommon(landscapeIni, landscapeId);

    QString type = landscapeIni.value("landscape/type").toString();
    if (type != "spherical")
    {
        qWarning() << "Landscape type mismatch for landscape "<< landscapeId
                   << ", expected spherical, found " << type
                   << ".  No landscape in use.\n";
        validLandscape = 0;
        return;
    }

    create(name, 0, getTexturePath(landscapeIni.value("landscape/maptex").toString(), landscapeId),
           landscapeIni.value("landscape/angle_rotatez", 0.).toFloat());
}


// create a spherical landscape from basic parameters (no ini file needed)
void LandscapeSpherical::create(const QString _name, bool _fullpath, const QString& _maptex,
                                double _angleRotateZ)
{
    // qDebug() << _name << " " << _fullpath << " " << _maptex << " " << _texturefov;
    validLandscape = 1;  // assume ok...
    name = _name;
    mapTex = StelApp::getInstance().getTextureManager().createTexture(_maptex, StelTexture::StelTextureParams(true));
    angleRotateZ = _angleRotateZ*M_PI/180.;
}


void LandscapeSpherical::draw(StelCore* core)
{
    if(!validLandscape) return;
    if(!landFader.getInterstate()) return;

    StelNavigator* nav = core->getNavigator();
    const StelProjectorP prj = core->getProjection(nav->getAltAzModelViewMat() * Mat4d::zrotation(-(angleRotateZ+(angleRotateZOffset*2*M_PI/360.))));
    StelPainter sPainter(prj);

    // Normal transparency mode
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    //float nightModeFilter = StelApp::getInstance().getVisionModeNight() ? 0. : 1.;
    float nightModeFilter = 1.;
    sPainter.setColor(skyBrightness,
                      skyBrightness*nightModeFilter,
                      skyBrightness*nightModeFilter,
                      landFader.getInterstate());

    glEnable(GL_CULL_FACE);
    sPainter.enableTexture2d(true);
    glEnable(GL_BLEND);
    mapTex->bind();

    // TODO: verify that this works correctly for custom projections
    // seam is at East
    //sPainter.sSphere(radius, 1.0, 40, 20, 1, true);
    // GZ: Want better angle resolution, optional!
    sPainter.sSphere(radius, 1.0, cols, rows, 1, true);
    glDisable(GL_CULL_FACE);
}

// Video Landscape //ASAF
LandscapeVideo::LandscapeVideo(float _radius) : Landscape(_radius)
{
    isLandscape = false;
    vbrightness = 1.0;
    vcontrast = 1.0;
    vsaturation = 1.0;
}

LandscapeVideo::~LandscapeVideo()
{
    //vp = NULL;
    //delete vop_current ;
    //vop_current = NULL;
    //    if(vp)
    //       delete vp;
}
void LandscapeVideo::load(const QSettings& landscapeIni, const QString& landscapeId)
{
    loadCommon(landscapeIni, landscapeId);
    type = landscapeIni.value("landscape/type").toString();
}

//void LandscapeVideo::loadvop(VideoFile* vop,const QString _name ,double _texturefov, double _angleRotateZ)
//{
void LandscapeVideo::loadvop(const QString _name ,double _texturefov, double _angleRotateZ)
{
    //vop_current = vop;

    validLandscape = 1;
    rows=40;//20;
    cols=80;//40;

    //double rotatez = -90;
    //_texturefov = -180;
    //name = "video";
    create(_name, 0, "" , _texturefov, _angleRotateZ);
}


// create a video landscape from basic parameters (no ini file needed)
void LandscapeVideo::create(const QString _name, bool _fullpath, const QString& _maptex, double _texturefov, double _angleRotateZ)
{
    // qDebug() << _name << " " << _fullpath << " " << _maptex << " " << _texturefov;
    validLandscape = 1;  // assume ok...
    name = _name;
    mapTex = StelApp::getInstance().getTextureManager().createTextureVideo(StelTexture::StelTextureParams(true));//,vop_current->getFrameWidth(),vop_current->getFrameHeight());

    mapTexY = StelApp::getInstance().getTextureManager().createTextureVideo(StelTexture::StelTextureParams(true),true);
    mapTexU = StelApp::getInstance().getTextureManager().createTextureVideo(StelTexture::StelTextureParams(true),true);
    mapTexV = StelApp::getInstance().getTextureManager().createTextureVideo(StelTexture::StelTextureParams(true),true);

    texFov = _texturefov*M_PI/180.;
    angleRotateZ = _angleRotateZ*M_PI/180.;

    mapTex->bind();
}

void LandscapeVideo::init(QSize sizevideo)
{
    //Shader mutlaka kullanýlacak

    videoGeo = QSize(sizevideo.width(),sizevideo.height());
    videoShaderProgram= new QGLShaderProgram();
    QGLShader* fShader = new QGLShader(QGLShader::Fragment);
    if (!fShader->compileSourceCode(
                "#version 120\n"
                "uniform sampler2D t_texture_y;\n"
                "uniform sampler2D t_texture_u;\n"
                "uniform sampler2D t_texture_v;\n"
                "uniform int blackscreen = 1;\n"
                "uniform float brightness = 1.0;\n"
                "uniform float contrast = 1.0;\n"
                "uniform float saturation = 1.0;\n"
                ""
                // For all settings: 1.0 = 100% 0.5=50% 1.5 = 150%\n"
                "vec3 ContrastSaturationBrightness(vec3 color, float brt, float sat, float con)\n"
                "{;\n"
                "    // Increase or decrease theese values to adjust r, g and b color channels seperately\n"
                "    const float AvgLumR = 0.5;\n"
                "    const float AvgLumG = 0.5;\n"
                "    const float AvgLumB = 0.5;\n"
                ""
                "    const vec3 LumCoeff = vec3(0.2125, 0.7154, 0.0721);\n"
                ""
                "    vec3 AvgLumin = vec3(AvgLumR, AvgLumG, AvgLumB);\n"
                "    vec3 brtColor = color * brt;\n"
                "    vec3 intensity = vec3(dot(brtColor, LumCoeff));\n"
                "    vec3 satColor = mix(intensity, brtColor, sat);\n"
                "    vec3 conColor = mix(AvgLumin, satColor, con);\n"
                "    return conColor;\n"
                "}\n"
                ""
                "void main(){\n"
                "float y,u,v,r,g,b;\n"
                "const float c1 = float(255.0/219.0);\n"
                "const float c2 = float(16.0/256.0);\n"
                "const float c3 = float(1.0/2.0);\n"
                "const float c4 = float(1596.0/1000.0);\n"
                "const float c5 = float(391.0/1000.0);\n"
                "const float c6 = float(813.0/1000.0);\n"
                "const float c7 = float(2018.0/1000.0);\n"
                "y = texture2D(t_texture_y, gl_TexCoord[0].xy).r;\n"
                "u = texture2D(t_texture_u, gl_TexCoord[0].xy).r;\n"
                "v = texture2D(t_texture_v, gl_TexCoord[0].xy).r;\n"
                "\n"
                "y = c1 * (y - c2);\n"
                "u = u - c3;\n"
                "v = v - c3;\n"
                "\n"
                "r = clamp(y + c4 * v, 0.0, 1.0);\n"
                "g = clamp(y - c5 * u - c6 * v, 0.0, 1.0);\n"
                "b = clamp(y + c7 * u, 0.0, 1.0);\n"
                "\n"
                "vec3 rgb = ContrastSaturationBrightness(vec3(r,g,b),brightness,saturation,contrast);"
                "\n"
                "gl_FragColor = vec4(rgb[0],rgb[1],rgb[2],1.0);\n"
                "if (blackscreen == 1)\n"
                "	gl_FragColor = vec4(0.0, 0.0, 0.0, 1.0);\n"
                "}\n"))
    {
        qWarning() << "Error while compiling fragment shader: " << fShader->log();
        useShader = false;
    }

    if (!fShader->log().isEmpty())
    {
        qWarning() << "Warnings while compiling fragment shader: " << fShader->log();
    }
    videoShaderProgram->addShader(fShader);
    if (!videoShaderProgram->link())
    {
        qWarning() << "Error while linking shader program: " << videoShaderProgram->log();
        useShader = false;
    }
    if (!videoShaderProgram->log().isEmpty())
    {
        qWarning() << "Warnings while linking shader: " << videoShaderProgram->log();
    }

    mapTexY->bind();
    glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D ( GL_TEXTURE_2D, 0, GL_LUMINANCE ,videoGeo.width(), videoGeo.height(), 0, GL_LUMINANCE, GL_UNSIGNED_BYTE,0 );

    mapTexU->bind();
    glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D ( GL_TEXTURE_2D, 0, GL_LUMINANCE, videoGeo.width() /2 ,videoGeo.height() /2, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE,0);

    mapTexV->bind();
    glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D ( GL_TEXTURE_2D, 0, GL_LUMINANCE, videoGeo.width() /2 ,videoGeo.height() /2, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, 0);

}

void LandscapeVideo::draw(StelCore* core)
{

    if(!validLandscape) return;
    if(!landFader.getInterstate()) return;

    if (frameChanged && !vp.isEmpty())
    {
        StelNavigator* nav = core->getNavigator();
        const StelProjectorP prj = core->getProjection(nav->getAltAzModelViewMat() * Mat4d::zrotation(-(angleRotateZ+(angleRotateZOffset*2*M_PI/360.))));
        StelPainter sPainter(prj);

        // Normal transparency mode
        if(isLandscape)
        {
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            sPainter.setColor(skyBrightness, skyBrightness, skyBrightness, landFader.getInterstate());
        }
        glEnable(GL_CULL_FACE);
        sPainter.enableTexture2d(true);

        if(isLandscape)
            glEnable (GL_BLEND);
        else
            glDisable(GL_BLEND);

        Q_ASSERT(videoShaderProgram);

        videoShaderProgram->bind();
        int _idxU = videoGeo.width() * videoGeo.height();
        int _idxV = _idxU + (_idxU / 4);

        videoShaderProgram->setUniformValue("blackscreen",0);
        videoShaderProgram->setUniformValue("brightness",vbrightness);
        videoShaderProgram->setUniformValue("contrast",vcontrast);
        videoShaderProgram->setUniformValue("saturation",vsaturation);

        mapTexU->bind(1);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        videoShaderProgram->setUniformValue("t_texture_u",1);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, videoGeo.width() /2, videoGeo.height() /2, GL_LUMINANCE, GL_UNSIGNED_BYTE, &vp.data()[_idxU]);

        mapTexV->bind(2);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        videoShaderProgram->setUniformValue("t_texture_v",2);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0,  videoGeo.width() /2, videoGeo.height() /2, GL_LUMINANCE, GL_UNSIGNED_BYTE,  &vp.data()[_idxV]);

        mapTexY->bind(0);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        videoShaderProgram->setUniformValue("t_texture_y",0);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, videoGeo.width(), videoGeo.height(), GL_LUMINANCE, GL_UNSIGNED_BYTE, vp.data());

        sPainter.xoffsetrate = (videoGeo.height()*1000.0) / videoGeo.width()/1000.0;
        if ( type == "sphericalvideo" )
            sPainter.sSphere(radius, -1.0, cols, rows, 0,true);
        else if(type =="fisheyevideo" )
            sPainter.sSphereMap(radius,cols,rows,texFov,2);
        else
            sPainter.sSphereMap(radius,cols,rows,texFov,2);

        if(isLandscape)
            glDisable (GL_BLEND);
        else
            glEnable(GL_BLEND);

        glDisable(GL_CULL_FACE);
        frameChanged = false;

        videoShaderProgram->release();

    }

}

void LandscapeVideo::stopvop()
{

    // clean up texture
    //glDeleteTextures(1, &mapTex->id);

    // clean up PBOs
    //glDeleteBuffersARB2(2, pboIds2);
    //delete vop_current;
    //vop_current = NULL;
    //vp = NULL;
    //vop_current->stop();
    //vop_current->close();
}
