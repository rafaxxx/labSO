
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

int M_SIZE;
int N_SAMPLE;

struct time_count {
  int nano_begin;
  int sec_begin;
  int nano_end;
  int sec_end;
  double diff;
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
  double diff_sec =  abs(t->sec_end  - t->sec_begin);
  double diff_nano = abs(t->nano_end - t->nano_begin);
  diff_nano /= 1000000000; //Deixando em fracao
  t->diff = diff_sec + diff_nano;
}

double count_write(char * shm_ptr){
  struct time_count t;
  struct timespec ts;
  start_count(&t, &ts);
  for (int n = 0; n < M_SIZE; n++){
    shm_ptr[n] = 'a';
  }
  stop_count(&t, &ts);
  return t.diff;
}

double count_read(char * shm_ptr){
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

double count_write_array(char array[M_SIZE]){
  struct time_count t;
  struct timespec ts;
  start_count(&t, &ts);
  for (int n = 0; n < M_SIZE; n++){
    array[n] = 'a';
  }
  stop_count(&t, &ts);
  return t.diff;
}

double count_read_array(char array[M_SIZE]){
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
  key = ftok((char *) "shared-memory/shm.c", 66);
  return shmget(key, M_SIZE, 0777 | IPC_CREAT ); // Cria a memoria compartilhada
}

char * attach(int shmid) {
  return (char *) shmat(shmid, NULL, 0); // Anexa a memoria compartilhada e retorna enderço de memoria
}

int main (int argc, char *argv[]) {
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
  int shmid = create_shm();
  char * array = (char *) malloc(M_SIZE);
  char * shm_ptr = attach(shmid);
  FILE *shm_file_read = fopen("./shared-memory/output/shm-read", "w");
  FILE *shm_file_write = fopen("./shared-memory/output/shm-write", "w");
  FILE *array_file_read = fopen("./shared-memory/output/array-read", "w");
  FILE *array_file_write = fopen("./shared-memory/output/array-write", "w");



  if(fork() == 0){
    for (int i = 0; i < N_SAMPLE; i++){
      double result = count_write(shm_ptr);
      fprintf(shm_file_write, "%.9f\n", result);
      fflush(shm_file_write);
    }


    for (int i = 0; i < N_SAMPLE; i++){
      double result = count_read(shm_ptr);
      fprintf(shm_file_read, "%.9f\n", result);
      fflush(shm_file_read);
    }
  }else{
    for (int i = 0; i < N_SAMPLE; i++){
      double result = count_write_array(array);
      fprintf(array_file_write, "%.9f\n", result);
      fflush(array_file_write);
    }

    for (int i = 0; i < N_SAMPLE; i++){
      double result = count_read_array(array);
      fprintf(array_file_read, "%.9f\n", result);
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
