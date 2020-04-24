#ifndef MESSIERWINDOW_H
#define MESSIERWINDOW_H

#include <QWidget>
#include <QList>
#include <QPushButton>
#include "presenter/flowlayout.h"
#include <catalogpages/messierimage.h>

namespace Ui {
class messierWindow;
}

class messierWindow : public QWidget
{
    Q_OBJECT

public:
    explicit messierWindow(QWidget *parent = 0);
    ~messierWindow();

    void languageChanged();
protected:
    void LoadButtonObjects();
private:
    Ui::messierWindow *ui;
    FlowLayout *flowLayout;
    QImage decryptImage(QString filename);

    QPushButton *getButtonFromName(const QString& name);

private slots:
    void on_btnObject_clicked(bool toggled);
    void on_chHideLand_toggled(bool checked);
    void on_btnHideAll_clicked();
};


#endif // MESSIERWINDOW_H
