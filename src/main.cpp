//
// Created by wuxianggujun on 2024/7/24.
//
#include "MainWindow.hpp"
#include <QApplication>

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    Tina::MainWindow w;
    w.show();
    return a.exec();
}