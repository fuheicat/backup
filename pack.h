#ifndef PACK_H
#define PACK_H
#include <QDirIterator>
#include <QFileInfo>
#include <QDebug>


class Pack {
  public:
    /*
     * destination 打开失败返回1
     * files 中的文件打开失败返回2
     * 正常返回0
     */
    static int pack(QStringList files, QString destination) {
        QFile tar(destination);
        bool success = tar.open(QFile::WriteOnly);
        if (!success) return 1;
        auto root = QFileInfo(files[0]).path();
        for (auto& file : files) {
            if (QFileInfo(file).isFile()) {
                auto relativePath = QString(file).replace(root, "");
                //qDebug() << relativePath.toStdString().c_str() << relativePath.length();
                int pathLength = relativePath.length();
                tar.write((const char*)&pathLength, sizeof (pathLength));
                tar.write(relativePath.toStdString().c_str());
                int fileLength = QFileInfo(file).size();
                int dir = 0;
                tar.write((const char*)&dir, sizeof (dir));
                tar.write((const char*)&fileLength, sizeof (fileLength));
                QFile data(file);
                bool success = data.open(QFile::ReadOnly);
                if (!success) return 2;
                tar.write(data.readAll());
                data.close();
            } else {
                QDirIterator iter(file, QDirIterator::Subdirectories);
                while (iter.hasNext()) {
                    iter.next();
                    QFileInfo info = iter.fileInfo();
                    if (info.fileName() == "..") continue;
                    if (info.isDir()) {
                        auto relativePath = QString(info.absoluteFilePath()).replace(root, "");
                        //qDebug() << relativePath.toStdString().c_str() << relativePath.length();
                        int pathLength = relativePath.length();
                        tar.write((const char*)&pathLength, sizeof (pathLength));
                        tar.write(relativePath.toStdString().c_str());
                        int fileLength = 0;
                        int dir = 1;
                        tar.write((const char*)&dir, sizeof (dir));
                        tar.write((const char*)&fileLength, sizeof (fileLength));
                    } else {
                        auto relativePath = QString(info.absoluteFilePath()).replace(root, "");
                        //qDebug() << relativePath.toStdString().c_str() << relativePath.length();
                        int pathLength = relativePath.length();
                        tar.write((const char*)&pathLength, sizeof (pathLength));
                        tar.write(relativePath.toStdString().c_str());
                        int fileLength = info.size();
                        int dir = 0;
                        tar.write((const char*)&dir, sizeof (dir));
                        tar.write((const char*)&fileLength, sizeof (fileLength));
                        QFile data(info.absoluteFilePath());
                        bool success = data.open(QFile::ReadOnly);
                        if (!success) return 2;
                        tar.write(data.readAll());
                        data.close();
                    }
                }
            }
        }
        tar.close();
        return 0;
    }
};

#endif // PACK_H
