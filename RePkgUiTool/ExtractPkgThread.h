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
    void ExtractPkg(const QString& strPkgPath);     //��pkg�е��ļ���ȡ����ʱĿ¼
    void DisposeExtractResult(const QString& strWallpaperId);                    //����ʱĿ¼�е�ͼƬ���Ƶ����Ŀ¼

signals:
    void signalExtractPkgFile(const QString& fileName);     //��ʼ��ȡһ��pkg�ļ�ʱ������ź�

private:
    bool m_stoped{ false };
    InputThreadPara m_inputPara;
    OutputThreadPara m_outputPara;

protected:
    virtual void run() override;
};
