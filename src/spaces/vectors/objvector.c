
#include "../../obj.h"

#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>

typedef struct sEuclDB
   { float *nums;  /* coords all together */
     int nnums;	  /* number of vectors (with space for one more) */
     int coords;  /* coordinates */
     float (*df) (float *p1, float *p2, int k); /* distance to use */
   } EuclDB;

static int never = 1;
static EuclDB DB;

#define db(p) (DB.nums + DB.coords*(int)p)

	/* L2 distance */

static float L2D (float *p1, float *p2, int k)

    { register int i;
      float tot = 0,dif;
      for (i=0;i<k;i++) 
	{ dif = (p1[i]-p2[i]);
	  tot += dif*dif;
	}
      return sqrt(tot);
    }

static float L1D (float *p1, float *p2, int k)

    { register int i;
      float tot = 0,dif;
      for (i=0;i<k;i++) 
	{ dif = (p1[i]-p2[i]);
	  if (dif < 0) dif = -dif;
	  tot += dif;
	}
      return tot;
    }

static float LiD (float *p1, float *p2, int k)

    { register int i;
      float max = 0,dif;
      for (i=0;i<k;i++) 
	{ dif = (p1[i]-p2[i]);
	  if (dif < 0) dif = -dif;
	  if (dif > max) max = dif;
	}
      return max;
    }

float distanceInter (Obj o1, Obj o2)

   { return DB.df (db(o1),db(o2),DB.coords);
   }

Obj parseobj (char *p)

   { float *d = db(NewObj);
     int i,step;
     for (i=0;i<DB.coords-1;i++)
	{ sscanf (p,"%f,%n",d+i,&step); 
	  p += step;
	}
     sscanf (p,"%f",d+i); 
     return NewObj;
   }

void printobj (Obj obj)

   { int i;
     float *p = db(obj);
     for (i=0;i<DB.coords-1;i++) printf ("%f,",p[i]);
     printf ("%f\n",p[i]);
   }

int openDB (char *name)

   { FILE *f;
     struct stat sdata;
     int func;

     closeDB ();
     f = fopen (name,"r");
     stat (name,&sdata);
     fread (&func,sizeof(int),1,f);
     if (func == 1) DB.df = L1D;
     else if (func == 2) DB.df = L2D;
     else DB.df = LiD;
     fread (&DB.coords,sizeof(int),1,f);
     DB.nnums = (sdata.st_size - 2*sizeof(int))/sizeof(float)/DB.coords;
     DB.nums = malloc ((DB.nnums+1) * sizeof(float) * DB.coords);
     fread (DB.nums + DB.coords,
	    DB.nnums * sizeof(float) * DB.coords,1,f);
     fclose (f);
     return DB.nnums;
   }

void closeDB (void)

   { if (never) { DB.nums = NULL; never=0;}
     if (DB.nums == NULL) return;
     free (DB.nums);
     DB.nums = NULL;
   }

