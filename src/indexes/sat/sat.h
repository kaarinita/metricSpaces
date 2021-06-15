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

#include <stdio.h>
#include "../../index.h"
#include "../../basics.h"

typedef struct
   { Obj elem;
     Tdist dist;   /* mejor dist */
     int who;   /* a que vecino es */
   } qelem;

typedef struct
   { qelem *data;
     int ndata;
   } arrdata;

typedef struct
   { int *num;
     int nnum;
   } arrnum;

typedef struct
  { Obj obj;
    Tdist maxd;
    arrnum vec;
    arrdata queue;
  } nodo;

typedef struct
    { int id;
      Tdist dist;  /* dist to q */
      Tdist lbound;  /* lower bound */
      Tdist mind; /* best distsance */
    } heapElem;

typedef struct
  { nodo *nodos;
    int nnodos;
    int np;
    char *descr;
  } grafo;

static double avgprof (grafo G, int n, int height, int *num);

