#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char *argv[], char *envp[]){
  int pipefd[2];
  pipe(pipefd);

  dup2(0, pipefd[0]);
  if(fork() == 0) {
    dup2(1, pipefd[1]);
    execvp("/bin/ps", argv);
  } else {
    execvp("/usr/bin/wc", argv);
    //wait(0);
  }
}
