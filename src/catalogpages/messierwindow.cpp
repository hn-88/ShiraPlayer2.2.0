#include "QDebug"

#include "messierwindow.h"
#include "ui_messierwindow.h"
#include "presenter/flowlayout.h"
#include "NebulaMgr.hpp"
#include "Nebula.hpp"
#include "StelModuleMgr.hpp"
#include "StelObject.hpp"
#include "StelObjectMgr.hpp"
#include "StelApp.hpp"
#include "StelMovementMgr.hpp"
#include "StelCore.hpp"
#include "StelNavigator.hpp"
#include "LandscapeMgr.hpp"
#include "StelFileMgr.hpp"
#include "StelPresentMgr.hpp"
#include "StelSkyLayer.hpp"
#include "encrypt/cBinExt.h"
#include "messierimage.h"
#include "StelTranslator.hpp"

messierWindow::messierWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::messierWindow)
{
    ui->setupUi(this);
    LoadButtonObjects();
}

messierWindow::~messierWindow()
{
    delete ui;
}

void messierWindow::languageChanged()
{
   //ui->retranslateUi();
   ui->chMove->setText(q_("Move to Object"));
   ui->chHideLand->setText(q_("Hide Landscape && Atmosphere"));
   ui->btnHideAll->setText(q_("Hide All"));
}

void messierWindow::LoadButtonObjects()
{
    flowLayout = new FlowLayout();
    NebulaMgr* nmgr = GETSTELMODULE(NebulaMgr);
    for (int i = 1 ; i<= 110; i++)
    {
        QPushButton* childWidget = new QPushButton(QString("M %0").arg(i));
        childWidget->setObjectName(QString("btnM%0").arg(i));
        childWidget->setGeometry(0,0,80,100);
        childWidget->setMinimumSize(80,100);

        childWidget->setCheckable(true);
        if (i != 45) // M45 Pleides
            childWidget->setAccessibleName(QString("M%0").arg(i));
        else
            childWidget->setAccessibleName("Pleiades");

        Nebula* n = dynamic_cast<Nebula*>(nmgr->searchByName(childWidget->accessibleName()).data());
        if (n)
        {
            if (n->getTypeString() == q_("Galaxy"))
                childWidget->setStyleSheet("QPushButton { \n     background-position: center bottom; background-repeat: no-repeat;background-origin: content; background-image: url(':/graphicGui/gui/galaxy.png');   text-align:top;"
                                           "border-style: inset;\n     border-width: 1px;\n     border-radius:5px;\n	 padding: 3px;\n     border-style: solid;\n     border-width: 1px;\n     border-radius:5px;\n     border-color: palette(button);\n	 padding: 3px;\n     font-size: 12pt;\n}\n\nQPushButton:checked { \n     border-color: palette(dark);\n    background-color: qradialgradient(cx: 0.7, cy: 1.4, fx: 0.7, fy: 1.9, radius: 1.5, stop: 0 rgb(255, 255, 0), stop: 1 transparent );\n}\n\nQPushButton:pressed { \n     border-color: palette(dark);\n     background-color: qradialgradient(cx: 0.7, cy: 1.4, fx: 0.7, fy: 1.9, radius: 1.5, stop: 0 palette(dark), stop: 1 transparent );\n}\n\n");
            else if ( n->getTypeString() == q_("Open cluster"))
                childWidget->setStyleSheet("QPushButton { \n     background-position: center bottom; background-repeat: no-repeat;background-origin: content; background-image: url(':/graphicGui/gui/opencluster.png');   text-align:top;"
                                           "border-style: inset;\n     border-width: 1px;\n     border-radius:5px;\n	 padding: 3px;\n     border-style: solid;\n     border-width: 1px;\n     border-radius:5px;\n     border-color: palette(button);\n	 padding: 3px;\n     font-size: 12pt;\n}\n\nQPushButton:checked { \n     border-color: palette(dark);\n    background-color: qradialgradient(cx: 0.7, cy: 1.4, fx: 0.7, fy: 1.9, radius: 1.5, stop: 0 rgb(255, 255, 0), stop: 1 transparent );\n}\n\nQPushButton:pressed { \n     border-color: palette(dark);\n     background-color: qradialgradient(cx: 0.7, cy: 1.4, fx: 0.7, fy: 1.9, radius: 1.5, stop: 0 palette(dark), stop: 1 transparent );\n}\n\n");
            else if ( n->getTypeString() == q_("Globular cluster"))
                childWidget->setStyleSheet("QPushButton { \n     background-position: center bottom; background-repeat: no-repeat;background-origin: content; background-image: url(':/graphicGui/gui/globalcluster1.png');   text-align:top;"
                                           "border-style: inset;\n     border-width: 1px;\n     border-radius:5px;\n	 padding: 3px;\n     border-style: solid;\n     border-width: 1px;\n     border-radius:5px;\n     border-color: palette(button);\n	 padding: 3px;\n     font-size: 12pt;\n}\n\nQPushButton:checked { \n     border-color: palette(dark);\n    background-color: qradialgradient(cx: 0.7, cy: 1.4, fx: 0.7, fy: 1.9, radius: 1.5, stop: 0 rgb(255, 255, 0), stop: 1 transparent );\n}\n\nQPushButton:pressed { \n     border-color: palette(dark);\n     background-color: qradialgradient(cx: 0.7, cy: 1.4, fx: 0.7, fy: 1.9, radius: 1.5, stop: 0 palette(dark), stop: 1 transparent );\n}\n\n");
            else if ( n->getTypeString() == q_("Nebula") )
                childWidget->setStyleSheet("QPushButton { \n     background-position: center bottom; background-repeat: no-repeat;background-origin: content; background-image: url(':/graphicGui/gui/nebula.png');   text-align:top;"
                                           "border-style: inset;\n     border-width: 1px;\n     border-radius:5px;\n	 padding: 3px;\n     border-style: solid;\n     border-width: 1px;\n     border-radius:5px;\n     border-color: palette(button);\n	 padding: 3px;\n     font-size: 12pt;\n}\n\nQPushButton:checked { \n     border-color: palette(dark);\n    background-color: qradialgradient(cx: 0.7, cy: 1.4, fx: 0.7, fy: 1.9, radius: 1.5, stop: 0 rgb(255, 255, 0), stop: 1 transparent );\n}\n\nQPushButton:pressed { \n     border-color: palette(dark);\n     background-color: qradialgradient(cx: 0.7, cy: 1.4, fx: 0.7, fy: 1.9, radius: 1.5, stop: 0 palette(dark), stop: 1 transparent );\n}\n\n");
            else if ( n->getTypeString() == q_("Planetary nebula"))
                childWidget->setStyleSheet("QPushButton { \n     background-position: center bottom; background-repeat: no-repeat;background-origin: content; background-image: url(':/graphicGui/gui/plnebula.png');   text-align:top;"
                                           "border-style: inset;\n     border-width: 1px;\n     border-radius:5px;\n	 padding: 3px;\n     border-style: solid;\n     border-width: 1px;\n     border-radius:5px;\n     border-color: palette(button);\n	 padding: 3px;\n     font-size: 12pt;\n}\n\nQPushButton:checked { \n     border-color: palette(dark);\n    background-color: qradialgradient(cx: 0.7, cy: 1.4, fx: 0.7, fy: 1.9, radius: 1.5, stop: 0 rgb(255, 255, 0), stop: 1 transparent );\n}\n\nQPushButton:pressed { \n     border-color: palette(dark);\n     background-color: qradialgradient(cx: 0.7, cy: 1.4, fx: 0.7, fy: 1.9, radius: 1.5, stop: 0 palette(dark), stop: 1 transparent );\n}\n\n");
            else if ( n->getTypeString() == q_("Cluster associated with nebulosity"))
                childWidget->setStyleSheet("QPushButton { \n     background-position: center bottom; background-repeat: no-repeat;background-origin: content; background-image: url(':/graphicGui/gui/plclnebula.png');   text-align:top;"
                                           "border-style: inset;\n     border-width: 1px;\n     border-radius:5px;\n	 padding: 3px;\n     border-style: solid;\n     border-width: 1px;\n     border-radius:5px;\n     border-color: palette(button);\n	 padding: 3px;\n     font-size: 12pt;\n}\n\nQPushButton:checked { \n     border-color: palette(dark);\n    background-color: qradialgradient(cx: 0.7, cy: 1.4, fx: 0.7, fy: 1.9, radius: 1.5, stop: 0 rgb(255, 255, 0), stop: 1 transparent );\n}\n\nQPushButton:pressed { \n     border-color: palette(dark);\n     background-color: qradialgradient(cx: 0.7, cy: 1.4, fx: 0.7, fy: 1.9, radius: 1.5, stop: 0 palette(dark), stop: 1 transparent );\n}\n\n");
        }
        else
            childWidget->setStyleSheet("QPushButton { \n"
                                       "border-style: inset;\n     border-width: 1px;\n     border-radius:5px;\n	 padding: 3px;\n     border-style: solid;\n     border-width: 1px;\n     border-radius:5px;\n     border-color: palette(button);\n	 padding: 3px;\n     font-size: 12pt;\n}\n\nQPushButton:checked { \n     border-color: palette(dark);\n    background-color: qradialgradient(cx: 0.7, cy: 1.4, fx: 0.7, fy: 1.9, radius: 1.5, stop: 0 rgb(255, 255, 0), stop: 1 transparent );\n}\n\nQPushButton:pressed { \n     border-color: palette(dark);\n     background-color: qradialgradient(cx: 0.7, cy: 1.4, fx: 0.7, fy: 1.9, radius: 1.5, stop: 0 palette(dark), stop: 1 transparent );\n}\n\n");

        connect(childWidget,SIGNAL(toggled(bool)),this,SLOT(on_btnObject_clicked(bool)));
        flowLayout->addWidget(childWidget);
    }
    ui->scrollAreaWidgetContents->setLayout(flowLayout);

}

QImage messierWindow::decryptImage(QString filename)
{
    cBinExt* m_BinExt = new cBinExt;

    m_BinExt->SetBinPath(filename.toStdString() );
    m_BinExt->DoBufferFile();

    m_BinExt->IsAttachmentPresent();
    QPixmap img = m_BinExt->ExtractImage(filename.toLocal8Bit().constData(),
                                         QString("").toStdString());

    if(m_BinExt != NULL)
    {
        delete m_BinExt;
        m_BinExt = NULL;
    }
    return img.toImage();

}


QPushButton *messierWindow::getButtonFromName(const QString &name)
{
    QPushButton *button = ui->scrollAreaWidgetContents->findChild<QPushButton *>(name);
    return button;
}


void messierWindow::on_btnObject_clicked(bool toggled)
{
    StelPresentMgr* sPmgr = GETSTELMODULE(StelPresentMgr);
    QPushButton* button = qobject_cast<QPushButton*>(sender());
    QString messier = button->accessibleName();
    if (messier == "Pleiades")
        messier ="M45";

    if (toggled)
    {
        if (ui->chMove->isChecked())
        {
            StelObjectMgr* objectMgr = GETSTELMODULE(StelObjectMgr);
            objectMgr->findAndSelectI18n(button->accessibleName());
            const QList<StelObjectP> newSelected = objectMgr->getSelectedObject();
            if (!newSelected.empty())
            {
                StelMovementMgr* mvmgr = GETSTELMODULE(StelMovementMgr);
                mvmgr->moveToObject(newSelected[0], mvmgr->getAutoMoveDuration());

                //---
                const Vec3d& current = newSelected[0]->getAltAzPos(StelApp::getInstance().getCore()->getNavigator());
                double alt, azi;
                StelUtils::rectToSphe(&azi, &alt, current);
                alt = (alt)*180/M_PI; // convert to degrees from radians
                azi = std::fmod((((azi)*180/M_PI)*-1)+180., 360.);

                Sleep(100);

                StelApp::getInstance().addNetworkCommand(
                            "core.selectObjectByName(\""+button->accessibleName()+"\",true);\n"+
                            "core.moveToAltAzi("+QString("%0").arg(alt)+","+QString("%0").arg(azi)+",2);");
                //--
                Sleep(100);
            }
        }

        QString strFile = StelFileMgr::getInstallationDir()+"/catalogs/messiers/"+messier;
        QImage img = decryptImage(strFile);

        messierimage* miLast = StelApp::getInstance().addMessierObject(messier,
                                                                        img.width(),
                                                                        img.height(),
                                                                        button->objectName());
        if(miLast)
        {
            QPushButton* btn = getButtonFromName(miLast->btnName);
            if (btn) btn->setChecked(false);
        }

        //Client tarafý
        StelApp::getInstance().getRsync()->sendChanges(RSYNC_COMMAND_MESS_ADD,
                                                       QString("%0@%1@%2@%3")
                                                       .arg(messier)
                                                       .arg(img.width())
                                                       .arg(img.height())
                                                       .arg(button->objectName()));
        img = QImage();
    }
    else
    {
        StelApp::getInstance().removeMessierObject(messier);
        //Client tarafý
        StelApp::getInstance().getRsync()->sendChanges(RSYNC_COMMAND_MESS_REMOVE,
                                                       QString("%0").arg(messier));


    }


}

void messierWindow::on_chHideLand_toggled(bool checked)
{
    LandscapeMgr* lmgr = GETSTELMODULE(LandscapeMgr);
    lmgr->setFlagLandscape(!checked);
    Sleep(200);
    lmgr->setFlagAtmosphere(!checked);
    Sleep(200);
    lmgr->setFlagFog(!checked);
}

void messierWindow::on_btnHideAll_clicked()
{
    StelPresentMgr* sPmgr = GETSTELMODULE(StelPresentMgr);
    foreach(messierimagePtr m, StelApp::getInstance().messierList)
    {
        sPmgr->removeWithFade(m.data()->id);
        QPushButton* btn = getButtonFromName(m.data()->btnName);
        if (btn) btn->setChecked(false);
    }
    StelApp::getInstance().messierList.clear();

    //Client tarafý
    StelApp::getInstance().getRsync()->sendChanges(RSYNC_COMMAND_MESS_REMOVEALL,"");

}
