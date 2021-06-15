
#include <math.h>
#include <stdio.h>

main (int argc, char **argv)

  { FILE *output = fopen(argv[1],"w");
    double tot,tfidf;
    unsigned int pos;
    scanf("       %lf\n",&tot);
    while (1)
      { scanf("%i %lf\n",&pos,&tfidf);
        if (pos == 9999999) break;
	fprintf(output,"%i %lf\n",pos,tfidf/tot);
      }
    fclose(output);
   }
