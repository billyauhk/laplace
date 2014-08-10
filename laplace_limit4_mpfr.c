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
  mpfr_t e2, coth, csch_half, temp, t1, t2, t3;
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

  mpfr_init(coth);
  mpfr_init(csch_half);
  mpfr_init(temp);
  mpfr_init(t1);
  mpfr_init(t2);
  mpfr_init(t3);

  while(mpfr_cmp(y,prevy) && store==0){
    mpfr_set(pprevy,prevy,MPFR_RNDN);
    mpfr_set(prevy,y,MPFR_RNDN);

    // Cache several common-terms in the f, f' and f"
    mpfr_add(t1,y,y,MPFR_RNDN);            // e2 = e(2*y);      # Here t1 is used as e2
    mpfr_exp(t1,t1,MPFR_RNDN);
    mpfr_sub_ui(t2,t1,1,MPFR_RNDN);        // denom = 1/(e2-1); # Here t2 is used to replace denom
    mpfr_ui_div(t2,1,t2,MPFR_RNDN);
    mpfr_mul(csch_half,t1,t2,MPFR_RNDN);   // csch_half = e2*denom;   # t1 retires here
    mpfr_add(coth,csch_half,t2,MPFR_RNDN); // coth = csch_half+denom; # t2 retires here

    // Solve quadratic equation containing the derivatives
    // (always take -ve root when initial guess is close enough)
    mpfr_sqr(t1,coth,MPFR_RNDN);          // t1 = coth*coth
    mpfr_sqr(temp,csch_half,MPFR_RNDN);   // temp = coth*csch_half^2
    mpfr_mul(temp,coth,temp,MPFR_RNDN);
    mpfr_sub(t2,coth,y,MPFR_RNDN);        // t2 = sqrt(coth^4-16*(coth-y)*temp)
    mpfr_mul(t2,temp,t2,MPFR_RNDN);
    mpfr_mul_ui(t2,t2,16,MPFR_RNDN);
    mpfr_sqr(t3,t1,MPFR_RNDN);
    mpfr_sub(t2,t3,t2,MPFR_RNDN);
    mpfr_sqrt(t2,t2,MPFR_RNDN);
    mpfr_sub(t3,t1,t2,MPFR_RNDN);         // dy = (t1-t2)/(8*temp)
    mpfr_mul_ui(temp,temp,8,MPFR_RNDN);
    mpfr_div(temp,t3,temp,MPFR_RNDN);
    mpfr_add(y,y,temp,MPFR_RNDN);         // y = y+dy

    if(!mpfr_cmp(y,pprevy)){
      // Oscillation occured, add 10 bits of precision
      putchar(',');
      int precision = mpfr_get_default_prec() + 100;
      mpfr_set_default_prec(precision);
      mpfr_set_prec(y, precision);
      mpfr_set(y, prevy, MPFR_RNDN);
      mpfr_set_prec(prevy, precision);
      mpfr_set(prevy, pprevy, MPFR_RNDN);
      mpfr_set_prec(pprevy, precision);
      mpfr_set(pprevy, prevy, MPFR_RNDN);
      mpfr_set_prec(t1, precision);
      mpfr_set_prec(t2, precision);
      mpfr_set_prec(t3, precision);
      mpfr_set_prec(csch_half, precision);
      mpfr_set_prec(coth, precision);
      mpfr_set_prec(temp, precision);
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
