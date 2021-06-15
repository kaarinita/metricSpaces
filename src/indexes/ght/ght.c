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


#include "ght.h"

void prnstats (Index S);

// chooses centers at random, but there is commented-out code to choose 
// far-away centers
#if 0
static int farthest (arrdata queue, Obj *obj, int nobj, unsigned long *cmp)

   { int i,j,f;
     Tdist maxd = -1;
     for (i=0;i<queue.ndata;i++)
	{ int d = 0;
          for (j=0; j<nobj; j++)
	      { d += distance(queue.data[i],obj[j]); (*cmp)++; }
	  if (d>maxd) { maxd = d; f = i; }
	}
     return f;
   }
#endif

static void Select (gnode *N, int arity)

   { 
     int i;
	/* si son pocos lo hago directamente root */
     if (N->queue.ndata <= arity)
        { N->root = realloc (N->queue.data, N->queue.ndata*sizeof(Obj));
	  N->arity = N->queue.ndata;
	  N->queue.data = NULL;
	  N->queue.ndata = 0;
	  return;
	}
     N->root = malloc (arity*sizeof(Obj));
/*     obj0 = N->queue.data[0]; 
     f = farthest(N->queue,&obj0,1,cmp);
     N->root[0] = N->queue.data[f];
     N->queue.data[f] = N->queue.data[--N->queue.ndata];
     for (i=1;i<arity;i++)
	{ f = farthest (N->queue,N->root,i,cmp);
	  N->root[i] = N->queue.data[f];
	  N->queue.data[f] = N->queue.data[--N->queue.ndata];
	}
*/
	/* random */
     for (i=0;i<arity;i++)
         N->root[i] = N->queue.data[--N->queue.ndata];
   }

#define BLK 5
#define space(ptr,size) \
 { if (((size) % BLK) == 0) \
      { (ptr) = realloc ((ptr),((size)+BLK)*sizeof(Obj)); } }

static void addqueue(gnode *N, Obj obj)
  { space(N->queue.data, N->queue.ndata);
    N->queue.data[N->queue.ndata++] = obj;
  }

static void distr (gnat *G, gnode *N)

   { int i,j;
     Select (N, G->arity);
     if (N->queue.data == NULL)  /* hoja */
	{ N->children = NULL;
	  N->maxd = NULL;
	  return;
	}
     N->arity = G->arity;
     N->children = malloc (N->arity * sizeof(gnode));
     N->maxd = malloc (N->arity * sizeof(Tdist));
     for (i=0; i<N->arity; i++) 
	{ N->maxd[i] = 0;
	  N->children[i].queue.ndata = 0;
	  N->children[i].queue.data = NULL;
	}

     for (i=0; i<N->queue.ndata; i++)
	{ int f=0;
          Tdist mind = -1;
	  for (j=0; j<N->arity; j++)
	     { Tdist dist = distance (N->queue.data[i],N->root[j]); 
	       if ((mind==-1)||(dist < mind)) { mind = dist; f = j; }
	     }
	  addqueue (&N->children[f],N->queue.data[i]);
	  if (N->maxd[f] < mind) N->maxd[f] = mind;
	}
     free (N->queue.data); N->queue.data = NULL; N->queue.ndata = 0;
     for (i=0;i<N->arity;i++)
	 distr (G,&N->children[i]);
   }

Index build (char *dbname, int n, int *argc, char ***argv)

   {    
	gnat *G;
	int i;

     if (*argc < 1)
        { fprintf (stderr,"Usage: <program> <args> ARITY\n");
          exit(1);
        }
     G = malloc (sizeof(gnat));

     G->descr = malloc (strlen(dbname)+1);
     strcpy (G->descr,dbname);
     G->np = openDB(G->descr);			
     if (n && (n < G->np)) G->np = n; 
     G->arity = atoi((*argv)[0]); (*argc)--; (*argv)++;
     G->tree.queue.ndata = 0;
     G->tree.queue.data = NULL;
	i = 0;
     while (i < G->np )
	 { 
	   i++;
	   addqueue (&G->tree,i);
	 }
     distr (G,&G->tree);
     prnstats((Index)G); 
     return (Index)G;
   }

static void freegnat (gnode *node)

   { free (node->root);
     if (node->children != NULL)
	{ int i;
          for (i=0;i<node->arity;i++)
              freegnat (&node->children[i]);
          free (node->children);
          free (node->maxd);
        }
   }

void freeIndex (Index S, bool libobj)

   { gnat *G = (gnat*)S;
     free (G->descr);
     freegnat (&G->tree);
     free (G);
     if (libobj) closeDB();
   }

static void savenode (gnode *node, FILE *f)

   { char hoja = (node->children == NULL) ? '1' : '0';
     int i;
     putc (hoja,f);
     fwrite (&node->arity,sizeof(int),1,f);
     for (i=0; i<node->arity; i++)
        fwrite(&node->root[i], sizeof(int),  1, f);
     if (node->children != NULL)
	{ int i;
     	  fwrite (node->maxd,sizeof(Tdist),node->arity,f);
          for (i=0;i<node->arity;i++)
     	      savenode (&node->children[i],f);
        }
   }

void saveIndex (Index S, char *fname)

   {
     gnat *G = (gnat*)S;
     FILE *f = fopen (fname,"w");
     fwrite (G->descr,strlen(G->descr)+1,1,f);
     fwrite (&G->arity,sizeof(int),1,f);
     savenode (&G->tree,f);
     fclose (f);
   }

static void loadnode (gnode *node, FILE *f)

   { char hoja;
     int i;
     hoja = getc(f);
     fread (&node->arity,sizeof(int),1,f);
     node->root = malloc (node->arity * sizeof(Obj));
     for (i=0; i<node->arity; i++)
	fread(&node->root[i], sizeof(int), 1, f);     
     if (hoja == '1') /* hoja */
        { node->children = NULL;
 	  node->maxd = NULL;
	  return;
	}
     node->children = malloc (node->arity * sizeof(gnode));
     node->maxd = malloc (node->arity * sizeof(Tdist));
     fread (node->maxd,sizeof(int),node->arity,f);
     for (i=0;i<node->arity;i++)
     	 loadnode (&node->children[i],f);
   }

Index loadIndex (char *fname)  
	/* OJO: no preparado para agregar cosas (mult de BLK) */

   { 
     char str[1024]; char *ptr = str;
     gnat *G = malloc (sizeof(gnat));
     FILE *f = fopen (fname,"r");
     
     while ((*ptr++ = getc(f)));
     G->descr = malloc (ptr-str);
     strcpy (G->descr,str);
     
     G->np = openDB (str);
     fread (&G->arity,sizeof(int),1,f);
     loadnode (&G->tree,f);
     fclose (f);
     return (Index)G;
   }

static int _search (gnode *node, Obj obj, Tdist r, bool show)

   { int i,f;
     Tdist mind = -1;
     int rep = 0;
     int ccmp = 0;
     Tdist *dists = malloc (node->arity*sizeof(Tdist));;
     for (i=0; i<node->arity; i++)
	{ dists[i] = distance (obj,node->root[i]); ccmp++;
	  if (dists[i] <= r)
	     { rep++;
	       if (show) printobj (node->root[i]);
	     }
	  if ((mind==-1)||(dists[i] < mind)) { mind = dists[i]; f = i; }
	}
	/* ahora sabemos el mas cercano. meternos en todos los que puedan
	   ser */
     if (node->children == NULL) /* hoja */
	{ free (dists); return rep; } 
	
     for (i=0; i<node->arity; i++)
	{ if ((dists[i]-r <= mind+r) && (dists[i]-r <= node->maxd[i]))
	     rep += _search (&node->children[i],obj,r,show);
	}
     free (dists);
     return rep;
   }

int search (Index S, Obj obj, Tdist r, bool show)
 
   { gnat *G = (gnat*)S;
     return _search (&G->tree,obj,r,show);
   }

typedef struct
   { int ptr;
     Tdist dist;
   } Tod;


int comp (const void *a, const void *b)
  {
   Tod *x, *y;
    x = (Tod *)a;
    y = (Tod *)b;

   if (x->dist > y->dist) return 1;
     else if (x->dist < y->dist) return -1;
     else return 0;
   }

static void _searchNN (gnode *node, Obj obj, Tcelem *res)


   { int i;
     Tdist mind;
     int ccmp = 0;
     Tod *od = malloc (node->arity*sizeof(Tod));;
     for (i=0; i<node->arity; i++)
        { od[i].dist = distance (obj,node->root[i]); ccmp++;
          od[i].ptr = i;
	  addCelem (res,node->root[i],od[i].dist);
        }
     if (node->children == NULL)   /* hoja */
	{ free (od); return; }
     qsort (od,node->arity,sizeof(Tod),comp);
     mind = od[0].dist;
        /* ahora sabemos el mas cercano. meternos en todos los que puedan
           ser */
     for (i=0; i<node->arity; i++)
         { if (outCelem(res,(od[i].dist-mind)/2)) break;
           if ((radCelem(res) == -1) ||
	       (od[i].dist-radCelem(res) <= node->maxd[od[i].ptr]))
              _searchNN (&node->children[od[i].ptr],obj,res);
         }
     free (od);
   }

Tdist searchNN (Index S, Obj obj, int k, bool show)

   { gnat *G = (gnat*)S;
     Tdist mdif;
     Tcelem res = createCelem(k);
     _searchNN (&G->tree,obj,&res);
     if (show) showCelem (&res);
     mdif = radCelem(&res);
     freeCelem (&res);
     return mdif;
   }

static int height (gnode *node)

   { int i;
     int max;
     if (node->children == NULL) return 1;
     else
        { max = 0;
          for (i=0;i<node->arity;i++)
              { int hch = height (&node->children[i]);
                if (hch > max) max = hch;
              }
          return 1+max;
        }
   }

static int numnodes (gnode *node)

   { int i,ret=1;
     if (node->children == NULL) return 0;
     for (i=0;i<node->arity;i++)
         ret += numnodes(&node->children[i]);
     return ret;
   }

static int numbucks (gnode *node)  /* de hojas */

   { int i,ret=0;
     if (node->children == NULL) return 1;
     for (i=0;i<node->arity;i++)
         ret += numbucks(&node->children[i]);
     return ret;
   }

static int sizebucks (gnode *node)

   { int i,ret=0;
     if (node->children == NULL) return node->arity;
     for (i=0;i<node->arity;i++)
         ret += sizebucks(&node->children[i]);
     return ret;
   }

static int profbucks (gnode *node, int depth)

   { int i,ret=0;
     if (node->children == NULL) return depth;
     for (i=0;i<node->arity;i++)
         ret +=profbucks(&node->children[i],depth+1);
     return ret;
   }

void prnstats (Index S)

   { gnat *G = (gnat*)S;
     int nbucks = numbucks(&G->tree);
     int sbucks = sizebucks(&G->tree);
     int nnodes = numnodes(&G->tree);
     printf ("number of elements: %i\n",nnodes*G->arity+sbucks);
     printf ("arity: %i\n",G->arity);
     printf ("maximum height: %i\n",height(&G->tree));
     printf ("number of nodes: %i\n",nnodes);
     printf ("number of leaves: %i\n",nbucks);
     printf ("average size of leaves: %0.2f\n", sbucks/(double)nbucks);
     printf ("average depth of leaves: %0.2f\n",
                        profbucks(&G->tree,0)/(double)nbucks);
   }

