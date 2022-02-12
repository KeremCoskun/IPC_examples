/** Compilation: gcc -o memreader memreader.c -lrt -lpthread **/
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>       
#include <fcntl.h>          
#include <unistd.h>
#include <semaphore.h>
#include <string.h>
#include "shmem.h"
#include <sys/time.h>
#include <time.h>

void report_and_exit(const char* msg) {
  perror(msg);
  exit(-1);
}

int main() {
  int fd = shm_open(BackingFile, O_RDWR | O_CREAT , AccessPerms);  /* empty to begin */
  if (fd < 0) report_and_exit("Can't get file descriptor...");

  ftruncate(fd, ByteSize); /* get the bytes */
  /* get a pointer to memory */
  caddr_t memptr = mmap(NULL,       /* let system pick where to put segment */
			ByteSize,   /* how many bytes */
			PROT_READ | PROT_WRITE, /* access protections */
			MAP_SHARED, /* mapping visible to other processes */
			fd,         /* file descriptor */
			0);         /* offset: start at 1st byte */
  if ((caddr_t) -1 == memptr) report_and_exit("Can't access segment...");

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
  
  char name[15]= "";
  printf("%d:P[%02d:%02d:%02d:%ld][Server](Server has been started, Ready to listen)\n", getpid(), hour, min ,sec,mill);

  // Write
  sem_t* semptrWrite = sem_open(SemaphoreWriter, /* name */
			   O_CREAT,       /* create the semaphore */
			   AccessPerms,   /* protection perms */
			   1);            /* initial value */

  // Reader
  sem_t* semptr = sem_open(SemaphoreReader, /* name */
			   O_CREAT,       /* create the semaphore */
			   AccessPerms,   /* protection perms */
			   0);            /* initial value */

    
  sem_post(semptrWrite);
   if (semptr == (void*) -1) report_and_exit("sem_open");
    /* use semaphore as a mutex (lock) by waiting for writer to increment it */
    while(1){
        if (!sem_wait(semptr)) { /* wait until semaphore != 0 */
         // time variables////////////////
          seconds = time(NULL);
          timeStruct = localtime(&seconds);
          gettimeofday(&timeSec, NULL); 
          hour = timeStruct->tm_hour;
          min = timeStruct->tm_min;
          sec = timeStruct->tm_sec;
          mill = timeSec.tv_usec/1000;
          ////////////////////////////////
          printf("%d:P[%02d:%02d:%02d:%ld][Server] Received : ", getpid(), hour, min, sec, mill );
          fflush( stdout );
          int i=0;
          for (i = 0; i < 50; i++){
              write(STDOUT_FILENO, memptr + i, 1); /* one byte at a time */
              if(memptr[i]== '\n'){
                 break;
              }
          }

        sem_post(semptrWrite);
        } 
    }
  
  /* cleanup */
  munmap(memptr, ByteSize);
  close(fd);
  sem_close(semptr);
  sem_close(semptrWrite);
  unlink(BackingFile);    
  
  return 0;
}
