/* Patrick's DEBUG printing constant... */
#define DEBUG 0
#define READY_LISTS 6
#define S_QUIT -1
#define S_READY 1
#define S_RUNNING 2
#define S_JOIN_BLOCKED 3    // Means a process has started a join but is blocked
#define S_ZAPPED 4        // Means a process has been zapped

typedef struct procStruct procStruct;

typedef struct procStruct * procPtr;

typedef struct readyList readyList;

struct readyList {
  procPtr   head;
  int       size;
};

struct procStruct {
   procPtr         nextProcPtr;
   procPtr         childProcPtr;
   procPtr         nextSiblingPtr;
   procPtr         parentPtr;
   char            name[MAXNAME];     /* process's name */
   char            startArg[MAXARG];  /* args passed to process */
   USLOSS_Context  state;             /* current context for process */
   short           pid;               /* process id */
   int             priority;
   int (* startFunc) (char *);   /* function where process begins -- launch */
   char           *stack;
   unsigned int    stackSize;
   int             status;        /* READY, BLOCKED, QUIT, etc. */
   int             procSlot;          /* The index of this process in the process table*/
   int             quitStatus;
   /* other fields as needed... */
};

struct psrBits {
    unsigned int curMode:1;
    unsigned int curIntEnable:1;
    unsigned int prevMode:1;
    unsigned int prevIntEnable:1;
    unsigned int unused:28;
};

union psrValues {
   struct psrBits bits;
   unsigned int integerPart;
};

/* Some useful constants.  Add more as needed... */
#define NO_CURRENT_PROCESS NULL
#define MINPRIORITY 5
#define MAXPRIORITY 1
#define SENTINELPID 1
#define SENTINELPRIORITY (MINPRIORITY + 1)

