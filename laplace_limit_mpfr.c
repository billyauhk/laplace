#include<stdio.h>
#include<stdlib.h>
#include<mpfr.h>

int main(int argc, char* argv[]){
  // This program computes the Laplace limit
  int digits;
  mpfr_t x, prevx, fx, dfx;
  mpfr_t temp1, temp2, temp3;
  char *buffer;

  digits = atol(argv[1])/4;
  buffer = (char*) malloc(sizeof(char)*(digits+10));
  
  mpfr_set_default_prec(atol(argv[1]));
  mpfr_init_set_d(x,0.6,MPFR_RNDN);
  mpfr_init_set_d(prevx,0,MPFR_RNDN);
  mpfr_init(fx);
  mpfr_init(dfx);

  while(mpfr_cmp(x,prevx)){
    mpfr_set(prevx,x,MPFR_RNDN);
    
    //fx = x/(1+sqrtq(1+x*x))*expq(sqrtq(1+x*x))-1;
    mpfr_init_set(temp1,x,MPFR_RNDN);
    mpfr_sqr(temp1,temp1,MPFR_RNDN);
    mpfr_add_ui(temp1,temp1,1,MPFR_RNDN);
    mpfr_sqrt(temp1,temp1,MPFR_RNDN);
    mpfr_init(temp2);
    mpfr_exp(temp2,temp1,MPFR_RNDN);
    mpfr_add_ui(temp1,temp1,1,MPFR_RNDN);
    mpfr_div(temp1,x,temp1,MPFR_RNDN);
    mpfr_mul(fx,temp1,temp2,MPFR_RNDN);
    mpfr_sub_ui(fx,fx,1,MPFR_RNDN);

    //dfx = (x*x+1+sqrtq(x*x+1))/((1+sqrtq(1+x*x))^2)*expq(sqrtq(1+x*x));
    mpfr_set(temp1,x,MPFR_RNDN);
    mpfr_sqr(temp1,temp1,MPFR_RNDN);
    mpfr_add_ui(temp1,temp1,1,MPFR_RNDN);
    mpfr_sqrt(temp1,temp1,MPFR_RNDN);
    mpfr_exp(temp2,temp1,MPFR_RNDN);
    mpfr_add_ui(temp1,temp1,1,MPFR_RNDN);
    mpfr_init(temp3);
    mpfr_sqr(temp3,temp1,MPFR_RNDN);
    mpfr_div(temp3,temp3,temp2,MPFR_RNDN);
    mpfr_set(temp2,x,MPFR_RNDN);
    mpfr_sqr(temp2,temp2,MPFR_RNDN);
    mpfr_add(temp1,temp1,temp2,MPFR_RNDN);
    mpfr_mul(dfx,temp1,temp3,MPFR_RNDN);
    mpfr_clear(temp2);
    mpfr_clear(temp3);
    
    //x = x - fx/dfx;
    mpfr_div(temp1,fx,dfx,MPFR_RNDN);
    mpfr_sub(x,x,temp1,MPFR_RNDN);
    mpfr_clear(temp1);
    
    //putchar('.');fflush(stdout);
  }
  mpfr_snprintf(buffer,digits+10,"%.*Rf",digits,x);
  printf("\n%s\n",buffer);
  free(buffer);
  return 0;
}
