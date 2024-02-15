#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <QFile>

MainWindow::MainWindow(QWidget* parent)
	: QMainWindow(parent)
	, ui(new Ui::MainWindow)
{
	ui->setupUi(this);

	/*QFile file(":/test.txt");
	file.open(QIODevice::ReadOnly);
	logViewer.addLog(file.readAll());
	logViewer.show();
	file.close();*/
	throw 5;
}

MainWindow::~MainWindow()
{
	delete ui;
}

LogViewer* MainWindow::getLogViewer()
{
	return &logViewer;
}