#include "laplace_mach.h"
#include<stdio.h>  // For printf()
#include<unistd.h> // For sleep()

int main(){
  initialize();
  printf("Main thread is awake!\n");
  sleep(1);
  printf("Main thread decided to kill threads!\n");
  terminate();
}
