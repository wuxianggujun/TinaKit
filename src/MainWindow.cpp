//
// Created by wuxianggujun on 2024/7/24.
//

// You may need to build the project (run Qt uic code generator) to get "ui_MainWindow.h" resolved

#include "MainWindow.hpp"
#include <QTableView>
namespace Tina
{
    void Ui_MainWindow::setUpUi(QMainWindow* mainWindow)
    {
        mainWindow->resize(800,600);

        // 创建工具栏
        toolBar = new QToolBar(mainWindow);
        mainWindow->addToolBar(toolBar);

        auto *newAction = new QAction("新建", mainWindow);
        auto *openAction = new QAction("打开", mainWindow);
        auto *saveAction = new QAction("保存", mainWindow);

        toolBar->addAction(newAction);
        toolBar->addAction(openAction);
        toolBar->addAction(saveAction);

        // 创建中央窗口
        auto* centralWidget = new QWidget(mainWindow);
        mainWindow->setCentralWidget(centralWidget);

        // 创建布局
        auto * layout = new QVBoxLayout(centralWidget);

        // 创建列表视图
        auto * listView = new QListView(centralWidget);
        listModel = new QStandardItemModel(centralWidget);
        listModel->appendRow(new QStandardItem("4G"));
        listModel->appendRow(new QStandardItem("5G"));
        listView->setModel(listModel);

        // 创建表格视图
        auto * tableView = new QTableView(centralWidget);
        tableModel = new QStandardItemModel(1,5,centralWidget);
        tableModel->setHeaderData(0, Qt::Horizontal, "文件名");
        tableModel->setHeaderData(1, Qt::Horizontal, "大小(MB)");
        tableModel->setHeaderData(2, Qt::Horizontal, "开始时间");
        tableModel->setHeaderData(3, Qt::Horizontal, "结束时间");
        tableModel->setHeaderData(4, Qt::Horizontal, "分析状态");

        QList<QStandardItem*> row;
        row.append(new QStandardItem("测试文件"));
        row.append(new QStandardItem("9.87"));
        row.append(new QStandardItem("2024/07/20 11:43:45"));
        row.append(new QStandardItem("2024/07/20 11:48:08"));
        row.append(new QStandardItem("已分析"));
        tableModel->appendRow(row);

        tableView->setModel(tableModel);

        layout->addWidget(listView);
        layout->addWidget(tableView);
    }

    void Ui_MainWindow::cleanUpUi() const
    {
        delete listView;
        delete tableView;
        delete toolBar;
        delete listModel;
        delete tableModel;
    }

    MainWindow::MainWindow(QWidget* parent) :
        QMainWindow(parent), ui()
    {
        ui.setUpUi(this);
        
        // 连接信号和槽
        connect(ui.toolBar->actions().at(0), &QAction::triggered, this, &MainWindow::on_actionNew_triggered);
        connect(ui.toolBar->actions().at(1), &QAction::triggered, this, &MainWindow::on_actionOpen_triggered);
        connect(ui.toolBar->actions().at(2), &QAction::triggered, this, &MainWindow::on_actionSave_triggered);
    }

    MainWindow::~MainWindow()
    {
        ui.cleanUpUi();
    }

    void MainWindow::on_actionNew_triggered()
    {
    }

    void MainWindow::on_actionOpen_triggered()
    {
    }

    void MainWindow::on_actionSave_triggered()
    {
    }
} // Tina
