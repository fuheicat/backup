#ifndef CHECK_H
#define CHECK_H
#include <QString>
#include <QVector>
#include <QFileInfo>
#include <QDirIterator>
#include <QDebug>
#include <QDir>
#include <QCryptographicHash>

class Check {
    static QByteArray getMD5ByFilename(QString filename) {
        QFile file(filename);
        file.open(QIODevice::ReadOnly);
        QByteArray md5 = QCryptographicHash::hash(file.readAll(), QCryptographicHash::Md5);
        file.close();
        return md5.toHex();
    }
  public:
    static QVector<QPair<QString, int>> check(QList<QString> files, QString directory) {
        QVector<QPair<QString, int>> result;
        if (files.empty()) return result;
        auto root = QFileInfo(files[0]).path();
        for (const auto& file : files) {
            if (!QFileInfo(file).exists()) {
                result.push_back(QPair<QString, int>(file, 0));
                continue;
            }
            if (QFileInfo(file).isDir()) {
                QDirIterator iter(file, QDirIterator::Subdirectories);
                while (iter.hasNext()) {
                    iter.next();
                    QFileInfo info = iter.fileInfo();
                    if (info.fileName() == "." || info.fileName() == "..") continue;
                    auto relativePath = info.absoluteFilePath().replace(root, "");
                    auto path = directory + "/" + relativePath;
                    if (!QFileInfo(path).exists()) {
                        result.push_back(QPair<QString, int>(info.absoluteFilePath(), 2));
                    } else if (info.isFile()) {
                        if (getMD5ByFilename(info.absoluteFilePath()) != getMD5ByFilename(path)) {
                            result.push_back(QPair<QString, int>(info.absoluteFilePath(), 1));
                        }
                    }
                }
            } else {
                auto relativePath = QString(file).replace(root, "");
                auto path = directory + "/" + relativePath;
                if (!QFileInfo(path).exists()) {
                    result.push_back(QPair<QString, int>(file, 2));
                } else if (getMD5ByFilename(file) != getMD5ByFilename(path)) {
                    result.push_back(QPair<QString, int>(file, 1));
                }
            }
        }
        return result;
    }
};

#endif // CHECK_H
