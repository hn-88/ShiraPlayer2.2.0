#ifndef STELQGLWIDGET_H
#define STELQGLWIDGET_H

#include <QtOpenGL/QGLWidget>
#include <QDebug>

class StelQGLWidget : public QGLWidget
{
public:
    StelQGLWidget(const QGLFormat& format, QWidget* parent);

protected:
    virtual void initializeGL();
    virtual void paintGL();
signals:

public slots:
};

#endif // STELQGLWIDGET_H
