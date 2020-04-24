#ifndef EMBEDAUDIOWARNING_H
#define EMBEDAUDIOWARNING_H

#include <QWidget>

namespace Ui {
class EmbedAudioWarning;
}

class EmbedAudioWarning : public QWidget
{
    Q_OBJECT

public:
    explicit EmbedAudioWarning(QWidget *parent = 0, QString filename = "");
    ~EmbedAudioWarning();

private slots:
    void on_checkBox_toggled(bool checked);

    void on_closeStelWindow_clicked();

private:
    Ui::EmbedAudioWarning *ui;
};

#endif // EMBEDAUDIOWARNING_H
