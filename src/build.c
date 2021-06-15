
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/times.h>
#include "obj.h"
#include "index.h"

extern long long numDistances;

int main (int argc, char **argv)

   { Index S;
     unsigned long n,np;
     struct stat sdata;
     struct tms t1,t2;
     char *fname, *finame;
    
     if (argc < 4)
	{ fprintf (stderr,"Usage: %s db-name size idxfile ... (extra index args)\n",argv[0]);
	  exit(1);
	}
     fname = argv[3];
     finame = argv[1];
     np = atoi(argv[2]);
     n = openDB(finame);
     if (!np || (np > n)) np = n;
     argc -= 4; argv += 4;

     printf ("indexing %li objects out of %li...\n",np,n);
     times (&t1);
     numDistances = 0;
     S = build (finame,np, &argc, &argv);
     times (&t2);
     printf ("finished... %lld distances computed\n",numDistances);
     printf ("user: %0.2f, system: %0.2f\n",
	     (t2.tms_utime-t1.tms_utime)/60.0,
	     (t2.tms_stime-t1.tms_stime)/60.0);
     printf ("\n\nsaving...\n");
     saveIndex (S,fname);
     stat (fname,&sdata);
     printf ("saved... %lli bytes\n",(long long)sdata.st_size);
     if (argc > 0) printf ("Obs: %i extra arguments ignored\n",argc);
	   return 0;
   }

