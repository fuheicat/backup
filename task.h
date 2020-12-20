#ifndef TASK_H
#define TASK_H
#include <QWidget>
#include <QList>
#include <QDateTime>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QFileInfo>

struct Task {
    Task(QList<QString> _files, QString _backupFilename, int _frequency, QString _password, bool _cloud, QDateTime _nextTime) :
        files(_files), backupFilename(_backupFilename), frequency(_frequency),
        password(_password), cloud(_cloud), nextTime(_nextTime) {}

    QList<QString> files;
    QString backupFilename;
    int frequency;
    QString password;
    bool cloud;
    QDateTime nextTime;
    bool operator == (const Task& t) const {
        return files == t.files && backupFilename == t.backupFilename
               && frequency == t.frequency && password == t.password
               && cloud == t.cloud && nextTime == t.nextTime;
    }
};

class TaskManager {
public:
    TaskManager();
    void init();
    void addTask(Task task);
    void removeTask(int index);
    void clear();
    void updateTime(int index, QDateTime nextTime);
    const QList<Task>& getTaskList();
    void writeJson();
private:
    QList<Task> taskList;
    QJsonDocument config;
};

#endif // TASK_H
