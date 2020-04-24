#include <QWidget>
#include "messierimage.h"
#include "StelModuleMgr.hpp"
#include "StelPresentMgr.hpp"
#include "StelApp.hpp"

messierimage::messierimage()
{
    timAnimate = new QTimer(this);
    connect(timAnimate,SIGNAL(timeout()),this,SLOT(on_animate_timeout()));

}

void messierimage::startTimer()
{
    if (!timAnimate->isActive())
        timAnimate->start(10);
}

void messierimage::on_animate_timeout()
{
    StelPresentMgr* sPmgr = GETSTELMODULE(StelPresentMgr);
    switch (animMode)
    {
    case 1:
        dec = dec - 1;
        size = size + 33;
        if (dec <= 20) timAnimate->stop();
        break;
    case 2:
        ra = ra -1;
        if (ra <= 0) timAnimate->stop();
        break;
    case 3:
        ra = ra -1;
        if (ra <= -60) timAnimate->stop();
        break;
    default:
        break;
    }

    QString strparams = QString("%1;%2;%3;%4;%5;%6;%7;%8;%9")
                                       .arg(filepath)
                                       .arg(ra).arg(dec)
                                       .arg(size).arg(rotate)
                                       .arg(p1).arg(p2).arg(aspectratio).arg(0);

    sPmgr->setLayerProperties(id,strparams);
}

