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
#include <sys/wait.h>

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

#define NTHREADS 4
 




query_result  approximate_search_inmemory_m (ts_type *ts, ts_type *paa, isax_index *index) 
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
        result.distance = calculate_node_distance_inmemory_m(index, node, ts, FLT_MAX);
        result.node = node;
    }
    else {
        result.node = NULL;
        result.distance = FLT_MAX;
    }

    free(sax);

    return result;
}

float calculate_node_distance_inmemory_m (isax_index *index, isax_node *node, ts_type *query, float bsf) 
{
    COUNT_CHECKED_NODE()

    // If node has buffered data
    if (node->buffer != NULL) 
    {
        int i;
        #pragma omp parallel for num_threads(maxquerythread) reduction(min : bsf)
        for (i=0; i<node->buffer->full_buffer_size; i++) {
            float dist = ts_euclidean_distance_SIMD(query, node->buffer->full_ts_buffer[i], 
                                               index->settings->timeseries_size, bsf);
            if (dist < bsf) {
                bsf = dist;
            }
        }
        #pragma omp parallel for num_threads(maxquerythread) reduction(min : bsf)
        for (i=0; i<node->buffer->tmp_full_buffer_size; i++) {
            float dist = ts_euclidean_distance_SIMD(query, node->buffer->tmp_full_ts_buffer[i], 
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
    
    //////////////////////////////////////
    
    return bsf;
}






query_result refine_answer_inmemory_m (ts_type *ts, ts_type *paa, isax_index *index,
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
                float distance = calculate_node_distance_inmemory_m(index, n->node, ts, bsf_result.distance);
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





query_result exact_search_serial_ParIS_nb_inmemory(ts_type *ts, ts_type *paa, isax_index *index, float minimum_distance, int min_checked_leaves) 
{

    RESET_BYTES_ACCESSED



    pthread_t threadid[maxquerythread];
    COUNT_INPUT_TIME_START
    query_result approximate_result = approximate_search_inmemory(ts, paa, index);
    ts_type *ts_buffer = malloc(index->settings->ts_byte_size);
    query_result bsf_result = approximate_result;
    int sum_of_lab=0;
    int tight_bound = index->settings->tight_bound;
    int aggressive_check = index->settings->aggressive_check;
    float bsf_distance;
    unsigned long j;
    
    // Early termination...
    if (approximate_result.distance == 0) {
        return approximate_result;
    }
    if(approximate_result.distance == FLT_MAX || min_checked_leaves > 1) {
        approximate_result = refine_answer_inmemory(ts, paa, index, approximate_result, minimum_distance, min_checked_leaves);
    }
    COUNT_INPUT_TIME_END
    /*BIT_ARRAY* bitarray = bit_array_create(index->sax_cache_size);*/

    unsigned long i;

    #ifdef AUTO_TUNE
    float *mindists = malloc(sizeof(float) * index->sax_cache_size);
    #endif
    
    SET_APPROXIMATE(approximate_result.distance);
    ParIS_LDCW_data *worker_data=malloc(sizeof(ParIS_LDCW_data)*(maxquerythread));
    pthread_rwlock_t lock_bsf=PTHREAD_RWLOCK_INITIALIZER;
    float bsfdistance=approximate_result.distance;
    COUNT_CAL_TIME_START
    for (i = 0; i < (maxquerythread-1); i++)
    { 
        worker_data[i].index=index;
        worker_data[i].lock_bsf=&lock_bsf;
        worker_data[i].start_number=i*(index->sax_cache_size/maxquerythread);
        worker_data[i].stop_number=(i+1)*(index->sax_cache_size/maxquerythread);
        worker_data[i].paa=paa;
        worker_data[i].ts=ts;
        worker_data[i].bsfdistance=approximate_result.distance;
        worker_data[i].sum_of_lab=0;
        worker_data[i].minidisvector=&bsfdistance;
        worker_data[i].rawfile=rawfile;
    }
        worker_data[maxquerythread-1].index=index;
        worker_data[maxquerythread-1].lock_bsf=&lock_bsf;
        worker_data[maxquerythread-1].start_number=(maxquerythread-1)*(index->sax_cache_size/maxquerythread);
        worker_data[maxquerythread-1].stop_number=index->sax_cache_size;
        worker_data[maxquerythread-1].paa=paa;
        worker_data[maxquerythread-1].ts=ts;
        worker_data[maxquerythread-1].bsfdistance=approximate_result.distance;
        worker_data[maxquerythread-1].sum_of_lab=0;
        worker_data[maxquerythread-1].minidisvector=&bsfdistance;
        worker_data[maxquerythread-1].rawfile=rawfile;
    for(i=0; i<maxquerythread; i++) {
        pthread_create(&(threadid[i]),NULL,ParIS_nb_worker_inmemory,(void*)&(worker_data[i]));

    }
    for (i = 0; i < maxquerythread; i++)
    {
        pthread_join(threadid[i],NULL);
        if(worker_data[i].bsfdistance<approximate_result.distance)
    approximate_result.distance=worker_data[i].bsfdistance;
    }


    COUNT_CAL_TIME_END
    free(worker_data);
    return approximate_result;
}
void exact_search_serial_ParIS_nb_batch_inmemory(ts_type *ts, ts_type *paa, isax_index *index, float minimum_distance, int min_checked_leaves,int batch_number) 
{

    RESET_BYTES_ACCESSED

    query_result approximate_result[batch_number];
    omp_lock_t bsflock[batch_number];

    
    ts_type *ts_buffer = malloc(index->settings->ts_byte_size);
    //query_result bsf_result = approximate_result;
    int sum_of_lab=0;
    int tight_bound = index->settings->tight_bound;
    int aggressive_check = index->settings->aggressive_check;
    float bsf_distance;
    COUNT_INPUT_TIME_START
    // Early termination...
    #pragma omp parallel for num_threads(maxquerythread)
    for (int i = 0; i < batch_number; i++)
    {
        omp_init_lock(&(bsflock[i]));
        approximate_result[i] = approximate_search_inmemory(&(ts[i*index->settings->timeseries_size]), &(paa[i*index->settings->paa_segments]), index);
        if(approximate_result[i].distance == FLT_MAX || min_checked_leaves > 1) {
            approximate_result[i] = refine_answer_inmemory(&(ts[i*index->settings->timeseries_size]), &(paa[i*index->settings->paa_segments]), index, approximate_result[i], minimum_distance, min_checked_leaves);
        }

    }  
    COUNT_INPUT_TIME_END
    
    /*BIT_ARRAY* bitarray = bit_array_create(index->sax_cache_size);*/
    COUNT_CAL_TIME_START
    #pragma omp parallel for num_threads(maxquerythread)
    for(unsigned long  j=0; j<index->sax_cache_size; j++) {
        sax_type *sax = &index->sax_cache[j * index->settings->paa_segments];
        for (int i = 0; i < batch_number; i++)
        if(minidist_paa_to_isax_raw_SIMD(&(paa[i*index->settings->paa_segments]), sax, index->settings->max_sax_cardinalities,
                                                         index->settings->sax_bit_cardinality,
                                                         index->settings->sax_alphabet_cardinality,
                                                         index->settings->paa_segments, MINVAL, MAXVAL,
                                                         index->settings->mindist_sqrt) <= approximate_result[i].distance) {
            ts_buffer=&rawfile[j*index->settings->timeseries_size];
            float dist = ts_euclidean_distance_SIMD(&(ts[i*index->settings->timeseries_size]), ts_buffer, index->settings->timeseries_size, approximate_result[i].distance);
            if(dist < approximate_result[i].distance) {
                omp_set_lock(&(bsflock[i]));
                if(dist < approximate_result[i].distance) {
                approximate_result[i]. distance = dist;
            }
            omp_unset_lock(&(bsflock[i]));
            }
        }
    }
    COUNT_CAL_TIME_END
    //SET_APPROXIMATE(approximate_result.distance);
    for (int i = 0; i < batch_number; i++)
    {
        PRINT_STATS(approximate_result[i].distance)
    }
}

void* ParIS_nb_worker_inmemory(void *worker_data)
{    
    
    unsigned long i;
    float bsfdistance,mindist;

    isax_index *index=((ParIS_LDCW_data*)worker_data)->index;
    ts_type *ts_buffer;
    unsigned long start_number=((ParIS_LDCW_data*)worker_data)->start_number;
    unsigned long stop_number=((ParIS_LDCW_data*)worker_data)->stop_number;
    ts_type *paa=((ParIS_LDCW_data*)worker_data)->paa;
    ts_type *ts=((ParIS_LDCW_data*)worker_data)->ts;
    
    for(i=start_number;i<stop_number;i++)
    {

        sax_type *sax = &index->sax_cache[i * index->settings->paa_segments];

        mindist = minidist_paa_to_isax_raw_SIMD(paa, sax, index->settings->max_sax_cardinalities,
                                                         index->settings->sax_bit_cardinality,
                                                         index->settings->sax_alphabet_cardinality,
                                                         index->settings->paa_segments, MINVAL, MAXVAL,
                                                         index->settings->mindist_sqrt);
     
        if(mindist <= ((ParIS_LDCW_data*)worker_data)->bsfdistance) {
            /*bit_array_set_bit(bitarray, i);*/
            //COUNT_INPUT_TIME_START

            ts_buffer= &((ParIS_LDCW_data*)worker_data)->rawfile[i*index->settings->ts_byte_size/sizeof(ts_type)];
            COUNT_INPUT_TIME_END
  
            float dist = ts_euclidean_distance_SIMD(ts, ts_buffer, index->settings->timeseries_size, ((ParIS_LDCW_data*)worker_data)->bsfdistance);
            
            if(dist<(((ParIS_LDCW_data*)worker_data)->bsfdistance))
            {
                (((ParIS_LDCW_data*)worker_data)->bsfdistance)=dist;
            }
        }
    }
}



query_result exact_search_parads_inmemory (ts_type *ts, ts_type *paa, isax_index *index,
                           float minimum_distance, int min_checked_leaves) 
{
    query_result approximate_result = approximate_search_inmemory_pRecBuf(ts, paa, index);
    query_result bsf_result = approximate_result;
    int tight_bound = index->settings->tight_bound;
    int aggressive_check = index->settings->aggressive_check;
    int i;
    // Early termination...
    if (approximate_result.distance == 0) {
        return approximate_result;
    }
    if(approximate_result.distance == FLT_MAX || min_checked_leaves > 1) {
        approximate_result = refine_answer_inmemory_m(ts, paa, index, approximate_result, minimum_distance, min_checked_leaves);
    }

    pqueue_t *pq = pqueue_init(index->settings->root_nodes_size,
                               cmp_pri, get_pri, set_pri, get_pos, set_pos);



    query_result *do_not_remove = &approximate_result;

    SET_APPROXIMATE(approximate_result.distance);

    RESET_BYTES_ACCESSED

    if(approximate_result.node != NULL) {
        // Insert approximate result in heap.
        pqueue_insert(pq, &approximate_result);
        //GOOD: if(approximate_result.node->filename != NULL)
        //GOOD: printf("POPS: %.5lf\t", approximate_result.distance);
    }

    // Insert all root nodes in heap.
    isax_node *current_root_node = index->first_node;

    

        pthread_t threadid[maxquerythread];

    refind_answer_fonction_data rfdata;
    pthread_mutex_t lock_queue=PTHREAD_MUTEX_INITIALIZER,lock_current_root_node=PTHREAD_MUTEX_INITIALIZER;
    pthread_rwlock_t lock_bsf=PTHREAD_RWLOCK_INITIALIZER;
    pthread_barrier_t lock_barrier;
    pthread_barrier_init(&lock_barrier, NULL, 1);
    rfdata.paa=paa;
    rfdata.ts=ts;
    
    rfdata.lock_queue=&lock_queue;
    rfdata.lock_current_root_node=&lock_current_root_node;
    rfdata.lock_bsf=&lock_bsf;
    
    rfdata.index=index;
    rfdata.minimum_distance=minimum_distance;
    //rfdata.limit=limit/4;

    rfdata.pq=pq;
    // Insert all root nodes in heap.
    rfdata.current_root_node = index->first_node;
    rfdata.bsf_result = &bsf_result;
   
    query_result * n;
    
    rfdata.lock_barrier=&lock_barrier;
    for (i = 0; i < maxquerythread; i++)
    {
        pthread_create(&(threadid[i]),NULL,exact_search_old_worker_inmemory,(void*)&(rfdata));
    }
    for (i = 0; i < maxquerythread; i++)
    {
        pthread_join(threadid[i],NULL);
    }


    // Free the nodes that where not popped.
    while ((n = pqueue_pop(pq)))
    {
        if(n != do_not_remove)
            free(n);
    }
    // Free the priority queue.
    pthread_barrier_destroy(&lock_barrier);
    pqueue_free(pq);
    
    bsf_result=*(rfdata.bsf_result);
    //free(rfdata);
    return *(rfdata.bsf_result);

    // Free the nodes that where not popped.

}
void* exact_search_old_worker_inmemory(void *rfdata)
{
    isax_node *current_root_node;
    query_result *n;
    isax_index *index=((refind_answer_fonction_data*)rfdata)->index;
    ts_type *paa=((refind_answer_fonction_data*)rfdata)->paa;
    ts_type *ts=((refind_answer_fonction_data*)rfdata)->ts;
    pqueue_t *pq=((refind_answer_fonction_data*)rfdata)->pq;
    query_result *do_not_remove = ((refind_answer_fonction_data*)rfdata)->bsf_result;
    float minimum_distance=((refind_answer_fonction_data*)rfdata)->minimum_distance;
    float bsfdisntance;
    int limit=((refind_answer_fonction_data*)rfdata)->limit;
    int checks = 0;
    int tight_bound = index->settings->tight_bound;
    int aggressive_check = index->settings->aggressive_check;
    query_result *bsf_result=(((refind_answer_fonction_data*)rfdata)->bsf_result);
    while (1) 
    {
        pthread_mutex_lock(((refind_answer_fonction_data*)rfdata)->lock_current_root_node);
        current_root_node= ((refind_answer_fonction_data*)rfdata)->current_root_node;
        if (current_root_node != NULL)
        {
            ((refind_answer_fonction_data*)rfdata)->current_root_node=((refind_answer_fonction_data*)rfdata)->current_root_node->next;
            pthread_mutex_unlock(((refind_answer_fonction_data*)rfdata)->lock_current_root_node);
        }
        else    
        {
            pthread_mutex_unlock(((refind_answer_fonction_data*)rfdata)->lock_current_root_node);
            break;
        }
        
        

        query_result * mindist_result = malloc(sizeof(query_result));
        mindist_result->distance =  minidist_paa_to_isax(paa, current_root_node->isax_values,
                                              current_root_node->isax_cardinalities,
                                              index->settings->sax_bit_cardinality,
                                              index->settings->sax_alphabet_cardinality,
                                              index->settings->paa_segments,
                                              MINVAL, MAXVAL,
                                              index->settings->mindist_sqrt);

        mindist_result->node = current_root_node;
        pthread_mutex_lock(((refind_answer_fonction_data*)rfdata)->lock_queue);

        pqueue_insert(pq, mindist_result);

        pthread_mutex_unlock(((refind_answer_fonction_data*)rfdata)->lock_queue);
    }
    //printf("this is the check point 1 !!!!!!!!!!!!!!!!!!!\n");
    pthread_barrier_wait(((refind_answer_fonction_data*)rfdata)->lock_barrier);

 while (1)
    {


        pthread_mutex_lock(((refind_answer_fonction_data*)rfdata)->lock_queue);
        n = pqueue_pop(pq);
        pthread_mutex_unlock(((refind_answer_fonction_data*)rfdata)->lock_queue);
        if(n==NULL)
            break;
        //printf("this is the check point of e s !!!\n");
        pthread_rwlock_rdlock(((refind_answer_fonction_data*)rfdata)->lock_bsf);
        bsfdisntance=bsf_result->distance;
        pthread_rwlock_unlock(((refind_answer_fonction_data*)rfdata)->lock_bsf);
        // The best node has a worse mindist, so search is finished!
        if (n->distance >= bsfdisntance || n->distance > minimum_distance) {
            pthread_mutex_lock(((refind_answer_fonction_data*)rfdata)->lock_queue);
            pqueue_insert(pq, n);
            pthread_mutex_unlock(((refind_answer_fonction_data*)rfdata)->lock_queue);
            break;
        }
        else 
        {
            // If it is a leaf, check its real distance.
            if (n->node->is_leaf) {
                // *** ADAPTIVE SPLITTING ***
                if (!n->node->has_full_data_file &&
                    (n->node->leaf_size > index->settings->min_leaf_size))
                {
                    // Split and push again in the queue
                    split_node(index, n->node);
                    pthread_mutex_lock(((refind_answer_fonction_data*)rfdata)->lock_queue);
                    pqueue_insert(pq, n);
                    pthread_mutex_unlock(((refind_answer_fonction_data*)rfdata)->lock_queue);
                    continue;
                }
                // *** EXTRA BOUNDING ***
                if(tight_bound) {
                    float mindistance = calculate_minimum_distance_inmemory(index, n->node, ts, paa);
                    if(mindistance >= bsfdisntance)
                    {
                        if(n != do_not_remove)
                            free(n);
                        continue;
                    }
                }
                // *** REAL DISTANCE ***
                checks++;
                float distance = calculate_node_distance_inmemory(index, n->node, ts, bsfdisntance);
                if (distance < bsfdisntance)
                {
                    pthread_rwlock_wrlock(((refind_answer_fonction_data*)rfdata)->lock_bsf);
                    if(distance < bsf_result->distance)
                    {
                        bsf_result->distance = distance;
                        bsf_result->node = n->node;
                    }

                    pthread_rwlock_unlock(((refind_answer_fonction_data*)rfdata)->lock_bsf);
                }
                //if(checks > limit) {
                //    pthread_mutex_lock(((refind_answer_fonction_data*)rfdata)->lock_queue);
                //    pqueue_insert(pq, n);
                //    pthread_mutex_unlock(((refind_answer_fonction_data*)rfdata)->lock_queue);
               //     break;
               // }
            }
            else {
                // If it is an intermediate node calculate mindist for children
                // and push them in the queue
                if (n->node->left_child->isax_cardinalities != NULL) {
                    if(n->node->left_child->is_leaf && !n->node->left_child->has_partial_data_file && aggressive_check){
                        float distance = calculate_node_distance_inmemory(index, n->node, ts, bsfdisntance);
                        if (distance < bsfdisntance)
                        {
                            pthread_rwlock_wrlock(((refind_answer_fonction_data*)rfdata)->lock_bsf);
                            if(distance <bsf_result->distance)
                            {
                                bsf_result->distance = distance;
                                bsf_result->node = n->node->left_child;
                            }
                            pthread_rwlock_unlock(((refind_answer_fonction_data*)rfdata)->lock_bsf);
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
                    pthread_mutex_lock(((refind_answer_fonction_data*)rfdata)->lock_queue);
                    pqueue_insert(pq, mindist_result);
                    pthread_mutex_unlock(((refind_answer_fonction_data*)rfdata)->lock_queue);
                    }
                }
                if (n->node->right_child->isax_cardinalities != NULL) {
                    if(n->node->right_child->is_leaf && !n->node->left_child->has_partial_data_file && aggressive_check){
                        float distance = calculate_node_distance_inmemory(index, n->node, ts, bsfdisntance);
                        if (distance < bsfdisntance)
                        {
                            pthread_rwlock_wrlock(((refind_answer_fonction_data*)rfdata)->lock_bsf);
                            if(distance <bsf_result->distance)
                            {
                                bsf_result->distance = distance;
                                bsf_result->node = n->node->right_child;
                            }
                            pthread_rwlock_unlock(((refind_answer_fonction_data*)rfdata)->lock_bsf);
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
                    pthread_mutex_lock(((refind_answer_fonction_data*)rfdata)->lock_queue);
                    pqueue_insert(pq, mindist_result);
                    pthread_mutex_unlock(((refind_answer_fonction_data*)rfdata)->lock_queue);
                    }
                }
            }

            // Free the node currently popped.
            if(n != do_not_remove)
                free(n);
        }
    }
}

query_result exact_search_serial_ParIS_inmemory(ts_type *ts, ts_type *paa, isax_index *index, float minimum_distance, int min_checked_leaves) 
{

    RESET_BYTES_ACCESSED


    pthread_t threadid[maxquerythread];
    COUNT_INPUT_TIME_START
    query_result approximate_result = approximate_search_inmemory_pRecBuf(ts, paa, index);
    ts_type *ts_buffer = malloc(index->settings->ts_byte_size);
    query_result bsf_result = approximate_result;

    int sum_of_lab=0;
    int tight_bound = index->settings->tight_bound;
    int aggressive_check = index->settings->aggressive_check;
    float bsf_distance;
    unsigned long j;

    // Early termination...
    if (approximate_result.distance == 0) {
        return approximate_result;
    }
    
    if(approximate_result.distance == FLT_MAX || min_checked_leaves > 1) {
        approximate_result = refine_answer_inmemory(ts, paa, index, approximate_result, minimum_distance, min_checked_leaves);
        //approximate_result = refine_answer_m(ts, paa, index, approximate_result2, minimum_distance, min_checked_leaves);
    }
    
    COUNT_INPUT_TIME_END
    /*BIT_ARRAY* bitarray = bit_array_create(index->sax_cache_size);*/
    //printf("the old distance is: %f \n",approximate_result.distance);
    unsigned long i;

    #ifdef AUTO_TUNE
    float *mindists = malloc(sizeof(float) * index->sax_cache_size);
    #endif
    
    SET_APPROXIMATE(approximate_result.distance);
    //printf("the index->sax_cache_size is %ld\n",index->sax_cache_size);
    ParIS_LDCW_data *essdata=malloc(sizeof(ParIS_LDCW_data)*(maxquerythread));
    pthread_rwlock_t lock_bsf=PTHREAD_RWLOCK_INITIALIZER;
    //pthread_mutex_t lock_bsf2=PTHREAD_MUTEX_INITIALIZER;
    COUNT_CAL_TIME_START
    for (i = 0; i < (maxquerythread-1); i++)
    {
        essdata[i].index=index;
        essdata[i].lock_bsf=&lock_bsf;
        essdata[i].start_number=i*(index->sax_cache_size/maxquerythread);
        essdata[i].stop_number=(i+1)*(index->sax_cache_size/maxquerythread);
        essdata[i].paa=paa;
        essdata[i].ts=ts;
        essdata[i].bsfdistance=approximate_result.distance;
        essdata[i].sum_of_lab=0;
    }
    essdata[maxquerythread-1].index=index;
    essdata[maxquerythread-1].lock_bsf=&lock_bsf;
    essdata[maxquerythread-1].start_number=(maxquerythread-1)*(index->sax_cache_size/maxquerythread);
    essdata[maxquerythread-1].stop_number=index->sax_cache_size;
    essdata[maxquerythread-1].paa=paa;
    essdata[maxquerythread-1].ts=ts;
    essdata[maxquerythread-1].bsfdistance=approximate_result.distance;
    essdata[maxquerythread-1].sum_of_lab=0;

    for(i=0; i<maxquerythread; i++) 
    {
        pthread_create(&(threadid[i]),NULL,mindistance_worker_inmemory,(void*)&(essdata[i]));
    }
    for (i = 0; i < maxquerythread; i++)
    {
        pthread_join(threadid[i],NULL);
        sum_of_lab+=essdata[i].sum_of_lab;
    }
    COUNT_CAL_TIME_END
    unsigned long* label_number=malloc(sizeof(unsigned long)*(sum_of_lab));
    float* minidisvector=malloc(sizeof(float)*(sum_of_lab));
   
    sum_of_lab=0;
    COUNT_OUTPUT_TIME_START
    for (i = 0; i < maxquerythread; i++)
    {
        memcpy(&(label_number[sum_of_lab]),essdata[i].label_number,sizeof(unsigned long)*essdata[i].sum_of_lab);
        memcpy(&(minidisvector[sum_of_lab]),essdata[i].minidisvector,sizeof(float)*essdata[i].sum_of_lab);
        free(essdata[i].label_number);
        free(essdata[i].minidisvector);
        sum_of_lab+=essdata[i].sum_of_lab;

    }
    pthread_t readthread[maxquerythread*maxreadthread];
    ParIS_read_worker_data readpointer;
    unsigned long readcounter=0;
    float bsfdistance=(approximate_result.distance);

        
    readpointer.ts=ts;
    readpointer.index=index;
    readpointer.counter=&readcounter;
    readpointer.bsf=approximate_result.distance;
    readpointer.load_point=label_number;
    readpointer.lock_bsf=&lock_bsf;
    readpointer.bsf2=&bsfdistance;
    readpointer.minidisvector=minidisvector;
    readpointer.sum_of_lab=sum_of_lab;
    readpointer.rawfile=rawfile;
   
    for (i = 0; i < maxquerythread*maxreadthread; i++)
    {   
        pthread_create(&(readthread[i]),NULL,readworker_inmemory,(void*)&(readpointer));
    }

    for (i = 0; i < maxquerythread*maxreadthread; i++)
    {   
        pthread_join(readthread[i],NULL);
    }
    COUNT_OUTPUT_TIME_END
    approximate_result.distance=bsfdistance;
    free(essdata);
    free(minidisvector);
    free(label_number);
    //printf("I need to check: %2.2lf%% of the data.\n", (double)tocheck*100/(double)index->sax_cache_size);
    /*bit_array_free(bitarray);*/
            //printf("the new distance is: %f \n",approximate_result.distance);
                //.sax_type *sax = &index->sax_cache[1 * index->settings->paa_segments];
    return approximate_result;
}
pqueue_bsf exact_topk_serial_ParIS_inmemory(ts_type *ts, ts_type *paa, isax_index *index, float minimum_distance, int min_checked_leaves, int k) 
{

    RESET_BYTES_ACCESSED

    pqueue_bsf *pq_bsf= pqueue_bsf_init(k);
    pthread_t threadid[maxquerythread];
    COUNT_INPUT_TIME_START
    approximate_topk_inmemory(ts, paa, index,pq_bsf);

    ts_type *ts_buffer = malloc(index->settings->ts_byte_size);

    int sum_of_lab=0;
    int tight_bound = index->settings->tight_bound;
    int aggressive_check = index->settings->aggressive_check;

    unsigned long j;

    // Early termination...

    
    if(pq_bsf->knn[k-1] == FLT_MAX || min_checked_leaves > 1) {
        refine_topk_answer_inmemory(ts, paa, index, pq_bsf, minimum_distance, min_checked_leaves);
    }
    
    COUNT_INPUT_TIME_END
    /*BIT_ARRAY* bitarray = bit_array_create(index->sax_cache_size);*/
    //printf("the old distance is: %f \n",approximate_result.distance);
    unsigned long i;

    #ifdef AUTO_TUNE
    float *mindists = malloc(sizeof(float) * index->sax_cache_size);
    #endif
    
    SET_APPROXIMATE(pq_bsf->knn[k-1]);
    //printf("the index->sax_cache_size is %ld\n",index->sax_cache_size);
    ParIS_LDCW_data *essdata=malloc(sizeof(ParIS_LDCW_data)*(maxquerythread));
    pthread_rwlock_t lock_bsf=PTHREAD_RWLOCK_INITIALIZER;
    //pthread_mutex_t lock_bsf2=PTHREAD_MUTEX_INITIALIZER;
    COUNT_CAL_TIME_START
    for (i = 0; i < (maxquerythread-1); i++)
    {
        essdata[i].index=index;
        essdata[i].lock_bsf=&lock_bsf;
        essdata[i].start_number=i*(index->sax_cache_size/maxquerythread);
        essdata[i].stop_number=(i+1)*(index->sax_cache_size/maxquerythread);
        essdata[i].paa=paa;
        essdata[i].ts=ts;
        essdata[i].bsfdistance=pq_bsf->knn[k-1];
        essdata[i].sum_of_lab=0;
    }
    essdata[maxquerythread-1].index=index;
    essdata[maxquerythread-1].lock_bsf=&lock_bsf;
    essdata[maxquerythread-1].start_number=(maxquerythread-1)*(index->sax_cache_size/maxquerythread);
    essdata[maxquerythread-1].stop_number=index->sax_cache_size;
    essdata[maxquerythread-1].paa=paa;
    essdata[maxquerythread-1].ts=ts;
    essdata[maxquerythread-1].bsfdistance=pq_bsf->knn[k-1];
    essdata[maxquerythread-1].sum_of_lab=0;

    for(i=0; i<maxquerythread; i++) 
    {
        pthread_create(&(threadid[i]),NULL,mindistance_worker_inmemory,(void*)&(essdata[i]));
    }
    for (i = 0; i < maxquerythread; i++)
    {
        pthread_join(threadid[i],NULL);
        sum_of_lab+=essdata[i].sum_of_lab;
    }
    COUNT_CAL_TIME_END
    unsigned long* label_number=malloc(sizeof(unsigned long)*(sum_of_lab));
    float* minidisvector=malloc(sizeof(float)*(sum_of_lab));
   
    sum_of_lab=0;
    COUNT_OUTPUT_TIME_START
    for (i = 0; i < maxquerythread; i++)
    {
        memcpy(&(label_number[sum_of_lab]),essdata[i].label_number,sizeof(unsigned long)*essdata[i].sum_of_lab);
        memcpy(&(minidisvector[sum_of_lab]),essdata[i].minidisvector,sizeof(float)*essdata[i].sum_of_lab);
        free(essdata[i].label_number);
        free(essdata[i].minidisvector);
        sum_of_lab+=essdata[i].sum_of_lab;

    }
    pthread_t readthread[maxquerythread*maxreadthread];
    ParIS_read_worker_data readpointer;
    unsigned long readcounter=0;
        
    readpointer.ts=ts;
    readpointer.index=index;
    readpointer.counter=&readcounter;
    readpointer.load_point=label_number;
    readpointer.lock_bsf=&lock_bsf;
    readpointer.minidisvector=minidisvector;
    readpointer.sum_of_lab=sum_of_lab;
    readpointer.rawfile=rawfile;
    readpointer.pq_bsf=pq_bsf;    


    for (i = 0; i < maxquerythread*maxreadthread; i++)
    {   
        pthread_create(&(readthread[i]),NULL,topk_readworker_inmemory,(void*)&(readpointer));
    }

    for (i = 0; i < maxquerythread*maxreadthread; i++)
    {   
        pthread_join(readthread[i],NULL);
    }
    COUNT_OUTPUT_TIME_END
    free(essdata);
    free(minidisvector);
    free(label_number);
    //printf("I need to check: %2.2lf%% of the data.\n", (double)tocheck*100/(double)index->sax_cache_size);
    /*bit_array_free(bitarray);*/
            //printf("the new distance is: %f \n",approximate_result.distance);
                //.sax_type *sax = &index->sax_cache[1 * index->settings->paa_segments];
    return *pq_bsf;
}



query_result exact_search_serial_ParIS2_inmemory(ts_type *ts, ts_type *paa, isax_index *index, float minimum_distance, int min_checked_leaves) 
{

    RESET_BYTES_ACCESSED


    pthread_t threadid[maxquerythread];
    query_result approximate_result = approximate_search_inmemory(ts, paa, index);
    ts_type *ts_buffer = malloc(index->settings->ts_byte_size);
    query_result bsf_result = approximate_result;

    int sum_of_lab=0;
    int tight_bound = index->settings->tight_bound;
    int aggressive_check = index->settings->aggressive_check;
    float bsf_distance;
    unsigned long j;

    // Early termination...
    if (approximate_result.distance == 0) {
        return approximate_result;
    }
    
    if(approximate_result.distance == FLT_MAX || min_checked_leaves > 1) {
        approximate_result = refine_answer_inmemory(ts, paa, index, approximate_result, minimum_distance, min_checked_leaves);
        //approximate_result = refine_answer_m(ts, paa, index, approximate_result2, minimum_distance, min_checked_leaves);
    }
    
    
    /*BIT_ARRAY* bitarray = bit_array_create(index->sax_cache_size);*/
    //printf("the old distance is: %f \n",approximate_result.distance);
    unsigned long i;

    #ifdef AUTO_TUNE
    float *mindists = malloc(sizeof(float) * index->sax_cache_size);
    #endif
    
    SET_APPROXIMATE(approximate_result.distance);
    //printf("the index->sax_cache_size is %ld\n",index->sax_cache_size);
    ParIS_LDCW_data *essdata=malloc(sizeof(ParIS_LDCW_data)*(maxquerythread));
    pthread_rwlock_t lock_bsf=PTHREAD_RWLOCK_INITIALIZER;
    //pthread_mutex_t lock_bsf2=PTHREAD_MUTEX_INITIALIZER;
    for (i = 0; i < (maxquerythread-1); i++)
    {
        essdata[i].index=index;
        essdata[i].lock_bsf=&lock_bsf;
        essdata[i].start_number=i*(index->sax_cache_size/maxquerythread);
        essdata[i].stop_number=(i+1)*(index->sax_cache_size/maxquerythread);
        essdata[i].paa=paa;
        essdata[i].ts=ts;
        essdata[i].bsfdistance=approximate_result.distance;
        essdata[i].sum_of_lab=0;
    }
    essdata[maxquerythread-1].index=index;
    essdata[maxquerythread-1].lock_bsf=&lock_bsf;
    essdata[maxquerythread-1].start_number=(maxquerythread-1)*(index->sax_cache_size/maxquerythread);
    essdata[maxquerythread-1].stop_number=index->sax_cache_size;
    essdata[maxquerythread-1].paa=paa;
    essdata[maxquerythread-1].ts=ts;
    essdata[maxquerythread-1].bsfdistance=approximate_result.distance;
    essdata[maxquerythread-1].sum_of_lab=0;

    for(i=0; i<maxquerythread; i++) 
    {
        pthread_create(&(threadid[i]),NULL,mindistance_worker_inmemory,(void*)&(essdata[i]));
    }
    for (i = 0; i < maxquerythread; i++)
    {
        pthread_join(threadid[i],NULL);
        sum_of_lab+=essdata[i].sum_of_lab;
    }
    unsigned long* label_number=malloc(sizeof(unsigned long)*(sum_of_lab));
    float* minidisvector=malloc(sizeof(float)*(sum_of_lab));
   
    sum_of_lab=0;

    for (i = 0; i < maxquerythread; i++)
    {
        memcpy(&(label_number[sum_of_lab]),essdata[i].label_number,sizeof(unsigned long)*essdata[i].sum_of_lab);
        memcpy(&(minidisvector[sum_of_lab]),essdata[i].minidisvector,sizeof(float)*essdata[i].sum_of_lab);
        free(essdata[i].label_number);
        free(essdata[i].minidisvector);
        sum_of_lab+=essdata[i].sum_of_lab;

    }
    pthread_t readthread[maxquerythread*maxreadthread];
    ParIS_read_worker_data readpointer;
    unsigned long readcounter;
    float bsfdistance=(approximate_result.distance);

    if(bf==0)
    {
        readcounter=0;
    }
    else
    {
        readcounter=sum_of_lab-1;
    }
    readpointer.ts=ts;
    readpointer.index=index;
    readpointer.counter=&readcounter;
    readpointer.bsf=approximate_result.distance;
    readpointer.load_point=label_number;
    readpointer.lock_bsf=&lock_bsf;
    readpointer.bsf2=&bsfdistance;
    readpointer.minidisvector=minidisvector;
    readpointer.sum_of_lab=sum_of_lab;
    readpointer.rawfile=rawfile;
    for (i = 0; i < maxquerythread*maxreadthread; i++)
    {   
        if (bf==0)
        {
            pthread_create(&(readthread[i]),NULL,readworker_inmemory,(void*)&(readpointer));
        }
        else
        {
            pthread_create(&(readthread[i]),NULL,readworker2_inmemory,(void*)&(readpointer));
        }
        
    }

    for (i = 0; i < maxquerythread*maxreadthread; i++)
    {   
        pthread_join(readthread[i],NULL);
    }
    approximate_result.distance=bsfdistance;
    if (bf==0)
    {
        bf=1;
    }
    else
    {
        bf=0;
    }
    free(essdata);
    free(minidisvector);
    free(label_number);
    //printf("the t is \n");
    //printf("I need to check: %2.2lf%% of the data.\n", (double)tocheck*100/(double)index->sax_cache_size);
    /*bit_array_free(bitarray);*/
            //printf("the new distance is: %f \n",approximate_result.distance);
                //.sax_type *sax = &index->sax_cache[1 * index->settings->paa_segments];
    return approximate_result;
}
void* mindistance_worker_inmemory(void *essdata)
{    
    
    unsigned long i;
    float bsfdistance,mindist;
    isax_index *index=((ParIS_LDCW_data*)essdata)->index;
    unsigned long start_number=((ParIS_LDCW_data*)essdata)->start_number;
    unsigned long stop_number=((ParIS_LDCW_data*)essdata)->stop_number;
    ts_type *paa=((ParIS_LDCW_data*)essdata)->paa;
    ts_type *ts=((ParIS_LDCW_data*)essdata)->ts;
    ((ParIS_LDCW_data*)essdata)->label_number=malloc(sizeof(unsigned long)*10000);
    ((ParIS_LDCW_data*)essdata)->minidisvector=malloc(sizeof(float)*10000);

    unsigned long max_number=10000;

    for(i=start_number;i<stop_number;i++)
    {

        sax_type *sax = &index->sax_cache[i * index->settings->paa_segments];

        mindist = minidist_paa_to_isax_rawa_SIMD(paa, sax, index->settings->max_sax_cardinalities,
                                                         index->settings->sax_bit_cardinality,
                                                         index->settings->sax_alphabet_cardinality,
                                                         index->settings->paa_segments, MINVAL, MAXVAL,
                                                         index->settings->mindist_sqrt);
        if(mindist <= ((ParIS_LDCW_data*)essdata)->bsfdistance) 
        {
            if ( ((ParIS_LDCW_data*)essdata)->sum_of_lab>=max_number)
            {
                max_number=(max_number+10000);
                ((ParIS_LDCW_data*)essdata)->label_number = (unsigned long*) realloc(((ParIS_LDCW_data*)essdata)->label_number, sizeof(unsigned long)*max_number);
                ((ParIS_LDCW_data*)essdata)->minidisvector = (float*) realloc(((ParIS_LDCW_data*)essdata)->minidisvector, sizeof(float)*max_number);
                
            }
            ((ParIS_LDCW_data*)essdata)->label_number[((ParIS_LDCW_data*)essdata)->sum_of_lab]=i;
            ((ParIS_LDCW_data*)essdata)->minidisvector[((ParIS_LDCW_data*)essdata)->sum_of_lab]=mindist;
            ((ParIS_LDCW_data*)essdata)->sum_of_lab++;
        }
    }
}

void* readworker_inmemory(void *read_pointer)
{
    isax_index *index=((ParIS_read_worker_data*)read_pointer)->index;
    float bsf,dist;
    ts_type *ts_buffer;
    ts_type *ts =((ParIS_read_worker_data*)read_pointer)->ts;
    unsigned long t=0,p;
    unsigned long sum_of_lab=((ParIS_read_worker_data*)read_pointer)->sum_of_lab;
    float *minidisvector=((ParIS_read_worker_data*)read_pointer)->minidisvector;
    bsf= *(((ParIS_read_worker_data*)read_pointer)->bsf2); 
    
    while(1)
    { 
        pthread_rwlock_rdlock(((ParIS_read_worker_data*)read_pointer)->lock_bsf); 
        bsf= *(((ParIS_read_worker_data*)read_pointer)->bsf2); 
        pthread_rwlock_unlock(((ParIS_read_worker_data*)read_pointer)->lock_bsf); 
        t=__sync_fetch_and_add(((ParIS_read_worker_data*)read_pointer)->counter,1);
        if (t>=sum_of_lab) 
        {
            break; 
        } 
        p=((ParIS_read_worker_data*)read_pointer)->load_point[t];
        if (minidisvector[t]<bsf)
        {
            ts_buffer= &((ParIS_read_worker_data*)read_pointer)->rawfile[p*index->settings->ts_byte_size/sizeof(ts_type)];
            dist = ts_euclidean_distance_SIMD(ts, ts_buffer, index->settings->timeseries_size, bsf); 
            if(dist < bsf)  
            {  
                pthread_rwlock_wrlock(((ParIS_read_worker_data*)read_pointer)->lock_bsf); 
                if (dist<*(((ParIS_read_worker_data*)read_pointer)->bsf2)) 
                {    
                    *(((ParIS_read_worker_data*)read_pointer)->bsf2)= dist;
                } 
                pthread_rwlock_unlock(((ParIS_read_worker_data*)read_pointer)->lock_bsf); 
            } 
        } 
    } 
}
void* topk_readworker_inmemory(void *read_pointer)
{
    isax_index *index=((ParIS_read_worker_data*)read_pointer)->index;
    float bsf,dist;
    pqueue_bsf *pq_bsf=((ParIS_read_worker_data*)read_pointer)->pq_bsf;
    ts_type *ts_buffer;
    ts_type *ts =((ParIS_read_worker_data*)read_pointer)->ts;
    unsigned long t=0,p;
    unsigned long sum_of_lab=((ParIS_read_worker_data*)read_pointer)->sum_of_lab;
    float *minidisvector=((ParIS_read_worker_data*)read_pointer)->minidisvector;
    bsf= *(((ParIS_read_worker_data*)read_pointer)->bsf2); 
    
    while(1)
    { 
        pthread_rwlock_rdlock(((ParIS_read_worker_data*)read_pointer)->lock_bsf); 
        bsf= pq_bsf->knn[pq_bsf->k-1];
        pthread_rwlock_unlock(((ParIS_read_worker_data*)read_pointer)->lock_bsf); 
        t=__sync_fetch_and_add(((ParIS_read_worker_data*)read_pointer)->counter,1);
        if (t>=sum_of_lab) 
        {
            break; 
        } 
        p=((ParIS_read_worker_data*)read_pointer)->load_point[t];
        if (minidisvector[t]<bsf)
        {
            ts_buffer= &((ParIS_read_worker_data*)read_pointer)->rawfile[p*index->settings->ts_byte_size/sizeof(ts_type)];
            dist = ts_euclidean_distance_SIMD(ts, ts_buffer, index->settings->timeseries_size, bsf); 
            if(dist <= bsf)  
            {  
                pthread_rwlock_wrlock(((ParIS_read_worker_data*)read_pointer)->lock_bsf); 
                pqueue_bsf_insert(pq_bsf,dist,p,NULL);
                pthread_rwlock_unlock(((ParIS_read_worker_data*)read_pointer)->lock_bsf); 
            } 
        } 
    } 
}
void* readworker2_inmemory(void *read_pointer)
{
    isax_index *index=((ParIS_read_worker_data*)read_pointer)->index;
    float bsf,dist;
    ts_type *ts_buffer;
    ts_type *ts =((ParIS_read_worker_data*)read_pointer)->ts;
    
    unsigned long sum_of_lab=((ParIS_read_worker_data*)read_pointer)->sum_of_lab;
    long int t,p;
    float *minidisvector=((ParIS_read_worker_data*)read_pointer)->minidisvector;
    bsf= *(((ParIS_read_worker_data*)read_pointer)->bsf2); 
    
    while(1)
    { 
        pthread_rwlock_rdlock(((ParIS_read_worker_data*)read_pointer)->lock_bsf); 
        bsf= *(((ParIS_read_worker_data*)read_pointer)->bsf2); 
        pthread_rwlock_unlock(((ParIS_read_worker_data*)read_pointer)->lock_bsf); 

        t=__sync_sub_and_fetch(((ParIS_read_worker_data*)read_pointer)->counter,1);
        if(t<=0) 
        {
            break; 
        }

         
        p=((ParIS_read_worker_data*)read_pointer)->load_point[t];
        if (minidisvector[t]<bsf)
        {
            ts_buffer= &((ParIS_read_worker_data*)read_pointer)->rawfile[p*index->settings->ts_byte_size/sizeof(ts_type)];
            dist = ts_euclidean_distance_SIMD(ts, ts_buffer, index->settings->timeseries_size, bsf); 
            if(dist < bsf)  
            {  
                pthread_rwlock_wrlock(((ParIS_read_worker_data*)read_pointer)->lock_bsf); 
                if (dist<*(((ParIS_read_worker_data*)read_pointer)->bsf2)) 
                {    
                    *(((ParIS_read_worker_data*)read_pointer)->bsf2)= dist;
                } 
                pthread_rwlock_unlock(((ParIS_read_worker_data*)read_pointer)->lock_bsf); 
            } 
        } 
    } 
}


query_result exact_search_serial_ParIS_openmp_inmemory(ts_type *ts, ts_type *paa, isax_index *index, float minimum_distance, int min_checked_leaves) 
{

    RESET_BYTES_ACCESSED
    //LBDcalculationnumber=0;
    //RDcalculationnumber=0;

    pthread_t threadid[maxquerythread];
    COUNT_INPUT_TIME_START
    query_result approximate_result = approximate_search_inmemory_pRecBuf(ts, paa, index);
    ts_type *ts_buffer = malloc(index->settings->ts_byte_size);
    query_result bsf_result = approximate_result;

    int sum_of_lab=0;
    int tight_bound = index->settings->tight_bound;
    int aggressive_check = index->settings->aggressive_check;
    float bsf_distance;
    unsigned long j;
    omp_lock_t bsflock;
    //omp_init_lock(&bsflock);
    // Early termination...
    if (approximate_result.distance == 0) {
        return approximate_result;
    }
     
    if(approximate_result.distance == FLT_MAX || min_checked_leaves > 1) {
        approximate_result = refine_answer_inmemory(ts, paa, index, approximate_result, minimum_distance, min_checked_leaves);
        //approximate_result = refine_answer_m(ts, paa, index, approximate_result2, minimum_distance, min_checked_leaves);
    }
    
    COUNT_INPUT_TIME_END
    /*BIT_ARRAY* bitarray = bit_array_create(index->sax_cache_size);*/
    //printf("the old distance is: %f \n",approximate_result.distance);


    #ifdef AUTO_TUNE
    float *mindists = malloc(sizeof(float) * index->sax_cache_size);
    #endif
    SET_APPROXIMATE(approximate_result.distance);
    bsf_distance=FLT_MAX;
    COUNT_CAL_TIME_START
    //LBDcalculationnumber=index->sax_cache_size;
    #pragma omp parallel for num_threads(maxquerythread) reduction(min : bsf_distance)
    for(unsigned long  j=0; j<index->sax_cache_size; j++) {
        sax_type *sax = &index->sax_cache[j * index->settings->paa_segments];
       // if(minidist_paa_to_isax_rawa_SIMD(paa, sax, index->settings->max_sax_cardinalities,
                                       //                  index->settings->sax_bit_cardinality,
                                     //                    index->settings->sax_alphabet_cardinality,
                                    //                     index->settings->paa_segments, MINVAL, MAXVAL,
                                    //                     index->settings->mindist_sqrt) <= bsf_distance) {
            ts_buffer=&rawfile[j*index->settings->timeseries_size];
            float dist = ts_euclidean_distance_SIMD(ts, ts_buffer, index->settings->timeseries_size, bsf_distance);
            //__sync_fetch_and_add(&RDcalculationnumber,1);
            if(dist < bsf_distance) {
                //omp_set_lock(&bsflock);
                bsf_distance = dist;
            //omp_unset_lock(&bsflock);
            }
        //}
    }
    approximate_result.distance=bsf_distance;
    COUNT_CAL_TIME_END

    //        printf("the number of LB distance calculation is %ld\t\t and the Real distance calculation is %ld\n ",LBDcalculationnumber,RDcalculationnumber);

    return approximate_result;
}
query_result exact_search_serial_ParGISG_openmp_inmemory(ts_type *ts, ts_type *paa, isax_index *index, float minimum_distance, int min_checked_leaves) 
{

    RESET_BYTES_ACCESSED
    //LBDcalculationnumber=0;
    //RDcalculationnumber=0;

    pthread_t threadid[maxquerythread];
    COUNT_INPUT_TIME_START
    query_result approximate_result = approximate_search_inmemory_messi(ts, paa, index);
    ts_type *ts_buffer = malloc(index->settings->ts_byte_size);
    query_result bsf_result = approximate_result;

    int sum_of_lab=0;
    int tight_bound = index->settings->tight_bound;
    int aggressive_check = index->settings->aggressive_check;
    float bsf_distance;
    unsigned long j;
    omp_lock_t bsflock;
    omp_init_lock(&bsflock);
    // Early termination...
    if (approximate_result.distance == 0) {
        return approximate_result;
    }
    if(approximate_result.distance == FLT_MAX || min_checked_leaves > 1) {
        approximate_result = refine_answer_inmemory(ts, paa, index, approximate_result, minimum_distance, min_checked_leaves);
        //approximate_result = refine_answer_m(ts, paa, index, approximate_result2, minimum_distance, min_checked_leaves);
    }
    
    COUNT_INPUT_TIME_END
    /*BIT_ARRAY* bitarray = bit_array_create(index->sax_cache_size);*/
    //printf("the old distance is: %f \n",approximate_result.distance);

    

    SET_APPROXIMATE(approximate_result.distance);
    bsf_distance=approximate_result.distance;
    int numberofbuffer=pow(2, index->settings->paa_segments);
    pqueue_bsf* pq=pqueue_bsf_init(numberofbuffer);

    int kkkk=0;

#pragma omp parallel for num_threads(maxquerythread)
for(int i=0;i<numberofbuffer;i++)
{   
    fbl_soft_buffer2 *current_buffer = &((first_buffer_layer2*)(index->fbl))->soft_buffers[i];
    if(current_buffer->initialized==1)
    {
        isax_node* node=(((first_buffer_layer2*)(index->fbl))->soft_buffers[i]).node;
        float distance =  minidist_paa_to_isax(paa, node->isax_values,
                                            node->isax_cardinalities,
                                            index->settings->sax_bit_cardinality,
                                            index->settings->sax_alphabet_cardinality,
                                            index->settings->paa_segments,
                                            MINVAL, MAXVAL,
                                            index->settings->mindist_sqrt);
    //COUNT_CAL_TIME_END
        if(distance <= bsf_distance)
        {
            //query_result * mindist_result = malloc(sizeof(query_result));
            ///mindist_result->node = node;
            //mindist_result->distance=distance;
                   
        kkkk++;
            omp_set_lock(&bsflock);  
            pqueue_bsfre_insert(pq,distance,(long int)i,NULL);
            omp_unset_lock(&bsflock);
        }
    }
}
    COUNT_CAL_TIME_START
    for (int i = 0; i < pq->nowk; i++)
    {   
        if(pq->knn[i]<bsf_distance)
        {
            fbl_soft_buffer2 *current_buffer = &((first_buffer_layer2*)(index->fbl))->soft_buffers[pq->position[i]];
            #pragma omp parallel for num_threads(maxquerythread) reduction(min : bsf_distance)
            for(unsigned long  j=0; j<current_buffer->max_buffer_size; j++)
            {
                sax_type *sax = &current_buffer->sax_records[j*index->settings->paa_segments] ;
                if(minidist_paa_to_isax_rawa_SIMD(paa, sax, index->settings->max_sax_cardinalities,
                                                         index->settings->sax_bit_cardinality,
                                                         index->settings->sax_alphabet_cardinality,
                                                         index->settings->paa_segments, MINVAL, MAXVAL,
                                                        index->settings->mindist_sqrt) <= bsf_distance) {
                    ts_buffer=&rawfile[current_buffer->pos_records[j]];
                    float dist = ts_euclidean_distance_SIMD(ts, ts_buffer, index->settings->timeseries_size, bsf_distance);
                    if(dist < bsf_distance) 
                    {
                        bsf_distance = dist;
                    }                                            
                }
            }
        }
    }
    
   // #pragma omp parallel for num_threads(maxquerythread) reduction(min : bsf_distance)
    //for(unsigned long  j=0; j<index->sax_cache_size; j++) {
      //  sax_type *sax = &index->sax_cache[j * index->settings->paa_segments];
      //  if(minidist_paa_to_isax_rawa_SIMD(paa, sax, index->settings->max_sax_cardinalities,
                //                                         index->settings->sax_bit_cardinality,
                       //                                  index->settings->sax_alphabet_cardinality,
              //                                           index->settings->paa_segments, MINVAL, MAXVAL,
              //                                           index->settings->mindist_sqrt) <= bsf_distance) {
          //  ts_buffer=&rawfile[j*index->settings->timeseries_size];
          //  float dist = ts_euclidean_distance_SIMD(ts, ts_buffer, index->settings->timeseries_size, bsf_distance);
            //__sync_fetch_and_add(&RDcalculationnumber,1);
           // if(dist < bsf_distance) {
                //omp_set_lock(&bsflock);
           //     bsf_distance = dist;
            //omp_unset_lock(&bsflock);
          //  }
       // }
   // }
    approximate_result.distance=bsf_distance;
    COUNT_CAL_TIME_END
    return approximate_result;
}

query_result exact_search_serial_ParGIS_openmp_inmemory(ts_type *ts, ts_type *paa, isax_index *index, float minimum_distance, int min_checked_leaves) 
{

    RESET_BYTES_ACCESSED
    //LBDcalculationnumber=0;
    //RDcalculationnumber=0;




    pthread_t threadid[maxquerythread];
    COUNT_INPUT_TIME_START
    bool *rdcbitmap=malloc(sizeof(bool) * index->sax_cache_size);
    query_result approximate_result = approximate_search_inmemory_pRecBuf(ts, paa, index);
    ts_type *ts_buffer = malloc(index->settings->ts_byte_size);
    query_result bsf_result = approximate_result;
    int sum_of_lab=0;
    int tight_bound = index->settings->tight_bound;
    int aggressive_check = index->settings->aggressive_check;
    float bsf_distance;
    unsigned long j;
    omp_lock_t bsflock;
    //omp_init_lock(&bsflock);
    // Early termination...
    if (approximate_result.distance == 0) {
        return approximate_result;
    }
     
    if(approximate_result.distance == FLT_MAX || min_checked_leaves > 1) {
        approximate_result = refine_answer_inmemory(ts, paa, index, approximate_result, minimum_distance, min_checked_leaves);
        //approximate_result = refine_answer_m(ts, paa, index, approximate_result2, minimum_distance, min_checked_leaves);
    }
    
    COUNT_INPUT_TIME_END
    /*BIT_ARRAY* bitarray = bit_array_create(index->sax_cache_size);*/
    //printf("the old distance is: %f \n",approximate_result.distance);


    #ifdef AUTO_TUNE
    float *mindists = malloc(sizeof(float) * index->sax_cache_size);
    #endif
    
    SET_APPROXIMATE(approximate_result.distance);
    bsf_distance=approximate_result.distance;
    COUNT_CAL_TIME_START
    //LBDcalculationnumber=index->sax_cache_size;
    #pragma omp parallel for num_threads(maxquerythread)
    for(j=0; j<index->sax_cache_size; j++) {
                rdcbitmap[j] = FALSE;
        sax_type *sax = &index->sax_cache[j * index->settings->paa_segments];
        if(minidist_paa_to_isax_rawa_SIMD(paa, sax, index->settings->max_sax_cardinalities,
                                                         index->settings->sax_bit_cardinality,
                                                         index->settings->sax_alphabet_cardinality,
                                                         index->settings->paa_segments, MINVAL, MAXVAL,
                                                         index->settings->mindist_sqrt) <= bsf_distance) {
                    rdcbitmap[j] = TRUE;

        }
    }
        COUNT_CAL_TIME_END

    COUNT_QUEUE_TIME_START
        #pragma omp parallel for num_threads(maxquerythread) reduction(min : bsf_distance)
    for(j=0; j<index->sax_cache_size; j++) {
        if(rdcbitmap[j])
        {
            ts_buffer=&rawfile[j*index->settings->timeseries_size];
            float dist = ts_euclidean_distance_SIMD(ts, ts_buffer, index->settings->timeseries_size, bsf_distance);
             if(dist < bsf_distance) {
                //omp_set_lock(&bsflock);
                bsf_distance = dist;
            //omp_unset_lock(&bsflock);
            }
        }
    }
    free(rdcbitmap);
    approximate_result.distance=bsf_distance;
        COUNT_QUEUE_TIME_END

    //        printf("the number of LB distance calculation is %ld\t\t and the Real distance calculation is %ld\n ",LBDcalculationnumber,RDcalculationnumber);

    return approximate_result;
}


query_result exact_search_inmemory_openmp (ts_type *ts, ts_type *paa, isax_index *index,
                           float minimum_distance, int min_checked_leaves) 
{
    query_result approximate_result = approximate_search_inmemory_m(ts, paa, index);
    query_result bsf_result = approximate_result;
    int tight_bound = index->settings->tight_bound;
    int aggressive_check = index->settings->aggressive_check;
    long int numbbbber=0;

    // Early termination...
    if (approximate_result.distance == 0) {
        return approximate_result;
    }
    if(approximate_result.distance == FLT_MAX || min_checked_leaves > 1) {
        approximate_result = refine_answer_inmemory_m(ts, paa, index, approximate_result, minimum_distance, min_checked_leaves);
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
        if (mindist_result->distance< approximate_result.distance)
        {
            pqueue_insert(pq, mindist_result);
                    }
        

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
                    COUNT_OUTPUT_TIME_START
                    float mindistance = calculate_minimum_distance_inmemory(index, n->node, ts, paa);
                    COUNT_OUTPUT_TIME_END
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
                float distance = calculate_node_distance_inmemory_m(index, n->node, ts, bsf_result.distance);
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
                        float distance = calculate_node_distance_inmemory_m(index, n->node->left_child, ts, bsf_result.distance);
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
                        float distance = calculate_node_distance_inmemory_m(index, n->node->right_child, ts, bsf_result.distance);
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

query_result exact_search_ParISnew_inmemory (ts_type *ts, ts_type *paa, isax_index *index,node_list *nodelist,
                           float minimum_distance, int min_checked_leaves) 
{
    //RDcalculationnumber=0;
    //LBDcalculationnumber=0;
    query_result approximate_result = approximate_search_inmemory_pRecBuf(ts, paa, index);
    query_result bsf_result = approximate_result;
    int tight_bound = index->settings->tight_bound;
    int aggressive_check = index->settings->aggressive_check;
    int i;
    // Early termination...
    if (approximate_result.distance == 0) {
        return approximate_result;
    }
    if(approximate_result.distance == FLT_MAX || min_checked_leaves > 1) {
        approximate_result = refine_answer_inmemory_m(ts, paa, index, approximate_result, minimum_distance, min_checked_leaves);
    }

    pqueue_t *pq = pqueue_init(index->settings->root_nodes_size/1000,
                               cmp_pri, get_pri, set_pri, get_pos, set_pos);



    query_result *do_not_remove = &approximate_result;

    SET_APPROXIMATE(approximate_result.distance);

    RESET_BYTES_ACCESSED

    if(approximate_result.node != NULL) {
        // Insert approximate result in heap.
        //pqueue_insert(pq, &approximate_result);
        //GOOD: if(approximate_result.node->filename != NULL)
        //GOOD: printf("POPS: %.5lf\t", approximate_result.distance);
    }
    // Insert all root nodes in heap.
    isax_node *current_root_node = index->first_node;
    time_t t;
    srand((unsigned) time(&t));

    pthread_t threadid[maxquerythread];
    refind_answer_fonction_data rfdata;
    pthread_mutex_t lock_queue=PTHREAD_MUTEX_INITIALIZER,lock_current_root_node=PTHREAD_MUTEX_INITIALIZER;
    pthread_rwlock_t lock_bsf=PTHREAD_RWLOCK_INITIALIZER;
    pthread_barrier_t lock_barrier;
    pthread_barrier_init(&lock_barrier, NULL, maxquerythread);
    rfdata.paa=paa;
    rfdata.ts=ts;
    rfdata.lockvalueq=false;
    rfdata.lock_queue=&lock_queue;
    rfdata.lock_current_root_node=&lock_current_root_node;
    rfdata.lock_bsf=&lock_bsf;
    rfdata.nodelist=nodelist->nlist;
    rfdata.amountnode=nodelist->node_amount;
    rfdata.index=index;
    rfdata.minimum_distance=minimum_distance;
    //rfdata.limit=limit/4;
    rfdata.node_counter=0;
    rfdata.pq=pq;
    // Insert all root nodes in heap.
    rfdata.current_root_node = index->first_node;
    rfdata.bsf_result = &bsf_result;
   
    query_result * n;
    
    rfdata.lock_barrier=&lock_barrier;
    for (i = 0; i < maxquerythread; i++)
    {
        pthread_create(&(threadid[i]),NULL,exact_search_worker_inmemory,(void*)&(rfdata));
    }
    for (i = 0; i < maxquerythread; i++)
    {
        pthread_join(threadid[i],NULL);
    }


    // Free the nodes that where not popped.
    while ((n = pqueue_pop(pq)))
    {
        if(n != do_not_remove)
            free(n);
    }
    // Free the priority queue.
    pthread_barrier_destroy(&lock_barrier);
    pqueue_free(pq);
    
    bsf_result=*(rfdata.bsf_result);
    //free(rfdata);
       // printf("the number of LB distance calculation is %ld\t\t and the Real distance calculation is %ld\n ",LBDcalculationnumber,RDcalculationnumber);

    return *(rfdata.bsf_result);
    // Free the nodes that where not popped.

}
query_result exact_search_ParISnew_inmemory_workstealing (ts_type *ts, ts_type *paa, isax_index *index,node_list *nodelist,
                           float minimum_distance, int min_checked_leaves) 
{
    query_result approximate_result = approximate_search_inmemory_pRecBuf(ts, paa, index);
    //query_result approximate_result = approximate_search_inmemory(ts, paa, index);
    query_result bsf_result = approximate_result;
    int tight_bound = index->settings->tight_bound;
    int aggressive_check = index->settings->aggressive_check;
    int node_counter=0;
    // Early termination...
    if (approximate_result.distance == 0) {
        return approximate_result;
    }
    if(approximate_result.distance == FLT_MAX || min_checked_leaves > 1) {
        approximate_result = refine_answer_inmemory_m(ts, paa, index, approximate_result, minimum_distance, min_checked_leaves);
    }
    pqueue_t **allpq=malloc(sizeof(pqueue_t*)*maxquerythread);

    pqueue_t *pq = pqueue_init(index->settings->root_nodes_size,
                               cmp_pri, get_pri, set_pri, get_pos, set_pos);

    pthread_mutex_t ququelock[maxquerythread];
    int queuelabel[maxquerythread];

    query_result *do_not_remove = &approximate_result;

    SET_APPROXIMATE(approximate_result.distance);

    RESET_BYTES_ACCESSED

    if(approximate_result.node != NULL) {

    }
    // Insert all root nodes in heap.
    isax_node *current_root_node = index->first_node;

    

    pthread_t threadid[maxquerythread];
    MESSI_workerdata workerdata[maxquerythread];
    pthread_mutex_t lock_queue=PTHREAD_MUTEX_INITIALIZER,lock_current_root_node=PTHREAD_MUTEX_INITIALIZER;
    pthread_rwlock_t lock_bsf=PTHREAD_RWLOCK_INITIALIZER;
    pthread_barrier_t lock_barrier;
    pthread_barrier_init(&lock_barrier, NULL, maxquerythread);
    localStack localstk[maxquerythread];

    for (int i = 0; i < maxquerythread; i++)
    {
        workerdata[i].paa=paa;
        workerdata[i].ts=ts;
        allpq[i]=pqueue_init(index->settings->root_nodes_size/maxquerythread,
                               cmp_pri, get_pri, set_pri, get_pos, set_pos);
        pthread_mutex_init(&ququelock[i], NULL);
        queuelabel[i]=1;
        localstk[i].val=malloc(sizeof(isax_node*)*100);
        localstk[i].top=0;
        localstk[i].bottom=0;
        workerdata[i].lock_queue=&lock_queue;
        workerdata[i].lock_current_root_node=&lock_current_root_node;
        workerdata[i].lock_bsf=&lock_bsf;
        workerdata[i].nodelist=nodelist->nlist;
        workerdata[i].amountnode=nodelist->node_amount;
        workerdata[i].index=index;
        workerdata[i].minimum_distance=minimum_distance;
        workerdata[i].node_counter=&node_counter;
        workerdata[i].pq=allpq[i];
        workerdata[i].bsf_result = &bsf_result;
        workerdata[i].lock_barrier=&lock_barrier;
        workerdata[i].localstk=&(localstk[i]);
        workerdata[i].allstk=localstk;
        workerdata[i].locallock=&ququelock[i];
        workerdata[i].alllock=ququelock;
        workerdata[i].queuelabel=&queuelabel[i];
        workerdata[i].allqueuelabel=queuelabel;
        workerdata[i].allpq=allpq;
    }
    

   
    query_result * n;
    
    
    for (int i = 0; i < maxquerythread; i++)
    {
        pthread_create(&(threadid[i]),NULL,exact_search_worker_inmemory_workstealing,(void*)&(workerdata[i]));
    }
    for (int i = 0; i < maxquerythread; i++)
    {
        pthread_join(threadid[i],NULL);
    }

    // Free the nodes that where not popped.
    while ((n = pqueue_pop(pq)))
    {
        if(n != do_not_remove)
            free(n);
    }
    // Free the priority queue.
    pthread_barrier_destroy(&lock_barrier);
        while(n=pqueue_pop(pq))
    {
            free(n);
    }
    pqueue_free(pq);
    free(allpq);
    bsf_result=bsf_result;

    //free(rfdata);
    return bsf_result;

    // Free the nodes that where not popped.

}

query_result exact_search_ParISnew_inmemory_hybrid (ts_type *ts, ts_type *paa, isax_index *index,node_list *nodelist,
                           float minimum_distance, int min_checked_leaves) 
{   
    RDcalculationnumber=0;
    LBDcalculationnumber=0;
    query_result approximate_result = approximate_search_inmemory_pRecBuf(ts, paa, index);
    //query_result approximate_result = approximate_search_inmemory(ts, paa, index);
    query_result bsf_result = approximate_result;
    int tight_bound = index->settings->tight_bound;
    int aggressive_check = index->settings->aggressive_check;
    int node_counter=0;
    // Early termination...
    if (approximate_result.distance == 0) {
        return approximate_result;
    }
    if(approximate_result.distance == FLT_MAX || min_checked_leaves > 1) {
        approximate_result = refine_answer_inmemory_m(ts, paa, index, approximate_result, minimum_distance, min_checked_leaves);
    }
    pqueue_t **allpq=malloc(sizeof(pqueue_t*)*N_PQUEUE);


    pthread_mutex_t ququelock[N_PQUEUE];
    int queuelabel[N_PQUEUE];

    query_result *do_not_remove = &approximate_result;

    SET_APPROXIMATE(approximate_result.distance);


    if(approximate_result.node != NULL) {
        // Insert approximate result in heap.
        //pqueue_insert(pq, &approximate_result);
        //GOOD: if(approximate_result.node->filename != NULL)
        //GOOD: printf("POPS: %.5lf\t", approximate_result.distance);
    }
    // Insert all root nodes in heap.
    isax_node *current_root_node = index->first_node;

    pthread_t threadid[maxquerythread];
    MESSI_workerdata workerdata[maxquerythread];
    pthread_mutex_t lock_queue=PTHREAD_MUTEX_INITIALIZER,lock_current_root_node=PTHREAD_MUTEX_INITIALIZER;
    pthread_rwlock_t lock_bsf=PTHREAD_RWLOCK_INITIALIZER;
    pthread_barrier_t lock_barrier;
    pthread_barrier_init(&lock_barrier, NULL, maxquerythread);
 
    
    for (int i = 0; i < N_PQUEUE; i++)
    {
                allpq[i]=pqueue_init(index->settings->root_nodes_size/N_PQUEUE,
                               cmp_pri, get_pri, set_pri, get_pos, set_pos);
                pthread_mutex_init(&ququelock[i], NULL);
                queuelabel[i]=1;
    }

    for (int i = 0; i < maxquerythread; i++)
    {
        workerdata[i].paa=paa;
        workerdata[i].ts=ts;
        workerdata[i].lock_queue=&lock_queue;
        workerdata[i].lock_current_root_node=&lock_current_root_node;
        workerdata[i].lock_bsf=&lock_bsf;
        workerdata[i].nodelist=nodelist->nlist;
        workerdata[i].amountnode=nodelist->node_amount;
        workerdata[i].index=index;
        workerdata[i].minimum_distance=minimum_distance;
        workerdata[i].node_counter=&node_counter;
        workerdata[i].pq=allpq[i];
        workerdata[i].bsf_result = &bsf_result;
        workerdata[i].lock_barrier=&lock_barrier;
        workerdata[i].alllock=ququelock;
        workerdata[i].allqueuelabel=queuelabel;
        workerdata[i].allpq=allpq;
        workerdata[i].startqueuenumber=i%N_PQUEUE;
    }
        
    
    for (int i = 0; i < maxquerythread; i++)
    {
        pthread_create(&(threadid[i]),NULL,exact_search_worker_inmemory_hybridpqueue,(void*)&(workerdata[i]));
    }
    for (int i = 0; i < maxquerythread; i++)
    {
        pthread_join(threadid[i],NULL);
    }

    // Free the nodes that where not popped.
    // Free the priority queue.
    pthread_barrier_destroy(&lock_barrier);

    //pqueue_free(pq);
    for (int i = 0; i < N_PQUEUE; i++)
    {
        pqueue_free(allpq[i]);
    }
    free(allpq);
    bsf_result=bsf_result;

    //free(rfdata);
        //printf("the number of insert node is \t%ld\t\t and the delete node is \t%ld\n ",LBDcalculationnumber,RDcalculationnumber);
    return bsf_result;

    // Free the nodes that where not popped.

}


query_result exact_search_ParISnew_inmemory_hybrid_workstealing (ts_type *ts, ts_type *paa, isax_index *index,node_list *nodelist,
                           float minimum_distance, int min_checked_leaves) 
{
    query_result approximate_result = approximate_search_inmemory_pRecBuf(ts, paa, index);
    //query_result approximate_result = approximate_search_inmemory(ts, paa, index);
    query_result bsf_result = approximate_result;
    int tight_bound = index->settings->tight_bound;
    int aggressive_check = index->settings->aggressive_check;
    int node_counter=0;
    // Early termination...
    if (approximate_result.distance == 0) {
        return approximate_result;
    }
    if(approximate_result.distance == FLT_MAX || min_checked_leaves > 1) {
        approximate_result = refine_answer_inmemory_m(ts, paa, index, approximate_result, minimum_distance, min_checked_leaves);
    }
    pqueue_t **allpq=malloc(sizeof(pqueue_t*)*N_PQUEUE);

    pqueue_t *pq = pqueue_init(index->settings->root_nodes_size,
                               cmp_pri, get_pri, set_pri, get_pos, set_pos);

    pthread_mutex_t ququelock[N_PQUEUE];
    int queuelabel[N_PQUEUE];

    query_result *do_not_remove = &approximate_result;

    SET_APPROXIMATE(approximate_result.distance);

    RESET_BYTES_ACCESSED

    if(approximate_result.node != NULL) {
        // Insert approximate result in heap.
        //pqueue_insert(pq, &approximate_result);
        //GOOD: if(approximate_result.node->filename != NULL)
        //GOOD: printf("POPS: %.5lf\t", approximate_result.distance);
    }
    // Insert all root nodes in heap.
    isax_node *current_root_node = index->first_node;

    pthread_t threadid[maxquerythread];
    MESSI_workerdata workerdata[maxquerythread];
    pthread_mutex_t lock_queue=PTHREAD_MUTEX_INITIALIZER,lock_current_root_node=PTHREAD_MUTEX_INITIALIZER;
    pthread_rwlock_t lock_bsf=PTHREAD_RWLOCK_INITIALIZER;
    pthread_barrier_t lock_barrier;
    pthread_barrier_init(&lock_barrier, NULL, maxquerythread);
    localStack localstk[maxquerythread];
    
    for (int i = 0; i < N_PQUEUE; i++)
    {
                allpq[i]=pqueue_init(index->settings->root_nodes_size/N_PQUEUE,
                               cmp_pri, get_pri, set_pri, get_pos, set_pos);
                pthread_mutex_init(&ququelock[i], NULL);
                queuelabel[i]=1;
    }

    for (int i = 0; i < maxquerythread; i++)
    {
        workerdata[i].paa=paa;
        workerdata[i].ts=ts;
        localstk[i].val=malloc(sizeof(isax_node*)*100);
        localstk[i].top=0;
        localstk[i].bottom=0;
        workerdata[i].lock_queue=&lock_queue;
        workerdata[i].lock_current_root_node=&lock_current_root_node;
        workerdata[i].lock_bsf=&lock_bsf;
        workerdata[i].nodelist=nodelist->nlist;
        workerdata[i].amountnode=nodelist->node_amount;
        workerdata[i].index=index;
        workerdata[i].minimum_distance=minimum_distance;
        workerdata[i].node_counter=&node_counter;
        workerdata[i].pq=allpq[i];
        workerdata[i].bsf_result = &bsf_result;
        workerdata[i].lock_barrier=&lock_barrier;
        workerdata[i].localstk=&(localstk[i]);
        workerdata[i].allstk=localstk;
        workerdata[i].alllock=ququelock;
        workerdata[i].allqueuelabel=queuelabel;
        workerdata[i].allpq=allpq;
        workerdata[i].startqueuenumber=i%N_PQUEUE;
        //printf("the workerdata[i].startqueuenumber is %d\n",workerdata[i].startqueuenumber );
    }
    

   
    query_result * n;
    
    
    for (int i = 0; i < maxquerythread; i++)
    {
        pthread_create(&(threadid[i]),NULL,exact_search_worker_inmemory_hybridpqueue_workstealing,(void*)&(workerdata[i]));
    }
    for (int i = 0; i < maxquerythread; i++)
    {
        pthread_join(threadid[i],NULL);
    }

    // Free the nodes that where not popped.
    while ((n = pqueue_pop(pq)))
    {
        if(n != do_not_remove)
            free(n);
    }
    // Free the priority queue.
    pthread_barrier_destroy(&lock_barrier);
        while(n=pqueue_pop(pq))
    {
            free(n);
    }
    pqueue_free(pq);
    free(allpq);
    bsf_result=bsf_result;

    //free(rfdata);
    return bsf_result;

    // Free the nodes that where not popped.

}

void* exact_search_worker_inmemory(void *rfdata)
{
    isax_node *current_root_node;
    query_result *n;
    isax_index *index=((refind_answer_fonction_data*)rfdata)->index;
    ts_type *paa=((refind_answer_fonction_data*)rfdata)->paa;
    ts_type *ts=((refind_answer_fonction_data*)rfdata)->ts;
    pqueue_t *pq=((refind_answer_fonction_data*)rfdata)->pq;
    query_result *do_not_remove = ((refind_answer_fonction_data*)rfdata)->bsf_result;
    float minimum_distance=((refind_answer_fonction_data*)rfdata)->minimum_distance;
    int limit=((refind_answer_fonction_data*)rfdata)->limit;
    int checks = 0;
    int current_root_node_number;
    int tight_bound = index->settings->tight_bound;
    int aggressive_check = index->settings->aggressive_check;
    query_result *bsf_result=(((refind_answer_fonction_data*)rfdata)->bsf_result);
    float bsfdisntance=bsf_result->distance;
    int calculate_node=0,calculate_node_quque=0;
    //COUNT_QUEUE_TIME_START

    //gettimeofday(&workertimestart, NULL);




    while (1) 
    {
            current_root_node_number=__sync_fetch_and_add(&((refind_answer_fonction_data*)rfdata)->node_counter,1);
            //printf("the number is %d\n",current_root_node_number );
            if(current_root_node_number>= ((refind_answer_fonction_data*)rfdata)->amountnode)
            break;

            current_root_node=((refind_answer_fonction_data*)rfdata)->nodelist[current_root_node_number];
            insert_tree_node_m(paa,current_root_node,index,bsfdisntance,pq,((refind_answer_fonction_data*)rfdata)->lock_queue);
            
    }
    //COUNT_QUEUE_TIME_END
    //gettimeofday(&workercurenttime, NULL);
    pthread_barrier_wait(((refind_answer_fonction_data*)rfdata)->lock_barrier);
    //printf("the size of quque is %d \n",pq->size);
 while (1)
    {
        while( !__sync_bool_compare_and_swap (&(((refind_answer_fonction_data*)rfdata)->lockvalueq), false, true))
        {

        }
        //pthread_mutex_lock(((refind_answer_fonction_data*)rfdata)->lockvalue);
        n = pqueue_pop(pq);
        //pthread_mutex_unlock(((refind_answer_fonction_data*)rfdata)->lock_queue);
        __sync_lock_release (&(((refind_answer_fonction_data*)rfdata)->lockvalueq));
        if(n==NULL)
            break;
        //pthread_rwlock_rdlock(((refind_answer_fonction_data*)rfdata)->lock_bsf);
        bsfdisntance=bsf_result->distance;
        //pthread_rwlock_unlock(((refind_answer_fonction_data*)rfdata)->lock_bsf);
        // The best node has a worse mindist, so search is finished!

        if (n->distance > bsfdisntance || n->distance > minimum_distance) {
            break;
        }
        else 
        {
            // If it is a leaf, check its real distance.
            if (n->node->is_leaf) {

                checks++;
                float distance = calculate_node_distance2_inmemory(index, n->node, ts,paa, bsfdisntance);
                if (distance < bsfdisntance)
                {   
                    //while( !__sync_bool_compare_and_swap((bsf_result->distance), bsfdisntance, distance))
                    //{
                        //bsfdisntance=bsf_result->distance;
                       // if(distance>=bsfdisntance)
                           // break;
                 //   }
                    pthread_rwlock_wrlock(((refind_answer_fonction_data*)rfdata)->lock_bsf);
                    if(distance < bsf_result->distance)
                    {
                        bsf_result->distance = distance;
                        bsf_result->node = n->node;
                    }
                    pthread_rwlock_unlock(((refind_answer_fonction_data*)rfdata)->lock_bsf);
                }

            }
            
        }
        //if(n != do_not_remove)//add
            free(n);
    }

    //worker_total_time += workercurenttime.tv_sec*1000000 + (workercurenttime.tv_usec)-workertimestart.tv_sec*1000000 - (workertimestart.tv_usec); 
    //printf("create pq time is %f \n",worker_total_time );
    //printf("the check's node is \t%d\n",checks);
}
void* exact_search_worker_inmemory_workstealing(void *rfdata)
{
    isax_node *current_root_node;
    query_result *n;
    isax_index *index=((MESSI_workerdata*)rfdata)->index;
    ts_type *paa=((MESSI_workerdata*)rfdata)->paa;
    ts_type *ts=((MESSI_workerdata*)rfdata)->ts;
    pqueue_t *pq=((MESSI_workerdata*)rfdata)->pq;
    query_result *do_not_remove = ((MESSI_workerdata*)rfdata)->bsf_result;
    float minimum_distance=((MESSI_workerdata*)rfdata)->minimum_distance;
    localStack *localstk=((MESSI_workerdata*)rfdata)->localstk;
    int limit=((MESSI_workerdata*)rfdata)->limit;
    int checks = 0;
    bool finished=true;
    int current_root_node_number;
    int tight_bound = index->settings->tight_bound;
    int aggressive_check = index->settings->aggressive_check;
    query_result *bsf_result=(((MESSI_workerdata*)rfdata)->bsf_result);
    float bsfdisntance=bsf_result->distance;
    int calculate_node=0,calculate_node_quque=0;
    
    //COUNT_QUEUE_TIME_START
    //struct timeval workertimestart;
    //struct timeval writetiemstart;
    //struct timeval workercurenttime;
    //struct timeval writecurenttime;
    //double worker_total_time,write_total_time;
    //gettimeofday(&workertimestart, NULL);

    while (1) 
    {
            current_root_node_number=__sync_fetch_and_add(((MESSI_workerdata*)rfdata)->node_counter,1);
            //printf("the number is %d\n",current_root_node_number );
            if(current_root_node_number>= ((MESSI_workerdata*)rfdata)->amountnode)
            break;

            current_root_node=((MESSI_workerdata*)rfdata)->nodelist[current_root_node_number];
            insert_tree_node_m_workstealing(paa,current_root_node,index,bsfdisntance,pq,((MESSI_workerdata*)rfdata)->lock_queue,localstk);
            //insert_tree_node_mW(paa,current_root_node,index,bsfdisntance,pq,((MESSI_workerdata*)rfdata)->lock_queue);

            
    }
    while(1)
    {   
        int offset=rand()% maxquerythread;
        finished=true;
        for (int i = 0; i < maxquerythread; i++)
        {
            if(((MESSI_workerdata*)rfdata)->allstk[(i+offset)%maxquerythread].bottom!=0)
            {
                finished=false;
                isax_node* newnode=poptop2(&(((MESSI_workerdata*)rfdata)->allstk[(i+offset)%maxquerythread]));
                if(newnode!=NULL)
                insert_tree_node_m_workstealing(paa,newnode,index,bsfdisntance,pq,((MESSI_workerdata*)rfdata)->lock_queue,localstk);
            }
        }
        if (finished)
        {
            break;
        }
    }

    //COUNT_QUEUE_TIME_END
    calculate_node_quque=pq->size;
    //gettimeofday(&workercurenttime, NULL); 
    pthread_barrier_wait(((MESSI_workerdata*)rfdata)->lock_barrier);
    //printf("the size of quque is %d \n",pq->size);
  while (1)
    {
        pthread_mutex_lock(((MESSI_workerdata*)rfdata)->locallock);
        n = pqueue_pop(pq);
        pthread_mutex_unlock(((MESSI_workerdata*)rfdata)->locallock);
        if(n==NULL)
            break;
        //pthread_rwlock_rdlock(((MESSI_workerdata*)rfdata)->lock_bsf);
        bsfdisntance=bsf_result->distance;
        //pthread_rwlock_unlock(((MESSI_workerdata*)rfdata)->lock_bsf);
        // The best node has a worse mindist, so search is finished!

        if (n->distance > bsfdisntance || n->distance > minimum_distance) {
            break;
        }
        else 
        {
            // If it is a leaf, check its real distance.
            if (n->node->is_leaf) {

                checks++;
                float distance = calculate_node_distance2_inmemory(index, n->node, ts,paa, bsfdisntance);
                if (distance < bsfdisntance)
                {
                    pthread_rwlock_wrlock(((MESSI_workerdata*)rfdata)->lock_bsf);
                    if(distance < bsf_result->distance)
                    {
                        bsf_result->distance = distance;
                        bsf_result->node = n->node;
                    }
                    pthread_rwlock_unlock(((MESSI_workerdata*)rfdata)->lock_bsf);
                }

            }
            
        }
            free(n);
    }

    *(((MESSI_workerdata*)rfdata)->queuelabel)=0;
    while(1)
    {   
        int offset=rand()% maxquerythread;
        finished=true;
        for (int i = 0; i < maxquerythread; i++)
        {
            if((((MESSI_workerdata*)rfdata)->allqueuelabel[i])==1)
            {
                finished=false;
                while(1)
                {
                    pthread_mutex_lock(&(((MESSI_workerdata*)rfdata)->alllock[i]));
                    n = pqueue_pop(((MESSI_workerdata*)rfdata)->allpq[i]);
                    pthread_mutex_unlock(&(((MESSI_workerdata*)rfdata)->alllock[i]));
                    if(n==NULL)
                    break;
                    if (n->distance > bsfdisntance || n->distance > minimum_distance) {
                        break;
                    }        
                    else 
                    {
                        // If it is a leaf, check its real distance.
                        if (n->node->is_leaf) 
                        {
                            checks++;
                            float distance = calculate_node_distance2_inmemory(index, n->node, ts,paa, bsfdisntance);
                            if (distance < bsfdisntance)
                            {
                                pthread_rwlock_wrlock(((MESSI_workerdata*)rfdata)->lock_bsf);
                                if(distance < bsf_result->distance)
                                {
                                    bsf_result->distance = distance;
                                    bsf_result->node = n->node;
                                }
                                pthread_rwlock_unlock(((MESSI_workerdata*)rfdata)->lock_bsf);
                            }

                        }
            
                    }
                //add
                free(n);
                }
            }
        }
        if (finished)
        {
            break;
        }
    }
    free(localstk->val);

    pthread_barrier_wait(((MESSI_workerdata*)rfdata)->lock_barrier);
    while(n=pqueue_pop(pq))
    {
            free(n);
    }
    pqueue_free(pq);
    //worker_total_time += workercurenttime.tv_sec*1000000 + (workercurenttime.tv_usec)-workertimestart.tv_sec*1000000 - (workertimestart.tv_usec); 
    //printf("create pq time is %f \n",worker_total_time );
    //printf("the worker's write  time is %f \n",write_total_time );
    //printf("the check's node is\t %d\tthe local queue's node is\t%d\n",checks,calculate_node_quque);
}
void* exact_search_worker_inmemory_hybridpqueue(void *rfdata)
{   

    isax_node *current_root_node;
    query_result *n;
    isax_index *index=((MESSI_workerdata*)rfdata)->index;
    ts_type *paa=((MESSI_workerdata*)rfdata)->paa;
    ts_type *ts=((MESSI_workerdata*)rfdata)->ts;
    pqueue_t *pq=((MESSI_workerdata*)rfdata)->pq;
    query_result *do_not_remove = ((MESSI_workerdata*)rfdata)->bsf_result;
    float minimum_distance=((MESSI_workerdata*)rfdata)->minimum_distance;
    int limit=((MESSI_workerdata*)rfdata)->limit;
    int checks = 0;
    bool finished=true;
    int current_root_node_number;
    int tight_bound = index->settings->tight_bound;
    int aggressive_check = index->settings->aggressive_check;
    query_result *bsf_result=(((MESSI_workerdata*)rfdata)->bsf_result);
    float bsfdisntance=bsf_result->distance;
    int calculate_node=0,calculate_node_quque=0;
    int tnumber=rand()% N_PQUEUE;
    int startqueuenumber=((MESSI_workerdata*)rfdata)->startqueuenumber;
    //COUNT_QUEUE_TIME_START

    while (1) 
    {
            current_root_node_number=__sync_fetch_and_add(((MESSI_workerdata*)rfdata)->node_counter,1);
            //printf("the number is %d\n",current_root_node_number );
            if(current_root_node_number>= ((MESSI_workerdata*)rfdata)->amountnode)
            break;
            current_root_node=((MESSI_workerdata*)rfdata)->nodelist[current_root_node_number];

            insert_tree_node_m_hybridpqueue(paa,current_root_node,index,bsfdisntance,((MESSI_workerdata*)rfdata)->allpq,((MESSI_workerdata*)rfdata)->alllock,&tnumber);
            //insert_tree_node_mW(paa,current_root_node,index,bsfdisntance,pq,((MESSI_workerdata*)rfdata)->lock_queue);

            
    }

    //COUNT_QUEUE_TIME_END
    //calculate_node_quque=pq->size;

    pthread_barrier_wait(((MESSI_workerdata*)rfdata)->lock_barrier);
    //printf("the size of quque is %d \n",pq->size);
    while (1)
    {
        pthread_mutex_lock(&(((MESSI_workerdata*)rfdata)->alllock[startqueuenumber]));
        n = pqueue_pop(((MESSI_workerdata*)rfdata)->allpq[startqueuenumber]);
                                        //__sync_fetch_and_add(&RDcalculationnumber,1);

        pthread_mutex_unlock(&(((MESSI_workerdata*)rfdata)->alllock[startqueuenumber]));
        if(n==NULL)
            break;
        //pthread_rwlock_rdlock(((MESSI_workerdata*)rfdata)->lock_bsf);
        bsfdisntance=bsf_result->distance;
        //pthread_rwlock_unlock(((MESSI_workerdata*)rfdata)->lock_bsf);
        // The best node has a worse mindist, so search is finished!

        if (n->distance > bsfdisntance || n->distance > minimum_distance) {
            break;
        }
        else 
        {
            // If it is a leaf, check its real distance.
            if (n->node->is_leaf) {

                checks++;

                float distance = calculate_node_distance2_inmemory(index, n->node, ts,paa, bsfdisntance);
                if (distance < bsfdisntance)
                {
                    pthread_rwlock_wrlock(((MESSI_workerdata*)rfdata)->lock_bsf);
                    if(distance < bsf_result->distance)
                    {
                        bsf_result->distance = distance;
                        bsf_result->node = n->node;
                    }
                    pthread_rwlock_unlock(((MESSI_workerdata*)rfdata)->lock_bsf);
                }

            }
            
        }
        free(n);
    }

    if( (((MESSI_workerdata*)rfdata)->allqueuelabel[startqueuenumber])==1)
    {
        (((MESSI_workerdata*)rfdata)->allqueuelabel[startqueuenumber])=0;
        pthread_mutex_lock(&(((MESSI_workerdata*)rfdata)->alllock[startqueuenumber]));
        while(n = pqueue_pop(((MESSI_workerdata*)rfdata)->allpq[startqueuenumber]))
        {
            free(n);
        }
        pthread_mutex_unlock(&(((MESSI_workerdata*)rfdata)->alllock[startqueuenumber]));
    }

    while(1)
    {   
        int offset=rand()% N_PQUEUE;
        finished=true;
        for (int i = 0; i < N_PQUEUE; i++)
        {
            if((((MESSI_workerdata*)rfdata)->allqueuelabel[i])==1)
            {
                finished=false;
                while(1)
                {
                    pthread_mutex_lock(&(((MESSI_workerdata*)rfdata)->alllock[i]));
                    n = pqueue_pop(((MESSI_workerdata*)rfdata)->allpq[i]);
                                             //       __sync_fetch_and_add(&RDcalculationnumber,1);

                    pthread_mutex_unlock(&(((MESSI_workerdata*)rfdata)->alllock[i]));
                    if(n==NULL)
                    break;
                    if (n->distance > bsfdisntance || n->distance > minimum_distance) {
                        break;
                    }        
                    else 
                    {
                        // If it is a leaf, check its real distance.
                        if (n->node->is_leaf) 
                        {
                            checks++;
                            float distance = calculate_node_distance2_inmemory(index, n->node, ts,paa, bsfdisntance);
                            if (distance < bsfdisntance)
                            {
                                pthread_rwlock_wrlock(((MESSI_workerdata*)rfdata)->lock_bsf);
                                if(distance < bsf_result->distance)
                                {
                                    bsf_result->distance = distance;
                                    bsf_result->node = n->node;
                                }
                                pthread_rwlock_unlock(((MESSI_workerdata*)rfdata)->lock_bsf);
                            }

                        }
            
                    }
                //add
                free(n);
                }

            }
        }
        if (finished)
        {
            break;
        }
    }


    //pthread_barrier_wait(((MESSI_workerdata*)rfdata)->lock_barrier);
    //while(n=pqueue_pop(pq))
    //{
            //free(n);
    //}
    //pqueue_free(pq);
    //
    

                                      //printf("create pq time is %f \n",worker_total_time );
    //printf("the check's node is\t %d\tthe local queue's node is\t%d\n",checks,calculate_node_quque);
}


void* exact_search_worker_inmemory_hybridpqueue_workstealing(void *rfdata)
{
    isax_node *current_root_node;
    query_result *n;
    isax_index *index=((MESSI_workerdata*)rfdata)->index;
    ts_type *paa=((MESSI_workerdata*)rfdata)->paa;
    ts_type *ts=((MESSI_workerdata*)rfdata)->ts;
    pqueue_t *pq=((MESSI_workerdata*)rfdata)->pq;
    query_result *do_not_remove = ((MESSI_workerdata*)rfdata)->bsf_result;

    float minimum_distance=((MESSI_workerdata*)rfdata)->minimum_distance;
    localStack *localstk=((MESSI_workerdata*)rfdata)->localstk;
    int limit=((MESSI_workerdata*)rfdata)->limit;
    int checks = 0;
    bool finished=true;
    int current_root_node_number;
    int tight_bound = index->settings->tight_bound;
    int aggressive_check = index->settings->aggressive_check;
    query_result *bsf_result=(((MESSI_workerdata*)rfdata)->bsf_result);
    float bsfdisntance=bsf_result->distance;
    int calculate_node=0,calculate_node_quque=0;
    int tnumber=rand()% N_PQUEUE;
    int startqueuenumber=((MESSI_workerdata*)rfdata)->startqueuenumber;
    //COUNT_QUEUE_TIME_START
    //struct timeval workertimestart;
    //struct timeval writetiemstart;
    //struct timeval workercurenttime;
    //struct timeval writecurenttime;
    //double worker_total_time,write_total_time;
    //gettimeofday(&workertimestart, NULL);

    while (1) 
    {
            current_root_node_number=__sync_fetch_and_add(((MESSI_workerdata*)rfdata)->node_counter,1);
            //printf("the number is %d\n",current_root_node_number );
            if(current_root_node_number>= ((MESSI_workerdata*)rfdata)->amountnode)
            break;

            current_root_node=((MESSI_workerdata*)rfdata)->nodelist[current_root_node_number];
            insert_tree_node_m_hybridpqueue_workstealing(paa,current_root_node,index,bsfdisntance,((MESSI_workerdata*)rfdata)->allpq,((MESSI_workerdata*)rfdata)->alllock,localstk,&tnumber);

            //insert_tree_node_mW(paa,current_root_node,index,bsfdisntance,pq,((MESSI_workerdata*)rfdata)->lock_queue);

    }
    while(1)
    {   
        int offset=rand()% maxquerythread;
        finished=true;
        for (int i = 0; i < maxquerythread; i++)
        {
            if(((MESSI_workerdata*)rfdata)->allstk[(i+offset)%maxquerythread].bottom!=0)
            {
                finished=false;
                isax_node* newnode=poptop2(&(((MESSI_workerdata*)rfdata)->allstk[(i+offset)%maxquerythread]));
                if(newnode!=NULL)
                insert_tree_node_m_hybridpqueue_workstealing(paa,newnode,index,bsfdisntance,((MESSI_workerdata*)rfdata)->allpq,((MESSI_workerdata*)rfdata)->alllock,localstk,&tnumber);
            }
        }
        if (finished)
        {
            break;
        }
    }

    //COUNT_QUEUE_TIME_END
    calculate_node_quque=pq->size;
    //gettimeofday(&workercurenttime, NULL); 
    pthread_barrier_wait(((MESSI_workerdata*)rfdata)->lock_barrier);
    //printf("the size of quque is %d \n",pq->size);
  while (1)
    {
        pthread_mutex_lock(&(((MESSI_workerdata*)rfdata)->alllock[startqueuenumber]));
        n = pqueue_pop(((MESSI_workerdata*)rfdata)->allpq[startqueuenumber]);
        pthread_mutex_unlock(&(((MESSI_workerdata*)rfdata)->alllock[startqueuenumber]));
        if(n==NULL)
            break;
        //pthread_rwlock_rdlock(((MESSI_workerdata*)rfdata)->lock_bsf);
        bsfdisntance=bsf_result->distance;
        //pthread_rwlock_unlock(((MESSI_workerdata*)rfdata)->lock_bsf);
        // The best node has a worse mindist, so search is finished!

        if (n->distance > bsfdisntance || n->distance > minimum_distance) {
            break;
        }
        else 
        {
            // If it is a leaf, check its real distance.
            if (n->node->is_leaf) {

                checks++;
                float distance = calculate_node_distance2_inmemory(index, n->node, ts,paa, bsfdisntance);
                if (distance < bsfdisntance)
                {
                    pthread_rwlock_wrlock(((MESSI_workerdata*)rfdata)->lock_bsf);
                    if(distance < bsf_result->distance)
                    {
                        bsf_result->distance = distance;
                        bsf_result->node = n->node;
                    }
                    pthread_rwlock_unlock(((MESSI_workerdata*)rfdata)->lock_bsf);
                }

            }
            
        }
            free(n);
    }

    (((MESSI_workerdata*)rfdata)->allqueuelabel[startqueuenumber])=0;
    while(1)
    {   
        int offset=rand()% N_PQUEUE;
        finished=true;
        for (int i = 0; i < N_PQUEUE; i++)
        {
            if((((MESSI_workerdata*)rfdata)->allqueuelabel[i])==1)
            {
                finished=false;
                while(1)
                {
                    pthread_mutex_lock(&(((MESSI_workerdata*)rfdata)->alllock[i]));
                    n = pqueue_pop(((MESSI_workerdata*)rfdata)->allpq[i]);
                    pthread_mutex_unlock(&(((MESSI_workerdata*)rfdata)->alllock[i]));
                    if(n==NULL)
                    break;
                    if (n->distance > bsfdisntance || n->distance > minimum_distance) {
                        break;
                    }        
                    else 
                    {
                        // If it is a leaf, check its real distance.
                        if (n->node->is_leaf) 
                        {
                            checks++;
                            float distance = calculate_node_distance2_inmemory(index, n->node, ts,paa, bsfdisntance);
                            if (distance < bsfdisntance)
                            {
                                pthread_rwlock_wrlock(((MESSI_workerdata*)rfdata)->lock_bsf);
                                if(distance < bsf_result->distance)
                                {
                                    bsf_result->distance = distance;
                                    bsf_result->node = n->node;
                                }
                                pthread_rwlock_unlock(((MESSI_workerdata*)rfdata)->lock_bsf);
                            }

                        }
            
                    }
                //add
                free(n);
                }
            }
        }
        if (finished)
        {
            break;
        }
    }
    free(localstk->val);

    //pthread_barrier_wait(((MESSI_workerdata*)rfdata)->lock_barrier);
    //while(n=pqueue_pop(pq))
    //{
            //free(n);
    //}
    //pqueue_free(pq);
    //worker_total_time += workercurenttime.tv_sec*1000000 + (workercurenttime.tv_usec)-workertimestart.tv_sec*1000000 - (workertimestart.tv_usec); 
    //printf("create pq time is %f \n",worker_total_time );
    //printf("the worker's write  time is %f \n",worker_total_time );
    //printf("the check's node is\t %d\tthe local queue's node is\t%d\n",checks,calculate_node_quque);
}
void insert_tree_node_m(float *paa,isax_node *node,isax_index *index,float bsf,pqueue_t *pq,pthread_mutex_t *lock_queue)
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
            pthread_mutex_lock(lock_queue);
            pqueue_insert(pq, mindist_result);
            pthread_mutex_unlock(lock_queue);
            added_tree_node++;
        }
        else
        {   
            if (node->left_child->isax_cardinalities != NULL)
            {
                insert_tree_node_m(paa,node->left_child,index, bsf,pq,lock_queue);
            }
            if (node->right_child->isax_cardinalities != NULL)
            {
                insert_tree_node_m(paa,node->right_child,index,bsf,pq,lock_queue);
            }
        }
    }
}



void insert_tree_node_mgpu(float *paa,isax_node *node,isax_index *index,float bsf,pqueue_bsf *pq,pthread_mutex_t *lock_queue)
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
            pthread_mutex_lock(lock_queue);
            pqueue_bsf_insert(pq,distance,0, node);
            pthread_mutex_unlock(lock_queue);
            added_tree_node++;
        }
        else
        {   
            if (node->left_child->isax_cardinalities != NULL)
            {
                insert_tree_node_mgpu(paa,node->left_child,index, bsf,pq,lock_queue);
            }
            if (node->right_child->isax_cardinalities != NULL)
            {
                insert_tree_node_mgpu(paa,node->right_child,index,bsf,pq,lock_queue);
            }
        }
    }
}

/*void insert_tree_node_m_parallelqueue(float *paa,isax_node *node,isax_index *index,float bsf,FGHPQueue *pq,FGHPQueueThreadState *th_state)
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
            //pthread_mutex_lock(lock_queue);
            //pqueue_insert(pq, mindist_result);
            //pthread_mutex_unlock(lock_queue);
            FGHPQueueAdd(pq, th_state, node, distance);
            added_tree_node++;
        }
        else
        {   
            if (node->left_child->isax_cardinalities != NULL)
            {
                insert_tree_node_m_parallelqueue(paa,node->left_child,index, bsf,pq,th_state);
            }
            if (node->right_child->isax_cardinalities != NULL)
            {
                insert_tree_node_m_parallelqueue(paa,node->right_child,index,bsf,pq,th_state);
            }
        }
    }
}*/


void insert_tree_node_m_workstealing(float *paa,isax_node *node,isax_index *index,float bsf,pqueue_t *pq,pthread_mutex_t *lock_queue,localStack* workstack)
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
            //pthread_mutex_lock(lock_queue);
            pqueue_insert(pq, mindist_result);
            //pthread_mutex_unlock(lock_queue);
            added_tree_node++;
        }
        else
        {   
            if (node->right_child->isax_cardinalities != NULL)
            {
                pushbottom(workstack, node->right_child);
            }
            if (node->left_child->isax_cardinalities != NULL)
            {
                insert_tree_node_m_workstealing(paa,node->left_child,index, bsf,pq,lock_queue,workstack);
            }
        }
    }
    isax_node *newnode=popbottom2(workstack);
    if(newnode!=NULL)
    {
        insert_tree_node_m_workstealing(paa,newnode,index, bsf,pq,lock_queue,workstack);
    }
}

void insert_tree_node_m_hybridpqueue_workstealing(float *paa,isax_node *node,isax_index *index,float bsf,pqueue_t **pq,pthread_mutex_t *lock_queue,localStack* workstack,int *tnumber)
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
            pthread_mutex_lock(&lock_queue[*tnumber]);
            pqueue_insert(pq[*tnumber], mindist_result);
            pthread_mutex_unlock(&lock_queue[*tnumber]);
            *tnumber=(*tnumber+1)%N_PQUEUE;
            added_tree_node++;
        }
        else
        {   
            if (node->right_child->isax_cardinalities != NULL)
            {
                pushbottom(workstack, node->right_child);
            }
            if (node->left_child->isax_cardinalities != NULL)
            {
                insert_tree_node_m_hybridpqueue_workstealing(paa,node->left_child,index, bsf,pq,lock_queue,workstack,tnumber);
            }
        }
    }
    isax_node *newnode=popbottom2(workstack);
    if(newnode!=NULL)
    {
        insert_tree_node_m_hybridpqueue_workstealing(paa,newnode,index, bsf,pq,lock_queue,workstack,tnumber);
    }
}


void insert_tree_node_m_hybridpqueue(float *paa,isax_node *node,isax_index *index,float bsf,pqueue_t **pq,pthread_mutex_t *lock_queue,int *tnumber)
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
            pthread_mutex_lock(&lock_queue[*tnumber]);
            pqueue_insert(pq[*tnumber], mindist_result);
           // __sync_fetch_and_add(&LBDcalculationnumber,1);

            pthread_mutex_unlock(&lock_queue[*tnumber]);
            *tnumber=(*tnumber+1)%N_PQUEUE;
            added_tree_node++;
        }
        else
        {   
            if (node->left_child->isax_cardinalities != NULL)
            {
                insert_tree_node_m_hybridpqueue(paa,node->left_child,index, bsf,pq,lock_queue,tnumber);
            }
            if (node->right_child->isax_cardinalities != NULL)
            {
                insert_tree_node_m_hybridpqueue(paa,node->right_child,index,bsf,pq,lock_queue,tnumber);
            }
        }
    }
}

void pushbottom(localStack *stk, isax_node *node) 
{
        stk->val[stk->bottom] = node;
        stk->bottom++;
}

bool isemptyqueue(localStack *stk)
{
    if(stk->top<stk->bottom)
        return true;
    else
        return false;
}
isax_node* poptop2(localStack *stk) 
{
    int localtop=stk->top;
    int localbottom=stk->bottom;
    if(localtop>=localbottom) 
    {
        return NULL;
    }
    else
    {   
        isax_node *localnode=stk->val[localtop];
        int localnewtop=localtop;
        localnewtop++;
        if(__sync_bool_compare_and_swap(&(stk->bottom),localnewtop,stk->bottom))
        {
            return NULL;
        }
        else
        {
            if(__sync_bool_compare_and_swap(&(stk->top),stk->top,localnewtop))
            return localnode;
            else
            return NULL;
        }
    }
}
isax_node* poptop(localStack *stk) 
{
    int localtop=stk->top;
    int localbottom=stk->bottom;
    if(localtop>=localbottom) 
    {
        return NULL;
    }
    else
    {   
        isax_node *localnode=stk->val[localtop];
        int localnewtop=localtop;
        localnewtop++;
        if(__sync_bool_compare_and_swap(&(stk->top),localtop,localnewtop))
        {
            return localnode;
        }
        else
        {
            return NULL;
        }
    }
}
isax_node* popbottom(localStack *stk)
{
    int localbottom=stk->bottom;
    {
        /* code */
    }
    if(localbottom==0)
    {
        //this is the end of the stack
        return NULL;
    }
    localbottom--;
    stk->bottom=localbottom;
    isax_node *localnode=stk->val[localbottom];
    int localtop=stk->top;
    if(localbottom>localtop)
        return localnode;

    stk->bottom=0;
    if(localbottom=localtop)
    {
        __sync_bool_compare_and_swap(&(stk->top),localtop,0);
        if(localtop==0)
        {
            return localnode;
        }
    }
    localtop=0;
    return NULL;

}   



isax_node* popbottom2(localStack *stk)
{
    if(stk->bottom==0)

    {        return NULL;
}
int localbottom=__sync_fetch_and_sub (&(stk->bottom), 1);
    localbottom--;
isax_node *localnode=stk->val[localbottom];
if(__sync_bool_compare_and_swap(&(stk->bottom),stk->top,0))
     {
        stk->top=0;
        return localnode;
}
    else
 {return localnode;
 }
}








