#include<mpfr.h>    // mpfr_*()

// Datatype as the "executable" of the VM
enum instrStatus{
  AHEAD_WIN,  // Ahead of the current execution window, default state at start
  IN_WIN,     // Inside current execution window but operand not ready
  SRC_READY,  // All source operands ready (but not yet dispatched)
  IN_CALC,    // Dispatched (i.e., computing by a thread)
  REF_COUNT,  // Counting reference (i.e., result ready)
  REF_DEL,    // Result is thrown away to make way for other commands
  BEHIND_WIN  // Behind current execution window, i.e., all instructions before it are either REF_DEL or BEHIND_WIN
};

enum instrType{
  ONE_SRC,
  TWO_SRC,
  SRC_UI,
  UI_SRC,
  SRC_SI,
  SI_SRC,
  PRINT,     // Make sure we could debug the "executable" later
  ASSIGN     // Special slots for putting initial values
};

// Each instr_t is called a "command" which will be given to 
typedef struct instruction{
  instrStatus status;
  instrType type;
  void* ptr;                // If it is PRINT type, a string to be printed before printing the register value
                            // If it is ASSIGN type, a pointer which we would copy a value from
  int refCount;
  int currCount;            // Reference counting for *each* instruction
  void (*func)();           // Function pointer (need a lot of casting...)
  int dest;                 // The destination register (assigned dynamically). Initially -1
  int src1;                 // Source operand (the instr# of the producers)
  int src2;
  mpfr_rnd_t rndMode;
} instr_t;

// Declare the existence of the "executable"
extern instr_t exe[];
extern int commandCount;

// Declare functions for the machine and driver C files
void* thread_main(void *tid);       // Thread entry point. The ALU's FSM
void initialize(int precision);     // Initialize threads
void resetScoreboard();             // Reset the entire executable for next loop
void runMachine();                  // Run all commands in the "executable"
void incPrecision(int precision);   // Increase precision of all registers in the VM
void assign(int cid, mpfr_t* dest); // Assign result of the cid-th command's result to dest
void terminate();                   // Terminate all threads and cleanup
