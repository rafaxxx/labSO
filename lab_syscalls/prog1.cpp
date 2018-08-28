/*
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char *argv[], char *envp[]){
  int pipefd[2];
  pipe(pipefd);

  if(fork() == 0) {
    dup2(pipefd[1], 1);
    close(pipefd[0]);
    execvp("/bin/ls", argv);
  } else {
    dup2(pipefd[0], 0);
    close(pipefd[1]);
    execvp("/usr/bin/wc", argv);
    //execvp("/bin/ls", argv);
    //wait(0);
  }
}
*/
