//
//  isax_query_engine.h
//  al_isax
//
//  Created by Kostas Zoumpatianos on 4/13/12.
//  Copyright 2012 University of Trento. All rights reserved.
//

#ifndef al_isax_isax_query_engine_h
#define al_isax_isax_query_engine_h
#include "../../config.h"
#include "../../globals.h"
#include "isax_index.h"
#include "isax_node.h"
#include "pqueue.h"

typedef struct query_result {
    float distance;
    isax_node *node;
    size_t pqueue_position;
} query_result;


static int
cmp_pri(double next, double curr)
{
	return (next > curr);
}


static double
get_pri(void *a)
{
	return (double) ((query_result *) a)->distance;
}


static void
set_pri(void *a, double pri)
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


/// VARIOUS QUERY TYPES
query_result exact_search_serial(ts_type *ts, ts_type *paa, isax_index *index, float minimum_distance, int min_checked_leaves);
pqueue_bsf exact_topk_serial(ts_type *ts, ts_type *paa, isax_index *index, float minimum_distance, int min_checked_leaves, int k);

query_result exact_search (ts_type *ts, ts_type *paa, isax_index *index, float minimum_distance, int min_checked_leaves);
pqueue_bsf exact_topk (ts_type *ts, ts_type *paa, isax_index *index, float minimum_distance, int min_checked_leaves, int k);
query_result sanity_check_query (ts_type *ts, ts_type *paa, isax_index *index, float minimum_distance, int min_checked_leaves);

/// HELPE FUNCTIONS
ts_type * get_ads_record(unsigned long tid, isax_index *index);
query_result approximate_search (ts_type *ts, ts_type *paa, isax_index *index);
query_result  approximate_search_manynode (ts_type *ts, ts_type *paa, isax_index *index);

void  approximate_topk (ts_type *ts, ts_type *paa, isax_index *index, pqueue_bsf *pq_bsf);
query_result approximate_search_SIMD (ts_type *ts, ts_type *paa, isax_index *index);
query_result refine_answer (ts_type *ts, ts_type *paa, isax_index *index,
							query_result approximate_bsf_result,
							float minimum_distance, int limit);
void refine_topk_answer (ts_type *ts, ts_type *paa, isax_index *index, 
              pqueue_bsf *pq_bsf, 
                            float minimum_distance, int limit);


#endif
