#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/stat.h>

int main(int argc, char *argv[])
{
  int fd          = 0;
  int counter     = 0;
  int old_counter = 0;
  
  /* open /dev/second */
  fd = open("/dev/second", O_RDONLY);
  if (fd != -1) 
  {
      while (1) 
	  {
          read(fd, &counter, sizeof(unsigned int)); /* 读目前经历的秒数 */
          if (counter != old_counter) 
		  {	
      	      printf("seconds after open /dev/second :%d\n", counter);
      	      old_counter = counter;
          }	

		  // sleep(10);
      }    
  } 
  else 
  {
      printf("Device open failure\n");
  }

  exit(0);
}
