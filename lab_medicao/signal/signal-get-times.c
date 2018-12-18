
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
 int send_package = 42;
 struct timespec ts;

void get_my_time(struct timespec * ts){
  clock_gettime(CLOCK_MONOTONIC_COARSE, ts);
}

int server(int sfd, FILE *signal_server_file) {
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
      if (si.ssi_int != send_package) {
        printf("ERROR\n");
        exit(1);
      }
    }
    if (si.ssi_signo == SIGUSR2){
      get_my_time(&ts);
      fprintf(signal_server_file, "%d:%d\n", ts.tv_sec,ts.tv_nsec);
      fflush(signal_server_file);
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

int client(int pid, int sfd, FILE *signal_client_file){
  union sigval value;
  int count = 0;
  value.sival_int = send_package;
  if (sigqueue(pid,SIGUSR1, value) < 0) exit(1); // Manda sinal igual kill, porém, com informação (inteiro ou apontador)
  get_my_time(&ts);
  fprintf(signal_client_file, "%d:%d\n", ts.tv_sec,ts.tv_nsec);
  fflush(signal_client_file);
  while(count < M_SIZE){
    if( waitpid(pid,0,WNOHANG) < 0) break; /* WNOHANG def'd in wait.h */
    value.sival_int = send_package;
    sigqueue(pid,SIGTERM, value); // Manda sinal igual kill, porém, com informação (inteiro ou apontador)
    count++;
//    sleep(1);
  }
  value.sival_int = send_package;
  sigqueue(pid,SIGUSR2, value); // Manda sinal igual kill, porém, com informação (inteiro ou apontador)
}

int main (int argc, char *argv[]) {
  int sfd;
  sigset_t mask;

  FILE *signal_client_file = fopen("signal/output/signal-client", "w");
  FILE *signal_server_file = fopen("signal/output/signal-server", "w");

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
	if(PID == 0) server(sfd, signal_server_file);
	else {
    sleep(1);
    for (int i = 0; i < N_SAMPLE; i++) {
      client(PID,sfd,signal_client_file);
    }
  }
  fclose(signal_server_file);
  fclose(signal_client_file);
  return 0;
}
