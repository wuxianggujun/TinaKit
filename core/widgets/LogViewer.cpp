#include "LogViewer.h"

LogViewer::LogViewer(QWidget* parent) :QDialog(parent)
{
	this->resize(640, 480);
	QVBoxLayout* mainLayout = new QVBoxLayout(this);
	textArea = new QPlainTextEdit(this);
	textArea->setReadOnly(1);
	mainLayout->addWidget(textArea);
	setLayout(mainLayout);
}

LogViewer::~LogViewer()
{}

void LogViewer::clearLog()
{
	textArea->clear();
}

void LogViewer::addLog(QString text)
{
	if (textArea)
	{
		textArea->appendPlainText(text);
	}
}