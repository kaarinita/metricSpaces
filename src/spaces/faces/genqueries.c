#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>


typedef int Obj;
typedef float Tdist;
typedef unsigned int ulong;

#define db(p) ( DB->vector[(int)p])
#define faleat (((unsigned)random())/((double)(unsigned)(1<<31)))
#define aleat(n) ((unsigned)((faleat)*(n)))


#define NewObj 0

typedef struct sEuclDB
   { char *descr;   /* descripcion */
     int nnums;	  /* nro pals (hay espacio para 1 mas alocado) */
     int nnums_test;
     float **vector;
     long *id;
     int tam_vector;
     char *descr_test;
   } EuclDB;

static EuclDB *DB = NULL;
void closeDB (void)
{ 
    int i;
    if (DB == NULL) return;
	for (i=0; i<DB->nnums; i++)
        free(DB->vector[i]);
	free (DB->descr);
	free (DB->id);
	free(DB);
	DB = NULL;
}


void printobj (Obj obj)

   { int i;
     Tdist *p = db(obj);
     for (i=0;i<DB->tam_vector;i++) printf ("%f,",p[i]);
        printf ("%f\n",p[i]);
   }


int openDB (char *descr)

   { 
	   char *ptr,*top;
	   FILE *f;
	   struct stat sdata;
	   int func, tam_vector, i, j, id2;
	   char fname[1024], t[254];
	   ulong dn;
	   long id;
	   
	   closeDB ();
	   DB = (struct sEuclDB *) malloc(sizeof(struct sEuclDB));
	   DB->descr = malloc((strlen(descr)+1) * sizeof(char));
	   strcpy (DB->descr,descr);
	   
	   strcpy(fname, descr);
	   f = fopen (fname,"r");
	   
	   fscanf(f, "%d %d\n", &func, &tam_vector);
	   DB->nnums = func;             //num de vectores
	   DB->tam_vector = tam_vector;
	   DB->vector = (Tdist **) malloc((DB->nnums+1) * sizeof(Tdist *));
	   DB->id = (long *) malloc((DB->nnums+1) * sizeof(long));
	   
	   // la posicion cero sera para las pruebas!! importante
	   DB->vector[0] = (Tdist *)malloc(sizeof(Tdist)*tam_vector);
	   for(i=1; i<=func; i++)
	   {
		   DB->vector[i] = (Tdist *)malloc(sizeof(Tdist)*tam_vector);
		   for(j=0; j<tam_vector; j++)
		   {
			   fscanf(f, "%f,", &DB->vector[i][j]);
		   }
		   fscanf(f, "%d\n", &id);
		   DB->id[i] = id;
	   }
	   fclose(f);
	   return DB->nnums;
   }

void perturb (int from, float r)
   { 
     int i;
     Obj orig = from+1+aleat(DB->nnums-from);
     memcpy (db(NewObj),db(orig),DB->tam_vector * sizeof(float));
     i = aleat(DB->tam_vector);
     if (aleat(2)) db(NewObj)[i] += r; else db(NewObj)[i] -= r;
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
//	free (db(NewObj));
      }
    printf("-0\n");
  }


										
