#ifndef UNPACK_H
#define UNPACK_H
#include <QDirIterator>
#include <QFileInfo>
#include <QDebug>

class Unpack {
  public:
    static int unpack(QString tarFilename, QString destination) {
        QFile tar(tarFilename);
        tar.open(QFile::ReadOnly);

        int pathLength;
        int dir;
        int fileLength;
        QString relativePath;
        while (tar.read((char*)&pathLength, sizeof (pathLength))) {
            char* _relativePath = new char[pathLength + 1];
            tar.read(_relativePath, pathLength);
            _relativePath[pathLength] = '\0';
            relativePath = QString::fromStdString(std::string(_relativePath));
            delete[] _relativePath;
            qDebug() << relativePath;
            tar.read((char*)&dir, sizeof (dir));
            tar.read((char*)&fileLength, sizeof (fileLength));
            qDebug() << fileLength;
            if (!dir) {
                QFile data(destination + "/" + relativePath);
                data.open(QFile::WriteOnly);
                if (fileLength) {
                    char* content = new char[fileLength];
                    tar.read(content, fileLength);
                    data.write(content);
                    delete[] content;
                } else {
                    data.write("");
                }
                data.close();
            } else {
                QDir dir;
                dir.mkdir(destination + "/" + relativePath);
            }
        }
        return 1;
    }
};

#endif // UNPACK_H
