#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include "sock.h"
#include <sys/time.h>
#include <time.h>
#include <fcntl.h>          
#include <unistd.h>
#include <semaphore.h>

#define AccessPerms 0644

void report(const char* msg, int terminate) {
  perror(msg);
  if (terminate) exit(-1); /* failure */
}

int main() {
  int fd = socket(AF_INET,     /* network versus AF_LOCAL */
		  SOCK_STREAM , /* reliable, bidirectional: TCP */
		  0);          /* system picks underlying protocol */
  if (fd < 0) report("socket", 1); /* terminate */
  	
  /* bind the server's local address in memory */
  struct sockaddr_in saddr;
  memset(&saddr, 0, sizeof(saddr));          /* clear the bytes */
  saddr.sin_family = AF_INET;                /* versus AF_LOCAL */
  saddr.sin_addr.s_addr = htonl(INADDR_ANY); /* host-to-network endian */
  saddr.sin_port = htons(PortNumber);        /* for listening */
  
  if (bind(fd, (struct sockaddr *) &saddr, sizeof(saddr)) < 0)
    report("bind", 1); /* terminate */
	 
  /* listen to the socket */
  if (listen(fd, MaxConnects) < 0) /* listen for clients, up to MaxConnects */
    report("listen", 1); /* terminate */

  // time variables////////////////
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
  //////////////////////////////////
  fprintf(stderr,"%d:P[%02d:%02d:%02d:%ld][Parent](Listening on port %i for clients...)\n", getpid(), hour, min, sec, mill ,PortNumber);
  
  // sem_t* semptrWrite = sem_open("SemaphoreWriter", /* name */
  //          O_CREAT,       /* create the semaphore */
  //          AccessPerms,   /* protection perms */
  //        1);            /* initial value */

  // a server traditionally listens indefinitely */

  //sem_post(semptrWrite); 
  while (1) {
    struct sockaddr_in caddr; /* client address */
    int len = sizeof(caddr);  /* address length could change */
    
    int client_fd = accept(fd, (struct sockaddr*) &caddr, &len);  /* accept blocks */
    if (client_fd < 0) {
      report("accept", 0); /* don't terminated, though there's a problem */
      continue;
    }

    /* read from client */
    char buffer[BuffSize + 1];
    memset(buffer, '\0', sizeof(buffer)); 
    int count = read(client_fd, buffer, sizeof(buffer));
   
    if (count > 0) {  
         ///////// time variables////////
        seconds = time(NULL);
        timeStruct = localtime(&seconds);
        gettimeofday(&timeSec, NULL);
        hour = timeStruct->tm_hour;
        min = timeStruct->tm_min;
        sec = timeStruct->tm_sec;
        mill = timeSec.tv_usec/1000;
        ///////////////////////////////
        fprintf(stderr, "%d:P[%02d:%02d:%02d:%ld][Server] Received: ", getpid(), hour, min, sec, mill);
        puts(buffer);
        write(client_fd, buffer, sizeof(buffer));        
    }

    close(client_fd); /* break connection */
  }

  return 0;
}

   