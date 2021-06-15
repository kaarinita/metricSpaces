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

#include "lcluster.h"

void prnstats (Index S);

	// strategy to select next center. left as random, others can be
	// activated by defining P2..P5 instead

#define P1
int compar (const void *a, const void *b)

   {
     Tod *x = (Tod *)a;
     Tod *y = (Tod *)b;

     if (x->dist > y->dist) return 1;
     else if (x->dist < y->dist) return -1;
     else return 0;
   }

static vpnode buildvpt (vpt *tree, Tod *od, int nobjs)

   { int i,per;
     Tdist max,min;
     vpnode node;

     node.hoja = (nobjs <= tree->bsize);

     if (node.hoja)
        { node.u.hoja.bucket = createbucket();
	  node.u.hoja.size = 0;
	  for (i=0;i<nobjs;i++) 
	     { node.u.hoja.bucket = addbucket (node.u.hoja.bucket,
					node.u.hoja.size, od[i].obj);
	       node.u.hoja.size++;
	     }
	}
     else
        { 
#ifdef P1
          int j = aleat (nobjs);
#endif
#ifdef P2
          int j = 0;
#endif
#ifdef P3
          int j = nobjs-1;
#endif
#ifdef P4
          int j = 0,jj;
          for (jj=1;jj<nobjs;jj++)
              if (od[jj].tdist < od[j].tdist) j = jj;
#endif
#ifdef P5
          int j = 0,jj;
          for (jj=1;jj<nobjs;jj++)
              if (od[jj].tdist > od[j].tdist) j = jj;
#endif
          node.u.interno.query = od[j].obj;
	  od[j] = od[--nobjs];
	  for (i=0;i<nobjs;i++) 
              { od[i].dist = distance (node.u.interno.query, od[i].obj);
                od[i].tdist += od[i].dist;
              }

          qsort (od,nobjs,sizeof(Tod),compar);
	  min = od[0].dist; max = od[nobjs-1].dist;
	  per = tree->bsize;
	  node.u.interno.dist = od[per].dist;
	  while ((per > 0) && (od[per+1].dist == node.u.interno.dist)) per--;
	  node.u.interno.child1 = malloc (sizeof(vpnode));
	  *node.u.interno.child1 = buildvpt (tree,od,per);
	  node.u.interno.child2 = malloc (sizeof(vpnode));
	  *node.u.interno.child2 = buildvpt (tree,od+per,nobjs-per);
	}
     return node;
   }

Index build (char *dbname, int n, int *argc, char ***argv)

   { vpt *tree;
     int i, j;
     Tod *od;
     int nobjs;
     if (*argc < 1)
        { fprintf (stderr,"Usage: <program> <args> BUCKET-SIZE\n");
          exit(1);
        }
     tree = malloc (sizeof(vpt));
     tree->descr = malloc (strlen(dbname)+1);
     strcpy (tree->descr,dbname);
     tree->bsize = atoi((*argv)[0]);
     (*argc) -= 1; (*argv) += 1;
 
     nobjs = openDB(dbname);
    if (n && (n < nobjs)) nobjs = n; 

     od = malloc (nobjs * sizeof (Tod));

     j=0; i=0;

     while (j < nobjs) { od[i].tdist = 0;  j++; od[i++].obj = j;  }
     tree->node = buildvpt (tree,od,nobjs);
     free (od);
     prnstats((Index)tree); 
     return (Index)tree;
   }


static void freevpt (vpnode *node)

   { if (node->hoja) freebucket (node->u.hoja.bucket,node->u.hoja.size);
     else 
	{ freevpt (node->u.interno.child1);
	  free (node->u.interno.child1);
	  freevpt (node->u.interno.child2);
	  free (node->u.interno.child2);
	}
   }

void freeIndex (Index S, bool libobj)

   { vpt *tree = (vpt*)S;
     free (tree->descr);
     freevpt (&tree->node);
     free (tree);
     if (libobj) closeDB();
   }

static void savenode (vpnode *node, int bsize, FILE *f)

   { char hoja = node->hoja ? '1' : '0';
     putc (hoja,f);
     if (node->hoja)
	{ fwrite (&node->u.hoja.size,sizeof(int),1,f);
          savebucket (node->u.hoja.bucket,node->u.hoja.size,f);
	}
     else
	{ fwrite(&node->u.interno.query,sizeof(int), 1, f);
	  fwrite (&node->u.interno.dist, sizeof(Tdist),1,f);
	  savenode(node->u.interno.child1,bsize,f);
	  savenode(node->u.interno.child2,bsize,f);
	}
   }

void saveIndex (Index S, char *fname)

   { FILE *f = fopen(fname,"w");
     vpt *tree = (vpt*)S;
     fwrite (tree->descr,strlen(tree->descr)+1,1,f);
     fwrite (&tree->bsize,sizeof(int),1,f);
     savenode (&tree->node,tree->bsize,f);
     fclose (f);
   }

static void loadnode (vpnode *node, int bsize, FILE *f)

   { char hoja;
     hoja = getc(f);
     node->hoja = (hoja == '1');
     if (node->hoja)
	{ fread (&node->u.hoja.size,sizeof(int),1,f);
	  node->u.hoja.bucket = loadbucket (node->u.hoja.size,f);
	}
     else
	{ fread(&node->u.interno.query, sizeof(int), 1, f);
	  fread (&node->u.interno.dist,sizeof(Tdist),1,f);
	  node->u.interno.child1 = malloc (sizeof(vpnode));
	  loadnode (node->u.interno.child1,bsize,f);
	  node->u.interno.child2 = malloc (sizeof(vpnode));
	  loadnode (node->u.interno.child2,bsize,f);
	}
   }

Index loadIndex (char *fname)

   { char str[1024]; char *ptr = str;
     FILE *f = fopen(fname,"r");
     vpt *tree = malloc (sizeof(vpt));

     while ((*ptr++ = getc(f)));
     tree->descr = malloc (ptr-str);
     strcpy (tree->descr,str);
     tree->np=openDB (str);
     fread (&tree->bsize,sizeof(int),1,f);
     loadnode (&tree->node,tree->bsize,f);
     fclose (f);
     return (Index)tree;
   }

static int _search (vpnode *node, Obj obj, Tdist r, bool show)

   { int rep = 0;
     if (node->hoja)
	{ rep += searchbucket (node->u.hoja.bucket,node->u.hoja.size,obj,r,show);
	}
     else
	{ Tdist dist;
	  dist = distance (obj, node->u.interno.query); 
	  if (dist <= r) { rep++; if (show) printobj(node->u.interno.query); }
	  if (dist-r < node->u.interno.dist)
	     rep += _search(node->u.interno.child1,obj,r,show);
	  if (dist+r >= node->u.interno.dist)
	     rep += _search(node->u.interno.child2,obj,r,show);
	}
     return rep;
   }

int search (Index S, Obj obj, Tdist r, int show)

   { vpt *tree = (vpt*)S;
     return _search (&tree->node,obj,r,show);
   }

static void _searchNN (vpnode *node, Obj obj, Tcelem *res)

   { 
     if (node->hoja)
	{ searchbucketNN (node->u.hoja.bucket,node->u.hoja.size,obj,res);
	}
     else
	{ Tdist dist;
	  dist = distance (obj, node->u.interno.query); 
          addCelem (res,node->u.interno.query,dist);
	  if (dist < node->u.interno.dist)
	     { _searchNN (node->u.interno.child1,obj,res);
	       if ((radCelem(res) == -1) ||
		   (dist + radCelem(res) >= node->u.interno.dist))
	          _searchNN (node->u.interno.child2,obj,res);
	     }
	  else
	     { _searchNN (node->u.interno.child2,obj,res);
	       if ((radCelem(res) == -1) ||
		   (dist - radCelem(res) < node->u.interno.dist))
	          _searchNN (node->u.interno.child1,obj,res);
	     }
	}
   }

Tdist searchNN (Index S, Obj obj, int k, bool show)

   { vpt *tree = (vpt*)S;
     Tdist mdif;
     Tcelem res = createCelem(k);
     _searchNN (&tree->node,obj,&res);
     if (show) showCelem (&res);
     mdif = radCelem(&res);
     freeCelem (&res);
     return mdif;
   }

static int height (vpnode *node)

   { int max1,max2;
     if (node->hoja) return 1;
     else
	{ max1 = height (node->u.interno.child1);
	  max2 = height (node->u.interno.child2);
	  return 1+(max1<max2?max2:max1);
	}
   }

static int numnodes (vpnode *node)

   { if (node->hoja) return 0;
     return 1+numnodes(node->u.interno.child1)+
	    numnodes(node->u.interno.child2);
   }

static int numbucks (vpnode *node)

   { if (node->hoja) return 1;
     return numbucks(node->u.interno.child1)+
	    numbucks(node->u.interno.child2);
   }

static int sizebucks (vpnode *node)

   { if (node->hoja) return node->u.hoja.size;
     return sizebucks(node->u.interno.child1)+
	    sizebucks(node->u.interno.child2);
   }

static int profbucks (vpnode *node, int depth)

   { if (node->hoja) return depth;
     return profbucks(node->u.interno.child1,depth+1)+
	    profbucks(node->u.interno.child2,depth+1);
   }

void prnstats (Index S)

   { vpt *tree = (vpt*)S;
     int nbucks = numbucks(&tree->node);
     int sbucks = sizebucks(&tree->node);
     int nnodes = numnodes(&tree->node);
     printf ("number of elements: %i\n",sbucks+nnodes);
     printf ("bucket size: %i\n",tree->bsize);
     printf ("maximum height: %i\n",height(&tree->node));
     printf ("number of nodes: %i\n",nnodes);
     printf ("number of buckets: %i\n",nbucks);
     printf ("average size of bucket: %0.2f\n",sbucks/(double)nbucks);
     printf ("average depth of bucket: %0.2f\n",
                        profbucks(&tree->node,0)/(double)nbucks);
   }

