#ifndef isaxlib_dtw_h
#define isaxlib_dtw_h

#include <float.h>
#include "../../config.h"
#include "../../globals.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <pthread.h>
#include <stdbool.h>
#include <sys/wait.h>
#include "../../config.h"
#include "../../globals.h"
#include "ads/sax/sax.h"
#include "ads/sax/ts.h"
#include "ads/sax/sax_breakpoints.h"
#include "ads/isax_index.h"

#include "omp.h"  
#include "ads/isax_query_engine.h"
#include "ads/inmemory_index_engine.h"
#include "ads/inmemory_query_engine.h"
#include "ads/parallel_inmemory_query_engine.h"
#include "ads/parallel_index_engine.h"
#include "ads/isax_first_buffer_layer.h"
#include "ads/pqueue.h"
#include "ads/sax/sax.h"
#include "ads/isax_node_split.h"
#include "ads/inmemory_topk_engine.h"


void lower_upper_lemire(float *t, int len, int r, float *l, float *u);
float   minidist_paa_to_isax_raw_DTW(float *paaU, float *paaL , sax_type *sax, 
                           sax_type *sax_cardinalities,
                           sax_type max_bit_cardinality,
                           int max_cardinality,
                           int number_of_segments,
                           int min_val,
                           int max_val,
                           float ratio_sqrt);
                           float minidist_paa_to_isax_DTW(float *paaU, float *paaL , sax_type *sax, 
                           sax_type *sax_cardinalities,
                           sax_type max_bit_cardinality,
                           int max_cardinality,
                           int number_of_segments,
                           int min_val,
                           int max_val,
                           float ratio_sqrt);
                           float   minidist_paa_to_isax_raw_DTW_SIMD(float *paaU,float *paaL, sax_type *sax, 
                           sax_type *sax_cardinalities,
                           sax_type max_bit_cardinality,
                           int max_cardinality,
                           int number_of_segments,
                           int min_val,
                           int max_val,
                           float ratio_sqrt);
void isax_DTWquery_binary_file_traditional(const char *ifilename, int q_num, isax_index *index,
                            float minimum_distance, int min_checked_leaves,int warpWind);
                    //query_result (*search_function)(ts_type*, ts_type*, isax_index*,node_list*, float, int));
                    void isax_DTWknn_query_binary_file_traditional(const char *ifilename,const char *labelfilename, int q_num, isax_index *index,
                            float minimum_distance, int min_checked_leaves,int warpWind ,int k,long int classlength,
                            pqueue_bsf (*search_function)(ts_type*, ts_type*, ts_type*, ts_type*, isax_index*,node_list*, float,int, int, int)) ;
float dtw(float* A, float* B,float *cb,int m, int r,  float bsf);
float dtwsimd(float* A, float* B, float *cb, int m, int r, float bsf, float* tSum, float* pCost, float* rDist);
float dtwsimdPruned(float* A, float* B,float *cb, int m, int r, float bsf, float* tSum, float* pCost, float* rDist);
query_result exact_DTW_serial_ParIS_inmemory(ts_type *ts,ts_type *paa, ts_type *paaU, ts_type *paaL,isax_index *index, float minimum_distance, int min_checked_leaves,int warpWind);
pqueue_bsf exact_DTWknn_serial_ParIS_inmemory(ts_type *ts,ts_type *paa, ts_type *paaU, ts_type *paaL,isax_index *index, float minimum_distance, int min_checked_leaves,int warpWind,int k);

void* mindtwdistance_worker_inmemory(void *essdata);
void* dtwreadworker_inmemory(void *read_pointer);
void* dtwknnreadworker_inmemory(void *read_pointer);

query_result exact_DTW_serial_ParIS_openmp_inmemory(ts_type *ts, ts_type *paa,ts_type *paaU, ts_type *paaL,isax_index *index, float minimum_distance, int min_checked_leaves,int warpWind);
query_result exact_DTW_MESSI_inmemory_hybrid (ts_type *ts,ts_type *paa, ts_type *paaU, ts_type *paaL, isax_index *index,node_list *nodelist,float minimum_distance, int min_checked_leaves,int warpWind);
pqueue_bsf exact_DTWknn_MESSI_inmemory_hybrid (ts_type *ts,ts_type *paa, ts_type *paaU, ts_type *paaL, isax_index *index,node_list *nodelist,float minimum_distance, int min_checked_leaves,int warpWind, int k);
void calculate_node_DTWknn_inmemory (isax_index *index, isax_node *node, ts_type *query,int warpWind, pqueue_bsf *pq_bsf);

float calculate_node_DTW2_inmemory (isax_index *index, isax_node *node, ts_type *query,float *uo, float *lo,ts_type *paa,ts_type *paaU,ts_type *paaL, float bsf,int warpWind);
void calculate_node_DTW2knn_inmemory (isax_index *index, isax_node *node, ts_type *query,float *uo, float *lo,ts_type *paa,ts_type *paaU,ts_type *paaL, float bsf,int warpWind, pqueue_bsf *pq_bsf, pthread_rwlock_t *lock_queue);

float calculate_node_DTW_inmemory (isax_index *index, isax_node *node, ts_type *query, float bsf,int warpWind) ;
query_result  approximate_DTW_inmemory_pRecBuf (ts_type *ts, ts_type *paa, isax_index *index,int warpWind);
void  approximate_DTWtopk_inmemory (ts_type *ts, ts_type *paa, isax_index *index,int warpWind,pqueue_bsf *pq_bsf);

void* exact_DTW_worker_inmemory_hybridpqueue(void *rfdata);
void* exact_DTWknn_worker_inmemory_hybridpqueue(void *rfdata);

void insert_tree_node_m_hybridpqueue_DTW(float *paaU,float *paaL,isax_node *node,isax_index *index,float bsf,pqueue_t **pq,pthread_mutex_t *lock_queue,int *tnumber);
float lb_keogh_cumulative_norm(float *qs, float *uo, float *lo, float *cb,int len, float mean, float std, float best_so_far);
float lb_keogh_data_bound( float* qo,float* tu,  float* tl, float* cb, int len, float bsf);
void isax_DTWknn_query_binary_file(const char *ifilename,const char *labelfilename, int q_num, isax_index *index,
                            float minimum_distance, int min_checked_leaves,int warpWind,int k,long int classlength,
                            pqueue_bsf (*search_function)(ts_type*,ts_type*, ts_type*,ts_type*, isax_index*, float, int,int, int)) ;
typedef struct deque
{   int *dq;
    int size,capacity;
    int f,r;
} deque;
#endif