#include "laplace_mach.h"
#include<stdio.h>  // For printf()
#include<unistd.h> // For sleep()

int main(){
  mpfr_t s;
  initialize(100);
  mpfr_init_set_d(s,2.0,MPFR_RNDN);
  assign(0, &s);
  runMachine();
  printf("Main thread decided to kill threads!\n");
  terminate();
}
