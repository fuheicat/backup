#include "task.h"
#include <QDebug>

TaskManager::TaskManager() {

}

void TaskManager::init() {
    if (QFileInfo("config.json").exists()) {
        QFile file("config.json");
        file.open(QFile::ReadWrite);
        config = QJsonDocument::fromJson(file.readAll());
        file.close();
        QJsonArray tasks = config.array();
        for (auto task : tasks) {
            auto information = task.toObject();
            QList<QString> files;
            for (auto file : information["files"].toArray()) {
                files.append(file.toString());
            }
            taskList.append(Task(files,
                                 information["backupFilename"].toString(),
                                 information["frequency"].toInt(),
                                 information["password"].toString(),
                                 information["cloud"].toBool(),
                                 QDateTime::fromString(information["nextTime"].toString())));
        }
    } else {
        QFile file("config.json");
        file.open(QFile::WriteOnly);
        file.write("");
        file.close();
    }
}

void TaskManager::writeJson() {
    QFile file("config.json");
    file.open(QFile::WriteOnly);
    QJsonArray tasks;
    for (auto task : taskList) {
        QJsonObject information;
        QJsonArray files;
        for (auto file : task.files) {
            files.append(file);
        }
        information["files"] = files;
        information["backupFilename"] = task.backupFilename;
        information["frequency"] = task.frequency;
        information["password"] = task.password;
        information["cloud"] = task.cloud;
        information["nextTime"] = task.nextTime.toString();
        tasks.append(information);
    }
    file.write(QJsonDocument(tasks).toJson());
    file.close();
}

void TaskManager::addTask(Task task) {
    taskList.append(task);
    writeJson();
}

void TaskManager::removeTask(int index) {
    taskList.removeAt(index);
    writeJson();
}

void TaskManager::clear() {
    taskList.clear();
    QFile file("config.json");
    file.open(QFile::WriteOnly);
    file.write("");
    file.close();
}

void TaskManager::updateTime(int index, QDateTime nextTime) {
    taskList[index].nextTime = nextTime;
}

const QList<Task>& TaskManager::getTaskList() {
    return taskList;
}
