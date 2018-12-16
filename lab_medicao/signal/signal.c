
/* Example of use of a Linux-specific call - signalfd() to handle signals using
 * a file descriptor. - http://www.linuxprogrammingblog.com/code-examples/signalfd
 */

 #include <unistd.h>
 #include <stdio.h>
 #include <stdlib.h>
 #include <signal.h>
 #include <sys/signalfd.h>
 #include <sys/wait.h>
 #include <time.h>
 #include <string.h>

 int N_SAMPLE;
 int M_SIZE;

void get_my_time(struct timespec * ts){
  clock_gettime(CLOCK_MONOTONIC_COARSE, ts);
}

int merge_time(struct timespec * ts){
  get_my_time(ts);
  int x = ts->tv_sec;
  int y= ts->tv_nsec;
  char xx[20], yy[20];
  sprintf(xx, "%d", x);
  sprintf(yy, "%d", y);
  for (int i = 0; i < strlen(xx) - 3; i++)
      xx[i] = ' ';
  for (int i = strlen(yy) - 1; i >= strlen(yy) - 3; i--)
    yy[i] = ' ';

  int len = strlen(xx);
  if (yy[5] == ' ') yy[5] = '0';
  x = atoi(xx);
  sprintf(xx, "%d", x);
  strcat(xx,yy);
  return atoi(xx);
}

int server(int sfd, FILE *signal_file) {
  static int begin;
  static int end;
  static int count = 0;
  struct signalfd_siginfo si;
  ssize_t res;
  while (1) {
     /** The buffor for read(), this structure contains information
     * about the signal we've read. */

    res = read (sfd, &si, sizeof(si));

    if (si.ssi_signo == SIGINT) exit(0);
    if (si.ssi_signo == SIGUSR1){
      begin = si.ssi_int;
    }
    if (si.ssi_signo == SIGUSR2){
      end = si.ssi_int;
      fprintf(signal_file, "%f\n", (end - begin)/1000000.);
      fflush(signal_file);
    }
    if (si.ssi_signo == SIGTERM){
      count++;
    }
  }
  //sleep(10);
  /* Close the file descriptor if we no longer need it. */
  close (sfd);
  return 0;
}

int client(int pid, int sfd){
  union sigval value;
  struct timespec ts;
  int count = 0;
  sleep(1);

  value.sival_int = merge_time(&ts);
  sigqueue(pid,SIGUSR1, value); // Manda sinal igual kill, porém, com informação (inteiro ou apontador)
  while(count < M_SIZE){
    if( waitpid(pid,0,WNOHANG) < 0) break; /* WNOHANG def'd in wait.h */
    value.sival_int = merge_time(&ts);
    sigqueue(pid,SIGTERM, value); // Manda sinal igual kill, porém, com informação (inteiro ou apontador)
    count++;
//    sleep(1);
  }
  value.sival_int = merge_time(&ts);
  sigqueue(pid,SIGUSR2, value); // Manda sinal igual kill, porém, com informação (inteiro ou apontador)
}

int main (int argc, char *argv[]) {
  int sfd;
  sigset_t mask;

  FILE *signal_file = fopen("signal/output/signal", "w");

  N_SAMPLE = atoi(argv[2]);
  switch (atoi(argv[1])) {
    case 1:
      M_SIZE = 1048576;
      break;
    case 128:
      M_SIZE = 134217728;
      break;
    case 256:
      M_SIZE = 268435456;
      break;
    case 512:
      M_SIZE = 536870912;
      break;
    default:
      return 0;
  }

  /* We will handle SIGTERM and SIGINT. */
  sigemptyset (&mask);
  sigaddset (&mask, SIGINT);
  sigaddset (&mask, SIGTERM);
  sigaddset (&mask, SIGUSR1);
  sigaddset (&mask, SIGUSR2);

  /* Block the signals thet we handle using signalfd(), so they don't
   * cause signal handlers or default signal actions to execute. */
  sigprocmask(SIG_BLOCK, &mask, NULL);

  /* Create a file descriptor from which we will read the signals. */
  sfd = signalfd (-1, &mask, 0);

	int PID=fork();
	if(PID == 0) server(sfd, signal_file);
	else {
    for (int i = 0; i < N_SAMPLE; i++) {
      client(PID,sfd);
    }
  }
  fclose(signal_file);
  return 0;
}
