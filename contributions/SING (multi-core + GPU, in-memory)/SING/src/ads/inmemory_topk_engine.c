#include <float.h>
#include "../../config.h"
#include "../../globals.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <pthread.h>
#include <stdbool.h>
#include "ads/isax_query_engine.h"
#include "ads/inmemory_query_engine.h"
#include "ads/parallel_query_engine.h"
#include "ads/isax_first_buffer_layer.h"
#include "ads/pqueue.h"
#include "ads/sax/sax.h"
#include "ads/isax_node_split.h"
#include "ads/inmemory_topk_engine.h"
#include "omp.h"  
#include "ads/parallel_inmemory_query_engine.h"
float *MINDISTS;
#define NTHREADS 4
void  approximate_topk_inmemory (ts_type *ts, ts_type *paa, isax_index *index,pqueue_bsf *pq_bsf) 
{

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
        calculate_node_topk_inmemory(index, node, ts, pq_bsf);
    }
    else {

    }
    for (int i = 0; i < pq_bsf->k-1; ++i)
    {
        pq_bsf->knn[i]=pq_bsf->knn[pq_bsf->k-1];
    }
    free(sax);
}

void refine_topk_answer_inmemory (ts_type *ts, ts_type *paa, isax_index *index, pqueue_bsf *pq_bsf,
                            float minimum_distance, int limit) 
{

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
                    //Split and push again in the queue
                    split_node(index, n->node);
                    pqueue_insert(pq, n);
                    continue;
                }
                // *** EXTRA BOUNDING ***
                if(tight_bound) {
                    j++;
                    float mindistance = calculate_minimum_distance_inmemory(index, n->node, ts, paa);

                    if(mindistance >= pq_bsf->knn[pq_bsf->k-1])
                    {
                        free(n);
                        continue;
                    }
                }
                // *** REAL DISTANCE ***
                checks++;
                calculate_node_topk_inmemory(index, n->node, ts, pq_bsf);

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
                        calculate_node_topk_inmemory(index, n->node->left_child, ts, pq_bsf);
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
                        calculate_node_topk_inmemory(index, n->node->right_child, ts, pq_bsf);

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
    for (int i = 0; i < pq_bsf->k-1; ++i)
    {
        pq_bsf->knn[i]=pq_bsf->knn[pq_bsf->k-1];
    }
    // Free the priority queue.

    pqueue_free(pq);
}


void calculate_node_topk_inmemory (isax_index *index, isax_node *node, ts_type *query, pqueue_bsf *pq_bsf) 
{
    COUNT_CHECKED_NODE()
    // If node has buffered data
    if (node->buffer != NULL) 
    {
        int i;
        for (i=0; i<node->buffer->full_buffer_size; i++) 
        {
            float dist = ts_euclidean_distance_SIMD(query, node->buffer->full_ts_buffer[i], 
                                               index->settings->timeseries_size, pq_bsf->knn[pq_bsf->k-1]);
            if (dist <= pq_bsf->knn[pq_bsf->k-1]) {
                pqueue_bsf_insert(pq_bsf,dist,0,node);
            }
        }

        for (i=0; i<node->buffer->tmp_full_buffer_size; i++) {
            float dist = ts_euclidean_distance_SIMD(query, node->buffer->tmp_full_ts_buffer[i], 
                                               index->settings->timeseries_size, pq_bsf->knn[pq_bsf->k-1]);
        
            if (dist <= pq_bsf->knn[pq_bsf->k-1]) {
                pqueue_bsf_insert(pq_bsf,dist,0,node);
            }
        }
        for (i=0; i<node->buffer->partial_buffer_size; i++) {
                                 
            float dist = ts_euclidean_distance_SIMD(query, &(rawfile[*node->buffer->partial_position_buffer[i]]), 
                                               index->settings->timeseries_size, pq_bsf->knn[pq_bsf->k-1]);
            if (dist <= pq_bsf->knn[pq_bsf->k-1]) {
                pqueue_bsf_insert(pq_bsf,dist,*node->buffer->partial_position_buffer[i]/index->settings->timeseries_size,node);
            }
            
        }
    }
    //////////////////////////////////////
    
}
void calculate_node2_topk_inmemory (isax_index *index, isax_node *node, ts_type *query,ts_type *paa, pqueue_bsf *pq_bsf, pthread_rwlock_t *lock_queue ) 
{
    COUNT_CHECKED_NODE()
    // If node has buffered data
    if (node->buffer != NULL) 
    {
        int i;
        for (i=0; i<node->buffer->full_buffer_size; i++) 
        {
            float dist = ts_euclidean_distance_SIMD(query, node->buffer->full_ts_buffer[i], 
                                               index->settings->timeseries_size, pq_bsf->knn[pq_bsf->k-1]);
            if (dist <= pq_bsf->knn[pq_bsf->k-1]) {
                pthread_rwlock_wrlock(lock_queue);
                pqueue_bsf_insert(pq_bsf,dist,0,node);
                pthread_rwlock_unlock(lock_queue);
            }
        }

        for (i=0; i<node->buffer->tmp_full_buffer_size; i++) {
            float dist = ts_euclidean_distance_SIMD(query, node->buffer->tmp_full_ts_buffer[i], 
                                               index->settings->timeseries_size, pq_bsf->knn[pq_bsf->k-1]);
            if (dist <= pq_bsf->knn[pq_bsf->k-1]) {
                pthread_rwlock_wrlock(lock_queue);
                pqueue_bsf_insert(pq_bsf,dist,0,node);
                pthread_rwlock_unlock(lock_queue);
            }
        }
        for (i=0; i<node->buffer->partial_buffer_size; i++) {
            float distmin=minidist_paa_to_isax_rawa_SIMD(paa, node->buffer->partial_sax_buffer[i],
                                               index->settings->max_sax_cardinalities,
                                               index->settings->sax_bit_cardinality,
                                               index->settings->sax_alphabet_cardinality,
                                               index->settings->paa_segments, MINVAL, MAXVAL,
                                               index->settings->mindist_sqrt);
            if (distmin<= pq_bsf->knn[pq_bsf->k-1])
            {                                   
            float dist = ts_euclidean_distance_SIMD(query, &(rawfile[*node->buffer->partial_position_buffer[i]]), 
                                               index->settings->timeseries_size, pq_bsf->knn[pq_bsf->k-1]);
            if (dist <= pq_bsf->knn[pq_bsf->k-1]) {
                pthread_rwlock_wrlock(lock_queue);
                COUNT_QUEUE_TIME_START
                pqueue_bsf_insert(pq_bsf,dist,*node->buffer->partial_position_buffer[i]/index->settings->timeseries_size,node);
                COUNT_QUEUE_TIME_END
                pthread_rwlock_unlock(lock_queue);
            }
            }
        }
    }
    //////////////////////////////////////
    
}

void calculate_node_cal_topk_inmemory (isax_index *index, isax_node *node, ts_type *query,ts_type *paa, pqueue_bsf *pq_bsf, pthread_rwlock_t *lock_queue ) 
{
    COUNT_CHECKED_NODE()
    // If node has buffered data
    if (node->buffer != NULL) 
    {
        int i;
        for (i=0; i<node->buffer->full_buffer_size; i++) 
        {
            float dist = ts_euclidean_distance_SIMD(query, node->buffer->full_ts_buffer[i], 
                                               index->settings->timeseries_size, pq_bsf->knn[pq_bsf->k-1]);
            if (dist <= pq_bsf->knn[pq_bsf->k-1]) {
                pthread_rwlock_wrlock(lock_queue);
                pqueue_bsf_insert(pq_bsf,dist,0,node);
                pthread_rwlock_unlock(lock_queue);
            }
        }

        for (i=0; i<node->buffer->tmp_full_buffer_size; i++) {
            float dist = ts_euclidean_distance_SIMD(query, node->buffer->tmp_full_ts_buffer[i], 
                                               index->settings->timeseries_size, pq_bsf->knn[pq_bsf->k-1]);
            if (dist <= pq_bsf->knn[pq_bsf->k-1]) {
                pthread_rwlock_wrlock(lock_queue);
                pqueue_bsf_insert(pq_bsf,dist,0,node);
                pthread_rwlock_unlock(lock_queue);
            }
        }
        for (i=0; i<node->buffer->partial_buffer_size; i++) {
            float distmin=minidist_paa_to_isax_raw_SING_SIMD(paa, node->buffer->partial_sax_buffer[i],
                                               index->settings->max_sax_cardinalities,
                                               index->settings->sax_bit_cardinality,
                                               index->settings->sax_alphabet_cardinality,
                                               index->settings->paa_segments, MINVAL, MAXVAL,
                                               index->settings->mindist_sqrt);
            if (distmin<= pq_bsf->knn[pq_bsf->k-1])
            {                                   
            float dist = ts_euclidean_distance_SIMD(query, &(rawfile[*node->buffer->partial_position_buffer[i]]), 
                                               index->settings->timeseries_size, pq_bsf->knn[pq_bsf->k-1]);
            if (dist <= pq_bsf->knn[pq_bsf->k-1]) {
                pthread_rwlock_wrlock(lock_queue);
                COUNT_QUEUE_TIME_START
                pqueue_bsf_insert(pq_bsf,dist,*node->buffer->partial_position_buffer[i]/index->settings->timeseries_size,node);
                COUNT_QUEUE_TIME_END
                pthread_rwlock_unlock(lock_queue);
            }
            }
        }
    }
    //////////////////////////////////////
    
}

pqueue_bsf exact_search_serial_topk_inmemory(ts_type *ts, ts_type *paa, isax_index *index, float minimum_distance, int min_checked_leaves, int k) 
{
    
    RESET_BYTES_ACCESSED

    // FOR THREAD USE
    COUNT_INPUT_TIME_START
    MINDISTS=malloc(sizeof(float) * index->sax_cache_size);
    pqueue_bsf *pq_bsf= pqueue_bsf_init(k);
    unsigned long j;
    for (j = 0; j < index->sax_cache_size; j++)
        MINDISTS[j] = FLT_MAX;
    // END

    approximate_topk_inmemory(ts, paa, index,pq_bsf);
    
    int tight_bound = index->settings->tight_bound;
    int aggressive_check = index->settings->aggressive_check;
    
    // Early termination...
    
    if(pq_bsf->knn[k-1] == FLT_MAX || min_checked_leaves > 1) {
        refine_topk_answer_inmemory(ts, paa, index, pq_bsf, minimum_distance, min_checked_leaves);
    }
    printf("the bsf is %f\n",pq_bsf->knn[k-1] ); 
    COUNT_INPUT_TIME_END
     
    
    
    unsigned long i;
    //FILE *raw_file = fopen(index->settings->raw_filename, "rb");
    //fseek(raw_file, 0, SEEK_SET);
    ts_type *ts_buffer;
    COUNT_INPUT_TIME_END
    
    //SET_APPROXIMATE(approximate_result.distance);

    // THREADED
    COUNT_CAL_TIME_START
    pthread_t thread[NTHREADS];
    struct args_in arguments[NTHREADS];

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
        int ret = pthread_create(&thread[i], NULL, compute_mindists_in, &arguments[i]);
    }
    
    for(i=0; i<NTHREADS;i++) {
        pthread_join(thread[i], NULL);
    }
    COUNT_CAL_TIME_END
    // END
    COUNT_OUTPUT_TIME_START
    for(i=0; i<index->sax_cache_size; i++) {
        sax_type *sax = &index->sax_cache[i * index->settings->paa_segments];
        if(MINDISTS[i] <= pq_bsf->knn[k-1]) {
            ts_buffer=&rawfile[i*index->settings->timeseries_size];
            float dist = ts_euclidean_distance(ts, ts_buffer, index->settings->timeseries_size, pq_bsf->knn[pq_bsf->k-1]);
            INCREASE_BYTES_ACCESSED(1)
            if (dist < pq_bsf->knn[pq_bsf->k-1]) {
                pqueue_bsf_insert(pq_bsf,dist,i,NULL);
            }
        }
    }
    free(MINDISTS);
    COUNT_OUTPUT_TIME_END
    return *pq_bsf;
}


pqueue_bsf exact_topk_MESSImq_inmemory (ts_type *ts, ts_type *paa, isax_index *index,node_list *nodelist,
                           float minimum_distance, int min_checked_leaves,int k) 
{
    
    pqueue_bsf *pq_bsf= pqueue_bsf_init(k);
    approximate_topk_inmemory(ts, paa, index, pq_bsf);
    //query_result approximate_result = approximate_search_inmemory(ts, paa, index);
    
    int tight_bound = index->settings->tight_bound;
    int aggressive_check = index->settings->aggressive_check;
    int node_counter=0;
    // Early termination...


    if(pq_bsf->knn[k-1] == FLT_MAX || min_checked_leaves > 1) {
        refine_topk_answer_inmemory(ts, paa, index, pq_bsf, minimum_distance, min_checked_leaves);
    }
    pqueue_t **allpq=malloc(sizeof(pqueue_t*)*N_PQUEUE);


    pthread_mutex_t ququelock[N_PQUEUE];
    int queuelabel[N_PQUEUE];
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
        workerdata[i].lock_barrier=&lock_barrier;
        workerdata[i].alllock=ququelock;
        workerdata[i].allqueuelabel=queuelabel;
        workerdata[i].allpq=allpq;
        workerdata[i].startqueuenumber=i%N_PQUEUE;
        workerdata[i].pq_bsf=pq_bsf;
    }
    

   
    query_result * n;
    
    
    for (int i = 0; i < maxquerythread; i++)
    {
        pthread_create(&(threadid[i]),NULL,exact_topk_worker_inmemory_hybridpqueue,(void*)&(workerdata[i]));
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

    //free(rfdata);
    return *pq_bsf;

    // Free the nodes that where not popped.

}


void* exact_topk_worker_inmemory_hybridpqueue(void *rfdata)
{
    isax_node *current_root_node;
    query_result *n;
    isax_index *index=((MESSI_workerdata*)rfdata)->index;
    ts_type *paa=((MESSI_workerdata*)rfdata)->paa;
    ts_type *ts=((MESSI_workerdata*)rfdata)->ts;
    pqueue_t *pq=((MESSI_workerdata*)rfdata)->pq;
    query_result *do_not_remove = ((MESSI_workerdata*)rfdata)->bsf_result;
    float minimum_distance=((MESSI_workerdata*)rfdata)->minimum_distance;
    pqueue_bsf *pq_bsf=((MESSI_workerdata*)rfdata)->pq_bsf;
    int limit=((MESSI_workerdata*)rfdata)->limit;
    int checks = 0;
    bool finished=true;
    int current_root_node_number;
    int tight_bound = index->settings->tight_bound;
    int aggressive_check = index->settings->aggressive_check;
    float bsfdisntance=pq_bsf->knn[pq_bsf->k-1];
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

            insert_tree_node_m_hybridpqueue(paa,current_root_node,index,bsfdisntance,((MESSI_workerdata*)rfdata)->allpq,((MESSI_workerdata*)rfdata)->alllock,&tnumber);
            //insert_tree_node_mW(paa,current_root_node,index,bsfdisntance,pq,((MESSI_workerdata*)rfdata)->lock_queue);

            
    }

    //COUNT_QUEUE_TIME_END
    //calculate_node_quque=pq->size;
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
        bsfdisntance=pq_bsf->knn[pq_bsf->k-1];
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
                //float distance = calculate_node_distance2_inmemory(index, n->node, ts,paa, bsfdisntance);
                calculate_node2_topk_inmemory(index, n->node, ts,paa, pq_bsf,((MESSI_workerdata*)rfdata)->lock_bsf);


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
                            //float distance = calculate_node_distance2_inmemory(index, n->node, ts,paa, bsfdisntance);
                            calculate_node2_topk_inmemory(index, n->node, ts,paa, pq_bsf,((MESSI_workerdata*)rfdata)->lock_bsf);

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
    //worker_total_time += workercurenttime.tv_sec*1000000 + (workercurenttime.tv_usec)-workertimestart.tv_sec*1000000 - (workertimestart.tv_usec); 
    //printf("create pq time is %f \n",worker_total_time );
    //printf("the worker's write  time is %f \n",worker_total_time );
    //printf("the check's node is\t %d\tthe local queue's node is\t%d\n",checks,calculate_node_quque);
}