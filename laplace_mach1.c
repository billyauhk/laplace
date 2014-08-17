// First trial of the parallel "Laplace machine" (i.e., a VM using MPFR calls as its basic elements)
#include "laplace_mach.h"
#include<mpfr.h>    // mpfr_*()
#include<pthread.h> // pthread_*()
#include<stdio.h>   // For definition of stdout
#include<unistd.h>  // For sleep()

// Allocate space for registers renaming
#define REGISTER_COUNT 8
mpfr_t r[REGISTER_COUNT];

// Allocate space for thread management
#define THREAD_COUNT 4
pthread_t thread[THREAD_COUNT];
pthread_mutex_t mtx_thread[THREAD_COUNT];
pthread_cond_t con_thread[THREAD_COUNT];
int command[THREAD_COUNT];
int result[THREAD_COUNT];

// Laplace machine compiled array type declaration in laplace_mach.h
// Commands to be executed by the Laplace machine will be prepared as a compiled array

// ===== Below this line is the actual implementations ====

// Function for creation of the threads
void initialize(){
  for(int i=0;i<THREAD_COUNT;i++){
    pthread_mutex_init(mtx_thread+i, NULL);
    pthread_cond_init(con_thread+i, NULL);
  }
  for(int i=0;i<THREAD_COUNT;i++){
    command[i] = 0;
    result[i] = -1;
    pthread_create(thread+i, NULL, &thread_main, (void*) i);
  }
}

// Thread entry point
void* thread_main(void *value){
  int tid = (int) value;
  fprintf(stdout, "%d/%d created\n", tid, THREAD_COUNT);fflush(stdout);
  while(command[tid]!=-1){
    fprintf(stdout, "%d/%d Executing command %d\n", tid, THREAD_COUNT, command[tid]);fflush(stdout);
    sleep(1);
    result[tid] = 0;
    pthread_mutex_lock(mtx_thread+tid);
    pthread_cond_wait(con_thread+tid, mtx_thread+tid);
  }
  fprintf(stdout, "%d/%d terminating\n", tid, THREAD_COUNT);fflush(stdout);
  pthread_mutex_unlock(mtx_thread+tid);
  return NULL;
}

void runMachine(){
  // The Tomasulo algorithm will be placed here

}

// Function for destroy of the threads
void terminate(){
  for(int i=0;i<THREAD_COUNT;i++){
    command[i] = -1;
    pthread_mutex_lock(mtx_thread+i);
    pthread_cond_signal(con_thread+i);
    pthread_mutex_unlock(mtx_thread+i);
  }
  fprintf(stdout,"Waiting for join\n");fflush(stdout);
  for(int i=0;i<THREAD_COUNT;i++){
    if(pthread_join(thread[i], NULL)!=0){
      fprintf(stderr,"pthread_join() have an error!\n");fflush(stderr);
    }
  }
  // Clean up
  for(int i=0;i<THREAD_COUNT;i++){
    pthread_cond_destroy(con_thread+i);
  }
}

void resetScoreboard(){
  // Reset the scoreboard
}
