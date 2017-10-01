/* ------------------------------------------------------------------------
   phase2.c

   University of Arizona
   Computer Science 452

   ------------------------------------------------------------------------ */

#include <phase1.h>
#include <phase2.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "handler.c"

/* ------------------------- Prototypes ----------------------------------- */
extern int start2 (char *);
int MboxSendZero(int, void*, int);
int MboxRecvZero(int, void*, int);
int waitDevice(int, int, int *);
int UnblockReceiver(int);
int UnblockSender(int);
int MboxRelease(int);
int start1 (char *);
int zapCheck(int);
int check_io();
int mode();

void AddToReceiveBlockList(int, int);
void AddToSendBlockList(int, int);
void check_kernel_mode(char *);
void DecrementProcs(int);
void disableInterrupts();
void enableInterrupts();
void dumpMboxes();

/* -------------------------- Globals ------------------------------------- */
int debugflag2 = 0;

// the mail boxes 
mailbox MailBoxTable[MAXMBOX];
proc ProcTable[50];
int mboxCount = 0;
int nextMboxID = 0;
int occupiedSlots = 0;

// also need array of mail slots, array of function ptrs to system call 
// handlers, ...

/* -------------------------- Functions ----------------------------------- */

/* start1-----------------------------------------------------------------
   Name - start1
   Purpose - Initializes mailboxes and interrupt vector.
             Start the phase2 test process.
   Parameters - one, default arg passed by fork1, not used here.
   Returns - one to indicate normal quit.
   Side Effects - lots since it initializes the phase2 data structures.
   ----------------------------------------------------------------------- */
int start1(char *arg)
{
    if (DEBUG2 && debugflag2)
        USLOSS_Console("start1(): at beginning\n");

    check_kernel_mode("start1");

    // Disable interrupts
    disableInterrupts();

    // Initialize the mail box table, slots, & other data structures.
    // Initialize USLOSS_IntVec and system call handlers,
    // allocate mailboxes for interrupt handlers.  Etc... 
    for (int i = 0; i < 7; i++){
      MboxCreate(0, 50);
    }

    USLOSS_IntVec[USLOSS_CLOCK_INT] = (void (*) (int, void *))clockHandler2;
    USLOSS_IntVec[USLOSS_TERM_INT] = (void (*) (int, void *))termHandler;
    USLOSS_IntVec[USLOSS_DISK_INT] = (void (*) (int, void *))diskHandler;
    

    enableInterrupts();

    // Create a process for start2, then block on a join until start2 quits
    int kid_pid, status;
    if (DEBUG2 && debugflag2)
        USLOSS_Console("start1(): fork'ing start2 process\n");
    kid_pid = fork1("start2", start2, NULL, 4 * USLOSS_MIN_STACK, 1);
    if ( join(&status) != kid_pid ) {
        USLOSS_Console("start2(): join returned something other than ");
        USLOSS_Console("start2's pid\n");
    }

    return 0;
} /* start1 */


/* MboxCreate-------------------------------------------------------------
   Name - MboxCreate
   Purpose - gets a free mailbox from the table of mailboxes and initializes it 
   Parameters - maximum number of slots in the mailbox and the max size of a msg
                sent to the mailbox.
   Returns - -1 to indicate that no mailbox was created, or a value >= 0 as the
             mailbox id.
   Side Effects - initializes one element of the mail box array. 
   ----------------------------------------------------------------------- */
int MboxCreate(int slots, int slot_size)
{
  int slot = -1;

  if (mboxCount >= MAXMBOX){
    return -1;
  }

  if (DEBUG2 && debugflag2){
    USLOSS_Console("MboxCreate(): Attempting to create a mailbox with %i slots and %i slot_size\n", slots, slot_size);
  }

  // Check if slots and slot size are valid. 
  if (slots < 0 || slot_size < 0){
    USLOSS_Console("MboxCreate(): Slots or slot_size is < 0. Halting...\n");
    USLOSS_Halt(1);
    return -1;
  }

  while (MailBoxTable[nextMboxID%MAXMBOX].status != EMPTY || (MailBoxTable[nextMboxID%MAXMBOX].status == RELEASED && MailBoxTable[nextMboxID%MAXMBOX].r_blockCount == 0 && MailBoxTable[nextMboxID%MAXMBOX].s_blockCount == 0)){
    nextMboxID++;
  }
  slot = nextMboxID%MAXMBOX;
  boxPtr box = &MailBoxTable[slot];
  box->status = TAKEN;
  box->numSlots = slots;    
  box->slot_size = slot_size;
  box->mboxID = slot;

  // Creating all of the slots of the mailbox

  struct mailSlot *temp = malloc(sizeof(*temp));
  temp->mboxID = box->mboxID;
  temp->status = SLOT_READY;
  temp->message = malloc(MAX_MESSAGE);
  box->slots = temp;
  for (int i = 0; i < box->numSlots; i++){
    temp->nextSlot = malloc(sizeof(*temp));
    temp = temp->nextSlot;
    temp->mboxID = box->mboxID;
    temp->status = SLOT_READY;
    temp->message = malloc(MAX_MESSAGE);
  }

  mboxCount++;
  return MailBoxTable[slot].mboxID;
} /* MboxCreate */


/* MboxSend---------------------------------------------------------------
   Name - MboxSend
   Purpose - Put a message into a slot for the indicated mailbox.
             Block the sending process if no slot available.
   Parameters - mailbox id, pointer to data of msg, # of bytes in msg.
   Returns - zero if successful, -1 if invalid args.
   Side Effects - none.
   ----------------------------------------------------------------------- */
int MboxSend(int mbox_id, void *msg_ptr, int msg_size)
{
  if (DEBUG2 && debugflag2){
    USLOSS_Console("MboxSend(): Sending a message to %i\n", mbox_id);
  }

  // Check if inputs are valid
  if (mbox_id < 0 || msg_size < 0){
    USLOSS_Console("MboxSend(): mbox_id or msg_size is < 0. Halting...\n");
    USLOSS_Halt(1);
    return -1;
  }
  if (msg_size > MAX_MESSAGE || msg_size > MailBoxTable[mbox_id%MAXMBOX].slot_size){
    return -1;
  }

  boxPtr box = &MailBoxTable[mbox_id%MAXMBOX];

  if (box->numSlots == 0){                      // If it is a zero slot mailbox we call MboxSendZero to deal with it.
    return MboxSendZero(mbox_id, msg_ptr, msg_size);
  }
  else {                                        // If the mailbox has more than zero slots...
    if (box->filledSlots < box->numSlots){      // If there are available slots in the mailbox...
      slotPtr iterator = box->slots;
      while(iterator->status != SLOT_READY){    // Add it to the first available slot we find
        iterator = iterator->nextSlot;
      }
      iterator->status = SLOT_TAKEN;
      memcpy(iterator->message, msg_ptr, msg_size);
      box->filledSlots++;                       // Let them know that we have added a msg.
      if (box->r_blockCount != 0){              // If there are rBlocked processes then we unblock 1...
        UnblockReceiver(mbox_id);
      }
      return zapCheck(0);
    }
    else {                                      // If there aren't available slots, we'll block
      box->s_blockCount++;                      // Increment s_blocked to reflect us blocking
      AddToSendBlockList(mbox_id, getpid());    // Add ourselves to the send blocked list.
      blockMe(SEND_BLOCKED);
      box->s_blockCount--;                      // decrement ourselves when we are unblocked
      if (box->status == RELEASED){             // if the mbox was released
        return -3;                              // return -3
      }
      slotPtr iterator = box->slots;            
      while (iterator->status != SLOT_READY){
        iterator = iterator->nextSlot;
      }
      iterator->status = SLOT_TAKEN;
      memcpy(iterator->message, msg_ptr, msg_size); // Copy over our message into the mbox
      box->filledSlots++;
      
      if (box->r_blockCount != 0){
        UnblockReceiver(mbox_id);
      }
      return zapCheck(0);
    }
  }
  return -10;
} /* MboxSend */

int MboxSendZero(int mbox_id, void *msg_ptr, int msg_size)
{
  boxPtr box = &MailBoxTable[mbox_id%MAXMBOX];

  if (box->r_blockCount == 0){
    box->s_blockCount++;
    AddToSendBlockList(mbox_id, getpid());
    blockMe(SEND_BLOCKED);
    box->s_blockCount--;
  }
  else {                                        // If there is a receiver blocked on a 0slot
    if (msg_size != 0){
      memcpy(box->slots->message, msg_ptr, msg_size);
    }
    box->receive_blocked->modified = 1;
    UnblockReceiver(mbox_id);
    return 0;
  }
  return -10;
} /* MboxSendZero */

/* MboxCondSend-----------------------------------------------------------
   Name - MboxCondSend
   Purpose - Put a message into a slot for the indicated mailbox.
             Block the sending process if no slot available.
   Parameters - mailbox id, pointer to data of msg, # of bytes in msg.
   Returns - zero if successful, -1 if invalid args.
   Side Effects - none.
   ----------------------------------------------------------------------- */
int MboxCondSend(int mbox_id, void * message, int msg_size)
{
  if (DEBUG2 && debugflag2){
    USLOSS_Console("MboxCondSend(): Attempting to conditionally send to mbox: %i\n", mbox_id);
  }

  int index = mbox_id % MAXMBOX;
  boxPtr box = &MailBoxTable[index];

  if (box->numSlots == 0){
    if (box->r_blockCount > 0){
      return MboxSendZero(mbox_id, message, msg_size);  
    }
    return -2;
  }
  
  if (box->filledSlots == box->numSlots){       // If there is no space, Return -2
    return -2;
  }
  else {                                        // If there is space, send it normally.
    return MboxSend(mbox_id, message, msg_size);
  }
} /* MboxCondSend */

/* MboxReceive------------------------------------------------------------
   Purpose - Get a msg from a slot of the indicated mailbox.
             Block the receiving process if no msg available.
   Parameters - mailbox id, pointer to put data of msg, max # of bytes that
                can be received.
   Returns - actual size of msg if successful, -1 if invalid args.
   Side Effects - none.
   ----------------------------------------------------------------------- */
int MboxReceive(int mbox_id, void *msg_ptr, int msg_size)
{
  if (DEBUG2 && debugflag2){
    USLOSS_Console("MboxReceive(): Starting MboxReceive with mbox_id %i\n", mbox_id);
  }

  // Check inputs --------------------------
  if (mbox_id < 0){
    return -1;
  }
  int index = mbox_id % MAXMBOX;
  if (MailBoxTable[index].status != 1){ 
    return -1;
  } // Check inputs ------------------------

  boxPtr box = &MailBoxTable[index];
  int procIndex = getpid() % 50;
  ProcTable[procIndex].mboxID = mbox_id;
  ProcTable[procIndex].pos = box->r_blockCount;
  slotPtr iterator, previous;

  if (box->numSlots == 0){
    return MboxRecvZero(mbox_id, msg_ptr, msg_size);
  }
  else {
    if (box->filledSlots != 0){                 // If there is a message, we'll recv it
      iterator = box->slots;
      previous = NULL;
      for (int i = 0; i < ProcTable[procIndex].pos; i++){
        previous = iterator;
        iterator = iterator->nextSlot;
      }
      memcpy(msg_ptr, iterator->message, msg_size);
      box->filledSlots--;                       // Decrement filled slots as we have taken a message out.
      // Need to unblock a sender
    }
    else {                                      // If there are no messages, block.
      box->r_blockCount++;
      AddToReceiveBlockList(mbox_id, getpid());
      blockMe(RECEIVE_BLOCKED);
      box->r_blockCount--;
      if(box->status == RELEASED){
        return -3;
      }
      iterator = box->slots;
      previous = NULL;
      for (int i = 0; i < ProcTable[procIndex].pos; i++){
        previous = iterator;
        iterator = iterator->nextSlot;
      }
      memcpy(msg_ptr, iterator->message, msg_size);
      box->filledSlots--;
    }

    if (previous != NULL){
      previous->nextSlot = iterator->nextSlot;  // These next steps will help maintain a FIFO Queue.
    }
    else {
      box->slots = box->slots->nextSlot;
    }
    iterator = box->slots;
    while (iterator->nextSlot != NULL){
      iterator = iterator->nextSlot;
    }
    iterator->nextSlot = malloc(sizeof(mailSlot));
    iterator = iterator->nextSlot;
    iterator->mboxID = mbox_id;
    iterator->status = SLOT_READY;
    iterator->message = malloc(MAX_MESSAGE);  // This is the end of FIFO maintenance.
    DecrementProcs(mbox_id);
    UnblockSender(mbox_id);
    return zapCheck(strlen(msg_ptr) + 1);
  }
  return -10;
} /* MboxReceive */

int MboxRecvZero(int mbox_id, void * msg_ptr, int msg_size)
{
  boxPtr box = &MailBoxTable[mbox_id%MAXMBOX];
  if (box->s_blockCount == 0){
    box->r_blockCount++;
    AddToReceiveBlockList(mbox_id, getpid());
    blockMe(RECEIVE_BLOCKED);
    box->r_blockCount--;
    if (box->status == RELEASED){
      return -3;
    }
    if (msg_size != 0){
      memcpy(msg_ptr, box->slots->message, sizeof(box->slots->message));  
      return zapCheck(strlen(msg_ptr) + 1);
    }
    else {
      return zapCheck(0);
    }
  }
  return -10;
} /* MboxRecvZero */

/* MboxCondReceive--------------------------------------------------------
   Purpose - Get a msg from a slot of the indicated mailbox.
             Block the receiving process if no msg available.
   Parameters - mailbox id, pointer to put data of msg, max # of bytes that
                can be received.
   Returns - actual size of msg if successful, -1 if invalid args.
   Side Effects - none.
   ----------------------------------------------------------------------- */
int MboxCondReceive(int mbox_id, void * msg_ptr, int msg_size)
{
  if (DEBUG2 && debugflag2){
    USLOSS_Console("MboxCondReceive(): conditionally receiving from mbox %i\n", mbox_id);
  }

  int index = mbox_id%MAXMBOX;
  boxPtr box = &MailBoxTable[index];
  if (box != NULL && box->status == TAKEN){
    if (box->filledSlots == 0){
      return -2;
    }
    else {
      return MboxReceive(mbox_id, msg_ptr, msg_size);
    }
  } 
  else {
    return -1;
  }
  return 0;
} /* MboxCondReceive */

/* MboxRelease------------------------------------------------------------
   Purpose - Get a msg from a slot of the indicated mailbox.
             Block the receiving process if no msg available.
   Parameters - mailbox id, pointer to put data of msg, max # of bytes that
                can be received.
   Returns - actual size of msg if successful, -1 if invalid args.
   Side Effects - none.
   ----------------------------------------------------------------------- */
int MboxRelease(int mbox_id)
{
  if (DEBUG2 && debugflag2){
    USLOSS_Console("MboxRelease(): Releasing mbox %i\n", mbox_id);
  }

  int index = mbox_id%MAXMBOX;
  if (MailBoxTable[index].status != TAKEN){
    USLOSS_Console("MboxRelease(): This mailbox cannot be released. Halting...\n");
    USLOSS_Halt(1);
    return -1;
  }

  mboxCount--;
  MailBoxTable[index].status = RELEASED;
  int sb = MailBoxTable[index].s_blockCount;
  int rb = MailBoxTable[index].r_blockCount;
  for (int i = 0; i < sb; i++){
    UnblockSender(mbox_id);
  }
  for (int i = 0; i < rb; i++){
    UnblockReceiver(mbox_id);
  }

  return 0;
} /* MboxRelease */

/* AddToSendBlockList-----------------------------------------------------
   Name - AddToBlockList
   Purpose - Adds a process to a mailboxes block list if it is waiting for
              a receive.
   Parameters - mailbox id, process id
   ----------------------------------------------------------------------- */
void AddToSendBlockList(int mbox_id, int pid)
{
  if (DEBUG2 && debugflag2){
    USLOSS_Console("AddToSendBlockList(): Adding pid:%i to mailbox:%i block list.\n", pid, mbox_id);
  }

  int index = mbox_id%MAXMBOX;
  boxPtr box = &MailBoxTable[index];
  if (box != NULL && box->status == TAKEN){
    blockPtr temp = box->send_blocked;
    if (temp == NULL){
      temp = malloc(sizeof(blockList));
      temp->message = malloc(MAX_MESSAGE);
      temp->blockedID = pid;
      box->send_blocked = temp;
    } else {
      while (temp->nextBlocked != NULL){
        temp = temp->nextBlocked;
      }
      temp->nextBlocked = malloc(sizeof(blockList));
      temp->nextBlocked->blockedID = pid;
    }
  } 
} /* AddToSendBlockList */

/* UnblockSender----------------------------------------------------------
   Name - UnblockSender
   Purpose - Unblocks a blocked process (if there is one) that is waiting
              to send a message from mailbox mbox_id.
   Parameters - mailbox id
   ----------------------------------------------------------------------- */
int UnblockSender(int mbox_id)
{
  if (DEBUG2 && debugflag2){
    USLOSS_Console("UnblockReceiver(): Unblocking first process at mailbox:%i\n", mbox_id);
  }

  int index = mbox_id%MAXMBOX;
  boxPtr box = &MailBoxTable[index];
  if (box->send_blocked != NULL){
    int unblockPID = box->send_blocked->blockedID;
    box->send_blocked = box->send_blocked->nextBlocked;
    unblockProc(unblockPID);
    return -1;
  } 
  else {
    return 0;
  }
} /* UnblockSender */

/* AddToReceiveBlockList--------------------------------------------------
   Name - AddToBlockList
   Purpose - Adds a process to a mailboxes block list if it is waiting for
              a receive.
   Parameters - mailbox id, process id
   ----------------------------------------------------------------------- */
void AddToReceiveBlockList(int mbox_id, int pid)
{
  if (DEBUG2 && debugflag2){
    USLOSS_Console("AddToBlockList(): Adding pid:%i to mailbox:%i block list.\n", pid, mbox_id);
  }

  int index = mbox_id%MAXMBOX;
  boxPtr box = &MailBoxTable[index];
  if (box != NULL && box->status == TAKEN){
    blockPtr temp = box->receive_blocked;
    if (temp == NULL){
      temp = malloc(sizeof(blockList));
      temp->message = malloc(MAX_MESSAGE);
      temp->blockedID = pid;
      box->receive_blocked = temp;
    } else {
      while (temp->nextBlocked != NULL){
        temp = temp->nextBlocked;
      }
      temp->nextBlocked = malloc(sizeof(blockList));
      temp->nextBlocked->blockedID = pid;
    }
  } 
} /* AddToReceiveBlockList */

/* UnblockReceiver--------------------------------------------------------
   Name - UnblockReceiver
   Purpose - Unblocks a blocked process (if there is one) that is waiting
              to receive a message from mailbox mbox_id.
   Parameters - mailbox id
   ----------------------------------------------------------------------- */
int UnblockReceiver(int mbox_id)
{
  if (DEBUG2 && debugflag2){
    USLOSS_Console("UnblockReceiver(): Unblocking first process at mailbox:%i\n", mbox_id);
  }

  int index = mbox_id%MAXMBOX;
  boxPtr box = &MailBoxTable[index];
  if (box->receive_blocked != NULL){
    int unblockPID = box->receive_blocked->blockedID;
    if (box->receive_blocked->nextBlocked == NULL){
      box->receive_blocked = NULL;
    } else {
      box->receive_blocked = box->receive_blocked->nextBlocked;
    }
    unblockProc(unblockPID);
    return 1;
  } 
  else {
    return 0;
  }
} /* UnblockReceiver */

/* waitDevice-------------------------------------------------------------
   Name - waitDevice
   Purpose - Unblocks a blocked process (if there is one) that is waiting
              to receive a message from mailbox mbox_id.
   Parameters - type, unit, status.
   ----------------------------------------------------------------------- */
int waitDevice(int type, int unit, int * status)
{
  if (DEBUG2 && debugflag2){
    USLOSS_Console("waitDevice():\n");
  }
  if (mode()){
    enableInterrupts();
  } 
  else {
    USLOSS_Halt(1);
  }
  if (type == USLOSS_CLOCK_INT){
    MboxReceive(CLOCK_MBOX, status, MAX_MESSAGE);
  }
  else if (type == USLOSS_TERM_INT){
    MboxReceive(unit, status, MAX_MESSAGE);
  } 
  else if (type == USLOSS_DISK_INT){
    MboxReceive(unit, status, MAX_MESSAGE);
  }
  if (isZapped()){
    return -1;
  }
  disableInterrupts();
  return 0;
} /* waitDevice */

void dumpMboxes()
{
  for (int i = 0; i < MAXMBOX; i++){
    printf("%i : ", i);
    if (MailBoxTable[i].status == EMPTY){
      printf("EMPTY\n");
    }
    else if (MailBoxTable[i].status == TAKEN){
      printf("TAKEN\n");
    } else if (MailBoxTable[i].status == RELEASED){
      printf("RELEASED\n");
    } else {
      printf("%i\n", MailBoxTable[i].status);
    }
  }
}

int zapCheck(int check)
{
  if (isZapped()){
    return -3;
  }
  return check;
}

/* DecrementProcs---------------------------------------------------------
   Name - waitDevice
   Purpose - Unblocks a blocked process (if there is one) that is waiting
              to receive a message from mailbox mbox_id.
   Parameters - type, unit, status.
   ----------------------------------------------------------------------- */
void DecrementProcs(int mbox_id)
{
  for (int i = 0; i < 50; i++){
    if (ProcTable[i].mboxID == mbox_id){
      ProcTable[i].pos--;
    }
  }
} /* DecrementProcs */

/* check_kernel_mode------------------------------------------------------
   Name - check_kernel_mode
   Purpose - called by phase 1 library
   Parameters - name
   ----------------------------------------------------------------------- */
void check_kernel_mode(char *name)
{
  // I want to be able to fold this function
  mode();
  // blank line
} /* check_kernel_mode */

/* Mode-------------------------------------------------------------------
   Name - Mode
   Purpose - checks if we are in kernel mode.
   ----------------------------------------------------------------------- */
int mode()
{
  unsigned int mode;
  mode = USLOSS_PsrGet();
  if ((mode & USLOSS_PSR_CURRENT_MODE) == 0){
    USLOSS_Console("Not running in kernel mode. Halting...\n");
    USLOSS_Halt(1);
    return 0;
  }
  return 1;
} /* Mode */

/* disableInterrupts------------------------------------------------------
   Name - disableInterruptes
   Purpose - disables interrupts
   ----------------------------------------------------------------------- */
void disableInterrupts()
{
  if (mode()){
    unsigned int mode = USLOSS_PsrGet();
    mode &= ~(USLOSS_PSR_CURRENT_INT);
    int a = USLOSS_PsrSet(mode);
    if (a == 45){
      printf("\n");
    }
  }
} /* disableInterrupts */

/* enableInterrupts-------------------------------------------------------
   Name - enableInterrupts
   Purpose - enables interrupts
   ----------------------------------------------------------------------- */
void enableInterrupts()
{
  if (mode()){
    unsigned int mode = USLOSS_PsrGet();
    mode |= USLOSS_PSR_CURRENT_INT;
    int a = USLOSS_PsrSet(mode);
    if (a == 45){
      printf("\n");
    }
  }
} /* enableInterrupts */

/* check_io---------------------------------------------------------------
   Name - check_io
   Purpose - checks if there are any devices blocked on io.
   ----------------------------------------------------------------------- */
int check_io()
{
  for (int i = 0; i < 7; i++){
    if (MailBoxTable[i].r_blockCount > 0){
      return 1;
    }
  }
  return 0;
} /* check_io */


