#include "MainWindow.hpp"
#include <QStackedWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>

namespace Tina
{
    void Ui_MainWindow::setUpUi(QMainWindow* mainWindow)
    {
        mainWindow->resize(800, 600);

        // 创建工具栏
        toolBar = new QToolBar(mainWindow);
        mainWindow->addToolBar(toolBar);

        auto* newAction = new QAction("新建", mainWindow);
        auto* openAction = new QAction("打开", mainWindow);
        auto* saveAction = new QAction("保存", mainWindow);

        toolBar->addAction(newAction);
        toolBar->addAction(openAction);
        toolBar->addAction(saveAction);

        // 创建中央窗口
        auto* centralWidget = new QWidget(mainWindow);
        mainWindow->setCentralWidget(centralWidget);

        // 创建主布局
        auto* mainLayout = new QHBoxLayout(centralWidget);
        centralWidget->setLayout(mainLayout);

        // 创建左侧按钮布局
        auto* leftWidget = new QWidget();
        auto* leftLayout = new QVBoxLayout(leftWidget);
        leftWidget->setLayout(leftLayout);

        auto* btnGroup = new QButtonGroup(leftWidget);
        btnGroup->setExclusive(true);

        // 创建按钮并连接槽函数
        auto* btn1 = new QPushButton("布局 1");
        auto* btn2 = new QPushButton("布局 2");
        auto* btn3 = new QPushButton("布局 3");
        auto* btn4 = new QPushButton("布局 4");
        auto* btn5 = new QPushButton("布局 5");

        leftLayout->addWidget(btn1);
        leftLayout->addWidget(btn2);
        leftLayout->addWidget(btn3);
        leftLayout->addWidget(btn4);
        leftLayout->addWidget(btn5);

        btnGroup->addButton(btn1, 0);
        btnGroup->addButton(btn2, 1);
        btnGroup->addButton(btn3, 2);
        btnGroup->addButton(btn4, 3);
        btnGroup->addButton(btn5, 4);

        btn1->setProperty("id", 0);
        btn2->setProperty("id", 1);
        btn3->setProperty("id", 2);
        btn4->setProperty("id", 3);
        btn5->setProperty("id", 4);


        // 创建右侧布局
        auto* rightWidgetContainer = new QWidget();
        auto* rightLayout = new QStackedLayout(rightWidgetContainer);
        rightWidgetContainer->setLayout(rightLayout);

        // 创建不同的右侧布局页面
        auto* rightWidget1 = new QWidget();
        auto* rightLayout1 = new QVBoxLayout(rightWidget1);
        rightWidget1->setLayout(rightLayout1);
        rightLayout1->addWidget(new QLabel("右侧布局 1"));

        auto* rightWidget2 = new QWidget();
        auto* rightLayout2 = new QVBoxLayout(rightWidget2);
        rightWidget2->setLayout(rightLayout2);
        rightLayout2->addWidget(new QLabel("右侧布局 2"));

        auto* rightWidget3 = new QWidget();
        auto* rightLayout3 = new QVBoxLayout(rightWidget3);
        rightWidget3->setLayout(rightLayout3);
        rightLayout3->addWidget(new QLabel("右侧布局 3"));

        auto* rightWidget4 = new QWidget();
        auto* rightLayout4 = new QVBoxLayout(rightWidget4);
        rightWidget4->setLayout(rightLayout4);
        rightLayout4->addWidget(new QLabel("右侧布局 4"));

        auto* rightWidget5 = new QWidget();
        auto* rightLayout5 = new QVBoxLayout(rightWidget5);
        rightWidget5->setLayout(rightLayout5);
        rightLayout5->addWidget(new QLabel("右侧布局 5"));

        // 将右侧布局页面添加到 QStackedLayout
        rightLayout->addWidget(rightWidget1);
        rightLayout->addWidget(rightWidget2);
        rightLayout->addWidget(rightWidget3);
        rightLayout->addWidget(rightWidget4);
        rightLayout->addWidget(rightWidget5);

        // 默认显示第一个布局
        rightLayout->setCurrentIndex(1);

        // 将左侧和右侧布局添加到主布局
        mainLayout->addWidget(leftWidget);
        mainLayout->addWidget(rightWidgetContainer);

        /*
        QObject::connect(btnGroup, QOverload<QAbstractButton*>::of(&QButtonGroup::buttonClicked),
                         [rightLayout](QAbstractButton* button)
                         {
                             int index = button->property("id").toInt();
                             qDebug() << "Button clicked with index:" << index;
                             rightLayout->setCurrentIndex(index);
                         });
                         */

        // 连接左侧按钮的点击信号与右侧布局的切换
        QObject::connect(btnGroup, QOverload<QAbstractButton*>::of(&QButtonGroup::buttonClicked),
                         [btnGroup, rightLayout](QAbstractButton* button)
                         {
                             int index = btnGroup->id(button);
                             qDebug() << "Button clicked with index:" << index;
                             rightLayout->setCurrentIndex(index);
                         });
    }

    void Ui_MainWindow::cleanUpUi() const
    {
    }

    MainWindow::MainWindow(QWidget* parent) :
        QMainWindow(parent), ui(new Ui_MainWindow())
    {
        ui->setUpUi(this);

        // 连接信号和槽
        connect(ui->toolBar->actions().at(0), &QAction::triggered, this, &MainWindow::on_actionNew_triggered);
        connect(ui->toolBar->actions().at(1), &QAction::triggered, this, &MainWindow::on_actionOpen_triggered);
        connect(ui->toolBar->actions().at(2), &QAction::triggered, this, &MainWindow::on_actionSave_triggered);
    }

    MainWindow::~MainWindow()
    {
        ui->cleanUpUi();
        delete ui;
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
