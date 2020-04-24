#include <QMouseEvent>
#include <QMessageBox>

#include "PrevglWidget.hpp"

PrevGLWidget::PrevGLWidget(const QGLFormat& format, QWidget* parent) : QGLWidget(format, parent)
{
    setAttribute(Qt::WA_PaintOnScreen);
    setAttribute(Qt::WA_NoSystemBackground);
    setAttribute(Qt::WA_OpaquePaintEvent);
    setBackgroundRole(QPalette::Window);
    setMouseTracking(true);
}
void PrevGLWidget::initializeGL() {
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_COLOR_MATERIAL);
    glEnable(GL_BLEND);
    glEnable(GL_POLYGON_SMOOTH);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glClearColor(0, 0, 0, 0);
}

void PrevGLWidget::resizeGL(int w, int h) {
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, w, 0, h); // set origin to bottom left corner
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void PrevGLWidget::paintGL() {


    QGLWidget::paintGL();
    //support redirecting painting
    QPaintDevice* device = QPainter::redirected(this);
    if (device != NULL && device != this)
    {
        QImage image = grabFrameBuffer();
        QPainter painter(this);
        painter.drawImage(QPointF(0.0,0.0),image);
    }

    glClear(GL_COLOR_BUFFER_BIT);
    glColor3f(1,0,0);
    glBegin(GL_POLYGON);
    glVertex2f(0,0);
    glVertex2f(100,500);
    glVertex2f(500,100);
    glEnd();
}

void PrevGLWidget::mousePressEvent(QMouseEvent *event)
{

}
void PrevGLWidget::mouseMoveEvent(QMouseEvent *event)
{
    //printf("%d, %d\n", event->x(), event->y());
}

void PrevGLWidget::keyPressEvent(QKeyEvent* event)
{
    switch(event->key()) {
    case Qt::Key_0:
        QMessageBox::information(0,"","",0,0);
        break;
    default:
        event->ignore();
        break;
    }
}
