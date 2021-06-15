/***************************************************************************
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
 ***************************************************************************/
/****************************************************************************
  This version of Dynamic Spatial Approximation Trees uses the combination of
  timestamping with bounded arity for incremental construction. To delete an
  element we replace it with his nearest neighbor in its own subtree, using
  "ghost hyperplanes", but we only admit a fraction of elements replaced within
  the tree. However it is important to note that the element is effectively
  deleted from the tree.
*****************************************************************************/

#include "dyn-sat.h"

void prnstats (Index S);

static int search_place (grafo *G, Obj obj, bool show,int posi);

static bool localize (grafo *G, Obj obj, bool show, int *n);


static void addHeap (heapElem *heap, int *hsize, int id,
		     Tdist dist, Tdist lbound,Tdist maxsuff,Tdist r)
  { int hs;
    heap--;
    hs = ++*hsize;
    while (hs > 1)
    { int hs2 = hs>>1;
      if (heap[hs2].lbound <= lbound) break; // ordena por lbound
      heap[hs] = heap[hs2];
      hs = hs2;
    }
    heap [hs].id = id; heap[hs].dist = dist;
    heap[hs].lbound = lbound; heap[hs].maxsuff = maxsuff;
    heap [hs].r = r;
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


static qelem *sort (qelem *r, int lo, int up)

  { int max = 0;
    int *qty;
    int i,j,pos;
    int n = up+1;
    qelem *nr;
    for (i=0;i<n;i++) if (r[i].time > max) max = r[i].time;
    qty = malloc ((max+1)*sizeof(int));
    for (j=0;j<=max;j++) qty[j] = 0;
    for (i=0;i<n;i++) qty[r[i].time]++;
    pos = 0;
    for (j=0;j<=max;j++) { int pj = pos; pos += qty[j]; qty[j] = pj; }
    nr = malloc (n*sizeof(qelem));
    for (i=0;i<n;i++) nr[qty[r[i].time]++] = r[i];
    free (r); free (qty);
    return nr;
  }


static int newnode(grafo *G, Obj obj)

  { int pos = -1;
    bool newn;
    if (G->libre == -1)
    { space (G->nodos,G->nnodos);
      pos = G->nnodos;
      newn = true;
    }
    else
    { pos = G->libre;
      G->libre = G->nodos[G->libre].time;
      newn= false;
    }
    G->nodos[pos].obj = obj;
    G->nodos[pos].time = G->lastime++;
    G->nodos[pos].nsust = 0;
    G->nodos[pos].size = 1;
    G->nodos[pos].maxd = 0;
    G->nodos[pos].pf = 0;
    G->nodos[pos].sust = false;
    G->nodos[pos].ptime = 0;
    G->nodos[pos].father = -1;
    G->nodos[pos].vec.num = NULL;
    G->nodos[pos].vec.nnum = 0;
    G->nobjs++;
    return ((!newn)? pos : G->nnodos++);
  }


static void desconectar (grafo *G, int node, int nodep)
  { int j,i;
    nodo *P = &G->nodos[nodep];
    for (j=0;j<P->vec.nnum;j++)
    { if (P->vec.num[j] == node) break;  }
    for(i=j+1;i<P->vec.nnum;i++)
    { P->vec.num[i-1] = P->vec.num[i];  }
    P->vec.nnum--;
    if (P->vec.nnum == 0)
    { P->maxd = 0;
      free ((void *)P->vec.num);
      P->vec.num = NULL;
    }
    G->nodos[node].father = -1;
  }


static void freeTree(grafo *G, int node, int nodep)

  { int np = nodep;
    int nsust = G->nodos[node].nsust;
    int size = G->nodos[node].size;
    desconectar(G,node,nodep);
    while (np != -1)
    { G->nodos[np].nsust -= nsust;
      G->nodos[np].size -= size;
      np = G->nodos[np].father;
    }
  }


static void resetnode (grafo *G, int node)
  { free((void*)G->nodos[node].vec.num)
    G->nodos[node].vec.num = NULL;
    G->nodos[node].vec.nnum = 0;
    G->nodos[node].nsust = 0;
    G->nodos[node].size = 1;
    G->nodos[node].maxd = 0;
    G->nodos[node].pf = 0;
    G->nodos[node].sust = false;
    G->nodos[node].ptime = 0;
    G->nodos[node].father = -1;
  }


static void freenode (grafo *G, int node)
  { G->nodos[node].obj = (Obj)-1;
    resetnode (G,node);
    G->nodos[node].time = G->libre;
    G->libre = node;
    G->nobjs--;
  }


static void newqueue(arrdata *queue, int time, int pos)
  { qelem *nq;
    space(queue->data, queue->ndata);
    nq = &queue->data[queue->ndata];
    nq->time = time;
    nq->pos = pos;
    queue->ndata++;
  }


static arrdata retrieve(grafo *G, int idp)
  { int i, pos, node, nodep, nodo, num, j, t;
    arrnum q;
    arrdata queue;
    int pq = 0;
    pos = 0;
    q.num = (int*)malloc(G->nnodos*sizeof(int));
    queue.data = NULL;
    q.nnum = 0;
    queue.ndata = 0;
    q.num[0] = idp;
    q.nnum ++;
    while (pos < q.nnum)
    { nodep = q.num[pos];
      pos++;
      i = 0;
      while (i<G->nodos[nodep].vec.nnum)
      { node = G->nodos[nodep].vec.num[i];
        freeTree (G,node,nodep);
        if (G->nodos[node].sust)
        { G->nodos[node].time = G->nodos[node].ptime; }
        newqueue (&queue,G->nodos[node].time,node);
        while (pq < queue.ndata )
        { num = G->nodos[queue.data[pq].pos].vec.nnum;
          for (j=0;j<num;j++)
          { nodo = G->nodos[queue.data[pq].pos].vec.num[j];
            if (!G->nodos[nodo].sust)
            { t = G->nodos[nodo].time; }
            else
            { t = G->nodos[nodo].time = G->nodos[nodo].ptime; }
              newqueue (&queue,t,nodo);
          }
          resetnode (G,queue.data[pq].pos);
          pq++;
        }
      }
    }
    free((void*)q.num);
    return(queue);
  }


static void newvec(grafo *G, int node, Obj obj, int posi)

  { int pos = (posi != -1)? posi : newnode (G,obj);
    nodo *N = &G->nodos[node];
    space (N->vec.num,N->vec.nnum);
    N->vec.num[N->vec.nnum] = pos;
    N->vec.nnum++;
    G->nodos[pos].father = node;
    while (node != -1)
    { G->nodos[node].size += 1;
      node = G->nodos[node].father;
    }
  }


static void reinsert (grafo *G, int node, int nodep, bool show)

  { int padre;
    Obj obj = G->nodos[node].obj;
    padre = search_place(G,obj,show,nodep);
    newvec (G,padre,obj,node);
  }


static void reinsertQueue (grafo *G, int nodep, arrdata *q, bool show)
   { int j, nodo;
    q->data = sort (q->data,0,q->ndata-1);
    for (j=0;j<q->ndata;j++)
    { nodo = q->data[j].pos;
      reinsert(G,nodo,nodep,show);
    }
    free((void*)q->data);
    q->data = NULL;
    q->ndata = 0;
  }

static int mas_cercano (grafo *G, int node, bool show)

  { Tdist min,lbound,suff,maxr,r;
    int j, i, ret;
    nodo *N;
    Tdist *dd, dpf;
    Obj obj;
    heapElem *heap;
    int hsize = 0;
    heapElem hel;
    Tcelem res = createCelem(1);
    N = &G->nodos[node];
    obj = N->obj;
    heap = malloc (N->size*sizeof(heapElem)); hsize = 0;
    dd = malloc (G->marity*sizeof(Tdist));
    min = dd[0] = distance (N->obj,G->nodos[N->vec.num[0]].obj);
    dpf = G->nodos[N->vec.num[0]].pf;
    for (j=1;j<N->vec.nnum;j++)
    { dd[j] = distance (N->obj,G->nodos[N->vec.num[j]].obj);  }
    for (j=0;j<N->vec.nnum;j++)
    {  r = 0;
       maxr = 0;
       suff = (dd[j]-G->nodos[N->vec.num[j]].pf)-(min+dpf);
       if (suff < 0) suff = 0;
       lbound = 0;
       if (lbound < ((dd[j]-G->nodos[N->vec.num[j]].pf)-(min+dpf))/2)
           lbound = ((dd[j]-G->nodos[N->vec.num[j]].pf)-(min+dpf))/2;
       if (lbound < suff/2) lbound = suff/2;
       if (lbound < (dd[j]-G->nodos[N->vec.num[j]].pf)-G->nodos[N->vec.num[j]].maxd)
           lbound = (dd[j]-G->nodos[N->vec.num[j]].pf)-G->nodos[N->vec.num[j]].maxd;
       for (i=0;i<j;i++)
       { r = ((dd[j]-G->nodos[N->vec.num[j]].pf)-(dd[i]+G->nodos[N->vec.num[i]].pf))/2;
         if (maxr < r) maxr = r;
       }
       addHeap (heap, &hsize,N->vec.num[j],dd[j],lbound,suff,maxr);
       if (min > dd[j])
       { min = dd[j];
         dpf = G->nodos[N->vec.num[j]].pf;
       }
    }
    while (hsize > 0)
    { hel = extrHeap (heap, &hsize);
      if (outCelem(&res,hel.lbound)) break;
      N = &G->nodos[hel.id];
      if(N->vec.nnum > 0)
      { min = dd[0] = distance(obj,G->nodos[N->vec.num[0]].obj);
        dpf = G->nodos[N->vec.num[0]].pf;
      }
      else
      { min = hel.dist;}
      if (hel.r <= min)
      { addCelem (&res,N->obj,hel.dist);
        if (N->vec.nnum>0)
        { for (j=1;j<N->vec.nnum;j++)
          { dd[j] = distance (obj,G->nodos[N->vec.num[j]].obj); }
          for (j=0;j<N->vec.nnum;j++)
          { r = 0;
            maxr = 0;
            suff = hel.maxsuff + (dd[j]+G->nodos[N->vec.num[j]].pf)-(min+dpf);
            if (suff<0) suff = 0;
            lbound = hel.lbound;
            if (lbound < ((dd[j]-G->nodos[N->vec.num[j]].pf)-(min+dpf))/2)
                lbound = ((dd[j]-G->nodos[N->vec.num[j]].pf)-(min+dpf))/2;
            if (lbound < suff/2) lbound = suff/2;
            if (lbound < (dd[j]-G->nodos[N->vec.num[j]].pf)-G->nodos[N->vec.num[j]].maxd)
                lbound = (dd[j]-G->nodos[N->vec.num[j]].pf)-G->nodos[N->vec.num[j]].maxd;
            for (i = 0; i < j; i++)
            { r = ((dd[j]-G->nodos[N->vec.num[j]].pf)-(dd[i]-G->nodos[N->vec.num[i]].pf))/2;
              if (maxr < r) maxr = r;
            }
            if (maxr > hel.r) maxr = hel.r;
            addHeap (heap,&hsize,N->vec.num[j],dd[j],lbound,suff,maxr);
            if (dd[j] < min)
            {  min = dd[j];
               dpf = G->nodos[N->vec.num[j]].pf;
            }
          }
        }
      }
    }
    if (show) showCelem(&res);
    ret = res.elems[0].id;
    free ((void*)dd); free ((void*)heap); freeCelem (&res);
    return ret;
  }


static void sustituir (grafo *G, int node, int *last, int *nsust, bool show)
  { int i, v,nodo_sust;
    nodo *N;
    Tdist dist;
    N = &G->nodos[node];
    nodo_sust = mas_cercano (G, node,show);
    dist = distance(N->obj, G->nodos[nodo_sust].obj);
    N->pf += dist;
    N->obj = G->nodos[nodo_sust].obj;
    if (G->nodos[nodo_sust].sust)
    { N->ptime = G->nodos[nodo_sust].ptime; }
    else
    { N->ptime = G->nodos[nodo_sust].time; }
    if (G->nodos[nodo_sust].vec.nnum > 0)
    { sustituir (G, nodo_sust, last, nsust,show);
      for (i=nodo_sust;i!=node;i=G->nodos[i].father)
      { G->nodos[i].size--;
        G->nodos[i].nsust+= (*nsust);
      }
    }
    else
    { (*last) = G->nodos[nodo_sust].father;
      v = (int) G->nodos[nodo_sust].sust;
      desconectar (G,nodo_sust, G->nodos[nodo_sust].father);
      freenode(G,nodo_sust);
      for (i=(*last);i!=node;i=G->nodos[i].father)
      { G->nodos[i].size--;
        G->nodos[i].nsust-= v;
      }
    }
    if (!N->sust)
    { (*nsust)++; }
    N->sust = true;
  }


void insert (Index S, Obj obj, bool show )

  { int nodep;
    grafo *G = (grafo*)S;
    nodep = search_place(G,obj,show,0);
    if (nodep > -1)
    { newvec (G,nodep,obj,-1); }
    else
    { newnode(G,obj);}
  }


static grafo* iniciar (char *dbname, int marity, int num, int den)
  { grafo *G = malloc (sizeof(grafo));
    G->descr = malloc (strlen(dbname)+1);
    strcpy (G->descr,dbname);
    G->marity = marity;
    G->nobjs = 0;
    G->nodos = NULL;
    G->nnodos = 0;
    G->lastime = 0;
    G->factor.num = num;
    G->factor.den = den;
    G->libre = -1;
    return G;
  }

extern int numDeleted;

void delete (Index S, Obj obj, bool show)

  { int node,nodep,ar,j,i,nsust;
    fraccion fr;
    arrdata queue;
    bool sust;
    grafo *G = (grafo*)S;
    nodo *P, *N;
    node = nodep = -1;
    if (localize (G,obj,show,&node))
    { if (show)
      { printf ("position: %i \n",node); }
      numDeleted++;
      nodep = G->nodos[node].father;
      P = &G->nodos[nodep];
      N = &G->nodos[node];
      if (N->vec.nnum == 0)
      { if (nodep != -1)
        { sust = N->sust;
          desconectar(G,node,nodep);
          freenode(G,node);
          node = nodep;
          nodep = P->father;
          while (node != -1)
          { P->size--;
            P->nsust-= (int)sust;
            if ((P->nsust*G->factor.den)<=(P->size*G->factor.num))
            { if (nodep != -1)
              { node = nodep;
                P = &G->nodos[node];
                nodep = P->father;
              }
              else node = -1;
            }
            else
            { j = G->nodos[node].father;
              while (j!= -1)
              { G->nodos[j].size--;
                G->nodos[j].nsust -= (int)sust;
                j = G->nodos[j].father;
              }
              break;
            }
          }
        }
        else
        { char *dbname;
          dbname = malloc (strlen(G->descr)+1);
          strcpy (dbname,G->descr);
          fr.num = G->factor.num;
          fr.den = G->factor.den;
          ar = G->marity;
          freeIndex(S,false);
          G = iniciar (dbname, ar,fr.num,fr.den);
          free ((void*)dbname);
        }
      }
      else
      { nsust = 0;
        sustituir (G,node,&i,&nsust,show);
        node = i;
        P = &G->nodos[node];
        nodep = P->father;
        while (node != -1)
        { if (P->time <= N->time)
          { P->nsust+= nsust;
            P->size--;
          }
          if ((P->nsust*G->factor.den)<=(P->size*G->factor.num))
          { if (nodep != -1)
            { node = nodep;
              P = &G->nodos[node];
              nodep = P->father;
            }
            else node = -1;
          }
          else
          { j = P->father;
            while (j!= -1)
            { if (G->nodos[j].time <= N->time)
              { G->nodos[j].nsust+= nsust;
                G->nodos[j].size--;
              }
              j = G->nodos[j].father;
            }
            break;
          }
        }
      }
      while (node!=-1)
      { if (node > 0)
        { queue = retrieve (G,nodep);
          reinsertQueue (G,nodep,&queue,show);
        }
        else
        { if (node != -1)
            printf ("it is not possible to delete the tree root \n");
        }
        node = nodep;
        P = &G->nodos[node];
        nodep = P->father;
        while (node != -1)
        { if ((P->nsust*G->factor.den)<=(P->size*G->factor.num))
          { if (nodep != -1)
            { node = nodep;
              P = &G->nodos[node];
              nodep = P->father;
            }
            else node = -1;
          }
          else break;
        }
      }
    }
    else
    { if (show)
      { printf ("the object: ");
        printobj (obj);
        printf ("is not founded\n");
      }
    }
  }


Index init_Index  (char *dbname, int *argc, char ***argv)

  { grafo *G = malloc (sizeof(grafo));
    G->descr = malloc (strlen(dbname)+1);
    strcpy (G->descr,dbname);
    G->marity = atoi((*argv)[0]);
    G->nobjs = 0;
    G->nodos = NULL;
    G->nnodos = 0;
    G->lastime = 0;
    G->factor.num = atoi((*argv)[1]);
    G->factor.den = atoi((*argv)[2]);
    G->libre = -1;
    (*argc) -= 3; (*argv) += 3;
    return (Index)G;
  }

extern long long numDistances;

Index build ( char *dbname, int n, int *argc, char ***argv)
  { int np, i;
    Obj obj;
	  if(*argc < 3)
	  { fprintf (stderr,"Usage: <program> <args> ARITY NUM DEN\n");
		exit(1);
	  }
    grafo *G = malloc (sizeof(grafo));
    G->descr = malloc (strlen(dbname)+1);
    strcpy (G->descr,dbname);
    G->marity = atoi((*argv)[0]);
    G->nobjs = 0;
    G->nodos = NULL;
    G->nnodos = 0;
    G->lastime = 0;
    G->factor.num = atoi((*argv)[1]);
    G->factor.den = atoi((*argv)[2]);
    G->libre = -1;
    np =  openDB(dbname);
    if (n && (n < np)) np = n;
    i = 0;
    int kPerc = 10;
    int perc = kPerc;
    int inc_amount = (np / kPerc) + (np % kPerc == 0 ? 0 : 1);
    int cur_amout = inc_amount;
    while (i < np)
    { obj = ++i;
      insert ((Index)G,obj,0);
      if (i >= cur_amout) {
        printf("perc= %d\tdc= %lld\n", perc, numDistances);
        perc += kPerc;
        cur_amout += inc_amount;
      }
    }
    if (perc <= 100) {
      printf("perc= 100\tdc= %lld\n", numDistances);
    }
    (*argc) -= 3; (*argv) += 3;
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
    G->nobjs = 0;
    G->libre = -1;
    G->lastime = 0;
    G->factor.num = 0;
    G->factor.den = 1;
    free (G);
    if (libobj) closeDB();
  }


void saveIndex (Index S, char *fname)

  { int n;
    grafo *G = (grafo*)S;
    FILE *f = fopen (fname,"w");
    fwrite (G->descr,strlen(G->descr)+1,1,f);
    fwrite (&G->nnodos,sizeof(int),1,f);
    fwrite (&G->nobjs,sizeof(int),1,f);
    fwrite (&G->lastime,sizeof(int),1,f);
    fwrite (&G->factor.num,sizeof(int),1,f);
    fwrite (&G->factor.den,sizeof(int),1,f);
    fwrite (&G->libre,sizeof(int),1,f);
      fwrite (&G-> marity,sizeof(int),1,f);

    for (n=0; n<G->nnodos; n++)
    { fwrite (&G->nodos[n].obj,sizeof(int),1,f);
      fwrite (&G->nodos[n].time,sizeof(int),1,f);
      fwrite (&G->nodos[n].nsust,sizeof(int),1,f);
      fwrite (&G->nodos[n].size,sizeof(int),1,f);
      fwrite (&G->nodos[n].maxd,sizeof(Tdist),1,f);
      fwrite (&G->nodos[n].pf,sizeof(Tdist),1,f);
      fwrite (&G->nodos[n].sust,sizeof(bool),1,f);
      fwrite (&G->nodos[n].ptime,sizeof(int),1,f);
      fwrite (&G->nodos[n].vec.nnum,sizeof(int),1,f);
      fwrite (G->nodos[n].vec.num,sizeof(int),G->nodos[n].vec.nnum,f);
    }
    fclose (f);
  }


Index loadIndex (char *fname)
/* OJO: no preparado para agregar cosas (mult de BLK) */

  { int i, np;
    char str[1024]; char *ptr = str;
    grafo *G = malloc (sizeof(grafo));
    FILE *f = fopen (fname,"r");
    while ((*ptr++ = getc(f)));
    G->descr = malloc (ptr-str);
    strcpy (G->descr,str);
    np = openDB (str);
    fread (&G->nnodos,sizeof(int),1,f);
    fread (&G->nobjs,sizeof(int),1,f);
    fread (&G->lastime,sizeof(int),1,f);
    fread (&G->factor.num,sizeof(int),1,f);
    fread (&G->factor.den,sizeof(int),1,f);
    fread (&G->libre,sizeof(int),1,f);
      fread (&G-> marity,sizeof(int),1,f);

    G->nodos = malloc (G->nnodos*sizeof(G->nodos[0]));
    for (i=0; i<G->nnodos; i++)
    { fread (&G->nodos[i].obj, sizeof(int), 1, f);
      fread (&G->nodos[i].time,sizeof(int),1,f);
      fread (&G->nodos[i].nsust,sizeof(int),1,f);
      fread (&G->nodos[i].size,sizeof(int),1,f);
      fread (&G->nodos[i].maxd,sizeof(Tdist),1,f);
      fread (&G->nodos[i].pf,sizeof(Tdist),1,f);
      fread (&G->nodos[i].sust,sizeof(bool),1,f);
      fread (&G->nodos[i].ptime,sizeof(int),1,f);
      fread (&G->nodos[i].vec.nnum,sizeof(int),1,f);
      G->nodos[i].vec.num = malloc (G->nodos[i].vec.nnum*sizeof(int));
      fread (G->nodos[i].vec.num,sizeof(int),G->nodos[i].vec.nnum,f);
    }
    fclose (f);
    return (Index)G;
  }

static int _search (grafo *G, int n, Obj obj, Tdist r, Tdist d0, bool show, int tstamp)
/* modificado para que tenga en cuenta los planos fantasmas */
  { int j, k, tm;
    Tdist min;
    nodo *N = &G->nodos[n];
    Tdist *dd;
    bool cond;
    Tdist dpf, dpf1;
    int rep = 0;
    if (N->sust)
    { cond = ((d0-r)-N->pf > N->maxd); }
    else
    { cond = (d0-r > N->maxd); }
    if (cond) return 0;
    if (d0 <= r)
    { rep++; if (show) printobj (N->obj); }
    dd = malloc (N->vec.nnum*sizeof(Tdist));
    if (N->vec.nnum > 0)
    { dpf = G->nodos[N->vec.num[0]].pf;
      dd[0] = min = distance (obj,G->nodos[N->vec.num[0]].obj);
      if (G->nodos[N->vec.num[0]].sust)
      { min+= dpf; }
      for (j=1;j<N->vec.nnum;j++)
      { dd[j] = distance (obj,G->nodos[N->vec.num[j]].obj);  }
    }
    for (j=0;j<N->vec.nnum;j++)
    { tm = G->nodos[N->vec.num[j]].time;
      if (G->nodos[N->vec.num[j]].sust)
      { dpf = G->nodos[N->vec.num[j]].pf;
        cond = (dd[j]-dpf <= min+2*r);
      }
      else
      { cond = (dd[j]<= min+2*r);}
      if ((cond) && (tm < tstamp))
      { tm = tstamp;
        for (k = j+1; k < N->vec.nnum; k++)
        { dpf1 = G->nodos[N->vec.num[k]].pf;
          if ((G->nodos[N->vec.num[k]].sust)&&(G->nodos[N->vec.num[j]].sust))
          { cond = (dd[j]-dpf > dd[k]+(2*r)+dpf1); }
          else
          { if ((G->nodos[N->vec.num[k]].sust)&&!(G->nodos[N->vec.num[j]].sust))
            { cond = (dd[j] > dd[k]+(2*r)+dpf1); }
            else
            { if (!(G->nodos[N->vec.num[k]].sust)&&(G->nodos[N->vec.num[j]].sust))
              { cond = (dd[j]-dpf > dd[k]+(2*r)); }
              else
              { cond = (dd[j]> dd[k]+(2*r)); }
            }
          }
          if (cond)
          { tm = G->nodos[N->vec.num[k]].time;
            break;
          }
        }
        rep += _search (G,N->vec.num[j],obj,r,dd[j],show,tm);
      }
      if (G->nodos[N->vec.num[j]].sust)
      { if (dd[j]+dpf < min) { min = dd[j]+dpf;} }
      else
      { if (dd[j]< min) { min = dd[j];} }
    }
    free (dd);
    return rep;
  }


int search (Index S, Obj obj, Tdist r, bool show )

  { grafo *G = (grafo*)S;
    Tdist d0;
    if(G->nnodos > 0)
    { d0 = distance(obj,G->nodos[0].obj);
      return _search (G,0,obj,r,d0,show,G->lastime);
    }
    else return(-1);
  }


Tdist searchNN (Index S, Obj obj, int k, bool show)
/* modificado para que tenga en cuenta los planos fantasmas */
  { Tdist ret,min,lbound,dist,suff,maxr,r;
    int j, i;

    grafo *G = (grafo*)S;
    nodo *N = G->nodos;
    Tdist *dd, dpf;
    heapElem *heap;
    int hsize = 0;
    heapElem hel;
    Tcelem res = createCelem(k);

    heap = malloc (N->size*sizeof(heapElem)); hsize = 0;

	dd = malloc (G->marity*sizeof(Tdist));
    dist = distance (obj,G->nodos[0].obj);
    dpf = G->nodos[0].pf;
    lbound = dist - (G->nodos[0].maxd + dpf);
    if (lbound < 0) lbound = 0;
    addHeap (heap,&hsize,0,dist,lbound,0,0);

      while (hsize > 0)
    { hel = extrHeap (heap,&hsize);

        if (outCelem(&res,hel.lbound)) break; /* prunning distance alcanzada */
      N = &G->nodos[hel.id];

        if(N->vec.nnum > 0)
      {

      min = dd[0] = distance(obj,G->nodos[N->vec.num[0]].obj);

          dpf = G->nodos[N->vec.num[0]].pf;

      }
      else
      { min = hel.dist;
      }
      if (hel.r <= min)
      { addCelem (&res,N->obj,hel.dist);
        if (N->vec.nnum>0)
        {
            for (j=1;j<N->vec.nnum;j++)
          { dd[j] = distance (obj,G->nodos[N->vec.num[j]].obj); }

            for (j=0;j<N->vec.nnum;j++)
          { r = 0;
            maxr = 0;
            suff = hel.maxsuff + (dd[j]+G->nodos[N->vec.num[j]].pf)-(min+dpf);
            if (suff<0) suff = 0;
            lbound = hel.lbound;
            if (lbound < ((dd[j]-G->nodos[N->vec.num[j]].pf)-(min+dpf))/2)
                lbound = ((dd[j]-G->nodos[N->vec.num[j]].pf)-(min+dpf))/2;
            if (lbound < suff/2) lbound = suff/2;
            if (lbound < (dd[j]-G->nodos[N->vec.num[j]].pf)-G->nodos[N->vec.num[j]].maxd)
                lbound = (dd[j]-G->nodos[N->vec.num[j]].pf)-G->nodos[N->vec.num[j]].maxd;
            for (i = 0; i < j; i++)
            { r = ((dd[j]-G->nodos[N->vec.num[j]].pf)-(dd[i]-G->nodos[N->vec.num[i]].pf))/2;
              if (maxr < r) maxr = r;
            }
            if (maxr > hel.r) maxr = hel.r;
            addHeap (heap,&hsize,N->vec.num[j],dd[j],lbound,suff,maxr);
            if (dd[j] < min)
            {  min = dd[j];
               dpf = G->nodos[N->vec.num[j]].pf;
            }
          }
        }
      }
    }

    if (show) showCelem(&res);
    ret = radCelem(&res);
    free ((void*)dd); free ((void*)heap); freeCelem (&res);
    return ret;
  }


static int search_place (grafo *G, Obj obj, bool show, int posi)

  { int j,pos = posi, posmin;
    Tdist adist, min, minv;
    bool find = false;
    nodo *N;
    if  (G->nobjs > 0)
    { N = &G->nodos[pos];
      min = distance(obj, N->obj);
      while (!find)
      { if (N->maxd+N->pf < min) { N->maxd = (min-N->pf);}
        if (N->vec.nnum > 0)
        { minv = distance (obj,G->nodos[N->vec.num[0]].obj);
          posmin = N->vec.num[0];
          for (j=1;j<N->vec.nnum;j++)
          { adist = distance(obj, G->nodos[N->vec.num[j]].obj);
            if (adist < minv)
            { minv = adist;
              posmin = N->vec.num[j];
            }
          }
          if ((min >= minv)|| (N->vec.nnum == G->marity))
          { pos = posmin;
            N = &G->nodos[pos];
            min = minv;
          }
          else
          { find = true; }
        }
        else
        { find = true;}
      }
    }
    else { pos = -1;}
    if (show) {printobj(G->nodos[pos].obj);}
    return pos;
  }


static void _localize (grafo *G, int n, Obj obj, Tdist d0, bool show, int tstamp, int *node, bool *find)

  { int j, k, tm;
    Tdist min;
    nodo *N = &G->nodos[n];
    Tdist *dd=NULL, dpf, dpf1;
    if (!(*find))
    { min = d0-N->pf;
      if (min > N->maxd) return;
      if (d0 == 0)
      { (*node) = n;
        if (show) printobj (N->obj);
        (*find) = true;
        return;
      }
      dd = malloc (N->vec.nnum*sizeof(Tdist));
      if (N->vec.nnum > 0)
      {  dpf = G->nodos[N->vec.num[0]].pf;
         dd[0] = distance (obj,G->nodos[N->vec.num[0]].obj);
         min = dd[0] + dpf;
         for (j=1;j<N->vec.nnum;j++)
         { dd[j] = distance (obj,G->nodos[N->vec.num[j]].obj); }
      }
      for (j=0;j<N->vec.nnum;j++)
      { tm = G->nodos[N->vec.num[j]].time;
        dpf = G->nodos[N->vec.num[j]].pf;
        if ((dd[j]-dpf<= min)&&(tm <= tstamp))
        { tm = tstamp;
          for (k = j+1; k < N->vec.nnum; k++)
          { dpf1 = G->nodos[N->vec.num[k]].pf;
            if (dd[j]-dpf> dd[k]+dpf1)
            { tm = G->nodos[N->vec.num[k]].time;
              break;
            }
          }
          _localize (G,N->vec.num[j],obj,dd[j],show,tm,node,find);
          if ((*find)) return;
        }
        /* minimiza a medida que atraviesa los vecinos */
        if (dd != NULL && (dd[j]+ dpf < min)) { min = dd[j] + dpf;}
      }
    }
    free((void*)dd);
    return;
  }


static bool localize (grafo *G, Obj obj,bool show, int *node)

  { Tdist d0;
    bool find = false;
    if(G->nnodos > 0)
    { d0 = distance(obj,G->nodos[0].obj);
      _localize (G,0,obj,d0,show,G->lastime,node,&find);
    }
    return find;
  }


static double avgari (grafo *G)

  { ulong i;
    double tot=0;
    for (i=0;i<G->nnodos;i++)
    { tot += G->nodos[i].vec.nnum;
    }
    return tot/i;
  }


static int maxari (grafo *G)

  { int i;
    int max=0;
    for (i=0;i<G->nnodos;i++)
        if (G->nodos[i].vec.nnum > max) max = G->nodos[i].vec.nnum;
    return max;
  }


static int wc (grafo *G, int n)

  { int i;
    int max = 0;
    for (i=0;i<G->nodos[n].vec.nnum;i++)
    { int a = wc (G,G->nodos[n].vec.num[i]);
      if (a>max) max=a;
    }
    return max+G->nodos[n].vec.nnum+1;
  }


static double ac (grafo *G, int n, int *num)

  { int i,lnum;
    double tot = 0;
    *num=1;
    for (i=0;i<G->nodos[n].vec.nnum;i++)
    { tot += ac (G,G->nodos[n].vec.num[i],&lnum);
      *num += lnum;
    }
    if (i == 0) return 1;
    return tot+*num*(G->nodos[n].vec.nnum+1);
  }


static int altura (grafo *G, int n)

  { int i;
    int max = 0;
    for (i=0;i<G->nodos[n].vec.nnum;i++)
    { int a = altura (G,G->nodos[n].vec.num[i]);
      if (a>max) max=a;
    }
    return max+1;
  }


static double avgprof (grafo *G, int n, int height, int *num)

  { int i;
    double tot = 0;
    for (i=0;i<G->nodos[n].vec.nnum;i++)
    { tot += avgprof (G,G->nodos[n].vec.num[i],height+1,num);  }
    if (i==0) { (*num)++; return height; }
    return tot;
  }


void prnstats (Index S)

  { grafo *G = (grafo*)S;
    int dummy;
    printf ("number of nodes: %i\t and elements: %i\n",G->nnodos,G->nobjs);
    printf ("average arity: %0.2f\n",avgari(G));
      printf ("marity: %i\n",G->marity);

      printf ("maximum arity: %i\n",maxari(G));
    printf ("worst case (r=0): %i\n",wc(G,0));
    printf ("average case (r=0): %0.2f\n",ac(G,0,&dummy)/G->nobjs);
    printf ("height: %i\n",altura(G,0));
    printf ("avg depth of leave: %0.2f\n",avgprof(G,0,1,&dummy)/dummy);
  }



