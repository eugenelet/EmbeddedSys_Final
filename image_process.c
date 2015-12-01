#ifdef __cplusplus
extern "C" {
#endif

#include "image_process.h"

void yuyv2rgb(int width, int height, const void *src, const void *dst)
{
  int tmp;
  unsigned char *yuyv_img = (unsigned char *)src;
  unsigned int *new_img = (unsigned int *)dst;

  //unsigned char y0,u,y1,v;
  int y0,u,y1,v;
  int r,g,b;
  int cnt=(width * height)*2;

  for (tmp = 0; tmp < cnt; tmp=tmp+4) {
    //y0= yuyv_img[tmp];
    //u= yuyv_img[tmp+1]; /* Cr */
    //y1= yuyv_img[tmp+2];
    //v= yuyv_img[tmp+3]; /* Cb */

    y0= yuyv_img[tmp];
    u= yuyv_img[tmp+1]-128;
    y1= yuyv_img[tmp+2];
    v= yuyv_img[tmp+3]-128;

	/*r = y0+1.402*v;
	g = y0-0.344*u-0.714*v;
	b = y0+1.772*u;*/
	r = y0+((1435*v)>>10);
	g = y0-((352*u+731*v)>>10);
	b = y0+((1815*u)>>10);

	if(r > 255) r = 255;
	if(g > 255) g = 255;
	if(b > 255) b = 255;
	if(r < 0) r = 0;
	if(g < 0) g = 0;
	if(b < 0) b = 0;

    *new_img = 0xFF000000|(r<<16)|(g<<8)|b; /* 0xFFRRGGBB */
	new_img++;

	/*r = y1+1.402*v;
	g = y1-0.344*u-0.714*v;
	b = y1+1.772*u;*/
	r = y1+((1435*v)>>10);
	g = y1-((352*u+731*v)>>10);
	b = y1+((1815*u)>>10);

	if(r > 255) r = 255;
	if(g > 255) g = 255;
	if(b > 255) b = 255;
	if(r < 0) r = 0;
	if(g < 0) g = 0;
	if(b < 0) b = 0;

    *new_img = 0xFF000000|(r<<16)|(g<<8)|b; /* 0xFFRRGGBB */
	new_img++;
  }
}

void yuyv2yuv420(int width, int height, const void *src, const void *dst)
{
  int tmp;
  unsigned char *yuyv_img = (unsigned char *)src;
  unsigned char *new_img = (unsigned char *)dst;

  unsigned char y0,u,y1,v;
  unsigned char *yp,*up,*vp;
  int cnt=(width * height)*2;

  yp = new_img;
  up = new_img +( width * height);
  vp = new_img +( width * height) + (width/2)*(height/2);

  for (tmp = 0; tmp < cnt; tmp=tmp+4) {
    y0= yuyv_img[tmp];
    u= yuyv_img[tmp+1]; /* Cr */
    y1= yuyv_img[tmp+2];
    v= yuyv_img[tmp+3]; /* Cb */
    *yp = y0;
    yp++;
    *yp = y1;
	yp++;
	
	if((tmp/2/width)&1){
		*up=u;
		up++;
		*vp=v;
		vp++;
	}
  }

}

#ifdef __cplusplus
}
#endif
