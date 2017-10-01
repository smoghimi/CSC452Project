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
int waitDevice(int, int, int *);
int UnblockReceiver(int);
int UnblockSender(int);
int MboxRelease(int);
int start1 (char *);
int zapCheck(int);

int check_io();

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

  boxPtr mb = &MailBoxTable[mbox_id%MAXMBOX];

  if (mb->status != TAKEN){
    return -1;
  }

  // No blocked receivers && zero slot mbox

  // blocked receiver && zero slot
  if (mb->numSlots == 0 && mb->r_blockCount != 0){
    if (msg_ptr != NULL){
      memcpy(mb->slots->message, msg_ptr, msg_size);
    }
    UnblockReceiver(mbox_id);
    return 0;
  }
  else if (mb->numSlots == 0 && mb->r_blockCount == 0) {
    int callerPID = getpid();
    AddToSendBlockList(mbox_id, callerPID);
    mb->s_blockCount++;
    if (msg_ptr != NULL){
      memcpy(mb->slots->message, msg_ptr, msg_size);
    }
    blockMe(SEND_BLOCKED);
    mb->s_blockCount--;
    if(mb->status == RELEASED){
      return -3;
    }
    return 0;
  }

  if (mb->filledSlots == mb->numSlots){
    int callerPID = getpid();
    AddToSendBlockList(mbox_id, callerPID);
    mb->s_blockCount++;
    blockMe(SEND_BLOCKED);
    mb->s_blockCount--;
  }

  if (mb->status == RELEASED || isZapped()){
    return -3;
  }

  slotPtr temp = mb->slots;
  while (temp != NULL && temp->status != SLOT_READY){
    temp = temp->nextSlot;
  }

  temp->status = SLOT_TAKEN;
  memcpy(temp->message, msg_ptr, msg_size);
  occupiedSlots++;
  mb->filledSlots++;
  UnblockReceiver(mbox_id);
  //unblockProc(callerPID);

  return 0;
} /* MboxSend */

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

  if (box != NULL && box->status == TAKEN){
    if ((box->filledSlots == box->numSlots && box->numSlots != 0) || occupiedSlots >= MAXSLOTS){
      return -2;
    } 
    else if (box->numSlots == 0 && box->r_blockCount != 0){
      memcpy(box->slots->message, message, msg_size);
      UnblockReceiver(mbox_id);
      return 0;
    }
    else {
      MboxSend(mbox_id, message, msg_size);
      return 0;
    }
  } else {
    return -1;
  }
  return 0;
}

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

  if (mbox_id < 0){
    USLOSS_Console("MboxReceive(): Mailbox id is < 0. Halting...\n");
    USLOSS_Halt(1);
    return -1;
  }
  int index = mbox_id % MAXMBOX;
  if (MailBoxTable[index].status != 1){ // Currently if the status is empty. Might need to change this later.
    USLOSS_Console("MboxReceive(): Mailbox is currently empty. Halting...\n");
    USLOSS_Halt(1);
    return -1;
  }

  boxPtr box = &MailBoxTable[index];
  int callerPID = getpid();
  ProcTable[callerPID%50].mboxID = mbox_id;
  ProcTable[callerPID%50].pos = box->r_blockCount;
  // Zero slot mbox && 0 s blocked
  if (box->numSlots == 0 && box->s_blockCount == 0){
    AddToReceiveBlockList(mbox_id, callerPID);
    box->r_blockCount++;
    blockMe(RECEIVE_BLOCKED);
    box->r_blockCount--;
    if (box->status == RELEASED){
      return -3;
    }
    if (box->slots->message != NULL && msg_size != 0){
      slotPtr rTemp;
      rTemp = box->slots;
      /*for (int i = 0; i < ProcTable[callerPID%50].pos; i++){
        rTemp = rTemp->nextSlot;
      }*/
      printf("recv before memcpy\n");
      memcpy(msg_ptr, box->slots->message, msg_size);
      printf("recv after memcpy\n");
      DecrementProcs(mbox_id);
      return strlen(rTemp->message) + 1;
    }
    else {
      return 0;
    }
  }
  if (box->numSlots == 0 && box->s_blockCount != 0){
    memcpy(msg_ptr, box->slots->message, msg_size);
    DecrementProcs(mbox_id);
    UnblockSender(mbox_id);
    return strlen(box->slots->message)+1;
  }

  if (box->filledSlots == 0){
    AddToReceiveBlockList(mbox_id, callerPID);
    box->r_blockCount++;
    blockMe(RECEIVE_BLOCKED);
    box->r_blockCount--;
  }

  if (box->status == RELEASED){
    return -3;
  }

  slotPtr temp = box->slots;
  /*for(int i = 0; i < box->numSlots; i++){
    if (temp->message != NULL && temp->status == SLOT_TAKEN){
      break;
    }
    temp = temp->nextSlot;
  }*/


  if (strlen(temp->message) > msg_size){
    USLOSS_Console("MboxReceive(): Msg_size is too small. Halting...\n");
    USLOSS_Halt(1);
    return -1;
  }

  if (ProcTable[callerPID%50].pos != 0){
    for (int i = 0; i < ProcTable[getpid()%50].pos; i++){
      temp = temp->nextSlot;
    }
    memcpy(msg_ptr, temp->message, msg_size);
  }
  else {
    memcpy(msg_ptr, temp->message, msg_size); 
    box->slots = box->slots->nextSlot;
    slotPtr temp2 = box->slots;
    while (temp2->nextSlot != NULL){
      temp2 = temp2->nextSlot;
    }
    temp2->nextSlot = malloc(sizeof(*temp2));
    temp2 = temp2->nextSlot;
    temp2->mboxID = box->mboxID;
    temp2->status = SLOT_READY;
    temp2->message = malloc(MAX_MESSAGE);
  }
      
  occupiedSlots--;
  box->filledSlots--;
  int returnValue = strlen(temp->message) + 1;
  temp->status = SLOT_READY;

  // This code removes the first link of the the slot list and adds a new one to the end
  /**/
  UnblockSender(mbox_id);
  if (box->filledSlots < box->numSlots && box->s_blockCount > 0){
    AddToReceiveBlockList(mbox_id, callerPID);
    box->r_blockCount++;
    blockMe(RECEIVE_BLOCKED);
    box->r_blockCount--;
  }
  if (isZapped()){
    return -3;
  }
  return returnValue;
} /* MboxReceive */

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

/* AddSendToBlockList-----------------------------------------------------
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
} /* AddSendToBlockList */

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
  if (type == USLOSS_CLOCK_INT){
    MboxReceive(CLOCK_MBOX, &status, MAX_MESSAGE);
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

void check_kernel_mode(char *name)
{
  unsigned int mode;
  mode = USLOSS_PsrGet();
  if ((mode & USLOSS_PSR_CURRENT_MODE) == 0){
    USLOSS_Console("%s is not running in kernel mode. Halting...\n", name);
    USLOSS_Halt(1);
  }
}

void disableInterrupts()
{

}

void enableInterrupts()
{

}

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


