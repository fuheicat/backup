#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QTreeWidgetItem>
#include <QMenu>
#include <QDateTime>
#include <QTimer>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include "task.h"
#include "compressor.h"
#include "decompressor.h"

const QString api = "http://119.28.65.16:5000/";

namespace Ui {
class Widget;
}

class Widget : public QWidget {
    Q_OBJECT

  public:
    explicit Widget(QWidget* parent = nullptr);
    ~Widget();

  private slots:
    void on_passwordCheckBox_stateChanged(int arg1);

    void on_browseButton_clicked();

    void on_addFileButton_clicked();

    void on_deleteFileButton_clicked();

    void on_clearFileButton_clicked();

    void on_addDirectoryButton_clicked();

    void on_startBackupButton_clicked();

    void on_localGroupBox_clicked(bool checked);

    void on_cloudGroupBox_clicked(bool checked);

    void on_browseLocalFile_clicked();

    void on_cloudFileList_currentItemChanged(QTreeWidgetItem* current, QTreeWidgetItem* previous);

    void on_browseRestoreDirectoryButton_clicked();

    void on_passwordCheckBox_2_stateChanged(int arg1);

    void on_deleteTaskButton_clicked();

    void on_clearTaskButton_clicked();

    void on_taskList_customContextMenuRequested(const QPoint& pos);

    void on_startRestoreButton_clicked();

    void on_cloudFileList_customContextMenuRequested(const QPoint& pos);

  private:
    Ui::Widget* ui;
    QMenu* popMenu;
    QAction* openFolder;
    QAction* removeCloudBackupFile;
    QAction* check;
    TaskManager taskManager;
    QTimer timer;
};

#endif // WIDGET_H
