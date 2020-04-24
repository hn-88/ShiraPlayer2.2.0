#include <QPushButton>
#include <QComboBox>
#include "constellationwindow.h"
#include "ui_constellationwindow.h"
#include "ConstellationMgr.hpp"
#include "Constellation.hpp"
#include "StelModuleMgr.hpp"
#include "StelTranslator.hpp"
#include "StelObject.hpp"
#include "StelApp.hpp"
#include <QColorDialog>


ConstellationWindow::ConstellationWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ConstellationWindow)
{
    ui->setupUi(this);
    LoadConstLists();
    LoadButtonObjects();

}

ConstellationWindow::~ConstellationWindow()
{
    delete ui;
}

void ConstellationWindow::languageChanged()
{
    //ui->retranslateUi();

}

QPushButton *ConstellationWindow::getButtonFromName(const QString &name)
{
    QPushButton *button = ui->scrollRight->findChild<QPushButton *>(name);
    return button;
}

void ConstellationWindow::uncheckSingleConsts()
{
    for(int i = 0; i < leftList.count(); i++)
    {
        QString nameC = QString("btnC_%0").arg(leftList.at(i));
        QPushButton *button = ui->scrollLeft->findChild<QPushButton *>(nameC);
        if (button)
        {
            button->blockSignals(true);
            button->setChecked(false);
            button->blockSignals(false);
        }
    }
    for(int i = 0; i < middleList.count(); i++)
    {
        QString nameC = QString("btnC_%0").arg(middleList.at(i));
        QPushButton *button = ui->scrollMiddle->findChild<QPushButton *>(nameC);
        if (button)
        {
            button->blockSignals(true);
            button->setChecked(false);
            button->blockSignals(false);
        }
    }
}

void ConstellationWindow::uncheckMultiConsts(QPushButton* btn)
{
    ConstellationMgr * cmgr = GETSTELMODULE(ConstellationMgr);
    for(int i = 0; i < rightList.count(); i++)
    {
        if ((i != 4) && (i != 7))
        {
            QString nameC = QString("btnC_Cmd%0").arg(i);
            QPushButton *button = getButtonFromName(nameC);
            if (button)
            {
                if (btn != button)
                {
                    button->blockSignals(true);
                    button->setChecked(false);
                    button->blockSignals(false);
                }
            }
        }
    }
}

bool ConstellationWindow::IsExistMultiCheck()
{
    for(int i = 0; i < rightList.count(); i++)
    {
        if ((i != 4) && (i != 7))
        {
            QString nameC = QString("btnC_Cmd%0").arg(i);
            QPushButton *button = getButtonFromName(nameC);
            if (button)
            {
                if (button->isChecked())
                    return true;
            }
        }
    }
    return false;
}

QString ConstellationWindow::colToConf(Vec3f c)
{
    return QString("%1,%2,%3").arg(c[0],2,'f',2).arg(c[1],2,'f',2).arg(c[2],2,'f',2);
}

void ConstellationWindow::setBtnBackround(QPushButton *btn, QColor col)
{
    btn->setStyleSheet(QString("background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:0, stop:0 rgba(%0, %1, %2, 255), stop:1 rgba(%0, %1, %2, 255));")
                       .arg(col.red())
                       .arg(col.green())
                       .arg(col.blue()));
}

void ConstellationWindow::clickButton(bool val)
{
    StelApp::getInstance().usingConstModule = true;
    ConstellationMgr * cmgr = GETSTELMODULE(ConstellationMgr);
    cmgr->setFlagIsolateSelected(true);
    QPushButton* button = qobject_cast<QPushButton*>(sender());
    //QMessageBox::information(0,0,"Uyari",button->objectName(),0,0);
    QString nameArt = button->objectName();
    nameArt = nameArt.replace("btnC_","");
    QPushButton* btnAL = getButtonFromName("btnC_Cmd4");
    QPushButton* btnFL = getButtonFromName("btnC_Cmd7");
    if (nameArt.indexOf("Cmd")>=0)
    {
        if (nameArt == "Cmd0") //Spring Const
        {
            uncheckSingleConsts();
            cmgr->clearAllConst();
            cmgr->ShowSpringConst(val,btnAL->isChecked(),btnFL->isChecked());
            uncheckMultiConsts(button);
        }
        else if(nameArt == "Cmd1") //Summer Const
        {
            uncheckSingleConsts();
            cmgr->clearAllConst();
            cmgr->ShowSummerConst(val,btnAL->isChecked(),btnFL->isChecked());
            uncheckMultiConsts(button);
        }
        else if(nameArt == "Cmd2") //Autumn Const
        {
            uncheckSingleConsts();
            cmgr->clearAllConst();
            cmgr->ShowAutumnConst(val,btnAL->isChecked(),btnFL->isChecked());
            uncheckMultiConsts(button);
        }
        else if(nameArt == "Cmd3") //Winter Const
        {
            uncheckSingleConsts();
            cmgr->clearAllConst();
            cmgr->ShowWinterConst(val,btnAL->isChecked(),btnFL->isChecked());
            uncheckMultiConsts(button);
        }
        else if(nameArt == "Cmd4") // Change Art->Line
        {
            if (cmgr->getSelectedCount() == 0) return;
            cmgr->setFlagArt(!val);
            cmgr->setFlagLines(val);
            StelApp::getInstance().getRsync()->sendChanges(RSYNC_COMMAND_CONST_ARTTOLINE,
                                                           QString("%0").arg(val));
        }
        else if(nameArt == "Cmd5") //Ecliptic Const
        {
            uncheckSingleConsts();
            cmgr->clearAllConst();
            cmgr->ShowHideZodiac(val,btnAL->isChecked(),btnFL->isChecked());
            uncheckMultiConsts(button);
        }
        else if(nameArt == "Cmd6") // Show/Hide All
        {
            uncheckSingleConsts();
            cmgr->clearAllConst();
            cmgr->ShowAllConst(val,btnAL->isChecked(),btnFL->isChecked());
            uncheckMultiConsts(button);
        }
        else if(nameArt == "Cmd7") // Show/Hide Const. Labels
        {
            if (cmgr->getSelectedCount() == 0) return;
            cmgr->ShowSelectedLabels(val);
        }

        if(nameArt != "Cmd4")
        {
            StelApp::getInstance().getRsync()->sendChanges(RSYNC_COMMAND_CONST_SELMULTI,
                                                           QString("%0@%1@%2@%3")
                                                           .arg(nameArt)
                                                           .arg(btnAL->isChecked())
                                                           .arg((val))
                                                           .arg(btnFL->isChecked()));
        }
    }
    else
    {
        Constellation* co = dynamic_cast<Constellation*>(cmgr->searchByName(nameArt).data());
        if(co)
        {
            if (IsExistMultiCheck())
                cmgr->clearAllConst();
            QPushButton* btnAL = getButtonFromName("btnC_Cmd4");
            QPushButton* btnFL = getButtonFromName("btnC_Cmd7");
            if(btnAL)
            {
                if(val) cmgr->setSelectedConst(co);
                else cmgr->unsetSelectedConstWithoutAll(co);

                if(btnAL->isChecked())
                    co->setFlagLines(val);
                else
                    co->setFlagArt(val);

                if(btnFL)
                    co->setFlagName(btnFL->isChecked() && (co->getFlagLines()||co->getFlagArt()));

                StelApp::getInstance().getRsync()->sendChanges(RSYNC_COMMAND_CONST_SELSINGLE,
                                                               QString("%0@%1@%2@%3@%4")
                                                               .arg(nameArt)
                                                               .arg(btnAL->isChecked())
                                                               .arg(IsExistMultiCheck())
                                                               .arg((val))
                                                               .arg(btnFL->isChecked()));
            }
            uncheckMultiConsts(button);
        }
    }
}

void ConstellationWindow::spWvalueChanged(int val)
{
    ConstellationMgr * cmgr = GETSTELMODULE(ConstellationMgr);
    cmgr->setLinesWidth(val);

    conf = StelApp::getInstance().getSettings();
    conf->setValue("viewing/flag_constellation_linewidth", val);
    StelApp::getInstance().getRsync()->sendChanges(RSYNC_COMMAND_CONST_LINEWIDTH,
                                                   QString("%0")
                                                   .arg(val));
}

void ConstellationWindow::btnColorClicked()
{
    ConstellationMgr * cmgr = GETSTELMODULE(ConstellationMgr);
    Vec3f cV = cmgr->getLinesColor();
    QColor oldC = QColor::fromRgbF(cV.v[0],cV.v[1],cV.v[2]);
    QColor c = QColorDialog::getColor(oldC, 0);
    if(c.isValid())
    {
        setBtnBackround(btnColor,c);
        cmgr->setLinesColor(Vec3f(c.redF(),c.greenF(),c.blueF()));
        conf = StelApp::getInstance().getSettings();
        conf->setValue("color/const_lines_color", colToConf(Vec3f(c.redF(),c.greenF(),c.blueF())));
        StelApp::getInstance().getRsync()->sendChanges(RSYNC_COMMAND_CONST_LINECOLOR,
                                                       QString("%0@%1@%2")
                                                       .arg(c.redF())
                                                       .arg(c.greenF())
                                                       .arg(c.blueF()));
    }
}

void ConstellationWindow::LoadButtonObjects()
{
    int w = 140; int h = 70;

    flowLayoutLeft = new FlowLayout();
    flowLayoutMiddle = new FlowLayout();
    flowLayoutRight = new FlowLayout();

    ConstellationMgr * cmgr = GETSTELMODULE(ConstellationMgr);

    for(int i = 0; i < leftList.count(); i++)
    {
        Constellation* co = dynamic_cast<Constellation*>(cmgr->searchByName(leftList.at(i)).data());
        QPushButton* childWidget = new QPushButton(co->getNameI18n());
        childWidget->setObjectName(QString("btnC_%0").arg(leftList.at(i)));
        childWidget->setGeometry(0,0,w,h);
        childWidget->setMinimumSize(w,h);
        childWidget->setMaximumSize(w,h);
        childWidget->setCheckable(true);
        childWidget->setStyleSheet("QPushButton { \n     "
                                   "background-position: center bottom; "
                                   "background-repeat: no-repeat;background-origin: content; "
                                   "background-image: url(':/gui/const/"+leftList.at(i)+".png');   "
                                   "text-align:top;"
                                   "border-style: inset;\n     "
                                   "border-width: 1px;\n     "
                                   "border-radius:5px;\n	 "
                                   "padding: 3px;\n     "
                                   "border-style: solid;\n     "
                                   "border-width: 1px;\n     "
                                   "border-radius:5px;\n     "
                                   "border-color: palette(button);\n	 "
                                   "padding: 3px;\n     "
                                   "font-size: 10pt;\n}\n\nQPushButton:checked { \n     border-color: palette(dark);\n    background-color: qradialgradient(cx: 0.7, cy: 1.4, fx: 0.7, fy: 1.9, radius: 1.5, stop: 0 rgb(255, 255, 0), stop: 1 transparent );\n}\n\nQPushButton:pressed { \n     border-color: palette(dark);\n     background-color: qradialgradient(cx: 0.7, cy: 1.4, fx: 0.7, fy: 1.9, radius: 1.5, stop: 0 palette(dark), stop: 1 transparent );\n}\n\n");

        connect(childWidget,SIGNAL(toggled(bool)),this,SLOT(clickButton(bool)));
        flowLayoutLeft->addWidget(childWidget);
    }

    for(int i = 0; i < middleList.count(); i++)
    {
        Constellation* co = dynamic_cast<Constellation*>(cmgr->searchByName(middleList.at(i)).data());
        QPushButton* childWidget = new QPushButton(co->getNameI18n());
        childWidget->setObjectName(QString("btnC_%0").arg(middleList.at(i)));
        childWidget->setGeometry(0,0,w,h);
        childWidget->setMinimumSize(w,h);
        childWidget->setMaximumSize(w,h);
        childWidget->setCheckable(true);
        childWidget->setStyleSheet("QPushButton { \n     "
                                   "background-position: center bottom; "
                                   "background-repeat: no-repeat;background-origin: content; "
                                   "background-image: url(':/gui/const/"+middleList.at(i)+".png');   "
                                   "text-align:top;"
                                   "border-style: inset;\n     "
                                   "border-width: 1px;\n     "
                                   "border-radius:5px;\n	 "
                                   "padding: 3px;\n     "
                                   "border-style: solid;\n     "
                                   "border-width: 1px;\n     "
                                   "border-radius:5px;\n     "
                                   "border-color: palette(button);\n	 "
                                   "padding: 3px;\n     "
                                   "font-size: 10pt;\n}\n\nQPushButton:checked { \n     border-color: palette(dark);\n    background-color: qradialgradient(cx: 0.7, cy: 1.4, fx: 0.7, fy: 1.9, radius: 1.5, stop: 0 rgb(255, 255, 0), stop: 1 transparent );\n}\n\nQPushButton:pressed { \n     border-color: palette(dark);\n     background-color: qradialgradient(cx: 0.7, cy: 1.4, fx: 0.7, fy: 1.9, radius: 1.5, stop: 0 palette(dark), stop: 1 transparent );\n}\n\n");
        connect(childWidget,SIGNAL(toggled(bool)),this,SLOT(clickButton(bool)));
        flowLayoutMiddle->addWidget(childWidget);
    }

    for(int i = 0; i < rightList.count(); i++)
    {
        QPushButton* childWidget = new QPushButton(rightList.at(i));
        childWidget->setObjectName(QString("btnC_Cmd%0").arg(i));

        childWidget->setGeometry(0,0,w,h);
        childWidget->setMinimumSize(w,h);
        childWidget->setMaximumSize(w,h);
        childWidget->setCheckable(true);
        childWidget->setStyleSheet("QPushButton { \n"
                                   "border-style: inset;\n     border-width: 1px;\n     border-radius:5px;\n	 padding: 3px;\n     border-style: solid;\n     border-width: 1px;\n     border-radius:5px;\n     border-color: palette(button);\n	 padding: 3px;\n     font-size: 12pt;\n}\n\nQPushButton:checked { \n     border-color: palette(dark);\n    background-color: qradialgradient(cx: 0.7, cy: 1.4, fx: 0.7, fy: 1.9, radius: 1.5, stop: 0 rgb(255, 255, 0), stop: 1 transparent );\n}\n\nQPushButton:pressed { \n     border-color: palette(dark);\n     background-color: qradialgradient(cx: 0.7, cy: 1.4, fx: 0.7, fy: 1.9, radius: 1.5, stop: 0 palette(dark), stop: 1 transparent );\n}\n\n");
        connect(childWidget,SIGNAL(toggled(bool)),this,SLOT(clickButton(bool)));
        flowLayoutRight->addWidget(childWidget);
    }

    //Diðer sað taraf controlleri
    QWidget* frmColor = new QWidget(this);
    QWidget* frmW = new QWidget(this);
    QWidget* frmArtB = new QWidget(this);

    QHBoxLayout* hlayoutArtB = new QHBoxLayout();
    QHBoxLayout* hlayoutW= new QHBoxLayout();
    QHBoxLayout* hlayout= new QHBoxLayout();
    frmArtB->setLayout(hlayoutArtB);
    frmColor->setLayout(hlayout);
    frmW->setLayout(hlayoutW);

    btnColor = new QPushButton(this);
    Vec3f cV = cmgr->getLinesColor();
    QColor oldC = QColor::fromRgbF(cV.v[0],cV.v[1],cV.v[2]);
    setBtnBackround(btnColor,oldC);

    btnColor->setMinimumSize(2*w/3.f,h*3.f/4.f);
    btnColor->setMaximumSize(2*w/3.f,h*3.f/4.f);
    connect(btnColor,SIGNAL(clicked()),this,SLOT(btnColorClicked()));

    QLabel *lblColor = new QLabel(q_("Line Color :"));
    lblColor->setStyleSheet("font-size: 12pt;");
    frmColor->layout()->addWidget(lblColor);
    frmColor->layout()->addWidget(btnColor);

    spW = new QSpinBox(this);
    spW->setMinimum(1);
    spW->setMaximum(4);
    spW->setStyleSheet("font-size: 12pt;");
    spW->setValue(cmgr->getLinesWidth());
    connect(spW,SIGNAL(valueChanged(int)),this,SLOT(spWvalueChanged(int)));


    QLabel *lblW = new QLabel(q_("Line Width :"));
    lblW->setStyleSheet("font-size: 12pt;");

    frmW->layout()->addWidget(lblW);
    frmW->layout()->addWidget(spW);

    spArt = new QDoubleSpinBox(this);
    spArt->setMinimum(0);
    spArt->setMaximum(1);
    spArt->setSingleStep(0.05);
    spArt->setStyleSheet("font-size: 12pt;");
    spArt->setValue(cmgr->getArtIntensity());
    connect(spArt, SIGNAL(valueChanged(double)), cmgr, SLOT(setArtIntensity(double)));


    QLabel *lblArt = new QLabel(q_("Art Brightness :"));
    lblArt->setStyleSheet("font-size: 12pt;");

    frmArtB->layout()->addWidget(lblArt);
    frmArtB->layout()->addWidget(spArt);

    frmColor->setGeometry(0,0,2*w,h);
    frmW->setGeometry(0,0,2*w,h);
    frmArtB->setGeometry(0,0,2*w,h);
    frmColor->setMinimumSize(2*w,h);
    frmColor->setMaximumSize(2*w,h);
    frmW->setMinimumSize(2*w,h);
    frmW->setMaximumSize(2*w,h);
    frmArtB->setMinimumSize(2*w,h);
    frmArtB->setMaximumSize(2*w,h);

    flowLayoutRight->addWidget(frmColor);
    flowLayoutRight->addWidget(frmW);
    flowLayoutRight->addWidget(frmArtB);

    ui->scrollLeft->setLayout(flowLayoutLeft);
    ui->scrollMiddle->setLayout(flowLayoutMiddle);
    ui->scrollRight->setLayout(flowLayoutRight);

}

void ConstellationWindow::LoadConstLists()
{
    leftList<<"BDI"<<"Umi"<<"UMa"<<"BAS"<<"Boo"<<"SMT"<<"Cyg"<<"Lyr"<<"Aql"<<"Sco"<<"SQP"<<"Peg"<<"And"<<"Cas"<<"Cet"<<"WIT"<<"Ori"<<"CMa"<<"CMi"<<"Tau";
    middleList<<"Vir"<<"Crv"<<"Leo"<<"SPT"<<"Cnc"<<"Sgr"<<"Lib"<<"Oph"<<"Her"<<"CrB"<<"Per"<<"Cap"<<"Aqr"<<"Psc"<<"Ari"<<"Aur"<<"Gem"<<"Mon"<<"WID"<<"Lep";
    rightList<<q_("Spring Const.\nAll On")<<q_("Summer Const.\nAll On")<<q_("Autumn Const.\nAll On")<<q_("Winter Const.\nAll On")<<q_("Change\nArt->Line")<<q_("Ecliptical\nConst.")<<q_("All Const.")<<q_("Constellation\nLabels");
}
