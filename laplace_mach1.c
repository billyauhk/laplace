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
    rc[i] = -1;
  }
  pthread_mutex_init(&mtx_main, NULL);
  pthread_cond_init(&con_main, NULL);
  for(int tid=0;tid<THREAD_COUNT;tid++){
    pthread_mutex_init(mtx_thread+tid, NULL);
    pthread_cond_init(con_thread+tid, NULL);
  }
  for(int tid=0;tid<THREAD_COUNT;tid++){
    pthread_mutex_lock(mtx_thread+tid);
    command[tid] = -1;
    result[tid] = -1;
    pthread_mutex_unlock(mtx_thread+tid);
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
  int cid = command[tid];
  pthread_mutex_unlock(mtx_thread+tid);
  while(cid!=-2){
    if(cid!=-1){ // If it is not the suicide command (-2)
      fprintf(stdout, "[%u] %d/%d Executing exe[%d]: status=%u, type=%u, ptr=%p, refC=%d, currC=%d, func=%p, dest=%d, src1=%d, src2=%d, rndMode=%d\n",
                      time(NULL), tid, THREAD_COUNT, cid, \
                      exe[cid].status, exe[cid].type, exe[cid].ptr, exe[cid].refCount, exe[cid].currCount, \
                      exe[cid].func, exe[cid].dest, exe[cid].src1, exe[cid].src2, exe[cid].rndMode);
      fflush(stdout);
      // TODO: The internal actual execution
      switch(exe[cid].type){
        case ONE_SRC:
            ((int (*)(mpfr_t,mpfr_t,mpfr_rnd_t)) exe[cid].func)(r[exe[cid].dest], r[exe[exe[cid].src1].dest], exe[cid].rndMode);
            exe[exe[cid].src1].currCount--;
          break;
        case TWO_SRC:
            ((int (*)(mpfr_t,mpfr_t,mpfr_t,mpfr_rnd_t)) exe[cid].func)(r[exe[cid].dest], r[exe[exe[cid].src1].dest], r[exe[exe[cid].src2].dest], exe[cid].rndMode);
            exe[exe[cid].src1].currCount--;
            exe[exe[cid].src2].currCount--;
          break;
        case SRC_UI:
            ((int (*)(mpfr_t,mpfr_t,unsigned long int,mpfr_rnd_t)) exe[cid].func)(r[exe[cid].dest], r[exe[exe[cid].src1].dest], exe[cid].src2, exe[cid].rndMode);
            exe[exe[cid].src1].currCount--;
          break;
        case UI_SRC:
            ((int (*)(mpfr_t,unsigned long int,mpfr_t,mpfr_rnd_t)) exe[cid].func)(r[exe[cid].dest], exe[cid].src1, r[exe[exe[cid].src2].dest], exe[cid].rndMode);
            exe[exe[cid].src2].currCount--;
          break;
        case SRC_SI:
            ((int (*)(mpfr_t,mpfr_t,long int,mpfr_rnd_t)) exe[cid].func)(r[exe[cid].dest], r[exe[exe[cid].src1].dest], exe[cid].src2, exe[cid].rndMode);
            exe[exe[cid].src1].currCount--;
          break;
        case SI_SRC:
            ((int (*)(mpfr_t,long int,mpfr_t,mpfr_rnd_t)) exe[cid].func)(r[exe[cid].dest], exe[cid].src1, r[exe[exe[cid].src2].dest], exe[cid].rndMode);
            exe[exe[cid].src2].currCount--;
          break;
        case PRINT:
          unsigned int digits;
          char* buffer;
          digits = mpfr_get_default_prec();
          buffer = (char*) malloc(sizeof(char)*(digits+10));
          mpfr_snprintf(buffer,digits+10,"%.*Rf",digits,r+exe[cid].src1);
          printf("%s%s\n",(unsigned char*) exe[cid].ptr,buffer);
          free(buffer);
          break;
        case ASSIGN:
            /* Do nothing but checking, as this is a virtual instruction */
            if(exe[cid].status!=REF_COUNT){
              fprintf(stderr,"Value of exe[%d] remains un-assigned\n");fflush(stderr);
            }
          break;
        default:
            fprintf(stderr,"UNDEFINED exe[cid].type!\n");fflush(stderr);
          break;
      }
    }
    // Set all the status
    exe[cid].currCount = exe[cid].refCount;
    exe[cid].status = REF_COUNT;

    pthread_mutex_lock(mtx_thread+tid);
    result[tid] = 0;
    // Release the lock before signal master
    fprintf(stdout, "[%u] %d/%d Finished exe[%d]\n", time(NULL), tid, THREAD_COUNT, cid);fflush(stdout);
    pthread_mutex_unlock(mtx_thread+tid);
    // Signal to master thread
    pthread_mutex_lock(&mtx_main);
    pthread_cond_signal(&con_main);
    pthread_mutex_unlock(&mtx_main);
    // Wait for a new command
    pthread_mutex_lock(mtx_thread+tid);
    pthread_cond_wait(con_thread+tid, mtx_thread+tid);
    // Read the new command
    cid = command[tid];
    pthread_mutex_unlock(mtx_thread+tid);
  }
  fprintf(stdout, "[%u] %d/%d terminating\n", time(NULL), tid, THREAD_COUNT);fflush(stdout);
  pthread_mutex_unlock(mtx_thread+tid);
  return NULL;
}

// Assign a value into the VM registers
void assign(int cid, mpfr_t* src){
  // Check whether exe[cid] is of type ASSIGN
  if(cid >= commandCount || exe[cid].type!=ASSIGN){
    fprintf(stderr,"cid=%d is not a ASSIGN instruction!\n", cid);fflush(stderr);
    return;
  }
  // Allocate a register for the value
  int alloc_reg = -1;
  for(int i=0;i<REGISTER_COUNT;i++){
    if(rc[i]==-1){
      alloc_reg = i;break;
    }
  }
  // Copy data into the register
  if(alloc_reg!=-1){
    mpfr_set(r[alloc_reg],*src,exe[cid].rndMode);
    // TODO: Set all the other status in a consistent way
    exe[cid].status = REF_COUNT;
    exe[cid].dest = alloc_reg;
    exe[cid].currCount = exe[cid].refCount;
  }else{
    fprintf(stderr,"Failed to allocate register to the ASSIGN stmt %d\n", cid);fflush(stderr);
    return;
  }
}

void runMachine(){
  // The Tomasulo algorithm will be placed here
  fprintf(stderr,"I got %u commands per loop\n", commandCount);

  int i=0;
  while(i<commandCount){
    int issueCommand = -1;
    int couldIssue = 0;
    int alloc_reg;

    // Scan for a register which is available
    for(alloc_reg=0;alloc_reg<REGISTER_COUNT;alloc_reg++){
      if(rc[alloc_reg]==-1 || exe[rc[alloc_reg]].currCount==0){
        if(exe[rc[alloc_reg]].currCount==0 && exe[rc[alloc_reg]].status==REF_COUNT && rc[alloc_reg]>i){
          rc[alloc_reg] = REF_DEL;
        }
        rc[alloc_reg]=-1;
        break;
      }
    }

    // Scan for a command which is available for execution
    if(rc[alloc_reg]==-1){
      // (Conditionally) upgrade AHEAD_WIN to IN_WIN
      for(int j=i;j<i+WINDOW_SIZE && j<commandCount;j++){
        if(exe[j].status==AHEAD_WIN)exe[j].status=IN_WIN;
      }
      // (Conditionally) upgrade IN_WIN to SRC_READY
      for(int j=i;j<i+WINDOW_SIZE && j<commandCount;j++){
        if(exe[j].status==IN_WIN){
          switch(exe[j].type){
            case ONE_SRC:
                if(exe[exe[j].src1].status==REF_COUNT && exe[exe[j].src1].dest!=-1)exe[j].status=SRC_READY;break;
            case TWO_SRC:
                if(exe[exe[j].src1].status==REF_COUNT && exe[exe[j].src1].dest!=-1 \
                && exe[exe[j].src2].status==REF_COUNT && exe[exe[j].src2].dest!=-1)exe[j].status=SRC_READY;break;
            case SRC_UI:
                if(exe[exe[j].src1].status==REF_COUNT && exe[exe[j].src1].dest!=-1)exe[j].status=SRC_READY;break;
            case UI_SRC:
                if(exe[exe[j].src2].status==REF_COUNT && exe[exe[j].src2].dest!=-1)exe[j].status=SRC_READY;break;
            case SRC_SI:
                if(exe[exe[j].src1].status==REF_COUNT && exe[exe[j].src1].dest!=-1)exe[j].status=SRC_READY;break;
            case SI_SRC:
                if(exe[exe[j].src2].status==REF_COUNT && exe[exe[j].src2].dest!=-1)exe[j].status=SRC_READY;break;
            case PRINT:
                if(exe[exe[j].src1].status==REF_COUNT && exe[exe[j].src1].dest!=-1)exe[j].status=SRC_READY;break;
            case ASSIGN:
                /* Do nothing as this is a virtual instruction */
              break;
            default:
                fprintf(stderr,"UNDEFINED exe[cid].type!\n");fflush(stderr);
              break;
          }
        }
      }
      // (Conditionally) select instruction with SRC_READY
      for(int j=i;j<i+WINDOW_SIZE && j<commandCount;j++){
        if(exe[j].status==SRC_READY){
          fprintf(stdout,"Selected command %d\n",j);fflush(stdout);
          couldIssue++;
          issueCommand=j;
          exe[issueCommand].dest = alloc_reg;
          rc[alloc_reg] = j;
          break;
        }
      }
    }else{
      fprintf(stderr,"No register available!\n");fflush(stderr);
    }

    // Scan for a thread which is available
    if(couldIssue>0){
      for(int tid=0;tid<THREAD_COUNT;tid++){
        if(pthread_mutex_trylock(mtx_thread+tid)==0){
          if(result[tid]==0){
            fprintf(stdout,"[%u] Main issue %d to %d/%d!\n", time(NULL), issueCommand, tid, THREAD_COUNT);
            command[tid] = issueCommand;
            result[tid] = -1;
            exe[issueCommand].status=IN_CALC;
            pthread_cond_signal(con_thread+tid);
          }
          pthread_mutex_unlock(mtx_thread+tid);
        }
        if(couldIssue--)break;
      }
    }else{
      fprintf(stderr,"No command could be issued\n");fflush(stderr);
    }

    for(int j=i;j<i+WINDOW_SIZE && j<commandCount;j++){
      if(exe[j].status==REF_DEL || exe[j].status==BEHIND_WIN){
        exe[j].status=BEHIND_WIN;
        i=j;
      }else{
        break;
      }
    }
    if(exe[commandCount-1].status==BEHIND_WIN){
      // Now all commands are run
      return;
    }

    // If failed to issue any command, wait until sbd wakes me up
    if(issueCommand==-1){
      pthread_mutex_lock(&mtx_main);
      fprintf(stdout,"[%u] Main sleeping\n", time(NULL));
      pthread_cond_wait(&con_main,&mtx_main);
      pthread_mutex_unlock(&mtx_main);
      fprintf(stdout,"[%u] Main awake\n", time(NULL));
      continue;
    }
    sleep(1); // TODO: Remove this and add the printScoreBoard here
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
