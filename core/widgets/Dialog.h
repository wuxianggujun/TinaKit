#pragma once

#include <QDialog>

class QCheckBox;
class QRadioButton;
class QPlainTextEdit;
class QPushButton;


class Dialog  : public QDialog
{
	Q_OBJECT

public:
	Dialog(QWidget *parent);
	~Dialog();


	QCheckBox* checkBoxUnder;
	QCheckBox* checkBoxItalic;
	QCheckBox* checkBoxBold;

	QRadioButton* radioBlack;
	QRadioButton* radioRed;
	QRadioButton* radioBlue;

	QPlainTextEdit* textEdit;

};
