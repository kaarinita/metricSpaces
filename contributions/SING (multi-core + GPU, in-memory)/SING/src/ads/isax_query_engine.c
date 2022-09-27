//
//  isax_query_engine.c
//  al_isax
//
//  Created by Kostas Zoumpatianos on 4/13/12.
//  Copyright 2012 University of Trento. All rights reserved.
//

// NOTICE: Adaptive splitting only splits when there is no full data file
// To enable full data file splitting remove this check:
// && !node->has_full_data_file &&
#ifdef VALUES
#include <values.h>
#endif
#include <float.h>
#include "../../config.h"
#include "../../globals.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>

#include "ads/isax_query_engine.h"
#include "ads/isax_first_buffer_layer.h"
#include "ads/pqueue.h"
#include "ads/sax/sax.h"
#include "ads/isax_node_split.h"

#define NTHREADS 4

float *MINDISTS;
struct args {
    unsigned int i;
    unsigned long from;
    unsigned long to;
    ts_type *paa;
    isax_index *index;
};


void *compute_mindists(void *ptr) {
    struct args *arguments = (struct args*) ptr;
    //printf("[%u] Computing mindists from: %lu to: %lu\n", arguments->i, arguments->from, arguments->to);
    unsigned long i;

    for(i=arguments->from; i<arguments->to; i++) {
        sax_type *sax = &(arguments->index->sax_cache[i * arguments->index->settings->paa_segments]);
        MINDISTS[i] = minidist_paa_to_isax_raw(arguments->paa, sax,
                                               arguments->index->settings->max_sax_cardinalities,
                                               arguments->index->settings->sax_bit_cardinality,
                                               arguments->index->settings->sax_alphabet_cardinality,
                                               arguments->index->settings->paa_segments, MINVAL, MAXVAL,
                                               arguments->index->settings->mindist_sqrt);
    }
    return NULL;
}



query_result exact_search_serial(ts_type *ts, ts_type *paa, isax_index *index, float minimum_distance, int min_checked_leaves) 
{
    
    RESET_BYTES_ACCESSED
    // FOR THREAD USE
    MINDISTS=malloc(sizeof(float) * index->sax_cache_size);
    unsigned long j;
    for (j = 0; j < index->sax_cache_size; j++)
        MINDISTS[j] = FLT_MAX;
    // END
    
    
    query_result approximate_result = approximate_search(ts, paa, index);
    query_result bsf_result = approximate_result;
    
    
    int tight_bound = index->settings->tight_bound;
    int aggressive_check = index->settings->aggressive_check;
    
    // Early termination...
    if (approximate_result.distance == 0) {
        return approximate_result;
    }
    
    if(approximate_result.distance == FLT_MAX || min_checked_leaves > 1) {
        approximate_result = refine_answer(ts, paa, index, approximate_result, minimum_distance, min_checked_leaves);
    }
    
    
    
    unsigned long i;
    COUNT_INPUT_TIME_START
    FILE *raw_file = fopen(index->settings->raw_filename, "rb");
    fseek(raw_file, 0, SEEK_SET);
    ts_type *ts_buffer = malloc(index->settings->ts_byte_size);
    COUNT_INPUT_TIME_END
    
    SET_APPROXIMATE(approximate_result.distance);

    // THREADED
    pthread_t thread[NTHREADS];
    struct args arguments[NTHREADS];
    //for ( i = 0; i < 100; i++)
    //{
    //    printf("the sax [%d ]%d\n",i,(int)(index->sax_cache[i * index->settings->paa_segments] ));
    //}


    for(i=0; i<NTHREADS; i++) {
        arguments[i].i = i;
        arguments[i].from = i*(index->sax_cache_size / NTHREADS);
        if(i < (NTHREADS-1)) {
            arguments[i].to = (i+1)*(index->sax_cache_size / NTHREADS);
        }
        else {
            arguments[i].to = index->sax_cache_size;
        }
        arguments[i].paa = paa;
        arguments[i].index = index;
        int ret = pthread_create(&thread[i], NULL, compute_mindists, &arguments[i]);
    }
    
    for(i=0; i<NTHREADS;i++) {
        pthread_join(thread[i], NULL);
    }
    // END
    
    //printf("the mindist 95008 is %f\n",MINDISTS[95008] ); 
    //printf("the mindist 280671 is %f\n",MINDISTS[280671] ); 
    //printf("the mindist 692396 is %f\n",MINDISTS[692396] ); 




    for(i=0; i<index->sax_cache_size; i++) {
        sax_type *sax = &index->sax_cache[i * index->settings->paa_segments];

        if(MINDISTS[i] <= approximate_result.distance) {
            COUNT_INPUT_TIME_START
            fseek(raw_file, i * index->settings->ts_byte_size, SEEK_SET);
            fread(ts_buffer, index->settings->ts_byte_size, 1, raw_file);
            COUNT_INPUT_TIME_END
            //printf(" the %d sax is :  %d !!!\n",i,index->sax_cache[i* index->settings->paa_segments] );
            float dist = ts_euclidean_distance(ts, ts_buffer, index->settings->timeseries_size, approximate_result.distance);
            if(dist < approximate_result.distance) {

                approximate_result.distance = dist;

    #ifdef STORE_ANSWER
                memcpy(index->answer, ts_buffer, index->settings->timeseries_size * sizeof(ts_type));
    #endif
            }
            INCREASE_BYTES_ACCESSED(index->settings->ts_byte_size)
        }
    }
    free(ts_buffer);
    fclose(raw_file);
    free(MINDISTS);
    
    return approximate_result;
}
pqueue_bsf exact_topk_serial(ts_type *ts, ts_type *paa, isax_index *index, float minimum_distance, int min_checked_leaves, int k) 
{
    
    RESET_BYTES_ACCESSED
    pqueue_bsf *pq_bsf= pqueue_bsf_init(k);
    // FOR THREAD USE
    MINDISTS=malloc(sizeof(float) * index->sax_cache_size);
    unsigned long j;
    for (j = 0; j < index->sax_cache_size; j++)
        MINDISTS[j] = FLT_MAX;
    // END
    
    
    approximate_topk(ts, paa, index,pq_bsf);
    
    
    int tight_bound = index->settings->tight_bound;
    int aggressive_check = index->settings->aggressive_check;
    
    // Early termination...
    if (pq_bsf->knn[k-1] == 0) {
        return *pq_bsf;
    }
    //  
    if(pq_bsf->knn[k-1] == FLT_MAX  || min_checked_leaves > 1) {
        refine_topk_answer(ts, paa, index, pq_bsf, minimum_distance, min_checked_leaves);
    }
    //printf("the bsf is %f\n",pq_bsf->knn[k-1] );
      
    unsigned long i;
    COUNT_INPUT_TIME_START
    FILE *raw_file = fopen(index->settings->raw_filename, "rb");
    fseek(raw_file, 0, SEEK_SET);
    ts_type *ts_buffer = malloc(index->settings->ts_byte_size);
    COUNT_INPUT_TIME_END
    
    SET_APPROXIMATE(pq_bsf->knn[k-1]);

    // THREADED
    pthread_t thread[NTHREADS];
    struct args arguments[NTHREADS];
    //for ( i = 0; i < 100; i++)
    //{
    //    printf("the sax [%d ]%d\n",i,(int)(index->sax_cache[i * index->settings->paa_segments] ));
    //}


    for(i=0; i<NTHREADS; i++) {
        arguments[i].i = i;
        arguments[i].from = i*(index->sax_cache_size / NTHREADS);
        if(i < (NTHREADS-1)) {
            arguments[i].to = (i+1)*(index->sax_cache_size / NTHREADS);
        }
        else {
            arguments[i].to = index->sax_cache_size;
        }
        arguments[i].paa = paa;
        arguments[i].index = index;
        int ret = pthread_create(&thread[i], NULL, compute_mindists, &arguments[i]);
    }
    
    for(i=0; i<NTHREADS;i++) {
        pthread_join(thread[i], NULL);
    }
    // END
    
    //printf("the mindist 95008 is %f\n",MINDISTS[95008] ); 
    //printf("the mindist 280671 is %f\n",MINDISTS[280671] ); 
    //printf("the mindist 692396 is %f\n",MINDISTS[692396] ); 




    for(i=0; i<index->sax_cache_size; i++) {
        sax_type *sax = &index->sax_cache[i * index->settings->paa_segments];

        if(MINDISTS[i] <= pq_bsf->knn[k-1]) {
            COUNT_INPUT_TIME_START
            fseek(raw_file, i * index->settings->ts_byte_size, SEEK_SET);
            fread(ts_buffer, index->settings->ts_byte_size, 1, raw_file);
            COUNT_INPUT_TIME_END
            //printf(" the %d sax is :  %d !!!\n",i,index->sax_cache[i* index->settings->paa_segments] );
            float dist = ts_euclidean_distance(ts, ts_buffer, index->settings->timeseries_size, pq_bsf->knn[k-1]);
                //approximate_result.distance = dist;
                if (dist <= pq_bsf->knn[pq_bsf->k-1]) {
                #ifdef STORE_ANSWER
                memcpy(index->answer, ts_buffer, index->settings->timeseries_size * sizeof(ts_type));
                #endif
            
                pqueue_bsf_insert(pq_bsf,dist,i,NULL);
            }
            INCREASE_BYTES_ACCESSED(index->settings->ts_byte_size)
        }
    }
    free(ts_buffer);
    fclose(raw_file);
    free(MINDISTS);
    return *pq_bsf;
}

ts_type * get_ads_record(unsigned long tid, isax_index *index) {
	FILE *raw_file = fopen(index->settings->raw_filename, "rb");
	fseek(raw_file, 0, SEEK_SET);
    ts_type *ts_buffer = malloc(index->settings->ts_byte_size);
    fseek(raw_file, tid * index->settings->ts_byte_size, SEEK_SET);
    fread(ts_buffer, index->settings->ts_byte_size, 1, raw_file);
    fclose(raw_file);
    return ts_buffer;
}


query_result  approximate_search (ts_type *ts, ts_type *paa, isax_index *index) 
{
    query_result result;

    sax_type *sax = malloc(sizeof(sax_type) * index->settings->paa_segments);
    sax_from_paa(paa, sax, index->settings->paa_segments,
                 index->settings->sax_alphabet_cardinality,
                 index->settings->sax_bit_cardinality);

    root_mask_type root_mask = 0;
    CREATE_MASK(root_mask, index, sax);

    if (index->fbl->soft_buffers[(int) root_mask].initialized) {
        isax_node *node = index->fbl->soft_buffers[(int) root_mask].node;
        //printf("Root: [%0#6X]\n", (unsigned long int) node);
        // Traverse tree

        // Adaptive splitting
        if (node->is_leaf && !node->has_full_data_file &&
            (node->leaf_size > index->settings->min_leaf_size))
        {
            split_node(index, node);
        }

        while (!node->is_leaf) {
            int location = index->settings->sax_bit_cardinality - 1 -
            node->split_data->split_mask[node->split_data->splitpoint];
            root_mask_type mask = index->settings->bit_masks[location];

            if(sax[node->split_data->splitpoint] & mask)
            {
                node = node->right_child;
            }
            else
            {
                node = node->left_child;
            }

            // Adaptive splitting
            if (node->is_leaf && !node->has_full_data_file &&
                (node->leaf_size > index->settings->min_leaf_size))
            {
                split_node(index, node);
            }
        }

        result.distance = calculate_node_distance(index, node, ts, FLT_MAX);


        result.node = node;
    }
    else {
        result.node = NULL;
        result.distance = FLT_MAX;
    }

    free(sax);

    return result;
}

query_result  approximate_search_manynode (ts_type *ts, ts_type *paa, isax_index *index) 
{
    query_result result;

    sax_type *sax = malloc(sizeof(sax_type) * index->settings->paa_segments);
    sax_from_paa(paa, sax, index->settings->paa_segments,
                 index->settings->sax_alphabet_cardinality,
                 index->settings->sax_bit_cardinality);

    root_mask_type root_mask = 0;
    CREATE_MASK(root_mask, index, sax);

    if (index->fbl->soft_buffers[(int) root_mask].initialized) {
        isax_node *node = index->fbl->soft_buffers[(int) root_mask].node;
        //printf("Root: [%0#6X]\n", (unsigned long int) node);
        // Traverse tree
        isax_node *node2;
        // Adaptive splitting
        if (node->is_leaf && !node->has_full_data_file &&
            (node->leaf_size > index->settings->min_leaf_size))
        {
            split_node(index, node);
        }

        while (!node->is_leaf) {
            int location = index->settings->sax_bit_cardinality - 1 -
            node->split_data->split_mask[node->split_data->splitpoint];
            root_mask_type mask = index->settings->bit_masks[location];

            if(sax[node->split_data->splitpoint] & mask)
            {
                node = node->right_child;
                node2=node->left_child;
            }
            else
            {
                node = node->left_child;
                node2=node->right_child;
            }

            // Adaptive splitting
            if (node->is_leaf && !node->has_full_data_file &&
                (node->leaf_size > index->settings->min_leaf_size))
            {
                split_node(index, node);
            }
        }

        result.distance = calculate_node_distance(index, node, ts, FLT_MAX);
        result.node = node;
        float distance2=calculate_node_distance(index, node2, ts, FLT_MAX);
        if(result.distance>distance2)
        {
            result.distance=distance2;
            result.node = node2;
        }

        
    }
    else {
        result.node = NULL;
        result.distance = FLT_MAX;
    }

    free(sax);

    return result;
}


void  approximate_topk (ts_type *ts, ts_type *paa, isax_index *index, pqueue_bsf *pq_bsf) 
{

    sax_type *sax = malloc(sizeof(sax_type) * index->settings->paa_segments);
    sax_from_paa(paa, sax, index->settings->paa_segments,
                 index->settings->sax_alphabet_cardinality,
                 index->settings->sax_bit_cardinality);

    root_mask_type root_mask = 0;
    CREATE_MASK(root_mask, index, sax);

    if (index->fbl->soft_buffers[(int) root_mask].initialized) {
        isax_node *node = index->fbl->soft_buffers[(int) root_mask].node;
        //printf("Root: [%0#6X]\n", (unsigned long int) node);
        // Traverse tree

        // Adaptive splitting
        if (node->is_leaf && !node->has_full_data_file &&
            (node->leaf_size > index->settings->min_leaf_size))
        {
            split_node(index, node);
        }

        while (!node->is_leaf) {
            int location = index->settings->sax_bit_cardinality - 1 -
            node->split_data->split_mask[node->split_data->splitpoint];
            root_mask_type mask = index->settings->bit_masks[location];

            if(sax[node->split_data->splitpoint] & mask)
            {
                node = node->right_child;
            }
            else
            {
                node = node->left_child;
            }

            // Adaptive splitting
            if (node->is_leaf && !node->has_full_data_file &&
                (node->leaf_size > index->settings->min_leaf_size))
            {
                split_node(index, node);
            }
        }

        calculate_node_topk(index, node, ts, pq_bsf);
    }
    else {

    }
    for (int i = 0; i < pq_bsf->k-1; ++i)
    {
        pq_bsf->knn[i]=pq_bsf->knn[pq_bsf->k-1];
    }
    free(sax);
}
query_result refine_answer (ts_type *ts, ts_type *paa, isax_index *index, 
              query_result approximate_bsf_result, 
                            float minimum_distance, int limit)  
{ 
  query_result bsf_result = approximate_bsf_result; 
 
   int tight_bound = index->settings->tight_bound; 
  int aggressive_check = index->settings->aggressive_check; 
 
    pqueue_t *pq = pqueue_init(index->settings->root_nodes_size, 
                               cmp_pri, get_pri, set_pri, get_pos, set_pos); 
 
    // Insert all root nodes in heap. 
    isax_node *current_root_node = index->first_node; 
    while (current_root_node != NULL) { 
    query_result * mindist_result = malloc(sizeof(query_result)); 
    mindist_result->distance =  minidist_paa_to_isax(paa, current_root_node->isax_values, 
                                              current_root_node->isax_cardinalities, 
                                              index->settings->sax_bit_cardinality, 
                                              index->settings->sax_alphabet_cardinality, 
                                              index->settings->paa_segments, 
                                              MINVAL, MAXVAL, 
                                              index->settings->mindist_sqrt); 
    mindist_result->node = current_root_node; 
        pqueue_insert(pq, mindist_result); 
        current_root_node = current_root_node->next; 
    } 
    query_result * n; 
    int checks = 0; 
    while ((n = pqueue_pop(pq))) 
    { 
    // The best node has a worse mindist, so search is finished! 
        if (n->distance >= bsf_result.distance || n->distance > minimum_distance) { 
            pqueue_insert(pq, n); 
            break; 
        } 
        else { 
          // If it is a leaf, check its real distance. 
            if (n->node->is_leaf) { 
        // *** ADAPTIVE SPLITTING *** 
            if (!n->node->has_full_data_file && 
          (n->node->leaf_size > index->settings->min_leaf_size)) 
            { 
          // Split and push again in the queue 
                split_node(index, n->node); 
          pqueue_insert(pq, n); 
                continue; 
            } 
        // *** EXTRA BOUNDING *** 
        if(tight_bound) { 
          float mindistance = calculate_minimum_distance(index, n->node, ts, paa); 
          if(mindistance >= bsf_result.distance) 
          { 
            free(n); 
            continue; 
          } 
        } 
        // *** REAL DISTANCE *** 
        checks++; 
        float distance = calculate_node_distance(index, n->node, ts, bsf_result.distance); 
        if (distance < bsf_result.distance) 
        { 
          bsf_result.distance = distance; 
          bsf_result.node = n->node; 
        } 
        if(checks > limit) { 
          pqueue_insert(pq, n); 
          break; 
        } 
      } 
            else { 
              // If it is an intermediate node calculate mindist for children 
              // and push them in the queue 
                if (n->node->left_child->isax_cardinalities != NULL) { 
          if(n->node->left_child->is_leaf && !n->node->left_child->has_partial_data_file && aggressive_check){ 
            float distance = calculate_node_distance(index, n->node->left_child, ts, bsf_result.distance); 
            if (distance < bsf_result.distance) 
            { 
              bsf_result.distance = distance; 
              bsf_result.node = n->node->left_child; 
            } 
          } 
          else { 
                    query_result * mindist_result = malloc(sizeof(query_result)); 
                    mindist_result->distance =  minidist_paa_to_isax(paa, n->node->left_child->isax_values, 
                                                                     n->node->left_child->isax_cardinalities, 
                                                                     index->settings->sax_bit_cardinality, 
                                                                     index->settings->sax_alphabet_cardinality, 
                                                                     index->settings->paa_segments, 
                                                                     MINVAL, MAXVAL, 
                                                                     index->settings->mindist_sqrt); 
          mindist_result->node = n->node->left_child; 
                    pqueue_insert(pq, mindist_result); 
          } 
                } 
                if (n->node->right_child->isax_cardinalities != NULL) { 
          if(n->node->right_child->is_leaf && !n->node->left_child->has_partial_data_file && aggressive_check){ 
            float distance = calculate_node_distance(index, n->node->right_child, ts, bsf_result.distance); 
            if (distance < bsf_result.distance) 
            { 
              bsf_result.distance = distance; 
              bsf_result.node = n->node->right_child; 
            } 
          } 
          else { 
                    query_result * mindist_result = malloc(sizeof(query_result)); 
          mindist_result->distance =  minidist_paa_to_isax(paa, n->node->right_child->isax_values, 
                                                                     n->node->right_child->isax_cardinalities, 
                                                                     index->settings->sax_bit_cardinality, 
                                                                     index->settings->sax_alphabet_cardinality, 
                                                                     index->settings->paa_segments, 
                                                                     MINVAL, MAXVAL, 
                                                                     index->settings->mindist_sqrt); 
                    mindist_result->node = n->node->right_child; 
                    pqueue_insert(pq, mindist_result); 
          } 
                } 
            } 
 
            // Free the node currently popped. 
           free(n); 
        } 
    } 
    // Free the nodes that where not popped. 
    while ((n = pqueue_pop(pq))) 
    { 
        free(n); 
    } 
    // Free the priority queue. 
    pqueue_free(pq); 
   return bsf_result; 
} 
void refine_topk_answer (ts_type *ts, ts_type *paa, isax_index *index, 
              pqueue_bsf *pq_bsf, 
                            float minimum_distance, int limit)  
{  
   int tight_bound = index->settings->tight_bound; 
  int aggressive_check = index->settings->aggressive_check; 
 
    pqueue_t *pq = pqueue_init(index->settings->root_nodes_size, 
                               cmp_pri, get_pri, set_pri, get_pos, set_pos); 
 
    // Insert all root nodes in heap. 
    isax_node *current_root_node = index->first_node; 
    while (current_root_node != NULL) { 
    query_result * mindist_result = malloc(sizeof(query_result)); 
    mindist_result->distance =  minidist_paa_to_isax(paa, current_root_node->isax_values, 
                                              current_root_node->isax_cardinalities, 
                                              index->settings->sax_bit_cardinality, 
                                              index->settings->sax_alphabet_cardinality, 
                                              index->settings->paa_segments, 
                                              MINVAL, MAXVAL, 
                                              index->settings->mindist_sqrt); 
    mindist_result->node = current_root_node; 
        if (mindist_result->distance < pq_bsf->knn[pq_bsf->k-1])
        {
            pqueue_insert(pq, mindist_result);
        }
        current_root_node = current_root_node->next; 
    } 
    query_result * n; 
    int checks = 0; 
    while ((n = pqueue_pop(pq))) 
    { 
    // The best node has a worse mindist, so search is finished! 
        if (n->distance >= pq_bsf->knn[pq_bsf->k-1] || n->distance > minimum_distance) {
            pqueue_insert(pq, n);
            break;
        } 
        else { 
          // If it is a leaf, check its real distance. 
            if (n->node->is_leaf) { 
        // *** ADAPTIVE SPLITTING *** 
            if (!n->node->has_full_data_file && 
          (n->node->leaf_size > index->settings->min_leaf_size)) 
            { 
          // Split and push again in the queue 
                split_node(index, n->node); 
          pqueue_insert(pq, n); 
                continue; 
            } 
        // *** EXTRA BOUNDING *** 
        if(tight_bound) { 
          float mindistance = calculate_minimum_distance(index, n->node, ts, paa); 
                    if(mindistance >= pq_bsf->knn[pq_bsf->k-1])
                    {
                        free(n);
                        continue;
                    }
        } 
        // *** REAL DISTANCE *** 
        checks++; 
        calculate_node_topk(index, n->node, ts, pq_bsf);
 
                if(pq_bsf->knn[pq_bsf->k-1] < FLT_MAX) {
                    pqueue_insert(pq, n);
                    break;
                }
      } 
            else { 
              // If it is an intermediate node calculate mindist for children 
              // and push them in the queue 
                if (n->node->left_child->isax_cardinalities != NULL) { 
          if(n->node->left_child->is_leaf && !n->node->left_child->has_partial_data_file && aggressive_check){ 
            calculate_node_topk(index, n->node->left_child, ts, pq_bsf);

          } 
          else { 
                    query_result * mindist_result = malloc(sizeof(query_result)); 
                    mindist_result->distance =  minidist_paa_to_isax(paa, n->node->left_child->isax_values, 
                                                                     n->node->left_child->isax_cardinalities, 
                                                                     index->settings->sax_bit_cardinality, 
                                                                     index->settings->sax_alphabet_cardinality, 
                                                                     index->settings->paa_segments, 
                                                                     MINVAL, MAXVAL, 
                                                                     index->settings->mindist_sqrt); 
          mindist_result->node = n->node->left_child;  
            if (mindist_result->distance < pq_bsf->knn[pq_bsf->k-1])
            {
                pqueue_insert(pq, mindist_result);
            }
          } 
                } 
                if (n->node->right_child->isax_cardinalities != NULL) { 
          if(n->node->right_child->is_leaf && !n->node->left_child->has_partial_data_file && aggressive_check){ 
            calculate_node_topk(index, n->node->right_child, ts, pq_bsf);
          } 
          else { 
                    query_result * mindist_result = malloc(sizeof(query_result)); 
          mindist_result->distance =  minidist_paa_to_isax(paa, n->node->right_child->isax_values, 
                                                                     n->node->right_child->isax_cardinalities, 
                                                                     index->settings->sax_bit_cardinality, 
                                                                     index->settings->sax_alphabet_cardinality, 
                                                                     index->settings->paa_segments, 
                                                                     MINVAL, MAXVAL, 
                                                                     index->settings->mindist_sqrt); 
                    mindist_result->node = n->node->right_child; 
            if (mindist_result->distance < pq_bsf->knn[pq_bsf->k-1])
            {
                pqueue_insert(pq, mindist_result);
            } 
          } 
                } 
            } 
 
            // Free the node currently popped. 
           free(n); 
        } 
    } 
    // Free the nodes that where not popped. 
    while ((n = pqueue_pop(pq))) 
    { 
        free(n); 
    } 
    // Free the priority queue. 
    for (int i = 0; i < pq_bsf->k-1; ++i)
    {
        pq_bsf->knn[i]=pq_bsf->knn[pq_bsf->k-1];
    }
    pqueue_free(pq); 
} 
query_result  approximate_search_SIMD (ts_type *ts, ts_type *paa, isax_index *index) 
{
    query_result result;

    sax_type *sax = malloc(sizeof(sax_type) * index->settings->paa_segments);
    sax_from_paa(paa, sax, index->settings->paa_segments,
                 index->settings->sax_alphabet_cardinality,
                 index->settings->sax_bit_cardinality);

    root_mask_type root_mask = 0;
    CREATE_MASK(root_mask, index, sax);

    if (index->fbl->soft_buffers[(int) root_mask].initialized) {
        isax_node *node = index->fbl->soft_buffers[(int) root_mask].node;
        //printf("Root: [%0#6X]\n", (unsigned long int) node);
        // Traverse tree

        // Adaptive splitting
        if (node->is_leaf && !node->has_full_data_file &&
            (node->leaf_size > index->settings->min_leaf_size))
        {
            split_node(index, node);
        }

        while (!node->is_leaf) {
            int location = index->settings->sax_bit_cardinality - 1 -
            node->split_data->split_mask[node->split_data->splitpoint];
            root_mask_type mask = index->settings->bit_masks[location];

            if(sax[node->split_data->splitpoint] & mask)
            {
                node = node->right_child;
            }
            else
            {
                node = node->left_child;
            }

            // Adaptive splitting
            if (node->is_leaf && !node->has_full_data_file &&
                (node->leaf_size > index->settings->min_leaf_size))
            {
                split_node(index, node);
            }
        }

        result.distance = calculate_node_distance(index, node, ts, FLT_MAX);
        result.node = node;
    }
    else {
        result.node = NULL;
        result.distance = FLT_MAX;
    }

    free(sax);
}


/*
query_result exact_search_serial(ts_type *ts, ts_type *paa, isax_index *index, float minimum_distance, int min_checked_leaves) 
{

	RESET_BYTES_ACCESSED
    
	query_result approximate_result = approximate_search(ts, paa, index);

    query_result bsf_result = approximate_result;
    query_result *approximate_result2=malloc(sizeof(query_result));
    approximate_result2->distance=approximate_result.distance;
    approximate_result2->node=approximate_result.node;
    approximate_result2->pqueue_position=approximate_result.pqueue_position;

	int tight_bound = index->settings->tight_bound;
	int aggressive_check = index->settings->aggressive_check;

    // Early termination...
    if (approximate_result.distance == 0) {
        return approximate_result;
    }
    
    if(approximate_result.distance == FLT_MAX || min_checked_leaves > 1) {
        approximate_result = refine_answer(ts, paa, index, approximate_result, minimum_distance, min_checked_leaves);
        //approximate_result = refine_answer_m(ts, paa, index, approximate_result2, minimum_distance, min_checked_leaves);
    }
    

    /*BIT_ARRAY* bitarray = bit_array_create(index->sax_cache_size);

    unsigned long i;
    COUNT_INPUT_TIME_START
    FILE *raw_file = fopen(index->settings->raw_filename, "rb");
    fseek(raw_file, 0, SEEK_SET);
    ts_type *ts_buffer = malloc(index->settings->ts_byte_size);
    COUNT_INPUT_TIME_END
    #ifdef AUTO_TUNE
    float *mindists = malloc(sizeof(float) * index->sax_cache_size);
    #endif

    SET_APPROXIMATE(approximate_result.distance);
    //printf("the index->sax_cache_size is %ld\n",index->sax_cache_size);
        //printf("the old distance is: %f \n",approximate_result.distance);

    for(i=0; i<index->sax_cache_size; i++) {
        
    	sax_type *sax = &index->sax_cache[i * index->settings->paa_segments];

    	float mindist = minidist_paa_to_isax_raw(paa, sax, index->settings->max_sax_cardinalities,
    													 index->settings->sax_bit_cardinality,
    													 index->settings->sax_alphabet_cardinality,
    													 index->settings->paa_segments, MINVAL, MAXVAL,
    													 index->settings->mindist_sqrt);
        
    	if(mindist <= approximate_result.distance) {
    		/*bit_array_set_bit(bitarray, i);
    		COUNT_INPUT_TIME_START
    		fseek(raw_file, i * index->settings->ts_byte_size, SEEK_SET);
    		fread(ts_buffer, index->settings->ts_byte_size, 1, raw_file);
    		COUNT_INPUT_TIME_END

    		float dist = ts_euclidean_distance(ts, ts_buffer, index->settings->timeseries_size, approximate_result.distance);
            //printf("the distance is : %f !!!!!!!!!\n", dist);
    		if(dist < approximate_result.distance) {
    			approximate_result.distance = dist;
                
                #ifdef STORE_ANSWER
                memcpy(index->answer, ts_buffer, index->settings->timeseries_size * sizeof(ts_type));
                #endif
    		}
    		INCREASE_BYTES_ACCESSED(index->settings->ts_byte_size)
    	}
        
    }
        //printf("the new distance is: %f \n",approximate_result.distance);

    free(ts_buffer);
    fclose(raw_file);
    //printf("I need to check: %2.2lf%% of the data.\n", (double)tocheck*100/(double)index->sax_cache_size);
    /*bit_array_free(bitarray);
    return approximate_result;
}
*/


query_result exact_search (ts_type *ts, ts_type *paa, isax_index *index,
                           float minimum_distance, int min_checked_leaves) 
{
    query_result approximate_result = approximate_search(ts, paa, index);
    query_result bsf_result = approximate_result;
    int tight_bound = index->settings->tight_bound;
    int aggressive_check = index->settings->aggressive_check;

    // Early termination...
    if (approximate_result.distance == 0) {
        return approximate_result;
    }
    if(approximate_result.distance == FLT_MAX || min_checked_leaves > 1) {
        approximate_result = refine_answer(ts, paa, index, approximate_result, minimum_distance, min_checked_leaves);
    }
    COUNT_QUEUE_TIME_START
    pqueue_t *pq = pqueue_init(index->settings->root_nodes_size,
                               cmp_pri, get_pri, set_pri, get_pos, set_pos);
    COUNT_QUEUE_TIME_END


    query_result *do_not_remove = &approximate_result;

    SET_APPROXIMATE(approximate_result.distance);

    RESET_BYTES_ACCESSED

    if(approximate_result.node != NULL) {
        // Insert approximate result in heap.
        COUNT_QUEUE_TIME_START
        pqueue_insert(pq, &approximate_result);
        COUNT_QUEUE_TIME_END
        //GOOD: if(approximate_result.node->filename != NULL)
        //GOOD: printf("POPS: %.5lf\t", approximate_result.distance);
    }

    // Insert all root nodes in heap.
    isax_node *current_root_node = index->first_node;
    while (current_root_node != NULL) {
        query_result * mindist_result = malloc(sizeof(query_result));
        mindist_result->distance =  minidist_paa_to_isax(paa, current_root_node->isax_values,
                                              current_root_node->isax_cardinalities,
                                              index->settings->sax_bit_cardinality,
                                              index->settings->sax_alphabet_cardinality,
                                              index->settings->paa_segments,
                                              MINVAL, MAXVAL,
                                              index->settings->mindist_sqrt);
        mindist_result->node = current_root_node;
        COUNT_QUEUE_TIME_START
        pqueue_insert(pq, mindist_result);
        COUNT_QUEUE_TIME_END
        current_root_node = current_root_node->next;
    }
    query_result * n;
    int checks = 0;
    while ((n = pqueue_pop(pq)))
    {
        // The best node has a worse mindist, so search is finished!
        //printf("this is the check point of e s !!!\n");
        if (n->distance >= bsf_result.distance || n->distance > minimum_distance) {
            COUNT_QUEUE_TIME_START
            pqueue_insert(pq, n);
            COUNT_QUEUE_TIME_END
            break;
        }
        else {
            // If it is a leaf, check its real distance.
            if (n->node->is_leaf) {
                // *** ADAPTIVE SPLITTING ***
                if (!n->node->has_full_data_file &&
                    (n->node->leaf_size > index->settings->min_leaf_size))
                {
                    // Split and push again in the queue
                    
                    split_node(index, n->node);
                    COUNT_QUEUE_TIME_START
                    pqueue_insert(pq, n);
                    COUNT_QUEUE_TIME_END
                    continue;
                }
                // *** EXTRA BOUNDING ***
                if(tight_bound) {
                    COUNT_CAL_TIME_START
                    float mindistance = calculate_minimum_distance(index, n->node, ts, paa);
                    COUNT_CAL_TIME_END
                    if(mindistance >= bsf_result.distance)
                    {
                        if(n != do_not_remove)//add
                            free(n);
                        continue;
                    }
                }
                // *** REAL DISTANCE ***
                checks++;

                COUNT_CAL_TIME_START
                float distance = calculate_node_distance(index, n->node, ts, bsf_result.distance);
                COUNT_CAL_TIME_END
                if (distance < bsf_result.distance)
                {
                    bsf_result.distance = distance;
                    bsf_result.node = n->node;
                }
                //no check limit juge
            }
            else {
                // If it is an intermediate node calculate mindist for children
                // and push them in the queue
                if (n->node->left_child->isax_cardinalities != NULL) {
                    if(n->node->left_child->is_leaf && !n->node->left_child->has_partial_data_file && aggressive_check){
                        COUNT_CAL_TIME_START
                        float distance = calculate_node_distance(index, n->node->left_child, ts, bsf_result.distance);
                        COUNT_CAL_TIME_END
                        if (distance < bsf_result.distance)
                        {
                            bsf_result.distance = distance;
                            bsf_result.node = n->node->left_child;
                        }
                    }
                    else {
                    query_result * mindist_result = malloc(sizeof(query_result));
                    mindist_result->distance =  minidist_paa_to_isax(paa, n->node->left_child->isax_values,
                                                                     n->node->left_child->isax_cardinalities,
                                                                     index->settings->sax_bit_cardinality,
                                                                     index->settings->sax_alphabet_cardinality,
                                                                     index->settings->paa_segments,
                                                                     MINVAL, MAXVAL,
                                                                     index->settings->mindist_sqrt);
                    mindist_result->node = n->node->left_child;
                    COUNT_QUEUE_TIME_START
                    pqueue_insert(pq, mindist_result);
                    COUNT_QUEUE_TIME_END
                    }
                }
                if (n->node->right_child->isax_cardinalities != NULL) {
                    if(n->node->right_child->is_leaf && !n->node->left_child->has_partial_data_file && aggressive_check){
                        COUNT_CAL_TIME_START
                        float distance = calculate_node_distance(index, n->node->right_child, ts, bsf_result.distance);
                        COUNT_CAL_TIME_END
                        if (distance < bsf_result.distance)
                        {
                            bsf_result.distance = distance;
                            bsf_result.node = n->node->right_child;
                        }
                    }
                    else {
                    query_result * mindist_result = malloc(sizeof(query_result));
                    mindist_result->distance =  minidist_paa_to_isax(paa, n->node->right_child->isax_values,
                                                                     n->node->right_child->isax_cardinalities,
                                                                     index->settings->sax_bit_cardinality,
                                                                     index->settings->sax_alphabet_cardinality,
                                                                     index->settings->paa_segments,
                                                                     MINVAL, MAXVAL,
                                                                     index->settings->mindist_sqrt);
                    mindist_result->node = n->node->right_child;
                    COUNT_QUEUE_TIME_START
                    pqueue_insert(pq, mindist_result);
                    COUNT_QUEUE_TIME_END
                    }
                }
            }

            // Free the node currently popped.
            if(n != do_not_remove)//add
                free(n);
        }
    }

    // Free the nodes that where not popped.
    while ((n = pqueue_pop(pq)))
    {
        if(n != do_not_remove)
            free(n);
    }
    // Free the priority queue.
    COUNT_QUEUE_TIME_START
    pqueue_free(pq);
    COUNT_QUEUE_TIME_END
    //PRINT_BYTES_ACCESSED
   return bsf_result;
}

pqueue_bsf exact_topk (ts_type *ts, ts_type *paa, isax_index *index, float minimum_distance, int min_checked_leaves, int k)
{
    pqueue_bsf *pq_bsf = pqueue_bsf_init(k);
    approximate_topk(ts, paa, index, pq_bsf);
    int tight_bound = index->settings->tight_bound;
    int aggressive_check = index->settings->aggressive_check;

    // Early termination...
    if (pq_bsf->knn[k-1] == 0) {
        return *pq_bsf;
    }
    if(pq_bsf->knn[k-1] == FLT_MAX || min_checked_leaves > 1) {
        refine_topk_answer(ts, paa, index, pq_bsf, minimum_distance, min_checked_leaves);
    }

    COUNT_QUEUE_TIME_START
    pqueue_t *pq = pqueue_init(index->settings->root_nodes_size, cmp_pri, get_pri, set_pri, get_pos, set_pos);
    COUNT_QUEUE_TIME_END

    //pqueue_bsf *do_not_remove = &pq_bsf;

    SET_APPROXIMATE(pq_bsf->knn[k-1]);

    RESET_BYTES_ACCESSED

    if(pq_bsf->node[k-1] != NULL) {
        // Insert approximate result in heap.
        COUNT_QUEUE_TIME_START
        //pqueue_insert(pq, &pq_bsf);
        COUNT_QUEUE_TIME_END
        //GOOD: if(approximate_result.node->filename != NULL)
        //GOOD: printf("POPS: %.5lf\t", approximate_result.distance);
    }

    // Insert all root nodes in heap.
    isax_node *current_root_node = index->first_node;
    while (current_root_node != NULL) {
        query_result * mindist_result = malloc(sizeof(query_result));
        mindist_result->distance =  minidist_paa_to_isax(paa, current_root_node->isax_values,
                                              current_root_node->isax_cardinalities,
                                              index->settings->sax_bit_cardinality,
                                              index->settings->sax_alphabet_cardinality,
                                              index->settings->paa_segments,
                                              MINVAL, MAXVAL,
                                              index->settings->mindist_sqrt);
        mindist_result->node = current_root_node;
        if (mindist_result->distance < pq_bsf->knn[k-1]) {
            COUNT_QUEUE_TIME_START
            pqueue_insert(pq, mindist_result);
            COUNT_QUEUE_TIME_END
        }
        current_root_node = current_root_node->next;
    }
    query_result * n;
    int checks = 0;
    while ((n = pqueue_pop(pq)))
    {
        // The best node has a worse mindist, so search is finished!
        //printf("this is the check point of e s !!!\n");
        if (n->distance >= pq_bsf->knn[pq_bsf->k-1] || n->distance > minimum_distance) {
            COUNT_QUEUE_TIME_START
            pqueue_insert(pq, n);
            COUNT_QUEUE_TIME_END
            break;
        }
        else {
            // If it is a leaf, check its real distance.
            if (n->node->is_leaf) {
                // *** ADAPTIVE SPLITTING ***
                if (!n->node->has_full_data_file &&
                    (n->node->leaf_size > index->settings->min_leaf_size))
                {
                    // Split and push again in the queue
                    
                    split_node(index, n->node);
                    COUNT_QUEUE_TIME_START
                    pqueue_insert(pq, n);
                    COUNT_QUEUE_TIME_END
                    continue;
                }
                // *** EXTRA BOUNDING ***
                if(tight_bound) {
                    COUNT_CAL_TIME_START
                    float mindistance = calculate_minimum_distance(index, n->node, ts, paa);
                    COUNT_CAL_TIME_END
                    if(mindistance >= pq_bsf->knn[pq_bsf->k-1])
                    {
                        free(n);
                        continue;
                    }
                }
                // *** REAL DISTANCE ***
                checks++;
                calculate_node_topk(index, n->node, ts, pq_bsf);

                //no check limit juge
            }
            else {
                // If it is an intermediate node calculate mindist for children
                // and push them in the queue
                if (n->node->left_child->isax_cardinalities != NULL) {
                    if(n->node->left_child->is_leaf && !n->node->left_child->has_partial_data_file && aggressive_check){
                        calculate_node_topk(index, n->node->left_child, ts, pq_bsf);
                    }
                    else {
                    query_result * mindist_result = malloc(sizeof(query_result));
                    mindist_result->distance =  minidist_paa_to_isax(paa, n->node->left_child->isax_values,
                                                                     n->node->left_child->isax_cardinalities,
                                                                     index->settings->sax_bit_cardinality,
                                                                     index->settings->sax_alphabet_cardinality,
                                                                     index->settings->paa_segments,
                                                                     MINVAL, MAXVAL,
                                                                     index->settings->mindist_sqrt);
                    mindist_result->node = n->node->left_child;
                    COUNT_QUEUE_TIME_START
                    pqueue_insert(pq, mindist_result);
                    COUNT_QUEUE_TIME_END
                    }
                }
                if (n->node->right_child->isax_cardinalities != NULL) {
                    if(n->node->right_child->is_leaf && !n->node->left_child->has_partial_data_file && aggressive_check){
                        calculate_node_topk(index, n->node->right_child, ts, pq_bsf);
                    }
                    else {
                    query_result * mindist_result = malloc(sizeof(query_result));
                    mindist_result->distance =  minidist_paa_to_isax(paa, n->node->right_child->isax_values,
                                                                     n->node->right_child->isax_cardinalities,
                                                                     index->settings->sax_bit_cardinality,
                                                                     index->settings->sax_alphabet_cardinality,
                                                                     index->settings->paa_segments,
                                                                     MINVAL, MAXVAL,
                                                                     index->settings->mindist_sqrt);
                    mindist_result->node = n->node->right_child;
                    COUNT_QUEUE_TIME_START
                    pqueue_insert(pq, mindist_result);
                    COUNT_QUEUE_TIME_END
                    }
                }
            }
        }
    }

    // Free the nodes that where not popped.
    while ((n = pqueue_pop(pq)))
    {
        //if(n != do_not_remove)
        //free(n);
    }
    // Free the priority queue.
    COUNT_QUEUE_TIME_START
    pqueue_free(pq);
    COUNT_QUEUE_TIME_END
    //PRINT_BYTES_ACCESSED
    /*for (int i=0; i < k-1; i++)
    {
        pq_bsf->knn[i] = pq_bsf->knn[k-1];
    }*/
    return *pq_bsf;
}



query_result sanity_check_query (ts_type *ts, ts_type *paa, isax_index *index, float minimum_distance, int min_checked_leaves) {
	float isax_tightness = 0;
	float tight_tightness = 0;
	long checks = 0;

	char failed = 0;
	query_result bsf_result;
    bsf_result.distance = FLT_MAX;

    pqueue_t *pq = pqueue_init(index->settings->root_nodes_size,
                               cmp_pri, get_pri, set_pri, get_pos, set_pos);

    RESET_BYTES_ACCESSED
    // Insert all root nodes in heap.
    isax_node *current_root_node = index->first_node;
    while (current_root_node != NULL) {
		query_result * mindist_result = malloc(sizeof(query_result));
		mindist_result->distance =  minidist_paa_to_isax(paa, current_root_node->isax_values,
                                              current_root_node->isax_cardinalities,
                                              index->settings->sax_bit_cardinality,
                                              index->settings->sax_alphabet_cardinality,
                                              index->settings->paa_segments,
                                              MINVAL, MAXVAL,
                                              index->settings->mindist_sqrt);
		mindist_result->node = current_root_node;
        pqueue_insert(pq, mindist_result);
        current_root_node = current_root_node->next;
    }
    query_result * n;
    while ((n = pqueue_pop(pq)))
    {
		// If it is a leaf, check its real distance.
		if (n->node->is_leaf) {
			float isax_mindist = minidist_paa_to_isax(paa, n->node->isax_values,
					 n->node->isax_cardinalities,
					 index->settings->sax_bit_cardinality,
					 index->settings->sax_alphabet_cardinality,
					 index->settings->paa_segments,
					 MINVAL, MAXVAL,
					 index->settings->mindist_sqrt);
			float tight_mindist = calculate_minimum_distance(index, n->node, ts, paa);
			float distance = calculate_node_distance(index, n->node, ts, FLT_MAX);

			checks++;
			tight_tightness += tight_mindist / distance;
			isax_tightness += isax_mindist / distance;

			if(distance < isax_mindist) {
				printf("ERROR: Real distance (%2.2lf) is less than iSAX distance (%2.2lf)\n", distance, isax_mindist);
				failed = 1;
			}
			if(distance < tight_mindist) {
				printf("ERROR: Real distance (%2.2lf) is less than the TIGHT distance (%2.2lf)\n", distance, tight_mindist);
				failed = 1;

				printf("DEBUGGING...\n");
				isax_node *node = n->node;
				char *filename = malloc((strlen(node->filename) + 10) * sizeof(char));
				strcpy(filename, node->filename);
				strcat(filename, ".full");
				filename[strlen(node->filename)+5] = '\0';
				FILE *leaf_file = fopen(filename, "rb");
                if(leaf_file == NULL) {
                	fprintf(stderr, "Canot open: %s\n", filename);
                	continue;
                }
				fseeko(leaf_file,0L, SEEK_END);
				unsigned long size = ftello(leaf_file) / index->settings->full_record_size;
				fseeko(leaf_file, 0L, SEEK_SET);

				int i;
				ts_type *ts_read = malloc(index->settings->ts_byte_size);
				ts_type *min_wedge = malloc(index->settings->ts_byte_size);
				ts_type *max_wedge = malloc(index->settings->ts_byte_size);

				for(i=0; i<index->settings->timeseries_size; i++) {
					min_wedge[i] = 100000;
					max_wedge[i] = -100000;
				}

				float mindist = FLT_MAX;
				float bound = 0;

				for(i=0; i<size; i++) {
					fseeko(leaf_file, index->settings->sax_byte_size + index->settings->position_byte_size, SEEK_CUR);
					fread(ts_read, index->settings->ts_byte_size, 1, leaf_file);
					float dist = ts_euclidean_distance(ts, ts_read, index->settings->timeseries_size, mindist);
					if(dist < mindist) {
						mindist = dist;
					}
					int j=0;
					for(j=0; j<index->settings->timeseries_size; j++) {
						if(ts_read[j] < min_wedge[j]) {
							min_wedge[j] = ts_read[j];
						}
						if(ts_read[j] > max_wedge[j]) {
							max_wedge[j] = ts_read[j];
						}
					}
				}


				for(i=0; i<index->settings->timeseries_size; i++) {
					if(ts[i] > max_wedge[i]) {
						bound += (ts[i] - max_wedge[i]) * (ts[i] - max_wedge[i]);
					}
					else if(ts[i] < max_wedge[i] && ts[i] > min_wedge[i]) {
						//bound += 0;
					}
					else {
						bound += (min_wedge[i] - ts[i]) * (min_wedge[i] - ts[i]);
					}
				}
				bound = sqrtf(bound);

				printf("\t%lf \n", mindist);
				printf("\tBound: %lf\n", bound);
				free(ts_read);
				free(min_wedge);
				free(max_wedge);
				free(filename);
				fclose(leaf_file);
			}

			if(distance < bsf_result.distance) {
				bsf_result.distance = distance;
			}
		}
		else {
			// If it is an intermediate node calculate mindist for children
			// and push them in the queue
			if (n->node->left_child->isax_cardinalities != NULL) {
				query_result * mindist_result = malloc(sizeof(query_result));
				mindist_result->distance =  minidist_paa_to_isax(paa, n->node->left_child->isax_values,
																 n->node->left_child->isax_cardinalities,
																 index->settings->sax_bit_cardinality,
																 index->settings->sax_alphabet_cardinality,
																 index->settings->paa_segments,
																 MINVAL, MAXVAL,
																 index->settings->mindist_sqrt);
				mindist_result->node = n->node->left_child;
				pqueue_insert(pq, mindist_result);
			}
			if (n->node->right_child->isax_cardinalities != NULL) {
				query_result * mindist_result = malloc(sizeof(query_result));
				mindist_result->distance =  minidist_paa_to_isax(paa, n->node->right_child->isax_values,
																 n->node->right_child->isax_cardinalities,
																 index->settings->sax_bit_cardinality,
																 index->settings->sax_alphabet_cardinality,
																 index->settings->paa_segments,
																 MINVAL, MAXVAL,
																 index->settings->mindist_sqrt);
				mindist_result->node = n->node->right_child;
				pqueue_insert(pq, mindist_result);
			}
		}
		// Free the node currently popped.
		free(n);
    }

    // Free the nodes that where not popped.
    while ((n = pqueue_pop(pq)))
    {
        free(n);
    }
    // Free the priority queue.
    pqueue_free(pq);


    query_result answer = exact_search(ts, paa, index, minimum_distance, min_checked_leaves);
    query_result serial_answer = exact_search_serial(ts, paa, index, minimum_distance, min_checked_leaves);

    if(answer.distance != bsf_result.distance) {
    	fprintf(stderr,"ERROR: answer %2.2lf is not the same as exhaustive search %2.2lf.\n", answer.distance, bsf_result.distance);
    	failed = 1;
    }
    if(serial_answer.distance != bsf_result.distance) {
    	fprintf(stderr,"ERROR: serial answer %2.2lf is not the same as exhaustive search %2.2lf.\n", serial_answer.distance, bsf_result.distance);
        failed = 1;
    }

    if(!failed) {
    	fprintf(stderr,"[OK] %2.2lf == %2.2lf == %2.2lf\n", answer.distance, serial_answer.distance, bsf_result.distance);
    	fprintf(stderr,"\t [TIGHTNESSES] iSAX=%lf, TIGHT:%lf\n", isax_tightness/(float)checks, tight_tightness / (float)checks);
    }
    else {
    	fprintf(stderr, "[FAILED]\n");
    }
    return bsf_result;
}
