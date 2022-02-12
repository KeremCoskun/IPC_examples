#include <stdio.h> 
#include <sys/ipc.h> 
#include <sys/msg.h>
#include <stdlib.h>
#include <string.h>
#include "queue.h"
#include <sys/time.h>
#include <time.h>
#include <semaphore.h>
#include <fcntl.h>   
#include <unistd.h>

#define AccessPerms 0644

void report_and_exit(const char* msg) {
  perror(msg);
  exit(-1); /* EXIT_FAILURE */
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


  // Semaphore for Writing operation
  sem_t* semptrWrite = sem_open("SemaphoreWriter", /* name */
        O_CREAT,       /* create the semaphore */
        AccessPerms,   /* protection perms */
        1);            /* initial value */

  key_t key = ftok(PathName, ProjectId); 
  if (key < 0) report_and_exit("couldn't get key...");
  
  int qid = msgget(key, 0666 | IPC_CREAT); 
  if (qid < 0) report_and_exit("couldn't get queue id...");

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
 
  sem_post(semptrWrite);
  while (1) {
    queuedMessage msg;
    msg.type = 1;
    char text[50];
    //scanf("%s", text);
    fgets(text, sizeof(text), stdin);
    strcpy(msg.payload, text);
    if(!sem_wait(semptrWrite)){
      msgsnd(qid, &msg, sizeof(msg), IPC_NOWAIT); /* don't block */
      // time variables////////////////
      seconds = time(NULL);
      timeStruct = localtime(&seconds);
      gettimeofday(&timeSec, NULL);
      hour = timeStruct->tm_hour;
      min = timeStruct->tm_min;
      sec = timeStruct->tm_sec;
      mill = timeSec.tv_usec/1000;
      ////////////////////////////////
      fprintf(stderr, "%d:C[%02d:%02d:%02d:%ld][%s](send a message)\n", getpid(), hour, min, sec, mill ,name);
      
      sem_post(semptrWrite);
    }
   
  }
  return 0;
}

