#include <QtGui>
#include <QtOpenGL>

#include "openglscene.hpp"

OpenGLScene::OpenGLScene()
{

}
void OpenGLScene::drawBackground(QPainter *painter, const QRectF &rect)
{
//    if (painter->paintEngine()->type() != QPaintEngine::OpenGL) {
//        qWarning("OpenGLScene: drawBackground needs a QGLWidget to be set as viewport on the graphics view");
//   //     return;
//    }

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glColor3f(1,0,0);
    glBegin(GL_POLYGON);
    glVertex2f(0,0);
    glVertex2f(100,500);
    glVertex2f(500,100);
    glEnd();    


    QTimer::singleShot(20, this, SLOT(update()));
}
