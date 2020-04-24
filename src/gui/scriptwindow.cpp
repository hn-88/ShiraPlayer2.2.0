#include "scriptwindow.h"
#include "ui_scriptwindow.h"
#include "StelScriptSyntaxHighlighter.hpp"
#include "StelTranslator.hpp"
#include "servermanager.h"
#include "StelScriptMgr.hpp"
#include "scriptrecorder.h"
#include "StelMainWindow.hpp"
#include "StelAppGraphicsWidget.hpp"


#include <QMessageBox>
#include <QFileDialog>

scriptwindow::scriptwindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::scriptwindow)
{
    ui->setupUi(this);

    highlighter = new StelScriptSyntaxHighlighter(ui->scriptEdit->document());

    selectedName = "";

    ui->comboRes->addItem("1024");
    ui->comboRes->addItem("2048");
    ui->comboRes->addItem("3072");
    ui->comboRes->addItem("4096");
    ui->comboRes->setCurrentText(QString("%0").arg(StelApp::getInstance().viewportRes));


    //connect(ui->stopButton, SIGNAL(clicked()), &StelMainGraphicsView::getInstance().getScriptMgr(), SLOT(stopScript()));
    connect(&StelMainGraphicsView::getInstance().getScriptMgr(), SIGNAL(scriptStopped()), this, SLOT(scriptEnded()));
    connect(&StelMainGraphicsView::getInstance().getScriptMgr(), SIGNAL(scriptDebug(const QString&)), this, SLOT(appendLogLine(const QString&)));

    scriptrunstate = ScriptRunState::SrStopped;
    fulldomesavestate = FulldomeSaveState::FsStopped;

    QString dir = StelApp::getInstance().getSettings()->value("main/produce_fulldome_path","C:/FulldomeProduce").toString();
    ui->edtPath->setText(dir);
}

scriptwindow::~scriptwindow()
{
    delete ui;
}

void scriptwindow::addScriptArea(QString scripttext)
{
    ui->scriptEdit->setPlainText(ui->scriptEdit->toPlainText()+"\n"+scripttext);
}

void scriptwindow::languageChanged()
{
    ui->loadButton->setToolTip(q_( "load script from file"));
    ui->loadButton->setShortcut(q_( "Ctrl+O"));
    ui->saveButton->setToolTip(q_( "save script to file"));
    ui->saveButton->setShortcut(q_( "Ctrl+S"));

    ui->clearButton->setToolTip(q_( "clear script"));

    ui->runButton->setToolTip(q_( "run script"));
    ui->runButton->setShortcut(q_( "Ctrl+Return"));

    ui->stopButton->setToolTip(q_( "stop script"));
    ui->stopButton->setShortcut(q_( "Ctrl+C"));

    ui->rowColumnLabel->setToolTip(q_( "Cursor position"));
    ui->rowColumnLabel->setText(q_( "R:0 C:0"));
    ui->label->setText(q_( "Include dir:"));
}

void scriptwindow::styleChanged()
{
    if (highlighter)
    {
        highlighter->setFormats();
        highlighter->rehighlight();
    }
}

void scriptwindow::on_btnOpenScript_clicked()
{
    if(textScript != "")
    {
        if((QMessageBox::information(0,
                                     q_("Information"),
                                     q_("The contents of the selected file will be added to the script area.\nContinue?"),
                                     QMessageBox::Yes,QMessageBox::No))== QMessageBox::No)
        {
            return;
        }
        addScriptArea(textScript);

    }
}

void scriptwindow::on_scriptEdit_cursorPositionChanged()
{
    ui->rowColumnLabel->setText(QString("R:%1 C:%2").arg(ui->scriptEdit->textCursor().blockNumber())
                                .arg(ui->scriptEdit->textCursor().columnNumber()));

}

void scriptwindow::loadScript()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    q_("Load Script"),
                                                    "./favorits",
                                                    q_("Script Files") + " " +"*.fsc");
    QFile file(fileName);
    if (file.open(QIODevice::ReadOnly))
    {
        selectedName = fileName;
        ui->scriptEdit->setPlainText(file.readAll());
        //ui->includeEdit->setText(QFileInfo(fileName).dir().canonicalPath());
        file.close();
    }
}

void scriptwindow::saveScript()
{
    QString saveDir = "./favorits";

    QString fileName = QFileDialog::getSaveFileName(this,
                                                    q_("Save Script"),
                                                    saveDir+"/"+selectedName,
                                                    q_("Script Files") + " " + "*.fsc");
    QFile file(fileName);
    if (file.open(QIODevice::WriteOnly))
    {
        QTextStream out(&file);
        out.setCodec("UTF-8");
        out << ui->scriptEdit->toPlainText();
        file.close();
    }
    else
        qWarning() << "ERROR - cannot write script file";
}

void scriptwindow::appendLogLine(const QString &s)
{
    QString html = ui->outputBrowser->toHtml();
    // if (html!="")
    // 	html += "<br />";

    html += s;
    ui->outputBrowser->setHtml(html);
}

void scriptwindow::scriptEnded()
{
    //qDebug() << "ScriptConsole::scriptEnded";
    //QString html = ui->outputBrowser->toHtml();
    appendLogLine(QString("Script finished at %1").arg(QDateTime::currentDateTime().toString()));
    ui->runButton->setEnabled(true);
    ui->stopButton->setEnabled(false);
    ui->pauseButton->setEnabled(false);
    ui->pauseButton->setChecked(false);
    ui->frameWithFulldome->setEnabled(true);

    setFulldomeScriptUI();

    if(ui->chWithFulldome->isChecked())
    {
        ui->groupFulldome->setEnabled(true);
    }
}

void scriptwindow::setRecordUIStatus()
{
    ScriptRecorder::RecordState state = StelMainGraphicsView::getInstance().getScriptRec().getstatusSc();
    switch (state) {
    case ScriptRecorder::RecordState::Recording:
    case ScriptRecorder::RecordState::Paused:
        ui->recordButton->setEnabled(false);
        ui->pauseRecButton->setEnabled(true);
        ui->stopRecButton->setEnabled(true);
        ui->groupRunBox->setEnabled(false);
        ui->chDelays->setEnabled(false);
        break;
    case ScriptRecorder::RecordState::Stopped:
        ui->recordButton->setEnabled(true);
        ui->pauseRecButton->setEnabled(false);
        ui->pauseRecButton->setChecked(false);
        ui->stopRecButton->setEnabled(false);
        ui->groupRunBox->setEnabled(true);
        ui->chDelays->setEnabled(true);
        break;
    default:
        break;
    }
}

void scriptwindow::setFulldomeScriptUI()
{
    bool status = (scriptrunstate == ScriptRunState::SrStopped) || (fulldomesavestate == FulldomeSaveState::FsStopped);
    ui->groupScriptRecord->setEnabled(status);
    ui->frameFileOp->setEnabled(status);
    ui->grpScriptIO->setEnabled(status);
    ui->btnOpenScript->setEnabled(status);
    owner->flagFavoritsList(status);

}

void scriptwindow::on_loadButton_clicked()
{
    loadScript();
}

void scriptwindow::on_saveButton_clicked()
{
    if(ui->scriptEdit->toPlainText() == "")
        return;
    saveScript();
    owner->createFavoritsList();
}

void scriptwindow::on_clearButton_clicked()
{
    ui->scriptEdit->clear();
    ui->outputBrowser->clear();
    selectedName = "newscript1.fsc";
}

void scriptwindow::on_runButton_clicked()
{
    ui->outputBrowser->setHtml("");
    QTemporaryFile file(QDir::tempPath() + "/stelscriptXXXXXX.fsc");
    QString fileName;
    if (file.open()) {
        QTextStream out(&file);
        out << ui->scriptEdit->toPlainText() << "\n";
        fileName = file.fileName();
        file.close();
    }
    else
    {
        QString msg = "ERROR - cannot open tmp file for writing script text";
        qWarning() << "ScriptWindow::runScript " + msg;
        appendLogLine(msg);
        return;
    }

    ui->runButton->setEnabled(false);
    ui->stopButton->setEnabled(true);
    ui->pauseButton->setEnabled(true);
    ui->frameWithFulldome->setEnabled(false);

    setFulldomeScriptUI();

    if(ui->chWithFulldome->isChecked())
    {
        on_btnFrRecStart_clicked();
        ui->groupFulldome->setEnabled(false);
    }

    appendLogLine(QString("Starting script at %1").arg(QDateTime::currentDateTime().toString()));

    if (!StelMainGraphicsView::getInstance().getScriptMgr()
            .runScriptNetwork(ui->scriptEdit->document()->toPlainText()))
    {
        QString msg = QString("ERROR - cannot run script from temp file: ");
        qWarning() << "ScriptWindow::runScript " + msg;
        appendLogLine(msg);
        if (file.open())
        {
            int n=0;
            while(!file.atEnd())
            {
                appendLogLine(QString("%1:%2").arg(n,2).arg(QString(file.readLine())));
            }
            file.close();
        }
        return;
    }



}

void scriptwindow::on_pauseButton_clicked(bool checked)
{
    if(checked)
        StelMainGraphicsView::getInstance().getScriptMgr().pauseScript();
    else
        StelMainGraphicsView::getInstance().getScriptMgr().resumeScript();
}

void scriptwindow::on_recordButton_clicked()
{
    StelMainGraphicsView::getInstance().getScriptRec().setRecDelayTime(ui->chDelays->isChecked());
    StelMainGraphicsView::getInstance().getScriptRec().start();
    setRecordUIStatus();

    ui->groupFulldome->setEnabled(false);
    ui->grpScriptIO->setEnabled(false);
    ui->btnOpenScript->setEnabled(false);
    ui->frameFileOp->setEnabled(false);
    ui->groupRunBox->setEnabled(false);
}

void scriptwindow::on_pauseRecButton_clicked()
{
    StelMainGraphicsView::getInstance().getScriptRec().pauseToggled();
    setRecordUIStatus();
}

void scriptwindow::on_stopRecButton_clicked()
{
    StelMainGraphicsView::getInstance().getScriptRec().stop();
    QString str = ui->scriptEdit->toPlainText().append(StelMainGraphicsView::getInstance().getScriptRec().getRecScript());
    ui->scriptEdit->setPlainText(str);
    setRecordUIStatus();

    ui->groupFulldome->setEnabled(true);
    ui->grpScriptIO->setEnabled(true);
    ui->btnOpenScript->setEnabled(true);
    ui->frameFileOp->setEnabled(true);
    ui->groupRunBox->setEnabled(true);
}


void scriptwindow::on_btnFrRecStart_clicked()
{    
    QMessageBox msgBox;
    msgBox.setWindowTitle(q_("Warning!"));
    msgBox.setText(q_("Projector screens will be closed\nwhile frame is saving,\nAre you sure?"));
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setWindowModality(Qt::WindowModality());
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.setWindowFlags(Qt::WindowStaysOnTopHint);

    if(msgBox.exec() != QMessageBox::Yes)
        return;

    StelAppGraphicsWidget::getInstance().savePath = ui->edtPath->text();
    StelAppGraphicsWidget::getInstance().saveType = ui->comboType->currentText();
    StelAppGraphicsWidget::getInstance().startSaveFrame(ui->comboRes->currentText().toInt(),true);

    ui->btnFrRecStart->setEnabled(false);
    ui->btnFrRecPause->setEnabled(true);
    ui->btnFrRecStop->setEnabled(true);

    setFulldomeScriptUI();

    ui->frmFrameset1->setEnabled(false);
    ui->frmFrameSet2->setEnabled(false);


}

void scriptwindow::on_btnFrRecStop_clicked()
{
    StelAppGraphicsWidget::getInstance()
            .startSaveFrame(ui->comboRes->currentText().toInt(),false);

    ui->btnFrRecStart->setEnabled(true);
    ui->btnFrRecPause->setEnabled(false);
    ui->btnFrRecPause->setChecked(false);
    ui->btnFrRecStop->setEnabled(false);

    setFulldomeScriptUI();

    ui->frmFrameset1->setEnabled(true);
    ui->frmFrameSet2->setEnabled(true);

}
void scriptwindow::on_btnFrRecPause_clicked()
{
    StelAppGraphicsWidget::getInstance().pauseSaveFrame(ui->btnFrRecPause->isChecked());
}

void scriptwindow::on_stopButton_clicked()
{
    StelMainGraphicsView::getInstance().getScriptMgr().stopScript();
    setFulldomeScriptUI();
    if(ui->chWithFulldome->isChecked())
    {
        on_btnFrRecStop_clicked();
    }

}

void scriptwindow::on_pauseButton_clicked()
{
    if(ui->pauseButton->isChecked())
        StelMainGraphicsView::getInstance().getScriptMgr().pauseScript();
    else
        StelMainGraphicsView::getInstance().getScriptMgr().resumeScript();

    if(ui->chWithFulldome->isChecked())
    {
        on_btnFrRecPause_clicked();
    }

}

void scriptwindow::on_btnSelectPath_clicked()
{


    QString dir = QFileDialog::getExistingDirectory(0, tr("Open Directory"),
                                                    ui->edtPath->text(),
                                                    QFileDialog::ShowDirsOnly
                                                    | QFileDialog::DontResolveSymlinks);
    if(!dir.isEmpty())
    {
        ui->edtPath->setText(dir);
        StelApp::getInstance().getSettings()->setValue("main/produce_fulldome_path",dir);
    }
}

void scriptwindow::on_chDrawFrameumber_toggled(bool checked)
{
    StelAppGraphicsWidget::getInstance().drawFrameNum = checked;
}
