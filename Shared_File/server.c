#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
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
  
  printf("%d:P[%02d:%02d:%02d:%ld][Server](Server has been started)\n", getpid(), hour, min ,sec,mill );


  struct flock lock;
  lock.l_type = F_WRLCK;    /* read/write (exclusive) lock */
  lock.l_whence = SEEK_SET; /* base for seek offsets */
  lock.l_start = 0;         /* 1st byte in file */
  lock.l_len = 0;           /* 0 here means 'until EOF' */
  lock.l_pid = getpid();    /* process id */
  
  int fd; /* file descriptor to identify a file within a process */
  if ((fd = open(FileName, O_RDONLY | O_TRUNC | O_CREAT, 0666)) < 0)  /* -1 signals an error */
    report_and_exit("open to read failed...");

  /* If the file is write-locked, we can't continue. */
  fcntl(fd, F_GETLK, &lock); /* sets lock.l_type to F_UNLCK if no write lock */
  if (lock.l_type != F_UNLCK)
    report_and_exit("file is still write locked...");

  lock.l_type = F_RDLCK; /* prevents any writing during the reading */
  if (fcntl(fd, F_SETLK, &lock) < 0)
    report_and_exit("can't get a read-only lock...");
  while(1){
    int c; /* buffer for read bytes */
    lock.l_type = F_RDLCK;
    int flag = 1;
    while (read(fd, &c, 1) > 0){
      if(flag){
         // time variables////////////////
        seconds = time(NULL);
        timeStruct = localtime(&seconds);
        gettimeofday(&timeSec, NULL);
        hour = timeStruct->tm_hour;
        min = timeStruct->tm_min;
        sec = timeStruct->tm_sec;
        mill = timeSec.tv_usec/1000;
        ////////////////////////////////
        fprintf(stderr, "%d:P[%02d:%02d:%02d:%ld][Server] Received : ", lock.l_pid, hour, min, sec, mill );
        fflush(stdout);
        flag = 0;
      }
      write(STDOUT_FILENO, &c, 1); /* write one byte to the standard output */

    }  
    /* Release the lock explicitly. */
    lock.l_type = F_UNLCK;
    sleep(2); // sleep due to the fact that give chance to clients
  }
 
  if (fcntl(fd, F_SETLK, &lock) < 0)
    report_and_exit("explicit unlocking failed...");
  
  close(fd); 
  return 0;  
}
