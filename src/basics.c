
#include "basics.h"

void *_free(void *p) { free(p); return NULL; }
void *verif(void *p) { if (p == NULL) { printf ("Error -- out of memory\n"); exit(
1); } return p; }

Tcelem createCelem (int k)

   { Tcelem celems;
     celems.elems = malloc (k*sizeof(celem));
     celems.k = k;
     celems.csize = 0;
     return celems;
   }

void addCelem (Tcelem *celems, Obj id, Tdist dist)

   { int i,pos = celems->csize,pos2;
     while ((pos>0) && (celems->elems[pos-1].dist > dist)) pos--;
     if (pos == celems->k) return;  /* no entra entre los elegidos */
     pos2=pos;
     while ((pos2>0) && (celems->elems[pos2-1].dist == dist)) 
	   { if (id == celems->elems[pos2].id) return; /* repetido */
 	     pos2--;
           }
     if (celems->csize < celems->k) celems->csize++;
     for (i=celems->csize-1;i>pos;i--) celems->elems[i] = celems->elems[i-1];
     celems->elems[pos].id = id; celems->elems[pos].dist = dist;
   }

bool outCelem (Tcelem *celems, Tdist dist)

   { return ((celems->csize == celems->k) && 
	     (dist > celems->elems[celems->k-1].dist));
   }

void showCelem (Tcelem *celems)

   { int j;
     for (j=0;j<celems->csize;j++)
         { printobj(celems->elems[j].id);
         }
   }

void freeCelem (Tcelem *celems)

   { free (celems->elems);
   }

Tdist radCelem (Tcelem *celems)

   { return (celems->csize > 0) ? celems->elems[celems->csize-1].dist : -1;
   } 


bool insRet(Tret *ret,Obj Or, Obj o, Tdist d)
{
   bool rt=false;
    if(!(ret->iret%ret->chunk))
   {
         ret->ret=(opair *)realloc(ret->ret,((ret->iret/ret->chunk)+1)*ret->chunk*sizeof(*(ret->ret)));
     }
//     if(((ret->ret)[ret->iret].d=distance(Or,o))<=r)
        {
           (ret->ret)[ret->iret].o=Or;
           (ret->ret)[ret->iret].d=d;
          (ret->iret)++;
         rt=true;
       }
     return rt;
     }

void FinsRet(Tret *ret,Obj Or)
{
   if(!(ret->iret%ret->chunk))
      {
           ret->ret=(opair *)realloc(ret->ret,((ret->iret/ret->chunk)+1)*ret->chunk*sizeof(*(ret->ret)));
       }
	 (ret->ret)[ret->iret].o=Or;
	 (ret->iret)++;
}

void iniciaRet(Tret *ret)
{
  ret->chunk = 3;
  ret->iret = 0;
  ret->ret = NULL;
}
