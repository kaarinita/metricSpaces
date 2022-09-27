#ifndef parallel_parallel_query_engine_h
#define parallel_parallel_query_engine_h
#include "../../config.h"
#include "../../globals.h"
#include "isax_index.h"
#include "isax_node.h"
#include "pqueue.h"
#include "inmemory_index_engine.h"
void isax_query_binary_file_para(const char *ifilename, int q_num,
							isax_index *index, float minimum_distance,
						    int min_checked_leaves,
                            query_result (*search_function)(ts_type*, ts_type*, isax_index*, float, int));
query_result exact_search_serial_para(ts_type *ts, ts_type *paa, isax_index *index, float minimum_distance, int min_checked_leaves, pthread_mutex_t *lock_index);    
query_result exact_search_serial_ParIS(ts_type *ts, ts_type *paa, isax_index *index, float minimum_distance, int min_checked_leaves);
query_result exact_search_serial_ParISnonsort(ts_type *ts, ts_type *paa, isax_index *index, float minimum_distance, int min_checked_leaves);

query_result exact_search_serial_ParIS_nb(ts_type *ts, ts_type *paa, isax_index *index, float minimum_distance, int min_checked_leaves);
pqueue_bsf exact_topk_serial_ParIS(ts_type *ts, ts_type *paa, isax_index *index, float minimum_distance, int min_checked_leaves, int k);
//query_result exact_search_serial_new(ts_type *ts, ts_type *paa, isax_index *index, float minimum_distance, int min_checked_leaves);
query_result exact_search_m (ts_type *ts, ts_type *paa, isax_index *index, float minimum_distance, int min_checked_leaves);
query_result refine_answer_m(ts_type *ts, ts_type *paa, isax_index *index,
                            query_result *approximate_bsf_result,
                            float minimum_distance, int limit);
void* exact_search_fonction(void *rfdata);
void* refind_answer_fonction(void *rfdata);
void* mindistance_worker(void *essdata);
void* mindistanceinsert_worker(void *essdata);
//void* mindistance_worker_paradsplus2(void *essdata);
void* ParIS_nb_worker(void *essdata);
void* read_worker(void *read_pointer);
void* topk_read_worker(void *read_pointer);
void* para_queries_worker(void *transvector);
//void* readworker_paradsplus2(void *read_pointer);
typedef struct refind_answer_fonction_data
{
	isax_node *current_root_node;
	ts_type *paa,*ts;
	pqueue_t *pq;
	isax_index *index;
	float minimum_distance;
	int limit;
	pthread_mutex_t *lock_current_root_node;
	pthread_mutex_t *lock_queue;
	pthread_barrier_t *lock_barrier;
	pthread_rwlock_t *lock_bsf;
	query_result *bsf_result;
	int node_counter;
	isax_node **nodelist;
	int amountnode;
	bool lockvalueq;
	bool lockvaluebsf;
}refind_answer_fonction_data;



typedef struct ParIS_read_worker_data
{	
	ts_type *ts,*tsU,*tsL;
	isax_index *index;
	int warpWind;
	float bsf;
	float *bsf2;
	float *minidisvector;
	unsigned long *load_point;
	int *ts_number;
	unsigned long start_number;
	unsigned long stop_number;
	unsigned long *counter;
	unsigned long sum_of_lab;
	pthread_rwlock_t *lock_bsf;
	float* rawfile;
	pqueue_bsf* pq_bsf;

}ParIS_read_worker_data;

typedef struct ParIS_LDCW_data
{
	unsigned long  start_number;
	unsigned long  stop_number;
	ts_type *paa,*paaU,*paaL;
	ts_type *ts;
	isax_index *index;
	float bsfdistance;
	pthread_rwlock_t *lock_bsf;
	unsigned long *label_number,*currentpositioncounter;
	int* ts_number;
	float* minidisvector;
	int sum_of_lab;
	float* rawfile;
	unsigned long read_time_conter;
}ParIS_LDCW_data;
typedef struct paraquery
{
	ts_type *ts;
	ts_type *paa;
	isax_index *index;
	float minimum_distance;
	int min_checked_leaves;
	pthread_mutex_t *lock_index;

}paraquery;
int maxquerythread;
int maxreadthread;
int bf;
#endif