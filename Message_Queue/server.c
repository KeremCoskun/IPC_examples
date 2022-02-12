#include <stdio.h> 
#include <sys/ipc.h> 
#include <sys/msg.h>
#include <stdlib.h>
#include "queue.h"
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

void report_and_exit(const char* msg) {
  perror(msg);
  exit(-1); /* EXIT_FAILURE */
}
  
int main() { 

  key_t key= ftok(PathName, ProjectId); /* key to identify the queue */
  if (key < 0) report_and_exit("key not gotten...");
  
  int qid = msgget(key, 0666 | IPC_CREAT); /* access if created already */
  if (qid < 0) report_and_exit("no access to queue...");
  
  // time variables
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

  fprintf(stderr, "%d:P[%02d:%02d:%02d:%ld][Server](server has been started) \n", getpid(), hour, min, sec, mill);
  

  while(1) { // infinite loop  for reading messages 
    queuedMessage msg; /* defined in queue.h */
    if (msgrcv(qid, &msg, sizeof(msg), 1, MSG_NOERROR ) < 0)
      puts("msgrcv trouble...");

   
    seconds = time(NULL);
    timeStruct = localtime(&seconds);
    gettimeofday(&timeSec, NULL);

    hour = timeStruct->tm_hour;
    min = timeStruct->tm_min;
    sec = timeStruct->tm_sec;
    mill = timeSec.tv_usec/1000; 

    fprintf(stderr, "%d:P[%02d:%02d:%02d:%ld][Server] Message Arrived : %s", getpid(), hour, min, sec, mill , msg.payload);
       
  }

  /** remove the queue **/
  if (msgctl(qid, IPC_RMID, NULL) < 0)  /* NULL = 'no flags' */
    report_and_exit("trouble removing queue...");
  
  return 0; 
} 
