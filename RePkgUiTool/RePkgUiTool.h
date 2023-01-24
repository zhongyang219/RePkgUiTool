#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_RePkgUiTool.h"
#include "ExtractPkgThread.h"
#include <QProgressBar>

class RePkgUiTool : public QMainWindow
{
    Q_OBJECT

public:
    RePkgUiTool(QWidget *parent = nullptr);
    ~RePkgUiTool();

private:
    QString GetDestFilePath(const QString& strWallpaperId, const QString strFileName);
    void EnableControl(bool enable);
    void SetProgressbarVisible(bool visible);

private slots:
    void OnStartExtrackClicked();
    void OnBrowseSearchFolder();
    void OnBrowseOutputFolder();
    void OnExtractThreadFinished();
    void OnExtractThreadProceed(const QString& curPkgPath);
    void OnStopBtnClicked();
    void OnAboutBtnClicked();

private:
    Ui::RePkgUiToolClass ui;
    QString m_strExtractTempDir;
    CExtractPkgThread m_extractPkgThread;

    //×´Ì¬À¸¿Ø¼þ
    //QLabel* m_pStatusbarInfoLable;
    QProgressBar* m_pProgressBar;
};
