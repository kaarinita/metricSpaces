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

#ifndef CABECERASINCLUDED
#define CABECERASINCLUDED



#include <sys/types.h>
#include <sys/stat.h>
#include <sys/times.h>
#include <stdlib.h>
#include <math.h>
#include "../../obj.h"
#include "../../index.h"
#include "../../basics.h"
#include <time.h>
#include <sys/times.h>

#define COM(X)
#define MAX_PAL 255
// s
#define _Aesa_ 115


#ifdef DISCR
#define INT_MAX 200
#else
#define INT_MAX 100.0
#endif

/* Estructura para almacenar las caracteristicas de cada objeto*/
typedef struct elem_caja
{
	Obj obj;                        // indice del objeto
	char marcado;
	Tdist dist_aprox;
	int sig;
}obj_gcv;

typedef struct t_fq
{
    int np;			// numero de pivotes
    char *dbname;	
}fq;

typedef struct t_Id_Dist
{
    int i;
    Tdist d;
}Id_Dist;

typedef struct t_dobleEntero
{
    int i, lcs;
}dobleEntero;

typedef struct t_nodo
{
    Tdist  d;
    double valor;
    long int elem;
}nodo;

obj_gcv *elem;                // guarda todoslos elementos de la BD


/****  Funciones propias ****/

int 			compara(const void *, const void *);
void			llena_arreglo_con_elementos_bd(fq *a);
dobleEntero* 		mide_LCS_BD(int *etiq, fq *a);
int			suma(int indice, int cont, int np, int *permut, fq *a, int, int *, char);
int			escoge_siguiente(int *ini, int tam, int cont, Obj obj, fq *a, char funcion);
void			marca_arreglo(nodo *piv, int num, int valor);
int			escoge_mejor_pivote(int num_pivotes_aleatorios, nodo **piv_temp,
                		int *sig, int tam, int num, Obj obj, Tdist r, bool show, int *cont);
int search_metodo(Index S, Obj obj, Tdist r, bool show);
Tdist sumaAESA(int indice, int cont);
int escoge_siguiente_dist(int *ini, int tam, int cont);
Tdist suma_dist(int indice, int cont);
Tdist suma_dist_l1(int indice, int cont);

void marca_elementos_descartados(int indice, int np, int *sig, Tdist r,
                        int *eliminados, Tdist d, int num);

void limpia_marcados(int np);
void inicializa_etiquetas(int np);
void search_metodo_NN(Index S, Obj obj, int k, char funcion, Tcelem *res);
int cuenta_lista(int ini);

void prnstats(Index S) ;


#endif
