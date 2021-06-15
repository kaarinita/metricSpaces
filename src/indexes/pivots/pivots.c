/**
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
**/

#include"pivots.h"

void prnstats (Index S);

Index build (char *dbname, int n, int *argc, char ***argv)

   { nn *tabla;
     int i,j,k;
     if (*argc < 1)
        { fprintf (stderr,"Usage: <program> <args> PIVOTS \n");
          exit(1);
        }
     tabla = malloc (sizeof(nn));
     tabla->descr = malloc (strlen(dbname)+1);
     strcpy (tabla->descr,dbname);
     tabla->coords = atoi((*argv)[0]);
     tabla->n = openDB(dbname);
     if (n && (n < tabla->n)) tabla-> n = n;
     if (tabla->n < tabla->coords) tabla->coords = tabla->n;
     tabla->piv = malloc (tabla->coords * sizeof(Obj));
     tabla->nums = malloc (tabla->n * tabla->coords * sizeof(Tdist));
     *argc -= 1; *argv += 1;
     
     k=0; i=0;
     while (k< tabla->coords) 
        { tabla->piv[i] = ++k;
	  i++;
	}
     k=0; i=0;
     while (k < tabla->n)
        { Obj obj = ++k;
	  for (j=0;j<tabla->coords;j++)
	      tabla->nums[i*tabla->coords + j] = distance (obj,tabla->piv[j]);
	  i++;
	}
     prnstats((Index)tabla); 	
     return (Index)tabla;
   }

void freeIndex (Index S, bool libobj)

   { nn *tabla = (nn*)S;
     free (tabla->descr);
     free (tabla->piv);
     free (tabla->nums);
     free (tabla);
     if (libobj) closeDB();
   }

void saveIndex (Index S, char *fname)

   { FILE *f = fopen(fname,"w");
     nn *tabla = (nn*)S;
   
     fwrite (tabla->descr,strlen(tabla->descr)+1,1,f);
     fwrite (&tabla->n,sizeof(int),1,f);
     fwrite (&tabla->coords,sizeof(int),1,f);
     fwrite (tabla->nums,tabla->n * tabla->coords,sizeof(Tdist),f);
     fclose (f);
   }

Index loadIndex (char *fname)

   { char str[1024]; char *ptr = str;
     FILE *f = fopen(fname,"r");
     nn *tabla = malloc (sizeof(nn));
     int i,k;
     while ((*ptr++ = getc(f)));
     tabla->descr = malloc (ptr-str);
     strcpy (tabla->descr,str);
     fread (&tabla->n,sizeof(int),1,f);
     fread (&tabla->coords,sizeof(int),1,f);
     tabla->piv = malloc (tabla->coords * sizeof(Obj));
     openDB(str);
     k=0; i=0;
     while (k < tabla->coords) 
        { tabla->piv[i] = ++k;
	  i++;
	}
     tabla->nums = malloc (tabla->n * tabla->coords * sizeof(Tdist));
     fread (tabla->nums,tabla->n * tabla->coords, sizeof(Tdist),f);
     fclose (f);
     return (Index)tabla;
   }


static Tdist DistMax (Tdist *p1, Tdist *p2, int k)

    { register int i;
      Tdist max = 0,dif;
      for (i=0;i<k;i++)
        { dif = (p1[i]-p2[i]);
          if (dif < 0) dif = -dif;
          if (dif > max) max = dif;
        }
      return max;
    }

static Tdist Dist1 (Tdist *p1, Tdist *p2, int k)

    { register int i;
      Tdist tot = 0,dif;
      for (i=0;i<k;i++)
        { dif = (p1[i]-p2[i]);
          if (dif < 0) dif = -dif;
          tot += dif;
        }
      return tot;
    }

int search (Index S, Obj obj, Tdist r, int show)

   { int i,rep=0;
     nn *tabla = (nn*)S;
     Tdist *vals = malloc (tabla->coords * sizeof(Tdist));
     for (i=0;i<tabla->coords;i++) 
	 { vals[i] = distance (obj,tabla->piv[i]);
	   if (vals[i] <= r)
	      { rep++;
		if (show) printobj(tabla->piv[i]);
	      }
	 }
     for (;i<tabla->n;i++) 
	 if (DistMax (vals,tabla->nums + tabla->coords * i,tabla->coords) <= r)
	    { 
	      if (distance (obj,i+1) <= r)
		 { rep++;
		   if (show) printobj(i+1);
		 }
	    }
     free (vals);
     return rep;
   }

typedef struct
   { int ptr;
     Tdist d1;
   } Tod;

int comp (const void *a, const void  *b)

   { Tod *x,*y;
     x = (Tod *) a;
     y = (Tod *) b;

     if (x->d1 > y->d1) return 1;
     if (x->d1 < y->d1) return -1;
     return 0;
   }

Tdist searchNN (Index S, Obj obj, int k, bool show)

   { int i;
     nn *tabla = (nn*)S;
     Tdist *vals = malloc (tabla->coords * sizeof(Tdist));
     Tcelem res = createCelem(k);
     Tdist mdif;
     Tod *od = malloc (tabla->n * sizeof(Tod));
     for (i=0;i<tabla->coords;i++) 
	 { vals[i] = distance (obj,tabla->piv[i]);
	   addCelem (&res,tabla->piv[i],vals[i]);
	 }
     for (i=tabla->coords;i<tabla->n;i++) 
	{ od[i-tabla->coords].ptr = i+1;
	  od[i-tabla->coords].d1 = Dist1(vals,tabla->nums + tabla->coords * i,
					   tabla->coords);
	}
     qsort (od,tabla->n - tabla->coords, sizeof(Tod), comp);
     for (i=0;i<tabla->n - tabla->coords;i++) 
//	 if (!outCelem(&res,DistMax(vals,tabla->nums + tabla->coords * od[i].ptr,
	 if (!outCelem(&res,DistMax(vals, tabla->nums + (tabla->coords * (od[i].ptr-1)),
			tabla->coords)))
	    { Tdist d;
	      d = distance (obj,od[i].ptr);
	      addCelem (&res,od[i].ptr,d);
	    }
     free (vals); free(od);
     if (show) showCelem (&res);
     mdif = radCelem(&res);
     freeCelem (&res);
     return mdif;
   }

void prnstats (Index S)

   { nn *tabla = (nn*)S;
     printf ("number of elements: %i\n",tabla->n);
     printf ("number of pivots: %i\n",tabla->coords);
   }
