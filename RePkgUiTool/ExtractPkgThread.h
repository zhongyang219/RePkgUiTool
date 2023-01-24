#pragma once

#include <QThread>

class CExtractPkgThread : public QThread
{
    Q_OBJECT

public:
    CExtractPkgThread(QObject *parent = nullptr);
    ~CExtractPkgThread();

    struct InputThreadPara
    {
        QString strSearchDir;
        QString strOutputDir;
        QString strExtractTempDir;
        bool skipExistFile{};
        bool createFolder{};
    };

    struct OutputThreadPara
    {
        int preceedItemCount{};
        int totalItemCount{};
        int skippedItemCount{};
    };

    void Stop();
    bool IsStopped();
    void SetThreadPara(const InputThreadPara& para);
    const OutputThreadPara& GetThreadPara() const;

protected:
    void ExtractPkg(const QString& strPkgPath);     //将pkg中的文件提取到临时目录
    void DisposeExtractResult(const QString& strWallpaperId);                    //将临时目录中的图片复制到输出目录

signals:
    void signalExtractPkgFile(const QString& fileName);     //开始提取一个pkg文件时发射此信号

private:
    bool m_stoped{ false };
    InputThreadPara m_inputPara;
    OutputThreadPara m_outputPara;

protected:
    virtual void run() override;
};
