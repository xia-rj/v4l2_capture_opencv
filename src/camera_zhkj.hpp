#ifndef CAMERA_ZHKJ_HPP
#define CAMERA_ZHKJ_HPP
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <getopt.h> /* getopt_long() */

#include <fcntl.h> /* low-level i/o */
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

#include <linux/videodev2.h>

#define CLEAR(x) memset(&(x), 0, sizeof(x))

class CameraZhkj
{
public:
    char *dev_name;

    int HEIGHT = 800;
    int WIDTH = 1280;
    int SIZE = HEIGHT *WIDTH * 2;
    
    enum io_method
    {
        IO_METHOD_READ,
        IO_METHOD_MMAP,
        IO_METHOD_USERPTR,
    };

    struct buffer
    {
        void *start;
        size_t length;
    };

    
    enum io_method io = IO_METHOD_MMAP;
    int fd = -1;
    struct buffer *buffers;
    unsigned int n_buffers;
    int force_format;

    void errno_exit(const char *s)
    {
        fprintf(stderr, "%s error %d, %s\n", s, errno, strerror(errno));
        exit(EXIT_FAILURE);
    }

    int xioctl(int fh, unsigned long int request, void *arg)
    {
        int r;

        do
        {
            r = ioctl(fh, request, arg);
        } while (-1 == r && EINTR == errno);

        return r;
    }

    // void process_image(const void *p, int size)
    // {
    //     printf("image size is %d\n", size);
    //     uint8_t yuv422frame[SIZE];
    //     if (size != SIZE)
    //         errno_exit("camera size not match");

    //     memcpy(yuv422frame, p, SIZE);
        
    //     cv::Mat cvmat(HEIGHT, WIDTH, CV_8UC1);
    //     cv::Mat(HEIGHT, WIDTH, CV_16UC1, (void *)yuv422frame).convertTo(cvmat, CV_8UC1, 1.0 / 256);


    //     cv::imshow("Capture", cvmat);

    //     if ((cv::waitKey(1) & 255) == 27)
    //     {
    //         exit(0);
    //     }
    // }

    int read_frame(void *p)
    {
        struct v4l2_buffer buf;
        unsigned int i;

        CLEAR(buf);

        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;

        if (-1 == xioctl(fd, VIDIOC_DQBUF, &buf))
        {
            switch (errno)
            {
            case EAGAIN:
                return 0;

            case EIO:
                /* Could ignore EIO, see spec. */

                /* fall through */

            default:
                errno_exit("VIDIOC_DQBUF");
            }
        }

        assert(buf.index < n_buffers);

        // p = buffers[buf.index].start;
        memcpy(p, buffers[buf.index].start, SIZE);

        // process_image(buffers[buf.index].start, buf.bytesused);

        if (-1 == xioctl(fd, VIDIOC_QBUF, &buf))
            errno_exit("VIDIOC_QBUF");
        
        if (buf.bytesused != SIZE)
            errno_exit("camera size not match");

        return 1;
    }

    int get_frame(void *p)
    {
        fd_set fds;
        struct timeval tv;
        int r;

        FD_ZERO(&fds);
        FD_SET(fd, &fds);

        /* Timeout. */
        tv.tv_sec = 2;
        tv.tv_usec = 100000;

        r = select(fd + 1, &fds, NULL, NULL, &tv);

        if (-1 == r)
        {
            // if (EINTR == errno)
            //     continue;
            errno_exit("select");
        }

        if (0 == r)
        {
            fprintf(stderr, "select timeout\n");
            exit(EXIT_FAILURE);
        }

        read_frame(p);
        /* EAGAIN - continue select loop. */
    }

    void stop_capturing(void)
    {
        enum v4l2_buf_type type;

        type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        if (-1 == xioctl(fd, VIDIOC_STREAMOFF, &type))
            errno_exit("VIDIOC_STREAMOFF");
    }

    void start_capturing(void)
    {
        unsigned int i;
        enum v4l2_buf_type type;

        for (i = 0; i < n_buffers; ++i)
        {
            struct v4l2_buffer buf;

            CLEAR(buf);
            buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            buf.memory = V4L2_MEMORY_MMAP;
            buf.index = i;

            if (-1 == xioctl(fd, VIDIOC_QBUF, &buf))
                errno_exit("VIDIOC_QBUF");
        }
        type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        if (-1 == xioctl(fd, VIDIOC_STREAMON, &type))
            errno_exit("VIDIOC_STREAMON");
    }

    void uninit_device(void)
    {
        unsigned int i;

        for (i = 0; i < n_buffers; ++i)
            if (-1 == munmap(buffers[i].start, buffers[i].length))
                errno_exit("munmap");

        free(buffers);
    }

    void init_mmap(void)
    {
        struct v4l2_requestbuffers req;

        CLEAR(req);

        req.count = 4;
        req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        req.memory = V4L2_MEMORY_MMAP;

        if (-1 == xioctl(fd, VIDIOC_REQBUFS, &req))
        {
            if (EINVAL == errno)
            {
                fprintf(stderr, "%s does not support "
                                "memory mapping\n",
                        dev_name);
                exit(EXIT_FAILURE);
            }
            else
            {
                errno_exit("VIDIOC_REQBUFS");
            }
        }

        if (req.count < 2)
        {
            fprintf(stderr, "Insufficient buffer memory on %s\n",
                    dev_name);
            exit(EXIT_FAILURE);
        }

        buffers = (buffer*)calloc(req.count, sizeof(*buffers));

        if (!buffers)
        {
            fprintf(stderr, "Out of memory\n");
            exit(EXIT_FAILURE);
        }

        for (n_buffers = 0; n_buffers < req.count; ++n_buffers)
        {
            struct v4l2_buffer buf;

            CLEAR(buf);

            buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            buf.memory = V4L2_MEMORY_MMAP;
            buf.index = n_buffers;

            if (-1 == xioctl(fd, VIDIOC_QUERYBUF, &buf))
                errno_exit("VIDIOC_QUERYBUF");

            buffers[n_buffers].length = buf.length;
            buffers[n_buffers].start =
                mmap(NULL /* start anywhere */,
                    buf.length,
                    PROT_READ | PROT_WRITE /* required */,
                    MAP_SHARED /* recommended */,
                    fd, buf.m.offset);

            if (MAP_FAILED == buffers[n_buffers].start)
                errno_exit("mmap");
        }
    }

    void init_device(void)
    {
        struct v4l2_capability cap;
        struct v4l2_cropcap cropcap;
        struct v4l2_crop crop;
        struct v4l2_format fmt;

        if (-1 == xioctl(fd, VIDIOC_QUERYCAP, &cap))
        {
            if (EINVAL == errno)
            {
                fprintf(stderr, "%s is no V4L2 device\n",
                        dev_name);
                exit(EXIT_FAILURE);
            }
            else
            {
                errno_exit("VIDIOC_QUERYCAP");
            }
        }

        if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE))
        {
            fprintf(stderr, "%s is no video capture device\n",
                    dev_name);
            exit(EXIT_FAILURE);
        }

        if (!(cap.capabilities & V4L2_CAP_STREAMING))
        {
            fprintf(stderr, "%s does not support streaming i/o\n",
                    dev_name);
            exit(EXIT_FAILURE);
        }

        /* Select video input, video standard and tune here. */

        CLEAR(cropcap);

        cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

        if (0 == xioctl(fd, VIDIOC_CROPCAP, &cropcap))
        {
            crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            crop.c = cropcap.defrect; /* reset to default */

            if (-1 == xioctl(fd, VIDIOC_S_CROP, &crop))
            {
                switch (errno)
                {
                case EINVAL:
                    /* Cropping not supported. */
                    break;
                default:
                    /* Errors ignored. */
                    break;
                }
            }
        }
        else
        {
            /* Errors ignored. */
        }

        CLEAR(fmt);

        fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

        /* Preserve original settings as set by v4l2-ctl for example */
        if (-1 == xioctl(fd, VIDIOC_G_FMT, &fmt))
            errno_exit("VIDIOC_G_FMT");

        init_mmap();
    }

    void close_device(void)
    {
        if (-1 == close(fd))
            errno_exit("close");

        fd = -1;
    }

    void open_device(void)
    {
        struct stat st;

        if (-1 == stat(dev_name, &st))
        {
            fprintf(stderr, "Cannot identify '%s': %d, %s\n",
                    dev_name, errno, strerror(errno));
            exit(EXIT_FAILURE);
        }

        if (!S_ISCHR(st.st_mode))
        {
            fprintf(stderr, "%s is no device\n", dev_name);
            exit(EXIT_FAILURE);
        }

        fd = open(dev_name, O_RDWR /* required */ | O_NONBLOCK, 0);

        if (-1 == fd)
        {
            fprintf(stderr, "Cannot open '%s': %d, %s\n",
                    dev_name, errno, strerror(errno));
            exit(EXIT_FAILURE);
        }
    }

    CameraZhkj(char *dev_name_)
    {
        dev_name = dev_name_;
        open_device();
        init_device();
        start_capturing();
        // mainloop();
    }

    ~CameraZhkj()
    {
        stop_capturing();
        uninit_device();
        close_device();
    }
};
#endif //CAMERA_ZHKJ_HPP