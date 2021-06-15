#include "../../obj.h"

#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>


Tdist similarity_cos(Tdist *v1, Tdist *v2, long NN); 
Tdist similarity_euc(Tdist *v1, Tdist *v2, long NN);

Tdist dato(Obj obj, int pos);
int datoID(Obj obj);


typedef struct sEuclDB
   { char *descr;   /* descripcion */
     int nnums;	  /* nro pals (hay espacio para 1 mas alocado) */
     int nnums_test;
     Tdist **vector;
     long *id;
     int tam_vector;
     char *descr_test;
   } EuclDB;

static EuclDB *DB = NULL;

#define db(p) ( DB->vector[(int)p])


	/* L2 distance */
#if 0
static Tdist L2D (Tdist *p1, Tdist *p2, int k)

    { register int i;
      Tdist tot = 0,dif;
      for (i=0;i<k;i++) 
	{ dif = (p1[i]-p2[i]);
	  tot += dif*dif;
	}
      return sqrt(tot);
    }
#endif
/*
static Tdist L1D (Tdist *p1, Tdist *p2, int k)

    { register int i;
      Tdist tot = 0,dif;
      for (i=0;i<k;i++) 
	{ dif = (p1[i]-p2[i]);
	  if (dif < 0) dif = -dif;
	  tot += dif;
	}
      return tot;
    }

static Tdist LiD (Tdist *p1, Tdist *p2, int k)

    { register int i;
      Tdist max = 0,dif;
      for (i=0;i<k;i++) 
	{ dif = (p1[i]-p2[i]);
	  if (dif < 0) dif = -dif;
	  if (dif > max) max = dif;
	}
      return max;
    }
 */

Tdist distanceInter (Obj obj1, Obj obj2)
   {    
	Tdist d = (Tdist) similarity_euc(db(obj1), db(obj2), DB->tam_vector);
	return d;
   }

Tdist distanceCoord (Obj obj1, Obj obj2, int C)
   {    
	Tdist d = (Tdist) similarity_euc(db(obj1), db(obj2), (long)C);
	return d;
   }

Tdist distance_piv (Obj obj1, Obj obj2, int x, int C)
   { 
   
	Tdist d = (Tdist) similarity_euc(db(obj1), db(obj2), C);
	return d;
   }


Obj parseobj (char *p)

   { 
	int i, j=0, k;
	char f[255];
	int largo = strlen(p);
        for(i=0; i<DB->tam_vector; i++)
                {
			k=0;
			while((p[j] !='\0' && p[j] != ',') && j < largo )
				f[k++] = p[j++]; 
			j++;
                        sscanf(f, "%f", &DB->vector[0][i]);
                }
		k=0;
		while(p[j] !='\0' && p[j] != ',' && p[j] !='\n')
			f[k++] = p[j++]; 
                sscanf(f, "%ld", &DB->id[0]);

//     return (Obj) atoi(p);
	return (Obj) 0;
   }

void printobj (Obj obj)

   { /*int i;
     Tdist *p = db(obj);*/
     printf("[pos=%d,ID=%ld]\n", (int)obj, DB->id[(int)obj]);
/*     printf ("(");
     for (i=0;i<DB->tam_vector;i++) printf ("%f,",p[i]);
        printf ("%f)\n",p[i]);
  */
   }

int openDB (char *descr)

   { 
     FILE *f;
     int func, tam_vector, i, j;
     char fname[1024];
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
                fscanf(f, "%ld\n", &id);
                DB->id[i] = id;
        }
    fclose(f);
    return DB->nnums;
}
	
char *descrDB (void)
 
   { return DB->descr;
   }

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


Tdist similarity_cos(Tdist *v1, Tdist *v2, long NN) {
    long i;
    Tdist dp,dp1,dp2, res;
	
    for( i=0,dp=0.0,dp1=0.0,dp2=0.0; i<NN; i++ ) {
        dp+=v1[i]*v2[i];
        dp1+=v1[i]*v1[i];
	dp2+=v2[i]*v2[i];
    }
    res = ( dp/sqrt(dp1*dp2) );
    return res;
}

					
Tdist similarity_euc(Tdist *v1, Tdist *v2, long NN) {
        long i;
        Tdist dist;
	Tdist *v = malloc(DB->tam_vector * sizeof(Tdist));

    /* no modifica ningun vector */
        for(i=0;i<NN;i++)
            v[i]=v1[i]-v2[i];
        for(i=0,dist=0.0;i<NN;i++)
            dist+=v[i]*v[i];

	free(v);

    return(  (sqrt(dist)) );	// el rango de valores esta entre 0 y 1
}

Tdist dato(Obj obj, int pos)
{
    return DB->vector[(int) obj][pos];
}

int datoID(Obj obj)
{
    return DB->id[(int) obj];
}
