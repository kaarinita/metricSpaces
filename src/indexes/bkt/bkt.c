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



#include "bkt.h"

static void addbkt (bkt *tree, bknode *node, Obj obj)

   { if (node->hoja)
	{ if (node->u.hoja.size < tree->bsize)
	     { node->u.hoja.bucket = addbucket (node->u.hoja.bucket,
						node->u.hoja.size, obj);
	       node->u.hoja.size++;
	     }
	  else  /* split bucket */
	     { void *buck = node->u.hoja.bucket;
	       int i,n=node->u.hoja.size;
	       node->hoja = false;
	       node->u.interno.nchildren = 0;
	       node->u.interno.children = NULL;
	       node->u.interno.query = elembuck(buck,0);
	       for (i=1;i<n;i++) addbkt (tree,node,elembuck(buck,i));
	       freebucket (buck, n);
	       addbkt (tree,node,obj);
	     }
	}
     else   /* nodo interno */
	{ int i,d;
	  Tdist dist = distance (obj,node->u.interno.query);
	  for (i=0;i<node->u.interno.nchildren;i++)
	      if ((child(node,i).dist <= dist) &&
	          (child(node,i).dist + tree->step > dist)) break;
	  if (i == node->u.interno.nchildren)
	     { node->u.interno.children = realloc(node->u.interno.children,
					        (i+1)*sizeof(Tchild));
               d = dist / tree->step;
	       child(node,i).dist = d * tree->step;
	       child(node,i).child.hoja = true;
	       child(node,i).child.u.hoja.size =0;
	       child(node,i).child.u.hoja.bucket = createbucket ();
	       node->u.interno.nchildren++;
	     }
	  addbkt (tree,&child(node,i).child,obj);
	}
   }

Index build (char *dbname, int n, int *argc, char ***argv)


   { bkt *tree;
     int i;
     if (*argc < 2)
        { fprintf (stderr,"Usage: <program> <args> BUCKET-SIZE STEP\n");
          exit(1);
        }
     tree = malloc (sizeof(bkt));
     tree->descr = malloc (strlen(dbname)+1);
     strcpy (tree->descr,dbname);
     tree->n = openDB (dbname);
     if (n && (n < tree->n)) tree->n = n;

     tree->bsize = atoi((*argv)[0]);
#ifdef CONT
     sscanf ((*argv)[1],"%f",&tree->step);
#else
     tree->step = atoi((*argv)[1]);
#endif
     tree->node.hoja = true;
     tree->node.u.hoja.bucket = createbucket();
     tree->node.u.hoja.size = 0;
     (*argc) -= 2; (*argv) += 2;

     i=0;
     while (i < tree->n)
        {
          i++;
          addbkt (tree,&tree->node,i);
        }
     //prnstats((Index)tree);
     return (Index)tree;
   }


static void freebkt (bknode *node)

   { if (node->hoja) freebucket (node->u.hoja.bucket,node->u.hoja.size);
     else
	{ int i;
	  for (i=0;i<node->u.interno.nchildren;i++)
	      freebkt (&child(node,i).child);
          free (node->u.interno.children);
	}
   }

void freeIndex (Index S, bool closedb)

   { bkt *tree = (bkt*)S;
     free (tree->descr);
     freebkt (&tree->node);
     free (tree);
     if (closedb) closeDB();
   }

static void savenode (bknode *node, int bsize, FILE *f)

   { char hoja = node->hoja ? '1' : '0';
     int i;
     Obj o;
     putc (hoja,f);
     if (node->hoja)
	{ if (bsize == 1)  /* 1 es mas compacto */
             { o = elembuck(node->u.hoja.bucket,0);
               fwrite (&o,sizeof(int),1,f);
	     }
          else { fwrite (&node->u.hoja.size,sizeof(int),1,f);
                 savebucket (node->u.hoja.bucket,node->u.hoja.size,f);
               }
	}
     else
	{ fwrite (&node->u.interno.query,sizeof(int),1,f);
	  fwrite (&node->u.interno.nchildren,sizeof(int),1,f);
	  for (i=0;i<node->u.interno.nchildren;i++)
	      { fwrite (&child(node,i).dist, sizeof(Tdist),1,f);
		savenode(&child(node,i).child,bsize,f);
	      }
	}
   }

void saveIndex (Index S, char *fname)

   { FILE *f = fopen(fname,"w");
     bkt *tree = (bkt*)S;
     fwrite (tree->descr,strlen(tree->descr)+1,1,f);
     fwrite (&tree->n,sizeof(int),1,f);
     fwrite (&tree->bsize,sizeof(int),1,f);
     fwrite (&tree->step,sizeof(Tdist),1,f);
     savenode (&tree->node,tree->bsize,f);
     fclose (f);
   }

static void loadnode (bknode *node, int bsize, FILE *f)

   { char hoja;
     int i;
     hoja = getc(f);
     node->hoja = (hoja == '1');
     if (node->hoja)
	{  if (bsize == 1)  /* es mas simple */
               node->u.hoja.size = 1;
          else fread (&node->u.hoja.size,sizeof(int),1,f);
	  node->u.hoja.bucket = loadbucket (node->u.hoja.size,f);
	}
     else
	{ fread (&node->u.interno.query,sizeof(int),1,f);
	  fread (&node->u.interno.nchildren,sizeof(int),1,f);
	  node->u.interno.children =
	      	        malloc(node->u.interno.nchildren*sizeof(Tchild));
	  for (i=0;i<node->u.interno.nchildren;i++)
	      { fread (&child(node,i).dist, sizeof(Tdist),1,f);
		loadnode (&child(node,i).child, bsize,f);
	      }
	}
   }

Index loadIndex (char *fname)

   { char str[1024]; char *ptr = str;
     FILE *f = fopen(fname,"r");
     bkt *tree = malloc (sizeof(bkt));
     while ((*ptr++ = getc(f)));
     tree->descr = malloc (ptr-str);
     strcpy (tree->descr,str);
     openDB (str);
 
    fread (&tree->n,sizeof(int),1,f);
     fread (&tree->bsize,sizeof(int),1,f);
     fread (&tree->step,sizeof(Tdist),1,f);
     loadnode (&tree->node,tree->bsize,f);

     fclose (f);
     return (Index)tree;
   }

static int _search (bknode *node, Obj obj, Tdist r, bool show, Tdist step)

   { int rep = 0;
     if (node->hoja)
	{ rep += searchbucket(node->u.hoja.bucket,node->u.hoja.size,obj,r,show);
	}
     else
	{ int i;
          Tdist dist;
	  dist = distance (obj, node->u.interno.query);
	  if (dist <= r) { rep++; if (show) printobj(node->u.interno.query); }
	  for (i=0;i<node->u.interno.nchildren;i++)
	      if ((child(node,i).dist+step > dist-r) &&
	          (child(node,i).dist <= dist+r))
		 rep += _search(&child(node,i).child,obj,r,show,step);
	}
     return rep;
   }

int search (Index S, Obj obj, Tdist r, int show)

   { bkt *tree = (bkt*)S;
     return _search (&tree->node,obj,r,show,tree->step);
   }

static void _searchNN (bknode *node, Obj obj, Tcelem *res, Tdist step)

   { if (node->hoja)
	{ searchbucketNN (node->u.hoja.bucket,node->u.hoja.size,obj,res);
	}
     else
	{ int i;
          Tdist maxd,dist,d;
	  dist = distance (obj, node->u.interno.query);
          addCelem (res,node->u.interno.query,dist);
          maxd = 0;
          for (i=0;i<node->u.interno.nchildren;i++)
              if (maxd < child(node,i).dist) maxd = child(node,i).dist;
          for (d=0; ((radCelem(res)==-1)||(d<radCelem(res)+step)) &&
                    ((d<=dist)||(d<maxd+step-dist)); d+=step)
            { for (i=0;i<node->u.interno.nchildren;i++)
                 if ((child(node,i).dist <= dist-d) &&
                     (child(node,i).dist+step > dist-d))
                    { _searchNN(&child(node,i).child,obj,res,step);
		      break;
		    }
              if ((d==0) ||
                  ((radCelem(res)!=-1)&&(d>=radCelem(res)+step))) continue;
              for (i=0;i<node->u.interno.nchildren;i++)
                 if ((child(node,i).dist <= dist+d) &&
                     (child(node,i).dist+step > dist+d))
                    { _searchNN(&child(node,i).child,obj,res,step);
		      break;
		    }
            }
	}
   }

Tdist searchNN (Index S, Obj obj, int k, bool show)

   { bkt *tree = (bkt*)S;
     Tdist mdif;
     Tcelem res = createCelem(k);
     _searchNN (&tree->node,obj,&res,tree->step);
     if (show) showCelem (&res);
     mdif = radCelem(&res);
     freeCelem (&res);
     return mdif;
   }

