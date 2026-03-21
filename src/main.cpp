#include "mainwindow.h"
#include <time.h>
#include <stdlib.h>
#include <QApplication>
#include <QProxyStyle>

std::default_random_engine generator;

int main(int argc, char *argv[]){
	QApplication a(argc, argv);
	MainWindow w;
	w.show();
	srand(time(NULL));

	return a.exec();
}
