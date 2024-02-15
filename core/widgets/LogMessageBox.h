#pragma once

#include <QDialog>

class LogMessageBox  : public QDialog
{
	Q_OBJECT

private:
	explicit LogMessageBox(bool success, const QString& text, QWidget* parent);

public:
	static void success(QWidget* parent, const QString& text);
	static void error(QWidget* parent, const QString& text);
};
