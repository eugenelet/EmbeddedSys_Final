#include "W.h"
#include <QtGui/QApplication>
#include <QtGui/QPushButton>
#include <QtGui/QFont>
#include <QtGui/QWidget>
#include <QtGui/QLabel>
#include <QtGui/QImage>
#include <QtGui/QPainter>
#include <QTimer>
#include <QImage>
#include <QString>

//L4V2 Lib
#include "camera.h"
#include "image_process.h"
using namespace cv;
using namespace std;
//implement your member function here

int fd;
camera_buffer camera_buffers;
void *ori_img;
unsigned int img[CAM_WIDTH*CAM_HEIGHT*sizeof(unsigned int)];


static CvMemStorage * storage = 0;
static CvHaarClassifierCascade * cascade = 0;


QImage *IplImageToQImage(IplImage* cvimage);
IplImage *QImageToIplImage( QImage * qImage);

//	ctor
PainterWidget::PainterWidget(QWidget *parent):QWidget(){
        timer = new QTimer(this);
	detect=0;
	camera_init(&fd,&camera_buffers,1);
     btnDetect= new QPushButton(this);
     btnDetect->setText("Detect");
     btnDetect->setFont(QFont("Courier", 18, QFont::Bold));
     btnDetect->setGeometry(580,100,110,50);

	connect( btnDetect, SIGNAL(clicked()), this, SLOT(setDetect()));
	connect(timer, SIGNAL(timeout()), this, SLOT(update()));

	timer->start(30);
}



IplImage* img_resize(IplImage* src_img, int new_width,int new_height)
{
    IplImage* des_img;
    des_img=cvCreateImage(cvSize(new_width,new_height),src_img->depth,src_img->nChannels);
    cvResize(src_img,des_img,CV_INTER_LINEAR);
    return des_img;
}




void PainterWidget::paintEvent(QPaintEvent* event){
       QPainter painter(this);

        capture(&fd,&camera_buffers,&ori_img,1,0);
        yuyv2rgb(CAM_WIDTH,CAM_HEIGHT,ori_img,img);
	image= QImage((unsigned char *)img,CAM_WIDTH,CAM_HEIGHT,QImage::Format_RGB32);
	iplimg=QImageToIplImage(&image);	
	painter.drawImage(330,0,process_qImage);
		
	painter.drawImage(0,0,image);


}




PainterWidget::~PainterWidget(){
	camera_uninit(&fd,&camera_buffers,1);
}

IplImage *QImageToIplImage( QImage * qImage){
	int width = qImage->width();
	int height = qImage->height();
	CvSize Size;
	Size.height = height;
	Size.width = width;
	IplImage *IplImageBuffer = cvCreateImage(Size, IPL_DEPTH_8U, 3);
	for (int y = 0; y < height; ++y){
		for (int x = 0; x < width; ++x){
			QRgb rgb = qImage->pixel(x, y);
			cvSet2D(IplImageBuffer, y, x, CV_RGB(qRed(rgb), qGreen(rgb), qBlue(rgb)));
		}
	}
	return IplImageBuffer;
}

QImage *IplImageToQImage(IplImage* cvimage){
	if (!cvimage)
		return 0;
	QImage* desImage = new QImage(cvimage->width,cvimage->height,QImage::Format_RGB32);
	for(int i = 0; i < cvimage->height; i++){
		for(int j = 0; j < cvimage->width; j++){
			int r,g,b;
		    if(3 == cvimage->nChannels){
				b=(int)CV_IMAGE_ELEM(cvimage,uchar,i,j*3+0);
				g=(int)CV_IMAGE_ELEM(cvimage,uchar,i,j*3+1);
				r=(int)CV_IMAGE_ELEM(cvimage,uchar,i,j*3+2);
			}
			else if(1 == cvimage->nChannels){
				b=(int)CV_IMAGE_ELEM(cvimage,uchar,i,j);
				g=b;
				r=b;
			}
			desImage->setPixel(j,i,qRgb(r,g,b));
		}
	}
	return desImage;
}


void PainterWidget::setDetect(){
	detect=!detect;
}

QImage& cvxCopyIplImage(const IplImage* pIplImage, QImage &qImage)
{   //implement the conversion function here
        if(!pIplImage) return qImage;

        if(qImage.isNull())
        {
                int w = pIplImage->width;
                int h = pIplImage->height;
                qImage = QImage(w, h, QImage::Format_RGB32);
        }

        int x, y;
        for(x = 0; x < pIplImage->width; ++x)
        {
           for(y = 0; y < pIplImage->height; ++y)
           {
              CvScalar color = cvGet2D(pIplImage, y, x);

              int r = (int)color.val[2];
              int g = (int)color.val[1];
              int b = (int)color.val[0];
	
              qImage.setPixel(x, y, qRgb(r,g,b));
           }
        }
   return qImage;
}
void PainterWidget::update(){
//	if(detect){processImage(iplimg);detect =0;}	
  	Mat src1(iplimg);
    Mat gray, edge, draw;
    cvtColor(src1, gray, CV_BGR2GRAY);
	Canny( gray, edge, 50, 150, 3);
	edge.convertTo(draw, CV_8U);
	IplImage copy = draw;
	IplImage* new_image = &copy;
	cvxCopyIplImage (new_image, process_qImage);
	
	timer->start(30);
        repaint();

}



//dog


//dog
void PainterWidget::processImage(IplImage* pIplImage){

        IplImage* processIplImage;

                processIplImage = cvCloneImage (pIplImage);


                CvHaarClassifierCascade* cascade;
                cascade = (CvHaarClassifierCascade*)cvLoad("haarcascade_frontalface_alt.xml", 0, 0, 0 );
                CvMemStorage* storage =cvCreateMemStorage(0);

                CvSeq* faces = cvHaarDetectObjects(     processIplImage,
                                                         cascade,
                                                         storage,
                                                        1.1, 2,
                                                        CV_HAAR_DO_CANNY_PRUNING,
                                                        cvSize(20, 20),cvSize(20, 20) );

                if (faces)
                {
                        for(int k= 0; k< faces->total; k++)
                        {
                                CvPoint pt1, pt2;
                                CvRect* rectangle = (CvRect*)cvGetSeqElem(faces, k);
                                pt1.x = rectangle->x;
                                pt2.x = rectangle->x + rectangle->width;
                                pt1.y = rectangle->y;
                                pt2.y = rectangle->y + rectangle->height;
                        //Draw a red rectanngle
                                cvRectangle( processIplImage, pt1,pt2, CV_RGB(255,0,0), 3, 8, 0 );
                        }
                }

                qImage = cvxCopyIplImage (pIplImage,qImage);
                process_qImage = cvxCopyIplImage (processIplImage, process_qImage);

}



