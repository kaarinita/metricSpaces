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


#include "fqh.h"

void prnstats (Index S);

int compar (const void *a, const void *b)

   { 
     Tod *x, *y;
     x = (Tod *)a;
     y = (Tod *)b;


     if (x->dist > y->dist) return 1;
     else if (x->dist < y->dist) return -1;
     else return 0;
   }

static fqvpnode buildfqvpt (fqvpt *tree, Tod *od, int nobjs, int depth)

   { int i,per,pptr;
     Tdist prev,max,min,step;
     fqvpnode node;

     if (depth == tree->height)
        { node.u.hoja.bucket = createbucket();
	  node.u.hoja.size = 0;
	  for (i=0;i<nobjs;i++) 
	     { node.u.hoja.bucket = addbucket (node.u.hoja.bucket,
					node.u.hoja.size, od[i].obj);
	       node.u.hoja.size++;
	     }
	}
     else
        { if (nobjs == 0)
	     { node.u.interno.children = NULL;
	       return node;
	     }
          node.u.interno.children = malloc (tree->arity * sizeof(Tchild));
	  for (i=0;i<nobjs;i++) 
	      od[i].dist = distance (tree->queries[depth].query, od[i].obj);
          qsort (od,nobjs,sizeof(Tod),compar);
          min = od[0].dist; max = od[nobjs-1].dist;
          step = (max-min)/tree->arity;
          prev = 0; per = 0; pptr = 0;
          for (i=0;i<tree->arity;i++)
              { child(&node,i).dist = prev;
                prev += step;
                if (i < tree->arity-1) { while (od[per].dist < prev) per++; }
                else per = nobjs;
                child(&node,i).child = buildfqvpt (tree,od+pptr,per-pptr,depth+1);
                pptr = per;
              }
	}
     return node;
   }


Index build (char *dbname, int n, int *argc, char ***argv)


   { fqvpt *tree;
     int i,j;
     Tod *od;
     int nobjs;
     if (*argc < 2)
        { fprintf (stderr,"Usage: <program> <args> FIXED-HEIGHT ARITY\n");
          exit(1);
        }
     tree = malloc (sizeof(fqvpt));
     tree->descr = malloc (strlen(dbname)+1);
     strcpy (tree->descr,dbname);
     tree->n = openDB(dbname);

     if (n && (n < tree->n)) tree->n = n;   

     tree->height = atoi((*argv)[0]);
     if (tree->height > tree->n) tree->height = tree->n;
     tree->arity = atoi((*argv)[1]);
     tree->queries = malloc (tree->height * sizeof(query));
     *argc -= 2; *argv += 2;
     
     j=0; 
     for (i=0; i < tree->height; i++)
	{ tree->queries[i].query = ++j;
	  tree->queries[i].dist = -1;
	}
     tree->height = i; 

     nobjs = tree->n;
     od = malloc ((nobjs - tree->height) * sizeof (Tod));
     i=0;
     while (j < nobjs) od[i++].obj = ++j;
     tree->node = buildfqvpt (tree,od,nobjs-tree->height,0);
     free (od);
     prnstats((Index)tree); 
     return (Index)tree;
   }

static void freefqvpt (fqvpnode *node, int depth, int arity)

   { if (depth==0) freebucket (node->u.hoja.bucket,node->u.hoja.size);
     else 
	{ int i;
          if (node->u.interno.children == NULL) return;
	  for (i=0;i<arity;i++)
	      freefqvpt (&child(node,i).child,depth-1,arity);
          free (node->u.interno.children);
	}
   }

void freeIndex (Index S, bool libobj)

   { fqvpt *tree = (fqvpt*)S;
     free (tree->descr);
     free (tree->queries);
     freefqvpt (&tree->node,tree->height,tree->arity);
     free (tree);
     if (libobj) closeDB();
   }

static void savenode (fqvpnode *node, FILE *f, int depth, int arity)

   { int i;
     char more;
     if (depth == 0)
	{ fwrite (&node->u.hoja.size,sizeof(int),1,f);
	  savebucket (node->u.hoja.bucket,node->u.hoja.size,f);
	}
     else
	{ more = (node->u.interno.children == NULL) ? '0' : '1';
          putc (more,f);
          if (more == '0') return;
          for (i=0;i<arity;i++)
	      { fwrite (&child(node,i).dist, sizeof(Tdist),1,f);
		savenode(&child(node,i).child,f,depth-1,arity);
	      }
	}
   }

void saveIndex (Index S, char *fname)

   { FILE *f = fopen(fname,"w");
     fqvpt *tree = (fqvpt*)S;
     int i;
     fwrite (tree->descr,strlen(tree->descr)+1,1,f);
     fwrite (&tree->height,sizeof(int),1,f);
     fwrite (&tree->arity,sizeof(int),1,f);
     for (i=0;i<tree->height;i++)
         fwrite(&tree->queries[i].query,sizeof(int),1,f);
     savenode (&tree->node,f, tree->height, tree->arity);
     fclose (f);
   }

static void loadnode (fqvpnode *node, FILE *f, int depth, int arity)

   { int i;
     char more;
     if (depth == 0)
	{ fread (&node->u.hoja.size,sizeof(int),1,f);
	  node->u.hoja.bucket = loadbucket (node->u.hoja.size,f);
	}
     else
	{ more = getc(f);
          if (more == '0')
	     { node->u.interno.children = NULL;
	       return;
	     }
          node->u.interno.children = malloc(arity*sizeof(Tchild));
	  for (i=0;i<arity;i++)
	      { fread (&child(node,i).dist, sizeof(Tdist),1,f);
		loadnode (&child(node,i).child,f,depth-1,arity);
	      }
	}
   }

Index loadIndex (char *fname)

   { char str[1024]; char *ptr = str;
     FILE *f = fopen(fname,"r");
     fqvpt *tree = malloc (sizeof(fqvpt));
     int i;
     while ((*ptr++ = getc(f)));
     tree->descr = malloc (ptr-str);
     strcpy (tree->descr,str);
     openDB (str);
     fread (&tree->height,sizeof(int),1,f);
     fread (&tree->arity,sizeof(int),1,f);
     tree->queries = malloc (tree->height * sizeof(query));
     for (i=0;i<tree->height;i++)
         fread(&tree->queries[i].query, sizeof(int),1,f);
     loadnode (&tree->node,f,tree->height,tree->arity);
     fclose (f);
     return (Index)tree;
   }

static int _search (fqvpt *tree, fqvpnode *node, Obj obj, Tdist r, int depth,
		  int arity, bool show)

   { int rep = 0;
     if (tree->height == depth)
        { rep += searchbucket (node->u.hoja.bucket,node->u.hoja.size,obj,r,show);
        }
     else
        { int i;
          Tdist dist;
          if (node->u.interno.children == NULL) return rep;
          dist = tree->queries[depth].dist;
          for (i=0;i<arity;i++)
              if (((i==arity-1)||(child(node,i+1).dist > dist-r)) &&
                  (child(node,i).dist <= dist+r))
                 rep += _search(tree,&child(node,i).child,obj,
                                r,depth+1,arity,show);
        }
     return rep;
   }


int search (Index S, Obj obj, Tdist r, int show)

   { int i;
     int rep = 0;
     fqvpt *tree = (fqvpt*)S;
     for (i=0;i<tree->height;i++) 
	 { tree->queries[i].dist = distance (obj,tree->queries[i].query);
	   if (tree->queries[i].dist <= r)
	      { rep++;
		if (show) printobj(tree->queries[i].query);
	      }
	 }
     return rep + _search (tree,&tree->node,obj,r,0,tree->arity,show);
   }

static void _searchNN (fqvpt *tree, fqvpnode *node, Obj obj, Tcelem *res, 
		       int depth)

   { int arity = tree->arity;
     if (tree->height == depth)
        { searchbucketNN (node->u.hoja.bucket,node->u.hoja.size,obj,res);
        }
     else
	{ int i,ci,d;
          Tdist dist;
	  bool ea,eb;
          if (node->u.interno.children == NULL) return;
          dist = tree->queries[depth].dist;
          for (ci=0;ci<arity;ci++)
             if (child(node,ci).dist > dist) break;
          ci--; ea = eb = false;
          for (d=0;d<=arity;d++)
              { i = ci-d;
                if (i < 0) ea = true;
                if (!ea)
                   { if ((i==arity-1) || (radCelem(res) == -1) ||
			 (dist-child(node,i+1).dist <= radCelem(res)))
                       _searchNN (tree,&child(node,i).child,
                                  obj,res,depth+1);
                     else ea = true;
                   }
                if (d==0) continue;
		i = ci+d;
                if (i >= arity) eb = true;
                if (!eb)
                   { if ((radCelem(res) == -1) ||
			 (child(node,i).dist-dist <= radCelem(res)))
                        _searchNN (tree,&child(node,i).child,
                                   obj,res,depth+1);
                     else eb = true;
                   }
              }
	}
   }

Tdist searchNN (Index S, Obj obj, int k, bool show)

   { fqvpt *tree = (fqvpt*)S;
     Tdist mdif;
     int i;
     Tcelem res = createCelem(k);
     for (i=0;i<tree->height;i++) 
	 { tree->queries[i].dist = distance (obj,tree->queries[i].query);
           addCelem (&res,tree->queries[i].query,tree->queries[i].dist);
	 }
     _searchNN (tree,&tree->node,obj,&res,0);
     if (show) showCelem (&res);
     mdif = radCelem(&res);
     freeCelem (&res);
     return mdif;
   }


static int numnodes (fqvpnode *node, int depth, int arity)

   { int i,ret=1;
     if (depth == 0) return 0;
     if (node->u.interno.children == NULL) return 0;
     for (i=0;i<arity;i++)
	 ret += numnodes(&child(node,i).child,depth-1,arity);
     return ret;
   }

static int numbucks (fqvpnode *node, int depth, int arity)

   { int i,ret=0;
     if (depth == 0) return 1;
     if (node->u.interno.children == NULL) return 0;
     for (i=0;i<arity;i++)
	 ret += numbucks(&child(node,i).child,depth-1,arity);
     return ret;
   }

static int sizebucks (fqvpnode *node, int depth, int arity)

   { int i,ret=0;
     if (depth == 0) return node->u.hoja.size;
     if (node->u.interno.children == NULL) return 0;
     for (i=0;i<arity;i++)
	 ret += sizebucks(&child(node,i).child,depth-1,arity);
     return ret;
   }

void prnstats (Index S)

   { fqvpt *tree = (fqvpt*)S;
     int nbucks = numbucks(&tree->node,tree->height,tree->arity);
     int sbucks = sizebucks(&tree->node,tree->height,tree->arity);
     printf ("Number of elements: %i\n",sbucks);
     printf ("fixed height: %i\n",tree->height);
     printf ("arity: %i\n",tree->arity);
     printf ("number of nodes: %i\n",
		numnodes(&tree->node,tree->height,tree->arity));
     printf ("number of buckets: %i\n",nbucks);
     printf ("average size of bucket: %0.2f\n", sbucks/(double)nbucks);
   }
