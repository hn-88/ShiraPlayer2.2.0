#ifndef FRMPROJECTOR_H
#define FRMPROJECTOR_H

#include <QGLWidget>
#include <QGLFramebufferObject>
#include <QOpenGLFunctions>

#include "objloader.h"


QT_FORWARD_DECLARE_CLASS(QGLShaderProgram)

class frmProjector : public QGLWidget
{
protected:
    void paintGL();    
    void resizeGL(int width, int height);
    QGLFramebufferObject* buffer;

    ObjLoader objL;

    GLuint texture;
public:
    explicit frmProjector(QWidget *parent = 0, QGLWidget *shareWidget = 0);
    //~frmProjector();
    void initializeGL() ;
    void setBuffer(QGLFramebufferObject* buf) {buffer = buf; }
    void setParams(const QString& _objFile, const QString& _blendFile, const QRect &_geometry);
    void ClearObjData();
    bool LoadObjData(const QString& filename);
    bool LoadBlendData(const QString& filename);
    void setFramelessWindow(bool val);

    void setTexture(GLuint tex){ texture = tex;}
private:

    QVector<QVector3D> vertices;
    QVector<QVector2D> texCoords;
    QGLShaderProgram *program;
    GLuint texBlendmask;

    QString objfile,blendfile;

};

#endif // FRMPROJECTOR_H
