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


#ifndef _STELAPPGRAPHICSWIDGET_HPP_
#define _STELAPPGRAPHICSWIDGET_HPP_

#include <QGraphicsWidget>
#include "StelProjector.hpp"
//class StelViewportEffect;
//#include "StelViewportDistorter.hpp";

//#include "multiprojector/frmprojector.h"
#include "multiprojector/channelhak.h"

//! @class StelAppGraphicsWidget
//! A QGraphicsWidget encapsulating all the Stellarium main sky view and the embedded GUI widgets
//! such as the moving button bars.
//! It manages redirection of users inputs to the core and GUI.
class StelAppGraphicsWidget : public QGraphicsWidget
{
    Q_OBJECT
public:
    StelAppGraphicsWidget();
    ~StelAppGraphicsWidget();

    //! Initialize the StelAppGraphicsWidget.
    void init(class QSettings* conf);

    //! Paint the main sky view and the embedded GUI widgets such as the moving button bars.
    //! This method is called automatically by the GraphicsView.
    virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget=0);

    //! Define the type of viewport effect to use
    //! @param effectName must be one of 'none', 'framebufferOnly', 'sphericMirrorDistorter'
    void setViewportEffect(const QString& effectName);
    //! Get the type of viewport effect currently used
    QString getViewportEffect() const;

    //ASAF
    void updateClientData();
    void updateClientData(double fov);

    static StelAppGraphicsWidget& getInstance() {Q_ASSERT(singleton); return *singleton;}
    StelProjector::StelProjectorParams params;

    //About frehand
    void smoothPath(QPainterPath &path, double smoothness, int from = 0, int to = -1);

protected:
    virtual void keyPressEvent(QKeyEvent* event);
    virtual void keyReleaseEvent(QKeyEvent* event);
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    virtual void wheelEvent(QGraphicsSceneWheelEvent * wheelEvent);
    virtual void resizeEvent(QGraphicsSceneResizeEvent* event);

private:
    double previousPaintTime;
    //! The time at the last time re refreshed the frame
    //! Since paint may decide to stop before we finish to render a complete scene, this is not necessarily the same
    //! than `previousPaintTime`.
    double previousPaintFrameTime;
    //! The main application instance.
    class StelApp* stelApp;
    //! The state of paintPartial method
    int paintState;

    //! set to true to use buffers
    bool useBuffers;

public:
    //! The framebuffer where we are currently drawing the scene
    class QGLFramebufferObject* backgroundBuffer;

    //! The framebuffer that we use while waiting for the drawing to be done
    class QGLFramebufferObject* foregroundBuffer;

    class QGLFramebufferObject* backgroundPrvBuffer;
    class QGLFramebufferObject* foregroundPrvBuffer;
    bool isDrawingPreview;
private:
    //! Initialize the opengl buffer objects.
    void initBuffers();
    //! Swap the buffers
    //! this should be called after we finish the paint
    void swapBuffers();
    void swapPrvBuffers();

    //! Iterate through the drawing sequence.
    bool paintPartial();

    class StelViewportDistorter* viewportEffect;
    void distortPos(QPointF* pos);

    // The StelAppGraphicsWidget singleton
    static StelAppGraphicsWidget* singleton;

    //About freehand
    bool IncludePathPoint(QPainterPath path, QPointF pos, int size);
    bool pressed;

    //About multiprojection
    QString strPCName;
    unsigned int compositorIndex;
    void CalcUV(double px,double py,double pz,double *u,double *v);
    void DomeScreen(double radius);
    void DrawDomeDistortion(QGLFramebufferObject* buf);

    double rateX;
    double rateY;


    //QList<frmProjector*> m_projectors;

    //About location change by mouse
    bool changeLocByMouse;
    QPointF locPosNew;

};

#endif // _STELAPPGRAPHICSWIDGET_HPP_

