#ifndef FORMTEST_H
#define FORMTEST_H

#include <QWidget>
#include <QTableWidgetItem>
#include <QAbstractButton>

namespace Ui {
class projSettingDialog;
}

class projSettingDialog : public QWidget
{
    Q_OBJECT

public:
    explicit projSettingDialog(QWidget *parent = 0);
    ~projSettingDialog();

private:
    Ui::projSettingDialog *ui;
    void updateChannelGUI();
    void saveProjSettings();
    void applySettings();

private slots:
    void on_addButton_clicked();
    void on_removeButton_clicked();
    void on_channelTable_itemChanged(QTableWidgetItem* pItem);
    void on_channelTable_currentItemChanged(QTableWidgetItem*current, QTableWidgetItem*previous);
    void on_buttonBox_clicked(QAbstractButton* button);
    void on_btnSelect1_clicked();
    void on_btnSelect2_clicked();
    void on_geometry_changed();
};

#endif // FORMTEST_H
