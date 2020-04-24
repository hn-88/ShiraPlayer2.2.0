#include <QSettings>

#include "embedaudiowarning.h"
#include "ui_embedaudiowarning.h"
#include "StelTranslator.hpp"

EmbedAudioWarning::EmbedAudioWarning(QWidget *parent, QString filename) :
    QWidget(parent),
    ui(new Ui::EmbedAudioWarning)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags()|Qt::WindowStaysOnTopHint);
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);

    QStringList sList = filename.split(".");
    QString file = sList[0];
    ui->label->setText(QString(q_("Movie file '%0' contains sound. Shira Player can play embed sound.\n"
                               "But to best performance you should put splitted sound file '%1.wav' or\n"
                               "'%1.mp3' in the same folder.\n"
                               "Shira Movie Encoder can split the sound file."))
                                .arg(filename)
                                .arg(file));
    this->setWindowTitle(q_("Shira Player"));
    ui->checkBox->setText(q_("Don't ask again."));
}

EmbedAudioWarning::~EmbedAudioWarning()
{
    delete ui;
}

void EmbedAudioWarning::on_checkBox_toggled(bool checked)
{
    QSettings settings("Sureyyasoft", "ShiraPlayer");
    settings.beginGroup("Others");
    settings.setValue("DontAskAudioWarn",checked);
    settings.endGroup();
}

void EmbedAudioWarning::on_closeStelWindow_clicked()
{
    close();
}
