#include <sys/types.h>
#include <sys/stat.h>
#include <sys/times.h>
#include "obj.h"
#include "index.h"

extern long long numDistances;

int main (int argc, char **argv)

   { char str[10000];
     Index S;
     struct stat sdata;
     struct tms t1,t2, tt1, tt2;
     int numQueries = 0;
    
     if (argc < 2)
        { fprintf (stderr,"Usage: %s index-file factor\n",argv[0]);
          exit(1);
        }

     fprintf (stderr,"reading index...\n");
     S = loadIndex (argv[1]);
     stat (argv[1],&sdata);
     fprintf (stderr,"read %i bytes\n",sdata.st_size);

     numDistances = 0;
	times(&tt1);
     while (true)
        { Obj qry;
	  bool fixed;

          if (scanf ("%[0123456789-id.]s",str) == 0) break;
	  if (!strcmp(str,"-0")) break; // -0 -> terminate

          if (str[0] == 'i') // insert
		  fixed = true;
	  else if(str[0] == 'd')	// delete
		fixed = false;
	  else
		break;		// not valid option

	if (getchar() != ',') break; // syntax error
	if (scanf ("%[^\n]s",str) == 0) break; // syntax error
	if (getchar() != '\n') break; // syntax error
	qry = parseobj (str);

	numQueries++;
               if (fixed)
	       { times(&t1);
		  insert(S,qry,false);
	          times(&t2);
	       }
	  else
	     { times(&t1);
	       delete(S,qry,false);
	       times(&t2);
	     }
	printf ("time p/q user: %0.4f, system: %0.4f\n",
		(t2.tms_utime-t1.tms_utime)/60.0,
		(t2.tms_stime-t1.tms_stime)/60.0);	

	}
	times(&tt2);
	printf ("time user: %0.4f, system: %0.4f\n",
		(tt2.tms_utime-tt1.tms_utime)/60.0,
		(tt2.tms_stime-tt1.tms_stime)/60.0);	
     fprintf(stderr,"Total distances per query: %f\n",
		numDistances/(float)numQueries);
     fprintf (stderr,"freeing...\n");
     freeIndex (S,true);
     fprintf (stderr,"done\n");
	return 0;
   }
