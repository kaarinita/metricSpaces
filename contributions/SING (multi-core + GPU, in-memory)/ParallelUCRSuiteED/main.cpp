/* 
 * File:   main.cpp
 * Author: michelelinardi
 *
 * Created on February 28, 2015, 4:56 PM
 */


/***********************************************************************/
/************************* DISCLAIMER **********************************/
/***********************************************************************/
/** This UCR Suite software is copyright protected © 2012 by          **/
/** Thanawin Rakthanmanon, Bilson Campana, Abdullah Mueen,            **/
/** Gustavo Batista and Eamonn Keogh.                                 **/
/**                                                                   **/
/** Unless stated otherwise, all software is provided free of charge. **/
/** As well, all software is provided on an "as is" basis without     **/
/** warranty of any kind, express or implied. Under no circumstances  **/
/** and under no legal theory, whether in tort, contract,or otherwise,**/
/** shall Thanawin Rakthanmanon, Bilson Campana, Abdullah Mueen,      **/
/** Gustavo Batista, or Eamonn Keogh be liable to you or to any other **/
/** person for any indirect, special, incidental, or consequential    **/
/** damages of any character including, without limitation, damages   **/
/** for loss of goodwill, work stoppage, computer failure or          **/
/** malfunction, or for any and all other damages or losses.          **/
/**                                                                   **/
/** If you do not agree with these terms, then you you are advised to **/
/** not use this software.                                            **/
/***********************************************************************/
/***********************************************************************/



#include <cstdlib>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <string.h>
#include <sched.h>
#include <cmath>
#include <iostream>
#include <stdbool.h>
#include <fstream>
#include "immintrin.h"


extern "C"
{
    #include "pqueue.h"
}
struct timeval total_time_start;
        struct timeval current_time;
        double tS,tE,total_time;

#define INF 1e20       //Pseudo Infitinte number for this code
        #define COUNT_TOTAL_TIME_START gettimeofday(&total_time_start, NULL);   
        #define COUNT_TOTAL_TIME_END  gettimeofday(&current_time, NULL); \
                                      tS = total_time_start.tv_sec*1000000 + (total_time_start.tv_usec); \
                                      tE = current_time.tv_sec*1000000  + (current_time.tv_usec); \
                                      total_time += (tE - tS); 


using namespace std;
typedef struct transfordata
{
    unsigned long  start_number;
    unsigned long  stop_number;
    float * bsfdistance;
    pthread_mutex_t *lock_bsf;
    long long totalDataPatterns;
    float * firsKResults;
    pqueue_t * resultQueue;
    long long pointCheckedTotal;
    float * Q, *query;
    int * order;
    float * rawfile;
    int SIMD_type;
}transfordata;
char const * datafilegl;
int mgl;
int full_time_series_sizegl;
int clean_cache;
void * query_time_series(void *);
int paraplleltrillion( char const *, char const*, int, int, int, int, int,int);
int simpleScan( char const * , int , int );
float ts_euclidean_distance_SIMD(float * t, float * s, int size, float bound);
float ts_euclidean_distance(float * t, float * s, int size, float bound);
void pushDistValueInTheTopK(float * firsKResults, int k, float dist)
{
   int i;
   float valueToInsert =-1;
   for(i=0;i<k;i++)
   {
       if (valueToInsert >= 0)
       {
           float temporary = firsKResults[i];
           firsKResults[i] = valueToInsert;
           valueToInsert = temporary;
           
           if (valueToInsert<0)
           {
               break;
           }
           
       }
       else if(firsKResults[i] > dist || firsKResults[i]<0)
       {
           valueToInsert = firsKResults[i];
           firsKResults[i] =  dist;
           if (valueToInsert<0)
           {
               break;
           }
       }
   }

}

typedef struct query_result {
    float distance;
    long long location;
    int sizeOfQuery;
    size_t pqueue_position; 
} query_result;

static int
cmp_pri(float next, float curr)
{
	return (next > curr);
}


static float
get_pri(void *a)
{
	return (float) ((query_result *) a)->distance;
}


static void
set_pri(void *a, float pri)
{
	((query_result *) a)->distance = (float)pri;
}


static size_t
get_pos(void *a)
{
	return ((query_result *) a)->pqueue_position;
}


static void
set_pos(void *a, size_t pos)
{
	((query_result *) a)->pqueue_position = pos;
}

/// Data structure for sorting the query.
typedef struct Index
    {   float value;
        int    index;
    } Index;
    
    


/// Comparison function for sorting the query.
/// The query will be sorted by absolute z-normalization value, |z_norm(Q[i])| from high to low.
int comp(const void *a, const void* b)
{   Index* x = (Index*)a;
    Index* y = (Index*)b;
    return abs(y->value) - abs(x->value);
}


/// Main function for calculating ED distance between the query, Q, and current data, T.
/// Note that Q is already sorted by absolute z-normalization value, |z_norm(Q[i])|
//float distance(const float * const Q, const float * const T , const int& j , const int& m , const float& mean , const float& std , const int* const order, const float& bsf)
float distance(float * Q, float * T , const int& j , const int& m , const float& mean , const float& std , int* order, const float& bsf, int& pointChecked)
{
    int i;
    pointChecked=0;
    float sum = 0;
    for ( i = 0 ; i < m && sum < bsf ; i++ )
    {
        pointChecked= pointChecked+1;
        float x = (T[(order[i]+j)]-mean)/std;
        sum += (x-Q[i])*(x-Q[i]);
    }
    return sum;
}
float distancenormal(float * Q, float * T , const int& j , const int& m  , int* order, const float& bsf, int& pointChecked)
{
    int i;
    pointChecked=0;
    float sum = 0;
    for ( i = 0 ; i < m && sum < bsf ; i++ )
    {
        pointChecked= pointChecked+1;
        float x = T[(order[i]+j)];
        sum += (x-Q[i])*(x-Q[i]);
    }
    return sum;
}

/// If serious error happens, terminate the program.
void error(int id)
{
    if(id==1)
        printf("ERROR : Memory can't be allocated!!!\n\n");
    else if ( id == 2 )
        printf("ERROR : Dataset File not Found!!!\n\n");
    else if ( id == 3 )
        printf("ERROR : Query File not Found!!!\n\n");
    exit(1);
}

int main(  int argc , char *argv[] )
{
	
	if (argc <= 1)
	{
		printf("Usage: dataset file, query file, size of query, num ts dataset, size ts dataset, query number, thread number,SIMD type(1: use SIMD/0: don't use SIMD)\n");
		printf("\n\nOR for having the statistics of the disk access time for a dataset:\n");
		printf("Usage: dataset file, num ts dataset, size ts dataset\n");
	}
	else
	{
		if(argc == 4)
		{
			int ts_num = atoi(argv[2]);
			int full_time_series_size = atoi(argv[3]);
			simpleScan(argv[1],ts_num,full_time_series_size);
		}
		else
		{	

        //printf("pid is :  %s\n",pidvalue);
        //char mkdircommand[200] = "mkdir /home/pbotao/Documents/testcommand/piddiskvalue/";
        //strcat(mkdircommand,pidvalue);
        //char cpvalue[200] = "cp /proc/";
       // strcat(cpvalue,pidvalue);
        //strcat(cpvalue,"/io /home/pbotao/Documents/testcommand/piddiskvalue/");
        //strcat(cpvalue,pidvalue);

			int size = atoi(argv[3]);
			int ts_num = atoi(argv[4]);
			int full_time_series_size = atoi(argv[5]);
      			int query_number = atoi(argv[6]);
      			int calculate_thread = atoi(argv[7]);
            int SIMD_type= atoi(argv[8]);
			paraplleltrillion(argv[1], argv[2], size,ts_num,full_time_series_size,query_number,calculate_thread,SIMD_type);

            //system(mkdircommand);
		//printf("cpvalue is :  %s\n",cpvalue);
            //system(cpvalue);
		}
	}

}


int paraplleltrillion( char const * datasetFile, char const * queryFile, int m, int ts_num, int full_time_series_size, int query_number, int calculate_thread_number,int SIMD_type)
{

	FILE *fp;              // the input file pointer
	FILE *qp;              // the query file pointer

	FILE *  queryAnswerFile;
	FILE *  completeStatFile;

    datafilegl= datasetFile;
    mgl=m;
    full_time_series_sizegl=full_time_series_size;

	char nameFileQueryResult[255];
	char const * nnameQueryCompleteStat = "completeStatTRILLION";
	time_t rawtime;
	time (&rawtime);
	struct tm* readable;
	readable = localtime(&rawtime);


	pqueue_t * resultQueue;

	float *Q;             // query array
	float *T;             // array of current data
	int *order;            // ordering of query by |z(q_i)|
	float bsf;            // best-so-far

	long long loc;     // answer: location of the best-so-far match
	long long dataScanned = 0;
	float d;
	long long i , j ;
	float ex , ex2 , mean, std;

	float t1,t2;
	float t1D,t2D;
	float sumCPUTime = 0;
	float sumDiskTime = 0;
	float averagePercentagePruning = 0;
	long long pointCheckedTotal = 0;
	long long avTotResult = 0;
    pthread_t threadid[calculate_thread_number];
    float *rawfile;
	qp = fopen(queryFile,"rb");
	if( qp == NULL )
	{ 
		error(3);
		exit(1); // maybe not needed alredy present in error(int arg)
	}

	char * line=NULL;
	size_t len=0;
    int read;
    int lineCounter = 0;
    int q_loaded=0;
    long long totalDataPatterns;
    fseek(qp, 0L, SEEK_END);
    unsigned long long sz = (unsigned long long) ftell(qp);
    unsigned long long total_records = sz/full_time_series_size/4;
    fseek(qp, 0L, SEEK_SET);
    
    fp = fopen(datasetFile,"r");
    rawfile=(float*) malloc(sizeof(float) * full_time_series_size * ts_num);
    fread(rawfile, full_time_series_size*sizeof(float), ts_num, fp);
    //while((read = getline(&line, &len, qp))!=-1) 
    while (q_loaded < query_number)
    {
        q_loaded++;
        totalDataPatterns = 0;
        pointCheckedTotal = 0;
        dataScanned = 0; 
        lineCounter++;
        
        float * ts = (float*)malloc(sizeof(float) * full_time_series_size);
        if( fp == NULL )
        {
            error(2);
            exit(1); // maybe not needed alredy present in error(int arg)
        }

        float * firsKResults;


            bsf = INF;
    
        
        
        i = 0;
        j = 0;
        ex = ex2 = 0;

        // file should be binary and it
        // has to start with number of series contained
        // and for every series the length has to be at the first place.
        //t1 = clock();
        COUNT_TOTAL_TIME_START
        /// Array for keeping the query data
        Q = (float *)malloc(sizeof(float)*m);

        fread(ts, sizeof(float),full_time_series_size,qp);

        if( Q == NULL )
            error(1);

        for (i = 0; i < full_time_series_size; i++)
        {
            Q[i]=(float)ts[i];
            //ex=ex+Q[i];
            //ex2 += Q[i]*Q[i];
        }
        /// Read the query data from input file and calculate its statistic such as mean, std
        // the query series has the points (real numbers) separated with a space
        //char *olds = line;
	   //char olddelim = ' ';
	    //while(olddelim && *line) {
		//while(*line && (' ' != *line)) line++;
		//*line ^= olddelim = *line; 
		//cb(olds);
                //d = atof(olds);
                //ex += d;
                //ex2 += d*d;
                //Q[i] = d;
                //i++;
                
		//*line++ ^= olddelim; 
		//olds = line;
	    //}
        line = NULL;
        
        mean = ex/m;
        std = ex2/m;
        std = sqrt(std-mean*mean);
 

        /// Do z_normalization on query data
        //for( i = 0 ; i < m ; i++ )
             //Q[i] = (Q[i] - mean)/std;

        /// Sort the query data
        order = (int *)malloc(sizeof(int)*m);
        if( order == NULL )
            error(1);
        Index *Q_tmp = (Index *)malloc(sizeof(Index)*m);
        if( Q_tmp == NULL )
            error(1);
        for( i = 0 ; i < m ; i++ )
        {
            Q_tmp[i].value = Q[i];
            Q_tmp[i].index = i;
        }
        qsort(Q_tmp, m, sizeof(Index),comp);
        for( i=0; i<m; i++)
        {   Q[i] = Q_tmp[i].value;
            order[i] = Q_tmp[i].index;
        }
        free(Q_tmp);

        /// Array for keeping the current data; Twice the size for removing modulo (circulation) in distance calculation
        T = (float *)malloc(sizeof(float)*2*m);
        if( T == NULL )
            error(1);

        float dist = 0;
        i = 0;
        j = 0;
        ex = ex2 = 0;
        t2D = 0;
        int in;

        //for(in=0;in<ts_num;in++)

        


        
            transfordata *transferdatamlt=(transfordata*)malloc(sizeof(transfordata)*(calculate_thread_number));
            pthread_mutex_t lock_bsf=PTHREAD_MUTEX_INITIALIZER;
            for (i = 0; i < (calculate_thread_number); i++)
            {

            transferdatamlt[i].lock_bsf=&lock_bsf;
            transferdatamlt[i].start_number=i*(ts_num/calculate_thread_number);
            transferdatamlt[i].stop_number=(i+1)*(ts_num/calculate_thread_number);
            transferdatamlt[i].bsfdistance= &bsf;
            transferdatamlt[i].totalDataPatterns=0;
            transferdatamlt[i].Q=Q;
            transferdatamlt[i].order=order;
            transferdatamlt[i].rawfile=rawfile;
            transferdatamlt[i].SIMD_type=SIMD_type;
            transferdatamlt[i].query=ts;
            }
            transferdatamlt[calculate_thread_number-1].lock_bsf=&lock_bsf;
            transferdatamlt[calculate_thread_number-1].start_number=(calculate_thread_number-1)*(ts_num/calculate_thread_number);
            transferdatamlt[calculate_thread_number-1].stop_number=ts_num;
            transferdatamlt[calculate_thread_number-1].bsfdistance=&bsf;
            transferdatamlt[calculate_thread_number-1].totalDataPatterns=0;
            transferdatamlt[calculate_thread_number-1].Q=Q;
            transferdatamlt[calculate_thread_number-1].order=order;
            transferdatamlt[calculate_thread_number-1].rawfile=rawfile;
            transferdatamlt[calculate_thread_number-1].SIMD_type=SIMD_type;
            transferdatamlt[calculate_thread_number-1].query=ts;
            void* templetransferdatamlt;


            for(i=0; i<calculate_thread_number; i++) 
            {

                pthread_create(&(threadid[i]),NULL,query_time_series,(void*)(&transferdatamlt[i]));
            }

            for (i = 0; i < calculate_thread_number; i++)
            {
                pthread_join(threadid[i],NULL);
            }
            free(transferdatamlt);
                     
        




        //t2 = clock();
        COUNT_TOTAL_TIME_END
        float totalExecutionTime = total_time/1000000;
        float totalDiskTime = t2D/CLOCKS_PER_SEC;
        printf(" Distance: %f \n\n",bsf);  
        //cout << "Result query # "<< lineCounter <<" filename: "<<queryFile<<endl;
        //fprintf(queryAnswerFile,"Result query # %i filename: %s\n",lineCounter,queryFile);

        
        
       
        
        
        //cout << "Data Scanned: " << dataScanned << endl;
        //cout << "Total Execution Time : " << totalExecutionTime<< " sec" << endl;
        //cout << "Total Disk Time : " << totalDiskTime << " sec" << endl;
        //cout << "Total CPU Time : " << totalExecutionTime-totalDiskTime << " sec" <<endl;
        //cout << "Pruning power (abandoning power) : " << pp << "%" << endl<<endl;
        
        //fprintf(queryAnswerFile,"Data Scanned: %lld \n", dataScanned);
        //fprintf(queryAnswerFile,"Total Execution Time : %f sec \n", totalExecutionTime);
        //fprintf(queryAnswerFile,"Total Disk Time: %f sec \n", totalDiskTime);
        //fprintf(queryAnswerFile,"Total CPU Time: %f sec \n", totalExecutionTime-totalDiskTime);
        //fprintf(queryAnswerFile,"Pruning power (abandoning power) : %f perc \n",pp );
        query_result * qr;


        
        
        long long cont =0;
        
        float prevDist = 0.0;
        
        

        
        

        //cout <<"##################"<< endl<<endl;
        //fprintf(queryAnswerFile,"##################\n");
        
        free(Q);    
        free(T);
        free(ts);
        free(order);
    }
    fclose(fp);
    if (lineCounter>0)
    {
        //cout << "Average result of queries in the file: "<<queryFile<<" on the dataset: "<<datasetFile<<endl;
        //cout << "Disk time: "<<sumDiskTime/lineCounter<<endl;
        //cout << "CPU time: "<<sumCPUTime/lineCounter<<endl;
        cout << "Total time: \n"<<(total_time/1000000)+(sumCPUTime/lineCounter)<<endl;
        //cout << "Average Pruning power (abandoning power) : " << (averagePercentagePruning/(float)lineCounter) <<"perc" << endl<<endl;
        
   // fprintf(completeStatFile,"Average result of queries in the file: %s on the dataset: %s\n",queryFile,datasetFile); 
    //fprintf(completeStatFile,"Disk time %f \n",sumDiskTime/lineCounter); 
    //fprintf(completeStatFile,"CPU time %f \n",sumCPUTime/lineCounter); 
    //fprintf(completeStatFile,"Total time: %f \n",(sumDiskTime/lineCounter)+(sumCPUTime/lineCounter)); 
    //fprintf(completeStatFile,"Average Pruning power (abandoning power) : %f perc\n",(averagePercentagePruning/(float)lineCounter)); 
    

      //  cout<<"##########################################################################"<<endl;
       // fprintf(completeStatFile,"##########################################################################\n\n"); 
    
    }
    
    fclose(qp);
    free(rawfile);

	
}

int simpleScan( char const * datasetFile, int ts_num, int full_time_series_size)
{

	FILE *fp;      // the input file pointer
	float t1D,t2D;
    t2D = 0;
   	int in;
	t1D = clock();
	fp = fopen(datasetFile,"r");
	
	for(in=0;in<ts_num;in++)
	{
		float * app = (float *) malloc(sizeof(float)*full_time_series_size);
			fread(app, (sizeof(float)*full_time_series_size), 1, fp);
		free(app);
	}
	t2D = t2D + (clock() - t1D);
	float totalDiskTime = t2D/CLOCKS_PER_SEC;
	cout << "Total Scan Time : " << totalDiskTime << " sec" << endl;
	fclose(fp);
      
}

void * query_time_series(void *transferdatamlt)
{

    unsigned long in=0;
    long long i,j,loc;
    unsigned long start_number=((transfordata*)transferdatamlt)->start_number;
    unsigned long stop_number=((transfordata*)transferdatamlt)->stop_number;
    float * rawfile=((transfordata*)transferdatamlt)->rawfile;
    FILE *fp = fopen(datafilegl, "rb");
    fseek(fp, 0, SEEK_SET);
    int m=mgl;
    int full_time_series_size=full_time_series_sizegl;
    float bsf=*(((transfordata*)transferdatamlt)->bsfdistance);
    float d,dist;
    float *T=(float *)malloc(sizeof(float)*2*m);
    float *Q=((transfordata*)transferdatamlt)->Q;
    int * order=((transfordata*)transferdatamlt)->order;

    
    for(in=start_number;in<stop_number;in++)
        {
            int jn;
            i = 0;
            j = 0;
            float * app = &rawfile[in*full_time_series_size];

            if(((transfordata*)transferdatamlt)->SIMD_type)
            {
                dist= ts_euclidean_distance_SIMD(app, ((transfordata*)transferdatamlt)->query, full_time_series_size, bsf);
                _Bool bCHeck;
                
                        bCHeck = dist < bsf;
                    
                    
                    if(bCHeck)
                    {   
                        bsf=dist;
                        pthread_mutex_lock(((transfordata*)transferdatamlt)->lock_bsf);
                        if (bsf<*(((transfordata*)transferdatamlt)->bsfdistance))
                        {   
                            *(((transfordata*)transferdatamlt)->bsfdistance)=bsf;
                        }
                        else
                        {
                            bsf=*(((transfordata*)transferdatamlt)->bsfdistance);
                        }
                        pthread_mutex_unlock(((transfordata*)transferdatamlt)->lock_bsf);
  
                    }

            }
            else
            {
                for(jn=0;jn<full_time_series_size;jn++)
                {
                //dataScanned = dataScanned + 1;
               
                d = (float) app[jn];

                T[i%m] = d;
                T[(i%m)+m] = d;

                /// If there is enough data in T, the ED distance can be calculated
                if( i >= m-1 )
                {
                    /// the current starting location of T
                    j = (i+1)%m;

                    /// Z_norm(T[i]) will be calculated on the fly

                    /// Calculate ED distance
                    int pointChecked;
                      ((transfordata*)transferdatamlt)->totalDataPatterns = ((transfordata*)transferdatamlt)->totalDataPatterns + m;
                    dist = distancenormal(Q,T,j,m,order,bsf,pointChecked);
                    ((transfordata*)transferdatamlt)->pointCheckedTotal = ((transfordata*)transferdatamlt)->pointCheckedTotal + (long long) pointChecked;
                    _Bool bCHeck;
                    

                        bCHeck = dist < bsf;
                    
                    
                    if(bCHeck)
                    {   
                        bsf=dist;
                        pthread_mutex_lock(((transfordata*)transferdatamlt)->lock_bsf);
                        if (bsf<*(((transfordata*)transferdatamlt)->bsfdistance))
                        {   
                            *(((transfordata*)transferdatamlt)->bsfdistance)=bsf;
                        }
                        else
                        {
                            bsf=*(((transfordata*)transferdatamlt)->bsfdistance);
                        }
                        pthread_mutex_unlock(((transfordata*)transferdatamlt)->lock_bsf);
  
                    }

                }
                i++;
                }
            }
        }
        printf("BSFis %f length is %d\n",bsf,full_time_series_size);
        free(T);
}


float ts_euclidean_distance_SIMD(float * t, float * s, int size, float bound) {
float distance = 0;
    int i =0;
float distancef[8];

__m256 v_t,v_s,v_d,distancev;
    while (size > 0 && distance <bound) {
        v_t=_mm256_loadu_ps (&t[i]);
        v_s=_mm256_loadu_ps (&s[i]);
        
        v_d= _mm256_sub_ps (v_t,v_s);

        v_d=_mm256_mul_ps (v_d,v_d);
        size-=8;

        i=i+8;
        distancev = _mm256_hadd_ps (v_d, v_d);
        distancev = _mm256_hadd_ps (distancev, distancev);
        _mm256_storeu_ps (distancef ,distancev);
        distance +=distancef[0]+distancef[4];

    }

//    distance = sqrtf(distance);
    
    return distance;
}


float ts_euclidean_distance(float * t, float * s, int size, float bound) {
float distance = 0;
    while (size > 0 && distance <bound) {
        size--;
        distance += (t[size] - s[size]) * (t[size] - s[size]);

    }

//    distance = sqrtf(distance);
    
    return distance;
}