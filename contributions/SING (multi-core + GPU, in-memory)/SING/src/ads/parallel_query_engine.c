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

#include "ads/isax_query_engine.h"
#include "ads/parallel_query_engine.h"
#include "ads/isax_first_buffer_layer.h"
#include "ads/pqueue.h"
#include "ads/sax/sax.h"
#include "ads/isax_node_split.h"
#define NTHREADS 4

void isax_query_binary_file_para(const char *ifilename, int q_num, isax_index *index,
                            float minimum_distance, int min_checked_leaves,
                            query_result (*search_function)(ts_type*, ts_type*, isax_index*, float, int)) {
    fprintf(stderr, ">>> Performing queries in file: %s\n", ifilename);

    FILE * ifile;
    ifile = fopen (ifilename,"rb");
    if (ifile == NULL) {
        fprintf(stderr, "File %s not found!\n", ifilename);
        exit(-1);
    }
    pthread_t threadid[q_num];
    fseek(ifile, 0L, SEEK_END);
    file_position_type sz = (file_position_type) ftell(ifile);
    file_position_type total_records = sz/index->settings->ts_byte_size;
    fseek(ifile, 0L, SEEK_SET);
    if (total_records < q_num) {
        fprintf(stderr, "File %s has only %llu records!\n", ifilename, total_records);
        exit(-1);
    }

    int q_loaded = 0;

    //sax_type * sax = malloc(sizeof(sax_type) * index->settings->paa_segments);
    paraquery paraqueries[q_num];
    pthread_mutex_t lock_index=PTHREAD_MUTEX_INITIALIZER;
            COUNT_TOTAL_TIME_START
        COUNT_OUTPUT2_TIME_START
    while (q_loaded < q_num)
    {
        //printf("Querying for: %d\n", index->settings->ts_byte_size * q_loaded);
        // Parse ts and make PAA representation
        paraqueries[q_loaded].ts= malloc(sizeof(ts_type) * index->settings->timeseries_size);
        paraqueries[q_loaded].paa= malloc(sizeof(ts_type) * index->settings->paa_segments);
        paraqueries[q_loaded].index=index;
        paraqueries[q_loaded].minimum_distance=minimum_distance;
        paraqueries[q_loaded].min_checked_leaves=min_checked_leaves;
        paraqueries[q_loaded].lock_index=&lock_index;
        COUNT_INPUT_TIME_START
        fread(paraqueries[q_loaded].ts, sizeof(ts_type),index->settings->timeseries_size,ifile);
        COUNT_INPUT_TIME_END
        paa_from_ts(paraqueries[q_loaded].ts, paraqueries[q_loaded].paa, index->settings->paa_segments,
                    index->settings->ts_values_per_paa_segment);


        pthread_create(&(threadid[q_loaded]),NULL,para_queries_worker,(void*)&(paraqueries[q_loaded]));
        //query_result result = search_function(ts, paa, index, minimum_distance, min_checked_leaves);

        //PRINT_STATS(result.distance)
        
        fflush(stdout);
#if VERBOSE_LEVEL >= 1
        printf("[%p]: Distance: %lf\n", result.node, result.distance);
#endif
        //sax_from_paa(paa, sax, index->settings->paa_segments, index->settings->sax_alphabet_cardinality, index->settings->sax_bit_cardinality);
        //if (index->settings->timeseries_size * sizeof(ts_type) * q_loaded == 1024) {
        //    sax_print(sax, index->settings->paa_segments, index->settings->sax_bit_cardinality);
        //}

        q_loaded++;
    }
    q_loaded = 0;
    while (q_loaded < q_num)
    {
        pthread_join(threadid[q_loaded],NULL);
        q_loaded++;
    }
        COUNT_OUTPUT2_TIME_END
        COUNT_TOTAL_TIME_END
    //free(paa);
    //free(ts);
    fclose(ifile);
    fprintf(stderr, ">>> Finished querying.\n");

}
void* para_queries_worker(void *transvector)
{
    
    query_result result = exact_search_serial_para(((paraquery*)transvector)->ts, ((paraquery*)transvector)->paa, ((paraquery*)transvector)->index, ((paraquery*)transvector)->minimum_distance, ((paraquery*)transvector)->min_checked_leaves,((paraquery*)transvector)->lock_index);
    PRINT_STATS(result.distance);       

}
query_result exact_search_serial_para(ts_type *ts, ts_type *paa, isax_index *index, float minimum_distance, int min_checked_leaves, pthread_mutex_t *lock_index) {
    
    RESET_BYTES_ACCESSED
    
    // FOR THREAD USE
    float *MINDISTS=malloc(sizeof(float) * index->sax_cache_size);
    unsigned long j;
    for (j = 0; j < index->sax_cache_size; j++)
        MINDISTS[j] = FLT_MAX;
    // END
    pthread_mutex_lock(lock_index); 
    query_result approximate_result = approximate_search(ts, paa, index);
    query_result bsf_result = approximate_result;
    
    
    int tight_bound = index->settings->tight_bound;
    int aggressive_check = index->settings->aggressive_check;
    
    // Early termination...
    if (approximate_result.distance == 0) {
        return approximate_result;
    }
    
    if(approximate_result.distance == FLT_MAX || min_checked_leaves > 1) 
    {
        approximate_result = refine_answer(ts, paa, index, approximate_result, minimum_distance, min_checked_leaves);
    }
    pthread_mutex_unlock(lock_index); 
    
    unsigned long i;
    COUNT_INPUT_TIME_START
    FILE *raw_file = fopen(index->settings->raw_filename, "rb");
    fseek(raw_file, 0, SEEK_SET);
    ts_type *ts_buffer = malloc(index->settings->ts_byte_size);
    COUNT_INPUT_TIME_END
    
    //SET_APPROXIMATE(approximate_result.distance);

    // THREADED
    //for ( i = 0; i < 100; i++)
    //{
    //    printf("the sax [%d ]%d\n",i,(int)(index->sax_cache[i * index->settings->paa_segments] ));
    //}
    for(i=0; i<index->sax_cache_size; i++) 
    {
        sax_type *sax = &(index->sax_cache[i * index->settings->paa_segments]);
        MINDISTS[i] = minidist_paa_to_isax_raw(paa, sax,
                                               index->settings->max_sax_cardinalities,
                                               index->settings->sax_bit_cardinality,
                                               index->settings->sax_alphabet_cardinality,
                                               index->settings->paa_segments, MINVAL, MAXVAL,
                                               index->settings->mindist_sqrt);
    }
    // END
    
    //printf("the mindist 95008 is %f\n",MINDISTS[95008] ); 
    //printf("the mindist 280671 is %f\n",MINDISTS[280671] ); 
    //printf("the mindist 692396 is %f\n",MINDISTS[692396] ); 




    for(i=0; i<index->sax_cache_size; i++) 
    {
        sax_type *sax = &index->sax_cache[i * index->settings->paa_segments];
        if(MINDISTS[i] <= approximate_result.distance) 
        {
            COUNT_INPUT_TIME_START
            fseek(raw_file, i * index->settings->ts_byte_size, SEEK_SET);
            fread(ts_buffer, index->settings->ts_byte_size, 1, raw_file);
            COUNT_INPUT_TIME_END
            //printf(" the %d sax is :  %d !!!\n",i,index->sax_cache[i* index->settings->paa_segments] );
            float dist = ts_euclidean_distance(ts, ts_buffer, index->settings->timeseries_size, approximate_result.distance);
            if(dist < approximate_result.distance) 
            {
                approximate_result.distance = dist;

#ifdef STORE_ANSWER
                memcpy(index->answer, ts_buffer, index->settings->timeseries_size * sizeof(ts_type));
#endif
            }
            //INCREASE_BYTES_ACCESSED(index->settings->ts_byte_size)
        }
    }
    free(ts_buffer);
    fclose(raw_file);
    free(MINDISTS);
    
    return approximate_result;
}
query_result exact_search_serial_ParIS(ts_type *ts, ts_type *paa, isax_index *index, float minimum_distance, int min_checked_leaves) 
{

    RESET_BYTES_ACCESSED


    pthread_t threadid[maxquerythread];
    query_result approximate_result = approximate_search(ts, paa, index);
    ts_type *ts_buffer = malloc(index->settings->ts_byte_size);
    

    int sum_of_lab=0;
    int tight_bound = index->settings->tight_bound;
    int aggressive_check = index->settings->aggressive_check;
    float bsf_distance;
    unsigned long j;
    //printf("check point 1 \n");
    // Early termination...
    if (approximate_result.distance == 0) {
        return approximate_result;
    }
    
    if(approximate_result.distance == FLT_MAX || min_checked_leaves > 1)
    {
        approximate_result = refine_answer(ts, paa, index, approximate_result, minimum_distance, min_checked_leaves);
        //approximate_result = refine_answer_m(ts, paa, index, approximate_result2, minimum_distance, min_checked_leaves);
    }
    query_result bsf_result = approximate_result;
    FILE *raw_file = fopen(index->settings->raw_filename, "rb");
    fseek(raw_file, 0, SEEK_SET);
    //printf("check point 2 \n");
    /*BIT_ARRAY* bitarray = bit_array_create(index->sax_cache_size);*/
    //printf("the old distance is: %f \n",approximate_result.distance);
    unsigned long i;

    #ifdef AUTO_TUNE
    float *mindists = malloc(sizeof(float) * index->sax_cache_size);
    #endif
    COUNT_CAL_TIME_START
    SET_APPROXIMATE(approximate_result.distance);
    //printf("the index->sax_cache_size is %ld\n",index->sax_cache_size);
    ParIS_LDCW_data *essdata=malloc(sizeof(ParIS_LDCW_data)*(maxquerythread));
    pthread_rwlock_t lock_bsf=PTHREAD_RWLOCK_INITIALIZER;
    //pthread_mutex_t lock_bsf2=PTHREAD_MUTEX_INITIALIZER;
    //printf("check point 3 \n");
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
        pthread_create(&(threadid[i]),NULL,mindistance_worker,(void*)&(essdata[i]));
        //printf("index->sax_cache_size is %ld\n", index->sax_cache_size);
        //printf("essdata[i].start_number is %ld\n", essdata[i].start_number);
        //printf("essdata[i].stop_number is %ld\n", essdata[i].stop_number);
    }
    for (i = 0; i < maxquerythread; i++)
    {
        pthread_join(threadid[i],NULL);
        sum_of_lab+=essdata[i].sum_of_lab;
    }

    //printf("check point 4 \n");
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
        COUNT_CAL_TIME_END
    COUNT_INPUT_TIME_START
    for (i = 0; i < maxquerythread*maxreadthread; i++)
    {   
        pthread_create(&(readthread[i]),NULL,read_worker,(void*)&(readpointer));
    }
    //printf("check point 7 \n");
    //wait the read worker finish
    for (i = 0; i < maxquerythread*maxreadthread; i++)
    {   
        pthread_join(readthread[i],NULL);
        //read_time_all=read_time_all+read_time_conter[i];
                //if(readpointer[i].bsf<approximate_result.distance)
    //approximate_result.distance=readpointer[i].bsf;
    }
    COUNT_INPUT_TIME_END
    //printf("check point 8 \n");
    approximate_result.distance=bsfdistance;
    free(essdata);
    free(minidisvector);
    free(label_number);
    fclose(raw_file);
    //printf("I need to check: %2.2lf%% of the data.\n", (double)tocheck*100/(double)index->sax_cache_size);
    /*bit_array_free(bitarray);*/
            //printf("the new distance is: %f \n",approximate_result.distance);
                //.sax_type *sax = &index->sax_cache[1 * index->settings->paa_segments];
    return approximate_result;
}

query_result exact_search_serial_ParISnonsort(ts_type *ts, ts_type *paa, isax_index *index, float minimum_distance, int min_checked_leaves) 
{

    RESET_BYTES_ACCESSED


    pthread_t threadid[maxquerythread];
    query_result approximate_result = approximate_search(ts, paa, index);
    ts_type *ts_buffer = malloc(index->settings->ts_byte_size);
    

    unsigned long sum_of_lab=0;
    int tight_bound = index->settings->tight_bound;
    int aggressive_check = index->settings->aggressive_check;
    float bsf_distance;
    unsigned long j;
    //printf("check point 1 \n");
    // Early termination...
    if (approximate_result.distance == 0) {
        return approximate_result;
    }
    
    if(approximate_result.distance == FLT_MAX || min_checked_leaves > 1)
    {
        approximate_result = refine_answer(ts, paa, index, approximate_result, minimum_distance, min_checked_leaves);
        //approximate_result = refine_answer_m(ts, paa, index, approximate_result2, minimum_distance, min_checked_leaves);
    }
    query_result bsf_result = approximate_result;
    FILE *raw_file = fopen(index->settings->raw_filename, "rb");
    fseek(raw_file, 0, SEEK_SET);
    //printf("check point 2 \n");
    /*BIT_ARRAY* bitarray = bit_array_create(index->sax_cache_size);*/
    //printf("the old distance is: %f \n",approximate_result.distance);
    unsigned long i;

    #ifdef AUTO_TUNE
    float *mindists = malloc(sizeof(float) * index->sax_cache_size);
    #endif
    COUNT_CAL_TIME_START
    SET_APPROXIMATE(approximate_result.distance);
    //printf("the index->sax_cache_size is %ld\n",index->sax_cache_size);
    ParIS_LDCW_data *essdata=malloc(sizeof(ParIS_LDCW_data)*(maxquerythread));
    pthread_rwlock_t lock_bsf=PTHREAD_RWLOCK_INITIALIZER;
    //pthread_mutex_t lock_bsf2=PTHREAD_MUTEX_INITIALIZER;
    //printf("check point 3 \n");
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
        pthread_create(&(threadid[i]),NULL,mindistance_worker,(void*)&(essdata[i]));
        //printf("index->sax_cache_size is %ld\n", index->sax_cache_size);
        //printf("essdata[i].start_number is %ld\n", essdata[i].start_number);
        //printf("essdata[i].stop_number is %ld\n", essdata[i].stop_number);
    }
    for (i = 0; i < maxquerythread; i++)
    {
        pthread_join(threadid[i],NULL);
        sum_of_lab+=essdata[i].sum_of_lab;
    }

    //printf("check point 4 \n");
    unsigned long* label_number=malloc(sizeof(unsigned long)*(sum_of_lab));
    float* minidisvector=malloc(sizeof(float)*(sum_of_lab));
   
    sum_of_lab=0;



    for (i = 0; i < maxquerythread; i++)
    {
        //free(essdata[i].label_number);
        //free(essdata[i].minidisvector);
        essdata[i].currentpositioncounter=&sum_of_lab;
        essdata[i].label_number=label_number;
        essdata[i].minidisvector=minidisvector;
        //m//emcpy(&(label_number[sum_of_lab]),essdata[i].label_number,sizeof(unsigned long)*essdata[i].sum_of_lab);
       // memcpy(&(minidisvector[sum_of_lab]),essdata[i].minidisvector,sizeof(float)*essdata[i].sum_of_lab);

        //sum_of_lab+=essdata[i].sum_of_lab;
    }
        for(i=0; i<maxquerythread; i++) 
    {
        pthread_create(&(threadid[i]),NULL,mindistanceinsert_worker,(void*)&(essdata[i]));
        
    }

        for (i = 0; i < maxquerythread; i++)
    {
        pthread_join(threadid[i],NULL);
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
        COUNT_CAL_TIME_END
    COUNT_INPUT_TIME_START
    for (i = 0; i < maxquerythread*maxreadthread; i++)
    {   
        pthread_create(&(readthread[i]),NULL,read_worker,(void*)&(readpointer));
    }
    //printf("check point 7 \n");
    //wait the read worker finish
    for (i = 0; i < maxquerythread*maxreadthread; i++)
    {   
        pthread_join(readthread[i],NULL);
        //read_time_all=read_time_all+read_time_conter[i];
                //if(readpointer[i].bsf<approximate_result.distance)
    //approximate_result.distance=readpointer[i].bsf;
    }
    COUNT_INPUT_TIME_END
    //printf("check point 8 \n");
    approximate_result.distance=bsfdistance;
    free(essdata);
    free(minidisvector);
    free(label_number);
    fclose(raw_file);
    //printf("I need to check: %2.2lf%% of the data.\n", (double)tocheck*100/(double)index->sax_cache_size);
    /*bit_array_free(bitarray);*/
            //printf("the new distance is: %f \n",approximate_result.distance);
                //.sax_type *sax = &index->sax_cache[1 * index->settings->paa_segments];
    return approximate_result;
}



pqueue_bsf exact_topk_serial_ParIS(ts_type *ts, ts_type *paa, isax_index *index, float minimum_distance, int min_checked_leaves, int k) 
{

    RESET_BYTES_ACCESSED

    FILE *raw_file = fopen(index->settings->raw_filename, "rb");
    fseek(raw_file, 0, SEEK_SET);
    pthread_t threadid[maxquerythread];
    pqueue_bsf *pq_bsf= pqueue_bsf_init(k);
    approximate_topk(ts, paa, index,pq_bsf);
    ts_type *ts_buffer = malloc(index->settings->ts_byte_size);

    int sum_of_lab=0;
    int tight_bound = index->settings->tight_bound;
    int aggressive_check = index->settings->aggressive_check;
    float bsf_distance;
    unsigned long j;
    //printf("check point 1 \n");
    // Early termination...
    if (pq_bsf->knn[k-1] == 0) {
        return *pq_bsf;
    }
    
    if(pq_bsf->knn[k-1] == FLT_MAX  || min_checked_leaves > 1) {
        refine_topk_answer(ts, paa, index, pq_bsf, minimum_distance, min_checked_leaves);
    }
    
    //printf("check point 2 \n");
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
    //printf("check point 3 \n");
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
        pthread_create(&(threadid[i]),NULL,mindistance_worker,(void*)&(essdata[i]));
        //printf("index->sax_cache_size is %ld\n", index->sax_cache_size);
        //printf("essdata[i].start_number is %ld\n", essdata[i].start_number);
        //printf("essdata[i].stop_number is %ld\n", essdata[i].stop_number);
    }
    for (i = 0; i < maxquerythread; i++)
    {
        pthread_join(threadid[i],NULL);
        sum_of_lab+=essdata[i].sum_of_lab;
    }
    //printf("check point 4 \n");
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
    unsigned long readcounter=0;

        
    readpointer.ts=ts;
    readpointer.index=index;
    readpointer.counter=&readcounter;
    readpointer.load_point=label_number;

    readpointer.lock_bsf=&lock_bsf;
    readpointer.minidisvector=minidisvector;
    readpointer.sum_of_lab=sum_of_lab;
    readpointer.pq_bsf=pq_bsf;
    //printf("check point 6 \n");
    unsigned long read_time_conter[maxquerythread*maxreadthread], read_time_all=0;

    for (i = 0; i < maxquerythread*maxreadthread; i++)
    {   
        pthread_create(&(readthread[i]),NULL,topk_read_worker,(void*)&(readpointer));
    }
    //printf("check point 7 \n");
    //wait the read worker finish
    for (i = 0; i < maxquerythread*maxreadthread; i++)
    {   
        pthread_join(readthread[i],NULL);
        //read_time_all=read_time_all+read_time_conter[i];
                //if(readpointer[i].bsf<approximate_result.distance)
    //approximate_result.distance=readpointer[i].bsf;
    }
    //printf("check point 8 \n");
    free(essdata);
    free(minidisvector);
    free(label_number);
    fclose(raw_file);
    //printf("I need to check: %2.2lf%% of the data.\n", (double)tocheck*100/(double)index->sax_cache_size);
    /*bit_array_free(bitarray);*/
            //printf("the new distance is: %f \n",approximate_result.distance);
                //.sax_type *sax = &index->sax_cache[1 * index->settings->paa_segments];
    return *pq_bsf;
}
/*query_result exact_search_serial_new(ts_type *ts, ts_type *paa, isax_index *index, float minimum_distance, int min_checked_leaves) 
{

    RESET_BYTES_ACCESSED

    FILE *raw_file = fopen(index->settings->raw_filename, "rb");
    fseek(raw_file, 0, SEEK_SET);
    pthread_t threadid[maxquerythread];
    query_result approximate_result = approximate_search(ts, paa, index);
    ts_type *ts_buffer = malloc(index->settings->ts_byte_size);
    query_result bsf_result = approximate_result;
    query_result *approximate_result2=malloc(sizeof(query_result));
    approximate_result2->distance=approximate_result.distance;
    approximate_result2->node=approximate_result.node;
    approximate_result2->pqueue_position=approximate_result.pqueue_position;
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
        approximate_result = refine_answer(ts, paa, index, approximate_result, minimum_distance, min_checked_leaves);
    }
    
    /*BIT_ARRAY* bitarray = bit_array_create(index->sax_cache_size);*/
    //printf("the old distance is: %f \n",approximate_result.distance);
    /*unsigned long i;

    #ifdef AUTO_TUNE
    float *mindists = malloc(sizeof(float) * index->sax_cache_size);
    #endif
    
    SET_APPROXIMATE(approximate_result.distance);
    //printf("the index->sax_cache_size is %ld\n",index->sax_cache_size);
    ParIS_LDCW_data *essdata=malloc(sizeof(ParIS_LDCW_data)*(maxquerythread));
    pthread_rwlock_t lock_bsf=PTHREAD_RWLOCK_INITIALIZER;
    pthread_mutex_t lock_bsf2=PTHREAD_MUTEX_INITIALIZER;
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


    for(i=0; i<maxquerythread; i++) {
        pthread_create(&(threadid[i]),NULL,mindistance_worker_paradsplus2,(void*)&(essdata[i]));
        //printf("index->sax_cache_size is %ld\n", index->sax_cache_size);
        //printf("essdata[i].start_number is %ld\n", essdata[i].start_number);
        //printf("essdata[i].stop_number is %ld\n", essdata[i].stop_number);
    }
    for (i = 0; i < maxquerythread; i++)
    {
        pthread_join(threadid[i],NULL);
        sum_of_lab+=essdata[i].sum_of_lab;
    }
    unsigned long* label_number=malloc(sizeof(unsigned long)*(sum_of_lab));
    float* minidisvector=malloc(sizeof(float)*(sum_of_lab));
    int* tsnumvector=malloc(sizeof(int)*(sum_of_lab));
    sum_of_lab=0;

    for (i = 0; i < maxquerythread; i++)
    {
        memcpy(&(label_number[sum_of_lab]),essdata[i].label_number,sizeof(unsigned long)*essdata[i].sum_of_lab);
        memcpy(&(minidisvector[sum_of_lab]),essdata[i].minidisvector,sizeof(float)*essdata[i].sum_of_lab);
        memcpy(&(tsnumvector[sum_of_lab]),essdata[i].ts_number,sizeof(int)*essdata[i].sum_of_lab);
        free(essdata[i].label_number);
        free(essdata[i].minidisvector);
        free(essdata[i].ts_number);
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
    readpointer.ts_number=tsnumvector;
    readpointer.lock_bsf=&lock_bsf2;
    readpointer.bsf2=&bsfdistance;
    readpointer.minidisvector=minidisvector;
    readpointer.sum_of_lab=sum_of_lab;

    for (i = 0; i < maxquerythread*maxreadthread; i++)
    {   
        pthread_create(&(readthread[i]),NULL,readworker_paradsplus2,(void*)&(readpointer));
    }

    for (i = 0; i < maxquerythread*maxreadthread; i++)
    {   
        pthread_join(readthread[i],NULL);

    }
    approximate_result.distance=bsfdistance;

    fclose(raw_file);
    
    free(label_number);
    free(minidisvector);
    free(tsnumvector);


    return approximate_result;
}*/
void* mindistanceinsert_worker(void *essdata)
{   
    unsigned long* localposition=((ParIS_LDCW_data*)essdata)->label_number;
    float* localmindist=((ParIS_LDCW_data*)essdata)->minidisvector;
        isax_index *index=((ParIS_LDCW_data*)essdata)->index;
    unsigned long start_number=((ParIS_LDCW_data*)essdata)->start_number;
    unsigned long stop_number=((ParIS_LDCW_data*)essdata)->stop_number;
    unsigned long i,t;
        float bsfdistance,mindist;
            ts_type *paa=((ParIS_LDCW_data*)essdata)->paa;
    ts_type *ts=((ParIS_LDCW_data*)essdata)->ts;
    for(i=start_number;i<stop_number;i++)
    {

        sax_type *sax = &index->sax_cache[i * index->settings->paa_segments];

        mindist = minidist_paa_to_isax_rawa_SIMD(paa, sax, index->settings->max_sax_cardinalities,
                                                         index->settings->sax_bit_cardinality,
                                                         index->settings->sax_alphabet_cardinality,
                                                         index->settings->paa_segments, MINVAL, MAXVAL,
                                                         index->settings->mindist_sqrt);
        if(mindist <= ((ParIS_LDCW_data*)essdata)->bsfdistance) {
            /*bit_array_set_bit(bitarray, i);*/
            //COUNT_INPUT_TIME_START
            //fseek(raw_file, i * index->settings->ts_byte_size, SEEK_SET);
            //fread(ts_buffer, index->settings->ts_byte_size, 1, raw_file);
            //COUNT_INPUT_TIME_END
             t=__sync_fetch_and_add(((ParIS_LDCW_data*)essdata)->currentpositioncounter,1);

            memcpy(&(((ParIS_LDCW_data*)essdata)->label_number[t]),&i,sizeof(unsigned int));
            memcpy(&( ((ParIS_LDCW_data*)essdata)->minidisvector[t]),&mindist,sizeof(float));
        }
    }



}

void* mindistance_worker(void *essdata)
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
        if(mindist <= ((ParIS_LDCW_data*)essdata)->bsfdistance) {
            /*bit_array_set_bit(bitarray, i);*/
            //COUNT_INPUT_TIME_START
            //fseek(raw_file, i * index->settings->ts_byte_size, SEEK_SET);
            //fread(ts_buffer, index->settings->ts_byte_size, 1, raw_file);
            //COUNT_INPUT_TIME_END
            if ( ((ParIS_LDCW_data*)essdata)->sum_of_lab>=max_number)
            {
                max_number=(max_number+10000);
                unsigned long* change_lab=((ParIS_LDCW_data*)essdata)->label_number;
                float* change_minivec=((ParIS_LDCW_data*)essdata)->minidisvector;
                ((ParIS_LDCW_data*)essdata)->label_number=malloc(sizeof(unsigned long)*(max_number+10000));
                ((ParIS_LDCW_data*)essdata)->minidisvector=malloc(sizeof(float)*(max_number+10000));
                memcpy(((ParIS_LDCW_data*)essdata)->label_number,change_lab,sizeof(unsigned long)*max_number);
                memcpy(((ParIS_LDCW_data*)essdata)->minidisvector,change_minivec,sizeof(float)*max_number);
                
                //((ParIS_LDCW_data*)essdata)->label_number = (unsigned long*) realloc(((ParIS_LDCW_data*)essdata)->label_number, max_number);
                //((ParIS_LDCW_data*)essdata)->minidisvector = (float*) realloc(((ParIS_LDCW_data*)essdata)->minidisvector, max_number);
                free(change_lab);
                free(change_minivec);
                
            }
            ((ParIS_LDCW_data*)essdata)->label_number[((ParIS_LDCW_data*)essdata)->sum_of_lab]=i;
            ((ParIS_LDCW_data*)essdata)->minidisvector[((ParIS_LDCW_data*)essdata)->sum_of_lab]=mindist;
            ((ParIS_LDCW_data*)essdata)->sum_of_lab++;
        }
    }
}
/*void* mindistance_worker_ParIS2(void *essdata)
{    
    
    unsigned long i,sumnumber=0;
    bool continue_juge=false;
    float bsfdistance,mindist;
    isax_index *index=((ParIS_LDCW_data*)essdata)->index;
    unsigned long start_number=((ParIS_LDCW_data*)essdata)->start_number;
    unsigned long stop_number=((ParIS_LDCW_data*)essdata)->stop_number;
    ts_type *paa=((ParIS_LDCW_data*)essdata)->paa;
    ts_type *ts=((ParIS_LDCW_data*)essdata)->ts;
    unsigned long *label_number=malloc(sizeof(unsigned long)*10000);
    int *ts_number=malloc(sizeof(int)*10000);
    float *minidisvector=malloc(sizeof(float)*10000);

    unsigned long max_number=10000;

    for(i=start_number;i<stop_number;i++)
    {

        sax_type *sax = &index->sax_cache[i * index->settings->paa_segments];

        mindist = minidist_paa_to_isax_raw_SIMD(paa, sax, index->settings->max_sax_cardinalities,
                                                         index->settings->sax_bit_cardinality,
                                                         index->settings->sax_alphabet_cardinality,
                                                         index->settings->paa_segments, MINVAL, MAXVAL,
                                                         index->settings->mindist_sqrt);
        if(mindist <= ((ParIS_LDCW_data*)essdata)->bsfdistance) {
            /*bit_array_set_bit(bitarray, i);*/
            //COUNT_INPUT_TIME_START
            //fseek(raw_file, i * index->settings->ts_byte_size, SEEK_SET);
            //fread(ts_buffer, index->settings->ts_byte_size, 1, raw_file);
            //COUNT_INPUT_TIME_END
           /* if ( sumnumber>=max_number)
            {

                unsigned long* change_lab=label_number;
                float* change_minivec=minidisvector;
                int* change_tsnum=ts_number;
                label_number=malloc(sizeof(unsigned long)*(max_number+10000));
                minidisvector=malloc(sizeof(float)*(max_number+10000));
                ts_number=malloc(sizeof(int)*(max_number+10000));
                memcpy(label_number,change_lab,sizeof(unsigned long)*max_number);
                memcpy(minidisvector,change_minivec,sizeof(float)*max_number);
                memcpy(ts_number,change_tsnum,sizeof(int)*max_number);
                free(change_lab);
                free(change_minivec);
                free(change_tsnum);
                max_number=(max_number+10000);
            }

            if(continue_juge)
            {   
                if( ts_number[sumnumber-1]<read_block_length)
                {
                    ts_number[sumnumber-1]=ts_number[sumnumber-1]+1;
                    minidisvector[sumnumber-1]=min(mindist,minidisvector[sumnumber-1]);  
                }
                else
                {
                    label_number[sumnumber]=i;
                    minidisvector[sumnumber]=mindist;
                    ts_number[sumnumber]=1;
                    sumnumber=sumnumber+1; 
                    continue_juge=true;
                }
            }
            else
            {
                label_number[sumnumber]=i;
                minidisvector[sumnumber]=mindist;
                ts_number[sumnumber]=1;
                sumnumber=sumnumber+1; 
                continue_juge=true;
            }
            

            //float dist = ts_euclidean_distance(ts, ts_buffer, index->settings->timeseries_size, ((exact_search_serial_fonction_data*)essdata)->bsfdistance);
            //pthread_rwlock_wrlock(((exact_search_serial_fonction_data*)essdata)->lock_bsf);
            //INCREASE_BYTES_ACCESSED(index->settings->ts_byte_size)
            //pthread_rwlock_unlock(((exact_search_serial_fonction_data*)essdata)->lock_bsf);

        }
        else
        {
            continue_juge=false;
        }
    }
    ((ParIS_LDCW_data*)essdata)->sum_of_lab=sumnumber;
        ((ParIS_LDCW_data*)essdata)->label_number=label_number;
    ((ParIS_LDCW_data*)essdata)->ts_number=ts_number;
    ((ParIS_LDCW_data*)essdata)->minidisvector=minidisvector;
}*/

/*void* read_data_fonction(void *read_pointer)
{   

    isax_index *index=((ParIS_read_worker_data*)read_pointer)->index;
    FILE *raw_file = fopen(index->settings->raw_filename, "rb");
    fseek(raw_file, 0, SEEK_SET);
    ts_type *ts_buffer = malloc(index->settings->ts_byte_size);
    ts_type *ts =((ParIS_read_worker_data*)read_pointer)->ts;
    unsigned long stop_number=((ParIS_read_worker_data*)read_pointer)->stop_number;
    unsigned long start_number=((ParIS_read_worker_data*)read_pointer)->start_number;
    float *minidisvector=((ParIS_read_worker_data*)read_pointer)->minidisvector;
    float bsf= ((ParIS_read_worker_data*)read_pointer)->bsf;
    //printf("the stop number is %d!!!!!!!! \n",stop_number);
    for (int i = start_number; i < stop_number; i++)
    {

        if (minidisvector[i]<bsf)
        {
            unsigned long p=((ParIS_read_worker_data*)read_pointer)->load_point[i];
            fseek(raw_file, p * index->settings->ts_byte_size, SEEK_SET);
            fread(ts_buffer, index->settings->ts_byte_size, 1, raw_file);
            float dist = ts_euclidean_distance(ts, ts_buffer, index->settings->timeseries_size, bsf);
            if(dist < bsf) 
            {
                    bsf= dist;
            }
        }
    //printf("the t is :%ld  !!!!!\n",t);
    }
    ((ParIS_read_worker_data*)read_pointer)->bsf=bsf;
    fclose(raw_file);
}*/
void* read_worker(void *read_pointer)
{
    isax_index *index=((ParIS_read_worker_data*)read_pointer)->index;
    FILE *raw_file = fopen(index->settings->raw_filename, "rb");
    fseek(raw_file, 0, SEEK_SET);
    ts_type *ts_buffer = malloc(index->settings->ts_byte_size);
    ts_type *ts =((ParIS_read_worker_data*)read_pointer)->ts;
    unsigned long t=0,p;
    unsigned long sum_of_lab=((ParIS_read_worker_data*)read_pointer)->sum_of_lab;
    float *minidisvector=((ParIS_read_worker_data*)read_pointer)->minidisvector;
    //printf(" t is %ld\n",*((ParIS_read_worker_data*)read_pointer)->counter);
    //unsigned long read_time_conter=0;

    float bsf,dist;
    while(1)
    { 
        
        pthread_rwlock_rdlock(((ParIS_read_worker_data*)read_pointer)->lock_bsf); 
        bsf= *(((ParIS_read_worker_data*)read_pointer)->bsf2); 
        //printf(" t is %ld\n",*(((ParIS_read_worker_data*)read_pointer)->counter));
 
        pthread_rwlock_unlock(((ParIS_read_worker_data*)read_pointer)->lock_bsf); 
                //t=*(((ParIS_read_worker_data*)read_pointer)->counter); 
        //*(((ParIS_read_worker_data*)read_pointer)->counter)=*(((ParIS_read_worker_data*)read_pointer)->counter)+1;
        t=__sync_fetch_and_add(((ParIS_read_worker_data*)read_pointer)->counter,1);
        //printf("%ld\n", ((ParIS_read_worker_data*)read_pointer)->sum_of_lab);
        if (t>=sum_of_lab) 
        {    
            break; 
        } 
        
         p=((ParIS_read_worker_data*)read_pointer)->load_point[t];
        //printf("t is %ld!!!\n",p );
        if (minidisvector[t]<bsf) 
        {
            
            fseek(raw_file, p * index->settings->ts_byte_size, SEEK_SET); 
            fread(ts_buffer, index->settings->ts_byte_size, 1, raw_file);
            //read_time_conter++;                 
             dist = ts_euclidean_distance_SIMD(ts, ts_buffer, index->settings->timeseries_size, bsf); 
             //printf("the distance is %f!!\n", dist);
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
        //printf("the t is :%ld  !!!!!\n",t); 
    }

    free(ts_buffer);
    fclose(raw_file);

} 
void* topk_read_worker(void *read_pointer)
{
    isax_index *index=((ParIS_read_worker_data*)read_pointer)->index;
    pqueue_bsf *pq_bsf=((ParIS_read_worker_data*)read_pointer)->pq_bsf;
    FILE *raw_file = fopen(index->settings->raw_filename, "rb");
    fseek(raw_file, 0, SEEK_SET);
    ts_type *ts_buffer = malloc(index->settings->ts_byte_size);
    ts_type *ts =((ParIS_read_worker_data*)read_pointer)->ts;
    unsigned long t=0,p;
    unsigned long sum_of_lab=((ParIS_read_worker_data*)read_pointer)->sum_of_lab;
    float *minidisvector=((ParIS_read_worker_data*)read_pointer)->minidisvector;
    //printf(" t is %ld\n",*((ParIS_read_worker_data*)read_pointer)->counter);
    //unsigned long read_time_conter=0;

    float bsf,dist;
    while(1)
    { 
         
        //pthread_rwlock_rdlock(((ParIS_read_worker_data*)read_pointer)->lock_bsf); 
        bsf= pq_bsf->knn[pq_bsf->k-1]; 
        //printf(" t is %ld\n",*(((ParIS_read_worker_data*)read_pointer)->counter));
 
        //pthread_rwlock_unlock(((ParIS_read_worker_data*)read_pointer)->lock_bsf); 
                //t=*(((ParIS_read_worker_data*)read_pointer)->counter); 
        //*(((ParIS_read_worker_data*)read_pointer)->counter)=*(((ParIS_read_worker_data*)read_pointer)->counter)+1;
        t=__sync_fetch_and_add(((ParIS_read_worker_data*)read_pointer)->counter,1);
        //printf("%ld\n", ((ParIS_read_worker_data*)read_pointer)->sum_of_lab);
        if (t>=sum_of_lab) 
        {    
            break; 
        } 
        
         p=((ParIS_read_worker_data*)read_pointer)->load_point[t];
        //printf("t is %ld!!!\n",p );
        if (minidisvector[t]<bsf) 
        {
            fseek(raw_file, p * index->settings->ts_byte_size, SEEK_SET); 
            fread(ts_buffer, index->settings->ts_byte_size, 1, raw_file);
            //read_time_conter++;                 
             dist = ts_euclidean_distance_SIMD(ts, ts_buffer, index->settings->timeseries_size, bsf); 
             //printf("the distance is %f!!\n", dist);
            if(dist <= bsf)  
            {  
                pthread_rwlock_wrlock(((ParIS_read_worker_data*)read_pointer)->lock_bsf); 
                pqueue_bsf_insert(pq_bsf,dist,p,NULL);
                pthread_rwlock_unlock(((ParIS_read_worker_data*)read_pointer)->lock_bsf); 
            } 
        } 
        //printf("the t is :%ld  !!!!!\n",t); 
    }

    free(ts_buffer);
    fclose(raw_file);
    //return read_time_conter;
} 
/*void* readworker_paradsplus2(void *read_pointer)
{
    isax_index *index=((ParIS_read_worker_data*)read_pointer)->index;
    FILE *raw_file = fopen(index->settings->raw_filename, "rb");
    fseek(raw_file, 0, SEEK_SET);
    int i;
    ts_type *ts_buffer = malloc(index->settings->ts_byte_size*read_block_length);
    ts_type *ts =((ParIS_read_worker_data*)read_pointer)->ts;
    unsigned long t=0,p;
    int n;
    unsigned long sum_of_lab=((ParIS_read_worker_data*)read_pointer)->sum_of_lab;
    float *minidisvector=((ParIS_read_worker_data*)read_pointer)->minidisvector;
   //printf(" t is %ld\n",*((ParIS_read_worker_data*)read_pointer)->counter);
    float bsf,dist;

    while(1)
    { 
         
        pthread_mutex_lock(((ParIS_read_worker_data*)read_pointer)->lock_bsf); 
        bsf= *(((ParIS_read_worker_data*)read_pointer)->bsf2); 
        //printf(" t is %ld\n",*(((ParIS_read_worker_data*)read_pointer)->counter));
        t=*(((ParIS_read_worker_data*)read_pointer)->counter); 
        *(((ParIS_read_worker_data*)read_pointer)->counter)=*(((ParIS_read_worker_data*)read_pointer)->counter)+1; 
        pthread_mutex_unlock(((ParIS_read_worker_data*)read_pointer)->lock_bsf); 
        //printf("%ld\n", ((ParIS_read_worker_data*)read_pointer)->sum_of_lab);
        if (t>=sum_of_lab) 
        {    
            break; 
        } 
        
         p=((ParIS_read_worker_data*)read_pointer)->load_point[t];
         n=((ParIS_read_worker_data*)read_pointer)->ts_number[t];
         
         //ts_type *ts_buffer = malloc(index->settings->ts_byte_size*n);
         


        if (minidisvector[t]<bsf) 
        {

            fseek(raw_file, p * index->settings->ts_byte_size, SEEK_SET);

            fread(ts_buffer, index->settings->ts_byte_size, n, raw_file);
            dist = ts_euclidean_distance_SIMD(ts, ts_buffer, index->settings->timeseries_size, bsf);
            for (i = 1; i < n;i++)
            {
                dist = min(dist,ts_euclidean_distance_SIMD(ts, ts_buffer+i*index->settings->timeseries_size, index->settings->timeseries_size, bsf)); 
            }                 

             
	
            if(dist < bsf)  
            {  

                pthread_mutex_lock(((ParIS_read_worker_data*)read_pointer)->lock_bsf); 
                if (dist<*(((ParIS_read_worker_data*)read_pointer)->bsf2)) 
                {    
                    *(((ParIS_read_worker_data*)read_pointer)->bsf2)= dist; 
                } 
                pthread_mutex_unlock(((ParIS_read_worker_data*)read_pointer)->lock_bsf); 
            } 
        }
    } 
    fclose(raw_file);
    free(ts_buffer); 
} */


query_result exact_search_serial_ParIS_nb(ts_type *ts, ts_type *paa, isax_index *index, float minimum_distance, int min_checked_leaves) 
{

    RESET_BYTES_ACCESSED
    pthread_t threadid[maxquerythread];
    query_result approximate_result = approximate_search(ts, paa, index);
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
    //printf("approximate_result.distance is %f\n",approximate_result.distance);
    if(approximate_result.distance == FLT_MAX || min_checked_leaves > 1)
    {
        approximate_result = refine_answer(ts, paa, index, approximate_result, minimum_distance, min_checked_leaves);
        //approximate_result = refine_answer_m(ts, paa, index, approximate_result2, minimum_distance, min_checked_leaves);
    }
    //printf("approximate_result.distance is %f\n",approximate_result.distance);
    
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
    float bsfdistance=approximate_result.distance;
    unsigned long read_time_conter=0;
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
        essdata[i].minidisvector=&bsfdistance;
        essdata[i].read_time_conter=0;
    }
    essdata[maxquerythread-1].index=index;
    essdata[maxquerythread-1].lock_bsf=&lock_bsf;
    essdata[maxquerythread-1].start_number=(maxquerythread-1)*(index->sax_cache_size/maxquerythread);
    essdata[maxquerythread-1].stop_number=index->sax_cache_size;
    essdata[maxquerythread-1].paa=paa;
    essdata[maxquerythread-1].ts=ts;
    essdata[maxquerythread-1].bsfdistance=approximate_result.distance;
    essdata[maxquerythread-1].sum_of_lab=0;
    essdata[maxquerythread-1].minidisvector=&bsfdistance;

    for(i=0; i<maxquerythread; i++) 
    {
        pthread_create(&(threadid[i]),NULL,ParIS_nb_worker,(void*)&(essdata[i]));
    }
    for (i = 0; i < maxquerythread; i++)
    {
        pthread_join(threadid[i],NULL);
        if(essdata[i].bsfdistance<approximate_result.distance)
            approximate_result.distance=essdata[i].bsfdistance;
    }
    free(essdata);
    return approximate_result;
}

void* ParIS_nb_worker(void *essdata)
{    
    unsigned long i;
    float bsfdistance,mindist;

    isax_index *index=((ParIS_LDCW_data*)essdata)->index;
        FILE *raw_file = fopen(index->settings->raw_filename, "rb");
    fseek(raw_file, 0, SEEK_SET);
    ts_type *ts_buffer = malloc(index->settings->ts_byte_size);
    unsigned long start_number=((ParIS_LDCW_data*)essdata)->start_number;
    unsigned long stop_number=((ParIS_LDCW_data*)essdata)->stop_number;
    ts_type *paa=((ParIS_LDCW_data*)essdata)->paa;
    ts_type *ts=((ParIS_LDCW_data*)essdata)->ts;

    for(i=start_number;i<stop_number;i++)
    {

        sax_type *sax = &index->sax_cache[i * index->settings->paa_segments];

        mindist = minidist_paa_to_isax_raw_SIMD(paa, sax, index->settings->max_sax_cardinalities,
                                                         index->settings->sax_bit_cardinality,
                                                         index->settings->sax_alphabet_cardinality,
                                                         index->settings->paa_segments, MINVAL, MAXVAL,
                                                         index->settings->mindist_sqrt);

        if(mindist <= ((ParIS_LDCW_data*)essdata)->bsfdistance) {

            fseek(raw_file, i * index->settings->ts_byte_size, SEEK_SET);
            fread(ts_buffer, index->settings->ts_byte_size, 1, raw_file);
            
            float dist = ts_euclidean_distance_SIMD(ts, ts_buffer, index->settings->timeseries_size, ((ParIS_LDCW_data*)essdata)->bsfdistance);
            if(dist<(((ParIS_LDCW_data*)essdata)->bsfdistance))
            {
                (((ParIS_LDCW_data*)essdata)->bsfdistance)=dist;
            }


        }
    }
    fclose(raw_file);
    free(ts_buffer);


}


query_result refin_answer_m(ts_type *ts, ts_type *paa, isax_index *index,
                            query_result *approximate_bsf_result,
                            float minimum_distance, int limit) 
{
    pthread_t threadid[8];

    refind_answer_fonction_data rfdata;
    pthread_mutex_t lock_queue=PTHREAD_MUTEX_INITIALIZER,lock_current_root_node=PTHREAD_MUTEX_INITIALIZER;
    pthread_rwlock_t lock_bsf=PTHREAD_RWLOCK_INITIALIZER;
    pthread_barrier_t lock_barrier;
    pthread_barrier_init(&lock_barrier, NULL, 4);
    rfdata.paa=paa;
    rfdata.ts=ts;
    
    rfdata.lock_queue=&lock_queue;
    rfdata.lock_current_root_node=&lock_current_root_node;
    rfdata.lock_bsf=&lock_bsf;
    
    rfdata.index=index;
    rfdata.minimum_distance=minimum_distance;
    rfdata.limit=limit/4;
    pqueue_t *pq = pqueue_init(index->settings->root_nodes_size,
                               cmp_pri, get_pri, set_pri, get_pos, set_pos);
    rfdata.pq=pq;
    // Insert all root nodes in heap.
    rfdata.current_root_node = index->first_node;
    rfdata.bsf_result = approximate_bsf_result;
    query_result bsf_result;
    query_result * n;
    
    rfdata.lock_barrier=&lock_barrier;

    for (int i = 0; i < 8; i++)
    {
        pthread_create(&(threadid[i]),NULL,refind_answer_fonction,(void*)&(rfdata));
    }
    for (int i = 0; i < 8; i++)
    {
        pthread_join(threadid[i],NULL);
    }


    // Free the nodes that where not popped.
    while ((n = pqueue_pop(pq)))
    {
        free(n);
    }
    // Free the priority queue.
    pthread_barrier_destroy(&lock_barrier);
    pqueue_free(pq);
    
    bsf_result=*(rfdata.bsf_result);
    //free(rfdata);
    return *(rfdata.bsf_result);
}
void* refind_answer_fonction(void *rfdata)
{
    isax_node *current_root_node;
    query_result *n;
    isax_index *index=((refind_answer_fonction_data*)rfdata)->index;
    ts_type *paa=((refind_answer_fonction_data*)rfdata)->paa;
    ts_type *ts=((refind_answer_fonction_data*)rfdata)->ts;
    pqueue_t *pq=((refind_answer_fonction_data*)rfdata)->pq;
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
    //printf("this is the check point 2 !!!!!!!!!!!!!!!!!!!\n");
    while (1)
    {
        pthread_mutex_lock(((refind_answer_fonction_data*)rfdata)->lock_queue);
        n = pqueue_pop(pq);
        pthread_mutex_unlock(((refind_answer_fonction_data*)rfdata)->lock_queue);

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
                    float mindistance = calculate_minimum_distance(index, n->node, ts, paa);
                    if(mindistance >= bsfdisntance)
                    {
                        free(n);
                        continue;
                    }
                }
                // *** REAL DISTANCE ***
                checks++;
                float distance = calculate_node_distance(index, n->node, ts, bsfdisntance);
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
                if(checks > limit) {
                    pthread_mutex_lock(((refind_answer_fonction_data*)rfdata)->lock_queue);
                    pqueue_insert(pq, n);
                    pthread_mutex_unlock(((refind_answer_fonction_data*)rfdata)->lock_queue);
                    break;
                }
            }
            else {
                // If it is an intermediate node calculate mindist for children
                // and push them in the queue
                if (n->node->left_child->isax_cardinalities != NULL) {
                    if(n->node->left_child->is_leaf && !n->node->left_child->has_partial_data_file && aggressive_check){
                        float distance = calculate_node_distance(index, n->node->left_child, ts, bsfdisntance);
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
                        float distance = calculate_node_distance(index, n->node->right_child, ts, bsfdisntance);
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
           free(n);
        }
    }
}

query_result exact_search_m (ts_type *ts, ts_type *paa, isax_index *index,
                           float minimum_distance, int min_checked_leaves) 
{
    query_result approximate_result = approximate_search_SIMD(ts, paa, index);
    query_result bsf_result = approximate_result;
    int tight_bound = index->settings->tight_bound;
    int aggressive_check = index->settings->aggressive_check;
    int i;
    // Early termination...
    if (approximate_result.distance == 0) {
        return approximate_result;
    }
    if(approximate_result.distance == FLT_MAX || min_checked_leaves > 1) {
        approximate_result = refine_answer(ts, paa, index, approximate_result, minimum_distance, min_checked_leaves);
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
    pthread_barrier_init(&lock_barrier, NULL, maxquerythread);
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
        pthread_create(&(threadid[i]),NULL,exact_search_fonction,(void*)&(rfdata));
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
void* exact_search_fonction(void *rfdata)
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
                    float mindistance = calculate_minimum_distance_SIMD(index, n->node, ts, paa);
                    if(mindistance >= bsfdisntance)
                    {
                        if(n != do_not_remove)
                            free(n);
                        continue;
                    }
                }
                // *** REAL DISTANCE ***
                checks++;
                float distance = calculate_node_distance_SIMD(index, n->node, ts, bsfdisntance);
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
                        float distance = calculate_node_distance_SIMD(index, n->node->left_child, ts, bsfdisntance);
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
                        float distance = calculate_node_distance_SIMD(index, n->node->right_child, ts, bsfdisntance);
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