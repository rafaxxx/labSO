
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
#include <time.h>

int const FLAG_SERVER = 1;
int const FLAG_CLIENT = 0;
int const INDEX_FLAG = 0;
int const INDEX_DATA_SERVER = 1;
int const INDEX_DATA_CLIENT = 2;

int read_shm(int * shm_ptr, int index){
  return (int) shm_ptr[index];
}

void write_shm(int * shm_ptr, int index, int data){
  shm_ptr[index] = data;
}

int server(int * shm_ptr) {
  srand(time(NULL));   // Initialization, should only be called once.

  int sleep_time = 2;
  int previous_data = 0;
  int sleep_count = 0;
  while (1) {
    int valor = read_shm(shm_ptr, INDEX_DATA_CLIENT);

    if (valor != previous_data){
      printf("[Server] - Recebi o valor %d do cliente\n",valor);
      previous_data = valor;

      if (sleep_count >= 5){
        int sleep_time = rand() % 10;
        write_shm(shm_ptr,INDEX_FLAG,FLAG_SERVER);
        write_shm(shm_ptr,INDEX_DATA_SERVER, sleep_time);
        printf("[Server] - Vou dormir por %d segundos\n",sleep_time + 1); // +1 por causa do ultimo sleep
        sleep(sleep_time);
        sleep_count = 0;
        write_shm(shm_ptr,INDEX_FLAG,FLAG_CLIENT);
      }
      sleep_count++;
    }
    sleep(1);
  }
  return 0;
}

int client(int pid, int * shm_ptr){
  int count = 0;
  while(1){
    int check = waitpid(pid,0,WNOHANG); /* WNOHANG def'd in wait.h */
    if (check < 0) break;

    if(read_shm(shm_ptr, INDEX_FLAG) == FLAG_CLIENT){
      printf("[Cliente] - Enviando %d... \n", count);
      write_shm(shm_ptr, INDEX_DATA_CLIENT, count++);
    }else{
      int sleep_time = read_shm(shm_ptr, INDEX_DATA_SERVER);
      sleep(sleep_time);
    }

    sleep(1);
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

  write_shm(shm_ptr,INDEX_FLAG,0);
  write_shm(shm_ptr,INDEX_DATA_CLIENT,0);
  write_shm(shm_ptr,INDEX_DATA_SERVER,0);

  int PID=fork();
  if(PID == 0) server(shm_ptr);
  else client(PID,shm_ptr);

  shmdt(shm_ptr); // Desanexar memoria
  shmctl(shmid,IPC_RMID,NULL); // Remover memoria
  return 0;
}
