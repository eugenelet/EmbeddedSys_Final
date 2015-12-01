/********************************************************************************

	Camera capturing functions
	update date: 2011/4/1

********************************************************************************/
#ifndef __CAMERA_H__
#define __CAMERA_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <fcntl.h>              /* low-level i/o */
#include <unistd.h>
#include <errno.h>
#include <malloc.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

#include <asm/types.h>          /* for videodev2.h */

#include <linux/videodev2.h>

#define CAM_WIDTH 320
#define CAM_HEIGHT 240
#define CAM_FORMAT V4L2_PIX_FMT_YUYV
#define MAX_DEV_COUNT 4
/* #define WATCH_SUPPORT_FORMAT  */
/* #define WATCH_CAPTURE_FORMAT */
/* #define WATCH_STREAM_PARAMETER */

struct buffer {
	void *start;
	size_t length;
};

typedef struct camera_buffer {
	struct buffer *buffers;
	unsigned int n_buffers;
}camera_buffer;

/* External functions. */
void camera_init(int *fd, camera_buffer *camera_buffers, int camera_count);
void camera_uninit(int *fd, camera_buffer *camera_buffers, int camera_count);
void capture(int *fd, camera_buffer *cam_bufs, void **img, int camera_count, int isSelect);

#ifdef __cplusplus
}
#endif

#endif
