#include "ExtractPkgThread.h"
#include <QApplication>
#include <QProcess>
#include <QFileInfo>
#include <QDir>
#include <QDirIterator>
#include <QDebug>

CExtractPkgThread::CExtractPkgThread(QObject *parent)
    : QThread(parent)
{
}

CExtractPkgThread::~CExtractPkgThread()
{
}

void CExtractPkgThread::Stop()
{
    m_stoped = true;
}

bool CExtractPkgThread::IsStopped()
{
    return m_stoped;
}

void CExtractPkgThread::SetThreadPara(const CExtractPkgThread::InputThreadPara& para)
{
    m_inputPara = para;
}

const CExtractPkgThread::OutputThreadPara& CExtractPkgThread::GetThreadPara() const
{
    return m_outputPara;
}

void CExtractPkgThread::ExtractPkg(const QString& strPkgPath)
{
    //ɾ����ʱĿ¼�е������ļ�
    QDir qDir(m_inputPara.strExtractTempDir);
    qDir.removeRecursively();

    //��ʼ��ȡ
    QString strCmd = QString("\"%1/RePKG.exe\" extract \"%2\" -o \"%3\"").arg(qApp->applicationDirPath()).arg(strPkgPath).arg(m_inputPara.strExtractTempDir);
    QProcess process;
    process.start(strCmd);
    process.waitForFinished();
}

void CExtractPkgThread::DisposeExtractResult(const QString& strWallpaperId)
{
    if (!QFileInfo(m_inputPara.strExtractTempDir).isDir())
        return;

    //����ʱĿ¼�в�����ȡ��ͼƬ
    QDirIterator iter(m_inputPara.strExtractTempDir + "/materials", { "*.jpg", "*.png" }, QDir::Files);
    while (iter.hasNext())
    {
        iter.next();
        if (iter.fileInfo().isFile())
        {
            QString strImgPath = iter.filePath();
            QString strFileName = QFileInfo(strImgPath).fileName();
            QString strDestImgPath;
            if (m_inputPara.createFolder)
                strDestImgPath = QString("%1/%2/%3").arg(m_inputPara.strOutputDir).arg(strWallpaperId).arg(strFileName);
            else
                strDestImgPath = QString("%1/%2_%3").arg(m_inputPara.strOutputDir).arg(strWallpaperId).arg(strFileName);

            //���Ŀ���ļ��в����ڣ��򴴽�
            QDir dir(QFileInfo(strDestImgPath).dir());
            if (!dir.exists())
                dir.mkdir(dir.path());

            //����ȡ����ͼƬ�ļ����Ƶ����Ŀ¼
            if (!m_inputPara.skipExistFile)
                QFile::remove(strDestImgPath);
            QFile::copy(strImgPath, strDestImgPath);
        }
    }
}

void CExtractPkgThread::run()
{
    qDebug() << u8"RePkgUiTool: extract started.";

    //����pkg�ļ�
    QDir dir(m_inputPara.strSearchDir);
    dir.setNameFilters(QStringList() << "*.pkg");
    QDirIterator iter(dir, QDirIterator::Subdirectories);
    QStringList pkgPathList;
    while (iter.hasNext())
    {
        iter.next();
        if (iter.fileInfo().isFile())
        {
            pkgPathList.push_back(iter.filePath());

        }
    }
    m_outputPara.preceedItemCount = 0;
    m_outputPara.totalItemCount = pkgPathList.size();
    
    for (const auto& strPkgPath : pkgPathList)
    {
        if (m_stoped)
            return;

        //��ֽ��id
        QString strWallpaperId = iter.fileInfo().dir().dirName();

        bool skip{ false };
        if (m_inputPara.skipExistFile)        //����ѡ���������ڵ��ļ�ʱ��ʱ
        {
            if (m_inputPara.createFolder)       //Ϊÿ����ֽ�����������ļ���
            {
                //������Ŀ¼�´����Ա�ֽid�������ļ��У�������
                QString strOutDirPath = QString("%1/%2").arg(m_inputPara.strOutputDir).arg(strWallpaperId);
                if (QFileInfo(strOutDirPath).isDir() && !QFileInfo(strOutDirPath).dir().isEmpty())
                {
                    skip = true;
                    qDebug() << QString(u8"RePkgUiTool: CreateFolder checked, folder %1 exist, skipped.").arg(strOutDirPath);
                }
            }
            else
            {
                //������Ŀ¼�´����ɱ�ֽid��ͷ���ļ���������
                QString strOutFilePath = QString("%1/%2*.*").arg(m_inputPara.strOutputDir).arg(strWallpaperId);
                if (QFileInfo(strOutFilePath).exists())
                {
                    skip = true;
                    qDebug() << QString(u8"RePkgUiTool: CreateFolder unchecked, file %1 exist, skipped.").arg(strOutFilePath);
                }
            }
        }

        if (!skip)
        {
            emit signalExtractPkgFile(strPkgPath);
            //��ȡpkg�ļ�
            ExtractPkg(strPkgPath);
            DisposeExtractResult(strWallpaperId);
            m_outputPara.preceedItemCount++;
        }
        else
        {
            m_outputPara.skippedItemCount++;
        }

    }
}
