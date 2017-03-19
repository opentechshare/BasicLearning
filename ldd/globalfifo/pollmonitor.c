/* poll test */
#include <asm/ioctl.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <linux/fcntl.h>
#include <stdio.h>

#define GLOBALFIFO_MAGIC 'G'
#define FIFO_CLEAR _IO(GLOBALFIFO_MAGIC, 0)  /* clear global fifo cmd word */

int main(int argc, char **argv)
{
    int    fd;
    fd_set rfds;
    fd_set wfds;

    fd = open("/dev/globalfifo", O_RDONLY | O_NONBLOCK);
    if (-1 != fd)
    {
        if (ioctl(fd, FIFO_CLEAR, 0) < 0)
        {
            printf("ioctl command failed!\n");
        }

        while (1)
        {
            FD_ZERO(&rfds);
            FD_ZERO(&wfds);
            FD_SET(fd, &rfds);
            FD_SET(fd, &wfds);

            select(fd + 1, &rfds, &wfds, NULL, NULL);

            if (FD_ISSET(fd, &rfds))
            {
                printf("Poll monitor: can be read.\n");
                sleep(10);
            }

            if (FD_ISSET(fd, &wfds))
            {
                printf("POLL monitor: can be write.\n");
                sleep(10);
            }
        }
    }
    else
    {
        printf("Device open failure!\n");
    }
    
    return 0;
}

