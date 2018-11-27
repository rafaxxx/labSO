
/*
 * https://linux.die.net/man/2/shmget
 * http://www.csl.mtu.edu/cs4411.ck/www/NOTES/process/shm/example-2.html
 * https://linux.die.net/man/2/shmat
 * https://www.geeksforgeeks.org/ipc-shared-memory/
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>


int main (int argc, char *argv[]) {
  /* A função ftok() usa o nome do arquivo apontado por path, que é único no sistema,
     como uma cadeia de caracteres, e o combina com um identificador proj para gerar
     uma chave do tipo key_t no sistema IPC.
  */
  key_t key = ftok((char *) "shm.c", 66);
  int shmid = shmget(key, 4097, 0777 | IPC_CREAT ); // Cria a memoria compartilhada

  int *shm_ptr = (int *) shmat(shmid, NULL, 0); // Anexa a memoria compartilhada
  printf("Data written in memory: %d\n",shm_ptr);
  shmdt(shm_ptr); // Desanexa a memoria

	/*
   TO-DO
   int PID=fork();
	 if(PID == 0) server(sfd);
	 else client(PID,sfd);
  */

  shmctl(shmid,IPC_RMID,NULL); // Remover memoria
  return 0;
}
