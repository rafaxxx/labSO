
/*
 * https://linux.die.net/man/2/shmget
 * http://www.csl.mtu.edu/cs4411.ck/www/NOTES/process/shm/example-2.html
 * https://linux.die.net/man/2/shmat
 * https://www.geeksforgeeks.org/ipc-shared-memory/
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <math.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <time.h>

int const M_SIZE = 536870912; //512MB
int const N_SAMPLE = 10000;

struct time_count {
  int nano_begin;
  int sec_begin;
  int nano_end;
  int sec_end;
  int diff;
};

void start_count(struct time_count * t, struct timespec * ts){
  clock_gettime(CLOCK_MONOTONIC_COARSE, ts);
  t->sec_begin = ts->tv_sec;
  t->nano_begin = ts->tv_nsec;
}

void stop_count(struct time_count * t, struct timespec * ts){
  clock_gettime(CLOCK_MONOTONIC_COARSE, ts);
  t->sec_end = ts->tv_sec;
  t->nano_end = ts->tv_nsec;
  int diff_sec =  t->sec_end  - t->sec_begin;
  int diff_nano = t->nano_end - t->nano_begin;
  diff_sec *= 1000000000; //Convertendo para nano
  t->diff = abs(diff_sec + diff_nano);
}

int count_write(char * shm_ptr){
  struct time_count t;
  struct timespec ts;
  start_count(&t, &ts);
  for (int n = 0; n < M_SIZE; n++){
    shm_ptr[n] = 'a';
  }
  stop_count(&t, &ts);
  return t.diff;
}

int count_read(char * shm_ptr){
  struct time_count t;
  struct timespec ts;
  start_count(&t, &ts);
  for (int n = 0; n < M_SIZE; n++){
    if (shm_ptr[n] != 'a'){
      printf("ERROR");
      break;
    }
  }
  stop_count(&t, &ts);
  return t.diff;
}

int count_write_array(char array[M_SIZE]){
  struct time_count t;
  struct timespec ts;
  start_count(&t, &ts);
  for (int n = 0; n < M_SIZE; n++){
    array[n] = 'a';
  }
  stop_count(&t, &ts);
  return t.diff;
}

int count_read_array(char array[M_SIZE]){
  struct time_count t;
  struct timespec ts;
  start_count(&t, &ts);
  for (int n = 0; n < M_SIZE; n++){
    if(array[n] == 'a'){
      continue;
    }
    printf("ERROR");
    break;
  }
  stop_count(&t, &ts);
  return t.diff;
}

int create_shm(){
  key_t key;
  key = ftok((char *) "shm.c", 68);
  return shmget(key, M_SIZE, 0777 | IPC_CREAT ); // Cria a memoria compartilhada
}

char * attach(int shmid) {
  return (char *) shmat(shmid, NULL, 0); // Anexa a memoria compartilhada e retorna enderço de memoria
}

int main (int argc, char *argv[]) {
  int shmid = create_shm();
  char * array = (char *) malloc(M_SIZE);
  char * shm_ptr = attach(shmid);
  FILE *shm_file_read = fopen("./output/shm-read", "w");
  FILE *shm_file_write = fopen("./output/shm-write", "w");
  FILE *array_file_read = fopen("./output/array-read", "w");
  FILE *array_file_write = fopen("./output/array-write", "w");

  if(fork() == 0){
    for (int i = 0; i < N_SAMPLE; i++){
      int result = count_write(shm_ptr);
      fprintf(shm_file_write, "%d\n", result);
      fflush(shm_file_write);
    }


    for (int i = 0; i < N_SAMPLE; i++){
      int result = count_read(shm_ptr);
      fprintf(shm_file_read, "%d\n", result);
      fflush(shm_file_read);
    }
  }else{
    for (int i = 0; i < N_SAMPLE; i++){
      int result = count_write_array(array);
      fprintf(array_file_write, "%d\n", result);
      fflush(array_file_write);
    }

    for (int i = 0; i < N_SAMPLE; i++){
      int result = count_read_array(array);
      fprintf(array_file_read, "%d\n", result);
      fflush(array_file_read);
    }
  }

  fclose(shm_file_read);
  fclose(shm_file_write);
  fclose(array_file_write);
  fclose(array_file_read);
  shmdt(shm_ptr); // Desanexar memoria
  shmctl(shmid,IPC_RMID,NULL); // Remover memoria
  return 0;
}
