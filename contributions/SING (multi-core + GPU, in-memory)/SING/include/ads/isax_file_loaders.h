//
//  isax_file_loaders.h
//  isax
//
//  Created by Kostas Zoumpatianos on 4/7/12.
//  Copyright 2012 University of Trento. All rights reserved.
//

#ifndef isax_isax_file_loaders_h
#define isax_isax_file_loaders_h
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
#include "inmemory_index_engine.h"

void isax_index_binary_file(const char *ifilename, int ts_num,
                            isax_index *index);
void isax_sorted_index_binary_file(const char *ifilename, int ts_num,
                            isax_index *index);
void isax_merge_sorted_index_binary_file(const char *ifilename, int ts_num,
                            isax_index *index);
void isax_query_binary_file(const char *ifilename, int q_num,
							isax_index *index, float minimum_distance,
						    int min_checked_leaves,
                            query_result (*search_function)(ts_type*, ts_type*, isax_index*, float, int));
void isax_query_binary_file_traditional(const char *ifilename, int q_num, isax_index *index,
                            float minimum_distance, int min_checked_leaves,
                            query_result (*search_function)(ts_type*, ts_type*, isax_index*,node_list*, float, int));
void isax_query_binary_file_batch(const char *ifilename, int q_num,
							isax_index *index, float minimum_distance,
						    int min_checked_leaves,
                            void (*search_function)(ts_type*, ts_type*, isax_index*, float, int,int));
void isax_query_binary_fixbsf_file(const char *ifilename, int q_num, isax_index *index,
                            float minimum_distance, int min_checked_leaves,
                            query_result (*search_function)(ts_type*, ts_type*, isax_index*, float, int,float));
void isax_index_baffuer_manager(const char *ifilename, int ts_num, isax_index *index);

void isax_topk_query_binary_file(const char *ifilename, int q_num, isax_index *index,
                            float minimum_distance, int min_checked_leaves, int k,
                            pqueue_bsf (*search_function)(ts_type*, ts_type*, isax_index*, float, int,int));
void isax_knn_query_binary_file(const char *ifilename,const char *labelfilename, int q_num, isax_index *index,
                            float minimum_distance, int min_checked_leaves,int k,long int classlength,
                            pqueue_bsf (*search_function)(ts_type*, ts_type*, isax_index*, float, int,int));
void isax_knn_query_binary_file_traditional(const char *ifilename,const char *labelfilename, int q_num, isax_index *index,
                            float minimum_distance, int min_checked_leaves,int k,long int classlength,
                            pqueue_bsf (*search_function)(ts_type*, ts_type*, isax_index*,node_list*, float, int,int));
#endif
