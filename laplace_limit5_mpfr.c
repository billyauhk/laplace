#include<stdio.h> // For printf()
#include<stdlib.h>// For atoi()
#include<mpfr.h>  // For mpfr_*()
#include<signal.h>// For sigaction()
#include<error.h> // For perror()

int digits;
char *buffer;
int store = 0;

static void saveResult(int n){
  store = 1;
}

inline void printResult(mpfr_t* y){
  buffer = (char*) malloc(sizeof(char)*(digits+10));
  mpfr_snprintf(buffer,digits+10,"%.*Rf",digits,*y);
  printf("%s\n",buffer);
  free(buffer);
}

#define FILE_BASE 36

int main(int argc, char* argv[]){
  // This program computes the Laplace limit
  mpfr_t y, prevy, pprevy;
  mpfr_t e1, e2, t1, t2, t3, denom, f0, f1, f2, f3, dy;
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

  mpfr_init(e1);
  mpfr_init(e2);
  mpfr_init(denom);
  mpfr_init(t1);
  mpfr_init(t2);
  mpfr_init(t3);
  mpfr_init(f0);
  mpfr_init(f1);
  mpfr_init(f2);
  mpfr_init(f3);
  mpfr_init(dy);

  while(mpfr_cmp(y,prevy) && store==0){
    mpfr_set(pprevy,prevy,MPFR_RNDN);
    mpfr_set(prevy,y,MPFR_RNDN);

    // Cache several common-terms in the f, f' and f"
    mpfr_exp(e1,y,MPFR_RNDN);             // e1 = e(y);
    mpfr_add(e2,y,y,MPFR_RNDN);           // e2 = e(2y);
    mpfr_exp(e2,e2,MPFR_RNDN);
    mpfr_sub_ui(denom,e2,1,MPFR_RNDN);    // denom = 1/(e2-1);
    mpfr_ui_div(denom,1,denom,MPFR_RNDN);
    mpfr_mul(t1,e1,denom,MPFR_RNDN);      // csch = e1*denom;       # Use t1 as csch
    mpfr_mul(t2,e1,t1,MPFR_RNDN);         // coth = e1*csch+denom;  # Use t2 as coth
    mpfr_add(t2,t2,denom,MPFR_RNDN);
    mpfr_add(t1,t1,t1,MPFR_RNDN);         // csch = csch+csch;
    mpfr_ui_div(t3,1,e2,MPFR_RNDN);       // cosh2 = 0.5*(e2+1/e2)  # Use t3 as cosh2
    mpfr_add(t3,e2,t3,MPFR_RNDN);
    mpfr_div_ui(t3,t3,2,MPFR_RNDN);       // # e1, e2, denom retired
//printf("csch=");printResult(&t1);
//printf("coth=");printResult(&t2);
//printf("cosh2=");printResult(&t3);

    // Compute f, f', f"/2 and f^(3)/6
    mpfr_sub(f0,t2,y,MPFR_RNDN);     // f0 = coth-y     
    mpfr_mul(f1,t2,t2,MPFR_RNDN);    // f1 = -coth*coth
    mpfr_mul_si(f1,f1,-1,MPFR_RNDN); 
    mpfr_mul(e1,t1,t1,MPFR_RNDN);    // cschsq = csch^2  # Use e1 as cschsq
    mpfr_mul(f2,t2,e1,MPFR_RNDN);    // f2 = coth*cschsq
    mpfr_mul(e2,e1,e1,MPFR_RNDN);    // f3 = -(cosh2+2)*(cschsq^2)/3  # Use e2 as cschsq^2
    mpfr_add_ui(f3,t3,2,MPFR_RNDN);
    mpfr_div_si(f3,f3,-3,MPFR_RNDN);
    mpfr_mul(f3,f3,e2,MPFR_RNDN);    // # t1, t2, t3, e1, e2 retired
//printf("f0=");printResult(&f0);
//printf("f1=");printResult(&f1);
//printf("f2/2=");printResult(&f2);
//printf("f3/6=");printResult(&f3);

    // Solve cubic equation and update
    // Near the root we have one real root only :)
                                     // delta0 = f2^2 - 3*f3*f1                    # Use e1 as delta0
    mpfr_mul(t1,f2,f2,MPFR_RNDN);    // t1 <- f2^2
    mpfr_mul(t2,f1,f3,MPFR_RNDN);
    mpfr_mul_ui(t2,t2,3,MPFR_RNDN);  // t2 <- 3*f1*f3
    mpfr_sub(e1,t1,t2,MPFR_RNDN);
//printf("delta0=");printResult(&e1);
                                     // delta1 = 2*f2^3 - 9*f3*f2*f1 + 27*f3^2*f0  # Use e2 as delta1
    mpfr_mul(t1,t1,f2,MPFR_RNDN);
    mpfr_add(t1,t1,t1,MPFR_RNDN);    // t1 <- 2*f2^3
    mpfr_mul(t2,t2,f2,MPFR_RNDN);
    mpfr_mul_ui(t2,t2,3,MPFR_RNDN);  // t2 <- 9*f1*f2*f3
    mpfr_sub(t1,t1,t2,MPFR_RNDN);    // t1 <- 2*f2^3 - 9*f3*f2*f1
    mpfr_mul(t3,f3,f3,MPFR_RNDN);    // t3 <- f3^2
    mpfr_mul_ui(t2,f0,27,MPFR_RNDN); // t2 <- 27*f0
    mpfr_mul(t3,t3,t2,MPFR_RNDN);
    mpfr_sub(e2,t1,t3,MPFR_RNDN);    // # t1, t2, t3 retired
//printf("delta1=");printResult(&e2);

                                     // c=cbrt((delta1+sqrt(delta1^2-4*delta0^3))/2)  # Use t1 as c
    mpfr_mul(t1,e2,e2,MPFR_RNDN);    // t1 <- delta1^2
    mpfr_pow_ui(t2,e1,3,MPFR_RNDN);  // t2 <- delta0^3
    mpfr_mul_ui(t2,t2,4,MPFR_RNDN);
    mpfr_sub(t1,t1,t2,MPFR_RNDN);
    mpfr_sqrt(t1,t1,MPFR_RNDN);
    mpfr_add(t1,t1,e2,MPFR_RNDN);
    mpfr_div_ui(t1,t1,2,MPFR_RNDN);
    mpfr_cbrt(t1,t1,MPFR_RNDN);
//printf("c=");printResult(&t1);

    mpfr_div(t2,e1,t1,MPFR_RNDN);    // dy=(f2+c+delta0/c)/(-3*f3)     # Use t2 as dy
    mpfr_add(t3,f2,t1,MPFR_RNDN);
    mpfr_mul_ui(f3,f3,3,MPFR_RNDN);  // f3 <- 3*f3
    mpfr_add(t1,t2,t3,MPFR_RNDN);
    mpfr_div(t1,t1,f3,MPFR_RNDN);
//printf("dy=");printResult(&t1);

    mpfr_add(y,y,t1,MPFR_RNDN);      // y = y+dy

//printf("     y=");printResult(&y);
//printf("pprevy=");printResult(&pprevy);
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
      mpfr_set_prec(e1, precision);
      mpfr_set_prec(e2, precision);
      mpfr_set_prec(denom, precision);
      mpfr_set_prec(t1, precision);
      mpfr_set_prec(t2, precision);
      mpfr_set_prec(t3, precision);
      mpfr_set_prec(f0, precision);
      mpfr_set_prec(f1, precision);
      mpfr_set_prec(f2, precision);
      mpfr_set_prec(f3, precision);
    }else{
//      putchar('.');
    }
//printf("=====\n");
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
    printResult(&y);
  }
  return 0;
}
