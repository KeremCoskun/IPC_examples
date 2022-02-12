#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>

#define FileName "shared_file.txt" 

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

  struct flock lock;
  //lock.l_type = F_WRLCK;    /* read/write (exclusive versus shared) lock */
  lock.l_whence = SEEK_SET; /* base for seek offsets */
  lock.l_start = 0;         /* 1st byte in file */
  lock.l_len = 0;           /* 0 here means 'until EOF' */
  lock.l_pid = getpid();    /* process id */
  
  int fd; /* file descriptor to identify a file within a process */
  if ((fd = open(FileName, O_RDWR | O_APPEND | O_CREAT, 0666)) < 0)  /* -1 signals an error */
    report_and_exit("open failed...");
  
  if (fcntl(fd, F_SETLK, &lock) < 0) /** F_SETLK doesn't block, F_SETLKW does **/
    report_and_exit("fcntl failed to get lock...");
  else {
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
    ////////////////////////////////
    printf("%d:C[%02d:%02d:%02d:%ld][%s](client name set to %s)\n", lock.l_pid, hour, min, sec, mill ,name, name);
    while(1){
      char message[50];
     //scanf(" %s", message);
      fgets(message, sizeof(message), stdin);
      lock.l_type = F_WRLCK;
     //strcat(message,"\n");
      write(fd, message, strlen(message));

      // time variables////////////////
      seconds = time(NULL);
      timeStruct = localtime(&seconds);
      gettimeofday(&timeSec, NULL);
      hour = timeStruct->tm_hour;
      min = timeStruct->tm_min;
      sec = timeStruct->tm_sec;
      mill = timeSec.tv_usec/1000;
      ////////////////////////////////
      fprintf(stderr, "%d:C[%02d:%02d:%02d:%ld][%s](send a message)\n", lock.l_pid, hour, min, sec, mill ,name);
      lock.l_type = F_UNLCK;
    }
  }

  /* Now release the lock explicitly. */
  lock.l_type = F_UNLCK;
  if (fcntl(fd, F_SETLK, &lock) < 0)
    report_and_exit("explicit unlocking failed...");
  
  close(fd); /* close the file: would unlock if needed */
  return 0;  /* terminating the process would unlock as well */
}

