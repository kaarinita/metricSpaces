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

#include "iaesa.h"
nodo *mejores;
Tdist **DISTANCIAS;

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
	
	a->tamEtiq = 30;    //just to realloc 
	elem = (obj_gcv *)malloc(a->np * sizeof(struct elem_caja));
       llena_arreglo_con_elementos_bd(a->np, a->np);      
        //prnstats((Index)a); 
	return (Index)a;
}
/************** COMPARACIONES ENTRE OBJETOS *******************/
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
	elem[indice].marcado = 1;  
	
	if(elem[indice].etiqueta != NULL)
	{
		free(elem[indice].etiqueta);
		elem[indice].etiqueta = NULL;
	}
}


/*********************** BUSQUEDA *************************************/
int search(Index S, Obj obj, Tdist r, bool show)
{
  return  search_metodo(S, obj, r, show, _Iaesa_);
}

void inicializa_etiquetas(fq *a)
{
	int i;
	for(i=0; i<a->np; i++)
	{	
		elem[i].etiqueta = malloc(sizeof(int) * a->tamEtiq);
		elem[i].inversa = malloc(sizeof(int) * a->tamEtiq);
	}
}


int search_metodo(Index S, Obj obj, Tdist r, bool show, char funcion)

{
    fq *a;
    int eliminados=0, sig=0, indice, siguiente, cont=0, num=0, ant;
    Tdist d;
	
    a = (fq *)S;
	
    mejores = (nodo *) malloc(a->np*sizeof(nodo));
    limpia_marcados(a->np);
	
    inicializa_etiquetas(a);
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
        indice = escoge_siguiente(&sig, a->np, num, obj, a, funcion);
	ant = siguiente = sig;
    }
	free(mejores);
	
    return cont;
}



/*********************** funciones utiles ******************************************/
int distPosicion(int tam, int *etiqInv, int *y,  int *x_toda, fq *a, int indice)
{
	int i, t, total=0;
//    static int *temp = NULL;
//	if(temp == NULL)
  //              temp = (int *)malloc(a->np * sizeof(int));
    //    for(i=0; i<tam; i++)
      //          temp[x_toda[i]] = i; 	// compone la secuencia

	for(i=0; i<tam; i++)
	{
		t = i - etiqInv[y[i]];
		t = t > 0 ? t : t*-1;
//		total += abs((i-etiqInv[y[i]]));
		total += t * t;
	}
	return total;
}


void imprime_permutacion(int *p, int tam)
{
    int i;
    printf("p[");
    for(i=0; i<tam; i++)
		printf("%d,",p[i]);
    printf("]\n");
}

void imprime_permutaciones_lcs(int *p1, int *p2, int tam, int lcs)
{
    imprime_permutacion(p1, tam);
    imprime_permutacion(p2, tam);
    printf("lcs=%d\n", lcs);
}
void reasignar_memoria(int indice, int cont, fq *a)
{
	int i, multiplo = cont / a->tamEtiq;
	int *nueva = malloc(sizeof(int) * a->tamEtiq * (multiplo+1));
	int *InvNueva = malloc(sizeof(int) * a->tamEtiq * (multiplo+1));
	
	for(i=0; i<cont; i++)
	{
		nueva[i] = elem[indice].etiqueta[i];
		InvNueva[i] = elem[indice].inversa[i];
	}
	free(elem[indice].etiqueta);
	free(elem[indice].inversa);

	elem[indice].etiqueta = nueva;
	elem[indice].inversa = InvNueva;
}

void insertsort_elemento_simple(int **permut, int cont, int indice, fq *a)
{
        Tdist v;
        int j;
        if(a->tamEtiq < cont && cont % a->tamEtiq == 1)             // solo debe actualizarse si es que es la$                
		reasignar_memoria(indice, cont, a);
                
        (*permut)[cont-1] = cont-1; 
        v = dame_distancia(indice, mejores[cont-1].elem);
        j=cont-1;
        while(j > 0 && v < dame_distancia(indice, mejores[ (*permut)[j-1] ].elem) )
        {
                (*permut)[j] = (*permut)[j-1];
                j--;
	}
        (*permut)[j] = cont-1;
}
// usando la inversa
int insertsort_elemento(int **permutU, int cont, int indice, int *etiqQinv, 
							int posicionQ, int *permutQ, fq *a)
{
	int i, Sp=0, posU=0;
	
	if(a->tamEtiq < cont && cont % a->tamEtiq == 1)		// solo debe actualizarse si es que es la primera vez que se ocupa
		reasignar_memoria(indice, cont, a);
	for(i=0; i<cont-2; i++)
	{
		if(dame_distancia(indice, mejores[ permutQ[cont-1] ].elem) 
						  < dame_distancia(indice, mejores[ permutQ[i] ].elem ))
		{
			posU++;
			elem[indice].inversa[ permutQ[i] ] ++;
		}
		Sp += abs(elem[indice].inversa[ permutQ[i] ] - etiqQinv[ permutQ[i] ]);
	}
	elem[indice].inversa[cont-1] = i - posU;
	Sp += abs(elem[indice].inversa[permutQ[cont-1]] - etiqQinv[cont-1]);
	return Sp;
}

int suma(int indice, int cont, int np, int *permut, fq *a, int posicionQ, int* etiqQinv, char funcion)
{
    int lcs=0;
	if(funcion == _Iaesa_ || funcion == _Iaesa2_ || posicionQ < cont/2)    // fza bruta
	{
		insertsort_elemento_simple(&elem[indice].etiqueta, cont, indice, a);
 		lcs = distPosicion(cont, etiqQinv, elem[indice].etiqueta, permut, a, indice);
	}

	elem[indice].Sp = lcs;
	elem[indice].dist_aprox = suma_dist(indice, cont);
	
    return lcs;
}

int insertsort(int **permut, int cont)
{
	Tdist v;
	int j;
	(*permut)[cont-1] = cont-1;
	v = mejores[cont-1].d;
	j=cont-1;
	while(j>0 && v < mejores[ (*permut)[j-1] ].d)
	{
		(*permut)[j] = (*permut)[j-1];
		j--;
	}
	(*permut)[j] = cont-1;
	return j;
}
int* forma_canonica(int* x_toda, int tam, fq *a)
{
	int i, *temp = NULL;
	if(temp == NULL)
		temp = (int *)malloc(tam * sizeof(int));
	for(i=0; i<tam; i++)
		temp[x_toda[i]] = i; 	// compone la secuencia	
	return temp;
}
/******** criterio de permutantes ***********/
int escoge_siguiente(int *ini, int tam, int cont, Obj obj, fq *a, char funcion)
{
    int i=0, posicionQ=-1, elegido=-1;
    int min = -1, sig=0, *etiquetaQinv;
    static int *permut = NULL;    
	
    if(permut == NULL)
        permut = (int *) malloc(sizeof(int) * a->np);
	posicionQ = insertsort(&permut, cont);				// la nueva permutacion de la consulta

	etiquetaQinv = forma_canonica(permut, cont, a);
	
    for(i=*ini; i!=-1; i=elem[i].sig)		// busco cual es el siguiente mejor elemento
    {
		if(elem[i].marcado == 0)
        {
            sig = suma(i, cont, a->np, permut, a, posicionQ, etiquetaQinv, funcion);
            if(sig < min || elegido == -1)
            {
                min = sig;
                elegido = i;
            }
        }
    }
	free(etiquetaQinv);
    return elegido;
}

int escoge_siguiente_desempate(int *ini, int tam, int cont, Obj obj, fq *a, char funcion)
{
    int i, elegido=-1;
    int min = -1, sig=0, posicionQ, *etiquetaQinv;
    static int *permut = NULL;    
	
    if(permut == NULL)
        permut = (int *) malloc(sizeof(int) * a->np);
	posicionQ = insertsort(&permut, cont);				// la nueva permutacion de la consulta
	
	etiquetaQinv = forma_canonica(permut, cont, a);
	
    for(i=*ini; i!=-1; i=elem[i].sig)		// busco cual es el siguiente mejor elemento
    {
		if(elem[i].marcado == 0)
        {
            sig = suma(i, cont, a->np, permut, a, posicionQ, etiquetaQinv, funcion);
            if(sig < min || elegido == -1)
            {
                min = sig;
                elegido = i;
            }
			else
				if(sig == min)
					elegido = elem[i].dist_aprox < elem[elegido].dist_aprox ? i : elegido;
        }
    }
    return elegido;
}


/*********************** otras ******************************************/

void llena_arreglo_con_elementos_bd(int num_palabras, int nPiv)
{
	int i, j,k=0;
	
	DISTANCIAS = (Tdist **)malloc(sizeof(Tdist *) * num_palabras);
	for(i=0; i<num_palabras; i++)
	{
        elem[i].obj = ++k;
        elem[i].etiqueta = NULL;
        elem[i].inversa = NULL;
        elem[i].marcado = 0;
	    DISTANCIAS[i] = (Tdist *) malloc(sizeof(Tdist) * num_palabras);    
    }
    for(i=0; i<num_palabras; i++)
    {
		DISTANCIAS[i][i] = (Tdist)0;
		for(j=i+1; j<num_palabras; j++)
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
	elem[i].Sp = 0;
	} 
	elem[i-1].sig = -1;
}

/******************* CODIGO AESA ****************************/
// int escoge_siguiente(int *ini, int tam, int cont, Obj obj, fq *a)
void marca_elementos_descartados(int indice, int np, int *sig, Tdist r,
			int *eliminados, Tdist d, int num)
{
    int i, j, ant=*sig, siguiente;

    siguiente = ant = *sig;
    for(i=*sig; i!=-1; i=siguiente)  // los K elementos ya fueron comparados directamente
    {   // reviso los elementos y los comparo solo si son posibles candidatos los con
            // los elem en piv, estan ordenados
		for(j=0; j<=num; j++)		// cambiar -------------- guardar el max nomas.
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
    Tdist aux=(Tdist)0, min=(Tdist)1000; // la distancia max. en palabras es 20
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
    Tdist aux, min=300; // la distancia max. en palabras es 20
    for(i=*ini; i!= -1; i=elem[i].sig)
    {
        if(elem[i].marcado == 0)
        {
            aux = suma_dist_l1(i, cont);
            if(aux < min)       // la primera vez es seguro que la minimiza
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

Tdist searchNN (Index S, Obj obj, int k, bool show)
{
     Tdist mdif;
     Tcelem res = createCelem(k);
     search_metodo_NN(S, obj, k, _Iaesa_, &res);
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
    fq *a;
				
    if( (fp=fopen(fname,"w")) == NULL )
    {
	fprintf(stderr,"Error opening output file\n");
        exit(-1);
    }
	a = (fq *)S;
    fwrite(a->dbname,strlen(a->dbname)+1,1,fp);
    fwrite(&a->np, sizeof(int), 1, fp);
    fwrite(&a->tamEtiq, sizeof(int), 1, fp);
						
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
    a =  (fq*) malloc(sizeof(fq) );

	while ((*ptr++ = getc(fp)));
	a->dbname = malloc (ptr-str);
	strcpy (a->dbname,str);
	
    fread(&a->np, sizeof(int), 1, fp);
    fread(&a->tamEtiq, sizeof(int), 1, fp);
				
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

void search_metodo_NN(Index S, Obj obj, int k, char funcion, Tcelem *res)
{
    fq *a;
    int eliminados=0, sig=0, indice, siguiente=0, num=0, ant=0;
    Tdist d, r=(Tdist)10000;
	
    a = (fq *)S;
    mejores = (nodo *) malloc(a->np*sizeof(nodo));
    limpia_marcados(a->np);
	
    inicializa_etiquetas(a);
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
	indice = escoge_siguiente(&sig, a->np, num, obj, a, funcion);
	ant = siguiente = sig;
    }
	free(mejores);	
}
