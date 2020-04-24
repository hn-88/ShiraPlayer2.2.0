#include "StelMainGraphicsView.hpp"

#include "warpwindow.h"
#include "ui_warpwindow.h"

//#include <d3d9.h>
//#include "d3dx9.h"

#include <QMessageBox>
#include <QtGui>
#include <QGLWidget>
#include <QImage>
#include <QDateTime>

//-----------------------------------------------------------------------------
// GLOBALS
//-----------------------------------------------------------------------------
//HWND               g_hWnd       = NULL;
//LPDIRECT3D9        g_pD3D       = NULL;
//LPDIRECT3DDEVICE9  g_pd3dDevice = NULL;
//LPD3DXMESH         g_pTeapotMesh= NULL;
//D3DMATERIAL9       g_teapotMtrl;
//D3DLIGHT9          g_pLight0;
//LPDIRECT3DTEXTURE9 texture      = NULL;

DWORD             g_dwBackBufferWidth  = 0;
DWORD             g_dwBackBufferHeight = 0;

float g_fSpinX = 0.0f;
float g_fSpinY = 0.0f;

void LoadTexture(QImage img)
{
    //D3DXCreateTextureFromFileA(g_pd3dDevice,filename,&texture);
    //QMessageBox::critical(0,"ok","1");
    img.invertPixels();
//    D3DXCreateTextureFromFileInMemory(g_pd3dDevice,img.data_ptr(),img.byteCount(),&texture);

}

//void dxinit(QWidget* m_this)
//{
//
//    g_pD3D = Direct3DCreate9( D3D_SDK_VERSION );
//
//    if( g_pD3D == NULL )
//    {
//        // TO DO: Respond to failure of Direct3DCreate8
//        QMessageBox::critical(0,"ok","1");
//        return;
//    }
//
//    D3DDISPLAYMODE d3ddm;
//
//    if( FAILED( g_pD3D->GetAdapterDisplayMode( D3DADAPTER_DEFAULT, &d3ddm ) ) )
//    {
//        // TO DO: Respond to failure of GetAdapterDisplayMode
//        QMessageBox::critical(0,"ok","2");
//        return;
//    }
//
//    HRESULT hr;
//
//    if( FAILED( hr = g_pD3D->CheckDeviceFormat( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL,
//                                                d3ddm.Format, D3DUSAGE_DEPTHSTENCIL,
//                                                D3DRTYPE_SURFACE, D3DFMT_D16 ) ) )
//    {
//        if( hr == D3DERR_NOTAVAILABLE )
//            // POTENTIAL PROBLEM: We need at least a 16-bit z-buffer!
//            QMessageBox::critical(0,"ok","3");
//        return;
//    }
//
//    //
//    // Do we support hardware vertex processing? if so, use it.
//    // If not, downgrade to software.
//    //
//
//    D3DCAPS9 d3dCaps;
//
//    if( FAILED( g_pD3D->GetDeviceCaps( D3DADAPTER_DEFAULT,
//                                       D3DDEVTYPE_HAL, &d3dCaps ) ) )
//    {
//        // TO DO: Respond to failure of GetDeviceCaps
//        QMessageBox::critical(0,"ok","4");
//        return;
//    }
//
//    DWORD dwBehaviorFlags = 0;
//
//    if( d3dCaps.VertexProcessingCaps != 0 )
//        dwBehaviorFlags |= D3DCREATE_HARDWARE_VERTEXPROCESSING;
//    else
//        dwBehaviorFlags |= D3DCREATE_SOFTWARE_VERTEXPROCESSING;
//
//    //
//    // Everything checks out - create a simple, windowed device.
//
//    D3DPRESENT_PARAMETERS d3dpp;
//    memset(&d3dpp, 0, sizeof(d3dpp));
//
//    d3dpp.BackBufferFormat       = d3ddm.Format;
//    d3dpp.SwapEffect             = D3DSWAPEFFECT_DISCARD;
//    d3dpp.Windowed               = TRUE;
//    d3dpp.EnableAutoDepthStencil = TRUE;
//    d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
//    d3dpp.PresentationInterval   = D3DPRESENT_INTERVAL_IMMEDIATE;
//
//    if( FAILED( g_pD3D->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, g_hWnd,
//                                      dwBehaviorFlags, &d3dpp, &g_pd3dDevice ) ) )
//    {
//        // TO DO: Respond to failure of CreateDevice
//        QMessageBox::critical(0,"ok","5");
//        return;
//    }
//
//    g_pd3dDevice->SetRenderState( D3DRS_LIGHTING, FALSE );
//    g_pd3dDevice->SetRenderState( D3DRS_CULLMODE,0);
//
//    g_pd3dDevice->LightEnable( 0, FALSE );
//
//    g_pd3dDevice->SetRenderState( D3DRS_AMBIENT, D3DCOLOR_COLORVALUE( 0.2f, 0.2f, 0.2f, 1.0f ) );
//
//
//    // Load up the teapot mesh...
//    LPCSTR meshfile="dome_125_3.x";
//    int hres=D3DXLoadMeshFromXA(meshfile, D3DXMESH_SYSTEMMEM, g_pd3dDevice,
//                      NULL, NULL, NULL, NULL, &g_pTeapotMesh );
//
//    if(hres!=0)
//    {
//        QMessageBox::critical(0,"ok","mesh yükleme hatasý");
//        m_this->close();
//    }
//
//    // Cache the width & height of the back-buffer...
//
////    LPDIRECT3DSURFACE9 pBackBuffer = NULL;
////    D3DSURFACE_DESC d3dsd;
////    g_pd3dDevice->GetBackBuffer( 0, 0, D3DBACKBUFFER_TYPE_MONO, &pBackBuffer );
////    pBackBuffer->GetDesc( &d3dsd );
////    pBackBuffer->Release();
////    g_dwBackBufferWidth  = d3dsd.Width;
////    g_dwBackBufferHeight = d3dsd.Height;
//
//
//}

//static void convertFromGLImage(uint* p, int w, int h, bool alpha_format, bool include_alpha)
//{
//    if (QSysInfo::ByteOrder == QSysInfo::BigEndian) {
//        // OpenGL gives RGBA; Qt wants ARGB
//        uint *end = p + w*h;
//        if (alpha_format && include_alpha) {
//            while (p < end) {
//                uint a = *p << 24;
//                *p = (*p >> 8) | a;
//                p++;
//            }
//        } else {
//            // This is an old legacy fix for PowerPC based Macs, which
//            // we shouldn't remove
//            while (p < end) {
//                *p = 0xff000000 | (*p>>8);
//                ++p;
//            }
//        }
//    } else {
//        uint *end = p + w*h;
//        if (alpha_format && include_alpha) {
//            while (p < end) {
//                uint a = *p << 24;
//                *p = (*p >> 8) | a;
//                p++;
//            }
//        } else {
//            // This is an old legacy fix for PowerPC based Macs, which
//            // we shouldn't remove
//            while (p < end) {
//                *p = 0xff000000 | (*p>>8);
//                ++p;
//            }
//        }
//    }
//    //img = img.mirrored();
//}


void RGBA_to_ARGB(UINT32* rgba)
{
    UINT32* argb=new UINT32[800* 800*4];
    for(int i=0;i<sizeof(rgba);i++)
    {
        argb[i] = (rgba[i] >> 8) & 0xffffff;
        argb[i] = argb[i] | ((rgba[i] & 0xff) << 24);
    }
    rgba=argb;
}

void DrawScene(QWidget* m_this)
{
//    QSize size(800,800);
//
//    QImage img(size,QImage::Format_RGB32);


    //GUID g("85C31227-3DE5-4f00-9B3A-F11AC38C18B5");

    ///Load Texture
    QGLWidget* glwindow=StelMainGraphicsView::getInstance().getOpenGLWin();
    if(glwindow!=NULL)
    {
        QImage im = glwindow->grabFrameBuffer();



//        try
//        {
//
//            UINT32* bits=new UINT32[800* 800*4];
//            //GLubyte* bits=new GLubyte[800* 800*4];
//
//            QDateTime qd=QDateTime::currentDateTime();
//            QString stronce,strsonra;
//            stronce=qd.time().toString();
//            qd.time().start();
//
//            glReadPixels(0, 0, 800, 800, GL_RGBA, GL_UNSIGNED_BYTE,bits);
//
//            //glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, bits);
//            RGBA_to_ARGB(bits);
//
//            //glGetTexImage();
//            //convertFromGLImage(bits,800,800,true,true);
//            //int hresult=D3DXCreateTextureFromFileInMemory(g_pd3dDevice,bits,sizeof(bits),&texture);
//
//
//
////            im = glwindow->grabFrameBuffer();
////
////            buffer.open(QIODevice::WriteOnly);
////            im.save(&buffer, "PNG");
////
//
//
//            //int hresult=D3DXCreateTextureFromFileInMemory(g_pd3dDevice,buffer.data(),buffer.data().count(),&texture);
//
//            int hresult=D3DXCreateTextureFromFileInMemory(g_pd3dDevice,bits,sizeof(bits),&texture);
//
//            //buffer.close();
//
//
//            //buffer.close();
//
//            //buffer.reset();
//
//            strsonra=QString("%0").arg(qd.time().elapsed());
//
//            QString str=QString("%0").arg(hresult);
//            //m_this->setWindowTitle(stronce+"-"+strsonra);
//            m_this->setWindowTitle(str);
//        }
//        catch (...)
//        {
//            qWarning() << "ERROR";
//        }



//        D3DXCreateTextureFromFileInMemory(g_pd3dDevice,glwindow,buffer.data().count(),&texture);
//
//        QString str=QString("%0").arg(im.byteCount());
//        m_this->setWindowTitle(str);
    }

//    // Now we can clear just view-port's portion of the buffer to red...
//    g_pd3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
//                         D3DCOLOR_COLORVALUE( 1.0f, 0.0f, 0.0f, 1.0f ), 1.0f, 0 );
//
//    g_pd3dDevice->BeginScene();
//    {
//        g_pd3dDevice->SetMaterial( &g_teapotMtrl );
//        g_pd3dDevice->SetTexture( 0, texture);
//        g_pTeapotMesh->DrawSubset(0);
//        g_pd3dDevice->SetTexture( 0, NULL);
//    }
//    g_pd3dDevice->EndScene();
//
//    g_pd3dDevice->Present( NULL, NULL, NULL, NULL );

}

//void dxrender(QWidget* m_this)
//{
//    D3DXMATRIX matProj;
//    D3DXMATRIX matView;
//    D3DXMATRIX matRotation;
//
//    //Projection
//
//    D3DXMatrixPerspectiveFovLH( &matProj,
//                                D3DX_PI / 4 ,1.0f,73.0f,1000.0f);
//    g_pd3dDevice->SetTransform( D3DTS_PROJECTION, &matProj );
//
//    //View
//    // For the left view-port, leave the view at the origin...
//    D3DXMatrixLookAtLH( &matView,
//                        new D3DXVECTOR3(0.0f,-18.0f,64.0f),
//                        new D3DXVECTOR3(0,0,0),
//                        new D3DXVECTOR3(0,1.0f,0));
//    g_pd3dDevice->SetTransform( D3DTS_VIEW, &matView );
//
//    //Buradaki 360 parçalara ayýrma açýsýný gösteriyor. Her parça için bu açý farklý olacak
//    D3DXMatrixRotationY( &matRotation,(2.0f * D3DX_PI) / 360.0f * 120.0f);
//    g_pd3dDevice->SetTransform( D3DTS_WORLD, &matRotation );
//
//    DrawScene(m_this);
//}

WarpWindow::WarpWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::WarpWindow)
{
    ui->setupUi(this);
//    this->setFixedHeight(800);
//    this->setFixedWidth(1200);

    //ui->panShow->setGeometry(0,0,1000,1000);
//    ui->panShow->setFixedWidth(1000);
//    ui->panShow->setFixedHeight(1000);

    //g_hWnd = ui->panShow->winId();
    //g_hWnd = this->winId();
    //Bu çok önemli
    //setAttribute(Qt::WA_UpdatesDisabled);
    //setAttribute(Qt::WA_WState_InPaintEvent);

   // dxinit(parent);

    //setAttribute(Qt::WA_NoBackground);
    ui->imagelabel->setAlignment(Qt::AlignLeft);
    ui->imagelabel->setFixedSize(800,800);

//    // Avoid white background at init
//    setAttribute(Qt::WA_PaintOnScreen);
//    setAttribute(Qt::WA_NoSystemBackground);
//    setAttribute(Qt::WA_OpaquePaintEvent);
//    setAutoFillBackground(true);
//    setBackgroundRole(QPalette::Window);
//    QPalette pal;
//    pal.setColor(QPalette::Window, Qt::black);
//    setPalette(pal);
}

WarpWindow::~WarpWindow()
{
    delete ui;
}

void WarpWindow::changeEvent(QEvent *e)
{
//    if(g_pd3dDevice!=NULL)
//    {
//        dxrender(this);
//    }

    QMainWindow::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void WarpWindow::paintEvent(QPaintEvent * e)
{
    QPainter painter(this);
    painter.begin(this);
//
//    QGLWidget* glwindow=StelMainGraphicsView::getInstance().getOpenGLWin();
//    QImage im = glwindow->grabFrameBuffer();
//
//    ui->imagelabel->setPixmap(QPixmap::fromImage(im));
//    QDateTime qd=QDateTime::currentDateTime();
//    this->setWindowTitle(qd.time().toString());
    painter.end();

}

void WarpWindow::on_pushButton_clicked()
{

    QGLWidget* glwindow=StelMainGraphicsView::getInstance().getOpenGLWin();
//    StelMainGraphicsView::getInstance().makeGLContextCurrent();
    //glwindow->updateGL();
    glwindow->update();
    QImage im = glwindow->grabFrameBuffer();
 //   ui->imagelabel->setPixmap(QPixmap::grabWidget(glwindow,0,0,800,800));


//    QString shotDir = "c:/test.png";
//    im.save(shotDir);
    ui->imagelabel->setPixmap(QPixmap::fromImage(im));
//    QDateTime qd=QDateTime::currentDateTime();
//    this->setWindowTitle(qd.time().toString());

}
