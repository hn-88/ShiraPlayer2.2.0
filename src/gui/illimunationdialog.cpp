#include "illimunationdialog.h"
#include "ui_illimunationdialog.h"
#include "StelApp.hpp"
#include "StelCore.hpp"
#include "StelTranslator.hpp"

illimunationDialog::illimunationDialog()
{
    ui = new Ui_illimunationDialog;
}

illimunationDialog::~illimunationDialog()
{
    delete ui;
}

void illimunationDialog::languageChanged()
{
    if (dialog)
    {
        ui->label->setText(q_("<html><head/><body><p align=\"center\"><span style=\" font-size:12pt; color:#ff0000;\">Click to illuminate the dome!</span></p></body></html>"));
    }
}

void illimunationDialog::createDialogContent()
{
    ui->setupUi(dialog);
    connect(ui->btnBlue,SIGNAL(clicked()),this,SLOT(on_btnColors_click()));
    connect(ui->btnGreen,SIGNAL(clicked()),this,SLOT(on_btnColors_click()));
    connect(ui->btnRed,SIGNAL(clicked()),this,SLOT(on_btnColors_click()));
    connect(ui->btnWhite,SIGNAL(clicked()),this,SLOT(on_btnColors_click()));

}

void illimunationDialog::prepareAsChild()
{}

void illimunationDialog::retranslate()
{
    languageChanged();
}

void illimunationDialog::on_btnColors_click()
{
    QPushButton* button = qobject_cast<QPushButton*>(sender());
    setButtonChecks(button);

    if (button->isChecked())
    {
        if (button == ui->btnRed)
            StelApp::getInstance().getCore()->allFaderColor.set(1,0,0);
        else if (button == ui->btnGreen)
            StelApp::getInstance().getCore()->allFaderColor.set(0,1,0);
        else if (button == ui->btnBlue)
            StelApp::getInstance().getCore()->allFaderColor.set(0,0,1);
        else if (button == ui->btnWhite)
            StelApp::getInstance().getCore()->allFaderColor.set(1,1,1);

        StelApp::getInstance().getCore()->ALLFader.setDuration(2000);
        StelApp::getInstance().getCore()->ALLFader = true;
    }
    else
    {
        StelApp::getInstance().getCore()->ALLFader.setDuration(2000);
        StelApp::getInstance().getCore()->ALLFader = false;
    }

    StelApp::getInstance().getRsync()->sendChanges(RSYNC_COMMAND_ILLUM,
                                                   QString("%1@%2")
                                                   .arg(button->isChecked())
                                                   .arg(button->accessibleName()));

}

void illimunationDialog::setButtonChecks(QPushButton *sender)
{
    if (sender != ui->btnBlue)
    ui->btnBlue->setChecked(false);
    if (sender != ui->btnGreen)
    ui->btnGreen->setChecked(false);
    if (sender != ui->btnRed)
    ui->btnRed->setChecked(false);
    if (sender != ui->btnWhite)
    ui->btnWhite->setChecked(false);
}
