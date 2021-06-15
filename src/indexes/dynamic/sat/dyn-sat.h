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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../../index.h"
#include "../../../basics.h"


typedef struct
	{ int time;   /* tiempo */
		int pos;   /* nodo */
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
	{ int num;
		int den;
	} fraccion;

typedef struct
	{ Obj obj;
		int time; /* guarda el tiempo en el que se inserto */
		int nsust; /* mantiene el numero de nodos reemplazados en su subarbol */
		int size; /* guarda el nro de nodos en su subarbol */
		Tdist maxd; /* guarda el radio de cobertura */
		Tdist pf; /* guarda la distancia entre el elemento anterior en el nodo y el sustituto */
		bool sust; /* es sustituido? */
		int ptime; /* guarda el timestamp que tenia el sustituto */
		int father; /* mantiene la posicion del padre */
		arrnum vec;
	} nodo;

typedef struct
	{ int id; /* posicion del nodo */
		Tdist r; /* por esta distancia poda debido al timestamp */
		Tdist dist;  /* dist. a q */
		Tdist lbound; /* lower bound */
		Tdist maxsuff; /* max suffix */
	} heapElem;

typedef struct
	{ nodo *nodos;
		int nnodos;
		int nobjs;
		int marity;
		int lastime;
		fraccion factor;
		int libre;
		char *descr;
	} grafo;


static double avgprof (grafo *G, int n, int height, int *num);

static int maxari (grafo *G);
