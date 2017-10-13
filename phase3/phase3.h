/*
 * These are the definitions for phase 3 of the project
 */

#ifndef _PHASE3_H
#define _PHASE3_H

#define MAXSEMS         200

typedef struct p3proc {
	int 	pid;
	int 	(* startFunc) (char *);
	char * 	arg;
} p3proc;

#endif /* _PHASE3_H */

