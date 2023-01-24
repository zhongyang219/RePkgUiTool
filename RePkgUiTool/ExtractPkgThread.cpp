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
    //删除临时目录中的所有文件
    QDir qDir(m_inputPara.strExtractTempDir);
    qDir.removeRecursively();

    //开始提取
    QString strCmd = QString("\"%1/RePKG.exe\" extract \"%2\" -o \"%3\"").arg(qApp->applicationDirPath()).arg(strPkgPath).arg(m_inputPara.strExtractTempDir);
    QProcess process;
    process.start(strCmd);
    process.waitForFinished();
}

void CExtractPkgThread::DisposeExtractResult(const QString& strWallpaperId)
{
    if (!QFileInfo(m_inputPara.strExtractTempDir).isDir())
        return;

    //从临时目录中查找提取的图片
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

            //如果目标文件夹不存在，则创建
            QDir dir(QFileInfo(strDestImgPath).dir());
            if (!dir.exists())
                dir.mkdir(dir.path());

            //将提取到的图片文件复制到输出目录
            if (!m_inputPara.skipExistFile)
                QFile::remove(strDestImgPath);
            QFile::copy(strImgPath, strDestImgPath);
        }
    }
}

void CExtractPkgThread::run()
{
    qDebug() << u8"RePkgUiTool: extract started.";

    //搜索pkg文件
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

        //壁纸的id
        QString strWallpaperId = iter.fileInfo().dir().dirName();

        bool skip{ false };
        if (m_inputPara.skipExistFile)        //当勾选“跳过存在的文件时”时
        {
            if (m_inputPara.createFolder)       //为每个壁纸创建单独的文件夹
            {
                //如果输出目录下存在以壁纸id命名的文件夹，则跳过
                QString strOutDirPath = QString("%1/%2").arg(m_inputPara.strOutputDir).arg(strWallpaperId);
                if (QFileInfo(strOutDirPath).isDir() && !QFileInfo(strOutDirPath).dir().isEmpty())
                {
                    skip = true;
                    qDebug() << QString(u8"RePkgUiTool: CreateFolder checked, folder %1 exist, skipped.").arg(strOutDirPath);
                }
            }
            else
            {
                //如果输出目录下存在由壁纸id开头的文件，则跳过
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
            //提取pkg文件
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
