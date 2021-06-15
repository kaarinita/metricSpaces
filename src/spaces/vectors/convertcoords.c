
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/times.h>

main(int argc, char **argv)

  { int n,k,d,g;
    struct timeval t;
    int i,j;
    float r;
    FILE *f;

    if (argc < 3)
	{ fprintf (stderr,"Usage: convertcoords ascii-file binary-file\n");
	  exit(1);
	}
    f = fopen(argv[1],"r");
    g = open (argv[2],O_TRUNC | O_CREAT | O_WRONLY, S_IREAD | S_IWRITE);
    fscanf (f,"%i %i %i\n",&k,&n,&d);
    write (g,&d,sizeof(d));
    write (g,&k,sizeof(k));
    for (i=0;i<n;i++)
	for (j=0;j<k;j++)
	   { fscanf(f,"%f",&r);
	     write (g,&r,sizeof(r));
	   }
    close (g); fclose (f);
  }
