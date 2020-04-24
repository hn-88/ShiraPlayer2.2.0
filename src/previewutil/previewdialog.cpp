
#include "previewdialog.hpp"
#include "ui_previewdialog.h"
#include "PrevPainter.hpp"
#include "PrevglWidget.hpp"
#include "StelPainter.hpp"

#include "QGLWidget"
#include "QGraphicsView"
#include "StelAppGraphicsWidget.hpp"
#include "shiraplayerform.hpp"
#include "StelFileMgr.hpp"
#include "StelGui.hpp"
#include "StelMainWindow.hpp"
#include "StelIniParser.hpp"

#include "openglscene.hpp"

class StelQGLWidget;


PreviewDialog::PreviewDialog(StelGui* agui)
{
    ui = new Ui_PreviewDialog();
}

PreviewDialog::~PreviewDialog()
{
    delete ui;
}

void PreviewDialog::languageChanged()
{
    if (dialog)
        ui->retranslateUi(dialog);
}

void PreviewDialog::styleChanged()
{
    // Nothing for now
}
void PreviewDialog::createDialogContent()
{
    ui->setupUi(dialog);
    connect(ui->closeStelWindow, SIGNAL(clicked()), this, SLOT(close()));

//    // Avoid white background at init
//    ui->graphicsView->setAttribute(Qt::WA_PaintOnScreen);
//    ui->graphicsView->setAttribute(Qt::WA_NoSystemBackground);
//    ui->graphicsView->setAttribute(Qt::WA_OpaquePaintEvent);
//    ui->graphicsView->setAutoFillBackground(true);
//    ui->graphicsView->setBackgroundRole(QPalette::Window);
//    QPalette pal;
//    pal.setColor(QPalette::Window, Qt::black);
//    ui->graphicsView->setPalette(pal);

  //  ui->graphicsView->setViewport(new QGLWidget(QGLFormat(QGL::StencilBuffer | QGL::DepthBuffer | QGL::DoubleBuffer )));
  //  ui->graphicsView->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
  //  ui->graphicsView->setScene(new OpenGLScene);
  //  ui->graphicsView->show();


     //Create an openGL viewport
//    QGLFormat glFormat(QGL::StencilBuffer | QGL::DepthBuffer | QGL::DoubleBuffer );
//    glWidget = new QGLWidget(glFormat, ui->graphicsView);
//    glWidget->setAttribute(Qt::WA_PaintOnScreen);
//    glWidget->setAttribute(Qt::WA_NoSystemBackground);
//    glWidget->setAttribute(Qt::WA_OpaquePaintEvent);
//    glWidget->setBackgroundRole(QPalette::Window);
//    glWidget->updateGL();

   // ui->graphicsView->setViewport(StelMainGraphicsView::getInstance().getOpenGLWin());
   // StelMainGraphicsView::getInstance().setGeometry(0,0,512,512);
   // ui->graphicsView->setScene(StelMainGraphicsView::getInstance().scene());

    //StelMainGraphicsView::getInstance().setParent(ui->graphicsView->scene());
    //ui->verticalLayout->addWidget((QWidget*)StelMainGraphicsView::getInstance().getOpenGLWin()->parentWidget());

    //ui->graphicsView->setViewport(glWidget);

    //    PrevApp::initStatic();
    //QGraphicsView* prevView = new QGraphicsView(ui->frame);
    //prevView->setSceneRect();

    //    ui->verticalLayout->addWidget(prevView);
    //
    //    // Avoid white background at init
    //    prevView->setAttribute(Qt::WA_PaintOnScreen);
    //    prevView->setAttribute(Qt::WA_NoSystemBackground);
    //    prevView->setAttribute(Qt::WA_OpaquePaintEvent);
    //    prevView->setAutoFillBackground(true);
    //    prevView->setBackgroundRole(QPalette::Window);
    //    QPalette pal;
    //    pal.setColor(QPalette::Window, Qt::black);
    //    prevView->setPalette(pal);
    //
    //    // Allows for precise FPS control
    //    prevView->setViewportUpdateMode(QGraphicsView::NoViewportUpdate);
    //    prevView->setFrameShape(QFrame::NoFrame);
    //    prevView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    //    prevView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    //    prevView->setFocusPolicy(Qt::StrongFocus);
    //
    //    // Create an openGL viewport
    //    QGLFormat glFormat(QGL::StencilBuffer | QGL::DepthBuffer | QGL::DoubleBuffer );
    //    glWidget = new QGLWidget(glFormat, prevView);
    //    glWidget->setAttribute(Qt::WA_PaintOnScreen);
    //    glWidget->setAttribute(Qt::WA_NoSystemBackground);
    //    glWidget->setAttribute(Qt::WA_OpaquePaintEvent);
    //    glWidget->setBackgroundRole(QPalette::Window);
    //    glWidget->updateGL();
    //    prevView->setViewport(glWidget);
    //
    //    prevView->setScene(new QGraphicsScene());
    //    backItem = new QGraphicsWidget();
    //    backItem->setFocusPolicy(Qt::NoFocus);
    //
    //    Q_ASSERT(glWidget!=NULL);
    //    Q_ASSERT(glWidget->isValid());
    //    glWidget->makeCurrent();
    //
    //    prevSkyItem = new PrevAppGraphicWidget();
    //    prevSkyItem->setZValue(-10);
    //    QGraphicsGridLayout* l = new QGraphicsGridLayout(backItem);
    //    l->setContentsMargins(0,0,0,0);
    //    l->setSpacing(0);
    //    l->addItem(prevSkyItem, 0, 0);
    //    prevView->scene()->addItem(backItem);
    //
    //    prevSkyItem->setFocus();
    //
    //    QPainter qPainter(glWidget);
    //   // PrevPainter::setQPainter(&qPainter);


    //prevGLWidget = new PrevGLWidget(NULL);


    //
    //         QGraphicsProxyWidget *proxy =ShiraPlayerForm::getInstance().scene()->addWidget((QWidget*)StelMainGraphicsView::getInstance().getOpenGLWin()->parent());
    //         proxy->setPos(10,10);
    //QGraphicsView view(ShiraPlayerForm::getInstance().scene());
    //view.setParent(ui->frameWidget);
    //view.show();

    //   prevGLWidget = new PrevGLWidget(NULL);
    //   ui->frameLayout->addWidget(prevGLWidget);
    // prevGLWidget->makeCurrent();
    //  prevGLWidget->initializeGL();
    //  prevGLWidget->updateGL();
    //prevGLWidget->setContext((QGLContext*)StelMainGraphicsView::getInstance().getOpenGLWin()->context(),NULL,true);

    //ui->frameLayout->addWidget((QWidget*)StelMainGraphicsView::getInstance().getStelAppGraphicsWidget());

    //prevGraphicsView = new StelMainGraphicsView(ui->frame);

    //    getProxy()->setAttribute(Qt::WA_NoSystemBackground);
    //
    //    getProxy()->setAttribute(Qt::WA_PaintOnScreen);
    //    getProxy()->setAttribute(Qt::WA_OpaquePaintEvent);

    //getProxy()->widget()->setUpdatesEnabled(true);// setViewportUpdateMode(QGraphicsView::NoViewportUpdate);



    //StelMainGraphicsView::getInstance().setParent(ui->frameWidget);

    //StelMainGraphicsView::getInstance().getTopLevelGraphicsWidget()->setParent(ui->frameWidget);

    //QWidget* testWidget = new QWidget(ShiraPlayerForm::getInstance().scene());
    //QHBoxLayout* hbox = new QHBoxLayout();
    //ShiraPlayerForm::getInstance().scene()->addWidget(prevGLWidget);
    //QGraphicsSimpleTextItem* text= ShiraPlayerForm::getInstance().scene()->addSimpleText("TEST",QFont("Tahoma",16,16,false));
    //text->setPos(10, 10);
    //ShiraPlayerForm::getInstance().scene()->addWidget((QWidget*)StelMainGraphicsView::getInstance().getOpenGLWin()->parentWidget());
    //ui->frameLayout->addWidget((QWidget*)StelMainGraphicsView::getInstance().getOpenGLWin()->parentWidget());
    //    hbox->addWidget((QWidget*)StelMainGraphicsView::getInstance().getOpenGLWin()->parentWidget());
    //ShiraPlayerForm::getInstance().scene()->addWidget(testWidget);
    //ui->frameLayout->addWidget(StelMainGraphicsView::getInstance().scene()->);
    //ui->frameLayout->addWidget(StelMainGraphicsView::getInstance().graphicsProxyWidget());
    //ui->frameLayout->addWidget((QWidget*)StelMainGraphicsView::getInstance().getTopLevelGraphicsWidget());
    //StelMainGraphicsView::getInstance().setGeometry(0,0,512,512);

    //ShiraPlayerForm::getInstance().scene()-> addWidget((QWidget*)StelMainGraphicsView::getInstance().getOpenGLWin()->parent());
    //QHBoxLayout* hbox = new QHBoxLayout();
    //hbox->addWidget((QWidget*)StelMainGraphicsView::getInstance().getOpenGLWin()->parent());


    //QPainter qP(StelMainGraphicsView::getInstance().getOpenGLWin());
    //StelPainter::setQPainter(&qP);
    //StelMainGraphicsView::getInstance().makeGLContextCurrent();

    //ShiraPlayerForm::getInstance().scene()->addPixmap();

    //            QPixmap pm;
    //
    //            QString path = StelFileMgr::findFile("c:/Tulips.png");
    //                pm = QPixmap(path);
    //
    //            QGraphicsPixmapItem* tex =  ShiraPlayerForm::getInstance().scene()->addPixmap(pm);
    //            tex->setPos(10, 10);
    //StelMainGraphicsView::getInstance().setParent(ui->frameWidget);

    // StelMainGraphicsView::getInstance().startMainLoop();
    //    disconnect(StelMainGraphicsView::getInstance().minFpsTimer, SIGNAL(timeout()), 0, 0);
    //    delete StelMainGraphicsView::getInstance().minFpsTimer;
    //    StelMainGraphicsView::getInstance().minFpsTimer = NULL;

    //  startMainLoop();


//    QString configFileFullPath = StelFileMgr::findFile( "config.ini", StelFileMgr::Flags(StelFileMgr::Writable|StelFileMgr::File));
//    QSettings* confSettings = NULL;
//    if (StelFileMgr::exists(configFileFullPath))
//    {
//        confSettings = new QSettings(configFileFullPath, StelIniFormat, NULL);
//    }
//
//    prevGraphicsView = new StelMainGraphicsView(ui->frameWidget);
//
//    prevGraphicsView->init(confSettings);
//    QMessageBox::information(0,0,"","",0);

 //   StelMainGraphicsView::getInstance().setParent((QWidget*)ShiraPlayerForm::getInstance().scene()->parent());
    //StelMainGraphicsView::getInstance().setGeometry(0,0,512,512);
}
