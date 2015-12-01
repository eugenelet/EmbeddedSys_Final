#ifndef W_H
#define W_H
#include <QWidget>
#include <QPainter>
#include <QImage>
#include <QTimer>
#include <QPushButton>
#include <stdio.h>
#include <QtGui/QLabel>

#include <opencv/highgui.h>
#include <opencv/cv.h>
#include <opencv/cxcore.h>
#include <iostream>
#include <string>


using namespace cv;
using namespace std;
		 
class PainterWidget: public QWidget{
	Q_OBJECT
		 protected:
	     public:
		 //member functions
		 	PainterWidget(QWidget *parent = 0);
			~PainterWidget();
		 	void paintEvent(QPaintEvent*); ///gene
			void setFaceDetect(); //gene
			
			void processImage(IplImage*);//dog
			
		 public slots:
		 //public slots
                        void setDetect();
			void update(); 
		private:
		//data members
			QPushButton *btnDetect; //gene
			QImage image; //gene
			int detect; //gene

                        IplImage* iplimg; //gene
                        QTimer *timer; //gene			
			QImage qImage;	//dog
			QImage process_qImage;	//dog
//			Mat src1;

};

#endif
