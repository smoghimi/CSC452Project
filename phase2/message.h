
#define DEBUG2 1

typedef struct mailSlot     *slotPtr;
typedef struct mailSlot     mailSlot;
typedef struct mailbox      mailbox;
typedef struct mailbox      *boxPtr;
typedef struct mboxProc     *mboxProcPtr;
typedef struct blockList    *blockPtr;
typedef struct blockList    blockList;

// io mailboxes
#define     CLOCK_MBOX  0
#define     TERM1       1
#define     TERM2       2
#define     TERM3       3
#define     TERM4       4
#define     DISK1       5
#define     DISK2       6

// Process Mailbox blocks:
#define     RECEIVE_BLOCKED 55
#define     SEND_BLOCKED    56

// Mailbox Statuses:
#define     EMPTY       0
#define     TAKEN       1
#define     RELEASED    3

// Mailslot Statuses:
#define     SLOT_READY 1
#define     SLOT_TAKEN 2

struct mailbox {
    int       mboxID;
    int       numSlots;
    int       status;
    int       slot_size;
    int       filledSlots;
    slotPtr   slots;
    blockPtr  receive_blocked; 
    blockPtr  send_blocked; 
    int       s_blockCount;
    int       r_blockCount;
    // other items as needed...
};

struct mailSlot {
    int       mboxID;
    int       status;
    slotPtr   nextSlot;
    void      *message;
    // other items as needed...
};

struct blockList {
    int       blockedID;
    blockPtr  nextBlocked;
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
