// First trial of the parallel "Laplace machine" (i.e., a VM using MPFR calls as its basic elements)
#include "laplace_mach.h"
#include<pthread.h> // pthread_*()
#include<stdio.h>   // For definition of stdout
#include<unistd.h>  // For sleep()
#include<time.h>    // For time()
#include<stdlib.h>  // For random()

// Allocate space for registers renaming
#define REGISTER_COUNT 8
// We suspect register count is the upper bound for the window size
// Which out-of-order executing more than WINDOW_SIZE ahead will be unsafe (may exhause reorder "registers")
#define WINDOW_SIZE REGISTER_COUNT
mpfr_t r[REGISTER_COUNT]; // The register buffer
int rc[REGISTER_COUNT];   // The producing instruction of the content

// Data for main thread
pthread_mutex_t mtx_main;
pthread_cond_t con_main;
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
void initialize(int precision){
  mpfr_set_default_prec(precision);
  for(int i=0;i<REGISTER_COUNT;i++){
    mpfr_init(r[i]);
  }
  pthread_mutex_init(&mtx_main, NULL);
  pthread_cond_init(&con_main, NULL);
  for(int tid=0;tid<THREAD_COUNT;tid++){
    pthread_mutex_init(mtx_thread+tid, NULL);
    pthread_cond_init(con_thread+tid, NULL);
  }
  for(int tid=0;tid<THREAD_COUNT;tid++){
    command[tid] = -1;
    result[tid] = -1;
    pthread_create(thread+tid, NULL, &thread_main, (void*) tid);
    pthread_mutex_lock(&mtx_main);
    pthread_cond_wait(&con_main, &mtx_main);
    pthread_mutex_unlock(&mtx_main);
  }
  fprintf(stdout,"Initialize finished\n");fflush(stdout);
}

// Thread entry point
void* thread_main(void *value){
  int tid = (int) value;
  fprintf(stdout, "%d/%d created\n", tid, THREAD_COUNT);fflush(stdout);

  // A worker thread always have its own mutex when it is on
  pthread_mutex_lock(mtx_thread+tid);
  while(command[tid]!=-2){
    if(command[tid]!=-1){
      fprintf(stdout, "[%u] %d/%d Executing exe[%d]: status=%u, type=%u, ptr=%p, refC=%d, currC=%d, func=%p, dest=%d, src1=%d, src2=%d, rndMode=%d\n",
                      time(NULL), tid, THREAD_COUNT, command[tid], \
                      exe[tid].status, exe[tid].type, exe[tid].ptr, exe[tid].refCount, exe[tid].currCount, \
                      exe[tid].func, exe[tid].dest, exe[tid].src1, exe[tid].src2, exe[tid].rndMode);
      fflush(stdout);
      // TODO: The internal actual executing part
    }
    result[tid] = 0;
    // Release the lock before signal master
    fprintf(stdout, "[%u] %d/%d Finished exe[%d]\n", time(NULL), tid, THREAD_COUNT, command[tid]);fflush(stdout);
    pthread_mutex_unlock(mtx_thread+tid);
    // Signal to master thread
    pthread_mutex_lock(&mtx_main);
    pthread_cond_signal(&con_main);
    pthread_mutex_unlock(&mtx_main);
    // Wait for a new command
    pthread_mutex_lock(mtx_thread+tid);
    pthread_cond_wait(con_thread+tid, mtx_thread+tid);
  }
  fprintf(stdout, "[%u] %d/%d terminating\n", time(NULL), tid, THREAD_COUNT);fflush(stdout);
  pthread_mutex_unlock(mtx_thread+tid);
  return NULL;
}

void runMachine(){
  // The Tomasulo algorithm will be placed here
  fprintf(stderr,"I got %u commands per loop\n", commandCount);

  int i=0;
  while(i<commandCount){
    int issueCommand = 0;
    for(int tid=0;tid<THREAD_COUNT;tid++){
      if(pthread_mutex_trylock(mtx_thread+tid)==0){
        if(result[tid]==0){
          fprintf(stdout,"[%u] Main issue %d to %d/%d!\n", time(NULL), i, tid, THREAD_COUNT);
          issueCommand++;
          command[tid] = i;
          result[tid] = -1;
          pthread_cond_signal(con_thread+tid);
        }
        pthread_mutex_unlock(mtx_thread+tid);
      }
      if(issueCommand>0)break;
    }
    if(issueCommand==0){
      // Failed to issue any command, wait until sbd wakes me up
      pthread_mutex_lock(&mtx_main);
      fprintf(stdout,"[%u] Main sleeping\n", time(NULL));
      pthread_cond_wait(&con_main,&mtx_main);
      pthread_mutex_unlock(&mtx_main);
      fprintf(stdout,"[%u] Main awake\n", time(NULL));
      continue;
    }
    i++;
  }
}

// Function for destroy of the threads
void terminate(){
  for(int tid=0;tid<THREAD_COUNT;tid++){
    pthread_mutex_lock(mtx_thread+tid);
    command[tid] = -2;
    result[tid] = -1;
    pthread_cond_signal(con_thread+tid);
    pthread_mutex_unlock(mtx_thread+tid);
  }
  fprintf(stdout,"[%u] Waiting for join\n",time(NULL));fflush(stdout);
  for(int tid=0;tid<THREAD_COUNT;tid++){
    if(pthread_join(thread[tid], NULL)!=0){
      fprintf(stderr,"pthread_join() of %d\%d have an error!\n",tid,THREAD_COUNT);fflush(stderr);
    }
  }
  // Clean up
  for(int tid=0;tid<THREAD_COUNT;tid++){
    pthread_cond_destroy(con_thread+tid);
  }
}

void resetScoreboard(){
  // Reset the scoreboard
}
