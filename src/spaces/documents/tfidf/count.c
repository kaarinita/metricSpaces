
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

main ()

  { char s[1024],olds[1024];
    int c=0,cnt;
    olds[0] = 0;
    while (c!=-1)
      { int i = 0;
        while (((c=getchar())!='\n') && (c !=-1)) s[i++]=c;
	s[i] = 0;
	if (!strcmp(s,olds)) cnt++;
	else { if (olds[0]) printf ("%s %i\n",olds,cnt);
	       strcpy(olds,s); cnt=1; }
	       }
	       }

