#ifndef WARPWINDOW_H
#define WARPWINDOW_H

#include <QMainWindow>

namespace Ui {
    class WarpWindow;
}

class WarpWindow : public QMainWindow {
    Q_OBJECT
public:
    WarpWindow(QWidget *parent = 0);
    ~WarpWindow();
protected:
    void changeEvent(QEvent *e);
    void paintEvent(QPaintEvent *);

private:
    Ui::WarpWindow *ui;

private slots:
    void on_pushButton_clicked();
};

#endif // WARPWINDOW_H
