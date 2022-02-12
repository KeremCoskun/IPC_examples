#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>   
#include <unistd.h>
#include <sys/time.h>
#include <semaphore.h>

#define AccessPerms 0644

int main() {
 
  sem_unlink("SemaphoreWriter");
  sem_unlink("SemaphoreReader");
  const char* file = "./fifoChannel";
  mkfifo(file, 0666);   
  int fd = open(file, O_RDWR | O_CREAT ); 
  if (fd < 0) return -1; /* no point in continuing */
 
  // Semaphore for Writing operation
  sem_t* semptrWrite = sem_open("SemaphoreWriter", /* name */
        O_CREAT,       /* create the semaphore */
        AccessPerms,   /* protection perms */
        0);            /* initial value */

  // Semaphore for Writing operation
  sem_t* semptrRead = sem_open("SemaphoreReader", /* name */
        O_CREAT,       /* create the semaphore */
        AccessPerms,   /* protection perms */
        1);            /* initial value */      

  time_t seconds;
  struct tm *timeStruct;
  seconds = time(NULL);
  timeStruct = localtime(&seconds);
 
  struct timeval timeSec;
  gettimeofday(&timeSec, NULL);

  int hour = timeStruct->tm_hour;
  int min = timeStruct->tm_min;
  int sec = timeStruct->tm_sec;
  long mill = timeSec.tv_usec/1000;

  printf("%d:P[%02d:%02d:%02d:%ld][Server](Server has been started)\n", getpid(), hour, min ,sec,mill);



  while (1) {  
    
    if(!sem_wait(semptrWrite)){ 
        char next[50];
        read(fd, &next, sizeof(next));   
        //printf("%s", next);

        seconds = time(NULL);
        timeStruct = localtime(&seconds);
        gettimeofday(&timeSec, NULL);
        hour = timeStruct->tm_hour;
        min = timeStruct->tm_min;
        sec = timeStruct->tm_sec;
        mill = timeSec.tv_usec/1000;
        printf("%d:C[%02d:%02d:%02d:%ld][Server] Message : %s ", getpid(), hour, min, sec, mill ,next);
      
    }
    sem_post(semptrRead);

  }

  close(fd);       /* close pipe from read end */
  unlink(file);    /* unlink from the underlying file */

  return 0;
}
