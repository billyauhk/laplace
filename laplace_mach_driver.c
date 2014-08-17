#include "laplace_mach.h"
#include<stdio.h>  // For printf()
#include<unistd.h> // For sleep()

int main(){
  initialize();
  runMachine();
  printf("Main thread decided to kill threads!\n");
  terminate();
}
