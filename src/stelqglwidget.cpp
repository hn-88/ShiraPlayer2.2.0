#include "stelqglwidget.h"

#include "StelCore.hpp"
#include "StelApp.hpp"
#include "StelPainter.hpp"
#include "StelProjector.hpp"


StelQGLWidget::StelQGLWidget(const QGLFormat& format, QWidget* parent)
 : QGLWidget(format, parent)
{
    qDebug()<<"StelQGLWidget created";
    setAttribute(Qt::WA_PaintOnScreen);
    setAttribute(Qt::WA_NoSystemBackground);
    setAttribute(Qt::WA_OpaquePaintEvent);
    //setAutoFillBackground(false);
    setBackgroundRole(QPalette::Window);
}

void StelQGLWidget::initializeGL()
{
    qDebug()<<"initializeGL done";
    QGLWidget::initializeGL();

    if (!format().stencil())
        qWarning("Could not get stencil buffer; results will be suboptimal");
    if (!format().depth())
        qWarning("Could not get depth buffer; results will be suboptimal");
    if (!format().doubleBuffer())
        qWarning("Could not get double buffer; results will be suboptimal");

}

void StelQGLWidget::paintGL()
{
    qDebug()<<"paintGL done";
    QGLWidget::paintGL();

}

