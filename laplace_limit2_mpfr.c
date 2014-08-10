#include<stdio.h>
#include<stdlib.h>
#include<mpfr.h>

int main(int argc, char* argv[]){
  // This program computes the Laplace limit
  int digits;
  mpfr_t y, prevy, pprevy;
  char *buffer;

  digits = atol(argv[1])/4;
  buffer = (char*) malloc(sizeof(char)*(digits+10));
  
  mpfr_set_default_prec(atol(argv[1]));
  mpfr_init_set_d(y,1.19967,MPFR_RNDN);
  mpfr_init_set_d(prevy,0.0,MPFR_RNDN);
  mpfr_init_set_d(pprevy,1.0,MPFR_RNDN);

  while(mpfr_cmp(y,prevy)){
    mpfr_set(pprevy,prevy,MPFR_RNDN);
    mpfr_set(prevy,y,MPFR_RNDN);

    //y_{n+1} = 2/(1-exp(-2*y_n))-1
    // The iterative method works, but sometimes run into oscillation when precision is not enough
    mpfr_add(y,y,y,MPFR_RNDN);
    mpfr_mul_si(y,y,-1,MPFR_RNDN);
    mpfr_exp(y,y,MPFR_RNDN);
    mpfr_ui_sub(y,1,y,MPFR_RNDN);
    mpfr_ui_div(y,2,y,MPFR_RNDN);
    mpfr_sub_ui(y,y,1,MPFR_RNDN);

    if(!mpfr_cmp(y,pprevy)){
      // Oscillation occured, add 10 bits of precision
      putchar(',');
      int precision = mpfr_get_default_prec() + 10;
      mpfr_set_default_prec(precision);
      mpfr_set_prec(y, precision);
      mpfr_set(y, prevy, MPFR_RNDN);
      mpfr_set_prec(prevy, precision);
      mpfr_set(prevy, pprevy, MPFR_RNDN);
      mpfr_set_prec(pprevy, precision);
      mpfr_set(pprevy, prevy, MPFR_RNDN);
    }else{
      putchar('.');
    }
  }
  // Compute x from y
  mpfr_sqr(y,y,MPFR_RNDN);
  mpfr_sub_ui(y,y,1,MPFR_RNDN);
  mpfr_sqrt(y,y,MPFR_RNDN);
  mpfr_snprintf(buffer,digits+10,"%.*Rf",digits,y);
  printf("\n%s\n",buffer);
  free(buffer);
  return 0;
}
