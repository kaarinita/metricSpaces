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

#include "mvp.h"

void prnstats (Index S);

int compar (const void *a, const void *b)

   {
     Tod *x = (Tod *)a;
     Tod *y = (Tod *)b;

     if (x->dist > y->dist) return 1;
     else if (x->dist < y->dist) return -1;
     else return 0;
   }

static vpnode buildvpt (vpt *tree, Tod *od, int nobjs)

   { int i,per,ptr,pptr;
     Tdist prev;
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
        { int j = aleat (nobjs);
          node.u.interno.children = malloc (tree->arity * sizeof(Tchild));
          node.u.interno.query = od[j].obj;
          od[j] = od[--nobjs];
	  for (i=0;i<nobjs;i++) 
	      od[i].dist = distance (node.u.interno.query, od[i].obj);
          qsort (od,nobjs,sizeof(Tod),compar);
	  prev = 0; per = 0;
	  for (i=0;i<tree->arity;i++)
	      { child(&node,i).dist = prev;
		per += nobjs / tree->arity;
		if (i >= tree->arity - (nobjs % tree->arity)) per++;
		if (per < nobjs) prev = od[per].dist;
	      }
          per = 0; pptr = 0;
	  for (i=0;i<tree->arity;i++)
	      { if (i < tree->arity-1)
		   { per += nobjs / tree->arity;
                     if (i >= tree->arity - (nobjs % tree->arity)) per++;
		     ptr = per;
		     prev = od[per].dist;
		     while ((ptr>=pptr) && (od[ptr].dist == prev)) ptr--;
		   }
		else ptr = nobjs-1;
		child(&node,i).child = buildvpt (tree,od+pptr,ptr-pptr+1);
		pptr = ptr+1;
	      }
	}
     return node;
   }

Index build (char *dbname, int n, int *argc, char ***argv)


   { vpt *tree;
     int i, j;
     Tod *od;
     int nobjs;
     if (*argc < 2)
        { fprintf (stderr,"Usage: <program> <args> BUCKET-SIZE ARITY\n");
          exit(1);
        }
     tree = malloc (sizeof(vpt));
     tree->descr = malloc (strlen(dbname)+1);
     strcpy (tree->descr,dbname);
     tree->bsize = atoi((*argv)[0]);
     tree->arity = atoi((*argv)[1]);
     (*argc) -= 2; (*argv) += 2;

     nobjs = openDB(dbname);
     if (n && (n < nobjs)) nobjs = n;      
     od = malloc (nobjs * sizeof (Tod));
     j=0;
     i=0;
     while (j< nobjs) od[i++].obj = ++j;
     tree->node = buildvpt (tree,od,nobjs);
     free (od);
     prnstats((Index)tree); 
     return (Index)tree;
   }


static void freevpt (vpnode *node, int arity)

   { if (node->hoja) freebucket (node->u.hoja.bucket,node->u.hoja.size);
     else 
	{ int i;
	  for (i=0;i<arity;i++)
	      freevpt (&child(node,i).child,arity);
          free (node->u.interno.children);
	}
   }

void freeIndex (Index S, bool libobj)

   { vpt *tree = (vpt*)S;
     free (tree->descr);
     freevpt (&tree->node,tree->arity);
     free (tree);
     if (libobj) closeDB();
   }

static void savenode (vpnode *node, int bsize, int arity, FILE *f)

   { char hoja = node->hoja ? '1' : '0';
     int i;
     putc (hoja,f);
     if (node->hoja)
	{ fwrite (&node->u.hoja.size,sizeof(int),1,f);
          savebucket (node->u.hoja.bucket,node->u.hoja.size,f);
	}
     else
	{ 
	  fwrite(&node->u.interno.query, sizeof(int), 1, f);
	  for (i=0;i<arity;i++)
	      { fwrite (&child(node,i).dist, sizeof(Tdist),1,f);
		savenode(&child(node,i).child,bsize, arity,f);
	      }
	}
   }

void saveIndex (Index S, char *fname)

   { FILE *f = fopen(fname,"w");
     vpt *tree = (vpt*)S;
 
     fwrite (tree->descr,strlen(tree->descr)+1,1,f);
     fwrite (&tree->bsize,sizeof(int),1,f);
     fwrite (&tree->arity,sizeof(int),1,f);
     savenode (&tree->node,tree->bsize,tree->arity,f);
     fclose (f);
   }

static void loadnode (vpnode *node, int bsize, int arity, FILE *f)

   { char hoja;
     int i;
     hoja = getc(f);
     node->hoja = (hoja == '1');
     if (node->hoja)
	{ fread (&node->u.hoja.size,sizeof(int),1,f);
	  node->u.hoja.bucket = loadbucket (node->u.hoja.size,f);
	}
     else
	{ 
	 fread(&node->u.interno.query,sizeof(int),1,f);
	  node->u.interno.children = malloc(arity*sizeof(Tchild));
	  for (i=0;i<arity;i++)
	      { fread (&child(node,i).dist,sizeof(Tdist),1,f);
		loadnode (&child(node,i).child,bsize,arity,f);
	      }
	}
   }

Index loadIndex (char *fname)

   { char str[1024]; char *ptr = str;
     FILE *f = fopen(fname,"r");
     vpt *tree = malloc (sizeof(vpt));

     while ((*ptr++ = getc(f)));
     tree->descr = malloc (ptr-str);
     strcpy (tree->descr,str);
     openDB (str);
     fread (&tree->bsize,sizeof(int),1,f);
     fread (&tree->arity,sizeof(int),1,f);
     loadnode (&tree->node,tree->bsize,tree->arity,f);
     fclose (f);
     return (Index)tree;
   }

static int _search (vpnode *node, Obj obj, Tdist r, bool show, int arity)

   { int rep = 0;
     if (node->hoja)
	{ rep += searchbucket (node->u.hoja.bucket,node->u.hoja.size,obj,r,show);
	}
     else
	{ int i;
          Tdist dist;
	  dist = distance (obj, node->u.interno.query); 
	  if (dist <= r) { rep++; if (show) printobj(node->u.interno.query); }
	  for (i=0;i<arity;i++)
	      if (((i==arity-1) || (child(node,i+1).dist > dist-r)) &&
	          (child(node,i).dist <= dist+r))
		 rep += _search(&child(node,i).child,obj,r,show,arity);
	}
     return rep;
   }

int search (Index S, Obj obj, Tdist r, int show)

   { vpt *tree = (vpt*)S;
     return _search (&tree->node,obj,r,show,tree->arity);
   }

static void _searchNN (vpnode *node, Obj obj, Tcelem *res, int arity)

   { 
     if (node->hoja)
	{ searchbucketNN (node->u.hoja.bucket,node->u.hoja.size,obj,res);
	}
     else
	{ int i,ci,d;
          Tdist dist;
	  bool ea,eb;
	  dist = distance (obj, node->u.interno.query); 
          addCelem (res,node->u.interno.query,dist);
          for (ci=0;ci<arity;ci++)
             if (child(node,ci).dist > dist) break;
          ci--; ea = eb = false;
          for (d=0;d<=arity;d++)
              { i = ci-d;
                if (i < 0) ea = true;
                if (!ea)
                   { if ((i==arity-1) || (radCelem(res) == -1) ||
			 (dist-child(node,i+1).dist <= radCelem(res)))
                       _searchNN (&child(node,i).child,obj,res,arity);
                     else ea = true;
                   }
                if (d==0) continue;
		i = ci+d;
                if (i >= arity) eb = true;
                if (!eb)
                   { if ((radCelem(res) == -1) ||
			 (child(node,i).dist-dist <= radCelem(res)))
                        _searchNN (&child(node,i).child,obj,res,arity);
                     else eb = true;
                   }
              }
	}
   }

Tdist searchNN (Index S, Obj obj, int k, bool show)

   { vpt *tree = (vpt*)S;
     Tdist mdif;
     Tcelem res = createCelem(k);
     _searchNN (&tree->node,obj,&res,tree->arity);
     if (show) showCelem (&res);
     mdif = radCelem(&res);
     freeCelem (&res);
     return mdif;
   }

static int height (vpnode *node, int arity)

   { int i;
     int max;
     if (node->hoja) return 1;
     else
	{ max = 0;
	  for (i=0;i<arity;i++)
	      { int hch = height (&child(node,i).child,arity);
	        if (hch > max) max = hch;
	      }
	  return 1+max;
	}
   }

static int numnodes (vpnode *node, int arity)

   { int i,ret=1;
     if (node->hoja) return 0;
     for (i=0;i<arity;i++)
         ret += numnodes(&child(node,i).child,arity);
     return ret;
   }

static int numbucks (vpnode *node, int arity)

   { int i,ret=0;
     if (node->hoja) return 1;
     for (i=0;i<arity;i++)
         ret += numbucks(&child(node,i).child,arity);
     return ret;
   }

static int sizebucks (vpnode *node, int arity)

   { int i,ret=0;
     if (node->hoja) return node->u.hoja.size;
     for (i=0;i<arity;i++)
         ret += sizebucks(&child(node,i).child,arity);
     return ret;
   }

static int profbucks (vpnode *node, int depth, int arity)

   { int i,ret=0;
     if (node->hoja) return depth;
     for (i=0;i<arity;i++)
         ret +=profbucks(&child(node,i).child,depth+1,arity);
     return ret;
   }

void prnstats (Index S)

   { vpt *tree = (vpt*)S;
     int nbucks = numbucks(&tree->node,tree->arity);
     int sbucks = sizebucks(&tree->node,tree->arity);
     int nnodes = numnodes(&tree->node,tree->arity);
     printf ("number of elements: %i\n",sbucks+nnodes);
     printf ("bucket size: %i\n",tree->bsize);
     printf ("arity: %i\n",tree->arity);
     printf ("maximum height: %i\n",height(&tree->node,tree->arity));
     printf ("number of nodes: %i\n",nnodes);
     printf ("number of buckets: %i\n",nbucks);
     printf ("average size of bucket: %0.2f\n",sbucks/(double)nbucks);
     printf ("average depth of bucket: %0.2f\n",
                        profbucks(&tree->node,0,tree->arity)/(double)nbucks);
   }

