#include <QtGui/QApplication>
#include <QDesktopWidget>
#include <QLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTextCodec>

#include "mainwindow.h"

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));
//    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("GBK"));
	qRegisterMetaType<UnPackInfo>("UnPackInfo");
	qRegisterMetaType<UnPackInfo>("UnPackInfo&");
	qRegisterMetaType<QString>("QString");
	qRegisterMetaType<QString>("QString&");
    qRegisterMetaType<UnpackStatus>("UnpackStatus");

	MainWindow w;
	w.show();
	w.move ((QApplication::desktop()->width() - w.width())/2,(QApplication::desktop()->height() - w.height())/2);

	return a.exec();
}
