#ifndef SCRIPTWINDOW_H
#define SCRIPTWINDOW_H

#include <QWidget>

namespace Ui {
class scriptwindow;
}

class StelScriptSyntaxHighlighter;
class servermanager;
class scriptwindow : public QWidget
{
    Q_OBJECT

public:
    explicit scriptwindow(QWidget *parent = 0);
    ~scriptwindow();

    servermanager* owner;
    QString textScript;
    QString selectedName;
    void addScriptArea(QString scripttext);
    void languageChanged();
    //! Notify that the application style changed
    void styleChanged();


private slots:
    void on_btnOpenScript_clicked();

    void on_scriptEdit_cursorPositionChanged();

    void on_loadButton_clicked();

    void on_saveButton_clicked();

    void on_clearButton_clicked();

    void on_runButton_clicked();

    void on_pauseButton_clicked(bool checked);

    void on_recordButton_clicked();

    void on_pauseRecButton_clicked();

    void on_stopRecButton_clicked();

    void on_btnFrRecStart_clicked();

    void on_btnFrRecStop_clicked();

    void on_stopButton_clicked();

    void on_pauseButton_clicked();

    void on_btnSelectPath_clicked();

    void on_btnFrRecPause_clicked();

    void on_chDrawFrameumber_toggled(bool checked);

public slots:
    void loadScript();
    void saveScript();
    void appendLogLine(const QString& s);
    void scriptEnded();

private:
    enum ScriptRunState
    {
        SrStopped,
        SrPaused,
        SrRunning
    };
    enum FulldomeSaveState
    {
        FsStopped,
        FsPaused,
        FsSaving
    };

    Ui::scriptwindow *ui;
    StelScriptSyntaxHighlighter* highlighter;

    void setRecordUIStatus();
    void setFulldomeScriptUI();

    ScriptRunState scriptrunstate;
    FulldomeSaveState fulldomesavestate;


};

#endif // SCRIPTWINDOW_H
