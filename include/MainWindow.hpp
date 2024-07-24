//
// Created by wuxianggujun on 2024/7/24.
//

#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <QWidget>

namespace Tina {
QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QWidget {
Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private:
    Ui::MainWindow *ui;
};
} // Tina

#endif //MAINWINDOW_HPP
