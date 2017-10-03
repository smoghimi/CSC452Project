/*
 * These are the definitions for phase 2 of the project
 */

#ifndef _PHASE2_H
#define _PHASE2_H

// Maximum line length. Used by terminal read and write.
#define MAXLINE         80

#define MAXMBOX         2000
#define MAXSLOTS        2500
#define MAX_MESSAGE     150  // largest possible message in a single slot

// returns id of mailbox, or -1 if no more mailboxes, -2 if invalid args
extern int MboxCreate(int slots, int slot_size);

// returns 0 if successful, -1 if invalid arg
extern int MboxRelease(int mbox_id);

// returns 0 if successful, -1 if invalid args
extern int MboxSend(int mbox_id, void *msg_ptr, int msg_size);

// returns size of received msg if successful, -1 if invalid args
extern int MboxReceive(int mbox_id, void *msg_ptr, int msg_max_size);

// returns 0 if successful, 1 if mailbox full, -1 if illegal args
extern int MboxCondSend(int mbox_id, void *msg_ptr, int msg_size);

// returns 0 if successful, 1 if no msg available, -1 if illegal args
extern int MboxCondReceive(int mbox_id, void *msg_ptr, int msg_max_size);

// type = interrupt device type, unit = # of device (when more than one),
// status = where interrupt handler puts device's status register.
extern int waitDevice(int type, int unit, int *status);
extern int MboxSendZero(int, void*, int);
extern int MboxRecvZero(int, void*, int);
extern int waitDevice(int, int, int *);
extern int UnblockReceiver(int);
extern int UnblockSender(int);
extern int MboxRelease(int);
extern int start1 (char *);
extern int zapCheck(int);
extern int check_io();
extern int mode();

extern void AddToReceiveBlockList(int, int);
extern void AddToSendBlockList(int, int);
extern void check_kernel_mode(char *);
extern void DecrementProcs(int);
extern void disableInterrupts();
extern void enableInterrupts();
extern void dumpMboxes();


//  The systemArgs structure
typedef struct systemArgs
{
        int number;
        void *arg1;
        void *arg2;
        void *arg3;
        void *arg4;
        void *arg5;
} systemArgs;

// 
extern void (*systemCallVec[])(systemArgs *args);

#endif
