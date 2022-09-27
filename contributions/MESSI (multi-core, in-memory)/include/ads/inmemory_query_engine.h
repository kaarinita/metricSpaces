
#ifndef al_inmemory_query_engine_h
#define al_inmemory_query_engine_h
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
#include "ads/isax_node_split.h"


float calculate_node_distance_inmemory (isax_index *index, isax_node *node, ts_type *query , float bsf);
float calculate_node_distance_inmemory_omp (isax_index *index, isax_node *node, ts_type *query, float bsf);
float calculate_node_distance2_inmemory (isax_index *index, isax_node *node, ts_type *query, ts_type *paa, float bsf);
query_result  approximate_search_inmemory (ts_type *ts, ts_type *paa, isax_index *index);
query_result  approximate_search_inmemory_messi (ts_type *ts, ts_type *paa, isax_index *index) ;
query_result  approximate_search_inmemory_pRecBuf (ts_type *ts, ts_type *paa, isax_index *index);
query_result exact_search_serial_inmemory(ts_type *ts, ts_type *paa, isax_index *index, float minimum_distance, int min_checked_leaves);
query_result exact_search_serial_1bsf_inmemory(ts_type *ts, ts_type *paa, isax_index *index, float minimum_distance, int min_checked_leaves,float bsf);
query_result refine_answer_inmemory (ts_type *ts, ts_type *paa, isax_index *index, query_result approximate_bsf_result, float minimum_distance, int limit);
float calculate_minimum_distance_inmemory (isax_index *index, isax_node *node, ts_type *raw_query, ts_type *query);

struct args_in 
{
    unsigned int i;
    unsigned long from;
    unsigned long to;
    float bsf;
    ts_type *paa;
    isax_index *index;
};

void *compute_mindists_in(void *ptr);
query_result exact_search_inmemory (ts_type *ts, ts_type *paa, isax_index *index,
                           float minimum_distance, int min_checked_leaves);
query_result exact_search_inmemory2 (ts_type *ts, ts_type *paa, isax_index *index,
                           float minimum_distance, int min_checked_leaves);
void insert_tree_node(float *paa,isax_node *node,isax_index *index,float bsf,pqueue_t *pq);
#endif
