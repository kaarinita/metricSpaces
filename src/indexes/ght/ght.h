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

#include "../../basics.h"
#include "../../index.h"

typedef struct
   { Obj *data;
     int ndata;
   } arrdata;

typedef struct sgnode
  { Obj *root;
    int arity;	/* para hojas cambia, ademas podemos implem otras politicas */
    arrdata queue;  /* solo para creacion */
    Tdist *maxd;
    struct sgnode *children;
  } gnode;

typedef struct
  { gnode tree;
    int arity;
    char *descr;
    int np;
  } gnat;


