
#ifndef al_inmemory_topk_engine_h
#define al_inmemory_topk_engine_h
#include "../../config.h"
#include "../../globals.h"
#include "sax/ts.h"
#include "sax/sax.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "isax_index.h"
#include "isax_query_engine.h"
#include "isax_node.h"
#include "pqueue.h"
#include "isax_first_buffer_layer.h"
#include "inmemory_index_engine.h"
#include "ads/isax_node_split.h"
void approximate_topk_inmemory (ts_type *ts, ts_type *paa, isax_index *index,pqueue_bsf *pq_bsf);
void calculate_node2_topk_inmemory (isax_index *index, isax_node *node, ts_type *query,ts_type *paa, pqueue_bsf *pq_bsf, pthread_rwlock_t *lock_queue) ;
void calculate_node_topk_inmemory (isax_index *index, isax_node *node, ts_type *query, pqueue_bsf *pq_bsf);
void refine_topk_answer_inmemory (ts_type *ts, ts_type *paa, isax_index *index,pqueue_bsf *pq_bsf , float minimum_distance, int limit);
pqueue_bsf exact_search_serial_topk_inmemory(ts_type *ts, ts_type *paa, isax_index *index, float minimum_distance, int min_checked_leaves, int k);
pqueue_bsf exact_topk_MESSImq_inmemory (ts_type *ts, ts_type *paa, isax_index *index,node_list *nodelist,
                           float minimum_distance, int min_checked_leaves,int k);
void* exact_topk_worker_inmemory_hybridpqueue(void *rfdata);
#endif