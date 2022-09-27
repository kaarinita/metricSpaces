
#ifndef al_inmemory_index_engine_h
#define al_inmemory_index_engine_h
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

void index_generate_inmemory(const char *ifilename, long int ts_num, isax_index *index);
void index_generate_inmemory_m(const char *ifilename, long int ts_num, isax_index *index);
void index_creation_m(const char *ifilename, long int ts_num, isax_index *index);
void index_creation_m_new(const char *ifilename, long int ts_num, isax_index *index);
void index_creation_m_gpu(const char *ifilename, long int ts_num, isax_index *index);
void index_creation_m2(const char *ifilename, long int ts_num, isax_index *index);
void index_creation_gpu(const char *ifilename, long int ts_num, isax_index *index);
void index_creation_mix(const char *ifilename, long int ts_num, isax_index *index);
void index_creation_pRecBuf(const char *ifilename, long int ts_num, isax_index *index);
void index_creation_pRecBuf_new(const char *ifilename, long int ts_num, isax_index *index);
void index_generate_inmemory_pRecBuf(const char *ifilename, long int ts_num, isax_index *index);
void* indexbulkloadingworker_inmemory(void *transferdata);
void* index_creation_worker_inmemory(void *transferdata);
void* index_creation_worker_inmemory_new(void *transferdata);
void* index_creation_worker2_inmemory(void *transferdata);
void* index_creation_mix_worker_inmemory(void *transferdata);
void* index_creation_pRecBuf_worker(void *transferdata);
void* index_creation_pRecBuf_worker_new(void *transferdata);
void* indexbulkloadingworker_pRecBuf_inmemory(void *transferdata);
root_mask_type isax_fbl_index_insert_inmemory_para(isax_index *index, 
                                    sax_type * sax,
                                    file_position_type * pos, pthread_mutex_t *lock_fbl, pthread_mutex_t *lock_cbl,pthread_mutex_t *lock_firstnode,pthread_mutex_t *lock_index);
root_mask_type isax_pRecBuf_index_insert_inmemory(isax_index *index, 
                                    sax_type * sax,
                                    file_position_type * pos,pthread_mutex_t *lock_firstnode,int workernumber,int total_workernumber);
root_mask_type isax_fbl_index_insert_inmemory(isax_index *index, sax_type * sax, file_position_type * pos);
enum response flush_fbl_inmemory(first_buffer_layer *fbl, isax_index *index);
enum response flush_fbl_inmemory_m(first_buffer_layer *fbl, isax_index *index);
isax_node * add_record_to_node_inmemory(isax_index *index, 
                                 isax_node *tree_node, 
                                 isax_node_record *record,
                                 const char leaf_size_check);
enum response flush_pRecBuf_inmemory(parallel_first_buffer_layer *fbl, isax_index *index);
void* flush_fbl_inmemory_worker(void *input);
void* flush_pRecBuf_inmemory_worker(void *input);
enum response flush_subtree_leaf_buffers_inmemory (isax_index *index, isax_node *node);
isax_index * isax_index_init_inmemory(isax_index_settings *settings);

typedef struct buffer_data_inmemory
{
	isax_index *index;
	int start_number,stop_number;	
	ts_type * ts;
	pthread_mutex_t *lock_record;
	pthread_mutex_t *lock_fbl;
	pthread_mutex_t *lock_index;
	pthread_mutex_t *lock_cbl;
	pthread_mutex_t *lock_firstnode;
	pthread_mutex_t *lock_nodeconter;
	pthread_mutex_t *lock_disk;
	int workernumber;
	int total_workernumber;
	pthread_barrier_t *lock_barrier1, *lock_barrier2;
	int *node_counter;
	bool finished;
	int *nodeid;
	unsigned long *shared_start_number;
}buffer_data_inmemory;

typedef struct transferfblinmemory
{
	int start_number,stop_number,conternumber;
  	int preworkernumber;
  	isax_index *index;
  	int *nodeid;
}transferfblinmemory;

float * rawfile;

typedef struct node_list
{
	isax_node **nlist;
	int node_amount;
}node_list;
#endif
