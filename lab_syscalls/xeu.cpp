#include "xeu_utils/StreamParser.h"

#include <iostream>
#include <vector>
#include <cstdio>
#include <sstream>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

using namespace xeu_utils;
using namespace std;

#define READ_END 0
#define WRITE_END 1

// This function is just to help you learn the useful methods from Command
void io_explanation(const Command& command) {
  // Let's use this input as example: (ps aux >out >o2 <in 2>er >ou)
  // we would print "$       io(): [0] >out [1] >o2 [2] <in [3] 2>er [4] >ou"
  cout << "$       io():";
  for (size_t i = 0; i < command.io().size(); i++) {
    IOFile io = command.io()[i];
    cout << " [" << i << "] " << io.repr();
    // Other methods - if we had 2>file, then:
    // io.fd() == 2
    // io.is_input() == false ('>' is output)
    // io.is_output() == true
    // io.path() == "file"
  }
}

// This function is just to help you learn the useful methods from Command
void command_explanation(const Command& command) {

  /* Methods that return strings (useful for debugging & maybe other stuff) */

  // This prints the command in a format that can be run by our xeu. If you
  // run the printed command, you will get the exact same Command
  {
    // Note: command.repr(false) doesn't show io redirection (i.e. <in >out)
    cout << "$     repr(): " << command.repr() << endl;
    cout << "$    repr(0): " << command.repr(false) << endl;
    // cout << "$   (string): " << string(command) << endl; // does the same
    // cout << "$ operator<<: " << command << endl; // does the same
  }

  // This is just args()[0] (e.g. in (ps aux), name() is "ps")
  {
    cout << "$     name(): " << command.name() << endl;
  }

  // Notice that args[0] is the command/filename
  {
    cout << "$     args():";
    for (int i = 0; i < command.args().size(); i++) {
      cout << " [" << i << "] " << command.args()[i];
    }
    cout << endl;
  }

  /* Methods that return C-string (useful in exec* syscalls) */

  // this is just the argv()[0] (same as name(), but in C-string)
  {
    printf("$ filename(): %s\n", command.filename());
  }

  // This is similar to args, but in the format required by exec* syscalls
  // After the last arg, there is always a NULL pointer (as required by exec*)
  {
    printf("$     argv():");
    for (int i = 0; command.argv()[i]; i++) {
      printf(" [%d] %s", i, command.argv()[i]);
    }
    printf("\n");
  }

  io_explanation(command);
}

// This function is just to help you learn the useful methods from Command
void commands_explanation(const vector<Command>& commands) {
  // Shows a representation (repr) of the command you input
  // cout << "$ Command::repr(0): " << Command::repr(commands, false) << endl;
  cout << "$ Command::repr(): " << Command::repr(commands) << endl << endl;

  // Shows details of each command
  for (int i = 0; i < commands.size(); i++) {
    cout << "# Command " << i << endl;
    command_explanation(commands[i]);
    cout << endl;
  }
}

int * createPipe(){
  int *fd = new int[2];
  pipe(fd);
  return fd;
}

int execCommand (Command c) {
  if(execvp(c.filename(), c.argv()) == -1) {
    printf("Esse comando nao existe\n");
  }
  return 0;
}

int pipeline(vector<Command> commands, int fd[2], int i){
  if (i == commands.size() - 1)
    fd[WRITE_END] = STDOUT_FILENO;

  if(i == 0){
    dup2(fd[WRITE_END],STDOUT_FILENO);
    //execvp(commands[i].filename(), commands[i].argv());
    execCommand(commands[i]);
  }else{
    int *pipeAux = createPipe();
    if(fork() == 0){
      pipeline(commands, pipeAux, i - 1);
    }else{
      dup2(pipeAux[READ_END],STDIN_FILENO);
      dup2(fd[WRITE_END],STDOUT_FILENO);
      close(pipeAux[WRITE_END]);
      //execvp(commands[i].filename(), commands[i].argv());
      execCommand(commands[i]);
    }
  }
}


int main(int argc, char *argv[], char *envp[]) {
  while(true) {
    vector<Command> commands;
    ParsingState p;
    cout << "\033[1;32m[xeu - pid:" << getpid() << "]$\033[0m ";
    p = StreamParser().parse(); // AQUI LER
    commands = p.commands();

    if(commands.size() != 0){
      if(fork() == 0){
        pipeline(commands, createPipe(), commands.size() - 1);
      }else{
        wait(0);
      }
    }

  }
  return 0;
}
