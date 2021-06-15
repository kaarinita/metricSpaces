
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/times.h>

#define faleat (((unsigned)random())/((double)(unsigned)(1<<31)))
#define aleat(n) ((unsigned)((faleat)*(n)))

main(int argc, char **argv)

  { int n,k,d;
    struct timeval t;
    int i,j;
    FILE *f;

    if (argc < 5)
	{ fprintf (stderr,"Usage: gencoords <123> dim n file\n"
			  "<123> = L1,L2 or Linfinite\n");
	  exit(1);
	}
    d = atoi(argv[1]);
    k = atoi(argv[2]);
    n = atoi(argv[3]);
    f = fopen (argv[4],"w");
    fprintf (f,"%i %i %i\n",k,n,d);
    gettimeofday (&t,NULL);
    srandom (t.tv_sec*t.tv_usec);
    for (i=0;i<n;i++)
	for (j=0;j<k;j++)
	   { float r = faleat;
	     fprintf (f,"%f%c",r,j<k-1?' ':'\n');
	   }
    fclose (f);
  }
