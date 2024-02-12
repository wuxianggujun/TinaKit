#include "mainwindow.h"

#include <QApplication>
#include <include/core/SkBitmap.h>
#include <include/core/SkCanvas.h>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
