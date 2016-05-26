/**************************************************************************
 *  main.cpp
 *  Created 26/5/2016 by Dimitar T. Dimitrov
 *  mitakatdd@gmail.com
 *
 *  MIT License
 *
 **************************************************************************/
#include "MainWindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	MainWindow w;
	w.hide();

	QApplication::setQuitOnLastWindowClosed(false);

	return a.exec();
}
