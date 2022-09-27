#ifndef SINGPROGRAM_H
#define SINGPROGRAM_H

#include <iostream>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <math.h>
#include <getopt.h>
#include <time.h>
#include <float.h>
#include <sched.h>
#include <stdbool.h>
#include <omp.h>

#include "../config.h"
#include "../globals.h"
#include "ParGIS.h"

extern "C" {
#include "ads.h"
#include "ads/sax/sax.h"
#include "ads/sax/sax_breakpoints.h"
#include "ads/sax/ts.h"
#include "ads/isax_visualize_index.h"
#include "ads/isax_file_loaders.h"
#include "ads/isax_visualize_index.h"
#include "ads/isax_first_buffer_layer.h"
#include "ads/isax_query_engine.h"
#include "ads/inmemory_query_engine.h"
#include "ads/parallel_inmemory_query_engine.h"
#include "ads/inmemory_index_engine.h"
#include "ads/parallel_query_engine.h"
#include "ads/parallel_index_engine.h"
#include "ads/inmemory_topk_engine.h"
}

isax_index *idx;
void INThandler(int);
void isax_query_binary_file_PplusG(const char *ifilename, int q_num, isax_index *index,float minimum_distance, int min_checked_leaves,query_result (*search_function)(ts_type*, ts_type*, isax_index*, float, int,float*,float*,sax_type*,bool*,bool*,float*));
void isax_query_binary_file_PplusGtable(const char *ifilename, int q_num, isax_index *index,float minimum_distance, int min_checked_leaves,query_result (*search_function)(ts_type*, ts_type*, isax_index*, float, int,float*,float*,sax_type*,bool*,bool*,float*));
void isax_query_binary_file_MplusG(const char *ifilename, int q_num, isax_index *index,float minimum_distance, int min_checked_leaves, query_result (*search_function)(ts_type*, ts_type*, isax_index*, float, int,float*,float*,sax_type*,float*,float*,float*,node_list*));
void isax_query_binary_file_SING(const char *ifilename, int q_num, isax_index *index,float minimum_distance, int min_checked_leaves,query_result (*search_function)(ts_type*, ts_type*, isax_index*, float, int,float*,float*,sax_type*,float*,float*,float*,node_list*,unsigned long int*));
void isax_knn_binary_file_SING(const char *ifilename, int q_num,int k, isax_index *index,float minimum_distance, int min_checked_leaves,pqueue_bsf (*search_function)(ts_type*, ts_type*, isax_index*, float, int,float*,float*,sax_type*,float*,float*,float*,node_list*,unsigned long int*,int));
void pass_tree_node_m(isax_node *node,isax_index *index,pthread_mutex_t *lock_queue,unsigned long int *currentposition,sax_type *saxarray,sax_type *sortsaxarray, float *lbdarray);


query_result exact_search_serial_ParGIS(ts_type *ts, ts_type *paa, isax_index *index, float minimum_distance, int min_checked_leaves,float *qts,float *gqts,sax_type *gsaxarray,bool *positionmap,bool *gpositionmap,float *gdictionary);
query_result exact_search_serial_ParGIStable(ts_type *ts, ts_type *paa, isax_index *index, float minimum_distance, int min_checked_leaves,float *qts,float *gqts,sax_type *gsaxarray,bool *positionmap,bool *gpositionmap,float *gdictionary);

query_result exact_search_serial_PplusG(ts_type *ts, ts_type *paa, isax_index *index, float minimum_distance, int min_checked_leaves,float *qts,float *gqts,sax_type *gsaxarray,bool *positionmap,bool *gpositionmap,float *gdictionary);
query_result exact_search_MplusG (ts_type *ts, ts_type *paa, isax_index *index,float minimum_distance, int min_checked_leaves,float *qts,float *gqts,sax_type *gsaxarray,float *positionmap,float *gpositionmap,float *gdictionary,node_list *nodelist );
query_result exact_search_SING_sort (ts_type *ts, ts_type *paa, isax_index *index, float minimum_distance, int min_checked_leaves,float *qts,float *gqts,sax_type *gsaxarray,float *positionmap,float *gpositionmap,float *gdictionary,node_list *nodelist,unsigned long int* offsetarray) ;
query_result exact_search_SING (ts_type *ts, ts_type *paa, isax_index *index,float minimum_distance, int min_checked_leaves,float *qts,float *gqts,sax_type *gsaxarray,float *positionmap,float *gpositionmap,float *gdictionary,node_list *nodelist,unsigned long int* offsetarray );
pqueue_bsf exact_knn_SING (ts_type *ts, ts_type *paa, isax_index *index,float minimum_distance, int min_checked_leaves,float *qts,float *gqts,sax_type *gsaxarray,float *positionmap,float *gpositionmap,float *gdictionary,node_list *nodelist,unsigned long int* offsetarray,int k );

void* exact_search_SING_sort_worker(void *rfdata);
void* exact_search_MplusG_worker(void *rfdata);
void* exact_search_SING_worker(void *rfdata);
void* exact_knn_SING_worker(void *rfdata);
void* gapworker( void *gapworkerdata);
void* multigapworker( void *gapworkerdata);

float calculate_node_distance_MplusG (isax_index *index, isax_node *node, ts_type *query,ts_type *paa,float* lbdmap, float bsf) ;
float calculate_node_distance_SING (isax_index *index, isax_node *node, ts_type *query,ts_type *paa, float bsf);
void calculate_node_topk_SING (isax_index *index, isax_node *node, ts_type *query,ts_type *paa, pqueue_bsf *pq_bsf, pthread_rwlock_t *lock_queue );



void  approximate_topk_SING (ts_type *ts, ts_type *paa, isax_index *index,pqueue_bsf *pq_bsf);
float nodedistance(float *paa, sax_type *sax, 
                           sax_type *sax_cardinalities,
                           sax_type max_bit_cardinality,
                           int max_cardinality,
                           int number_of_segments,
                           int min_val,
                           int max_val,
                           float ratio_sqrt);
float minidist_paa_to_isax_Breakpoly(float *paa, sax_type *sax, 
                           sax_type *sax_cardinalities,
                           sax_type max_bit_cardinality,
                           int max_cardinality,
                           int number_of_segments,
                           int min_val,
                           int max_val,
                           float ratio_sqrt);
void insert_tree_node_m_hybridpqueueBreakpoly(float *paa,isax_node *node,isax_index *index,float bsf,pqueue_t **pq,pthread_mutex_t *lock_queue,int *tnumber);
void insert_tree_node_m_hybridpqueueBreakpolyroot(float *paa,isax_node *node,isax_index *index,float bsf,pqueue_t **pq,pthread_mutex_t *lock_queue,int *tnumber);

typedef struct GPUtransferdata
{
    sax_type* gsaxarray;
    bool* positionmap,*gpositionmap;
    float* paa, * gqts;
    float *bsf_distance;
    long int datasize;
	long int offsetnumber;
    int loopnumber,startloop,stoploop;
    pthread_barrier_t *lock_barrier1;
    int segmentnumber;
	float segmentsize;
}GPUtransferdata;
void* PplusGworker(void *GPUdtransferdata);

typedef struct SING_workerdata
{
	isax_node *current_root_node;
	ts_type *paa,*paaU,*paaL,*ts,*uo,*lo;
	pqueue_t *pq;
	isax_index *index;
	float minimum_distance;
	int limit;
	pthread_mutex_t *lock_current_root_node;
	pthread_mutex_t *lock_queue;
	pthread_barrier_t *lock_barrier;
	pthread_rwlock_t *lock_bsf;
	query_result *bsf_result;
	int *node_counter;
	isax_node **nodelist;
	int amountnode;
	localStack *localstk; 
	localStack *allstk;
	pthread_mutex_t *locallock,*alllock;
	int *queuelabel,*allqueuelabel;
	pqueue_t **allpq;
	int startqueuenumber;
	int warpWind;
	pqueue_bsf *pq_bsf;
    float *lbdmap;
    bool *labelvalue;
    bool *activenode;
    int *offsetvalue;
    unsigned long int *gpuoffset;
}SING_workerdata;


typedef struct gap_workerdata
{
    isax_node **nodelist;
	int amountnode,workerstartnode,workerstopnode;
    int *startnode, *stopnode, *gapstartnode,*gapstopnode;
    int *nodecounter,*nodecounter2;
    bool *activechunk;
    bool *activenode;
    isax_index *index;
    int chunknumber;
    float bsf;
	ts_type *paa,*paaU,*paaL,*ts,*uo,*lo;
    pthread_mutex_t *lockposition;
    unsigned long *offsetarray;
};














/*
query_result exact_search_SING_sort_pruned (ts_type *ts, ts_type *paa, isax_index *index, float minimum_distance, int min_checked_leaves,float *qts,float *gqts,sax_type *gsaxarray,float *positionmap,float *gpositionmap,float *gdictionary,node_list *nodelist,unsigned long int* offsetarray);

void isax_query_binary_file_gpugrid(const char *ifilename, int q_num, isax_index *index,
                            float minimum_distance, int min_checked_leaves,
                            query_result (*search_function)(ts_type*, ts_type*, isax_index*, float, int,float*,float*,sax_type**,bool**,bool**,float*,unsigned long* ));
void isax_query_binary_file_gpufloatgray(const char *ifilename, int q_num, isax_index *index,
                            float minimum_distance, int min_checked_leaves,
                            query_result (*search_function)(ts_type*, ts_type*, isax_index*, float, int,float*,float*,sax_type*,float*,float*,float*,node_list*,unsigned long int*));
void isax_query_binary_file_gpu2(const char *ifilename,const char *ifilename2, int q_num, isax_index *index,
                            float minimum_distance, int min_checked_leaves,
                            query_result (*search_function)(ts_type*, ts_type*, isax_index*, float, int,float*,float*,sax_type*,bool*,bool*,float*,unsigned long long*,unsigned long int *));
query_result exact_search_SING_sort_new_3 (ts_type *ts, ts_type *paa, isax_index *index,
                           float minimum_distance, int min_checked_leaves,float *qts,float *gqts,sax_type *gsaxarray,float *positionmap,float *gpositionmap,float *gdictionary,node_list *nodelist,unsigned long int* offsetarray );
query_result exact_search_SING_sort_new_4 (ts_type *ts, ts_type *paa, isax_index *index,
                           float minimum_distance, int min_checked_leaves,float *qts,float *gqts,sax_type *gsaxarray,float *positionmap,float *gpositionmap,float *gdictionary,node_list *nodelist,unsigned long int* offsetarray );
pqueue_bsf exact_knn_SING_sort_new_4 (ts_type *ts, ts_type *paa, isax_index *index,
                           float minimum_distance, int min_checked_leaves,float *qts,float *gqts,sax_type *gsaxarray,float *positionmap,float *gpositionmap,float *gdictionary,node_list *nodelist,unsigned long int* offsetarray,int k );
void* exact_search_worker_inmemory_SING_sort_new_2(void *rfdata);
void* exact_search_worker_inmemory_SING_sort_new_3(void *rfdata);
void* exact_search_worker_inmemory_SING_sort_new_4(void *rfdata);
void* exact_search_worker_inmemory_SING_sort_new_5(void *rfdata);
void* exact_search_worker_inmemory_SING_sort_new_6(void *rfdata);
void* exact_search_worker_inmemory_SING_sort_new_7(void *rfdata);
query_result exact_search_serial_ParGIS_openmp_inmemoryhybrid2(ts_type *ts, ts_type *paa, isax_index *index, float minimum_distance, int min_checked_leaves,float *qts,float *gqts,sax_type *gsaxarray,bool *positionmap,bool *gpositionmap,float *gdictionary);
query_result exact_search_serial_ParGIGS2_openmp_inmemory(ts_type *ts, ts_type *paa, isax_index *index, float minimum_distance, int min_checked_leaves,float *qts,float *gqts,sax_type **gsaxarray,bool **positionmap,bool **gpositionmap,float *gdictionary,unsigned long *gridnumber);
query_result exact_search_serial_ParGIS_openmp_inmemoryhybrid3(ts_type *ts, ts_type *paa, isax_index *index, float minimum_distance, int min_checked_leaves,float *qts,float *gqts,sax_type *gsaxarray,bool *positionmap,bool *gpositionmap,float *gdictionary,unsigned long long *positionarray,unsigned long int *offsetarray);
query_result exact_search_serial_ParGIS_openmp_inmemoryfloat(ts_type *ts, ts_type *paa, isax_index *index, float minimum_distance, int min_checked_leaves,float *qts,float *gqts,sax_type *gsaxarray,float *positionmap,float *gpositionmap,float *gdictionary);
query_result exact_search_serial_ParGIGS_openmp_inmemory(ts_type *ts, ts_type *paa, isax_index *index, float minimum_distance, int min_checked_leaves,float *qts,float *gqts,sax_type **gsaxarray,bool **positionmap,bool **gpositionmap,float *gdictionary, unsigned long *gridnumber);
void* twogapworker( void *gapworkerdata);
query_result exact_search_ParISnew_inmemory_hybridgplus (ts_type *ts, ts_type *paa, isax_index *index,node_list *nodelist,float minimum_distance, int min_checked_leaves) ;
void isax_query_binary_file_traditionalgplus(const char *ifilename, int q_num, isax_index *index,float minimum_distance, int min_checked_leaves,query_result (*search_function)(ts_type*, ts_type*, isax_index*,node_list*, float, int)) ;
void* exact_search_worker_inmemory_hybridpqueuegplus(void *rfdata);
query_result  approximate_search_inmemory_pargis(ts_type *ts, ts_type *paa, unsigned long long *positionmap,isax_index *index);
float calculate_node_distance_inmemory_pargis (isax_index *index, isax_node *node, ts_type *query,unsigned long long *positionmap, float bsf) ;
*/
#endif 
