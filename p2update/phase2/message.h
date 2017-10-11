
#define DEBUG2 1

typedef struct mailSlot *slotPtr;
typedef struct mailbox   mailbox;
typedef struct mboxProc *mboxProcPtr;

struct mailbox {
    int       mboxID;
    // other items as needed...
};

struct mailSlot {
    int       mboxID;
    int       status;
    // other items as needed...
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
