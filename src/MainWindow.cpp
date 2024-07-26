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


        // 创建右侧布局
        auto* rightWidgetContainer = new QWidget();
        auto* rightLayout = new QStackedLayout(rightWidgetContainer);
        rightWidgetContainer->setLayout(rightLayout);


        
        for (int i = 0; i < 10; i++)
        {
            auto* btn = new QPushButton(QString("按钮 %1").arg(i + 1));
            btnGroup->addButton(btn,i);
            leftLayout->addWidget(btn);
            

            auto* label = new QLabel(QString("标签 %1").arg(i + 1));
            auto* widget = new QWidget();
            auto* layout = new QVBoxLayout(widget);
            widget->setLayout(layout);
            layout->addWidget(label);
            rightLayout->addWidget(widget);
            
        }


  

        // 默认显示第一个布局
        rightLayout->setCurrentIndex(1);

        // 将左侧和右侧布局添加到主布局
        mainLayout->addWidget(leftWidget);
        mainLayout->addWidget(rightWidgetContainer);
        

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
