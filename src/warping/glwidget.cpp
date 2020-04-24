
#include <QtGui>
#include <QtOpenGL>
#include <GL/glext.h>

#include <math.h>

#include "glwidget.h"
#include "qtlogo.h"

#include "StelMainGraphicsView.hpp"

#ifndef GL_MULTISAMPLE
#define GL_MULTISAMPLE  0x809D
#endif


// function pointers for PBO Extension
// Windows needs to get function pointers from ICD OpenGL drivers,
// because opengl32.dll does not support extensions higher than v1.1.
#ifdef _WIN32
PFNGLGENBUFFERSARBPROC pglGenBuffersARB = 0;                     // VBO Name Generation Procedure
PFNGLBINDBUFFERARBPROC pglBindBufferARB = 0;                     // VBO Bind Procedure
PFNGLBUFFERDATAARBPROC pglBufferDataARB = 0;                     // VBO Data Loading Procedure
PFNGLBUFFERSUBDATAARBPROC pglBufferSubDataARB = 0;               // VBO Sub Data Loading Procedure
PFNGLDELETEBUFFERSARBPROC pglDeleteBuffersARB = 0;               // VBO Deletion Procedure
PFNGLGETBUFFERPARAMETERIVARBPROC pglGetBufferParameterivARB = 0; // return various parameters of VBO
PFNGLMAPBUFFERARBPROC pglMapBufferARB = 0;                       // map VBO procedure
PFNGLUNMAPBUFFERARBPROC pglUnmapBufferARB = 0;                   // unmap VBO procedure
#define glGenBuffersARB           pglGenBuffersARB
#define glBindBufferARB           pglBindBufferARB
#define glBufferDataARB           pglBufferDataARB
#define glBufferSubDataARB        pglBufferSubDataARB
#define glDeleteBuffersARB        pglDeleteBuffersARB
#define glGetBufferParameterivARB pglGetBufferParameterivARB
#define glMapBufferARB            pglMapBufferARB
#define glUnmapBufferARB          pglUnmapBufferARB
#endif

GLubyte* imgdata; // Image's Data (Pixels)
long data_size;
GLuint	g_texid[1];
GLuint pboIds[2];  // IDs of PBO

int w,h;


//! [0]
GLWidget::GLWidget(QWidget *parent)
    : QGLWidget(QGLFormat(QGL::SampleBuffers), parent)
{
    logo = 0;
    xRot = 0;
    yRot = 0;
    zRot = 0;
    zoom = 0.5f;

    qtGreen = QColor::fromCmykF(0.40, 0.0, 1.0, 0.0);
    qtPurple = QColor::fromCmykF(0.39, 0.39, 0.0, 0.0);
}
//! [0]

//! [1]
GLWidget::~GLWidget()
{
}
//! [1]

//! [2]
QSize GLWidget::minimumSizeHint() const
{
    return QSize(50, 50);
}
//! [2]

//! [3]
QSize GLWidget::sizeHint() const
//! [3] //! [4]
{
    return QSize(400, 400);
}
//! [4]

static void qNormalizeAngle(int &angle)
{
    while (angle < 0)
        angle += 360 * 16;
    while (angle > 360 * 16)
        angle -= 360 * 16;
}

//! [5]
void GLWidget::setXRotation(int angle)
{
    qNormalizeAngle(angle);
    if (angle != xRot) {
        xRot = angle;
        emit xRotationChanged(angle);
        updateGL();
    }
}
//! [5]

void GLWidget::setYRotation(int angle)
{
    qNormalizeAngle(angle);
    if (angle != yRot) {
        yRot = angle;
        emit yRotationChanged(angle);
        updateGL();
    }
}

void GLWidget::setZRotation(int angle)
{
    qNormalizeAngle(angle);
    if (angle != zRot) {
        zRot = angle;
        emit zRotationChanged(angle);
        updateGL();
    }
}

//! [6]
void GLWidget::initializeGL()
{


    qglClearColor(qtPurple.dark());

//    logo = new QtLogo(this, 64);
//    logo->setColor(qtGreen.dark());

    //ASAF
    m_pModel = glmReadOBJ("c:/domemesh.obj");
    glmFacetNormals(m_pModel);
    glmVertexNormals(m_pModel, 89.5f);
    m_modelIndex = glGenLists(1);
    glNewList(m_modelIndex, GL_COMPILE);
    glEnable(GL_TEXTURE_2D);//ASAF
    glmDraw(m_pModel, (GLM_FLAT) | GLM_MATERIAL | GLM_TEXTURE);
    glEndList();

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glShadeModel(GL_SMOOTH);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_MULTISAMPLE);
    static GLfloat lightPosition[4] = { 0.5, 5.0, 7.0, 1.0 };
    glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);


//    QImage im ;
//    im.load("c:/fulldome.bmp");

    glStelwindow = StelMainGraphicsView::getInstance().getOpenGLWin();
    w = glStelwindow->width();
    h = glStelwindow->height();
    setup_textures(w,h);
    int NbBytes = w *h ;
    pPixelData = new unsigned char[NbBytes];

    m_timer = new QTimer(this);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(updateStellaGrab()));
    m_timer->start(30);
}
//! [6]

//! [7]
void GLWidget::paintGL()
{
    glPushAttrib(GL_ENABLE_BIT);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    glTranslatef(0.0, 0.0, -10.0);
    glRotatef(xRot / 16.0, 1.0, 0.0, 0.0);
    glRotatef(yRot / 16.0, 0.0, 1.0, 0.0);
    glRotatef(zRot / 16.0, 0.0, 0.0, 1.0);
    //logo->draw();

    static int index = 0;
    int nextIndex = 0;                  // pbo index used for next frame

    if (m_modelIndex)
    {
        glEnable(GL_CULL_FACE);
        glCallList(m_modelIndex);
    }


//    if(!im.isNull())
//    {
//        texture = QGLWidget::convertToGLFormat(im);
//
//        //if(isActiveWindow())
//        {
//            //glStelwindow = StelMainGraphicsView::getInstance().getOpenGLWin();
//            //QMessageBox::critical(this,"ok",glStelwindow->windowTitle(),0,0);
//            //glStelwindow->grabFrameBuffer(true);
//            //im = StelMainGraphicsView::getInstance().getOpenGLWin()->grabFrameBuffer();
//           // QImage texture = QGLWidget::convertToGLFormat(im);
//
////            glStelwindow->makeCurrent();
////            im = glStelwindow->grabFrameBuffer();
////            texture = QGLWidget::convertToGLFormat(im);
//
//
//
////            GLvoid* pixels;
////            glCopyTexImage2D(GL_TEXTURE_2D,0,GL_RGB,0,0,128,128,0);
////            glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels);
//
//         //   glReadBuffer(GL_FRONT);
//          //  glReadPixels(0, 0, w, h, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, pPixelData );
//
////            glReadBuffer(GL_COLOR_ATTACHMENT0_EXT);
////            glReadPixels(0,0,w, h,GL_RGB, GL_UNSIGNED_BYTE, pPixelData);
//
//           // this->makeCurrent();
//           // glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w,h, GL_RGB, GL_UNSIGNED_BYTE, pPixelData);
//
//
//
//            glEnable(GL_TEXTURE_2D);
//            glBindTexture(GL_TEXTURE_2D, g_texid[0]);
//            //glTexImage2D(GL_TEXTURE_2D, 0, 3, w, h, 0,
//            //            GL_RGBA, GL_UNSIGNED_BYTE, texture.bits());
//
//            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w , h, GL_RGBA, GL_UNSIGNED_BYTE, texture.bits());
////
////            glBindTexture(GL_TEXTURE_2D, g_texid[0]);
////
////            index = (index + 1) % 2;
////            nextIndex = (index + 1) % 2;
////
////            glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, pboIds[index]);
////
////            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w, h, GL_RGB, GL_UNSIGNED_BYTE, 0);
////
////            glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, pboIds[nextIndex]);
////
////            glBufferDataARB(GL_PIXEL_UNPACK_BUFFER_ARB, data_size, 0, GL_STREAM_DRAW_ARB);
////            GLubyte* ptr = (GLubyte*)glMapBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, GL_WRITE_ONLY_ARB);
////            if(ptr)
////            {
////                memcpy(ptr , texture.bits(),sizeof(texture.bits()));
////                //                if(getMovieWidth()== getMovieHeight())
////                //                        av_Flip_Image( ptr,getMovieWidth(),getMovieHeight(), get_currFrame()->linesize[0]/getMovieHeight()*8);
////                //CopyMemory(ptr , (GLuint **)get_currFrame()->data[0], get_movie_size() );
////                glUnmapBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB); // release pointer to mapping buffer
////
////            }
////            glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, 0);
////
////            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	// Clear The Screen And The Depth Buffer
////
////            glBindTexture(GL_TEXTURE_2D, g_texid[0]);
//
//
//            glEnable(GL_TEXTURE_2D);
//
//            if(m_pModel!=NULL)
//                glmDraw(m_pModel, (GLM_FLAT) | GLM_MATERIAL | GLM_TEXTURE);
//
//            glPopAttrib();
//            glPopMatrix();
//        }
//    }



    glmDraw(m_pModel, (GLM_FLAT) | GLM_MATERIAL | GLM_TEXTURE);
}
//! [7]

//! [8]
void GLWidget::resizeGL(int width, int height)
{
    int side = qMin(width, height);
    glViewport((width - side) / 2, (height - side) / 2, side, side);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
#ifdef QT_OPENGL_ES_1
    glOrthof(-0.5, +0.5, -0.5, +0.5, 4.0, 15.0);
#else
    glOrtho(-0.5, +0.5, -0.5, +0.5, 4.0, 100.0);
#endif
    glMatrixMode(GL_MODELVIEW);
}
//! [8]

//! [9]
void GLWidget::mousePressEvent(QMouseEvent *event)
{
    lastPos = event->pos();
}
//! [9]

//! [10]
void GLWidget::mouseMoveEvent(QMouseEvent *event)
{
    int dx = event->x() - lastPos.x();
    int dy = event->y() - lastPos.y();

    if (event->buttons() & Qt::LeftButton) {
        setXRotation(xRot + 8 * dy);
        setYRotation(yRot + 8 * dx);

    } else if (event->buttons() & Qt::RightButton) {
        setXRotation(xRot + 8 * dy);
        setZRotation(zRot + 8 * dx);
    }
    lastPos = event->pos();
}
//! [10]
void GLWidget::wheelEvent(QWheelEvent *event)
{
    zoom = zoom + (event->delta()/120.0)/10.0;

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-zoom, +zoom, -zoom, +zoom, 4.0, 100.0);
    glMatrixMode(GL_MODELVIEW);

    updateGL();

    event->accept();
}

bool GLWidget::setup_textures(int TEX_WIDTH,int TEX_HEIGHT)
{


    data_size = 4 * sizeof(GLubyte) * TEX_HEIGHT * TEX_WIDTH;
    imgdata = (GLubyte*) malloc(data_size);
    FillMemory(imgdata,data_size,0x7f);

    // Create The Textures' Id List
    glGenTextures(1, g_texid);
    // Typical Texture Generation Using Data From The Image
    glBindTexture(GL_TEXTURE_2D, g_texid[0]);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, 3, TEX_WIDTH, TEX_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, imgdata);
    //glTexSubImage2D(GL_TEXTURE_2D,0,0,0,img.w,img.h,GL_BGRA,GL_UNSIGNED_BYTE, img.data);
    // Finished With Our Image, Free The Allocated Data
    delete[] imgdata;

    ///PBO iþlemleri
    // get pointers to GL functions
    glGenBuffersARB = (PFNGLGENBUFFERSARBPROC)wglGetProcAddress("glGenBuffersARB");
    glBindBufferARB = (PFNGLBINDBUFFERARBPROC)wglGetProcAddress("glBindBufferARB");
    glBufferDataARB = (PFNGLBUFFERDATAARBPROC)wglGetProcAddress("glBufferDataARB");
    glBufferSubDataARB = (PFNGLBUFFERSUBDATAARBPROC)wglGetProcAddress("glBufferSubDataARB");
    glDeleteBuffersARB = (PFNGLDELETEBUFFERSARBPROC)wglGetProcAddress("glDeleteBuffersARB");
    glGetBufferParameterivARB = (PFNGLGETBUFFERPARAMETERIVARBPROC)wglGetProcAddress("glGetBufferParameterivARB");
    glMapBufferARB = (PFNGLMAPBUFFERARBPROC)wglGetProcAddress("glMapBufferARB");
    glUnmapBufferARB = (PFNGLUNMAPBUFFERARBPROC)wglGetProcAddress("glUnmapBufferARB");

    // create 2 pixel buffer objects, you need to delete them when program exits.
    // glBufferDataARB with NULL pointer reserves only memory space.
    glGenBuffersARB(2, pboIds);
    glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, pboIds[0]);
    glBufferDataARB(GL_PIXEL_UNPACK_BUFFER_ARB, data_size, 0, GL_STREAM_DRAW_ARB);
    glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, pboIds[1]);
    glBufferDataARB(GL_PIXEL_UNPACK_BUFFER_ARB, data_size, 0, GL_STREAM_DRAW_ARB);
    glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, 0);

    return true;
}

void GLWidget::updateStellaGrab()
{
    static int index = 0;
    int nextIndex = 0;                  // pbo index used for next frame

    if(StelMainGraphicsView::getInstance().window()!= NULL)
    {
        if(QThread::currentThread()->isRunning())
        {
            StelMainGraphicsView::getInstance().saveScreenShot("","");
            im = StelMainGraphicsView::getInstance().imagescreen;
            if(!im.isNull())
            {

                texture = QGLWidget::convertToGLFormat(im);

                //if(isActiveWindow())
                {
                    //glStelwindow = StelMainGraphicsView::getInstance().getOpenGLWin();
                    //QMessageBox::critical(this,"ok",glStelwindow->windowTitle(),0,0);
                    //glStelwindow->grabFrameBuffer(true);
                    //im = StelMainGraphicsView::getInstance().getOpenGLWin()->grabFrameBuffer();
                    // QImage texture = QGLWidget::convertToGLFormat(im);

                    //            glStelwindow->makeCurrent();
                    //            im = glStelwindow->grabFrameBuffer();
                    //            texture = QGLWidget::convertToGLFormat(im);



                    //            GLvoid* pixels;
                    //            glCopyTexImage2D(GL_TEXTURE_2D,0,GL_RGB,0,0,128,128,0);
                    //            glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels);

                    //   glReadBuffer(GL_FRONT);
                    //  glReadPixels(0, 0, w, h, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, pPixelData );

                    //            glReadBuffer(GL_COLOR_ATTACHMENT0_EXT);
                    //            glReadPixels(0,0,w, h,GL_RGB, GL_UNSIGNED_BYTE, pPixelData);

                    // this->makeCurrent();
                    // glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w,h, GL_RGB, GL_UNSIGNED_BYTE, pPixelData);



                    //glEnable(GL_TEXTURE_2D);
                    //glBindTexture(GL_TEXTURE_2D, g_texid[0]);
                    //glTexImage2D(GL_TEXTURE_2D, 0, 3, w, h, 0,
                    //            GL_RGBA, GL_UNSIGNED_BYTE, texture.bits());

                    //glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w , h, GL_RGBA, GL_UNSIGNED_BYTE, texture.bits());

                    glBindTexture(GL_TEXTURE_2D, g_texid[0]);

                    index = (index + 1) % 2;
                    nextIndex = (index + 1) % 2;

                    glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, pboIds[index]);

                    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, 0);

                    glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, pboIds[nextIndex]);

                    glBufferDataARB(GL_PIXEL_UNPACK_BUFFER_ARB, data_size, 0, GL_STREAM_DRAW_ARB);
                    GLubyte* ptr = (GLubyte*)glMapBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, GL_WRITE_ONLY_ARB);
                    if(ptr)
                    {
                        memcpy(ptr , texture.bits(),texture.numBytes());
                        //                if(getMovieWidth()== getMovieHeight())
                        //                        av_Flip_Image( ptr,getMovieWidth(),getMovieHeight(), get_currFrame()->linesize[0]/getMovieHeight()*8);
                        //CopyMemory(ptr , (GLuint **)get_currFrame()->data[0], get_movie_size() );
                        glUnmapBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB); // release pointer to mapping buffer

                    }
                    glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, 0);

                    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	// Clear The Screen And The Depth Buffer

                    glBindTexture(GL_TEXTURE_2D, g_texid[0]);


                    glEnable(GL_TEXTURE_2D);

                    if(m_pModel!=NULL)
                        glmDraw(m_pModel, (GLM_FLAT) | GLM_MATERIAL | GLM_TEXTURE);

                    glPopAttrib();
                    glPopMatrix();
                }

                updateGL();
            }
        }
    }
}
