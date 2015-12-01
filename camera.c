#ifdef __cplusplus
extern "C"{
#endif

#include "camera.h"

#define CLEAR(x) memset (&(x), 0, sizeof (x))

static void init_vars(int *fd, camera_buffer *cam_bufs,int camera_count){
	int i;
	for (i=0;i<camera_count;i++){
		fd[i]=-1;
		cam_bufs[i].buffers=NULL;
		cam_bufs[i].n_buffers=0;
	}
}

static void errno_exit(const char *file_name, int line_num, const char *s)
{
	fprintf (stderr, "%s error %d in %s, line %d, %s\n",s, errno, file_name, line_num, strerror (errno));
	exit (EXIT_FAILURE);
}

static int xioctl(int fd, int request, void *arg)
{
	int r;
	do r = ioctl (fd, request, arg);
	while (-1 == r && EINTR == errno);
	return r;
}

static void *read_frame(int fd, camera_buffer *cam_buf)
{
	struct v4l2_buffer buf;

	CLEAR (buf);
	buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	buf.memory = V4L2_MEMORY_MMAP;

	if (-1 == xioctl (fd, VIDIOC_DQBUF, &buf)) {
		switch (errno) {
			case EAGAIN:
				return 0;
			case EIO:
				/* Could ignore EIO, see spec. */
				/* fall through */
			default:
				errno_exit (__FILE__,__LINE__,"VIDIOC_DQBUF");
		}
	}

	assert (buf.index < cam_buf->n_buffers);

	if (-1 == xioctl (fd, VIDIOC_QBUF, &buf))
		errno_exit (__FILE__,__LINE__,"VIDIOC_QBUF");

	return cam_buf->buffers[buf.index].start;
}

void capture(int *fd, camera_buffer *cam_bufs, void **img, int camera_count, int isSelect)
{
	int i;
	void *ptr;

	for (i=0;i<camera_count;i++){
		for (;;) {
			fd_set fds;
			struct timeval tv;
			int r;

			FD_ZERO (&fds);
			FD_SET (fd[i], &fds);

			/* Timeout. */
			tv.tv_sec = 0;
			tv.tv_usec = 1000000;

			if (isSelect==1)
			{
				r = select (fd[i] + 1, &fds, NULL, NULL, &tv);

				if (-1 == r) 
				{
     					if (EINTR == errno)
					continue;
					errno_exit (__FILE__,__LINE__,"select");
				}

				if (0 == r) 
				{
					fprintf (stderr, "select timeout\n");
					exit (EXIT_FAILURE);
				}
			}
			
			ptr=read_frame(fd[i],&cam_bufs[i]);
			if (ptr){
				img[i]=ptr;
				break;
			}
			/* EAGAIN - continue select loop. */
		}
	}
}

static void stop_capturing(int fd)
{
	enum v4l2_buf_type type;

	type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	if (-1 == xioctl (fd, VIDIOC_STREAMOFF, &type))
		errno_exit (__FILE__,__LINE__,"VIDIOC_STREAMOFF");
}

static void start_capturing(int fd, camera_buffer *cam_buf)
{
	unsigned int i;
	enum v4l2_buf_type type;

	for (i = 0; i < cam_buf->n_buffers; ++i) {
		struct v4l2_buffer buf;

		CLEAR (buf);

		buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory      = V4L2_MEMORY_MMAP;
		buf.index       = i;

		if (-1 == xioctl (fd, VIDIOC_QBUF, &buf))
			errno_exit (__FILE__,__LINE__,"VIDIOC_QBUF");
	}

	type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	if (-1 == xioctl (fd, VIDIOC_STREAMON, &type))
		errno_exit (__FILE__,__LINE__,"VIDIOC_STREAMON");
}

static void uninit_device(camera_buffer *cam_buf)
{
	unsigned int i;

	for (i = 0; i < cam_buf->n_buffers; ++i)
		if (-1 == munmap (cam_buf->buffers[i].start, cam_buf->buffers[i].length))
			errno_exit (__FILE__,__LINE__,"munmap");

	free (cam_buf->buffers);
}

static void init_mmap(int fd, camera_buffer *cam_buf, const char *dev_name)
{
	struct v4l2_requestbuffers req;

	CLEAR (req);

	req.count               = 4;
	req.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	req.memory              = V4L2_MEMORY_MMAP;

	if (-1 == xioctl (fd, VIDIOC_REQBUFS, &req)) {
		if (EINVAL == errno) {
			fprintf (stderr, "%s does not support "
					"memory mapping\n", dev_name);
			exit (EXIT_FAILURE);
		} else {
			errno_exit (__FILE__,__LINE__,"VIDIOC_REQBUFS");
		}
	}

	if (req.count < 2) {
		fprintf (stderr, "Insufficient buffer memory on %s\n",
				dev_name);
		exit (EXIT_FAILURE);
	}

	cam_buf->buffers = (struct buffer *)calloc(req.count, sizeof (*(cam_buf->buffers)));

	if (!cam_buf->buffers) {
		fprintf (stderr, "Out of memory\n");
		exit (EXIT_FAILURE);
	}

	for (cam_buf->n_buffers = 0; cam_buf->n_buffers < req.count; ++(cam_buf->n_buffers)) {
		struct v4l2_buffer buf;

		CLEAR (buf);

		buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory      = V4L2_MEMORY_MMAP;
		buf.index       = cam_buf->n_buffers;

		if (-1 == xioctl (fd, VIDIOC_QUERYBUF, &buf))
			errno_exit (__FILE__,__LINE__,"VIDIOC_QUERYBUF");

		cam_buf->buffers[cam_buf->n_buffers].length = buf.length;
		cam_buf->buffers[cam_buf->n_buffers].start =
			mmap (NULL /* start anywhere */,
					buf.length,
					PROT_READ | PROT_WRITE /* required */,
					MAP_SHARED /* recommended */,
					fd, buf.m.offset
			);

		if (MAP_FAILED == cam_buf->buffers[cam_buf->n_buffers].start)
			errno_exit (__FILE__,__LINE__,"mmap");
	}
}

static void init_device(int fd, camera_buffer *cam_buf, const char *dev_name)
{
	struct v4l2_capability cap;
	struct v4l2_cropcap cropcap;
	struct v4l2_crop crop;
	struct v4l2_format fmt;
	unsigned int min;

#ifdef WATCH_SUPPORT_FORMAT
	/* watch support format. */
	struct v4l2_fmtdesc fmtdesc;

	fmtdesc.index=0;
	fmtdesc.type=V4L2_BUF_TYPE_VIDEO_CAPTURE;

	printf("Support formats:\n");
	while(ioctl(fd,VIDIOC_ENUM_FMT,&fmtdesc)!=-1)
	{
		printf("\t%d.%s\n",fmtdesc.index+1,fmtdesc.description);
		fmtdesc.index++;
	}
	/* --End-- */
#endif


	if (-1 == xioctl (fd, VIDIOC_QUERYCAP, &cap)) {
		if (EINVAL == errno) {
			fprintf (stderr, "%s is no V4L2 device\n",
					dev_name);
			exit (EXIT_FAILURE);
		} else {
			errno_exit (__FILE__,__LINE__,"VIDIOC_QUERYCAP");
		}
	}

	if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
		fprintf (stderr, "%s is no video capture device\n",
				dev_name);
		exit (EXIT_FAILURE);
	}

	if (!(cap.capabilities & V4L2_CAP_STREAMING)) {
		fprintf (stderr, "%s does not support streaming i/o\n",
				dev_name);
		exit (EXIT_FAILURE);
	}


	/* Select video input, video standard and tune here. */

	CLEAR (cropcap);

	cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	if (0 == xioctl (fd, VIDIOC_CROPCAP, &cropcap)) {
		crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		crop.c = cropcap.defrect; /* reset to default */

		if (-1 == xioctl (fd, VIDIOC_S_CROP, &crop)) {
			switch (errno) {
				case EINVAL:
					/* Cropping not supported. */
					break;
				default:
					/* Errors ignored. */
					break;
			}
		}
	} else {	
		/* Errors ignored. */
	}

	/* Set capture format. */
	CLEAR (fmt);
	fmt.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	fmt.fmt.pix.width       = CAM_WIDTH; /* Camera capture width. */ 
	fmt.fmt.pix.height      = CAM_HEIGHT; /* Camera capture height. */
	fmt.fmt.pix.pixelformat = CAM_FORMAT;
	//fmt.fmt.pix.field       = V4L2_FIELD_INTERLACED;
	fmt.fmt.pix.field       = V4L2_FIELD_ANY;

	if (-1 == xioctl (fd, VIDIOC_S_FMT, &fmt))
		errno_exit (__FILE__,__LINE__,"VIDIOC_S_FMT");
	/* --End-- */
	/* Note VIDIOC_S_FMT may change width and height. */

#ifdef WATCH_CAPTURE_FORMAT

	/* Watch capture width, height, format and size. */
	printf("width:%d height:%d format:%x size=%d\n",
			fmt.fmt.pix.width,
			fmt.fmt.pix.height,
			fmt.fmt.pix.pixelformat,
			fmt.fmt.pix.sizeimage
		  );
	/* --end-- */
#endif

#ifdef WATCH_STREAM_PARAMETER
	/* Watch stream parameter */
	struct v4l2_streamparm parm;
	parm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if (-1 == xioctl (fd, VIDIOC_G_PARM, &parm))
		errno_exit (__FILE__,__LINE__,"VIDIOC_G_PARM");

	printf("capability:%u\n"
			"capturemode:%u\n"
			"frame.numerator:%u\n"
			"frame.denominator%u:\n"
			"extendedmode:%u\n"
			"readbuffers:%u\n",
			parm.parm.capture.capability,
			parm.parm.capture.capturemode,
			parm.parm.capture.timeperframe.numerator,
			parm.parm.capture.timeperframe.denominator,
			parm.parm.capture.extendedmode,
			parm.parm.capture.readbuffers
		  );
	/* --end-- */
#endif

	/* Buggy driver paranoia. */
	min = fmt.fmt.pix.width * 2;
	if (fmt.fmt.pix.bytesperline < min)
		fmt.fmt.pix.bytesperline = min;
	min = fmt.fmt.pix.bytesperline * fmt.fmt.pix.height;
	if (fmt.fmt.pix.sizeimage < min)
		fmt.fmt.pix.sizeimage = min;

	init_mmap(fd,cam_buf,dev_name);
}


static void close_device(int *fdptr)
{
	if (-1 == close (*fdptr))
	    errno_exit (__FILE__,__LINE__,"close");
	*fdptr = -1;
}

static void open_device(int *fdptr, const char *dev_name)
{
    struct stat st;

    if (-1 == stat (dev_name, &st)) {
            fprintf (stderr, "Cannot identify '%s': %d, %s\n",
                    dev_name, errno, strerror (errno));
            exit (EXIT_FAILURE);
    }

    if (!S_ISCHR (st.st_mode)) {
            fprintf (stderr, "%s is no device\n", dev_name);
            exit (EXIT_FAILURE);
    }

    *fdptr = open (dev_name, O_RDWR /* required */ | O_NONBLOCK, 0);

    if (-1 == *fdptr) {
            fprintf (stderr, "Cannot open '%s': %d, %s\n",
                    dev_name, errno, strerror (errno));
            exit (EXIT_FAILURE);
    }
}

void camera_init(int *fd, camera_buffer *camera_buffers, int camera_count){
	int i;
	const char *dev_names[MAX_DEV_COUNT]={"/dev/video0","/dev/video4","/dev/video2","/dev/video3"};
	if(camera_count>MAX_DEV_COUNT){
		fprintf(stderr,"error: camera_count must equal or less than %d\n",MAX_DEV_COUNT);
		exit(EXIT_FAILURE);
	}
	init_vars(fd,camera_buffers,camera_count);
	for (i=0;i<camera_count;i++){
		open_device(&fd[i],dev_names[i]);
		init_device(fd[i],&camera_buffers[i],dev_names[i]);
		start_capturing(fd[i],&camera_buffers[i]);
	}
}

void camera_uninit(int *fd, camera_buffer *camera_buffers, int camera_count){
	int i;
	for (i=0;i<camera_count;i++){
		stop_capturing(fd[i]);
		uninit_device(&camera_buffers[i]);
		close_device(&fd[i]);
	}
}

#ifdef __cplusplus
}
#endif
