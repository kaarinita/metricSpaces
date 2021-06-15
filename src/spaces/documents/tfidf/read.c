
#include <math.h>
#include <stdio.h>

main (int argc, char **argv)

  { FILE *input = fopen(argv[1],"r");
    double tot,tfidf;
    unsigned int pos;
    while (1)
      { if (!fread(&pos,sizeof(int),1,input)) break;
        if (!fread(&tfidf,sizeof(double),1,input)) break;
        printf ("%i %f\n",pos,tfidf);
      }
     fclose(input);
   }
