#include "LogMessageBox.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QIcon>

LogMessageBox::LogMessageBox(bool success, const QString& text, QWidget* parent) :QDialog(parent)
{
	// 设置横态
	setWindowModality(Qt::ApplicationModal);
	// 隐藏？号
	setWindowFlags(Qt::Dialog | Qt::WindowCloseButtonHint | Qt::WindowStaysOnTopHint);
	setWindowTitle("提示");

	setFixedSize(320, 120);
	setStyleSheet(".LogMessageBox(background-color:rgb(255,255,255);)");

	QVBoxLayout* boxLayout = new QVBoxLayout(this);
	boxLayout->setContentsMargins(0, 0, 0, 0);
	boxLayout->setSpacing(0);

	QWidget* line = new QWidget(this);
	line->setStyleSheet(".QWidget{background-color:rgb(233,233,233);}");
	line->setFixedHeight(1);

	QWidget* boxHWidget = new QWidget(this);
	QHBoxLayout* boxHLayout = new QHBoxLayout(boxHWidget);
	boxHLayout->setContentsMargins(20, 0, 20, 0);

	// TODO 省略一下标签跟图标，暂时不会访问res的资源
	QString fileName;
	if (success)
	{
		fileName = ":/images/icon/success.png";

	}
	else
	{
		fileName = ":/images/icon/error.png";
	}

	QLabel* iconLabel = new QLabel(this);
	iconLabel->setPixmap(QIcon(fileName).pixmap(50, 50));


	QLabel* textLabel = new QLabel(this);
	textLabel->setWordWrap(true);
	QString tt = QString("<p style='font-family:Microsoft YaHei;font-size:15px;margin:10px;line-height:25px;'>%1</p>").arg(text);
	textLabel->setText(tr(tt.toUtf8()));

	boxHLayout->addWidget(iconLabel);
	boxHLayout->addSpacing(10);
	boxHLayout->addWidget(textLabel);
	boxHLayout->addStretch(5);

	boxLayout->addWidget(line);
	boxLayout->addWidget(boxHWidget);
}

void LogMessageBox::success(QWidget* parent, const QString& text)
{
	LogMessageBox lmb(true, text, parent);
	lmb.exec();
}

void LogMessageBox::error(QWidget* parent, const QString& text)
{
	LogMessageBox lmb(false, text, parent);
	lmb.exec();
}


