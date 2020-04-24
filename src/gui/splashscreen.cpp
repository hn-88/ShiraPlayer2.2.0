#include "splashscreen.h"
#include "ui_splashscreen.h"
#include <QDebug>
#include "licenceutils/qlisansform.h"
#include "StelApp.hpp"
#include "StelCore.hpp"
#include "StelMainWindow.hpp"
#include "shiraprojector.h"

splashScreen::splashScreen(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::splashScreen)
{
    ui->setupUi(this);

    setWindowFlags(Qt::Widget | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    //    setParent(0); // Create TopLevel-Widget
    setAttribute(Qt::WA_NoSystemBackground, true);
    setAttribute(Qt::WA_TranslucentBackground, true);
    setAttribute(Qt::WA_PaintOnScreen);
    fader = new LinearFader();
    fader.setDuration(6000);
    fader = true;

    timerId = startTimer(50);

    if(StelMainWindow::getInstance().is_Licenced)
       ui->lblUser->setText("Licensed User: "+ StelMainWindow::getInstance().uname);
    else
       ui->lblUser->setText("UnLicensed User");


    ui->lblversion->setText(QString("ver: %0").arg(PACKAGE_VERSION));

    ui->listPlugins->addItem(StelMainWindow::getInstance().projdll->getPluginName());

}

splashScreen::~splashScreen()
{
    delete ui;
}

void splashScreen::timerEvent(QTimerEvent *e)
{
    fader.update((int)(300));
    repaint();
}
void splashScreen::paintEvent(QPaintEvent *event)
{

    //qDebug()<< fader.getInterstate();

    this->setWindowOpacity(1.0-fader.getInterstate());
    if (this->windowOpacity()== 0 )
    {
        killTimer(timerId);
        this->hide();
    }
}

