#include "widget.h"
#include "ui_widget.h"
#include <QDebug>
#include <QFileDialog>
#include <QListView>
#include <QMessageBox>
#include <regex>
#include <QDesktopServices>
#include <QProcess>

Widget::Widget(QWidget* parent) : QWidget(parent), ui(new Ui::Widget) {
    ui->setupUi(this);
    // 使 localGroupBox 可用而 cloudGroupBox不可用
    on_localGroupBox_clicked(true);
    on_cloudGroupBox_clicked(true);
    on_localGroupBox_clicked(true);
    taskManager.init();
    for (const auto& task : taskManager.getTaskList()) {
        QTreeWidgetItem* taskItem = new QTreeWidgetItem;
        taskItem->setText(0, QFileInfo(task.backupFilename).fileName());
        taskItem->setText(1, task.nextTime.toString());
        taskItem->setText(2, task.frequency == 1 ? "每天" : "每周");
        taskItem->setText(3, task.password.trimmed() != "" ? "是" : "否");
        taskItem->setText(4, task.cloud ? "是" : "否");
        taskItem->setText(5, task.backupFilename);
        ui->taskList->addTopLevelItem(taskItem);
    }
    if (!ui->taskList->currentItem() && ui->taskList->topLevelItemCount()) {
        ui->taskList->setCurrentItem(ui->taskList->topLevelItem(0));
    }
    connect(&timer, &QTimer::timeout, this, [ = ]() {
        if (taskManager.getTaskList().count()) {
            for (auto& task : taskManager.getTaskList()) {
                // if (task.nextTime.toString() == QDateTime::currentDateTime().toString()) {
                if (task.nextTime <= QDateTime::currentDateTime()) {
                    // 执行
                    // 更新下一次备份时间
                    int index = taskManager.getTaskList().indexOf(task);
                    while (task.nextTime <= QDateTime::currentDateTime()) {
                        taskManager.updateTime(index, task.nextTime.addDays(task.frequency == 1 ? 1 : 7));
                    }
                    ui->taskList->topLevelItem(index)->setText(1, task.nextTime.toString());
                    taskManager.writeJson();
                }
            }
        }
    });
    timer.start(1000);
}

Widget::~Widget() {
    on_clearFileButton_clicked();
    delete ui;
}


void Widget::on_passwordCheckBox_stateChanged(int arg1) {
    ui->passwordLineEdit->setEnabled(ui->passwordCheckBox->checkState());
}

void Widget::on_browseButton_clicked() {
    QString directory = QFileDialog::getExistingDirectory(
                            this,
                            tr("备份到"),
                            "/home",
                            QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (directory != "") {
        ui->backupFileDictoryLineEdit->setText(directory);
    }
}

void Widget::on_addFileButton_clicked() {
    QStringList files = QFileDialog::getOpenFileNames(
                            this,
                            "选择一个或多个文件",
                            "/home",
                            "所有文件 (*.*)");
    for (const auto& file : files) {
        // 去重
        bool duplication = false;
        for (int i = 0; i < ui->backupFileList->topLevelItemCount(); ++i) {
            if (ui->backupFileList->topLevelItem(i)->text(1) == file) {
                duplication = true;
                break;
            }
        }
        if (duplication) continue;

        QTreeWidgetItem* fileItem = new QTreeWidgetItem;
        fileItem->setText(0, QFileInfo(file).fileName());
        fileItem->setText(1, file);
        ui->backupFileList->addTopLevelItem(fileItem);
    }
    if (!ui->backupFileList->currentItem() && ui->backupFileList->topLevelItemCount()) {
        ui->backupFileList->setCurrentItem(ui->backupFileList->topLevelItem(0));
    }
}

void Widget::on_deleteFileButton_clicked() {
    if (ui->backupFileList->currentItem()) {
        delete ui->backupFileList->currentItem();
    }
}

void Widget::on_clearFileButton_clicked() {
    while (ui->backupFileList->currentItem()) {
        delete ui->backupFileList->currentItem();
    }
}

void Widget::on_addDictoryButton_clicked() {
    QString directory = QFileDialog::getExistingDirectory(
                            this,
                            tr("选择一个文件夹"),
                            "/home",
                            QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (directory != "") {
        // 去重
        for (int i = 0; i < ui->backupFileList->topLevelItemCount(); ++i) {
            if (ui->backupFileList->topLevelItem(i)->text(1) == directory) return;
        }
        QTreeWidgetItem* fileItem = new QTreeWidgetItem;
        fileItem->setText(0, QFileInfo(directory).fileName());
        fileItem->setText(1, directory);
        ui->backupFileList->addTopLevelItem(fileItem);
    }
    if (!ui->backupFileList->currentItem() && ui->backupFileList->topLevelItemCount()) {
        ui->backupFileList->setCurrentItem(ui->backupFileList->topLevelItem(0));
    }
}

void Widget::on_startBackupButton_clicked() {
    if (!ui->backupFileList->topLevelItemCount()) {
        QMessageBox::information(this, "提示", "请添加需要备份的文件。",
                                 QMessageBox::Yes, QMessageBox::Yes);
        return;
    }
    if (ui->backupFilenameLineEdit->text().trimmed() == "") {
        QMessageBox::information(this, "提示", "请输入备份后的文件名。",
                                 QMessageBox::Yes, QMessageBox::Yes);
        return;
    }
    if (ui->backupFileDictoryLineEdit->text().trimmed() == "") {
        QMessageBox::information(this, "提示", "请输入备份保存到的目录。",
                                 QMessageBox::Yes, QMessageBox::Yes);
        return;
    }
    std::regex filenameExpress("[\\/:*?\"<>|]");
    if (std::regex_search(ui->backupFilenameLineEdit->text().toStdString(), filenameExpress)) {
        QMessageBox::information(this, "提示", "请输入合法的备份文件名。",
                                 QMessageBox::Yes, QMessageBox::Yes);
        return;
    }
    if (QFileInfo(ui->backupFileDictoryLineEdit->text() + "\\" + ui->backupFilenameLineEdit->text()).exists()) {
        if (QMessageBox::Yes != QMessageBox::question(this, "警告", "文件已存在，确认覆盖？", QMessageBox::Yes | QMessageBox::No)) {
            return;
        }
    }
    if (ui->passwordCheckBox->isChecked() && ui->passwordLineEdit->text().trimmed() == "") {
        QMessageBox::information(this, "提示", "请输入密码。",
                                 QMessageBox::Yes, QMessageBox::Yes);
        return;
    }
    // 都与第一个文件的目录对比
    auto rootDictory = QFileInfo(ui->backupFileList->topLevelItem(0)->text(1)).path();
    for (int i = 1; i < ui->backupFileList->topLevelItemCount(); ++i) {
        if (QFileInfo(ui->backupFileList->topLevelItem(i)->text(1)).path() != rootDictory) {
            QMessageBox::information(this, "提示", "选择的文件或文件夹应位于同一目录下。",
                                     QMessageBox::Yes, QMessageBox::Yes);
            return;
        }
    }
    if (!ui->noneRadioButton->isChecked()) {
        QTreeWidgetItem* taskItem = new QTreeWidgetItem;
        taskItem->setText(0, ui->backupFilenameLineEdit->text());
        auto nextTime = QDateTime::currentDateTime().addDays(ui->everydayRadioButton->isChecked() ? 1 : 7);
        taskItem->setText(1, nextTime.toString());
        taskItem->setText(2, ui->everydayRadioButton->isChecked() ? "每天" : "每周");
        taskItem->setText(3, ui->passwordCheckBox->isChecked() ? "是" : "否");
        taskItem->setText(4, ui->cloudCheckBox->isChecked() ? "是" : "否");
        taskItem->setText(5, ui->backupFileDictoryLineEdit->text() + ui->backupFilenameLineEdit->text());
        ui->taskList->addTopLevelItem(taskItem);
        QList<QString> files;
        for (int i = 0; i < ui->backupFileList->topLevelItemCount(); ++i) {
            files.append(ui->backupFileList->topLevelItem(i)->text(1));
        }
        taskManager.addTask(Task(files,
                                 ui->backupFileDictoryLineEdit->text() + ui->backupFilenameLineEdit->text(),
                                 ui->everydayRadioButton->isChecked() ? 1 : 2,
                                 ui->passwordLineEdit->text(),
                                 ui->cloudCheckBox->isChecked(),
                                 nextTime));
        if (!ui->taskList->currentItem() && ui->taskList->topLevelItemCount()) {
            ui->taskList->setCurrentItem(ui->taskList->topLevelItem(0));
        }
    }
    // 调用打包压缩加密
    QProcess tar;
    QStringList files;
    for (int i = 0; i < ui->backupFileList->topLevelItemCount(); ++i) {
        files.append("./" + ui->backupFileList->topLevelItem(i)->text(0));
    }
    auto currentDictory = QDir::current();
    QDir::setCurrent(rootDictory);
    tar.start(currentDictory.path() + "/tar.exe", QStringList() << "-cvf" << ui->backupFilenameLineEdit->text() + ".tar" << files);
    tar.waitForStarted(-1);
    tar.waitForFinished(-1);
    Compressor compressor;
    int code = compressor.compress(ui->backupFilenameLineEdit->text().toStdString() + ".tar",
                                   ui->backupFileDictoryLineEdit->text().toStdString(),
                                   ui->passwordCheckBox->isChecked() ? ui->passwordLineEdit->text().toStdString() : "");
    if (code) {
        QMessageBox::information(this, "提示", "压缩失败。",
                                 QMessageBox::Yes, QMessageBox::Yes);
        qDebug() << code;
        return;
    }
    QFile tarFile(ui->backupFilenameLineEdit->text() + ".tar");
    tarFile.remove();
    QDir::setCurrent(currentDictory.path());
    // 校验
    // 云上传
    if (ui->cloudCheckBox->isChecked()) {
        QNetworkAccessManager* manager = new QNetworkAccessManager(this);
        QFile uploadFile(ui->backupFileDictoryLineEdit->text() + ui->backupFilenameLineEdit->text() + ".bak");
        QNetworkRequest request(QUrl(api + "file/" + ui->backupFilenameLineEdit->text() + ".bak"));
        request.setRawHeader("Content-Type", "application/bak");
        uploadFile.open(QFile::ReadOnly);
        QNetworkReply* reply = manager->put(request, uploadFile.readAll().toBase64());
        uploadFile.close();
        connect(manager, &QNetworkAccessManager::finished, this, [ = ](QNetworkReply * _reply) {
            if (_reply->readAll().toStdString() == "success") {
                QMessageBox::information(this, "提示", "上传成功。",
                                         QMessageBox::Yes, QMessageBox::Yes);
            }
        });
        connect(reply, &QNetworkReply::uploadProgress, this, [](qint64 bytesReceived, qint64 bytesTotal) {
            qDebug() << bytesReceived << "/" << bytesTotal;
        });
        connect(reply,
                QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::error),
                this,
        [ = ](QNetworkReply::NetworkError code) {
            if (code) {
                QMessageBox::information(this, "提示", "上传失败。",
                                         QMessageBox::Yes, QMessageBox::Yes);
            }
        });
    }
}

void Widget::on_localGroupBox_clicked(bool checked) {
    ui->localGroupBox->setChecked(true);
    ui->cloudGroupBox->setChecked(false);
}

void Widget::on_cloudGroupBox_clicked(bool checked) {
    ui->localGroupBox->setChecked(false);
    ui->cloudGroupBox->setChecked(true);
    ui->cloudFileRestoreLineEdit->setEnabled(false);
    ui->cloudFileList->clear();
    QNetworkAccessManager* manager = new QNetworkAccessManager(this);
    QNetworkRequest request(QUrl(api + "filelist"));
    QNetworkReply* reply = manager->get(request);
    connect(manager, &QNetworkAccessManager::finished, this, [ = ](QNetworkReply * _reply) {
        auto filelist = QJsonDocument::fromJson(_reply->readAll()).array();
        for (auto file : filelist) {
            QTreeWidgetItem* item = new QTreeWidgetItem;
            item->setText(0, file.toString());
            ui->cloudFileList->addTopLevelItem(item);
        }
    });
    connect(reply,
            QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::error),
            this,
    [ = ](QNetworkReply::NetworkError code) {
        if (code) {
            QMessageBox::information(this, "提示", "拉取云端列表失败。",
                                     QMessageBox::Yes, QMessageBox::Yes);
        }
    });
}

void Widget::on_browseLocalFile_clicked() {
    auto file = QFileDialog::getOpenFileName(
                    this,
                    "选择一个备份文件",
                    "",
                    "所有文件 (*.*)");
    if (file != "") {
        ui->localFileRestoreLineEdit->setText(file);
    }
}

void Widget::on_cloudFileList_currentItemChanged(QTreeWidgetItem* current, QTreeWidgetItem* previous) {
    if (current)
        ui->cloudFileRestoreLineEdit->setText(current->text(0));
}

void Widget::on_browseRestoreDictoryButton_clicked() {
    QString directory = QFileDialog::getExistingDirectory(
                            this,
                            tr("恢复到"),
                            "/home",
                            QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (directory != "") {
        ui->backupFileRestoreDictoryLineEdit->setText(directory);
    }
}

void Widget::on_passwordCheckBox_2_stateChanged(int arg1) {
    ui->passwordLineEdit_2->setEnabled(ui->passwordCheckBox_2->checkState());
}

void Widget::on_deleteTaskButton_clicked() {
    if (ui->taskList->currentItem()) {
        taskManager.removeTask(ui->taskList->indexOfTopLevelItem(ui->taskList->currentItem()));
        delete ui->taskList->currentItem();
        if (!ui->taskList->currentItem() && ui->taskList->topLevelItemCount()) {
            ui->taskList->setCurrentItem(ui->taskList->topLevelItem(0));
        }
    }
}

void Widget::on_clearTaskButton_clicked() {
    ui->taskList->clear();
    taskManager.clear();
}

void Widget::on_taskList_customContextMenuRequested(const QPoint& pos) {
    QTreeWidgetItem* currentItem = ui->taskList->itemAt(pos);
    if (currentItem) {
        popMenu = new QMenu(this);
        openFolder = popMenu->addAction("打开备份文件所在目录");
        connect(openFolder, &QAction::triggered, this, [ = ]() {
            QDesktopServices::openUrl(QUrl("file:///" + QFileInfo(currentItem->text(5)).path(), QUrl::TolerantMode));
        });
        popMenu->exec(QCursor::pos());
    }
}

void Widget::on_startRestoreButton_clicked() {
    if (ui->localGroupBox->isChecked() && ui->localFileRestoreLineEdit->text().trimmed() == "") {
        QMessageBox::information(this, "提示", "请选择要恢复的备份文件。",
                                 QMessageBox::Yes, QMessageBox::Yes);
        return;
    }
    if (ui->cloudGroupBox->isChecked() && ui->cloudFileRestoreLineEdit->text().trimmed() == "") {
        QMessageBox::information(this, "提示", "请选择要恢复的备份文件。",
                                 QMessageBox::Yes, QMessageBox::Yes);
        return;
    }
    if (ui->backupFileRestoreDictoryLineEdit->text().trimmed() == "") {
        QMessageBox::information(this, "提示", "请选择要恢复到的目录。",
                                 QMessageBox::Yes, QMessageBox::Yes);
        return;
    }
    if (ui->passwordCheckBox_2->isChecked() && ui->passwordLineEdit_2->text().trimmed() == "") {
        QMessageBox::information(this, "提示", "请输入密码。",
                                 QMessageBox::Yes, QMessageBox::Yes);
        return;
    }
    if (ui->cloudGroupBox->isChecked()) {
        QNetworkAccessManager* manager = new QNetworkAccessManager(this);
        QNetworkRequest request(QUrl(api + "file/" + ui->cloudFileRestoreLineEdit->text()));
        QNetworkReply* reply = manager->get(request);
        connect(manager, &QNetworkAccessManager::finished, this, [ = ](QNetworkReply * _reply) {
            auto file = QByteArray::fromBase64(_reply->readAll());
            qDebug() << file.size();
            QFile cloudFile("temp_" + ui->cloudFileRestoreLineEdit->text());
            cloudFile.open(QFile::WriteOnly);
            cloudFile.write(file);
            cloudFile.close();
            Decompressor decompressor;
            if (decompressor.decompress("temp_" + ui->cloudFileRestoreLineEdit->text().toStdString(),
                                        ui->backupFileRestoreDictoryLineEdit->text().toStdString() + "/",
                                        ui->passwordCheckBox_2->isChecked() ? ui->passwordLineEdit_2->text().toStdString() : "")) {
                QMessageBox::information(this, "提示", "解压失败。",
                                         QMessageBox::Yes, QMessageBox::Yes);
                return;
            }
            cloudFile.remove();
            QProcess tar;
            auto currentDictory = QDir::current();
            QDir::setCurrent(ui->backupFileRestoreDictoryLineEdit->text());
            QString tempFilename = "temp_" + ui->cloudFileRestoreLineEdit->text().left(ui->cloudFileRestoreLineEdit->text().length() - 3) + "tar";
            tar.start(currentDictory.path() + "/tar.exe", QStringList() << "-xvf" << tempFilename);
            tar.waitForStarted(-1);
            tar.waitForFinished(-1);
            QFile tarFile(tempFilename);
            tarFile.remove();
            QDir::setCurrent(currentDictory.path());
            QMessageBox::information(this, "提示", "恢复完成。",
                                     QMessageBox::Yes, QMessageBox::Yes);
        });
        connect(reply,
                QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::error),
                this,
        [ = ](QNetworkReply::NetworkError code) {
            if (code) {
                QMessageBox::information(this, "提示", "云备份下载失败。",
                                         QMessageBox::Yes, QMessageBox::Yes);
            }
        });
    } else {
        Decompressor decompressor;
        int code = decompressor.decompress(ui->localFileRestoreLineEdit->text().toStdString(),
                                           ui->backupFileRestoreDictoryLineEdit->text().toStdString() + "/",
                                           ui->passwordCheckBox_2->isChecked() ? ui->passwordLineEdit_2->text().toStdString() : "");
        if (code) {
            QMessageBox::information(this, "提示", "解压失败。",
                                     QMessageBox::Yes, QMessageBox::Yes);
            qDebug() << code;
            return;
        }
        QProcess tar;
        auto currentDictory = QDir::current();
        QDir::setCurrent(ui->backupFileRestoreDictoryLineEdit->text());
        QString tempFilename = QFileInfo(ui->localFileRestoreLineEdit->text()).fileName();
        tempFilename = tempFilename.left(tempFilename.length() - 3) + "tar";
        tar.start(currentDictory.path() + "/tar.exe", QStringList() << "-xvf" << tempFilename);
        tar.waitForStarted(-1);
        tar.waitForFinished(-1);
        QFile tarFile(tempFilename);
        tarFile.remove();
        QDir::setCurrent(currentDictory.path());
        QMessageBox::information(this, "提示", "恢复完成。",
                                 QMessageBox::Yes, QMessageBox::Yes);
    }
}

void Widget::on_cloudFileList_customContextMenuRequested(const QPoint& pos) {
    QTreeWidgetItem* currentItem = ui->cloudFileList->itemAt(pos);
    if (currentItem) {
        popMenu = new QMenu(this);
        removeCloudBackupFile = popMenu->addAction("删除该云备份");
        connect(removeCloudBackupFile, &QAction::triggered, this, [ = ]() {
            if (QMessageBox::Yes != QMessageBox::question(this, "警告", "确认删除？", QMessageBox::Yes | QMessageBox::No)) {
                return;
            }
            QNetworkAccessManager* manager = new QNetworkAccessManager(this);
            QNetworkRequest request(QUrl(api + "file/" + currentItem->text(0)));
            QNetworkReply* reply = manager->deleteResource(request);
            connect(manager, &QNetworkAccessManager::finished, this, [ = ](QNetworkReply * _reply) {
                if (_reply->readAll().toStdString() == "success") {
                    QMessageBox::information(this, "提示", "删除成功。",
                                             QMessageBox::Yes, QMessageBox::Yes);
                    if (ui->cloudFileList->topLevelItemCount()) {
                        ui->cloudFileList->setCurrentItem(ui->cloudFileList->topLevelItem(0));
                    }
                }
            });
            connect(reply,
                    QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::error),
                    this,
            [ = ](QNetworkReply::NetworkError code) {
                if (code) {
                    QMessageBox::information(this, "提示", "删除失败。",
                                             QMessageBox::Yes, QMessageBox::Yes);
                }
            });
        });
        popMenu->exec(QCursor::pos());
    }
}
