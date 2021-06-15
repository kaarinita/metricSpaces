
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

main (int argc, char **argv)

  { char s[1024],s2[1024];
    char *sp,*sp2;
    int c=0,pos;
    int N = atoi(argv[1]);
    double tot=0;
    pos=0;
    while (c!=-1)
      { int i = 0;
        while (((c=getchar())!='\n') && (c !=-1)) s[i++]=c;
	s[i] = 0;
	pos++;
	if (!(sp = strchr(s,' '))) continue;
	*sp++ = 0;
	i = 0;
        while (((c=getchar())!='\n') && (c !=-1)) s2[i++]=c;
	s2[i] = 0;
	if (!(sp2 = strchr(s2,'+'))) 
		{ fprintf(stderr,"Error --- word %s not followed by its idf\n",
			  s);
	          exit(1);
		}
	*sp2++ = 0;
	if (strcmp(s,s2))
		{ fprintf(stderr,"Error --- word %s not followed by its idf\n",
			  s);
	          exit(1);
		}
	printf ("%7i %.15f\n",pos,atoi(sp)*log(N/(double)atoi(sp2)));
	tot += atoi(sp)*log(N/(double)atoi(sp2));
      }
     printf ("       %f\n9999999 0\n",tot);
   }
