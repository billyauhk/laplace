// First trial of the parallel "Laplace machine" (i.e., a VM using MPFR calls as its basic elements)
#include "laplace_mach.h"
#include<mpfr.h>    // mpfr_*()
#include<pthread.h> // pthread_*()
#include<stdio.h>   // For definition of stdout
#include<unistd.h>  // For sleep()
#include<time.h>    // For time()
#include<stdlib.h>  // For random()

// Allocate space for registers renaming
#define REGISTER_COUNT 8
mpfr_t r[REGISTER_COUNT];

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
void initialize(){
  pthread_mutex_init(&mtx_main, NULL);
  pthread_cond_init(&con_main, NULL);
  for(int i=0;i<THREAD_COUNT;i++){
    pthread_mutex_init(mtx_thread+i, NULL);
    pthread_cond_init(con_thread+i, NULL);
  }
  for(int i=0;i<THREAD_COUNT;i++){
    command[i] = 0;
    result[i] = -1;
    pthread_create(thread+i, NULL, &thread_main, (void*) i);
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
  while(command[tid]!=-1){
    if(command[tid]!=0){
      int waitTime = random()%10;
      fprintf(stdout, "[%u] %d/%d Executing command %d: %d sec\n", time(NULL), tid, THREAD_COUNT, command[tid], waitTime);fflush(stdout);
      sleep(waitTime);
    }
    result[tid] = 0;
    // Release the lock before signal master
    fprintf(stdout, "[%u] %d/%d Finished command %d\n", time(NULL), tid, THREAD_COUNT, command[tid]);fflush(stdout);
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
  int i=1;
  while(i<20){
    int issueCommand = 0;
    for(int tid=0;tid<THREAD_COUNT;tid++){
      if(pthread_mutex_trylock(mtx_thread+tid)==0){
        if(result[tid]==0){
          fprintf(stdout,"Main issue %d to %d/%d!\n", i, tid, THREAD_COUNT);
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
  for(int i=0;i<THREAD_COUNT;i++){
    pthread_mutex_lock(mtx_thread+i);
    command[i] = -1;
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
