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

#include "sat.h"

void prnstats (Index S);

static void addHeap (heapElem *heap, int *hsize, int id, 
		     Tdist dist, Tdist lbound,Tdist mind)

   { int hs;
     heap--;
     hs = ++*hsize;
     while (hs > 1)
	{ int hs2 = hs>>1;
	  if (heap[hs2].lbound <= lbound) break;
	  heap[hs] = heap[hs2];
	  hs = hs2;
	}
     heap [hs].id = id; heap[hs].dist = dist;
     heap[hs].lbound = lbound; heap[hs].mind = mind;
   }

static heapElem extrHeap (heapElem *heap, int *hsize)

   { int i,dosi,newi,hs;
     heapElem ret = heap [0], tmp;
     heap[0] = heap[--*hsize];
     hs = *hsize;
     heap--;
     i = 1; dosi = i<<1;
     while (dosi <= hs)
        { newi = dosi;
          if ((dosi < hs) && (heap[dosi+1].lbound <heap[dosi].lbound))
             newi++;
          if (heap[newi].lbound <heap[i].lbound)
             { tmp = heap[newi]; heap[newi] = heap[i]; heap[i] = tmp;
               i = newi; dosi = i<<1;
             }
          else break;
        }
     return ret;
   }


#define BLK 5
#define space(ptr,size) \
 { if (((size) % BLK) == 0) \
      { (ptr) = realloc ((ptr),((size)+BLK)*sizeof(*(ptr))); } }

static int newnode(grafo *G, Obj obj) 
  { space (G->nodos,G->nnodos); 
    G->nodos[G->nnodos].obj = obj;
    G->nodos[G->nnodos].vec.num = NULL; 
    G->nodos[G->nnodos].vec.nnum = 0; 
    G->nodos[G->nnodos].queue.data = NULL; 
    G->nodos[G->nnodos].queue.ndata = 0; 
    return G->nnodos++;
  }

static void newvec(grafo *G, int node, Obj obj)
  { int pos = newnode (G,obj);
    nodo *N = &G->nodos[node];
    space (N->vec.num,N->vec.nnum);
    N->vec.num[N->vec.nnum] = pos;
    N->vec.nnum++;
  }

static void newqueue(grafo *G, int nodo, Obj obj, Tdist dist)
  { qelem *nq;
    space(G->nodos[nodo].queue.data, G->nodos[nodo].queue.ndata);
    nq = &G->nodos[nodo].queue.data[G->nodos[nodo].queue.ndata];
    nq->elem = obj;
    nq->dist = dist;
    nq->who = -1;
    G->nodos[nodo].queue.ndata++;
  }

#ifdef DISCR

static qelem *sort (qelem *r, int lo, int up)

   { Tdist max = 0;
     int *qty;
     int i,j,pos;
     int n = up+1;
     qelem *nr;
     for (i=0;i<n;i++) if (r[i].dist > max) max = r[i].dist;
     qty = malloc ((max+1)*sizeof(int));
     for (j=0;j<=max;j++) qty[j] = 0;
     for (i=0;i<n;i++) qty[r[i].dist]++;
     pos = 0;
     for (j=0;j<=max;j++) { int pj = pos; pos += qty[j]; qty[j] = pj; }
     nr = malloc (n*sizeof(qelem));
     for (i=0;i<n;i++) nr[qty[r[i].dist]++] = r[i];
     free (r); free (qty);
     return nr;
   }

#else

static void bubble (qelem *r, long lo, long up)

   { long i,j;
     qelem temp;
     for (i=lo+1;i<=up;i++)
        { j = i;
          while ((j > lo) && (r[j].dist < r[j-1].dist))
              { temp = r[j];
                r[j] = r[j-1];
                r[--j] = temp;
              }
        }
   }

static qelem *sort (qelem *r, long lo, long up)

   { long i, j;
     qelem tempr;
     while (lo + 30 < up)
        { i = lo;
          j = up;
          tempr = r[lo];
          while (i < j)
              { while (tempr.dist < r[j].dist) j--;
                r[i] = r[j];
                while ((i<j) && (tempr.dist >= r[i].dist)) i++;
                r[j] = r[i];
              }
          r[i] = tempr;
          if (i-lo < up-i)
               { sort(r,lo,i-1); lo = i+1; }
          else { sort(r,i+1,up); up = i-1; }
       }
     bubble (r,lo,up);
     return r;
   }

#endif

static void distr (grafo *G, int n)

   { int i,j,ni;
     nodo *N = &G->nodos[n];
     qelem *q;
     N->queue.data = sort (N->queue.data,0,N->queue.ndata-1);
     N->maxd = (N->queue.ndata == 0) ? 0 : N->queue.data[N->queue.ndata-1].dist;
     ni = 0;
     for (i=0;i<N->queue.ndata;i++)
	{ q = &N->queue.data[i];
	  for (j=0;j<N->vec.nnum;j++)
	      { Tdist d = distance (N->queue.data[i].elem,
				  G->nodos[N->vec.num[j]].obj); 
		if (d <=q->dist)
		   { q->dist = d; q->who = j;
		   }
	      }
	  if (q->who == -1)   /* agregarlo como vecino */
	     { newvec (G,n,N->queue.data[i].elem);
	       N = &G->nodos[n];
	     }
	  else /* se queda en la queue */
	     { N->queue.data[ni++] = N->queue.data[i];
	     }
	}
	/* mandar a los demas al mejor vecino */
     for (i=0;i<ni;i++)
	{ q = &N->queue.data[i];
	  for (j=q->who+1;j<N->vec.nnum;j++)
	      { Tdist d = distance (N->queue.data[i].elem,
			     G->nodos[N->vec.num[j]].obj);
	        if (d <=q->dist) { q->dist = d; q->who = j;}
	      }
          newqueue (G,N->vec.num[q->who],q->elem,q->dist);
	}
     free (N->queue.data); N->queue.data = NULL; N->queue.ndata = 0;
     for (j=0;j<G->nodos[n].vec.nnum;j++)
	{ /* printf ("%i	de %i\n",G->nodos[n].vec.num[j],G->nnodos);*/
	  distr (G,G->nodos[n].vec.num[j]);
	}
   }

Index build (char *dbname, int n, int *argc, char ***argv)

   { grafo *G = malloc (sizeof(grafo));
     int i;
    
     Obj obj0;

     G->descr = malloc (strlen(dbname)+1);
     strcpy (G->descr,dbname);
     G->nodos = NULL; G->nnodos = 0;
     G->np =  openDB(dbname);
     if (n && (n < G->np)) G->np = n;
     i = 1;
     obj0 = i;
     newnode (G,obj0);
     while (i < G->np)
	 { 
	    i++; 
	    newqueue (G,0,i,distance(obj0,i));
	 }
     distr (G,0);
     prnstats((Index)G); 
     return (Index)G;
   }

void freeIndex (Index S, bool libobj)

   { int n;
     grafo *G = (grafo*)S;
     free (G->descr);
     for (n=0; n<G->nnodos; n++)
	 free (G->nodos[n].vec.num);
     free (G->nodos);
     G->nodos = NULL;
     G->nnodos = 0;
     free (G);
     if (libobj) closeDB();
   }

void saveIndex (Index S, char *fname)

   { int n;
     grafo *G = (grafo*)S;
     FILE *f = fopen (fname,"w");
     fwrite (G->descr,strlen(G->descr)+1,1,f);
     fwrite (&G->nnodos,sizeof(int),1,f);
     for (n=0; n<G->nnodos; n++)
	 { fwrite(&G->nodos[n].obj, sizeof(int), 1, f);
	   fwrite (&G->nodos[n].vec.nnum,sizeof(int),1,f);
	   fwrite (&G->nodos[n].maxd,sizeof(Tdist),1,f);
	   fwrite (G->nodos[n].vec.num,sizeof(int),G->nodos[n].vec.nnum,f);
	 }
     fclose (f);
   }

Index loadIndex (char *fname)  
	/* OJO: no preparado para agregar cosas (mult de BLK) */

   { int i;
     char str[1024]; char *ptr = str;
     grafo *G = malloc (sizeof(grafo));
     FILE *f = fopen (fname,"r");
     while ((*ptr++ = getc(f)));
     G->descr = malloc (ptr-str);
     strcpy (G->descr,str);
     G->np = openDB (str);
     fread (&G->nnodos,sizeof(int),1,f);
     G->nodos = malloc (G->nnodos*sizeof(G->nodos[0]));
     
     for (i=0; i<G->nnodos; i++)
	 { fread(&G->nodos[i].obj, sizeof(int), 1, f); 
	   fread (&G->nodos[i].vec.nnum,sizeof(int),1,f);
	   fread (&G->nodos[i].maxd,sizeof(Tdist),1,f);
	   G->nodos[i].vec.num = malloc (G->nodos[i].vec.nnum*sizeof(int));
	   fread (G->nodos[i].vec.num,sizeof(int),G->nodos[i].vec.nnum,f);
	 }
     fclose (f);
     return (Index)G;
   }

static int _search (grafo G, int n, Obj obj, Tdist r, Tdist d0, 
		 Tdist mind, bool show)

   { int j;
     nodo *N = &G.nodos[n];
     Tdist *dd;
     int rep = 0;
     if (N->vec.nnum == 0) { }
     if (d0-r > N->maxd) return 0;
     dd = malloc (N->vec.nnum*sizeof(Tdist));
     if (d0 <= r)
	{ rep++; if (show) printobj (N->obj); }
     for (j=0;j<N->vec.nnum;j++)
	{ dd[j] = distance (obj,G.nodos[N->vec.num[j]].obj); 
	  if (dd[j]<mind) mind = dd[j];
	}
     for (j=0;j<N->vec.nnum;j++)
	{ if (dd[j] <= mind+2*r)
	     rep += _search (G,N->vec.num[j],obj,r,dd[j],mind, show);
	}
     free (dd);
     return rep;
   }

int search (Index S, Obj obj, Tdist r, bool show)
 
   { grafo *G = (grafo*)S;
     Tdist d0 = distance(obj,G->nodos[0].obj);

     return _search (*G,0,obj,r,d0,d0,show);
   }


Tdist searchNN (Index S, Obj obj, int k, bool show)

   { Tdist ret,lbound,dist;
     int j;
     grafo G = *((grafo*)S);
     nodo *N;
     Tdist *dd;
     heapElem *heap;
     int hsize = 0;
     heapElem hel;
     Tcelem res = createCelem(k);
     heap = malloc (G.nnodos*sizeof(heapElem)); hsize = 0;
     dd = malloc (G.nnodos*sizeof(Tdist));
     dist = distance(obj,G.nodos[0].obj); 
     lbound = dist-G.nodos[0].maxd;
     if (lbound < 0) lbound = 0;
     addHeap (heap,&hsize,0,dist,lbound,dist);
     while (hsize > 0)
	{ 
          hel = extrHeap (heap, &hsize);
	  if (outCelem(&res,hel.lbound)) break; /* pruning distance alcanzada */
          N = &G.nodos[hel.id];
	  addCelem (&res,N->obj,hel.dist);
          for (j=0;j<N->vec.nnum;j++)
	     { dd[j] = distance (obj,G.nodos[N->vec.num[j]].obj); 
	       if (dd[j] < hel.mind) hel.mind = dd[j];
	     }
          for (j=0;j<N->vec.nnum;j++)
	      { lbound = hel.lbound;
		if (lbound < (dd[j]-hel.mind)/2) lbound = (dd[j]-hel.mind)/2;
		if (lbound < dd[j]-G.nodos[N->vec.num[j]].maxd)
		   lbound = dd[j]-G.nodos[N->vec.num[j]].maxd;
		addHeap (heap,&hsize,N->vec.num[j],dd[j],lbound,hel.mind);
	      }
	}
     if (show) showCelem(&res);
     ret = radCelem(&res);
     free (dd); free (heap); freeCelem (&res);
     return ret;
   }


static double avgari (grafo G)

   { int i;
     double tot=0;
     for (i=0;i<G.nnodos;i++)
        tot += G.nodos[i].vec.nnum;
     return tot/i;
   }

static int maxari (grafo G)

   { int i;
     int max=0;
     for (i=0;i<G.nnodos;i++)
        if (G.nodos[i].vec.nnum > max) max = G.nodos[i].vec.nnum;
     return max;
   }

static int wc (grafo G, int n)

   { int i;
     int max = 0;
     for (i=0;i<G.nodos[n].vec.nnum;i++)
         { int a = wc (G,G.nodos[n].vec.num[i]);
	   if (a>max) max=a;
	 }
     return max+G.nodos[n].vec.nnum+1;
   }

static double ac (grafo G, int n, int *num)

   { int i,lnum;
     double tot = 0;
     *num=1;
     for (i=0;i<G.nodos[n].vec.nnum;i++)
         { tot += ac (G,G.nodos[n].vec.num[i],&lnum);
	   *num += lnum;
	 }
     if (i == 0) return 1;
     return tot+*num*(G.nodos[n].vec.nnum+1);
   }

static int altura (grafo G, int n)

   { int i;
     int max = 0;
     for (i=0;i<G.nodos[n].vec.nnum;i++)
         { int a = altura (G,G.nodos[n].vec.num[i]);
	   if (a>max) max=a;
	 }
     return max+1;
   }

static double avgprof (grafo G, int n, int height, int *num)

   { int i;
     double tot = 0;
     for (i=0;i<G.nodos[n].vec.nnum;i++)
         { tot += avgprof (G,G.nodos[n].vec.num[i],height+1,num);
	 }
     if (i==0) { (*num)++; return height; }
     return tot;
   }

void prnstats (Index S)

   { grafo *G = (grafo*)S;
     int dummy;
     printf ("number of nodes and elements: %i\n",G->nnodos);
     printf ("average arity: %0.2f\n",avgari(*G));
     printf ("maximum arity: %i\n",maxari(*G));
     printf ("worst case (r=0): %i\n",wc(*G,0));
     printf ("average case (r=0): %0.2f\n",ac(*G,0,&dummy)/G->nnodos);
     printf ("height: %i\n",altura(*G,0));
     printf ("average depth of leaves: %0.2f\n",avgprof(*G,0,1,&dummy)/dummy);
   }
