#ifndef PREVGLWIDGET_HPP
#define PREVGLWIDGET_HPP

#include <QGLWidget>

class PrevGLWidget : public QGLWidget
{
Q_OBJECT
public:
    PrevGLWidget(const QGLFormat& format, QWidget* parent);

    void initializeGL();
    void resizeGL(int w, int h);
    void paintGL();
protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void keyPressEvent(QKeyEvent *event);

};

#endif // PREVGLWIDGET_HPP
