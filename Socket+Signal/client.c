#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include "sock.h"
#include <semaphore.h>
#include <sys/time.h>
#include <time.h>
#include <fcntl.h>          
#include <unistd.h>
#include <signal.h>

#define AccessPerms 0644

void report(const char* msg, int terminate) {
  perror(msg);
  if (terminate) exit(-1); /* failure */
}

void termination(int signum) {

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

  printf("%d:C[%02d:%02d:%02d:%ld] (Client about to terminate)\n", getpid(), hour, min ,sec,mill );
  sleep(1);
  _exit(0); 
}

int main() {
  
  //signal
  struct sigaction current;
  sigemptyset(&current.sa_mask);         /* clear the signal set */
  current.sa_flags = 0;                  /* enables setting sa_handler, not sa_action */
  current.sa_handler = termination;         /* specify a handler */
  sigaction(SIGTERM, &current, NULL);    /* register the handler */

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
  
  sem_t* semptrWrite = sem_open("SemaphoreWriter", /* name */
          O_CREAT,       /* create the semaphore */
          AccessPerms,   /* protection perms */
          1);            /* initial value */
  sem_post(semptrWrite);


  // time variables////////////////
  seconds = time(NULL);
  timeStruct = localtime(&seconds);
  gettimeofday(&timeSec, NULL);
  sec = timeStruct->tm_sec;
  mill = timeSec.tv_usec/1000;
  ////////////////////////////////
  printf("%d:C[%02d:%02d:%02d:%ld][%s](Connect to the server, Ready to Write)\n", getpid(), hour, min, sec, mill ,name);

  while(1){
  
  int sockfd = socket(AF_INET,  /* versus AF_LOCAL */
		      SOCK_STREAM,          /* reliable, bidirectional */
		      0);                   /* system picks protocol (TCP) */
  if (sockfd < 0) report("socket", 1); /* terminate */

  /* get the address of the host */
  struct hostent* hptr = gethostbyname(Host); /* localhost: 127.0.0.1 */ 
  if (!hptr) report("gethostbyname", 1); /* is hptr NULL? */
  if (hptr->h_addrtype != AF_INET)       /* versus AF_LOCAL */
    report("bad address family", 1);
  
  /* connect to the server: configure server's address 1st */
  struct sockaddr_in saddr;
  memset(&saddr, 0, sizeof(saddr));
  saddr.sin_family = AF_INET;
  saddr.sin_addr.s_addr = 
     ((struct in_addr*) hptr->h_addr_list[0])->s_addr;
  saddr.sin_port = htons(PortNumber); /* port number in big-endian */
  
  if (connect(sockfd, (struct sockaddr*) &saddr, sizeof(saddr)) < 0)
    report("connect", 1);

   // Message sending part
  if(!sem_wait(semptrWrite)){
    char message[50];
    //scanf("%s", message);
    fgets(message, sizeof(message), stdin);
    char pid[60];
    sprintf(pid, "%d-", getpid());
    strcat(pid,message);

   
    if (write(sockfd, pid, strlen(pid)) > 0) {
      char buffer[BuffSize + 1];
      memset(buffer, '\0', sizeof(buffer));
      if (read(sockfd, buffer, sizeof(buffer)) > 0){
        //puts(message);
        ///////// time variables////////
        seconds = time(NULL);
        timeStruct = localtime(&seconds);
        gettimeofday(&timeSec, NULL);
        hour = timeStruct->tm_hour;
        min = timeStruct->tm_min;
        sec = timeStruct->tm_sec;
        mill = timeSec.tv_usec/1000;
        ///////////////////////////////
        fprintf(stderr, "%d:C[%02d:%02d:%02d:%ld][%s](send a message)\n", getpid(), hour, min, sec, mill ,name);
      }
    }
    sem_post(semptrWrite);
  }
  close(sockfd); 
  } // while end
  return 0;
}


