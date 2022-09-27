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
#include "ads/inmemory_index_engine.h"
#include "ads/inmemory_query_engine.h"
#include "ads/parallel_index_engine.h"
#include "ads/isax_first_buffer_layer.h"
#include "ads/pqueue.h"
#include "ads/sax/sax.h"
#include "ads/isax_node_split.h"


void index_generate_inmemory(const char *ifilename, long int ts_num, isax_index *index)
{
	fprintf(stderr, ">>> Indexing: %s\n", ifilename);
    FILE * ifile;
    COUNT_INPUT_TIME_START
	ifile = fopen (ifilename,"rb");
    COUNT_INPUT_TIME_END
    if (ifile == NULL) {
        fprintf(stderr, "File %s not found!\n", ifilename);
        exit(-1);
    }
    fseek(ifile, 0L, SEEK_END);
    file_position_type sz = (file_position_type) ftell(ifile);
    file_position_type total_records = sz/index->settings->ts_byte_size;
    fseek(ifile, 0L, SEEK_SET);

    if (total_records < ts_num) {
        fprintf(stderr, "File %s has only %llu records!\n", ifilename, total_records);
        exit(-1);
    }

    long int ts_loaded = 0;
    rawfile=malloc(sizeof(ts_type) * index->settings->timeseries_size*ts_num);
    index->sax_cache= malloc(sizeof(sax_type) * index->settings->paa_segments*ts_num);
    ts_type * ts = malloc(sizeof(ts_type) * index->settings->timeseries_size);
    sax_type * sax = malloc(sizeof(sax_type) * index->settings->paa_segments);
    file_position_type * pos = malloc(sizeof(file_position_type));
    index->settings->raw_filename = malloc(256);
    strcpy(index->settings->raw_filename, ifilename);
    COUNT_INPUT_TIME_START
    int read_number=fread(rawfile, sizeof(ts_type), index->settings->timeseries_size*ts_num, ifile);
    COUNT_INPUT_TIME_END
    COUNT_OUTPUT_TIME_START
    while (ts_loaded<ts_num)
    {

        *pos = (file_position_type)(ts_loaded*index->settings->timeseries_size);
        //printf("the position is: %lld\n",*pos);
        
        memcpy(ts,&(rawfile[ts_loaded*index->settings->timeseries_size]), sizeof(float)* index->settings->timeseries_size);

        if(sax_from_ts(ts, sax, index->settings->ts_values_per_paa_segment,
                       index->settings->paa_segments, index->settings->sax_alphabet_cardinality,
                       index->settings->sax_bit_cardinality) == SUCCESS)
        {
            memcpy(&(index->sax_cache[ts_loaded*index->settings->paa_segments]),sax, sizeof(sax_type)* index->settings->paa_segments);
            isax_fbl_index_insert_inmemory(index, sax, pos);
            ts_loaded++;
        }
        else
        {
            fprintf(stderr, "error: cannot insert record in index, since sax representation\
                    failed to be created");
        }
	}
    free(ts);
    free(sax);
    free(pos);
	fclose(ifile);
    fprintf(stderr, ">>> Finished indexing\n");
        //printf(" the sax point is %d\n",index->first_node->isax_cardinalities[0]);
    COUNT_OUTPUT_TIME_END
}
void index_generate_inmemory_m(const char *ifilename, long int ts_num, isax_index *index)
{
    fprintf(stderr, ">>> Indexing: %s\n", ifilename);
    FILE * ifile;
    COUNT_INPUT_TIME_START
    ifile = fopen (ifilename,"rb");
    COUNT_INPUT_TIME_END
    if (ifile == NULL) {
        fprintf(stderr, "File %s not found!\n", ifilename);
        exit(-1);
    }
    fseek(ifile, 0L, SEEK_END);
    file_position_type sz = (file_position_type) ftell(ifile);
    file_position_type total_records = sz/index->settings->ts_byte_size;
    fseek(ifile, 0L, SEEK_SET);

    if (total_records < ts_num) {
        fprintf(stderr, "File %s has only %llu records!\n", ifilename, total_records);
        exit(-1);
    }
    index->sax_file=NULL;

    long int ts_loaded = 0;
    int i;
    pthread_t threadid[maxquerythread];
    buffer_data_inmemory *input_data=malloc(sizeof(buffer_data_inmemory)*(maxquerythread));
    rawfile=malloc(sizeof(ts_type) * index->settings->timeseries_size*ts_num);
    index->sax_cache= malloc(sizeof(sax_type) * index->settings->paa_segments*ts_num);
    

    index->settings->raw_filename = malloc(256);
    strcpy(index->settings->raw_filename, ifilename);
    COUNT_INPUT_TIME_START
    int read_number=fread(rawfile, sizeof(ts_type), index->settings->timeseries_size*ts_num, ifile);
    COUNT_INPUT_TIME_END
    pthread_mutex_t lock_record=PTHREAD_MUTEX_INITIALIZER,lockfbl=PTHREAD_MUTEX_INITIALIZER,lock_index=PTHREAD_MUTEX_INITIALIZER,
                    lock_firstnode=PTHREAD_MUTEX_INITIALIZER,lock_disk=PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_t *lockcbl;
    // set the thread on decided cpu
    COUNT_OUTPUT_TIME_START
    lockcbl= malloc(sizeof(pthread_mutex_t)*LOCK_SIZE);

    for(i=0; i<LOCK_SIZE ;i++)
    {
        pthread_mutex_init(&lockcbl[i], NULL);
    }

    

    
    for ( i = 0; i < maxquerythread; i++)
    {   
        input_data[i].index=index;
        input_data[i].lock_fbl=&lockfbl;
        input_data[i].lock_record=&lock_record;
        input_data[i].lock_cbl=lockcbl;
        input_data[i].lock_firstnode =&lock_firstnode;
        input_data[i].lock_index=&lock_index;
        input_data[i].ts=rawfile;
        input_data[i].lock_disk=&lock_disk;
        input_data[i].start_number=i*(ts_num/maxquerythread);
        input_data[i].stop_number=(i+1)*(ts_num/maxquerythread);
    }

    input_data[maxquerythread-1].start_number=(maxquerythread-1)*(ts_num/maxquerythread);
    input_data[maxquerythread-1].stop_number=ts_num;
    for (i = 0; i < maxquerythread; i++)
    {
        pthread_create(&(threadid[i]),NULL,indexbulkloadingworker_inmemory,(void*)&(input_data[i]));
    }
    //wait for the finish of other threads
    for (i = 0; i < maxquerythread; i++)
    {
        pthread_join(threadid[i],NULL);
    }
    fclose(ifile);
    fprintf(stderr, ">>> Finished indexing\n");
    free(lockcbl);
    free(input_data);
        //printf(" the sax point is %d\n",index->first_node->isax_cardinalities[0]);
    COUNT_OUTPUT_TIME_END
}
void index_creation_m(const char *ifilename, long int ts_num, isax_index *index)
{
    fprintf(stderr, ">>> Indexing: %s\n", ifilename);
    FILE * ifile;
    COUNT_INPUT_TIME_START
    ifile = fopen (ifilename,"rb");
    COUNT_INPUT_TIME_END
    if (ifile == NULL) {
        fprintf(stderr, "File %s not found!\n", ifilename);
        exit(-1);
    }
    fseek(ifile, 0L, SEEK_END);
    file_position_type sz = (file_position_type) ftell(ifile);
    file_position_type total_records = sz/index->settings->ts_byte_size;
    fseek(ifile, 0L, SEEK_SET);

    if (total_records < ts_num) {
        fprintf(stderr, "File %s has only %llu records!\n", ifilename, total_records);
        exit(-1);
    }
    index->sax_file=NULL;

    long int ts_loaded = 0;
    int i,node_counter=0;
    pthread_t threadid[maxquerythread];
    buffer_data_inmemory *input_data=malloc(sizeof(buffer_data_inmemory)*(maxquerythread));
    rawfile=malloc(sizeof(ts_type) * index->settings->timeseries_size*ts_num);
    index->sax_cache= malloc(sizeof(sax_type) * index->settings->paa_segments*ts_num);
    

    index->settings->raw_filename = malloc(256);
    strcpy(index->settings->raw_filename, ifilename);
    COUNT_INPUT_TIME_START
    int read_number=fread(rawfile, sizeof(ts_type), index->settings->timeseries_size*ts_num, ifile);
    COUNT_INPUT_TIME_END
    pthread_mutex_t lock_record=PTHREAD_MUTEX_INITIALIZER,lockfbl=PTHREAD_MUTEX_INITIALIZER,lock_index=PTHREAD_MUTEX_INITIALIZER,
                    lock_firstnode=PTHREAD_MUTEX_INITIALIZER,lock_disk=PTHREAD_MUTEX_INITIALIZER;
            pthread_mutex_t *lockcbl;
    pthread_barrier_t lock_barrier1;
    pthread_barrier_init(&lock_barrier1, NULL, maxquerythread);
    // set the thread on decided cpu

    lockcbl= malloc(sizeof(pthread_mutex_t)*LOCK_SIZE);

    for(i=0; i<LOCK_SIZE ;i++)
    {
        pthread_mutex_init(&lockcbl[i], NULL);
    }

    COUNT_OUTPUT_TIME_START

    
    for ( i = 0; i < maxquerythread; i++)
    {   
        input_data[i].index=index;
        input_data[i].lock_fbl=&lockfbl;
        input_data[i].lock_record=&lock_record;
        input_data[i].lock_cbl=lockcbl;
        input_data[i].lock_firstnode =&lock_firstnode;
        input_data[i].lock_index=&lock_index;
        input_data[i].ts=rawfile;
        input_data[i].lock_disk=&lock_disk;
        input_data[i].start_number=i*(ts_num/maxquerythread);
        input_data[i].stop_number=(i+1)*(ts_num/maxquerythread);
        input_data[i].node_counter=&node_counter;
        input_data[i].lock_barrier1=&lock_barrier1;
    }

    input_data[maxquerythread-1].start_number=(maxquerythread-1)*(ts_num/maxquerythread);
    input_data[maxquerythread-1].stop_number=ts_num;
    for (i = 0; i < maxquerythread; i++)
    {
        pthread_create(&(threadid[i]),NULL,index_creation_worker_inmemory,(void*)&(input_data[i]));
    }
    //wait for the finish of other threads
    for (i = 0; i < maxquerythread; i++)
    {
        pthread_join(threadid[i],NULL);
    }
    fclose(ifile);
    fprintf(stderr, ">>> Finished indexing\n");
    free(lockcbl);
    free(input_data);
    pthread_barrier_destroy(&lock_barrier1);
        //printf(" the sax point is %d\n",index->first_node->isax_cardinalities[0]);
    COUNT_OUTPUT_TIME_END
}
void index_creation_m_new(const char *ifilename, long int ts_num, isax_index *index)
{
    fprintf(stderr, ">>> Indexing: %s\n", ifilename);
    FILE * ifile;
    COUNT_INPUT_TIME_START
    ifile = fopen (ifilename,"rb");
    COUNT_INPUT_TIME_END
    if (ifile == NULL) {
        fprintf(stderr, "File %s not found!\n", ifilename);
        exit(-1);
    }
    fseek(ifile, 0L, SEEK_END);
    file_position_type sz = (file_position_type) ftell(ifile);
    file_position_type total_records = sz/index->settings->ts_byte_size;
    fseek(ifile, 0L, SEEK_SET);

    if (total_records < ts_num) {
        fprintf(stderr, "File %s has only %llu records!\n", ifilename, total_records);
        exit(-1);
    }
    index->sax_file=NULL;
    unsigned long shared_start_number=0;
    long int ts_loaded = 0;
    int i,node_counter=0;
    pthread_t threadid[maxquerythread];
    buffer_data_inmemory *input_data=malloc(sizeof(buffer_data_inmemory)*(maxquerythread));
    rawfile=malloc(sizeof(ts_type) * index->settings->timeseries_size*ts_num);
    index->sax_cache= malloc(sizeof(sax_type) * index->settings->paa_segments*ts_num);
    

    index->settings->raw_filename = malloc(256);
    strcpy(index->settings->raw_filename, ifilename);
    COUNT_INPUT_TIME_START
    int read_number=fread(rawfile, sizeof(ts_type), index->settings->timeseries_size*ts_num, ifile);
    COUNT_INPUT_TIME_END
    pthread_mutex_t lock_record=PTHREAD_MUTEX_INITIALIZER,lockfbl=PTHREAD_MUTEX_INITIALIZER,lock_index=PTHREAD_MUTEX_INITIALIZER,
                    lock_firstnode=PTHREAD_MUTEX_INITIALIZER,lock_disk=PTHREAD_MUTEX_INITIALIZER;
            pthread_mutex_t *lockcbl;
    pthread_barrier_t lock_barrier1;
    pthread_barrier_init(&lock_barrier1, NULL, maxquerythread);
    // set the thread on decided cpu

    lockcbl= malloc(sizeof(pthread_mutex_t)*LOCK_SIZE);

    for(i=0; i<LOCK_SIZE ;i++)
    {
        pthread_mutex_init(&lockcbl[i], NULL);
    }

    COUNT_OUTPUT_TIME_START

    
    for ( i = 0; i < maxquerythread; i++)
    {   
        input_data[i].index=index;
        input_data[i].lock_fbl=&lockfbl;
        input_data[i].lock_record=&lock_record;
        input_data[i].lock_cbl=lockcbl;
        input_data[i].lock_firstnode =&lock_firstnode;
        input_data[i].lock_index=&lock_index;
        input_data[i].ts=rawfile;
        input_data[i].lock_disk=&lock_disk;
        input_data[i].start_number=i*(ts_num/maxquerythread);
        input_data[i].stop_number=ts_num;
        input_data[i].node_counter=&node_counter;
        input_data[i].lock_barrier1=&lock_barrier1;
        input_data[i].shared_start_number=&shared_start_number;

    }

    for (i = 0; i < maxquerythread; i++)
    {
        pthread_create(&(threadid[i]),NULL,index_creation_worker_inmemory_new,(void*)&(input_data[i]));
    }
    //wait for the finish of other threads
    for (i = 0; i < maxquerythread; i++)
    {
        pthread_join(threadid[i],NULL);
    }
    fclose(ifile);
    fprintf(stderr, ">>> Finished indexing\n");
    free(lockcbl);
    free(input_data);
    pthread_barrier_destroy(&lock_barrier1);
        //printf(" the sax point is %d\n",index->first_node->isax_cardinalities[0]);
    COUNT_OUTPUT_TIME_END
}



void index_creation_m2(const char *ifilename, long int ts_num, isax_index *index)
{
    fprintf(stderr, ">>> Indexing: %s\n", ifilename);
    FILE * ifile;
    COUNT_INPUT_TIME_START
    ifile = fopen (ifilename,"rb");
    COUNT_INPUT_TIME_END
    if (ifile == NULL) {
        fprintf(stderr, "File %s not found!\n", ifilename);
        exit(-1);
    }
    fseek(ifile, 0L, SEEK_END);
    file_position_type sz = (file_position_type) ftell(ifile);
    file_position_type total_records = sz/index->settings->ts_byte_size;
    fseek(ifile, 0L, SEEK_SET);

    if (total_records < ts_num) {
        fprintf(stderr, "File %s has only %llu records!\n", ifilename, total_records);
        exit(-1);
    }
    index->sax_file=NULL;

    long int ts_loaded = 0;
    int i,node_counter=0;
    pthread_t threadid[maxquerythread];
    buffer_data_inmemory *input_data=malloc(sizeof(buffer_data_inmemory)*(maxquerythread));
    rawfile=malloc(sizeof(ts_type) * index->settings->timeseries_size*ts_num);
    index->sax_cache= malloc(sizeof(sax_type) * index->settings->paa_segments*ts_num);
    

    index->settings->raw_filename = malloc(256);
    strcpy(index->settings->raw_filename, ifilename);
    COUNT_INPUT_TIME_START
    int read_number=fread(rawfile, sizeof(ts_type), index->settings->timeseries_size*ts_num, ifile);
    COUNT_INPUT_TIME_END
    pthread_mutex_t lock_record=PTHREAD_MUTEX_INITIALIZER,lockfbl=PTHREAD_MUTEX_INITIALIZER,lock_index=PTHREAD_MUTEX_INITIALIZER,
                    lock_firstnode=PTHREAD_MUTEX_INITIALIZER,lock_disk=PTHREAD_MUTEX_INITIALIZER;
            pthread_mutex_t *lockcbl;
    pthread_barrier_t lock_barrier1;
    pthread_barrier_init(&lock_barrier1, NULL, maxquerythread);
    // set the thread on decided cpu
    destroy_fbl(index->fbl);
        index->fbl = (first_buffer_layer*)initialize_simrec(index->settings->initial_fbl_buffer_size,
                                pow(2, index->settings->paa_segments), 
                                index->settings->max_total_buffer_size+DISK_BUFFER_SIZE*(PROGRESS_CALCULATE_THREAD_NUMBER-1), index);
    lockcbl= malloc(sizeof(pthread_mutex_t)*LOCK_SIZE);

    for(i=0; i<LOCK_SIZE ;i++)
    {
        pthread_mutex_init(&lockcbl[i], NULL);
    }

    COUNT_OUTPUT_TIME_START

    #pragma omp parallel for num_threads(maxquerythread)
    for(unsigned long  j=0; j<ts_num; j++) {
            sax_type * sax =  &(index->sax_cache[j*index->settings->paa_segments]);

        if(sax_from_ts(&(rawfile[j*index->settings->timeseries_size]), sax, index->settings->ts_values_per_paa_segment,
                       index->settings->paa_segments, index->settings->sax_alphabet_cardinality,
                       index->settings->sax_bit_cardinality) == SUCCESS)
                       {
                            //file_position_type *pos = (file_position_type)(j*index->settings->timeseries_size);
                            //memcpy(&(index->sax_cache[j*index->settings->paa_segments]),sax, sizeof(sax_type)* index->settings->paa_segments);
                            //memcpy(&(index->sax_cache[j*index->settings->paa_segments]),sax, sizeof(sax_type)* index->settings->paa_segments);

                            root_mask_type first_bit_mask = 0x00; 
                            CREATE_MASK(first_bit_mask, index,sax);
                                //printf("the number is fjlweoifwjioefwjioe %d \n",(int)first_bit_mask);
                            //fbl_soft_buffer2 *current_buffer = &((first_buffer_layer2* (index->fbl->soft_buffers)[(int) first_bit_mask];
                            fbl_soft_buffer2 *current_buffer = &((first_buffer_layer2*)(index->fbl))->soft_buffers[(int) first_bit_mask];

                            //current_buffer->initialized = 1;
                            //current_buffer->max_buffer_size;
                            __sync_fetch_and_add(&(current_buffer->max_buffer_size),1);

                       }
    }

    #pragma omp parallel for num_threads(maxquerythread)
    for(int  i=0; i<((first_buffer_layer2*)(index->fbl))->number_of_buffers; i++) 
    {
        fbl_soft_buffer2 *current_buffer = &((first_buffer_layer2*)(index->fbl))->soft_buffers[i];
        if(current_buffer->max_buffer_size!=0)
        {
        current_buffer->initialized = 1;
        
        current_buffer->pos_records = malloc(sizeof(file_position_type) * 
                                                 current_buffer->max_buffer_size);
            current_buffer->node = isax_root_node_init((root_mask_type) i, 
                                                   index->settings->initial_leaf_buffer_size);
            current_buffer->node->is_leaf = 1;
        }

    }

    
    #pragma omp parallel for num_threads(maxquerythread)
    for(unsigned long  j=0; j<ts_num; j++) {
        root_mask_type first_bit_mask = 0x00; 
        sax_type * sax =  &(index->sax_cache[j*index->settings->paa_segments]);

        CREATE_MASK(first_bit_mask, index, sax);
        fbl_soft_buffer2 *current_buffer = &((first_buffer_layer2*)(index->fbl))->soft_buffers[(int) first_bit_mask];
        int buffersize=__sync_fetch_and_add(&(current_buffer->buffer_size),1);
        current_buffer->pos_records[buffersize]=(file_position_type)(j*index->settings->timeseries_size);
    }


    for ( i = 0; i < maxquerythread; i++)
    {   
        input_data[i].index=index;
        input_data[i].lock_fbl=&lockfbl;
        input_data[i].lock_record=&lock_record;
        input_data[i].lock_cbl=lockcbl;
        input_data[i].lock_firstnode =&lock_firstnode;
        input_data[i].lock_index=&lock_index;
        input_data[i].ts=rawfile;
        input_data[i].lock_disk=&lock_disk;
        input_data[i].start_number=i*(ts_num/maxquerythread);
        input_data[i].stop_number=(i+1)*(ts_num/maxquerythread);
        input_data[i].node_counter=&node_counter;
        input_data[i].lock_barrier1=&lock_barrier1;
    }

    input_data[maxquerythread-1].start_number=(maxquerythread-1)*(ts_num/maxquerythread);
    input_data[maxquerythread-1].stop_number=ts_num;
    for (i = 0; i < maxquerythread; i++)
    {
          pthread_create(&(threadid[i]),NULL,index_creation_worker2_inmemory,(void*)&(input_data[i]));
    }
    //wait for the finish of other threads
    for (i = 0; i < maxquerythread; i++)
    {
        pthread_join(threadid[i],NULL);
    }
    fclose(ifile);
    fprintf(stderr, ">>> Finished indexing\n");
    free(lockcbl);
    free(input_data);
    pthread_barrier_destroy(&lock_barrier1);
        //printf(" the sax point is %d\n",index->first_node->isax_cardinalities[0]);
    COUNT_OUTPUT_TIME_END
}

void index_creation_gpu(const char *ifilename, long int ts_num, isax_index *index)
{
    fprintf(stderr, ">>> Indexing: %s\n", ifilename);
    FILE * ifile;
    COUNT_INPUT_TIME_START
    ifile = fopen (ifilename,"rb");
    COUNT_INPUT_TIME_END
    if (ifile == NULL) {
        fprintf(stderr, "File %s not found!\n", ifilename);
        exit(-1);
    }
    fseek(ifile, 0L, SEEK_END);
    file_position_type sz = (file_position_type) ftell(ifile);
    file_position_type total_records = sz/index->settings->ts_byte_size;
    fseek(ifile, 0L, SEEK_SET);

    if (total_records < ts_num) {
        fprintf(stderr, "File %s has only %llu records!\n", ifilename, total_records);
        exit(-1);
    }
    index->sax_file=NULL;

    long int ts_loaded = 0;
    int i,node_counter=0;
    pthread_t threadid[maxquerythread];
    buffer_data_inmemory *input_data=malloc(sizeof(buffer_data_inmemory)*(maxquerythread));
    rawfile=malloc(sizeof(ts_type) * index->settings->timeseries_size*ts_num);
    index->sax_cache= malloc(sizeof(sax_type) * index->settings->paa_segments*ts_num);
    

    index->settings->raw_filename = malloc(256);
    strcpy(index->settings->raw_filename, ifilename);
    COUNT_INPUT_TIME_START
    int read_number=fread(rawfile, sizeof(ts_type), index->settings->timeseries_size*ts_num, ifile);
    COUNT_INPUT_TIME_END
    pthread_mutex_t lock_record=PTHREAD_MUTEX_INITIALIZER,lockfbl=PTHREAD_MUTEX_INITIALIZER,lock_index=PTHREAD_MUTEX_INITIALIZER,
                    lock_firstnode=PTHREAD_MUTEX_INITIALIZER,lock_disk=PTHREAD_MUTEX_INITIALIZER;
            pthread_mutex_t *lockcbl;
    pthread_barrier_t lock_barrier1;
    pthread_barrier_init(&lock_barrier1, NULL, maxquerythread);
    // set the thread on decided cpu
    destroy_fbl(index->fbl);
        index->fbl = (first_buffer_layer*)initialize_simrec(index->settings->initial_fbl_buffer_size,
                                pow(2, index->settings->paa_segments), 
                                index->settings->max_total_buffer_size+DISK_BUFFER_SIZE*(PROGRESS_CALCULATE_THREAD_NUMBER-1), index);
    lockcbl= malloc(sizeof(pthread_mutex_t)*LOCK_SIZE);

    for(i=0; i<LOCK_SIZE ;i++)
    {
        pthread_mutex_init(&lockcbl[i], NULL);
    }

    COUNT_OUTPUT_TIME_START

    #pragma omp parallel for num_threads(maxquerythread)
    for(unsigned long  j=0; j<ts_num; j++) {
            sax_type * sax =  &(index->sax_cache[j*index->settings->paa_segments]);

        if(sax_from_ts_new(&(rawfile[j*index->settings->timeseries_size]), sax, index->settings->ts_values_per_paa_segment,
                       index->settings->paa_segments, index->settings->sax_alphabet_cardinality,
                       index->settings->sax_bit_cardinality) == SUCCESS)
                       {
                            //file_position_type *pos = (file_position_type)(j*index->settings->timeseries_size);
                            //memcpy(&(index->sax_cache[j*index->settings->paa_segments]),sax, sizeof(sax_type)* index->settings->paa_segments);
                            //memcpy(&(index->sax_cache[j*index->settings->paa_segments]),sax, sizeof(sax_type)* index->settings->paa_segments);

                            root_mask_type first_bit_mask = 0x00; 
                            CREATE_MASK(first_bit_mask, index,sax);
                                //printf("the number is fjlweoifwjioefwjioe %d \n",(int)first_bit_mask);
                            //fbl_soft_buffer2 *current_buffer = &((first_buffer_layer2* (index->fbl->soft_buffers)[(int) first_bit_mask];
                            fbl_soft_buffer2 *current_buffer = &((first_buffer_layer2*)(index->fbl))->soft_buffers[(int) first_bit_mask];

                            //current_buffer->initialized = 1;
                            //current_buffer->max_buffer_size;
                            __sync_fetch_and_add(&(current_buffer->max_buffer_size),1);

                       }
    }

    #pragma omp parallel for num_threads(maxquerythread)
    for(int  i=0; i<((first_buffer_layer2*)(index->fbl))->number_of_buffers; i++) 
    {
        fbl_soft_buffer2 *current_buffer = &((first_buffer_layer2*)(index->fbl))->soft_buffers[i];
        if(current_buffer->max_buffer_size!=0)
        {
        current_buffer->initialized = 1;
        current_buffer->sax_records = malloc(sizeof(sax_type ) * index->settings->paa_segments*
                                                 current_buffer->max_buffer_size);
        current_buffer->pos_records = malloc(sizeof(file_position_type) * 
                                                 current_buffer->max_buffer_size);
            current_buffer->node = isax_root_node_init((root_mask_type) i, 
                                                   index->settings->initial_leaf_buffer_size);
            current_buffer->node->is_leaf = 1;
        }
    }

    
    #pragma omp parallel for num_threads(maxquerythread)
    for(unsigned long  j=0; j<ts_num; j++) {
        root_mask_type first_bit_mask = 0x00; 
        sax_type * sax =  &(index->sax_cache[j*index->settings->paa_segments]);

        CREATE_MASK(first_bit_mask, index, sax);
        fbl_soft_buffer2 *current_buffer = &((first_buffer_layer2*)(index->fbl))->soft_buffers[(int) first_bit_mask];
        int buffersize=__sync_fetch_and_add(&(current_buffer->buffer_size),1);
        memcpy(&current_buffer->sax_records[buffersize*index->settings->paa_segments],sax,sizeof(sax_type)* index->settings->paa_segments);
        current_buffer->pos_records[buffersize]=(file_position_type)(j*index->settings->timeseries_size);
    }
    for ( i = 0; i < maxquerythread; i++)
    {   
        input_data[i].index=index;
        input_data[i].lock_fbl=&lockfbl;
        input_data[i].lock_record=&lock_record;
        input_data[i].lock_cbl=lockcbl;
        input_data[i].lock_firstnode =&lock_firstnode;
        input_data[i].lock_index=&lock_index;
        input_data[i].ts=rawfile;
        input_data[i].lock_disk=&lock_disk;
        input_data[i].start_number=i*(ts_num/maxquerythread);
        input_data[i].stop_number=(i+1)*(ts_num/maxquerythread);
        input_data[i].node_counter=&node_counter;
        input_data[i].lock_barrier1=&lock_barrier1;
    }

    input_data[maxquerythread-1].start_number=(maxquerythread-1)*(ts_num/maxquerythread);
    input_data[maxquerythread-1].stop_number=ts_num;
    for (i = 0; i < maxquerythread; i++)
    {
          pthread_create(&(threadid[i]),NULL,index_creation_worker2_inmemory,(void*)&(input_data[i]));
    }
    //wait for the finish of other threads
    for (i = 0; i < maxquerythread; i++)
    {
        pthread_join(threadid[i],NULL);
    }
        index->sax_cache_size=ts_num;
    fclose(ifile);
    fprintf(stderr, ">>> Finished indexing\n");
    free(lockcbl);
    free(input_data);
    pthread_barrier_destroy(&lock_barrier1);
        //printf(" the sax point is %d\n",index->first_node->isax_cardinalities[0]);
    COUNT_OUTPUT_TIME_END
}
void index_creation_mix(const char *ifilename, long int ts_num, isax_index *index)
{
    fprintf(stderr, ">>> Indexing: %s\n", ifilename);
    FILE * ifile;
    COUNT_INPUT_TIME_START
    ifile = fopen (ifilename,"rb");
    COUNT_INPUT_TIME_END
    if (ifile == NULL) {
        fprintf(stderr, "File %s not found!\n", ifilename);
        exit(-1);
    }
    fseek(ifile, 0L, SEEK_END);
    file_position_type sz = (file_position_type) ftell(ifile);
    file_position_type total_records = sz/index->settings->ts_byte_size;
    fseek(ifile, 0L, SEEK_SET);

    if (total_records < ts_num) {
        fprintf(stderr, "File %s has only %llu records!\n", ifilename, total_records);
        exit(-1);
    }
    index->sax_file=NULL;

    long int ts_loaded = 0;
    int i,node_counter=0;
    pthread_t threadid[maxquerythread];
    buffer_data_inmemory *input_data=malloc(sizeof(buffer_data_inmemory)*(maxquerythread));
    rawfile=malloc(sizeof(ts_type) * index->settings->timeseries_size*ts_num);
    index->sax_cache= malloc(sizeof(sax_type) * index->settings->paa_segments*ts_num);
    

    index->settings->raw_filename = malloc(256);
    strcpy(index->settings->raw_filename, ifilename);
    COUNT_INPUT_TIME_START
    int read_number=fread(rawfile, sizeof(ts_type), index->settings->timeseries_size*ts_num, ifile);
    COUNT_INPUT_TIME_END
    pthread_mutex_t lock_record=PTHREAD_MUTEX_INITIALIZER,lockfbl=PTHREAD_MUTEX_INITIALIZER,lock_index=PTHREAD_MUTEX_INITIALIZER,
                    lock_firstnode=PTHREAD_MUTEX_INITIALIZER,lock_disk=PTHREAD_MUTEX_INITIALIZER;
            pthread_mutex_t *lockcbl;
    pthread_barrier_t lock_barrier1;
    pthread_barrier_init(&lock_barrier1, NULL, maxquerythread);
    // set the thread on decided cpu

    lockcbl= malloc(sizeof(pthread_mutex_t)*LOCK_SIZE);

    for(i=0; i<LOCK_SIZE ;i++)
    {
        pthread_mutex_init(&lockcbl[i], NULL);
    }

    COUNT_OUTPUT_TIME_START

    
    for ( i = 0; i < maxquerythread; i++)
    {   
        input_data[i].index=index;
        input_data[i].lock_fbl=&lockfbl;
        input_data[i].lock_record=&lock_record;
        input_data[i].lock_cbl=lockcbl;
        input_data[i].lock_firstnode =&lock_firstnode;
        input_data[i].lock_index=&lock_index;
        input_data[i].ts=rawfile;
        input_data[i].lock_disk=&lock_disk;
        input_data[i].start_number=i*(ts_num/maxquerythread);
        input_data[i].stop_number=(i+1)*(ts_num/maxquerythread);
        input_data[i].node_counter=&node_counter;
        input_data[i].lock_barrier1=&lock_barrier1;
    }

    input_data[maxquerythread-1].start_number=(maxquerythread-1)*(ts_num/maxquerythread);
    input_data[maxquerythread-1].stop_number=ts_num;
    for (i = 0; i < maxquerythread; i++)
    {
        pthread_create(&(threadid[i]),NULL,index_creation_mix_worker_inmemory,(void*)&(input_data[i]));
    }
    //wait for the finish of other threads
    for (i = 0; i < maxquerythread; i++)
    {
        pthread_join(threadid[i],NULL);
    }
    fclose(ifile);
    fprintf(stderr, ">>> Finished indexing\n");
    free(lockcbl);
    free(input_data);
    pthread_barrier_destroy(&lock_barrier1);
        //printf(" the sax point is %d\n",index->first_node->isax_cardinalities[0]);
    COUNT_OUTPUT_TIME_END
}

void index_generate_inmemory_pRecBuf(const char *ifilename, long int ts_num, isax_index *index)
{
    fprintf(stderr, ">>> Indexing: %s\n", ifilename);
    FILE * ifile;
    COUNT_INPUT_TIME_START
    ifile = fopen (ifilename,"rb");
    COUNT_INPUT_TIME_END
    if (ifile == NULL) {
        fprintf(stderr, "File %s not found!\n", ifilename);
        exit(-1);
    }
    fseek(ifile, 0L, SEEK_END);
    file_position_type sz = (file_position_type) ftell(ifile);
    file_position_type total_records = sz/index->settings->ts_byte_size;
    fseek(ifile, 0L, SEEK_SET);

    if (total_records < ts_num) {
        fprintf(stderr, "File %s has only %llu records!\n", ifilename, total_records);
        exit(-1);
    }
    index->sax_file=NULL;

    long int ts_loaded = 0;
    int i;
    pthread_t threadid[maxquerythread];
    buffer_data_inmemory *input_data=malloc(sizeof(buffer_data_inmemory)*(maxquerythread));
    rawfile=malloc(sizeof(ts_type) * index->settings->timeseries_size*ts_num);
    index->sax_cache= malloc(sizeof(sax_type) * index->settings->paa_segments*ts_num);
    

    index->settings->raw_filename = malloc(256);
    strcpy(index->settings->raw_filename, ifilename);
    COUNT_INPUT_TIME_START
    int read_number=fread(rawfile, sizeof(ts_type), index->settings->timeseries_size*ts_num, ifile);
    COUNT_INPUT_TIME_END
    pthread_mutex_t lock_record=PTHREAD_MUTEX_INITIALIZER,lockfbl=PTHREAD_MUTEX_INITIALIZER,lock_index=PTHREAD_MUTEX_INITIALIZER,
                    lock_firstnode=PTHREAD_MUTEX_INITIALIZER,lock_disk=PTHREAD_MUTEX_INITIALIZER;
    
    destroy_fbl(index->fbl);
        index->fbl = (first_buffer_layer*)initialize_pRecBuf(index->settings->initial_fbl_buffer_size,
                                pow(2, index->settings->paa_segments), 
                                index->settings->max_total_buffer_size+DISK_BUFFER_SIZE*(PROGRESS_CALCULATE_THREAD_NUMBER-1), index);
    // set the thread on decided cpu



    COUNT_OUTPUT_TIME_START

    
    for ( i = 0; i < maxquerythread; i++)
    {   
        input_data[i].index=index;
        input_data[i].lock_fbl=&lockfbl;
        input_data[i].lock_record=&lock_record;
        input_data[i].lock_firstnode =&lock_firstnode;
        input_data[i].lock_index=&lock_index;
        input_data[i].ts=rawfile;
        input_data[i].lock_disk=&lock_disk;
        input_data[i].workernumber=i;
        input_data[i].total_workernumber=maxquerythread;
        input_data[i].start_number=i*(ts_num/maxquerythread);
        input_data[i].stop_number=(i+1)*(ts_num/maxquerythread);
    }

    input_data[maxquerythread-1].start_number=(maxquerythread-1)*(ts_num/maxquerythread);
    input_data[maxquerythread-1].stop_number=ts_num;
    for (i = 0; i < maxquerythread; i++)
    {
        pthread_create(&(threadid[i]),NULL,indexbulkloadingworker_pRecBuf_inmemory,(void*)&(input_data[i]));
    }
    //wait for the finish of other threads
    for (i = 0; i < maxquerythread; i++)
    {
        pthread_join(threadid[i],NULL);
    }
    __sync_fetch_and_add(&(index->total_records),ts_num);

    fclose(ifile);
    fprintf(stderr, ">>> Finished indexing\n");
    free(input_data);
        //printf(" the sax point is %d\n",index->first_node->isax_cardinalities[0]);
    COUNT_OUTPUT_TIME_END
}

void index_creation_pRecBuf(const char *ifilename, long int ts_num, isax_index *index)
{
    fprintf(stderr, ">>> Indexing: %s\n", ifilename);
    FILE * ifile;
    COUNT_INPUT_TIME_START
    ifile = fopen (ifilename,"rb");
    COUNT_INPUT_TIME_END
    if (ifile == NULL) {
        fprintf(stderr, "File %s not found!\n", ifilename);
        exit(-1);
    }
    fseek(ifile, 0L, SEEK_END);
    file_position_type sz = (file_position_type) ftell(ifile);
    file_position_type total_records = sz/index->settings->ts_byte_size;
    fseek(ifile, 0L, SEEK_SET);

    if (total_records < ts_num) {
        fprintf(stderr, "File %s has only %llu records!\n", ifilename, total_records);
        exit(-1);
    }
    index->sax_file=NULL;

    long int ts_loaded = 0;
    int i;
    int node_counter=0;
    pthread_t threadid[maxquerythread];
    buffer_data_inmemory *input_data=malloc(sizeof(buffer_data_inmemory)*(maxquerythread));
    rawfile=malloc(sizeof(ts_type) * index->settings->timeseries_size*ts_num);
    index->sax_cache= malloc(sizeof(sax_type) * index->settings->paa_segments*ts_num);
    pthread_barrier_t lock_barrier1;
    pthread_barrier_init(&lock_barrier1, NULL, maxquerythread);

    index->settings->raw_filename = malloc(256);
    strcpy(index->settings->raw_filename, ifilename);
    COUNT_INPUT_TIME_START
    int read_number=fread(rawfile, sizeof(ts_type), index->settings->timeseries_size*ts_num, ifile);
    COUNT_INPUT_TIME_END
    pthread_mutex_t lock_record=PTHREAD_MUTEX_INITIALIZER,lockfbl=PTHREAD_MUTEX_INITIALIZER,lock_index=PTHREAD_MUTEX_INITIALIZER,
                    lock_firstnode=PTHREAD_MUTEX_INITIALIZER,lock_disk=PTHREAD_MUTEX_INITIALIZER;
    
    destroy_fbl(index->fbl);
        index->fbl = (first_buffer_layer*)initialize_pRecBuf(index->settings->initial_fbl_buffer_size,
                                pow(2, index->settings->paa_segments), 
                                index->settings->max_total_buffer_size+DISK_BUFFER_SIZE*(PROGRESS_CALCULATE_THREAD_NUMBER-1), index);
    // set the thread on decided cpu



    COUNT_OUTPUT_TIME_START

    
    for ( i = 0; i < maxquerythread; i++)
    {   
        input_data[i].index=index;
        input_data[i].lock_fbl=&lockfbl;
        input_data[i].lock_record=&lock_record;
        input_data[i].lock_firstnode =&lock_firstnode;
        input_data[i].lock_index=&lock_index;
        input_data[i].ts=rawfile;
        input_data[i].lock_disk=&lock_disk;
        input_data[i].workernumber=i;
        input_data[i].total_workernumber=maxquerythread;
        input_data[i].start_number=i*(ts_num/maxquerythread);
        input_data[i].stop_number=(i+1)*(ts_num/maxquerythread);
        input_data[i].node_counter=&node_counter;
        input_data[i].lock_barrier1=&lock_barrier1;
    }

    input_data[maxquerythread-1].start_number=(maxquerythread-1)*(ts_num/maxquerythread);
    input_data[maxquerythread-1].stop_number=ts_num;
    for (i = 0; i < maxquerythread; i++)
    {
        pthread_create(&(threadid[i]),NULL,index_creation_pRecBuf_worker,(void*)&(input_data[i]));
    }
    //wait for the finish of other threads
    for (i = 0; i < maxquerythread; i++)
    {
        pthread_join(threadid[i],NULL);
    }
    __sync_fetch_and_add(&(index->total_records),ts_num);
    index->sax_cache_size=index->total_records;
    fclose(ifile);
    fprintf(stderr, ">>> Finished indexing\n");
    free(input_data);
    COUNT_OUTPUT_TIME_END
}

void index_creation_pRecBuf_new(const char *ifilename, long int ts_num, isax_index *index)
{
    fprintf(stderr, ">>> Indexing: %s\n", ifilename);
    FILE * ifile;
    COUNT_INPUT_TIME_START
    ifile = fopen (ifilename,"rb");
    COUNT_INPUT_TIME_END
    if (ifile == NULL) {
        fprintf(stderr, "File %s not found!\n", ifilename);
        exit(-1);
    }
    fseek(ifile, 0L, SEEK_END);
    file_position_type sz = (file_position_type) ftell(ifile);
    file_position_type total_records = sz/index->settings->ts_byte_size;
    fseek(ifile, 0L, SEEK_SET);

    if (total_records < ts_num) {
        fprintf(stderr, "File %s has only %llu records!\n", ifilename, total_records);
        exit(-1);
    }
    index->sax_file=NULL;

    long int ts_loaded = 0;
    unsigned long shared_start_number=0;
    int i;
    int node_counter=0;
    pthread_t threadid[maxquerythread];
    buffer_data_inmemory *input_data=malloc(sizeof(buffer_data_inmemory)*(maxquerythread));
    rawfile=malloc(sizeof(ts_type) * index->settings->timeseries_size*ts_num);
    index->sax_cache= malloc(sizeof(sax_type) * index->settings->paa_segments*ts_num);
    pthread_barrier_t lock_barrier1,lock_barrier2;
    pthread_barrier_init(&lock_barrier1, NULL, maxquerythread+1);
    pthread_barrier_init(&lock_barrier2, NULL, maxquerythread+1);
    index->settings->raw_filename = malloc(256);
    strcpy(index->settings->raw_filename, ifilename);
    COUNT_INPUT_TIME_START
    int read_number=fread(rawfile, sizeof(ts_type), index->settings->timeseries_size*ts_num, ifile);
    COUNT_INPUT_TIME_END
    pthread_mutex_t lock_record=PTHREAD_MUTEX_INITIALIZER,lockfbl=PTHREAD_MUTEX_INITIALIZER,lock_index=PTHREAD_MUTEX_INITIALIZER,
                    lock_firstnode=PTHREAD_MUTEX_INITIALIZER,lock_disk=PTHREAD_MUTEX_INITIALIZER;
    
    destroy_fbl(index->fbl);
        index->fbl = (first_buffer_layer*)initialize_pRecBuf(index->settings->initial_fbl_buffer_size,
                                pow(2, index->settings->paa_segments), 
                                index->settings->max_total_buffer_size+DISK_BUFFER_SIZE*(PROGRESS_CALCULATE_THREAD_NUMBER-1), index);
    // set the thread on decided cpu



    COUNT_OUTPUT_TIME_START
    int nodeid[index->fbl->number_of_buffers];
    int nodesize[index->fbl->number_of_buffers];
    
    for ( i = 0; i < maxquerythread; i++)
    {   
        input_data[i].index=index;
        input_data[i].lock_fbl=&lockfbl;
        input_data[i].lock_record=&lock_record;
        input_data[i].lock_firstnode =&lock_firstnode;
        input_data[i].lock_index=&lock_index;
        input_data[i].ts=rawfile;
        input_data[i].lock_disk=&lock_disk;
        input_data[i].workernumber=i;
        input_data[i].total_workernumber=maxquerythread;
        input_data[i].start_number=i*(ts_num/maxquerythread);
        input_data[i].shared_start_number=&shared_start_number;
        input_data[i].stop_number=ts_num;
        input_data[i].node_counter=&node_counter;
        input_data[i].lock_barrier1=&lock_barrier1;
        input_data[i].lock_barrier2=&lock_barrier2;
        input_data[i].nodeid=nodeid;
    }
    for (i = 0; i < maxquerythread; i++)
    {
        pthread_create(&(threadid[i]),NULL,index_creation_pRecBuf_worker_new,(void*)&(input_data[i]));
    }

    pthread_barrier_wait(&lock_barrier1);
   /*  #pragma omp parallel for num_threads(maxquerythread)
    for (int i = 0; i < index->fbl->number_of_buffers; i++)
    {
        if(index->fbl->soft_buffers[i].initialized)
        {
            nodeid[i]=i;
            nodesize[i]=index->fbl->soft_buffers[i].buffer_size;
        }
        else
        {
            nodeid[i]=i;
            nodesize[i]=0;
        }
    }

           for (int i = 0; i < index->fbl->number_of_buffers; i++)
            {
                int maxval=nodesize[i];
                int maxindex=i;
                #pragma omp parallel for reduction(max : maxval)
                for (int j = i+1; j < index->fbl->number_of_buffers; j++)
                {
                    if(nodesize[j] > maxval)
                    {
                        maxval = nodesize[j];
                        maxindex = j;
                    }
                }
                int tmp = nodesize[i];
                nodesize[i] = maxval;
                nodesize[maxindex] = tmp;
                tmp = nodeid[i];
                nodeid[i] = maxval;
                nodeid[maxindex] = tmp;
            }*/
    pthread_barrier_wait(&lock_barrier2);


    //wait for the finish of other threads
    for (i = 0; i < maxquerythread; i++)
    {
        pthread_join(threadid[i],NULL);
    }
    __sync_fetch_and_add(&(index->total_records),ts_num);
    index->sax_cache_size=index->total_records;
    fclose(ifile);
    fprintf(stderr, ">>> Finished indexing\n");
    free(input_data);
        //printf(" the sax point is %d\n",index->first_node->isax_cardinalities[0]);
    COUNT_OUTPUT_TIME_END
}


void* indexbulkloadingworker_inmemory(void *transferdata)
{
    sax_type * sax = malloc(sizeof(sax_type) * ((buffer_data_inmemory*)transferdata)->index->settings->paa_segments);
            //struct timeval workertimestart;
    //struct timeval writetiemstart;
    //struct timeval workercurenttime;
    //struct timeval writecurenttime;
    //double worker_total_time,tee,tss;
    //gettimeofday(&workertimestart, NULL);       


    unsigned long start_number=((buffer_data_inmemory*)transferdata)->start_number;
    unsigned long stop_number=((buffer_data_inmemory*)transferdata)->stop_number;
    file_position_type *pos = malloc(sizeof(file_position_type));
    isax_index *index= ((buffer_data_inmemory*)transferdata)->index;
    ts_type * ts = malloc(sizeof(ts_type) * index->settings->timeseries_size);
    int paa_segments=((buffer_data_inmemory*)transferdata)->index->settings->paa_segments;

    unsigned long i=0;
    float *raw_file=((buffer_data_inmemory*)transferdata)->ts;
    for (i=start_number;i<stop_number;i++)
    {
    #ifndef DEBUG
    #if VERBOSE_LEVEL == 2
        printf("\r\x1b[32mLoading: \x1b[36m%d\x1b[0m",(i + 1));
    #endif
    #endif

        memcpy(ts,&(raw_file[i*index->settings->timeseries_size]), sizeof(float)* index->settings->timeseries_size);

        if(sax_from_ts(ts, sax, index->settings->ts_values_per_paa_segment,
                       index->settings->paa_segments, index->settings->sax_alphabet_cardinality,
                       index->settings->sax_bit_cardinality) == SUCCESS)
        {
            *pos = (file_position_type)(i*index->settings->timeseries_size);

            memcpy(&(index->sax_cache[i*index->settings->paa_segments]),sax, sizeof(sax_type)* index->settings->paa_segments);

            isax_fbl_index_insert_inmemory_para(index, sax, pos, ((buffer_data_inmemory*)transferdata)->lock_fbl,
                    ((buffer_data_inmemory*)transferdata)->lock_cbl,((buffer_data_inmemory*)transferdata)->lock_firstnode,((buffer_data_inmemory*)transferdata)->lock_index);
        }
        else
        {
            fprintf(stderr, "error: cannot insert record in index, since sax representation\
                    failed to be created");
        }
    }
    free(pos);
    free(sax);
    free(ts);
        //gettimeofday(&workercurenttime, NULL);
    //tss = workertimestart.tv_sec*1000000 + (workertimestart.tv_usec); 
    //tee = workercurenttime.tv_sec*1000000  + (workercurenttime.tv_usec); 
    //worker_total_time += (tee - tss); 
    //printf("the worker time is %f\n",worker_total_time );
}
void* index_creation_worker_inmemory(void *transferdata)
{
    sax_type * sax = malloc(sizeof(sax_type) * ((buffer_data_inmemory*)transferdata)->index->settings->paa_segments);
            //struct timeval workertimestart;
    //struct timeval writetiemstart;
    //struct timeval workercurenttime;
    //struct timeval writecurenttime;
    //double worker_total_time,tee,tss;
    //gettimeofday(&workertimestart, NULL);       


    unsigned long start_number=((buffer_data_inmemory*)transferdata)->start_number;
    unsigned long stop_number=((buffer_data_inmemory*)transferdata)->stop_number;
    file_position_type *pos = malloc(sizeof(file_position_type));
    isax_index *index= ((buffer_data_inmemory*)transferdata)->index;
    ts_type * ts = malloc(sizeof(ts_type) * index->settings->timeseries_size);
    int paa_segments=((buffer_data_inmemory*)transferdata)->index->settings->paa_segments;

    unsigned long i=0;
    float *raw_file=((buffer_data_inmemory*)transferdata)->ts;
    for (i=start_number;i<stop_number;i++)
    {
    #ifndef DEBUG
    #if VERBOSE_LEVEL == 2
        printf("\r\x1b[32mLoading: \x1b[36m%d\x1b[0m",(i + 1));
    #endif
    #endif

        memcpy(ts,&(raw_file[i*index->settings->timeseries_size]), sizeof(float)* index->settings->timeseries_size);

        if(sax_from_ts(ts, sax, index->settings->ts_values_per_paa_segment,
                       index->settings->paa_segments, index->settings->sax_alphabet_cardinality,
                       index->settings->sax_bit_cardinality) == SUCCESS)
        {
            *pos = (file_position_type)(i*index->settings->timeseries_size);

            memcpy(&(index->sax_cache[i*index->settings->paa_segments]),sax, sizeof(sax_type)* index->settings->paa_segments);

            isax_fbl_index_insert_inmemory_para(index, sax, pos, ((buffer_data_inmemory*)transferdata)->lock_fbl,
                    ((buffer_data_inmemory*)transferdata)->lock_cbl,((buffer_data_inmemory*)transferdata)->lock_firstnode,((buffer_data_inmemory*)transferdata)->lock_index);
        }
        else
        {
            fprintf(stderr, "error: cannot insert record in index, since sax representation\
                    failed to be created");
        }
    }
    free(pos);
    free(sax);
    free(ts);

    pthread_barrier_wait(((buffer_data_inmemory*)transferdata)->lock_barrier1);

    int j,c=1,k;
    isax_node_record *r = malloc(sizeof(isax_node_record));
    //for (j=((trans_fbl_input*)input)->start_number; j<((trans_fbl_input*)input)->stop_number; j++) 
    while(1)
    {

        j=__sync_fetch_and_add(((buffer_data_inmemory*)transferdata)->node_counter,1);
        if(j>=index->fbl->number_of_buffers)
        {
            break;
        }
        fbl_soft_buffer *current_fbl_node = &index->fbl->soft_buffers[j];
        if (!current_fbl_node->initialized) {
            continue;
        }

        if (current_fbl_node->buffer_size > 0) {

            for (k=0; k<current_fbl_node->buffer_size; k++) {
                r->sax = (sax_type *) current_fbl_node->sax_records[k];
                r->position = (file_position_type *) current_fbl_node->pos_records[k];
                r->insertion_mode = NO_TMP | PARTIAL;
                // Add record to index
                //printf("the position 1 is %d\n",*(r->position));
                //sleep(1);
                add_record_to_node(index, current_fbl_node->node, r, 1);
            }
            flush_subtree_leaf_buffers_inmemory(index, current_fbl_node->node);

            // clear FBL records moved in LBL buffers
            free(current_fbl_node->sax_records);
            free(current_fbl_node->pos_records);
            // clear records read from files (free only prev sax buffers)
        }
        
    }
    free(r);

}
void* index_creation_worker_inmemory_new(void *transferdata)
{
    sax_type * sax = malloc(sizeof(sax_type) * ((buffer_data_inmemory*)transferdata)->index->settings->paa_segments);
            //struct timeval workertimestart;
    //struct timeval writetiemstart;
    //struct timeval workercurenttime;
    //struct timeval writecurenttime;
    //double worker_total_time,tee,tss;
    //gettimeofday(&workertimestart, NULL);       
    unsigned long roundfinishednumber;
    unsigned long start_number;

    unsigned long stop_number=((buffer_data_inmemory*)transferdata)->stop_number;
    file_position_type *pos = malloc(sizeof(file_position_type));
    isax_index *index= ((buffer_data_inmemory*)transferdata)->index;
    ts_type * ts = malloc(sizeof(ts_type) * index->settings->timeseries_size);
    int paa_segments=((buffer_data_inmemory*)transferdata)->index->settings->paa_segments;

    unsigned long i=0;
    float *raw_file=((buffer_data_inmemory*)transferdata)->ts;
    while(1)
    {   
        start_number=__sync_fetch_and_add(((buffer_data_inmemory*)transferdata)->shared_start_number,read_block_length);
        if(start_number>stop_number)
        {
            break;
        }
        else if(start_number>stop_number-read_block_length)
        {
            roundfinishednumber=stop_number;
        }
        else
        {
            roundfinishednumber=start_number+read_block_length;
        }
         for (i=start_number;i<roundfinishednumber;i++)
         {
    #ifndef DEBUG
    #if VERBOSE_LEVEL == 2
        printf("\r\x1b[32mLoading: \x1b[36m%d\x1b[0m",(i + 1));
    #endif
    #endif

        memcpy(ts,&(raw_file[i*index->settings->timeseries_size]), sizeof(float)* index->settings->timeseries_size);

        if(sax_from_ts(ts, sax, index->settings->ts_values_per_paa_segment,
                       index->settings->paa_segments, index->settings->sax_alphabet_cardinality,
                       index->settings->sax_bit_cardinality) == SUCCESS)
        {
            *pos = (file_position_type)(i*index->settings->timeseries_size);

            memcpy(&(index->sax_cache[i*index->settings->paa_segments]),sax, sizeof(sax_type)* index->settings->paa_segments);

            isax_fbl_index_insert_inmemory_para(index, sax, pos, ((buffer_data_inmemory*)transferdata)->lock_fbl,
                    ((buffer_data_inmemory*)transferdata)->lock_cbl,((buffer_data_inmemory*)transferdata)->lock_firstnode,((buffer_data_inmemory*)transferdata)->lock_index);
        }
        else
        {
            fprintf(stderr, "error: cannot insert record in index, since sax representation\
                    failed to be created");
        }
    }
    }
    free(pos);
    free(sax);
    free(ts);

    pthread_barrier_wait(((buffer_data_inmemory*)transferdata)->lock_barrier1);

    int j,c=1,k;
    isax_node_record *r = malloc(sizeof(isax_node_record));
    //for (j=((trans_fbl_input*)input)->start_number; j<((trans_fbl_input*)input)->stop_number; j++) 
    while(1)
    {

        j=__sync_fetch_and_add(((buffer_data_inmemory*)transferdata)->node_counter,1);
        if(j>=index->fbl->number_of_buffers)
        {
            break;
        }
        fbl_soft_buffer *current_fbl_node = &index->fbl->soft_buffers[j];
        if (!current_fbl_node->initialized) {
            continue;
        }

        if (current_fbl_node->buffer_size > 0) {

            for (k=0; k<current_fbl_node->buffer_size; k++) {
                r->sax = (sax_type *) current_fbl_node->sax_records[k];
                r->position = (file_position_type *) current_fbl_node->pos_records[k];
                r->insertion_mode = NO_TMP | PARTIAL;
                // Add record to index
                //printf("the position 1 is %d\n",*(r->position));
                //sleep(1);
                add_record_to_node(index, current_fbl_node->node, r, 1);
            }
            flush_subtree_leaf_buffers_inmemory(index, current_fbl_node->node);

            // clear FBL records moved in LBL buffers
            free(current_fbl_node->sax_records);
            free(current_fbl_node->pos_records);
            // clear records read from files (free only prev sax buffers)
        }
        
    }
    free(r);

}

void* index_creation_worker2_inmemory(void *transferdata)
{
    sax_type * sax = malloc(sizeof(sax_type) * ((buffer_data_inmemory*)transferdata)->index->settings->paa_segments);
            //struct timeval workertimestart;
    //struct timeval writetiemstart;
    //struct timeval workercurenttime;
    //struct timeval writecurenttime;
    //double worker_total_time,tee,tss;
    //gettimeofday(&workertimestart, NULL);       


    unsigned long start_number=((buffer_data_inmemory*)transferdata)->start_number;
    unsigned long stop_number=((buffer_data_inmemory*)transferdata)->stop_number;
    file_position_type *pos = malloc(sizeof(file_position_type));
    isax_index *index= ((buffer_data_inmemory*)transferdata)->index;
    ts_type * ts = malloc(sizeof(ts_type) * index->settings->timeseries_size);
    int paa_segments=((buffer_data_inmemory*)transferdata)->index->settings->paa_segments;

    unsigned long i=0;
    float *raw_file=((buffer_data_inmemory*)transferdata)->ts;

    free(pos);
    free(sax);
    free(ts);


    int j,c=1,k;
    isax_node_record *r = malloc(sizeof(isax_node_record));
    //for (j=((trans_fbl_input*)input)->start_number; j<((trans_fbl_input*)input)->stop_number; j++) 
    while(1)
    {

        j=__sync_fetch_and_add(((buffer_data_inmemory*)transferdata)->node_counter,1);
        if(j>=index->fbl->number_of_buffers)
        {
            break;
        }
        fbl_soft_buffer2 *current_fbl_node = &((first_buffer_layer2*)(index->fbl))->soft_buffers[j];
        if (!current_fbl_node->initialized) {
            continue;
        }

        if (current_fbl_node->buffer_size > 0) {//index->settings->timeseries_size

            for (k=0; k<current_fbl_node->buffer_size; k++) {
                //memcpy(sax,&(index->sax_cache[(((file_position_type *) current_fbl_node->pos_records)[k])/index->settings->timeseries_size]),sizeof(sax_type) * ((buffer_data_inmemory*)transferdata)->index->settings->paa_segments);
                r->sax = &(index->sax_cache[current_fbl_node->pos_records[k]/index->settings->timeseries_size*index->settings->paa_segments]);
                r->position = &current_fbl_node->pos_records[k];
                r->insertion_mode = NO_TMP | PARTIAL;
                // Add record to index
                //printf("the position 1 is %d\n",*(r->position));
                //sleep(1);
                add_record_to_node(index, current_fbl_node->node, r, 1);
            }
            flush_subtree_leaf_buffers_inmemory(index, current_fbl_node->node);

            // clear FBL records moved in LBL buffers
            //free(current_fbl_node->sax_records);
            //free(current_fbl_node->pos_records);
            // clear records read from files (free only prev sax buffers)
        }
    }
    free(r);

}

void* index_creation_mix_worker_inmemory(void *transferdata)
{
    
            //struct timeval workertimestart;
    //struct timeval writetiemstart;
    //struct timeval workercurenttime;
    //struct timeval writecurenttime;
    //double worker_total_time,tee,tss;
    //gettimeofday(&workertimestart, NULL);       

    sax_type * sax = malloc(sizeof(sax_type) * ((buffer_data_inmemory*)transferdata)->index->settings->paa_segments);
    unsigned long start_number=((buffer_data_inmemory*)transferdata)->start_number;
    unsigned long stop_number=((buffer_data_inmemory*)transferdata)->stop_number;
    file_position_type *pos = malloc(sizeof(file_position_type));
    isax_index *index= ((buffer_data_inmemory*)transferdata)->index;
    ts_type * ts = malloc(sizeof(ts_type) * index->settings->timeseries_size);
    int paa_segments=((buffer_data_inmemory*)transferdata)->index->settings->paa_segments;
    root_mask_type first_bit_mask = 0x00;       
    fbl_soft_buffer *current_buffer;
    isax_node_record *r = malloc(sizeof(isax_node_record));
    unsigned long i=0;
    float *raw_file=((buffer_data_inmemory*)transferdata)->ts;
    for (i=start_number;i<stop_number;i++)
    {
    #ifndef DEBUG
    #if VERBOSE_LEVEL == 2
        printf("\r\x1b[32mLoading: \x1b[36m%d\x1b[0m",(i + 1));
    #endif
    #endif

        memcpy(ts,&(raw_file[i*index->settings->timeseries_size]), sizeof(float)* index->settings->timeseries_size);

        if(sax_from_ts(ts, sax, index->settings->ts_values_per_paa_segment,
                       index->settings->paa_segments, index->settings->sax_alphabet_cardinality,
                       index->settings->sax_bit_cardinality) == SUCCESS)
        {
            *pos = (file_position_type)(i*index->settings->timeseries_size);

            memcpy(&(index->sax_cache[i*index->settings->paa_segments]),sax, sizeof(sax_type)* index->settings->paa_segments);
            r->sax = sax;
                r->position = pos;
                r->insertion_mode = NO_TMP | PARTIAL;
                CREATE_MASK(first_bit_mask, index, sax);

         current_buffer = &index->fbl->soft_buffers[(int) first_bit_mask];

    // Check if this buffer is initialized
    pthread_mutex_lock(&(((buffer_data_inmemory*)transferdata)->lock_cbl[first_bit_mask]));
    if (!current_buffer->initialized) {
        current_buffer->initialized = 1;
        current_buffer->max_buffer_size = 0;
        current_buffer->buffer_size = 0;

        current_buffer->node = isax_root_node_init(first_bit_mask, 
                                                   index->settings->initial_leaf_buffer_size);

        current_buffer->node->is_leaf = 1;
        

        index->root_nodes++;//counter

        
        if(index->first_node == NULL)
        {
            index->first_node = current_buffer->node;
            current_buffer->node->next = NULL;
            current_buffer->node->previous = NULL;
            
        }
        else
        {
            isax_node * prev_first = index->first_node;
            index->first_node = current_buffer->node;
            index->first_node->next = prev_first;
            
            prev_first->previous = current_buffer->node;
        }
    }



        add_record_to_node(index, current_buffer->node, r, 1);
        pthread_mutex_unlock(&((buffer_data_inmemory*)transferdata)->lock_cbl[first_bit_mask]);

            //isax_fbl_index_insert_inmemory_para(index, sax, pos, ((buffer_data_inmemory*)transferdata)->lock_fbl,
            //((buffer_data_inmemory*)transferdata)->lock_cbl,((buffer_data_inmemory*)transferdata)->lock_firstnode,((buffer_data_inmemory*)transferdata)->lock_index);
        }
        else
        {
            fprintf(stderr, "error: cannot insert record in index, since sax representation\
                    failed to be created");
        }
    }
    free(pos);
    free(sax);
    free(ts);

    pthread_barrier_wait(((buffer_data_inmemory*)transferdata)->lock_barrier1);

    int j,c=1,k;
    
    //for (j=((trans_fbl_input*)input)->start_number; j<((trans_fbl_input*)input)->stop_number; j++) 
    while(1)
    {

        j=__sync_fetch_and_add(((buffer_data_inmemory*)transferdata)->node_counter,1);
        if(j>=index->fbl->number_of_buffers)
        {
            break;
        }
        fbl_soft_buffer *current_fbl_node = &index->fbl->soft_buffers[j];
        if (!current_fbl_node->initialized) {
            continue;
        }

        if (current_fbl_node->buffer_size > 0) {

            //for (k=0; k<current_fbl_node->buffer_size; k++) {
            //    r->sax = (sax_type *) current_fbl_node->sax_records[k];
            //    r->position = (file_position_type *) current_fbl_node->pos_records[k];
           //     r->insertion_mode = NO_TMP | PARTIAL;
                // Add record to index
                //printf("the position 1 is %d\n",*(r->position));
                //sleep(1);
             //   add_record_to_node(index, current_fbl_node->node, r, 1);
          // }
            flush_subtree_leaf_buffers_inmemory(index, current_fbl_node->node);

            // clear FBL records moved in LBL buffers
            //free(current_fbl_node->sax_records);
            //free(current_fbl_node->pos_records);
            // clear records read from files (free only prev sax buffers)
        }
        
    }
    free(r);

}


void* indexbulkloadingworker_pRecBuf_inmemory(void *transferdata)
{
    sax_type * sax = malloc(sizeof(sax_type) * ((buffer_data_inmemory*)transferdata)->index->settings->paa_segments);
        //struct timeval workertimestart;
    //struct timeval writetiemstart;
    //struct timeval workercurenttime;
    //struct timeval writecurenttime;
    //double worker_total_time,tee,tss;
    //gettimeofday(&workertimestart, NULL);    


    unsigned long start_number=((buffer_data_inmemory*)transferdata)->start_number;
    unsigned long stop_number=((buffer_data_inmemory*)transferdata)->stop_number;
    file_position_type *pos = malloc(sizeof(file_position_type));
    isax_index *index= ((buffer_data_inmemory*)transferdata)->index;
    ts_type * ts = malloc(sizeof(ts_type) * index->settings->timeseries_size);
    int paa_segments=((buffer_data_inmemory*)transferdata)->index->settings->paa_segments;

    unsigned long i=0;
    float *raw_file=((buffer_data_inmemory*)transferdata)->ts;
    
    for (i=start_number;i<stop_number;i++)
    {
    #ifndef DEBUG
    #if VERBOSE_LEVEL == 2
        printf("\r\x1b[32mLoading: \x1b[36m%d\x1b[0m",(i + 1));
    #endif
    #endif
        memcpy(ts,&(raw_file[i*index->settings->timeseries_size]), sizeof(float)* index->settings->timeseries_size);

        if(sax_from_ts(ts, sax, index->settings->ts_values_per_paa_segment,
                       index->settings->paa_segments, index->settings->sax_alphabet_cardinality,
                       index->settings->sax_bit_cardinality) == SUCCESS)
        {
            *pos = (file_position_type)(i*index->settings->timeseries_size);

            memcpy(&(index->sax_cache[i*index->settings->paa_segments]),sax, sizeof(sax_type)* index->settings->paa_segments);

            isax_pRecBuf_index_insert_inmemory(index, sax, pos, ((buffer_data_inmemory*)transferdata)->lock_firstnode,((buffer_data_inmemory*)transferdata)->workernumber,((buffer_data_inmemory*)transferdata)->total_workernumber);

        }
        else
        {
            fprintf(stderr, "error: cannot insert record in index, since sax representation\
                    failed to be created");
        }
    }
    free(pos);
    free(sax);
    free(ts);
    //gettimeofday(&workercurenttime, NULL);
    //tss = workertimestart.tv_sec*1000000 + (workertimestart.tv_usec); 
    //tee = workercurenttime.tv_sec*1000000  + (workercurenttime.tv_usec); 
    //worker_total_time += (tee - tss); 
    //printf("the worker time is %f\n",worker_total_time );
}
void* index_creation_pRecBuf_worker(void *transferdata)
{
    sax_type * sax = malloc(sizeof(sax_type) * ((buffer_data_inmemory*)transferdata)->index->settings->paa_segments);
        //struct timeval workertimestart;
    //struct timeval writetiemstart;
    //struct timeval workercurenttime;
    //struct timeval writecurenttime;
    //double worker_total_time,tee,tss;
    //gettimeofday(&workertimestart, NULL);    


    unsigned long start_number=((buffer_data_inmemory*)transferdata)->start_number;
    unsigned long stop_number=((buffer_data_inmemory*)transferdata)->stop_number;
    file_position_type *pos = malloc(sizeof(file_position_type));
    isax_index *index= ((buffer_data_inmemory*)transferdata)->index;
    ts_type * ts = malloc(sizeof(ts_type) * index->settings->timeseries_size);
    int paa_segments=((buffer_data_inmemory*)transferdata)->index->settings->paa_segments;

    unsigned long i=0;
    float *raw_file=((buffer_data_inmemory*)transferdata)->ts;
    
    for (i=start_number;i<stop_number;i++)
    {
    #ifndef DEBUG
    #if VERBOSE_LEVEL == 2
        printf("\r\x1b[32mLoading: \x1b[36m%d\x1b[0m",(i + 1));
    #endif
    #endif
        memcpy(ts,&(raw_file[i*index->settings->timeseries_size]), sizeof(float)* index->settings->timeseries_size);

        if(sax_from_ts(ts, sax, index->settings->ts_values_per_paa_segment,
                       index->settings->paa_segments, index->settings->sax_alphabet_cardinality,
                       index->settings->sax_bit_cardinality) == SUCCESS)
        {
            *pos = (file_position_type)(i*index->settings->timeseries_size);

            memcpy(&(index->sax_cache[i*index->settings->paa_segments]),sax, sizeof(sax_type)* index->settings->paa_segments);

            isax_pRecBuf_index_insert_inmemory(index, sax, pos, ((buffer_data_inmemory*)transferdata)->lock_firstnode,((buffer_data_inmemory*)transferdata)->workernumber,((buffer_data_inmemory*)transferdata)->total_workernumber);

        }
        else
        {
            fprintf(stderr, "error: cannot insert record in index, since sax representation\
                    failed to be created");
        }
    }
    free(pos);
    free(sax);
    free(ts);
    //gettimeofday(&workercurenttime, NULL);
    //tss = workertimestart.tv_sec*1000000 + (workertimestart.tv_usec); 
    //tee = workercurenttime.tv_sec*1000000  + (workercurenttime.tv_usec); 
    //worker_total_time += (tee - tss); 
    //printf("the worker time is %f\n",worker_total_time );

    pthread_barrier_wait(((buffer_data_inmemory*)transferdata)->lock_barrier1);

    bool have_record=false;
    int j;
    isax_node_record *r = malloc(sizeof(isax_node_record));
    //int preworkernumber=((buffer_data_inmemory*)transferdata)->total_workernumber;

    //for (j=((trans_fbl_input*)input)->start_number; j<((trans_fbl_input*)input)->stop_number; j++) 
    while(1)
    {

        j=__sync_fetch_and_add(((buffer_data_inmemory*)transferdata)->node_counter,1);

        if(j>=index->fbl->number_of_buffers)
        {
            break;
        }
        //fbl_soft_buffer *current_fbl_node = &index->fbl->soft_buffers[j];
        parallel_fbl_soft_buffer *current_fbl_node = &((parallel_first_buffer_layer*)(index->fbl))->soft_buffers[j];
        if (!current_fbl_node->initialized) {
            continue;
        }

        int i;
        have_record=false;
        for (int k = 0; k < ((buffer_data_inmemory*)transferdata)->total_workernumber; k++)
        {
        if (current_fbl_node->buffer_size[k] > 0)
            have_record=true;
            
        for (i=0; i<current_fbl_node->buffer_size[k]; i++) 
        {
                r->sax = (sax_type *) &(((current_fbl_node->sax_records[k]))[i*index->settings->paa_segments]);
                r->position = (file_position_type *) &((file_position_type *)(current_fbl_node->pos_records[k]))[i];
                r->insertion_mode = NO_TMP | PARTIAL;
                // Add record to index
                //printf("the position 1 is %d\n",*(r->position));
                //sleep(1);
                add_record_to_node(index, current_fbl_node->node, r, 1);
            }
        }
        if (have_record) 
        {
            flush_subtree_leaf_buffers_inmemory(index, current_fbl_node->node);

            // clear FBL records moved in LBL buffers

            // clear records read from files (free only prev sax buffers)
        }
        
    }
    free(r);
}
void* index_creation_pRecBuf_worker_new(void *transferdata)
{
    sax_type * sax = malloc(sizeof(sax_type) * ((buffer_data_inmemory*)transferdata)->index->settings->paa_segments);
        //struct timeval workertimestart;
    //struct timeval writetiemstart;
    //struct timeval workercurenttime;
    //struct timeval writecurenttime;
    //double worker_total_time,tee,tss;
    //gettimeofday(&workertimestart, NULL);    
    unsigned long roundfinishednumber;

    unsigned long start_number;
    unsigned long stop_number=((buffer_data_inmemory*)transferdata)->stop_number;
    file_position_type *pos = malloc(sizeof(file_position_type));
    isax_index *index= ((buffer_data_inmemory*)transferdata)->index;
    ts_type * ts = malloc(sizeof(ts_type) * index->settings->timeseries_size);
    int paa_segments=((buffer_data_inmemory*)transferdata)->index->settings->paa_segments;

    unsigned long i=0;
    float *raw_file=((buffer_data_inmemory*)transferdata)->ts;
    while(1)
    {   
        start_number=__sync_fetch_and_add(((buffer_data_inmemory*)transferdata)->shared_start_number,read_block_length);
        if(start_number>stop_number)
        {
            break;
        }
        else if(start_number>stop_number-read_block_length)
        {
            roundfinishednumber=stop_number;
        }
        else
        {
            roundfinishednumber=start_number+read_block_length;
        }
         for (i=start_number;i<roundfinishednumber;i++)
         {
            memcpy(ts,&(raw_file[i*index->settings->timeseries_size]), sizeof(float)* index->settings->timeseries_size);
            if(sax_from_ts(ts, sax, index->settings->ts_values_per_paa_segment,
                       index->settings->paa_segments, index->settings->sax_alphabet_cardinality,
                       index->settings->sax_bit_cardinality) == SUCCESS)
            {
                *pos = (file_position_type)(i*index->settings->timeseries_size);
                memcpy(&(index->sax_cache[i*index->settings->paa_segments]),sax, sizeof(sax_type)* index->settings->paa_segments);

                isax_pRecBuf_index_insert_inmemory(index, sax, pos, ((buffer_data_inmemory*)transferdata)->lock_firstnode,((buffer_data_inmemory*)transferdata)->workernumber,((buffer_data_inmemory*)transferdata)->total_workernumber);

            }
            else
            {
                fprintf(stderr, "error: cannot insert record in index, since sax representation\
                    failed to be created");
            }
        }

    }

    free(pos);
    free(sax);
    free(ts);
    //gettimeofday(&workercurenttime, NULL);
    //tss = workertimestart.tv_sec*1000000 + (workertimestart.tv_usec); 
    //tee = workercurenttime.tv_sec*1000000  + (workercurenttime.tv_usec); 
    //worker_total_time += (tee - tss); 
    //printf("the worker time is %f\n",worker_total_time );

    pthread_barrier_wait(((buffer_data_inmemory*)transferdata)->lock_barrier1);

    pthread_barrier_wait(((buffer_data_inmemory*)transferdata)->lock_barrier2);
    bool have_record=false;
    int j;
    isax_node_record *r = malloc(sizeof(isax_node_record));
    //int preworkernumber=((buffer_data_inmemory*)transferdata)->total_workernumber;

    //for (j=((trans_fbl_input*)input)->start_number; j<((trans_fbl_input*)input)->stop_number; j++) 
    while(1)
    {

        j=__sync_fetch_and_add(((buffer_data_inmemory*)transferdata)->node_counter,1);

        if(j>=index->fbl->number_of_buffers)
        {
            break;
        }
        //fbl_soft_buffer *current_fbl_node = &index->fbl->soft_buffers[j];
        parallel_fbl_soft_buffer *current_fbl_node = &((parallel_first_buffer_layer*)(index->fbl))->soft_buffers[j];
        if (!current_fbl_node->initialized) {
            continue;
        }

        int i;
        have_record=false;
        for (int k = 0; k < ((buffer_data_inmemory*)transferdata)->total_workernumber; k++)
        {
            if (current_fbl_node->buffer_size[k] > 0)
            have_record=true;
            for (i=0; i<current_fbl_node->buffer_size[k]; i++) 
            {
                r->sax = (sax_type *) &(((current_fbl_node->sax_records[k]))[i*index->settings->paa_segments]);
                r->position = (file_position_type *) &((file_position_type *)(current_fbl_node->pos_records[k]))[i];
                r->insertion_mode = NO_TMP | PARTIAL;
                // Add record to index
                //printf("the position 1 is %d\n",*(r->position));
                //sleep(1);
                add_record_to_node(index, current_fbl_node->node, r, 1);
            }
        }
        if (have_record) 
        {
            flush_subtree_leaf_buffers_inmemory(index, current_fbl_node->node);

            // clear FBL records moved in LBL buffers

            // clear records read from files (free only prev sax buffers)



    
        }
    }
    free(r);
}
root_mask_type isax_fbl_index_insert_inmemory_para(isax_index *index, 
                                    sax_type * sax,
                                    file_position_type * pos, pthread_mutex_t *lock_fbl, pthread_mutex_t *lock_cbl,pthread_mutex_t *lock_firstnode,pthread_mutex_t *lock_index)
{
    int i,t;
    int totalsize = index->settings->max_total_buffer_size;

    // Create mask for the first bit of the sax representation
    
    // Step 1: Check if there is a root node that represents the 
    //         current node's sax representation
    
    // TODO: Create INSERTION SHORT AND BINARY SEARCH METHODS.

    root_mask_type first_bit_mask = 0x00;       

    CREATE_MASK(first_bit_mask, index, sax);
    
    //__sync_fetch_and_add(&(index->total_records),1);

    pthread_mutex_lock(&(lock_cbl[first_bit_mask%LOCK_SIZE])); 
    insert_to_fbl_m(index->fbl, sax, pos,
                first_bit_mask, index,lock_firstnode,lock_fbl);

    pthread_mutex_unlock(&(lock_cbl[first_bit_mask%LOCK_SIZE]));

    return first_bit_mask;
}
root_mask_type isax_pRecBuf_index_insert_inmemory(isax_index *index, 
                                    sax_type * sax,
                                    file_position_type * pos,pthread_mutex_t *lock_firstnode,int workernumber,int total_workernumber)
{
    int i,t;
    int totalsize = index->settings->max_total_buffer_size;

    // Create mask for the first bit of the sax representation
    
    // Step 1: Check if there is a root node that represents the 
    //         current node's sax representation
    
    // TODO: Create INSERTION SHORT AND BINARY SEARCH METHODS.

    root_mask_type first_bit_mask = 0x00;       

    CREATE_MASK(first_bit_mask, index, sax);
    

    //insert_to_fbl_m(index->fbl, sax, pos,first_bit_mask, index,lock_firstnode,lock_fbl);
    insert_to_pRecBuf((parallel_first_buffer_layer*)(index->fbl), sax, pos,first_bit_mask, index,lock_firstnode,workernumber,total_workernumber);
    return first_bit_mask;
}
root_mask_type isax_fbl_index_insert_inmemory(isax_index *index, 
                                    sax_type * sax,
                                    file_position_type * pos)
{

    // Create mask for the first bit of the sax representation
    
    // Step 1: Check if there is a root node that represents the 
    //         current node's sax representation
    
    // TODO: Create INSERTION SHORT AND BINARY SEARCH METHODS.
    root_mask_type first_bit_mask = 0x00;
    CREATE_MASK(first_bit_mask, index, sax);
    insert_to_fbl(index->fbl, sax, pos,
                  first_bit_mask, index);
    index->total_records++;
    return first_bit_mask;
}
enum response flush_fbl_inmemory_m(first_buffer_layer *fbl, isax_index *index) 
{
    #ifdef DEBUG
    printf("*** FLUSHING ***\n\n");
    #else
    #if VERBOSE_LEVEL == 2
    printf("\n");
    fflush(stdout);
    int i=1;
    fprintf(stdout, "\x1b[31mFlushing: \x1b[36m00.00%%\x1b[0m");
    #endif
    #if VERBOSE_LEVEL == 1
    printf("\n");
    fflush(stdout);
    int i=1;
    fprintf(stdout, "Flushing...\n");
    #endif    
    #endif
    int j;
    transferfblinmemory input_data;
    pthread_t threadid[maxquerythread];
    pthread_mutex_t lock_fbl_conter=PTHREAD_MUTEX_INITIALIZER;

    input_data.index=index;
    input_data.conternumber=0;
    input_data.stop_number=fbl->number_of_buffers;
    //input_data.nodeid=nodeid;
    for (int k = 0; k < maxquerythread; k++)
    {
        pthread_create(&(threadid[k]),NULL,flush_fbl_inmemory_worker,(void*)&(input_data));
    }
        
    for (int k = 0; k < maxquerythread; k++)
    {
        pthread_join(threadid[k],NULL);
    }   
    
    fbl->current_record_index = 0;
    fbl->current_record = fbl->hard_buffer;
    index->sax_cache_size=index->total_records;
    
    return SUCCESS;
}
enum response flush_pRecBuf_inmemory(parallel_first_buffer_layer *fbl, isax_index *index) 
{
    #ifdef DEBUG
    printf("*** FLUSHING ***\n\n");
    #else
    #if VERBOSE_LEVEL == 2
    printf("\n");
    fflush(stdout);
    int i=1;
    fprintf(stdout, "\x1b[31mFlushing: \x1b[36m00.00%%\x1b[0m");
    #endif
    #if VERBOSE_LEVEL == 1
    printf("\n");
    fflush(stdout);
    int i=1;
    fprintf(stdout, "Flushing...\n");
    #endif    
    #endif
    int j;
    transferfblinmemory input_data;
    pthread_t threadid[maxquerythread];
    pthread_mutex_t lock_fbl_conter=PTHREAD_MUTEX_INITIALIZER;

    input_data.index=index;
    input_data.conternumber=0;
    input_data.stop_number=fbl->number_of_buffers;
    input_data.preworkernumber=maxquerythread;
    for (int k = 0; k < maxquerythread; k++)
    {
        pthread_create(&(threadid[k]),NULL,flush_pRecBuf_inmemory_worker,(void*)&(input_data));
    }
        
    for (int k = 0; k < maxquerythread; k++)
    {
        pthread_join(threadid[k],NULL);
    }   
    
    fbl->current_record_index = 0;
    fbl->current_record = fbl->hard_buffer;
    index->sax_cache_size=index->total_records;
    
    return SUCCESS;
}
void* flush_fbl_inmemory_worker(void *input)
{
                //gettimeofday(&writecurenttime, NULL); 
            //write_total_time += writecurenttime.tv_sec*1000000 + (writecurenttime.tv_usec)-writetiemstart.tv_sec*1000000 - (writetiemstart.tv_usec); 
            //COUNT_CAL_TIME_END
            // clear FBL records moved in LBL buffers
    struct timeval workertimestart;
    //struct timeval writetiemstart;
    struct timeval workercurenttime;
    //s//truct timeval writecurenttime;
    double worker_total_time,tee,tss;
    //gettimeofday(&workertimestart, NULL);
    gettimeofday(&workertimestart, NULL);

            // flush index node
            //COUNT_CAL_TIME_START
            //gettimeofday(&writetiemstart, NULL);

    isax_index *index=((transferfblinmemory*)input)->index;

    int j,c=1;
    isax_node_record *r = malloc(sizeof(isax_node_record));
    //for (j=((trans_fbl_input*)input)->start_number; j<((trans_fbl_input*)input)->stop_number; j++) 
    while(1)
    {

        j=__sync_fetch_and_add(&((transferfblinmemory*)input)->conternumber,1);
        if(j>=((transferfblinmemory*)input)->stop_number)
        {
            break;
        }
        fbl_soft_buffer *current_fbl_node = &index->fbl->soft_buffers[j];
        if (!current_fbl_node->initialized) {
            continue;
        }

    #ifndef DEBUG
    #if VERBOSE_LEVEL == 2
        fprintf(stdout,"\r\x1b[31mFlushing: \x1b[36m%2.2lf%%\x1b[0m", ((float)c/(float)index->root_nodes)*100);
        c++;
        fflush(stdout);
    #endif
    #endif
        int i;
        if (current_fbl_node->buffer_size > 0) {
            for (i=0; i<current_fbl_node->buffer_size; i++) {
                r->sax = (sax_type *) current_fbl_node->sax_records[i];
                r->position = (file_position_type *) current_fbl_node->pos_records[i];
                r->insertion_mode = NO_TMP | PARTIAL;
                // Add record to index
                //printf("the position 1 is %d\n",*(r->position));
                //sleep(1);
                add_record_to_node(index, current_fbl_node->node, r, 1);
            }
            flush_subtree_leaf_buffers_inmemory(index, current_fbl_node->node);

            // clear FBL records moved in LBL buffers
            //free(current_fbl_node->sax_records);
            //free(current_fbl_node->pos_records);
            // clear records read from files (free only prev sax buffers)
        }
        
    }
    free(r);
    gettimeofday(&workercurenttime, NULL);
    tss = workertimestart.tv_sec*1000000 + (workertimestart.tv_usec); 
    tee = workercurenttime.tv_sec*1000000  + (workercurenttime.tv_usec); 
    worker_total_time += (tee - tss); 
    printf("the worker time is %f\n",worker_total_time );
}
void* flush_pRecBuf_inmemory_worker(void *input)
{
        //struct timeval workertimestart;
    //struct timeval writetiemstart;
    //struct timeval workercurenttime;
    //struct timeval writecurenttime;
    //double worker_total_time,tss,tee;
    //gettimeofday(&workertimestart, NULL);

    isax_index *index=((transferfblinmemory*)input)->index;
    bool have_record=false;
    int j,c=1;
    isax_node_record *r = malloc(sizeof(isax_node_record));
    int preworkernumber=((transferfblinmemory*)input)->preworkernumber;

    //for (j=((trans_fbl_input*)input)->start_number; j<((trans_fbl_input*)input)->stop_number; j++) 
    while(1)
    {

        j=__sync_fetch_and_add(&((transferfblinmemory*)input)->conternumber,1);

        if(j>=((transferfblinmemory*)input)->stop_number)
        {
            break;
        }
        //fbl_soft_buffer *current_fbl_node = &index->fbl->soft_buffers[j];
        parallel_fbl_soft_buffer *current_fbl_node = &((parallel_first_buffer_layer*)(index->fbl))->soft_buffers[j];
        if (!current_fbl_node->initialized) {
            continue;
        }

    #ifndef DEBUG
    #if VERBOSE_LEVEL == 2
        fprintf(stdout,"\r\x1b[31mFlushing: \x1b[36m%2.2lf%%\x1b[0m", ((float)c/(float)index->root_nodes)*100);
        c++;
        fflush(stdout);
    #endif
    #endif
        int i;
        have_record=false;
        for (int k = 0; k < preworkernumber; k++)
        {
        if (current_fbl_node->buffer_size[k] > 0)
            have_record=true;
            
        for (i=0; i<current_fbl_node->buffer_size[k]; i++) 
        {
                r->sax = (sax_type *) &(((current_fbl_node->sax_records[k]))[i*index->settings->paa_segments]);
                r->position = (file_position_type *) &((file_position_type *)(current_fbl_node->pos_records[k]))[i];
                r->insertion_mode = NO_TMP | PARTIAL;
                // Add record to index
                //printf("the position 1 is %d\n",*(r->position));
                //sleep(1);
                add_record_to_node(index, current_fbl_node->node, r, 1);
            }
        }
              if (have_record) 
        {
            flush_subtree_leaf_buffers_inmemory(index, current_fbl_node->node);

            // clear FBL records moved in LBL buffers

            // clear records read from files (free only prev sax buffers)
        }
        
    }
    free(r);
        //gettimeofday(&workercurenttime, NULL);
    //tss = workertimestart.tv_sec*1000000 + (workertimestart.tv_usec); 
    //tee = workercurenttime.tv_sec*1000000  + (workercurenttime.tv_usec); 
    //worker_total_time += (tee - tss); 
    //printf("the worker time is %f\n",worker_total_time );
}
enum response flush_fbl_inmemory(first_buffer_layer *fbl, isax_index *index) 
{
    #ifdef DEBUG
    printf("*** FLUSHING ***\n\n");
    #else
    #if VERBOSE_LEVEL == 2
    printf("\n");
    fflush(stdout);
    int i=1;
    fprintf(stdout, "\x1b[31mFlushing: \x1b[36m00.00%%\x1b[0m");
    #endif
    #if VERBOSE_LEVEL == 1
    printf("\n");
    fflush(stdout);
    int i=1;
    fprintf(stdout, "Flushing...\n");
    #endif    
    #endif
    
    int c = 1;
    int j;
    isax_node_record *r = malloc(sizeof(isax_node_record));
    for (j=0; j<fbl->number_of_buffers; j++) 
    {
        
        fbl_soft_buffer *current_fbl_node = &index->fbl->soft_buffers[j];
        
        if (!current_fbl_node->initialized) {
            continue;
        }
        

        int i;
        if (current_fbl_node->buffer_size > 0) {
            // For all records in this buffer 
            //COUNT_CAL_TIME_START
            for (i=0; i<current_fbl_node->buffer_size; i++) {
                r->sax = (sax_type *) current_fbl_node->sax_records[i];
                r->position = (file_position_type *) current_fbl_node->pos_records[i];
                r->insertion_mode = NO_TMP | PARTIAL;
                // Add record to index
                add_record_to_node(index, current_fbl_node->node, r, 1);
            }
            flush_subtree_leaf_buffers_inmemory(index, current_fbl_node->node);
            free(current_fbl_node->sax_records);
            free(current_fbl_node->pos_records);
            // flush index node
            //COUNT_CAL_TIME_START
        }
        
    }
    free(r);
    fbl->current_record_index = 0;
    fbl->current_record = fbl->hard_buffer;
        index->sax_cache_size=index->total_records;

    return SUCCESS;
} 
isax_node * add_record_to_node_inmemory(isax_index *index, 
                                 isax_node *tree_node, 
                                 isax_node_record *record,
                                 const char leaf_size_check) 
{
    #ifdef DEBUG
    printf("*** Adding to node ***\n\n");
    #endif
    isax_node *node = tree_node;

    // Traverse tree
    while (!node->is_leaf) {
        int location = index->settings->sax_bit_cardinality - 1 -
        node->split_data->split_mask[node->split_data->splitpoint];
        
        root_mask_type mask = index->settings->bit_masks[location];
        if(record->sax[node->split_data->splitpoint] & mask) 
        {
            node = node->right_child;
        }
        else
        {
            node = node->left_child;
        }
    }
    // Check if split needed
    if ((node->leaf_size) >= index->settings->max_leaf_size && leaf_size_check) {
    #ifdef DEBUG
        printf(">>> %s leaf size: %d\n\n", node->filename, node->leaf_size);
    #endif
        split_node_inmemory(index, node);
        add_record_to_node(index, node, record, leaf_size_check);
    }
    else
    {
        if (node->filename == NULL) {
            create_node_filename(index,node,record);
        }
        add_to_node_buffer(node->buffer, record, index->settings->paa_segments, 
                           index->settings->timeseries_size);
        node->leaf_size++;

    }
    return node;
}


enum response flush_subtree_leaf_buffers_inmemory (isax_index *index, isax_node *node)
{
    
    if (node->is_leaf && node->filename != NULL) {
        // Set that unloaded data exist in disk
        if (node->buffer->partial_buffer_size > 0 
            || node->buffer->tmp_partial_buffer_size > 0) {
            node->has_partial_data_file = 1;
        }
        // Set that the node has flushed full data in the disk
        if (node->buffer->full_buffer_size > 0 
            || node->buffer->tmp_full_buffer_size > 0) {
            node->has_full_data_file = 1;
        }

        if(node->has_full_data_file) {
            int prev_rec_count = node->leaf_size - (node->buffer->full_buffer_size + node->buffer->tmp_full_buffer_size);
            
            int previous_page_size =  ceil((float) (prev_rec_count * index->settings->full_record_size) / (float) PAGE_SIZE);   
            int current_page_size =   ceil((float) (node->leaf_size * index->settings->full_record_size) / (float) PAGE_SIZE);
            __sync_fetch_and_add(&(index->memory_info.disk_data_full),(current_page_size - previous_page_size));
            //index->memory_info.disk_data_full += (current_page_size - previous_page_size);
        }
        if(node->has_partial_data_file) {
            int prev_rec_count = node->leaf_size - (node->buffer->partial_buffer_size + node->buffer->tmp_partial_buffer_size);
            
            int previous_page_size =  ceil((float) (prev_rec_count * index->settings->partial_record_size) / (float) PAGE_SIZE);   
            int current_page_size =   ceil((float) (node->leaf_size * index->settings->partial_record_size) / (float) PAGE_SIZE);
            
            //index->memory_info.disk_data_partial += (current_page_size - previous_page_size);
            __sync_fetch_and_add(&(index->memory_info.disk_data_partial),(current_page_size - previous_page_size));
        }
        if(node->has_full_data_file && node->has_partial_data_file) {
             printf("WARNING: (Mem size counting) this leaf has both partial and full data.\n");
        }
        //index->memory_info.disk_data_full += (node->buffer->full_buffer_size + 
                                              //node->buffer->tmp_full_buffer_size);
        __sync_fetch_and_add(&(index->memory_info.disk_data_full),(node->buffer->full_buffer_size + node->buffer->tmp_full_buffer_size));
        //index->memory_info.disk_data_partial += (node->buffer->partial_buffer_size + 
                                                 //node->buffer->tmp_partial_buffer_size); 
         __sync_fetch_and_add(&(index->memory_info.disk_data_partial),(node->buffer->partial_buffer_size + node->buffer->tmp_partial_buffer_size));
        //flush_node_buffer(node->buffer, index->settings->paa_segments, 
                          //index->settings->timeseries_size,
                          //node->filename);
    }
    else if (!node->is_leaf)
    {
        flush_subtree_leaf_buffers_inmemory(index, node->left_child);
        flush_subtree_leaf_buffers_inmemory(index, node->right_child);
    }
    
    return SUCCESS;
}
isax_index * isax_index_init_inmemory(isax_index_settings *settings)
{
    isax_index *index = malloc(sizeof(isax_index));
    if(index == NULL) {
        fprintf(stderr,"error: could not allocate memory for index structure.\n");
        return NULL;
    }
    index->memory_info.mem_tree_structure = 0;
    index->memory_info.mem_data = 0;
    index->memory_info.mem_summaries = 0;
    index->memory_info.disk_data_full = 0;
    index->memory_info.disk_data_partial = 0;
    
    index->settings = settings;
    index->first_node = NULL;
    index->fbl = initialize_fbl(settings->initial_fbl_buffer_size,
                                pow(2, settings->paa_segments), 
                                settings->max_total_buffer_size+DISK_BUFFER_SIZE*(PROGRESS_CALCULATE_THREAD_NUMBER-1), index);




  
    index->sax_cache = NULL;
    
    index->total_records = 0;
    index->loaded_records = 0;

    index->root_nodes = 0;
    index->allocated_memory = 0;
    index->has_wedges = 0;
    //index->locations = malloc(sizeof(int) * settings->timeseries_size);

    index->answer = malloc(sizeof(ts_type) * settings->timeseries_size);
    return index;
}

