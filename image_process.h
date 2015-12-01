/********************************************************************************

	YUYV to RGB function
	author:
		pppoe2345 (hty96u@cs.ccu.edu.tw)
	update date: 2011/5/10

********************************************************************************/
#ifndef __IMAGE_PROCESS_H__
#define __IMAGE_PROCESS_H__

#ifdef __cplusplus
extern "C" {
#endif


/* External functions. */
void yuyv2rgb(int width, int height, const void *src, const void *dst);
void yuyv2yuv420(int width, int height, const void *src, const void *dst);

#ifdef __cplusplus
}
#endif

#endif
