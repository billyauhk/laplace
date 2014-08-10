#include<stdio.h> // For printf()
#include<stdlib.h>// For atoi()
#include<mpfr.h>  // For mpfr_*()
#include<signal.h>// For sigaction()
#include<error.h> // For perror()

int store = 0;

static void saveResult(int n){
  store = 1;
}

#define FILE_BASE 36

int main(int argc, char* argv[]){
  // This program computes the Laplace limit
  int digits;
  mpfr_t y, prevy, pprevy;
  char *buffer;
  struct sigaction sa;

  sa.sa_handler = &saveResult;
  sa.sa_flags = SA_SIGINFO;
  if(sigaction(SIGINT, &sa, NULL) < 0){
    perror("sigaction: SIGINT");
    return -1;
  }
  if(sigaction(SIGTERM, &sa, NULL) < 0){
    perror("sigaction: SIGTERM");
    return -1;
  }

  if(argc<2){
    fprintf(stderr,"Not enough arguments!\n");
  }else{
    digits = atol(argv[1])/4;
    mpfr_set_default_prec(atol(argv[1]));
    if(argc==2){
      mpfr_init_set_d(y,1.19967,MPFR_RNDN);
      mpfr_init_set_d(prevy,0.0,MPFR_RNDN);
      mpfr_init_set_d(pprevy,1.0,MPFR_RNDN);
    }else{
      fprintf(stderr,"Reading data from continue.dat\n");
      FILE* infile;
      infile = fopen("continue.dat", "r");
      mpfr_init(y);
      mpfr_init_set_d(prevy,0.0,MPFR_RNDN);
      mpfr_init_set_d(pprevy,1.0,MPFR_RNDN);
      printf("Read %u bytes\n", mpfr_inp_str(y,infile,FILE_BASE,MPFR_RNDN));
    }
  }

  while(mpfr_cmp(y,prevy) && store==0){
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
//      putchar(',');
      int precision = mpfr_get_default_prec() + 10;
      mpfr_set_default_prec(precision);
      mpfr_set_prec(y, precision);
      mpfr_set(y, prevy, MPFR_RNDN);
      mpfr_set_prec(prevy, precision);
      mpfr_set(prevy, pprevy, MPFR_RNDN);
      mpfr_set_prec(pprevy, precision);
      mpfr_set(pprevy, prevy, MPFR_RNDN);
    }else{
//      putchar('.');
    }
  }

  fprintf(stderr,"Storing data into continue.dat\n");
  FILE* outfile;
  outfile = fopen("continue.dat","w");
  mpfr_out_str(outfile, FILE_BASE, 0, y, MPFR_RNDN);
  fclose(outfile);

  if(store==0){
    // Compute x from y
    mpfr_sqr(y,y,MPFR_RNDN);
    mpfr_sub_ui(y,y,1,MPFR_RNDN);
    mpfr_sqrt(y,y,MPFR_RNDN);
    buffer = (char*) malloc(sizeof(char)*(digits+10));
    mpfr_snprintf(buffer,digits+10,"%.*Rf",digits,y);
    printf("%s\n",buffer);
    free(buffer);
  }
  return 0;
}
