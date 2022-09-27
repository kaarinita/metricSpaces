
#ifndef al_parallel_inmemory_query_engine_h
#define al_parallel_inmemory_query_engine_h
#include "../../config.h"
#include "../../globals.h"
#include "sax/ts.h"
#include "sax/sax.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "isax_index.h"
#include "isax_query_engine.h"
#include "parallel_query_engine.h"
#include "isax_node.h"
#include "pqueue.h"
#include "isax_first_buffer_layer.h"
#include "ads/isax_node_split.h"
#include "inmemory_index_engine.h"

typedef struct localStack {
    isax_node **val; 
    int top;
    int bottom;
}localStack;

typedef struct MESSI_workerdata
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
}MESSI_workerdata;

float calculate_node_distance_inmemory_m (isax_index *index, isax_node *node, ts_type *query, float bsf);

query_result  approximate_search_inmemory_m (ts_type *ts, ts_type *paa, isax_index *index);
query_result refine_answer_inmemory_m (ts_type *ts, ts_type *paa, isax_index *index, query_result approximate_bsf_result,float minimum_distance, int limit);

query_result exact_search_serial_ParIS_nb_inmemory(ts_type *ts, ts_type *paa, isax_index *index, float minimum_distance, int min_checked_leaves);
void* ParIS_nb_worker_inmemory(void *worker_data);
query_result exact_search_parads_inmemory (ts_type *ts, ts_type *paa, isax_index *index,
                           float minimum_distance, int min_checked_leaves);
void* exact_search_old_worker_inmemory(void *rfdata);
query_result exact_search_serial_ParIS_inmemory(ts_type *ts, ts_type *paa, isax_index *index, float minimum_distance, int min_checked_leaves);
pqueue_bsf exact_topk_serial_ParIS_inmemory(ts_type *ts, ts_type *paa, isax_index *index, float minimum_distance, int min_checked_leaves,int k);

query_result exact_search_serial_ParIS2_inmemory(ts_type *ts, ts_type *paa, isax_index *index, float minimum_distance, int min_checked_leaves);
query_result exact_search_serial_ParIS_openmp_inmemory(ts_type *ts, ts_type *paa, isax_index *index, float minimum_distance, int min_checked_leaves);
query_result exact_search_serial_ParGISG_openmp_inmemory(ts_type *ts, ts_type *paa, isax_index *index, float minimum_distance, int min_checked_leaves);
query_result exact_search_serial_ParGIS_openmp_inmemory(ts_type *ts, ts_type *paa, isax_index *index, float minimum_distance, int min_checked_leaves);
void* mindistance_worker_inmemory(void *essdata);
void* readworker_inmemory(void *read_pointer);
void* readworker2_inmemory(void *read_pointer);
void* topk_readworker_inmemory(void *read_pointer);

void exact_search_serial_ParIS_nb_batch_inmemory(ts_type *ts, ts_type *paa, isax_index *index, float minimum_distance, int min_checked_leaves,int batch_number);
query_result exact_search_inmemory_openmp (ts_type *ts, ts_type *paa, isax_index *index,
                           float minimum_distance, int min_checked_leaves);
query_result exact_search_ParISnew_inmemory (ts_type *ts, ts_type *paa, isax_index *index,node_list *nodelist,
                           float minimum_distance, int min_checked_leaves);
query_result exact_search_ParISnew_inmemory_workstealing (ts_type *ts, ts_type *paa, isax_index *index,node_list *nodelist,
                           float minimum_distance, int min_checked_leaves);
query_result exact_search_ParISnew_inmemory_hybrid (ts_type *ts, ts_type *paa, isax_index *index,node_list *nodelist,
                           float minimum_distance, int min_checked_leaves);

query_result exact_search_ParISnew_inmemory_hybrid_workstealing (ts_type *ts, ts_type *paa, isax_index *index,node_list *nodelist,
                           float minimum_distance, int min_checked_leaves);
void* exact_search_worker_inmemory(void *rfdata);
void* exact_search_worker_inmemory_workstealing(void *rfdata);
void* exact_search_worker_inmemory_hybridpqueue(void *rfdata);
void* exact_search_worker_inmemory_hybridpqueue_workstealing(void *rfdata);
void insert_tree_node_m(float *paa,isax_node *node,isax_index *index,float bsf,pqueue_t *pq,pthread_mutex_t *lock_queue);
void insert_tree_node_mgpu(float *paa,isax_node *node,isax_index *index,float bsf,pqueue_bsf *pq,pthread_mutex_t *lock_queue);
void insert_tree_node_m_workstealing(float *paa,isax_node *node,isax_index *index,float bsf,pqueue_t *pq,pthread_mutex_t *lock_queue,localStack* workstack);
void insert_tree_node_m_hybridpqueue(float *paa,isax_node *node,isax_index *index,float bsf,pqueue_t **pq,pthread_mutex_t *lock_queue,int *tnumber);
void insert_tree_node_m_hybridpqueue_workstealing(float *paa,isax_node *node,isax_index *index,float bsf,pqueue_t **pq,pthread_mutex_t *lock_queue,localStack* workstack,int *tnumber);
void pushbottom(localStack *stk, isax_node *node);
isax_node* poptop(localStack *stk);
isax_node* popbottom(localStack *stk);
bool isemptyqueue(localStack *stk);
isax_node* poptop2(localStack *stk);
isax_node* popbottom2(localStack *stk);
int  N_PQUEUE;

#endif
