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
  const char* pipeName = "./fifoChannel";
  mkfifo(pipeName, 0666);                      /* read/write for user/group/others */
  int fd = open(pipeName, O_WRONLY); /* open as write-only */
  if (fd < 0) return -1;                       /** error **/

  // Semaphore for Writing operation
  sem_t* semptrWrite = sem_open("SemaphoreWriter", /* name */
        O_CREAT,       /* create the semaphore */
        AccessPerms,   /* protection perms */
        0);            /* initial value */

  // Semaphore for Writing operation
  sem_t* semptrRead = sem_open("SemaphoreReader", /* name */
        O_CREAT,       /* create the semaphore */
        AccessPerms,   /* protection perms */
        0);            /* initial value */      

  
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

  // print  
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
  printf("%d:C[%02d:%02d:%02d:%ld][%s](client name set to %s)\n", getpid(), hour, min, sec, mill ,name, name);
 
  while(1){
    char message[50];
    //scanf("%s", message); 
    fgets(message, sizeof(message), stdin);
    if(!sem_wait(semptrRead)){
        write(fd, message, sizeof(message)); 
    }
    // time variables//////////////
    seconds = time(NULL);
    timeStruct = localtime(&seconds);
    gettimeofday(&timeSec, NULL);
    hour = timeStruct->tm_hour;
    min = timeStruct->tm_min;
    sec = timeStruct->tm_sec;
    mill = timeSec.tv_usec/1000;
    //////////////////////////////
    fprintf(stderr, "%d:C[%02d:%02d:%02d:%ld][%s](send a message)\n", getpid(), hour, min, sec, mill ,name);

    sem_post(semptrWrite);
  }

  sem_close(semptrWrite);
  sem_close(semptrRead);
  close(fd);                                /* close pipe: generates an end-of-file */
  unlink(pipeName);                         /* unlink from the implementing file */

  return 0;
}
