
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/times.h>

// #include "obj.h"
typedef int Obj;
#define NewObj 0
#define NullObj (-1)

#define faleat (((unsigned)random())/((double)(unsigned)(1<<31)))
#define aleat(n) ((unsigned)((faleat)*(n)))

typedef struct sPalDB
   { char *pals;  /* words all together */
     char **ptrs;  /* pointers to each word */
     int npals;	  /* number of words (there is space for 1 more) */
     int csize;   /* allocated c size */
     int *c;
   } PalDB;

static int never = 1;
static PalDB DB;

int DBlen;

#define db(p) (DB.ptrs[(int)p])

void printobj (Obj obj)

   { printf ("%s\n",db(obj));
   }

int openDB (char *name)

   { char *ptr,*top;
     FILE *f;
     struct stat sdata;
     unsigned long dn;

     f = fopen (name,"r");
     stat (name,&sdata);
     DB.pals = malloc (sdata.st_size);
     DBlen = sdata.st_size;
     fread (DB.pals,sdata.st_size,1,f);
     fclose (f);
     ptr = DB.pals; top = ptr + sdata.st_size;
     dn = 0;
     while (ptr < top) 
	{ while (*ptr != '\n') ptr++;
	  dn++; *ptr++ = 0;
	}
     DB.ptrs = malloc ((dn+1)*sizeof(char*));
     dn = 0; ptr = DB.pals;
     DB.ptrs[0] = NULL;
     while (ptr < top) 
	{ DB.ptrs[++dn] = ptr;
	  while (*ptr++);
        }
     DB.npals = dn;
     return DB.npals;
   }

// ins and subs choose random chars from the vocabulary

static void perturb1 (char *o1, char *o2)

   { int l1 = strlen(o1);
     int opc = aleat(3);
     int pos;
     char c;
     if (opc == 1)  /* repl */
	{ pos = aleat(l1);
	  strcpy (o2,o1);
	  do { c = DB.pals[aleat(DBlen)]; } while (!c);
	  o2[pos] = c;
	}
     else if (opc == 2)  /* del */
	{ pos = aleat(l1);
	  memcpy (o2,o1,pos);
	  strcpy (o2+pos,o1+pos+1);
	}
     else   /* ins */
	{ pos = aleat(l1+1);
	  memcpy (o2,o1,pos);
	  do { c = DB.pals[aleat(DBlen)]; } while (!c);
	  o2[pos] = c;
	  strcpy (o2+pos+1,o1+pos);
	}
   }

void perturb (Obj obj, int k)

   { int i;
     char *o1,*o2,*oa;
     o1 = (char*)malloc (strlen(db(obj))+k+1);
     o2 = (char*)malloc (strlen(db(obj))+k+1);
     strcpy (o1,db(obj));
     for (i=0;i<k;i++)
	{ perturb1 (o1,o2);
	  oa = o1; o1 = o2; o2 = oa;
	}
     free (o2);
     db(NewObj) = o1;
   }

int main (int argc, char **argv)

  { int n,from,nq;
    int pert;
    char *kk;
    struct timeval t;

    if (argc < 6)
	{ fprintf (stderr,"Usage: genqueries <db file> <from> <nqueries> "
			    "<k> <perturb>\n  See the manual for details.\n");
	  exit(1);
	}
    n = openDB(argv[1]);
    from = atoi(argv[2]);
    nq = atoi(argv[3]);
    kk = argv[4];
    pert = atoi(argv[5]);
   
    gettimeofday (&t,NULL);
    srandom (t.tv_sec*t.tv_usec);

    while (nq--)
      { printf ("%s,",kk);
        perturb(from+1+aleat(n-from),pert);
        printobj(NewObj);
	free (db(NewObj));
      }
    printf("-0\n");
  }

