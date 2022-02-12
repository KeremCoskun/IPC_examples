/** Compilation: gcc -o memwriter memwriter.c -lrt -lpthread **/
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
  printf("%d:C[%02d:%02d:%02d:%ld][%s](client has been started)\n", getpid(), hour, min ,sec,mill ,name);


  int fd = shm_open(BackingFile, /* name from smem.h */
		    O_RDWR , //write permis
		    AccessPerms);     /* access permissions (0644) */
  if (fd < 0) report_and_exit("Can't open shared mem segment...");

 // ftruncate(fd, ByteSize); /* get the bytes */

  caddr_t memptr = mmap(NULL,       /* let system pick where to put segment */
			ByteSize,   /* how many bytes */
			PROT_READ | PROT_WRITE, /* access protections */
			MAP_SHARED, /* mapping visible to other processes */
			fd,         /* file descriptor */
			0);         /* offset: start at 1st byte */
  if ((caddr_t) -1  == memptr) report_and_exit("Can't get segment...");

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

  if (semptr == (void*) -1) report_and_exit("sem_open");

   printf("Please write your name\n");
    scanf("%s", name);
    getchar();
    // time variables////////////////
    seconds = time(NULL);
    timeStruct = localtime(&seconds);
    gettimeofday(&timeSec, NULL);
    hour = timeStruct->tm_hour;
    min = timeStruct->tm_min;
    sec = timeStruct->tm_sec;
    mill = timeSec.tv_usec/1000;
    ////////////////////////////////
    printf("%d:C[%02d:%02d:%02d:%ld][%s](client name set to %s)\n", getpid(), hour, min, sec, mill ,name, name);

  while(1){

      
    if(!sem_wait(semptrWrite)){
       char text[50];
      //scanf("%s", text);
      fgets(text, sizeof(text), stdin);
      
      strcpy(memptr, text); /* copy some ASCII bytes to the segment */
     
       // time variables////////////////
      seconds = time(NULL);
      timeStruct = localtime(&seconds);
      gettimeofday(&timeSec, NULL);
      hour = timeStruct->tm_hour;
      min = timeStruct->tm_min;
      sec = timeStruct->tm_sec; 
      mill = timeSec.tv_usec/1000;
      ////////////////////////////////
      printf("%d:C[%02d:%02d:%02d:%ld][%s](send a message)\n", getpid(), hour, min, sec, mill ,name);

      /* increment the semaphore so that memreader can read */
      if (sem_post(semptr) < 0) report_and_exit("sem_post"); 
    }
  
  }
  /* clean up */
  munmap(memptr, ByteSize); /* unmap the storage */
  close(fd);
  sem_close(semptr);
  sem_close(semptrWrite);
  shm_unlink(BackingFile); /* unlink from the backing file */
 
  return 0;
}


