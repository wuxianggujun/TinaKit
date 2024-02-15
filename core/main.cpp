#include "mainwindow.h"
#include "widgets/LogMessageBox.h"


#include <windows.h>
#include <QApplication>
//#include <include/core/SkBitmap.h>
//#include <include/core/SkCanvas.h>


LONG ApplicationCrashHandler(EXCEPTION_POINTERS* pException) {
	EXCEPTION_RECORD* record = pException->ExceptionRecord;

	QString errCode(QString::number(record->ExceptionCode, 16)), errAdr(QString::number((uint)record->ExceptionAddress, 16)), errMod;
	QString crashMsg = QString("抱歉，软件发生了崩溃，请重启。错误代码：%1，错误地址：%2").
		arg(errCode).arg(errAdr);

	LogMessageBox::error(nullptr, crashMsg);

	// 表示我已经处理了异常，可以优雅地结束了
	return EXCEPTION_EXECUTE_HANDLER;
}


int main(int argc, char* argv[])
{
	// 注册异常捕获函数
	SetUnhandledExceptionFilter((LPTOP_LEVEL_EXCEPTION_FILTER)ApplicationCrashHandler);

	QApplication a(argc, argv);
	MainWindow w;
	w.show();
	return a.exec();
}