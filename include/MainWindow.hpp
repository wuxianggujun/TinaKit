//
// Created by wuxianggujun on 2024/7/24.
//

#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <QLabel>
#include <QWidget>
#include <QMainWindow>
#include <QTableView>
#include <QToolBar>
#include <QListView>
#include <QPushButton>
#include <QToolButton>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QStackedWidget>
#include <QStackedLayout>
#include <QButtonGroup>
#include <QStandardItemModel>

namespace Tina
{
    class Ui_MainWindow
    {
    public:
        void setUpUi(QMainWindow* mainWindow);
        void cleanUpUi() const;

    public:
        QToolBar* toolBar{};
        QStackedWidget* stackedWidget{};
    };

    class MainWindow : public QMainWindow
    {
        Q_OBJECT

    public:
        explicit MainWindow(QWidget* parent = nullptr);
        ~MainWindow() override;

    private slots:
        void on_actionNew_triggered();
        void on_actionOpen_triggered();
        void on_actionSave_triggered();

    private:
        Ui_MainWindow *ui;
    };
} // Tina

#endif //MAINWINDOW_HPP
