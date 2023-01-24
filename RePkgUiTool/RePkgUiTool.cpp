#include "RePkgUiTool.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QDebug>
#include <QSettings>
#include <QApplication>
#include "define.h"

RePkgUiTool::RePkgUiTool(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
    setWindowIcon(QIcon(":/RePkgUiTool/RePkgUiTool.ico"));

    m_strExtractTempDir = QDir::tempPath() + "/RePkgOutput";

    //��ʼ���ؼ�
    ui.lineEditSearchFolder->addAction(ui.actionBrowseSearchFolder, QLineEdit::TrailingPosition);
    ui.lineEditOutputFolder->addAction(ui.actionBrowseOutputFolder, QLineEdit::TrailingPosition);
    //m_pStatusbarInfoLable = new QLabel();
    m_pProgressBar = new QProgressBar();
    m_pProgressBar->setMaximumWidth(DPI(70));
    m_pProgressBar->setMaximumHeight(DPI(16));
    m_pProgressBar->setRange(0, 1000);
    m_pProgressBar->setTextVisible(false);
    ui.statusBar->addPermanentWidget(m_pProgressBar);
    SetProgressbarVisible(false);
    EnableControl(true);

    //��ʼ�źŲ�
    connect(ui.startExtrackBtn, SIGNAL(clicked()), this, SLOT(OnStartExtrackClicked()));
    connect(ui.pushButtonStop, SIGNAL(clicked()), this, SLOT(OnStopBtnClicked()));
    connect(ui.actionBrowseSearchFolder, SIGNAL(triggered()), this, SLOT(OnBrowseSearchFolder()));
    connect(ui.actionBrowseOutputFolder, SIGNAL(triggered()), this, SLOT(OnBrowseOutputFolder()));
    connect(&m_extractPkgThread, SIGNAL(finished()), this, SLOT(OnExtractThreadFinished()));
    connect(&m_extractPkgThread, SIGNAL(signalExtractPkgFile(const QString&)), this, SLOT(OnExtractThreadProceed(const QString&)));
    connect(ui.pushButtonAbout, SIGNAL(clicked()), this, SLOT(OnAboutBtnClicked()));

    //��������
    QSettings settings(qApp->applicationDirPath() + "./config.ini", QSettings::IniFormat);
    ui.lineEditSearchFolder->setText(settings.value("config/SearchFolder").toString());
    ui.lineEditOutputFolder->setText(settings.value("config/OutputFolder").toString());
    ui.checkBoxCreateFolder->setChecked(settings.value("config/CreateFolder").toBool());
    ui.checkBoxSkipExistFile->setChecked(settings.value("config/SkipExistFile").toBool());

    //ExtractPkg("D:/Temp/scene.pkg");
}

RePkgUiTool::~RePkgUiTool()
{
    //��������
    QSettings settings(qApp->applicationDirPath() + "./config.ini", QSettings::IniFormat);
    settings.setValue("config/SearchFolder", ui.lineEditSearchFolder->text());
    settings.setValue("config/OutputFolder", ui.lineEditOutputFolder->text());
    settings.setValue("config/CreateFolder", ui.checkBoxCreateFolder->isChecked());
    settings.setValue("config/SkipExistFile", ui.checkBoxSkipExistFile->isChecked());

    //�����߳�
    if (m_extractPkgThread.isRunning())
    {
        m_extractPkgThread.Stop();
        m_extractPkgThread.wait(10000);
    }
}

QString RePkgUiTool::GetDestFilePath(const QString& strWallpaperId, const QString strFileName)
{
    if (ui.checkBoxCreateFolder->isChecked())
        return QString("%1/%2/%3").arg(ui.lineEditOutputFolder->text()).arg(strWallpaperId).arg(strFileName);
    else
        return QString("%1/%2_%3").arg(ui.lineEditOutputFolder->text()).arg(strWallpaperId).arg(strFileName);
    return QString();
}

void RePkgUiTool::EnableControl(bool enable)
{
    ui.lineEditOutputFolder->setEnabled(enable);
    ui.lineEditSearchFolder->setEnabled(enable);
    ui.actionBrowseOutputFolder->setEnabled(enable);
    ui.actionBrowseSearchFolder->setEnabled(enable);
    ui.checkBoxCreateFolder->setEnabled(enable);
    ui.checkBoxSkipExistFile->setEnabled(enable);
    ui.startExtrackBtn->setEnabled(enable);
    ui.pushButtonStop->setEnabled(!enable);
}

void RePkgUiTool::SetProgressbarVisible(bool visible)
{
    m_pProgressBar->setVisible(visible);
    if (!visible)
        ui.statusBar->showMessage(u8"����");
}

void RePkgUiTool::OnStartExtrackClicked()
{
    //����Ƿ����ִ��
    if (!QFileInfo(ui.lineEditSearchFolder->text()).isDir())
    {
        QMessageBox::warning(this, QString(), u8"����Ŀ¼�����ڣ���ѡ����ȷ������Ŀ¼��");
        return;
    }
    if (!QFileInfo(ui.lineEditOutputFolder->text()).isDir())
    {
        QMessageBox::warning(this, QString(), u8"���Ŀ¼�����ڣ���ѡ����ȷ�����Ŀ¼��");
        return;
    }
    if (!QFileInfo(qApp->applicationDirPath() + "/RePKG.exe").isFile())
    {
        QMessageBox::warning(this, QString(), u8"RePKG.exe��ʧ���޷�ִ����ȡ���ܣ��뽫RePKG.exe�ŵ���������Ŀ¼������һ�Σ�");
        return;
    }

    EnableControl(false);
    CExtractPkgThread::InputThreadPara para;
    para.strSearchDir = ui.lineEditSearchFolder->text();
    para.strOutputDir = ui.lineEditOutputFolder->text();
    para.strExtractTempDir = m_strExtractTempDir;
    para.createFolder = ui.checkBoxCreateFolder->isChecked();
    para.skipExistFile = ui.checkBoxSkipExistFile->isChecked();
    m_extractPkgThread.SetThreadPara(para);
    if (!m_extractPkgThread.isRunning())
    {
        m_extractPkgThread.start();
        SetProgressbarVisible(true);
    }
}

void RePkgUiTool::OnBrowseSearchFolder()
{
    QString str = QFileDialog::getExistingDirectory(this, QString(), QString(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    ui.lineEditSearchFolder->setText(str);
}

void RePkgUiTool::OnBrowseOutputFolder()
{
    QString str = QFileDialog::getExistingDirectory(this, QString(), QString(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    ui.lineEditOutputFolder->setText(str);
}

void RePkgUiTool::OnExtractThreadFinished()
{
    const auto& outputThreadPara{ m_extractPkgThread.GetThreadPara() };
    QMessageBox::information(this, QString(), QString(u8"��ɣ��Ѵ��� %1 ��pkg�ļ������� %2 ��pkg�ļ���").arg(outputThreadPara.preceedItemCount).arg(outputThreadPara.skippedItemCount));
    EnableControl(true);
    SetProgressbarVisible(false);
}

void RePkgUiTool::OnExtractThreadProceed(const QString& curPkgPath)
{
    const auto& threadPara{ m_extractPkgThread.GetThreadPara() };
    int progress{ threadPara.preceedItemCount * 1000 / threadPara.totalItemCount };
    double percent{ static_cast<double>(progress) / 10 };
    QString strStatusbarInfo = QString(u8"������ȡ��%1��(%2/%3)������� %4%").arg(curPkgPath).arg(threadPara.preceedItemCount).arg(threadPara.totalItemCount).arg(percent, 0, 'f', 1);
    ui.statusBar->showMessage(strStatusbarInfo);
    m_pProgressBar->setValue(progress);
}

void RePkgUiTool::OnStopBtnClicked()
{
    if (m_extractPkgThread.isRunning())
    {
        m_extractPkgThread.Stop();
        m_extractPkgThread.wait();
    }
}

void RePkgUiTool::OnAboutBtnClicked()
{
    QString strAboutInfo;
    QMessageBox::about(this, u8"����RePkgUiTool", u8"RePkgUiTool V0.1.0\r\n����һ��RePKG��ͼ�ν����װ��\r\nCopyright(C) 2023 by ZhongYang");
}
