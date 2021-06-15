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

#include "aesa.h"
nodo *mejores;
Tdist **DISTANCIAS;

void prnstats(Index S);

Tdist dame_distancia(int x, int y)
{
    return DISTANCIAS[x][y];
}

Index build (char *dbname, int n, int *argc, char ***argv)
{
	fq *a;
	a = (fq *)malloc(sizeof(struct t_fq));
	a->dbname = malloc(strlen(dbname)+1);
	strcpy(a->dbname, dbname);
	a->np = openDB(dbname);	

        if (n && (n < a->np)) a->np = n; 
	
	elem = (obj_gcv *)malloc(a->np * sizeof(struct elem_caja));
   llena_arreglo_con_elementos_bd(a);      
        //prnstats((Index)a); 
	return (Index)a;
}

int compara(const void *a, const void *b)
{
    Id_Dist *x = (Id_Dist *) a;
    Id_Dist *y = (Id_Dist *) b;
    if(x->d > y->d)	return 1;
    if(x->d < y->d)	return -1;
    return 0;
}
int compara_dobleEnteros(const void *a, const void *b)
{
	dobleEntero *x, *y;
	x = (dobleEntero *) a;
	y = (dobleEntero *) b;

	if(x->lcs > y->lcs) return 1;
	if(x->lcs < y->lcs) return -1;
	return 0;
}
void actualiza_lista(int indice, int *ant, int *sig, int *inicio)
{
	if(*inicio == indice)
		*inicio = elem[indice].sig;
	if(*ant == indice)
		*sig = *ant = elem[indice].sig;
	else	// ant no cambia
	{
		elem[*ant].sig = *sig = elem[indice].sig;
	}
	elem[indice].sig = -1;
}

void eliminar_elemento(int indice, int *ant, int *sig, int *eliminados, int *inicio)
{
	if(elem[indice].marcado == 0)
		(*eliminados)++;
	elem[indice].marcado = 1;        // lo marco como eliminado
}


/*********************** BUSQUEDA *************************************/
int search(Index S, Obj obj, Tdist r, bool show)
{
	return search_metodo(S, obj, r, show);
}

int search_metodo(Index S, Obj obj, Tdist r, bool show)
{
    fq *a;
	int eliminados=0, sig=0, indice, siguiente, cont=0, num=0, ant=0;
    Tdist d;

    a = (fq *)S;
	
    mejores = (nodo *) malloc(a->np*sizeof(nodo));
    limpia_marcados(a->np);
	
    sig = indice = 0;
	
    while(eliminados < a->np && indice != -1)
    {
	eliminar_elemento(indice, &ant, &siguiente, &eliminados, &sig);
	d = distance(obj, elem[indice].obj);
		
	mejores[num].elem = indice;
        mejores[num++].d = d;
 		
        if(d <= r)
        {
            cont++;
	    if(show)
		printobj(elem[indice].obj);     
	}
	marca_elementos_descartados(indice, a->np, &sig, r, &eliminados,  d, num-1);
	indice = escoge_siguiente_dist(&sig, a->np, num);	
	ant = siguiente = sig;
    }
    free(mejores);
	
    return cont;
}

int suma(int indice, int cont, int np, int *permut, fq *a, int posicionQ, int* etiqQinv, char funcion)
{
    int lcs=0;
	elem[indice].dist_aprox = suma_dist(indice, cont);
	
    return lcs;
}

/*********************** otras ******************************************/

void llena_arreglo_con_elementos_bd(fq *a)
{
	int i, j,k=0;
	
	DISTANCIAS = (Tdist **)malloc(sizeof(Tdist *) * (a->np));
	for(i=0; i<a->np; i++)
	{
        elem[i].obj = ++k;
	    DISTANCIAS[i] = (Tdist *) malloc(sizeof(Tdist) * a->np);    
    }
    for(i=0; i<a->np; i++)
    {
		DISTANCIAS[i][i] = (Tdist)0;
		for(j=i+1; j<a->np; j++)
		{
			DISTANCIAS[j][i] = DISTANCIAS[i][j] = distance(elem[i].obj, elem[j].obj);
		}
    }
}


void limpia_marcados(int np)
{
    int i;
    for(i=0; i<np; i++)
	{
	elem[i].marcado = 0;
	elem[i].dist_aprox = (Tdist) 0;
	elem[i].sig = i+1;
	} 
	elem[i-1].sig = -1;
}

/******************* CODIGO AESA ****************************/
void marca_elementos_descartados(int indice, int np, int *sig, Tdist r,
			int *eliminados, Tdist d, int num)
{
    int i,j, ant=*sig, siguiente;

    siguiente = ant = *sig;
    for(i=*sig; i!=-1; i=siguiente)  // los K elementos ya fueron comparados directamente
    {   // reviso los elementos y los comparo solo si son posibles candidatos los con
		for(j=0; j<=num; j++)	
		{
#ifdef DISCR
			if(abs(dame_distancia(i, mejores[j].elem) - mejores[j].d ) > r)
#else
			if(fabs(dame_distancia(i, mejores[j].elem) - mejores[j].d ) > r)
#endif
			{
				eliminar_elemento(i, &ant, &siguiente, eliminados, sig);
				actualiza_lista(i, &ant, &siguiente, sig);	
				break;
			}
		}
		if(j>num)
		{
			if(elem[i].marcado == 1)
				actualiza_lista(i, &ant, &siguiente, sig);
			else
			{
				if(ant != i)
					elem[ant].sig = i;
				ant = i;
				siguiente = elem[ant].sig;
			}
		}
    }
}
int escoge_siguiente_aleatorio(int *ini, int tam, int cont)
{
    int i;
    for(i=*ini; i!= -1; i=elem[i].sig)
    {
        if(elem[i].marcado == 0)
	    return i;
    }
    return -1;
}


int escoge_siguiente_dist(int *ini, int tam, int cont)
{
    int i, prox=-1;
    Tdist aux=(Tdist)0, min=(Tdist)1000; 
    for(i=*ini; i!= -1; i=elem[i].sig)
    {
        if(elem[i].marcado == 0)
        {
            aux = suma_dist(i, cont);
            if(aux < min || prox == -1)       // la primera vez es seguro que la minimiza
            {
                min = aux;
                prox = i;
            }
        }
    }
    return prox;
}

int escoge_siguiente_dist_l1(int *ini, int tam, int cont)
{
    int i, prox=-1, bandera=0;
    Tdist aux, min=300; 
    for(i=*ini; i!= -1; i=elem[i].sig)
    {
        if(elem[i].marcado == 0)
        {
            aux = suma_dist_l1(i, cont);
            if(aux < min)       
            {
                min = aux;
                prox = i;
                if(bandera==0)
                {
                    *ini=i;
                    bandera = 1;
                }
            }
        }
    }
    return prox;
}

Tdist suma_dist(int indice, int cont)
{
#ifdef DISCR
        elem[indice].dist_aprox += abs(mejores[cont-1].d - dame_distancia(indice, mejores[cont-1].elem));
#else
        elem[indice].dist_aprox += fabs(mejores[cont-1].d - dame_distancia(indice, mejores[cont-1].elem));
#endif
    return elem[indice].dist_aprox; 
}

Tdist suma_dist_l1(int indice, int cont)
{
    Tdist d;
#ifdef DISCR
        d = abs(mejores[cont-1].d - dame_distancia(indice, mejores[cont-1].elem));
#else
        d = fabs(mejores[cont-1].d - dame_distancia(indice, mejores[cont-1].elem));
#endif
    if(elem[indice].dist_aprox < d)
	elem[indice].dist_aprox = d;
    return elem[indice].dist_aprox; 
}

Tdist sumaAESA(int indice, int cont)
{
    Tdist resultado;
    int j;	
    resultado = 0;
    for(j=0; j<cont; j++)
#ifdef DISCR
        resultado += abs(mejores[j].d - dame_distancia(indice, mejores[j].elem));
#else
        resultado += fabs(mejores[j].d - dame_distancia(indice, mejores[j].elem));
#endif
    return resultado;
}

void prnstats(Index S)
{
}

Tdist searchNN (Index S, Obj obj, int k, bool show)
{
     Tdist mdif;
     Tcelem res = createCelem(k);
     search_metodo_NN(S, obj, k, _Aesa_, &res);	
     if (show) showCelem (&res);
     mdif = radCelem(&res);
     freeCelem (&res);
     return mdif;

}

void freeIndex(Index S, bool libobj)
{
    fq *a = (fq*)S;
    free(a);
}
void saveIndex(Index S, char *fname)
{
    long int j, i;
    FILE *fp;
    fq *a = (fq *)S;
				
    if( (fp=fopen(fname,"w")) == NULL )
    {
	fprintf(stderr,"Error opening output file\n");
        exit(-1);
    }
    fwrite(a->dbname,strlen(a->dbname)+1,1,fp);
    fwrite(&a->np, sizeof(int), 1, fp);
						
    for(i=0; i<a->np; i++)
            fwrite(&elem[i].obj, sizeof(Obj), 1, fp);
    for(i=0; i<a->np; i++)
          for(j=0; j<a->np; j++)
              fwrite(&DISTANCIAS[i][j], sizeof(Tdist), 1, fp);
			              
    fclose(fp);
}

Index loadIndex(char *fname)
{
    char str[1024]; char *ptr = str;	
    long int j, i;
    FILE *fp;
    fq *a;
				
    if( (fp=fopen(fname,"r")) == NULL )
    {
	fprintf(stderr,"Error loading file\n");
        exit(-1);
    }
    a =  malloc(sizeof(a) );

     while ((*ptr++ = getc(fp)));
     a->dbname = malloc (ptr-str);
     strcpy (a->dbname,str);

    fread(&a->np, sizeof(int), 1, fp);
 				
    DISTANCIAS = (Tdist **) malloc(sizeof(Tdist *) * a->np);		
     elem = (obj_gcv *)malloc(a->np * sizeof(struct elem_caja));
    for(i=0; i<a->np; i++)
    {
            fread(&elem[i].obj, sizeof(Obj), 1, fp);
	    DISTANCIAS[i] = (Tdist *) malloc(sizeof(Tdist) * a->np);
    }
    for(i=0; i<a->np; i++)
          for(j=0; j<a->np; j++)
              fread(&DISTANCIAS[i][j], sizeof(Tdist), 1, fp);
			              
    fclose(fp);
    openDB(a->dbname);
    return (Index)a;
}

int cuenta_lista(int ini)
{
	int cont=0;
	while(ini != -1)
	{
		cont++;
		ini = elem[ini].sig;
	}
	return cont;
}

void search_metodo_NN(Index S, Obj obj, int k, char funcion, Tcelem *res)
{
    fq *a;
	int eliminados=0, sig=0, indice, siguiente=0, num=0, ant=0;
    Tdist d, r=(Tdist)10000;

    a = (fq *)S;
    mejores = (nodo *) malloc(a->np*sizeof(nodo));
    limpia_marcados(a->np);
	
    sig = indice = 0;
	
    while(eliminados < a->np && indice != -1)
    {
	eliminar_elemento(indice, &ant, &siguiente, &eliminados, &sig);
	d = distance(obj, elem[indice].obj);
	mejores[num].elem = indice;
        mejores[num++].d = d;
        if(d <= r)
        {
	        addCelem (res,elem[indice].obj,d);
		if(res->csize >= k)
    		  r = radCelem(res);
        }
	marca_elementos_descartados(indice, a->np, &sig, r, &eliminados, d, num-1);
	if(funcion == _Aesa_)
		indice = escoge_siguiente_dist(&sig, a->np, num);	
	ant = siguiente = sig;
    }
    free(mejores);	
}

