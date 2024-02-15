#include "Dialog.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QDialog>
#include <QCheckBox>
#include <QRadioButton>
#include <QPlainTextEdit>
#include <QPushButton>

Dialog::Dialog(QWidget *parent)
	: QDialog(parent)
{
	checkBoxUnder = new QCheckBox("下划线");
	checkBoxItalic = new QCheckBox("斜体");
	checkBoxBold = new QCheckBox("加粗");
	QHBoxLayout* HLayl = new QHBoxLayout();
	HLayl->addWidget(checkBoxUnder);
	HLayl->addWidget(checkBoxItalic);
	HLayl->addWidget(checkBoxBold);


	radioBlack = new QRadioButton("黑色");
	radioRed = new QRadioButton("红色");
	radioBlue = new QRadioButton("蓝色");

	QHBoxLayout* HLayl2 = new QHBoxLayout();
	HLayl2->addWidget(radioBlack);
	HLayl2->addWidget(radioRed);
	HLayl2->addWidget(radioBlue);

	textEdit = new QPlainTextEdit();
	textEdit->setPlainText("Hello World\n");


	QVBoxLayout* Vlay1 = new QVBoxLayout();
	Vlay1->addLayout(HLayl);
	Vlay1->addLayout(HLayl2);
	Vlay1->addWidget(textEdit);
	

	setLayout(Vlay1);
}

Dialog::~Dialog()
{}
