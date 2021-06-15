
#ifndef OBJINCLUDED
#define OBJINCLUDED

	/* object database. for now, only one active database at a time will
	   be permitted. all the operations fail if there is no loaded DB */

typedef int Obj;	/* object id */

#define NullObj (-1)   /* null object */

#define NewObj 0      /* new object */

#include "basics.h"

	/* loads a DB given its name, and returns its size. if there is
	   another one already open, it will be closed first */
int openDB (char *name);

	/* frees the currently open DB, if any */
void closeDB (void);

	/*(internal) computes the distance between two objects */ 
#ifdef CONT
float distanceInter (Obj o1, Obj o2);
#else
int distanceInter (Obj o1, Obj o2);
#endif

      /* exported, computes distance and does the accounting */
#define distance(o1,o2) (numDistances++,distanceInter(o1,o2))

	/* returns an object identifier from description s. if the object
	   does not belong to the DB, it must be identified as NewObj */
Obj parseobj (char *str);

	/* prints obj in a user-friendly way, terminated with a newline */
void printobj (Obj obj);

#endif
