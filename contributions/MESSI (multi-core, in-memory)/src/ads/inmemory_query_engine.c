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
#include <pthread.h>
#include <stdbool.h>
#include <omp.h>
#include "ads/isax_query_engine.h"
#include "ads/inmemory_query_engine.h"
#include "ads/parallel_query_engine.h"
#include "ads/parallel_inmemory_query_engine.h"
#include "ads/parallel_index_engine.h"
#include "ads/isax_first_buffer_layer.h"
#include "ads/pqueue.h"
#include "ads/sax/sax.h"
#include "ads/isax_node_split.h"
#define NTHREADS 4
int checkts=0;
float *MINDISTS;
void *compute_mindists_in(void *ptr) {
    struct args_in *arguments = (struct args_in*) ptr;
    unsigned long i;
 
    for(i=arguments->from; i<arguments->to; i++) {
        sax_type *sax = &(arguments->index->sax_cache[i * arguments->index->settings->paa_segments]);
        MINDISTS[i] = minidist_paa_to_isax_rawa_SIMD(arguments->paa, sax,
                                               arguments->index->settings->max_sax_cardinalities,
                                               arguments->index->settings->sax_bit_cardinality,
                                               arguments->index->settings->sax_alphabet_cardinality,
                                               arguments->index->settings->paa_segments, MINVAL, MAXVAL,
                                               arguments->index->settings->mindist_sqrt);
    }

    return NULL;
}

query_result  approximate_search_inmemory (ts_type *ts, ts_type *paa, isax_index *index) 
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
        // Traverse tree

        // Adaptive splitting

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
        }
        result.distance = calculate_node_distance_inmemory(index, node, ts, FLT_MAX);
        result.node = node;
    }
    else {
        result.node = NULL;
        result.distance = FLT_MAX;
    }

    free(sax);

    return result;
}
query_result  approximate_search_inmemory_messi (ts_type *ts, ts_type *paa, isax_index *index) 
{
    query_result result;

    sax_type *sax = malloc(sizeof(sax_type) * index->settings->paa_segments);
    sax_from_paa(paa, sax, index->settings->paa_segments,
                 index->settings->sax_alphabet_cardinality,
                 index->settings->sax_bit_cardinality);

    root_mask_type root_mask = 0;
    CREATE_MASK(root_mask, index, sax);


    if ((&((first_buffer_layer2*)(index->fbl))->soft_buffers[(int) root_mask])->initialized) {
        isax_node *node = (&((first_buffer_layer2*)(index->fbl))->soft_buffers[(int) root_mask])->node;
        // Traverse tree

        // Adaptive splitting

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
        }
        result.distance = calculate_node_distance_inmemory_omp(index, node, ts, FLT_MAX);
        result.node = node;
    }
    else {
        result.node = NULL;
        result.distance = FLT_MAX;
    }

    free(sax);

    return result;
}
query_result  approximate_search_inmemory_pRecBuf (ts_type *ts, ts_type *paa, isax_index *index) 
{
    query_result result;

    sax_type *sax = malloc(sizeof(sax_type) * index->settings->paa_segments);
    sax_from_paa(paa, sax, index->settings->paa_segments,
                 index->settings->sax_alphabet_cardinality,
                 index->settings->sax_bit_cardinality);

    root_mask_type root_mask = 0;
    CREATE_MASK(root_mask, index, sax);

    if ((&((parallel_first_buffer_layer*)(index->fbl))->soft_buffers[(int) root_mask])->initialized) {
        isax_node *node = (&((parallel_first_buffer_layer*)(index->fbl))->soft_buffers[(int) root_mask])->node;
        // Traverse tree

        // Adaptive splitting

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
        }
        result.distance = calculate_node_distance_inmemory(index, node, ts, FLT_MAX);
        result.node = node;
    }
    else {
        result.node = NULL;
        result.distance = FLT_MAX;
    }

    free(sax);

    return result;
}

float calculate_node_distance_inmemory (isax_index *index, isax_node *node, ts_type *query, float bsf) 
{
    COUNT_CHECKED_NODE()
    // If node has buffered data

    if (node->buffer != NULL) 
    {   
        int i;
        for (i=0; i<node->buffer->full_buffer_size; i++) 
        {
            float dist = ts_euclidean_distance(query, node->buffer->full_ts_buffer[i], 
                                               index->settings->timeseries_size, bsf);
            if (dist < bsf) {
                bsf = dist;
            }
        }

        for (i=0; i<node->buffer->tmp_full_buffer_size; i++) {
            float dist = ts_euclidean_distance(query, node->buffer->tmp_full_ts_buffer[i], 
                                               index->settings->timeseries_size, bsf);
            if (dist < bsf ) {
                bsf = dist;
            }
        }
       // RDcalculationnumber=RDcalculationnumber+node->buffer->partial_buffer_size;
        for (i=0; i<node->buffer->partial_buffer_size; i++) {

            float dist = ts_euclidean_distance_SIMD(query, &(rawfile[*node->buffer->partial_position_buffer[i]]), 
                                               index->settings->timeseries_size, bsf);

            if (dist < bsf) {
                bsf = dist;

            }
        }
    }
    
    return bsf;
}


float calculate_node_distance_inmemory_omp (isax_index *index, isax_node *node, ts_type *query, float bsf) 
{
    if (node->buffer != NULL) 
    {   
        int i;
        for (i=0; i<node->buffer->full_buffer_size; i++) 
        {
            float dist = ts_euclidean_distance(query, node->buffer->full_ts_buffer[i], 
                                               index->settings->timeseries_size, bsf);
            if (dist < bsf) {
                bsf = dist;
            }
        }

        for (i=0; i<node->buffer->tmp_full_buffer_size; i++) {
            float dist = ts_euclidean_distance(query, node->buffer->tmp_full_ts_buffer[i], 
                                               index->settings->timeseries_size, bsf);
            if (dist < bsf ) {
                bsf = dist;
            }
        }
        #pragma omp parallel for num_threads(maxquerythread) reduction(min : bsf)
        for (i=0; i<node->buffer->partial_buffer_size; i++) {
            float dist = ts_euclidean_distance_SIMD(query, &(rawfile[*node->buffer->partial_position_buffer[i]]), 
                                               index->settings->timeseries_size, bsf);

            if (dist < bsf) {
                bsf = dist;

            }
        }
    }
    
    return bsf;
}


float calculate_node_distance2_inmemory (isax_index *index, isax_node *node, ts_type *query,ts_type *paa, float bsf) 
{
    COUNT_CHECKED_NODE()
    float distmin;
    // If node has buffered data
    if (node->buffer != NULL) 
    {
        int i;
        //#pragma omp parallel for num_threads(2) reduction(min : bsf)
        
        //__sync_fetch_and_add(&LBDcalculationnumber,node->buffer->partial_buffer_size);
        for (i=0; i<node->buffer->partial_buffer_size; i++) {

            distmin=minidist_paa_to_isax_rawa_SIMD(paa, node->buffer->partial_sax_buffer[i],
                                               index->settings->max_sax_cardinalities,
                                               index->settings->sax_bit_cardinality,
                                               index->settings->sax_alphabet_cardinality,
                                               index->settings->paa_segments, MINVAL, MAXVAL,
                                               index->settings->mindist_sqrt);
            if (distmin<bsf)
            {
                float dist = ts_euclidean_distance_SIMD(query, &(rawfile[*node->buffer->partial_position_buffer[i]]), 
                                               index->settings->timeseries_size, bsf);
                //__sync_fetch_and_add(&RDcalculationnumber,1);
                if (dist < bsf) {
                    bsf = dist;
                }
            }
        }
    }
    return bsf;
}



query_result exact_search_serial_inmemory(ts_type *ts, ts_type *paa, isax_index *index, float minimum_distance, int min_checked_leaves) {
    checkts=0;
    RESET_BYTES_ACCESSED

    // FOR THREAD USE
    MINDISTS=malloc(sizeof(float) * index->sax_cache_size);
    unsigned long j;
    for (j = 0; j < index->sax_cache_size; j++)
        MINDISTS[j] = FLT_MAX;
    // END
    COUNT_INPUT_TIME_START
    query_result approximate_result = approximate_search_inmemory_pRecBuf(ts, paa, index);
    query_result bsf_result = approximate_result;

    int tight_bound = index->settings->tight_bound;
    int aggressive_check = index->settings->aggressive_check;
    
    // Early termination...
    if (approximate_result.distance == 0) {
        return approximate_result;
    }
    
    if(approximate_result.distance == FLT_MAX || min_checked_leaves > 1) {
        approximate_result = refine_answer_inmemory(ts, paa, index, approximate_result, minimum_distance, min_checked_leaves);
    }
    
    COUNT_INPUT_TIME_END
    
    
    unsigned long i;
    
    //FILE *raw_file = fopen(index->settings->raw_filename, "rb");
    //fseek(raw_file, 0, SEEK_SET);
    ts_type *ts_buffer;
    
    
    SET_APPROXIMATE(approximate_result.distance);

    // THREADED
    pthread_t thread[1];
    struct args_in arguments[1];
    COUNT_CAL_TIME_START
    for(i=0; i<1; i++) {
        arguments[i].i = i;
        arguments[i].from = i*(index->sax_cache_size / 1);
        arguments[i].bsf=approximate_result.distance;
        if(i < (1-1)) {
            arguments[i].to = (i+1)*(index->sax_cache_size / 1);
            
        }
        else {
            arguments[i].to = index->sax_cache_size;

        }
        arguments[i].paa = paa;
        arguments[i].index = index;
        int ret = pthread_create(&thread[i], NULL, compute_mindists_in, &arguments[i]);
    }
    
    for(i=0; i<1;i++) {
        pthread_join(thread[i], NULL);
    }
    // END
    COUNT_CAL_TIME_END
    //printf("the min distance 0 is %f\n",MINDISTS[0] );
    COUNT_OUTPUT_TIME_START
    for(i=0; i<index->sax_cache_size; i++) {
        sax_type *sax = &index->sax_cache[i * index->settings->paa_segments];
        if(MINDISTS[i] <= approximate_result.distance) {
            ts_buffer=&rawfile[i*index->settings->timeseries_size];
            checkts++;
            float dist = ts_euclidean_distance_SIMD(ts, ts_buffer, index->settings->timeseries_size, FLT_MAX);
            if(dist < approximate_result.distance) {
                approximate_result. distance = dist;

#ifdef STORE_ANSWER
                memcpy(index->answer, ts_buffer, index->settings->timeseries_size * sizeof(ts_type));
#endif
            }
        }
    }
    COUNT_OUTPUT_TIME_END
    free(MINDISTS);
    
    //FILE *pfile = fopen("hhhehehehf.bin", "a+");
        //float kkkkkk=approximate_result.distance;
        //fwrite(&kkkkkk, sizeof(float), 1, pfile);            
        //fclose(pfile);
            return approximate_result;
}
query_result exact_search_serial_1bsf_inmemory(ts_type *ts, ts_type *paa, isax_index *index, float minimum_distance, int min_checked_leaves,float bsf) {
    
    RESET_BYTES_ACCESSED

    // FOR THREAD USE
    MINDISTS=malloc(sizeof(float) * index->sax_cache_size);
    unsigned long j;
    unsigned long diskconter=0, bsfupdate=0;

    for (j = 0; j < index->sax_cache_size; j++)
        MINDISTS[j] = FLT_MAX;
    // END
    COUNT_INPUT_TIME_START
    query_result approximate_result;
    if (bsf==FLT_MAX)
    {
        approximate_result=approximate_search_inmemory(ts, paa, index);
        if (approximate_result.distance == 0) {
        return approximate_result;
    }
    if(approximate_result.distance == FLT_MAX || min_checked_leaves > 1) {
        approximate_result = refine_answer_inmemory(ts, paa, index, approximate_result, minimum_distance, min_checked_leaves);
    }
    }
    else
    {
        approximate_result.distance=bsf;
    }


    
    // Early termination...
    if (approximate_result.distance == 0) {
        return approximate_result;
    }
    

    
    COUNT_INPUT_TIME_END
    
    
    unsigned long i;
    
    //FILE *raw_file = fopen(index->settings->raw_filename, "rb");
    //fseek(raw_file, 0, SEEK_SET);
    ts_type *ts_buffer;
    
    
    SET_APPROXIMATE(approximate_result.distance);

    // THREADED
    pthread_t thread[maxquerythread];
    struct args_in arguments[maxquerythread];
    COUNT_CAL_TIME_START
    for(i=0; i<maxquerythread; i++) {
        arguments[i].i = i;
        arguments[i].from = i*(index->sax_cache_size / maxquerythread);
        if(i < (maxquerythread-1)) {
            arguments[i].to = (i+1)*(index->sax_cache_size / maxquerythread);
        }
        else {
            arguments[i].to = index->sax_cache_size;
        }
        arguments[i].paa = paa;
        arguments[i].index = index;
        int ret = pthread_create(&thread[i], NULL, compute_mindists_in, &arguments[i]);
    }
    
    for(i=0; i<maxquerythread;i++) {
        pthread_join(thread[i], NULL);
    }
    // END
    COUNT_CAL_TIME_END

    COUNT_OUTPUT_TIME_START
    for(i=0; i<index->sax_cache_size; i++) {
        sax_type *sax = &index->sax_cache[i * index->settings->paa_segments];
        if(MINDISTS[i] <= approximate_result.distance) {
            ts_buffer=&rawfile[i*index->settings->timeseries_size];
            diskconter++;
            float dist = ts_euclidean_distance(ts, ts_buffer, index->settings->timeseries_size, approximate_result.distance);
            if(dist < approximate_result.distance) {
                approximate_result. distance = dist;
                bsfupdate++;
                #ifdef STORE_ANSWER
                memcpy(index->answer, ts_buffer, index->settings->timeseries_size * sizeof(ts_type));
                #endif
            }
        }
    }
    COUNT_OUTPUT_TIME_END
    free(MINDISTS);
    //printf("the bsf update time is: %ld\n",bsfupdate);
    return approximate_result;
}


query_result refine_answer_inmemory (ts_type *ts, ts_type *paa, isax_index *index,
							query_result approximate_bsf_result,
                            float minimum_distance, int limit) 
{
	query_result bsf_result = approximate_bsf_result;

 	int tight_bound = index->settings->tight_bound;
	int aggressive_check = index->settings->aggressive_check;
    
    int j=0;
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
		            //split_node(index, n->node);
					//pqueue_insert(pq, n);
		            continue;
		        }
				// *** EXTRA BOUNDING ***
				if(tight_bound) {
                    j++;
					float mindistance = calculate_minimum_distance_inmemory(index, n->node, ts, paa);

					if(mindistance >= bsf_result.distance)
					{
						free(n);
						continue;
					}
				}
				// *** REAL DISTANCE ***
				checks++;
				float distance = calculate_node_distance_inmemory(index, n->node, ts, bsf_result.distance);
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
						float distance = calculate_node_distance_inmemory(index, n->node->left_child, ts, bsf_result.distance);
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
						float distance = calculate_node_distance_inmemory(index, n->node->right_child, ts, bsf_result.distance);
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




float calculate_minimum_distance_inmemory (isax_index *index, isax_node *node, ts_type *raw_query, ts_type *query) 
{
    //printf("Calculating minimum distance...\n");
    float bsfLeaf =   minidist_paa_to_isax(query, node->isax_values, 
                                                                     node->isax_cardinalities,
                                                                     index->settings->sax_bit_cardinality,  
                                                                     index->settings->sax_alphabet_cardinality, 
                                                                     index->settings->paa_segments, 
                                                                     MINVAL, MAXVAL,
                                                                     index->settings->mindist_sqrt);
    float bsfRecord = FLT_MAX;                                                               
    //printf("---> Distance: %lf\n", bsfLeaf);
    //sax_print(node->isax_values, 1,  index->settings->sax_bit_cardinality);

    if(!index->has_wedges) {
    //      printf("--------------\n");
        int i = 0;


        if (node->buffer != NULL) {
            for (i=0; i<node->buffer->partial_buffer_size; i++) {
                float mindist = minidist_paa_to_isax_raw_SIMD(query, node->buffer->partial_sax_buffer[i], index->settings->max_sax_cardinalities,
                                                     index->settings->sax_bit_cardinality,
                                                     index->settings->sax_alphabet_cardinality,
                                                     index->settings->paa_segments, MINVAL, MAXVAL,
                                                     index->settings->mindist_sqrt);
    //              printf("+[PARTIAL] %lf\n", mindist);
                if(mindist < bsfRecord) {
                    bsfRecord = mindist;
                }
            }

            for (i=0; i<node->buffer->tmp_partial_buffer_size; i++) {
                float mindist = minidist_paa_to_isax_raw_SIMD(query, node->buffer->tmp_partial_sax_buffer[i], index->settings->max_sax_cardinalities,
                                                     index->settings->sax_bit_cardinality,
                                                     index->settings->sax_alphabet_cardinality,
                                                     index->settings->paa_segments, MINVAL, MAXVAL,
                                                     index->settings->mindist_sqrt);
    //              printf("+[TMP_PARTIAL] %lf\n", mindist);
                if(mindist < bsfRecord) {
                    bsfRecord = mindist;
                }
            }
        }
    }
    else {
        int i=0;
        if(node->wedges[0] == FLT_MIN) {
            bsfRecord = FLT_MAX;
        }
        else {
            bsfRecord = 0;
            ts_type *min_wedge = &node->wedges[0];
            ts_type *max_wedge = &node->wedges[index->settings->timeseries_size];
            if(raw_query[i] > max_wedge[i]) {
                bsfRecord += (raw_query[i] - max_wedge[i]) * (raw_query[i] - max_wedge[i]);
            }
            else if(raw_query[i] < max_wedge[i] && raw_query[i] > min_wedge[i]) {
                //bound += 0;
            }
            else {
                bsfRecord += (min_wedge[i] - raw_query[i]) * (min_wedge[i] - raw_query[i]);
            }
            //bsfRecord = sqrtf(bsfRecord);
        }

    }
    float bsf = (bsfRecord == FLT_MAX) ? bsfLeaf : bsfRecord;
    //  printf("\t%.2lf - %d [%d] : %s.%s\n",bsfRecord, node->leaf_size, node->is_leaf, node->filename, node->has_full_data_file ? ".full" : ".part");

        
    //printf("---> Final: %lf\n", bsf);
    return  bsf;
}




query_result exact_search_inmemory (ts_type *ts, ts_type *paa, isax_index *index,
                           float minimum_distance, int min_checked_leaves) 
{
    query_result approximate_result = approximate_search_inmemory(ts, paa, index);
    query_result bsf_result = approximate_result;
    int tight_bound = index->settings->tight_bound;
    int aggressive_check = index->settings->aggressive_check;


    // Early termination...
    if (approximate_result.distance == 0) {
        return approximate_result;
    }
    if(approximate_result.distance == FLT_MAX || min_checked_leaves > 1) {
        approximate_result = refine_answer_inmemory(ts, paa, index, approximate_result, minimum_distance, min_checked_leaves);
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
        else 
        {
            // If it is a leaf, check its real distance.
            if (n->node->is_leaf) 
            {
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
         /*       if(tight_bound) {
                    COUNT_CAL_TIME_START
                    float mindistance = calculate_minimum_distance_inmemory(index, n->node, ts, paa);
                    COUNT_CAL_TIME_END
                    if(mindistance >= bsf_result.distance)
                    {
                        if(n != do_not_remove)//add
                            free(n);
                        continue;
                    }
                }*/
                // *** REAL DISTANCE ***
                checks++;

                COUNT_CAL_TIME_START
                float distance = calculate_node_distance_inmemory(index, n->node, ts, bsf_result.distance);
                COUNT_CAL_TIME_END
                if (distance < bsf_result.distance)
                {
                    bsf_result.distance = distance;
                    bsf_result.node = n->node;
                }
                //no check limit juge
            }
            else 
            {
                // If it is an intermediate node calculate mindist for children
                // and push them in the queue
                if (n->node->left_child->isax_cardinalities != NULL) {
                    if(n->node->left_child->is_leaf && !n->node->left_child->has_partial_data_file && aggressive_check){
                        COUNT_CAL_TIME_START
                        float distance = calculate_node_distance_inmemory(index, n->node->left_child, ts, bsf_result.distance);
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
                        float distance = calculate_node_distance_inmemory(index, n->node->right_child, ts, bsf_result.distance);
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

query_result exact_search_inmemory2 (ts_type *ts, ts_type *paa, isax_index *index,
                           float minimum_distance, int min_checked_leaves)
{
    query_result approximate_result = approximate_search_inmemory(ts, paa, index);
    query_result bsf_result = approximate_result;
    int tight_bound = index->settings->tight_bound;
    int aggressive_check = index->settings->aggressive_check;
    // Early termination...
    checkts=0;
    if (approximate_result.distance == 0) {
        return approximate_result;
    }
    if(approximate_result.distance == FLT_MAX || min_checked_leaves > 1) {
        approximate_result = refine_answer_inmemory(ts, paa, index, approximate_result, minimum_distance, min_checked_leaves);
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

        COUNT_QUEUE_TIME_START
        insert_tree_node(paa,current_root_node,index,bsf_result.distance,pq);
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
        else 
        {
            // If it is a leaf, check its real distance.
            if (n->node->is_leaf) 
            {
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
                    //COUNT_CAL_TIME_START
                    float mindistance = calculate_minimum_distance_inmemory(index, n->node, ts, paa);
                    //COUNT_CAL_TIME_END
                    if(mindistance >= bsf_result.distance)
                    {
                        if(n != do_not_remove)//add
                            free(n);
                        continue;
                    }
                }
                // *** REAL DISTANCE ***
                checks++;

                //COUNT_CAL_TIME_START
                //float distance = calculate_node_distance_inmemory(index, n->node, ts, bsf_result.distance);
                float distance = calculate_node_distance2_inmemory(index, n->node, ts,paa, bsf_result.distance);
                //COUNT_CAL_TIME_END
                if (distance < bsf_result.distance)
                {
                    //printf("node distance is %f\n",n->distance );
                    bsf_result.distance = distance;
                    bsf_result.node = n->node;
                }
                //no check limit juge
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
    printf("the check ts is %d\n",checkts );
    COUNT_QUEUE_TIME_START
    pqueue_free(pq);
    COUNT_QUEUE_TIME_END
    //PRINT_BYTES_ACCESSED
   return bsf_result;
}


void insert_tree_node(float *paa,isax_node *node,isax_index *index,float bsf,pqueue_t *pq)
{   
    //COUNT_CAL_TIME_START
    float distance =  minidist_paa_to_isax(paa, node->isax_values,
                                            node->isax_cardinalities,
                                            index->settings->sax_bit_cardinality,
                                            index->settings->sax_alphabet_cardinality,
                                            index->settings->paa_segments,
                                            MINVAL, MAXVAL,
                                            index->settings->mindist_sqrt);
    //COUNT_CAL_TIME_END

    if(distance < bsf)
    {
        if (node->is_leaf) 
        {   
            query_result * mindist_result = malloc(sizeof(query_result));
            mindist_result->node = node;
            mindist_result->distance=distance;
            pqueue_insert(pq, mindist_result);
            added_tree_node++;
        }
        else
        {   
            if (node->left_child->isax_cardinalities != NULL)
            {
                insert_tree_node(paa,node->left_child,index, bsf,pq);
            }
            if (node->right_child->isax_cardinalities != NULL)
            {
                insert_tree_node(paa,node->right_child,index,bsf,pq);
            }
        }
    }
}



