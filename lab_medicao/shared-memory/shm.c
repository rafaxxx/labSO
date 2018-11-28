
/*
 * https://linux.die.net/man/2/shmget
 * http://www.csl.mtu.edu/cs4411.ck/www/NOTES/process/shm/example-2.html
 * https://linux.die.net/man/2/shmat
 * https://www.geeksforgeeks.org/ipc-shared-memory/
 */

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>

int read_shm(int * shm_ptr){
  return (int) shm_ptr[0];
}

void write_shm(int * shm_ptr, int data){
  shm_ptr[0] = data;
}

int server(int * shm_ptr) {
  while (1) {
    int data = read_shm(shm_ptr);
    if( data == 5 ) break;
    printf("Data read in shared memory (index 0): %d\n",data);
    sleep(2);
  }
  return 0;
}

int client(int pid, int * shm_ptr){
  int count = 0;
  while(1){
    int check = waitpid(pid,0,WNOHANG); /* WNOHANG def'd in wait.h */
    if (check < 0) break;

    write_shm(shm_ptr, count++);
    sleep(2);
  }
  return 0;
}


int main (int argc, char *argv[]) {
  printf("My process ID : %d\n", getpid());
  /*
     A função ftok() usa o nome do arquivo apontado por path, que é único no sistema,
     como uma cadeia de caracteres, e o combina com um identificador proj para gerar
     uma chave do tipo key_t no sistema IPC.
  */
  key_t key = ftok((char *) "shm.c", 66);
  int shmid = shmget(key, 4097, 0777 | IPC_CREAT ); // Cria a memoria compartilhada
  int *shm_ptr = (int *) shmat(shmid, NULL, 0); // Anexa a memoria compartilhada e retorna enderço de memoria

  int PID=fork();
  if(PID == 0) server(shm_ptr);
  else client(PID,shm_ptr);

  shmdt(shm_ptr); // Desanexar memoria
  shmctl(shmid,IPC_RMID,NULL); // Remover memoria
  return 0;
}
