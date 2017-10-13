/*
 * These are the definitions for phase 3 of the project
 */

#ifndef _PHASE3_H
#define _PHASE3_H

#define MAXSEMS         200

typedef struct p3proc * procPtr;

typedef struct p3proc {
	int 		pid;
	int 		children;
	int 		(* startFunc) (char *);
	char * 		arg;	
	procPtr		child;
	procPtr 	nextSibling;
} p3proc;

#endif /* _PHASE3_H */

