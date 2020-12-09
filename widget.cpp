#include "widget.h"
#include "ui_widget.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFont>
#include <QIcon>
#include <QAbstractItemView>
#include <QDebug>

Widget::Widget(QWidget *parent) : QWidget(parent), ui(new Ui::Widget) {
    ui->setupUi(this);
    this->setWindowTitle("文件备份系统");
    this->setFixedSize(768, 768);
    auto font = QFont("微软雅黑");
    font.setPointSize(10);
    this->setFont(font);
    QWidget * backupWidget = new QWidget();
    QHBoxLayout backupLayout;
    backupList.setHorizontalHeaderLabels(QStringList({"文件名", "完整路径"}));
    backupList.setEditTriggers(QAbstractItemView::NoEditTriggers);
    backupList.setSelectionBehavior(QAbstractItemView::SelectRows);
    backupList.setColumnWidth(0, 100);
    qDebug() << backupList.width() << endl;
    backupList.setColumnWidth(1, 495);
    backupLayout.addWidget(&backupList);
    backupWidget->setLayout(&backupLayout);


    QWidget * restoreWidget = new QWidget();
    QWidget * taskWidget = new QWidget();

    tabWidget.addTab(backupWidget, QIcon(":/image/image/backup.png"), "备份");
    tabWidget.addTab(restoreWidget, QIcon(":/image/image/restore.png"), "恢复");
    tabWidget.addTab(taskWidget, QIcon(":/image/image/task.png"), "定时");
    tabWidget.resize(this->width(), this->height());
    tabWidget.setTabShape(QTabWidget::Triangular);
    tabWidget.setIconSize(QSize(20, 20));
    tabWidget.setMovable(true);

    QHBoxLayout layout;
    layout.addWidget(&tabWidget);
    this->setLayout(&layout);
    this->setWindowIcon(QIcon(":/image/image/icon.png"));
}

Widget::~Widget() {
    delete ui;
}
