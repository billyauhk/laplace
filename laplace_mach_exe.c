#include "laplace_mach.h"

/*
enum instrStatus{  AHEAD_WIN,  IN_WIN,     SRC_READY,  IN_CALC,    REF_COUNT,  REF_DEL,    BEHIND_WIN};
enum instrType{  ONE_SRC,  TWO_SRC,  SRC_UI,   UI_SRC,   SRC_SI,   SI_SRC,   PRINT,    ASSIGN  };
*/

// Test program...repeatedly take square root of a number...and it will terminate when it become 1.0
// Later we will write a compiler to generate this array
instr_t exe[] = { \
//         status     type     ptr   rCcC  func                    dst src1 src2  rndMode
/* 000 */ {AHEAD_WIN, ASSIGN,  NULL, 1, 0, (void (*)()) mpfr_set_d, -1, -1, -1, MPFR_RNDN}, \
/* 001 */ {AHEAD_WIN, ONE_SRC, NULL, 1, 0, (void (*)()) mpfr_sqrt,  -1,  0, -1, MPFR_RNDN}, \
/* 002 */ {AHEAD_WIN, PRINT,   "c=", 0, 0, NULL,                    -1,  1, -1, MPFR_RNDN}  \
};
int commandCount = sizeof(exe)/sizeof(exe[0]);
