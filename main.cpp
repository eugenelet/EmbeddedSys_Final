#include <QtGui/QApplication>
#include <QtGui/QPushButton>
#include <QtGui/QFont>
#include <QtGui/QWidget>
#include <QtGui/QLabel>
#include <QtGui/QImage>
#include <QtGui/QPainter>
#include <QTimer>
#include <QImage>
#include "W.h"

using namespace std;

int main(int argc, char *argv[]){
	
	QApplication app(argc,argv);
	PainterWidget* widget = new PainterWidget();

	widget->show();

	return app.exec();

}
