//
//  parallel_index_engine.c
//  
//
//  Created by Botao PENG on 29/1/18.
//


#include "../../config.h"
#include "../../globals.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <inttypes.h>
#include <pthread.h>
#include <unistd.h>
//#include <semaphore.h>
#include <stdbool.h>

#include "ads/isax_node.h"
#include "ads/isax_index.h"
#include "ads/isax_query_engine.h"
#include "ads/isax_node_record.h"
#include "ads/isax_file_loaders.h"
#include "ads/isax_first_buffer_layer.h"
#include "ads/parallel_index_engine.h"
#include "ads/parallel_query_engine.h"


void isax_index_binary_file_m(const char *ifilename, int ts_num, isax_index *index,int calculate_thread)
{
    fprintf(stderr, ">>> Indexing: %s\n", ifilename);
    FILE * ifile;
    
    COUNT_INPUT_TIME_START
    ifile = fopen (ifilename,"rb");
    COUNT_INPUT_TIME_END
    index_buffer_data *input_data=malloc(sizeof(index_buffer_data)*(calculate_thread-1));
    file_position_type *pos = malloc(sizeof(file_position_type));
    pthread_t threadid[calculate_thread-1];
    bool sax_fist_time_check = false;
    long int ts_loaded = 0;
    int j,conter=0;
    long long int i;
    int prev_flush_time=0,now_flush_time=0;
    int sax_save_number;
    //initial the locks
    pthread_mutex_t lock_record=PTHREAD_MUTEX_INITIALIZER,lockfbl=PTHREAD_MUTEX_INITIALIZER,lock_index=PTHREAD_MUTEX_INITIALIZER,
                    lock_firstnode=PTHREAD_MUTEX_INITIALIZER,lock_disk=PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_t *lockcbl;
    pthread_barrier_t lock_barrier1, lock_barrier2;
    pthread_barrier_init(&lock_barrier1, NULL, calculate_thread);
    pthread_barrier_init(&lock_barrier2, NULL, calculate_thread);

    // set the thread on decided cpu

    lockcbl= malloc(sizeof(pthread_mutex_t)*LOCK_SIZE);

    for(i=0; i<LOCK_SIZE ;i++)
    {
        pthread_mutex_init(&lockcbl[i], NULL);
    }

    for (i = 0; i < (calculate_thread-1); i++)
    {
        input_data[i].index=index;
        input_data[i].lock_fbl=&lockfbl;
        input_data[i].lock_record=&lock_record;
        input_data[i].lock_cbl=lockcbl;
        input_data[i].lock_firstnode =&lock_firstnode;
        input_data[i].lock_index=&lock_index;
        input_data[i].blocid=i*read_block_length;
        input_data[i].lock_disk=&lock_disk;
        
        input_data[i].lock_barrier1=&lock_barrier1;
        input_data[i].lock_barrier2=&lock_barrier2;
        input_data[i].finished=false;
    }
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


    ts_type * ts     = malloc(sizeof(ts_type) * index->settings->timeseries_size*read_block_length*(calculate_thread-1));
    ts_type * ts1    = malloc(sizeof(ts_type) * index->settings->timeseries_size*read_block_length*(calculate_thread-1));
    ts_type * ts2;
    sax_type * saxv  = malloc(sizeof(sax_type) * index->settings->paa_segments*read_block_length*(calculate_thread-1));
    sax_type * saxv1 = malloc(sizeof(sax_type) * index->settings->paa_segments*read_block_length*(calculate_thread-1));
    sax_type * saxv2;
    
    index->settings->raw_filename = malloc(256);
    strcpy(index->settings->raw_filename, ifilename);

    #ifdef BENCHMARK    
    int percentage = (int) (ts_num / (file_position_type) 100);
    #endif
        
    *pos = ftell(ifile);


    if(ts_num>read_block_length*(calculate_thread-1))
    {   COUNT_INPUT_TIME_START
        fread(ts1, sizeof(ts_type), index->settings->timeseries_size*read_block_length*(calculate_thread-1), ifile);
        COUNT_INPUT_TIME_END
        ts2   =ts;
        ts    =ts1;  
        ts1   =ts2;

        for ( j = 0; j < (calculate_thread-1); j++)
        {   
            input_data[j].pos        =*pos+index->settings->timeseries_size*sizeof(ts_type)*j*read_block_length;
            input_data[j].ts         =&(ts[index->settings->timeseries_size*j*read_block_length]);
            input_data[j].saxv       =saxv;
            input_data[j].fin_number =read_block_length;
        }
        for (j = 0; j < (calculate_thread-1); j++)
        {
            pthread_create(&(threadid[j]),NULL,indexbulkloadingworker,(void*)&(input_data[j]));
        }

        for (i = read_block_length*(calculate_thread-1)*2; i <= ts_num; i+=read_block_length*(calculate_thread-1))
        {
            
            *pos = ftell(ifile);
            //read the data of next round
            COUNT_INPUT_TIME_START
            fread(ts1, sizeof(ts_type), index->settings->timeseries_size*read_block_length*(calculate_thread-1), ifile);
            COUNT_INPUT_TIME_END
            //write the sax in disk (last round)
            if (sax_fist_time_check)
            {
                COUNT_OUTPUT_TIME_START
                fwrite(saxv1, index->settings->sax_byte_size, read_block_length*(calculate_thread-1), index->sax_file);
                COUNT_OUTPUT_TIME_END
            }
            else
            { 
                sax_fist_time_check=true;
            }

            pthread_barrier_wait(&lock_barrier1);
            //wait for the finish of other threads
            ts2   =ts;
            ts    =ts1;  
            ts1   =ts2;
            __sync_fetch_and_add(&(index->fbl->current_record_index),read_block_length*(calculate_thread-1));
            now_flush_time=i/(index->settings->max_total_buffer_size);
            if(now_flush_time!=prev_flush_time)
            {
                COUNT_QUEUE_TIME_START
                indexconstruction(index->fbl, index,&lock_index,&lock_disk,calculate_thread);
                COUNT_QUEUE_TIME_END
            }
            //printf("the saxv2 in the worker is %d\n",saxv2[0] );
            saxv2 =saxv;
            saxv  =saxv1;
            saxv1 =saxv2;

            prev_flush_time=now_flush_time;
            for ( j = 0; j < (calculate_thread-1); j++)
            {   
                input_data[j].pos        =*pos+index->settings->timeseries_size*sizeof(ts_type)*j*read_block_length;
                input_data[j].ts         =&(ts[index->settings->timeseries_size*j*read_block_length]);
                input_data[j].saxv       =saxv;
                input_data[j].fin_number =read_block_length;
            }
            pthread_barrier_wait(&lock_barrier2);
        }

        pthread_barrier_wait(&lock_barrier1);
        for (j = 0; j < (calculate_thread-1); j++)
        {
            input_data[j].finished=true;
        }
            //wait for the finish of other threads
        __sync_fetch_and_add(&(index->fbl->current_record_index),read_block_length*(calculate_thread-1));
        now_flush_time=i/(index->settings->max_total_buffer_size);
        if(now_flush_time!=prev_flush_time)
        {
            COUNT_QUEUE_TIME_START
            indexconstruction(index->fbl, index,&lock_index,&lock_disk,calculate_thread);
            COUNT_QUEUE_TIME_END
        }

        prev_flush_time=now_flush_time;
        for ( j = 0; j < (calculate_thread-1); j++)
        {   
            input_data[j].pos        =*pos+index->settings->timeseries_size*sizeof(ts_type)*j*read_block_length;
            input_data[j].ts         =&(ts[index->settings->timeseries_size*j*read_block_length]);
            input_data[j].saxv       =saxv;
            input_data[j].fin_number =read_block_length;
        }
        pthread_barrier_wait(&lock_barrier2);
        for (j = 0; j < (calculate_thread-1); j++)
        {
            pthread_join(threadid[j],NULL);
        }
    }
    COUNT_INPUT_TIME_START
    *pos = ftell(ifile);    
    fread(ts1, sizeof(ts_type), index->settings->timeseries_size*(ts_num%(read_block_length*(calculate_thread-1))), ifile);
    COUNT_INPUT_TIME_END
    ts2   =ts;
    ts    =ts1;
    ts1   =ts2;
    


    //handle the rest data
    int conter_ts_number=ts_num%(read_block_length*(calculate_thread-1));
    sax_save_number=conter_ts_number;

    for ( j = 0; j < (calculate_thread-1); j++)
    {   
        input_data[j].pos = *pos+index->settings->timeseries_size*sizeof(ts_type)*j*read_block_length;
        conter++;
        input_data[j].ts=&(ts[index->settings->timeseries_size*j*read_block_length]);
        input_data[j].fin_number=min(conter_ts_number,read_block_length);
        input_data[j].saxv=saxv;
        conter_ts_number=conter_ts_number-read_block_length;
            
        if(conter_ts_number<0)
            break;
    }

    pthread_barrier_init(&lock_barrier1, NULL, conter+1);
    pthread_barrier_init(&lock_barrier2, NULL, conter+1);
    for ( j = 0; j < conter; j++)
    { 
        input_data[j].finished=false;
        pthread_create(&(threadid[j]),NULL,indexbulkloadingworker,(void*)&(input_data[j]));
    }

    if (sax_fist_time_check)
    {
        pthread_mutex_lock(&lock_disk);
        COUNT_OUTPUT_TIME_START
        fwrite(saxv1, index->settings->sax_byte_size, read_block_length*(calculate_thread-1), index->sax_file);
        COUNT_OUTPUT_TIME_END
        pthread_mutex_unlock(&lock_disk);
    }
    else
    {
        sax_fist_time_check=true;
    }
    pthread_barrier_wait(&lock_barrier1);
    saxv2 =saxv;
    saxv  =saxv1;
    saxv1 =saxv2;
    for ( j = 0; j < conter; j++)
    { 
        input_data[j].finished=true;
    }
    pthread_barrier_wait(&lock_barrier2);

    for (j = 0; j < conter; j++)
    {
        pthread_join(threadid[j],NULL);
    }
    pthread_mutex_lock(&lock_disk);
    COUNT_OUTPUT_TIME_START
    fwrite(saxv1, index->settings->sax_byte_size, sax_save_number, index->sax_file);
    COUNT_OUTPUT_TIME_END
    pthread_mutex_unlock(&lock_disk);
    __sync_fetch_and_add(&(index->fbl->current_record_index),sax_save_number);
    COUNT_QUEUE_TIME_START
    indexconstruction(index->fbl, index,&lock_index,&lock_disk,calculate_thread);
    COUNT_QUEUE_TIME_END
    free(ts);
    free(ts1);
    free(input_data);
    free(lockcbl);
    free(saxv);
    free(saxv1);
    //free(sax);
    free(pos);
    pthread_barrier_destroy(&lock_barrier1);
    pthread_barrier_destroy(&lock_barrier2);
    COUNT_INPUT_TIME_START
    fclose(ifile);
    COUNT_INPUT_TIME_END
}

void isax_index_binary_file_m_new(const char *ifilename, int ts_num, isax_index *index,int calculate_thread)
{
    fprintf(stderr, ">>> Indexing: %s\n", ifilename);
    FILE * ifile;
    COUNT_INPUT_TIME_START
    ifile = fopen (ifilename,"rb");
    COUNT_INPUT_TIME_END
    index_buffer_data *input_data=malloc(sizeof(index_buffer_data)*(calculate_thread-1));
    file_position_type *pos = malloc(sizeof(file_position_type));
    pthread_t threadid[calculate_thread-1];
    bool sax_fist_time_check = false;
    long int ts_loaded = 0;
    int j,conter=0;
    long long int i;
    int prev_flush_time=0,now_flush_time=0;
    int sax_save_number;
    int nodecounter=0;
    int bufferpresize[index->fbl->number_of_buffers];

    for (int i = 0; i < index->fbl->number_of_buffers; i++)
    {
        bufferpresize[i]=0;
    }
    //initial the locks
    pthread_mutex_t lock_record=PTHREAD_MUTEX_INITIALIZER,lockfbl=PTHREAD_MUTEX_INITIALIZER,lock_index=PTHREAD_MUTEX_INITIALIZER,
                    lock_firstnode=PTHREAD_MUTEX_INITIALIZER,lock_disk=PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_t *lockcbl;
    pthread_barrier_t lock_barrier1, lock_barrier2, lock_barrier3;
    pthread_barrier_init(&lock_barrier1, NULL, calculate_thread);
    pthread_barrier_init(&lock_barrier2, NULL, calculate_thread);
    pthread_barrier_init(&lock_barrier3, NULL, calculate_thread-1);
    // set the thread on decided cpu

    lockcbl= malloc(sizeof(pthread_mutex_t)*LOCK_SIZE);

    for(i=0; i<LOCK_SIZE ;i++)
    {
        pthread_mutex_init(&lockcbl[i], NULL);
    }

    for (i = 0; i < (calculate_thread-1); i++)
    {
        input_data[i].index=index;
        input_data[i].lock_fbl=&lockfbl;
        input_data[i].lock_record=&lock_record;
        input_data[i].lock_cbl=lockcbl;
        input_data[i].lock_firstnode =&lock_firstnode;
        input_data[i].lock_index=&lock_index;
        input_data[i].blocid=i*read_block_length;
        input_data[i].lock_disk=&lock_disk;
        input_data[i].bufferpresize=bufferpresize;
        input_data[i].lock_barrier1=&lock_barrier1;
        input_data[i].lock_barrier2=&lock_barrier2;
        input_data[i].lock_barrier3=&lock_barrier3;
        input_data[i].finished=false;
        input_data[i].nodecounter=&nodecounter;
    }
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


    ts_type * ts     = malloc(sizeof(ts_type) * index->settings->timeseries_size*read_block_length*(calculate_thread-1));
    ts_type * ts1    = malloc(sizeof(ts_type) * index->settings->timeseries_size*read_block_length*(calculate_thread-1));
    ts_type * ts2;
    sax_type * saxv  = malloc(sizeof(sax_type) * index->settings->paa_segments*read_block_length*(calculate_thread-1));
    sax_type * saxv1 = malloc(sizeof(sax_type) * index->settings->paa_segments*read_block_length*(calculate_thread-1));
    sax_type * saxv2;
    
    index->settings->raw_filename = malloc(256);
    strcpy(index->settings->raw_filename, ifilename);

        
    *pos = ftell(ifile);


    if(ts_num>read_block_length*(calculate_thread-1))
    {
        COUNT_INPUT_TIME_START
        fread(ts1, sizeof(ts_type), index->settings->timeseries_size*read_block_length*(calculate_thread-1), ifile);
        COUNT_INPUT_TIME_END
        ts2   =ts;
        ts    =ts1;  
        ts1   =ts2;

        for ( j = 0; j < (calculate_thread-1); j++)
        {   
            input_data[j].pos        =*pos+index->settings->timeseries_size*sizeof(ts_type)*j*read_block_length;
            input_data[j].ts         =&(ts[index->settings->timeseries_size*j*read_block_length]);
            input_data[j].saxv       =saxv;
            input_data[j].fin_number =read_block_length;
        }
        for (j = 0; j < (calculate_thread-1); j++)
        {
            pthread_create(&(threadid[j]),NULL,indexbulkloadingworker_new,(void*)&(input_data[j]));
        }

        for (i = read_block_length*(calculate_thread-1)*2; i <= ts_num; i+=read_block_length*(calculate_thread-1))
        {
            
            *pos = ftell(ifile);
            //read the data of next round
            COUNT_INPUT_TIME_START
            fread(ts1, sizeof(ts_type), index->settings->timeseries_size*read_block_length*(calculate_thread-1), ifile);
            COUNT_INPUT_TIME_END
            
            //write the sax in disk (last round)
            if (sax_fist_time_check)
            {
                COUNT_OUTPUT_TIME_START
                fwrite(saxv1, index->settings->sax_byte_size, read_block_length*(calculate_thread-1), index->sax_file);
                COUNT_OUTPUT_TIME_END
            }
            else
            { 
                sax_fist_time_check=true;
            }

            pthread_barrier_wait(&lock_barrier1);
            //wait for the finish of other threads
            ts2   =ts;
            ts    =ts1;  
            ts1   =ts2;
            __sync_fetch_and_add(&(index->fbl->current_record_index),read_block_length*(calculate_thread-1));
            now_flush_time=i/(index->settings->max_total_buffer_size);
            if(now_flush_time!=prev_flush_time)
            {
                COUNT_OUTPUT_TIME_START
                indexflush(index->fbl, index,&lock_index,&lock_disk,calculate_thread);
                COUNT_OUTPUT_TIME_END
                    for (int i = 0; i < index->fbl->number_of_buffers; i++)
            {
                    bufferpresize[i]=0;
            }
            }
            //printf("the saxv2 in the worker is %d\n",saxv2[0] );
            saxv2 =saxv;
            saxv  =saxv1;
            saxv1 =saxv2;

            prev_flush_time=now_flush_time;
            for ( j = 0; j < (calculate_thread-1); j++)
            {   
                input_data[j].pos        =*pos+index->settings->timeseries_size*sizeof(ts_type)*j*read_block_length;
                input_data[j].ts         =&(ts[index->settings->timeseries_size*j*read_block_length]);
                input_data[j].saxv       =saxv;
                input_data[j].fin_number =read_block_length;
            }
            nodecounter=0;
            pthread_barrier_wait(&lock_barrier2);
            
        }

        pthread_barrier_wait(&lock_barrier1);
        for (j = 0; j < (calculate_thread-1); j++)
        {
            input_data[j].finished=true;
        }
            //wait for the finish of other threads
        __sync_fetch_and_add(&(index->fbl->current_record_index),read_block_length*(calculate_thread-1));
        now_flush_time=i/(index->settings->max_total_buffer_size);
        if(now_flush_time!=prev_flush_time)
        {
            COUNT_OUTPUT_TIME_START
            indexflush(index->fbl, index,&lock_index,&lock_disk,calculate_thread);
            COUNT_OUTPUT_TIME_END
        for (int i = 0; i < index->fbl->number_of_buffers; i++)
            {
                bufferpresize[i]=0;
            }
        }

        prev_flush_time=now_flush_time;
        for ( j = 0; j < (calculate_thread-1); j++)
        {   
            input_data[j].pos        =*pos+index->settings->timeseries_size*sizeof(ts_type)*j*read_block_length;
            input_data[j].ts         =&(ts[index->settings->timeseries_size*j*read_block_length]);
            input_data[j].saxv       =saxv;
            input_data[j].fin_number =read_block_length;
        }
        nodecounter=0;
        pthread_barrier_wait(&lock_barrier2);
        for (j = 0; j < (calculate_thread-1); j++)
        {
            pthread_join(threadid[j],NULL);
        }
    }
    *pos = ftell(ifile);    
    COUNT_INPUT_TIME_START
    fread(ts1, sizeof(ts_type), index->settings->timeseries_size*(ts_num%(read_block_length*(calculate_thread-1))), ifile);
    COUNT_INPUT_TIME_END
    ts2   =ts;
    ts    =ts1;
    ts1   =ts2;
    


    //handle the rest data
    int conter_ts_number=ts_num%(read_block_length*(calculate_thread-1));
    sax_save_number=conter_ts_number;

    for ( j = 0; j < (calculate_thread-1); j++)
    {   
        input_data[j].pos = *pos+index->settings->timeseries_size*sizeof(ts_type)*j*read_block_length;
        conter++;
        input_data[j].ts=&(ts[index->settings->timeseries_size*j*read_block_length]);
        input_data[j].fin_number=min(conter_ts_number,read_block_length);
        input_data[j].saxv=saxv;
        conter_ts_number=conter_ts_number-read_block_length;
            
        if(conter_ts_number<0)
            break;
    }

    pthread_barrier_init(&lock_barrier1, NULL, conter+1);
    pthread_barrier_init(&lock_barrier2, NULL, conter+1);
    pthread_barrier_init(&lock_barrier3, NULL, conter);
    for ( j = 0; j < conter; j++)
    { 
        input_data[j].finished=false;
        pthread_create(&(threadid[j]),NULL,indexbulkloadingworker_new,(void*)&(input_data[j]));
    }

    if (sax_fist_time_check)
    {
        pthread_mutex_lock(&lock_disk);
        COUNT_OUTPUT_TIME_START
        fwrite(saxv1, index->settings->sax_byte_size, read_block_length*(calculate_thread-1), index->sax_file);
        COUNT_OUTPUT_TIME_END
        pthread_mutex_unlock(&lock_disk);
    }
    else
    {
        sax_fist_time_check=true;
    }

    pthread_barrier_wait(&lock_barrier1);
    saxv2 =saxv;
    saxv  =saxv1;
    saxv1 =saxv2;
    for ( j = 0; j < conter; j++)
    { 
        input_data[j].finished=true;
    }
    nodecounter=0;
    pthread_barrier_wait(&lock_barrier2);

    for (j = 0; j < conter; j++)
    {
        pthread_join(threadid[j],NULL);
    }
    pthread_mutex_lock(&lock_disk);
    COUNT_OUTPUT_TIME_START
    fwrite(saxv1, index->settings->sax_byte_size, sax_save_number, index->sax_file);
    COUNT_OUTPUT_TIME_END
    pthread_mutex_unlock(&lock_disk);
    __sync_fetch_and_add(&(index->fbl->current_record_index),sax_save_number);
    COUNT_OUTPUT_TIME_START
    indexflush(index->fbl, index,&lock_index,&lock_disk,calculate_thread);
    COUNT_OUTPUT_TIME_END
        for (int i = 0; i < index->fbl->number_of_buffers; i++)
    {
        bufferpresize[i]=0;
    }
    free(ts);
    free(ts1);
    free(input_data);
    free(lockcbl);
    free(saxv);
    free(saxv1);
    //free(sax);
    free(pos);
    pthread_barrier_destroy(&lock_barrier1);
    pthread_barrier_destroy(&lock_barrier2);
    pthread_barrier_destroy(&lock_barrier3);
    COUNT_INPUT_TIME_START
    fclose(ifile);
    COUNT_INPUT_TIME_END
}

void isax_index_binary_file_pRecBuf(const char *ifilename, int ts_num, isax_index *index,int calculate_thread)
{
    fprintf(stderr, ">>> Indexing: %s\n", ifilename);
    FILE * ifile;
    
    ifile = fopen (ifilename,"rb");
    index_buffer_data *input_data=malloc(sizeof(index_buffer_data)*(calculate_thread-1));
    file_position_type *pos = malloc(sizeof(file_position_type));
    pthread_t threadid[calculate_thread-1];
    bool sax_fist_time_check = false;
    long int ts_loaded = 0;
    int j,conter=0;
    long long int i;
    int prev_flush_time=0,now_flush_time=0;
    int sax_save_number;
    //initial the locks
    pthread_mutex_t lock_record=PTHREAD_MUTEX_INITIALIZER,lockfbl=PTHREAD_MUTEX_INITIALIZER,lock_index=PTHREAD_MUTEX_INITIALIZER,
                    lock_firstnode=PTHREAD_MUTEX_INITIALIZER,lock_disk=PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_t *lockcbl;
    pthread_barrier_t lock_barrier1, lock_barrier2;
    pthread_barrier_init(&lock_barrier1, NULL, calculate_thread);
    pthread_barrier_init(&lock_barrier2, NULL, calculate_thread);
    destroy_fbl(index->fbl);
        index->fbl = (first_buffer_layer*)initialize_pRecBuf(index->settings->initial_fbl_buffer_size,
                                pow(2, index->settings->paa_segments), 
                                index->settings->max_total_buffer_size+DISK_BUFFER_SIZE*(PROGRESS_CALCULATE_THREAD_NUMBER-1), index);
    // set the thread on decided cpu

    lockcbl= malloc(sizeof(pthread_mutex_t)*LOCK_SIZE);

    for(i=0; i<LOCK_SIZE ;i++)
    {
        pthread_mutex_init(&lockcbl[i], NULL);
    }

    for (i = 0; i < (calculate_thread-1); i++)
    {
        input_data[i].index=index;
        input_data[i].lock_fbl=&lockfbl;
        input_data[i].lock_record=&lock_record;
        input_data[i].lock_cbl=lockcbl;
        input_data[i].lock_firstnode =&lock_firstnode;
        input_data[i].lock_index=&lock_index;
        input_data[i].blocid=i*read_block_length;
        input_data[i].lock_disk=&lock_disk;
        input_data[i].workernumber=i;
        input_data[i].total_workernumber=calculate_thread-1;
        input_data[i].lock_barrier1=&lock_barrier1;
        input_data[i].lock_barrier2=&lock_barrier2;
        input_data[i].finished=false;
    }
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


    ts_type * ts     = malloc(sizeof(ts_type) * index->settings->timeseries_size*read_block_length*(calculate_thread-1));
    ts_type * ts1    = malloc(sizeof(ts_type) * index->settings->timeseries_size*read_block_length*(calculate_thread-1));
    ts_type * ts2;
    sax_type * saxv  = malloc(sizeof(sax_type) * index->settings->paa_segments*read_block_length*(calculate_thread-1));
    sax_type * saxv1 = malloc(sizeof(sax_type) * index->settings->paa_segments*read_block_length*(calculate_thread-1));
    sax_type * saxv2;
    
    index->settings->raw_filename = malloc(256);
    strcpy(index->settings->raw_filename, ifilename);

    #ifdef BENCHMARK    
    int percentage = (int) (ts_num / (file_position_type) 100);
    #endif
        
    *pos = ftell(ifile);


    if(ts_num>read_block_length*(calculate_thread-1))
    {
        fread(ts1, sizeof(ts_type), index->settings->timeseries_size*read_block_length*(calculate_thread-1), ifile);
        ts2   =ts;
        ts    =ts1;  
        ts1   =ts2;

        for ( j = 0; j < (calculate_thread-1); j++)
        {   
            input_data[j].pos        =*pos+index->settings->timeseries_size*sizeof(ts_type)*j*read_block_length;
            input_data[j].ts         =&(ts[index->settings->timeseries_size*j*read_block_length]);
            input_data[j].saxv       =saxv;
            input_data[j].fin_number =read_block_length;
        }
        for (j = 0; j < (calculate_thread-1); j++)
        {
            pthread_create(&(threadid[j]),NULL,indexbulkloadingworker_pRecBuf,(void*)&(input_data[j]));
        }

        for (i = read_block_length*(calculate_thread-1); i < ts_num; i+=read_block_length*(calculate_thread-1))
        {
            
            *pos = ftell(ifile);
            
            //read the data of next round
            fread(ts1, sizeof(ts_type), index->settings->timeseries_size*read_block_length*(calculate_thread-1), ifile);
            
            //write the sax in disk (last round)
            if (sax_fist_time_check)
            {
                //printf("the sax0 is %d\n",saxv[0] );
                COUNT_OUTPUT_TIME_START
                fwrite(saxv1, index->settings->sax_byte_size, read_block_length*(calculate_thread-1), index->sax_file);
                COUNT_OUTPUT_TIME_END
            }
            else
            { 
                sax_fist_time_check=true;
            }

            pthread_barrier_wait(&lock_barrier1);
            //wait for the finish of other threads
            ts2   =ts;
            ts    =ts1;  
            ts1   =ts2;

            now_flush_time=i/(index->settings->max_total_buffer_size);
                __sync_fetch_and_add(&(index->total_records),read_block_length*(calculate_thread-1));

            __sync_fetch_and_add(&(index->fbl->current_record_index),read_block_length*(calculate_thread-1));
            if(now_flush_time!=prev_flush_time)
            {
                COUNT_QUEUE_TIME_START
                indexconstruction_pRecBuf(index->fbl, index,&lock_index,calculate_thread);
                COUNT_QUEUE_TIME_END
            }
            //printf("the saxv2 in the worker is %d\n",saxv2[0] );
            saxv2 =saxv;
            saxv  =saxv1;
            saxv1 =saxv2;

            prev_flush_time=now_flush_time;
            for ( j = 0; j < (calculate_thread-1); j++)
            {   
                input_data[j].pos        =*pos+index->settings->timeseries_size*sizeof(ts_type)*j*read_block_length;
                input_data[j].ts         =&(ts[index->settings->timeseries_size*j*read_block_length]);
                input_data[j].saxv       =saxv;
                input_data[j].fin_number =read_block_length;
            }
            pthread_barrier_wait(&lock_barrier2);
        }

        pthread_barrier_wait(&lock_barrier1);
        for (j = 0; j < (calculate_thread-1); j++)
        {
            input_data[j].finished=true;
        }
            //wait for the finish of other threads
        __sync_fetch_and_add(&(index->total_records),read_block_length*(calculate_thread-1));
        __sync_fetch_and_add(&(index->fbl->current_record_index),read_block_length*(calculate_thread-1));
        now_flush_time=i/(index->settings->max_total_buffer_size);
        if(now_flush_time!=prev_flush_time)
        {
            COUNT_QUEUE_TIME_START
            indexconstruction_pRecBuf(index->fbl, index,&lock_index,calculate_thread);
            COUNT_QUEUE_TIME_END
        }

        prev_flush_time=now_flush_time;
        for ( j = 0; j < (calculate_thread-1); j++)
        {   
            input_data[j].pos        =*pos+index->settings->timeseries_size*sizeof(ts_type)*j*read_block_length;
            input_data[j].ts         =&(ts[index->settings->timeseries_size*j*read_block_length]);
            input_data[j].saxv       =saxv;
            input_data[j].fin_number =read_block_length;
        }
        pthread_barrier_wait(&lock_barrier2);
        for (j = 0; j < (calculate_thread-1); j++)
        {
            pthread_join(threadid[j],NULL);
        }
    }
        
    fread(ts1, sizeof(ts_type), index->settings->timeseries_size*(ts_num%(read_block_length*(calculate_thread-1))), ifile);
    ts2   =ts;
    ts    =ts1;
    ts1   =ts2;

    


    //handle the rest data
    int conter_ts_number=ts_num%(read_block_length*(calculate_thread-1));
    sax_save_number=conter_ts_number;

    for ( j = 0; j < (calculate_thread-1); j++)
    {   
        input_data[j].pos = *pos+index->settings->timeseries_size*sizeof(ts_type)*j*read_block_length;
        conter++;
        input_data[j].ts=&(ts[index->settings->timeseries_size*j*read_block_length]);
        input_data[j].fin_number=min(conter_ts_number,read_block_length);
        conter_ts_number=conter_ts_number-read_block_length;
            
        if(conter_ts_number<0)
            break;
    }

    pthread_barrier_init(&lock_barrier1, NULL, conter+1);
    pthread_barrier_init(&lock_barrier2, NULL, conter+1);
    for ( j = 0; j < conter; j++)
    { 
        input_data[j].finished=false;
        pthread_create(&(threadid[j]),NULL,indexbulkloadingworker_pRecBuf,(void*)&(input_data[j]));
    }

    if (sax_fist_time_check)
    {
        pthread_mutex_lock(&lock_disk);
        COUNT_OUTPUT_TIME_START
        fwrite(saxv1, index->settings->sax_byte_size, read_block_length*(calculate_thread-1), index->sax_file);
        COUNT_OUTPUT_TIME_END
        pthread_mutex_unlock(&lock_disk);
    }
    else
    {
        sax_fist_time_check=true;
    }
    pthread_barrier_wait(&lock_barrier1);
    saxv2 =saxv;
    saxv  =saxv1;
    saxv1 =saxv2;
    for ( j = 0; j < conter; j++)
    { 
        input_data[j].finished=true;
    }
    __sync_fetch_and_add(&(index->total_records),conter_ts_number);
    __sync_fetch_and_add(&(index->fbl->current_record_index),conter_ts_number);

    pthread_barrier_wait(&lock_barrier2);

    for (j = 0; j < conter; j++)
    {
        pthread_join(threadid[j],NULL);
    }
    pthread_mutex_lock(&lock_disk);
    COUNT_OUTPUT_TIME_START
    fwrite(saxv, index->settings->sax_byte_size, sax_save_number, index->sax_file);
    COUNT_OUTPUT_TIME_END
    pthread_mutex_unlock(&lock_disk);
    
    indexconstruction_pRecBuf(index->fbl, index,&lock_index,calculate_thread);
    
    free(ts);
    free(ts1);
    free(input_data);
    free(lockcbl);
    free(saxv);
    free(saxv1);
    //free(sax);
    free(pos);
    pthread_barrier_destroy(&lock_barrier1);
    pthread_barrier_destroy(&lock_barrier2);
    COUNT_INPUT2_TIME_START
    fclose(ifile);
    COUNT_INPUT2_TIME_END
}
void isax_index_binary_file_2RecBuf(const char *ifilename, int ts_num, isax_index *index,int calculate_thread)
{
    fprintf(stderr, ">>> Indexing: %s\n", ifilename);
    FILE * ifile;
    int propotion_number = 0;
    ifile = fopen (ifilename,"rb");
    index_buffer_data *input_data=malloc(sizeof(index_buffer_data)*(calculate_thread/2+propotion_number));
    file_position_type *pos = malloc(sizeof(file_position_type));
    pthread_t threadid[calculate_thread/2+propotion_number];
    bool sax_fist_time_check = false;
    
    long int ts_loaded = 0;
    int j,conter=0;
    long long int i;
    int prev_flush_time=0,now_flush_time=0;
    int sax_save_number;
    //initial the locks
    pthread_mutex_t lock_record=PTHREAD_MUTEX_INITIALIZER,lockfbl=PTHREAD_MUTEX_INITIALIZER,lock_index=PTHREAD_MUTEX_INITIALIZER,
                    lock_firstnode=PTHREAD_MUTEX_INITIALIZER,lock_disk=PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_t *lockcbl;
    lockcbl= malloc(sizeof(pthread_mutex_t)*LOCK_SIZE);
    pthread_barrier_t lock_barrier1, lock_barrier2, lock_barrier3, lock_barrier4;
    pthread_barrier_init(&lock_barrier1, NULL, calculate_thread/2+propotion_number+1);
    pthread_barrier_init(&lock_barrier2, NULL, calculate_thread/2+propotion_number+1);
    pthread_barrier_init(&lock_barrier3, NULL, calculate_thread/2-propotion_number);
    pthread_barrier_init(&lock_barrier4, NULL, calculate_thread/2-propotion_number);
    destroy_fbl(index->fbl);
        index->fbl = (first_buffer_layer*)initialize_pRecBuf(index->settings->initial_fbl_buffer_size,
                                pow(2, index->settings->paa_segments), 
                                index->settings->max_total_buffer_size+DISK_BUFFER_SIZE*(PROGRESS_CALCULATE_THREAD_NUMBER-1), index);
    // set the thread on decided cpu
    int indexconstructionroundchose=0;
    bool firstflush=true;
    for(i=0; i<LOCK_SIZE ;i++)
    {
        pthread_mutex_init(&lockcbl[i], NULL);
    }
    int bufferpresize[index->fbl->number_of_buffers*2];

    for (int i = 0; i < index->fbl->number_of_buffers*2; i++)
    {
        bufferpresize[i]=0;
    }

    trans_fbl_input constructioninput_data;
    pthread_t threadconstructionid[calculate_thread/2-propotion_number-1];
    pthread_mutex_t lock_fbl_conter=PTHREAD_MUTEX_INITIALIZER;
    constructioninput_data.index=index;
    constructioninput_data.lock_index=&lock_index;
    constructioninput_data.lock_fbl_conter=&lock_fbl_conter;
    constructioninput_data.conternumber=0;
    constructioninput_data.stop_number=index->fbl->number_of_buffers;
    constructioninput_data.preworkernumber=2;
    constructioninput_data.fbloffset=indexconstructionroundchose;
    constructioninput_data.lock_barrier1=&lock_barrier3;
    constructioninput_data.lock_barrier2=&lock_barrier4;
    constructioninput_data.buffersize=bufferpresize;
    constructioninput_data.finished=false;


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


    ts_type * ts     = malloc(sizeof(ts_type) * index->settings->timeseries_size*read_block_length*(calculate_thread/2+propotion_number));
    ts_type * ts1    = malloc(sizeof(ts_type) * index->settings->timeseries_size*read_block_length*(calculate_thread/2+propotion_number));
    ts_type * ts2;
    sax_type * saxv  = malloc(sizeof(sax_type) * index->settings->paa_segments*read_block_length*(calculate_thread/2+propotion_number));
    sax_type * saxv1 = malloc(sizeof(sax_type) * index->settings->paa_segments*read_block_length*(calculate_thread/2+propotion_number));
    sax_type * saxv2;
    
    for (i = 0; i < (calculate_thread/2+propotion_number); i++)
    {
        input_data[i].index=index;
        input_data[i].lock_fbl=&lockfbl;
        input_data[i].lock_cbl=lockcbl;
        input_data[i].lock_record=&lock_record;
        input_data[i].bufferpresize=bufferpresize;
        input_data[i].lock_firstnode =&lock_firstnode;
        input_data[i].lock_index=&lock_index;
        input_data[i].blocid=i*read_block_length;
        input_data[i].lock_disk=&lock_disk;
        input_data[i].workernumber=0;
        input_data[i].total_workernumber=2;
        input_data[i].lock_barrier1=&lock_barrier1;
        input_data[i].lock_barrier2=&lock_barrier2;
        input_data[i].saxv =saxv;
        input_data[i].finished=false;
    }
    index->settings->raw_filename = malloc(256);
    strcpy(index->settings->raw_filename, ifilename);

    #ifdef BENCHMARK    
    int percentage = (int) (ts_num / (file_position_type) 100);
    #endif

    *pos = ftell(ifile);

    if(ts_num>read_block_length*(calculate_thread/2+propotion_number))
    {
        fread(ts1, sizeof(ts_type), index->settings->timeseries_size*read_block_length*(calculate_thread/2+propotion_number), ifile);
        ts2   =ts;
        ts    =ts1;  
        ts1   =ts2;

        for ( j = 0; j < (calculate_thread/2+propotion_number); j++)
        {   
            input_data[j].pos        =*pos+index->settings->timeseries_size*sizeof(ts_type)*j*read_block_length;
            input_data[j].ts         =&(ts[index->settings->timeseries_size*j*read_block_length]);
            input_data[j].saxv       =saxv;
            input_data[j].fin_number =read_block_length;
        }
        for (j = 0; j < (calculate_thread/2+propotion_number); j++)
        {
            pthread_create(&(threadid[j]),NULL,indexbulkloadingworker_2RecBuf,(void*)&(input_data[j]));
        }
        for (int k = 0; k < calculate_thread/2-propotion_number-1; k++)
        {
            pthread_create(&(threadconstructionid[k]),NULL,indexconstructionworker_2RecBuf,(void*)&(constructioninput_data));
        }

        for (i = read_block_length*(calculate_thread/2+propotion_number); i < ts_num; i+=read_block_length*(calculate_thread/2+propotion_number))
        {   
                    

            *pos = ftell(ifile);
            //read the data of next round
            fread(ts1, sizeof(ts_type), index->settings->timeseries_size*read_block_length*(calculate_thread/2+propotion_number), ifile);
            //write the sax in disk (last round)
            if (sax_fist_time_check)
            {
                //printf("the sax0 is %d\n",saxv[0] );
                COUNT_OUTPUT_TIME_START
                fwrite(saxv1, index->settings->sax_byte_size, read_block_length*(calculate_thread/2+propotion_number), index->sax_file);
                COUNT_OUTPUT_TIME_END
            }
            else
            { 
                sax_fist_time_check=true;
            }
            
            pthread_barrier_wait(&lock_barrier1);
            pthread_barrier_wait(&lock_barrier3);
            //wait for the finish of other threads
            ts2   =ts;
            ts    =ts1;
            ts1   =ts2;
            saxv2 =saxv;
            saxv  =saxv1;
            saxv1 =saxv2;
            now_flush_time=i/(index->settings->max_total_buffer_size);
            __sync_fetch_and_add(&(index->total_records),read_block_length*(calculate_thread/2+propotion_number));
            __sync_fetch_and_add(&(index->fbl->current_record_index),read_block_length*(calculate_thread/2+propotion_number));

            for ( j = 0; j < calculate_thread/2+propotion_number; j++)
            {   
                input_data[j].pos        =*pos+index->settings->timeseries_size*sizeof(ts_type)*j*read_block_length;
                input_data[j].ts         =&(ts[index->settings->timeseries_size*j*read_block_length]);
                input_data[j].saxv       =saxv;
                input_data[j].fin_number =read_block_length;
                if(input_data[j].workernumber==0)
                    input_data[j].workernumber=1;
                else
                    input_data[j].workernumber=0;
            }
            if(constructioninput_data.fbloffset==0)
                constructioninput_data.fbloffset=1;
            else
                constructioninput_data.fbloffset=0;

            constructioninput_data.conternumber=0;
            pthread_barrier_wait(&lock_barrier2);
            pthread_barrier_wait(&lock_barrier4);
        }

        *pos = ftell(ifile);
        fread(ts1, sizeof(ts_type), index->settings->timeseries_size*(ts_num%(read_block_length*(calculate_thread/2+propotion_number))), ifile);
        if (sax_fist_time_check)
        {
            //printf("the sax0 is %d\n",saxv[0] );
            COUNT_OUTPUT_TIME_START
            fwrite(saxv1, index->settings->sax_byte_size, read_block_length*(calculate_thread/2+propotion_number), index->sax_file);
            COUNT_OUTPUT_TIME_END
        }
        else
        { 
            sax_fist_time_check=true;
        }
        pthread_barrier_wait(&lock_barrier1);
        pthread_barrier_wait(&lock_barrier3);
        ts2   =ts;
        ts    =ts1;
        ts1   =ts2;
        saxv2 =saxv;
        saxv  =saxv1;
        saxv1 =saxv2;
        int conter_ts_number=ts_num%(read_block_length*(calculate_thread/2+propotion_number));
        sax_save_number=conter_ts_number;
        int threadnumber=0;
        pthread_barrier_t lock_barrier1_bas,lock_barrier2_bas;


        for ( j = 0; j < calculate_thread/2+propotion_number; j++)
        {   
            input_data[j].pos = *pos+index->settings->timeseries_size*sizeof(ts_type)*j*read_block_length;
            conter++;
            input_data[j].ts=&(ts[index->settings->timeseries_size*j*read_block_length]);
            input_data[j].fin_number=min(conter_ts_number,read_block_length);
            conter_ts_number=conter_ts_number-read_block_length;

            if(input_data[j].workernumber==0)
                input_data[j].workernumber=1;
            else
                input_data[j].workernumber=0;
            if((conter_ts_number+read_block_length)<=0)
                input_data[j].finished=true;
            else
                threadnumber=j+2;
        }
        pthread_barrier_init(&lock_barrier1_bas, NULL, threadnumber);
        pthread_barrier_init(&lock_barrier2_bas, NULL, threadnumber);
        for ( j = 0; j < calculate_thread/2+propotion_number; j++)
        {  
            input_data[j].lock_barrier1=&lock_barrier1_bas;
            input_data[j].lock_barrier2=&lock_barrier2_bas;
        }

        if(constructioninput_data.fbloffset==0)
            constructioninput_data.fbloffset=1;
        else
            constructioninput_data.fbloffset=0;
        constructioninput_data.conternumber=0;
        
        pthread_barrier_wait(&lock_barrier2);
        pthread_barrier_wait(&lock_barrier4);
        
        if (sax_fist_time_check)
        {
            //printf("the sax0 is %d\n",saxv[0] );
            COUNT_OUTPUT_TIME_START
            fwrite(saxv, index->settings->sax_byte_size, ts_num%read_block_length*(calculate_thread/2+propotion_number), index->sax_file);
            COUNT_OUTPUT_TIME_END
        }
        else
        { 
            sax_fist_time_check=true;
        }
        pthread_barrier_wait(&lock_barrier1_bas);
        pthread_barrier_wait(&lock_barrier3);
        for ( j = 0; j < calculate_thread/2+propotion_number; j++)
        {
        input_data[j].finished=true;
        }
        if(input_data[0].workernumber==0)
        constructioninput_data.fbloffset=1;
        else
        constructioninput_data.fbloffset=0;
        constructioninput_data.conternumber=0;
        constructioninput_data.finished=1;
        pthread_barrier_wait(&lock_barrier2_bas);
        pthread_barrier_wait(&lock_barrier4);


            for (int k = 0; k < calculate_thread/2+propotion_number; k++)
            {
                pthread_join(threadid[k],NULL);
            } 
                        for (int k = 0; k < calculate_thread/2-propotion_number-1; k++)
            {
                pthread_join(threadconstructionid[k],NULL);
            } 

    }

    /*
    {



                
        constructioninput_data.finished=1;




        fread(ts1, sizeof(ts_type), index->settings->timeseries_size*(ts_num%(read_block_length*(calculate_thread/2-1))), ifile);
        ts2   =ts;
        ts    =ts1;
        ts1   =ts2;
        //handle the rest data
        int conter_ts_number=ts_num%(read_block_length*(calculate_thread/2-1));
        sax_save_number=conter_ts_number;
        for ( j = 0; j < (calculate_thread/2-1); j++)
        {   
            input_data[j].pos = *pos+index->settings->timeseries_size*sizeof(ts_type)*j*read_block_length;
            conter++;
            input_data[j].ts=&(ts[index->settings->timeseries_size*j*read_block_length]);
            input_data[j].fin_number=min(conter_ts_number,read_block_length);
            conter_ts_number=conter_ts_number-read_block_length;
            
            if(conter_ts_number<0)
            break;
        }

        pthread_barrier_destroy(&lock_barrier1);
        pthread_barrier_destroy(&lock_barrier2);

        pthread_barrier_init(&lock_barrier1, NULL, conter+1);
        pthread_barrier_init(&lock_barrier2, NULL, conter+1);

        for (j = 0; j < (calculate_thread/2+propotion_number); j++)
        {
            pthread_create(&(threadid[j]),NULL,indexbulkloadingworker_pRecBuf,(void*)&(input_data[j]));
        }
        
        pthread_barrier_wait(&lock_barrier1);
        saxv2 =saxv;
        saxv  =saxv1;
        saxv1 =saxv2;
        for ( j = 0; j < conter; j++)
        { 
            input_data[j].finished=true;
        }
        __sync_fetch_and_add(&(index->total_records),conter_ts_number);
        __sync_fetch_and_add(&(index->fbl->current_record_index),conter_ts_number);
        pthread_barrier_wait(&lock_barrier2);
        for (j = 0; j < conter; j++)
        {
            pthread_join(threadid[j],NULL);
        }
        pthread_mutex_lock(&lock_disk);
        COUNT_OUTPUT_TIME_START
        fwrite(saxv, index->settings->sax_byte_size, sax_save_number, index->sax_file);
        COUNT_OUTPUT_TIME_END
        pthread_mutex_unlock(&lock_disk);

        constructioninput_data.stop_number=index->fbl->number_of_buffers;
        constructioninput_data.fbloffset=0;
        constructioninput_data.preworkernumber=calculate_thread-2;
        constructioninput_data.conternumber=0;
        for (int k = 0; k < calculate_thread/2; k++)
        {
            pthread_create(&(threadconstructionid[k]),NULL,indexconstructionworker_pRecBuf_new,(void*)&(constructioninput_data));
        }  

        for (int k = 0; k < calculate_thread/2; k++)
        {
            pthread_join(threadconstructionid[k],NULL);
        } 
    }*/


    //indexflush(index->fbl, index,&lock_index,&lock_disk,calculate_thread);
    printf("this is the check point 7\n");
    free(ts);
    free(ts1);
    free(input_data);
    free(saxv);
    free(saxv1);
    //free(sax);
    free(pos);
    pthread_barrier_destroy(&lock_barrier1);
    pthread_barrier_destroy(&lock_barrier2);
    pthread_barrier_destroy(&lock_barrier3);
    pthread_barrier_destroy(&lock_barrier4);
    COUNT_INPUT2_TIME_START
    fclose(ifile);
    COUNT_INPUT2_TIME_END
}
    
void isax_index_binary_file_2nRecBuf(const char *ifilename, int ts_num, isax_index *index,int calculate_thread)
{
    fprintf(stderr, ">>> Indexing: %s\n", ifilename);
    FILE * ifile;
    int propotion_number = 0;
    ifile = fopen (ifilename,"rb");
    index_buffer_data *input_data=malloc(sizeof(index_buffer_data)*(calculate_thread/2+propotion_number));
    file_position_type *pos = malloc(sizeof(file_position_type));
    pthread_t threadid[calculate_thread/2+propotion_number];
    bool sax_fist_time_check = false;
    
    long int ts_loaded = 0;
    int j,conter=0;
    long long int i;
    int prev_flush_time=0,now_flush_time=0;
    int sax_save_number;
    //initial the locks
    pthread_mutex_t lock_record=PTHREAD_MUTEX_INITIALIZER,lockfbl=PTHREAD_MUTEX_INITIALIZER,lock_index=PTHREAD_MUTEX_INITIALIZER,
                    lock_firstnode=PTHREAD_MUTEX_INITIALIZER,lock_disk=PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_t *lockcbl;
    lockcbl= malloc(sizeof(pthread_mutex_t)*LOCK_SIZE);

    pthread_barrier_t lock_barrier1, lock_barrier2, lock_barrier3, lock_barrier4;
    pthread_barrier_init(&lock_barrier1, NULL, calculate_thread);
    pthread_barrier_init(&lock_barrier2, NULL, calculate_thread);
    pthread_barrier_init(&lock_barrier3, NULL, calculate_thread/2-propotion_number);
    pthread_barrier_init(&lock_barrier4, NULL, calculate_thread/2-propotion_number);
    destroy_fbl(index->fbl);
    index->fbl = (first_buffer_layer*)initialize_2pRecBuf(index->settings->initial_fbl_buffer_size,
                                pow(2, index->settings->paa_segments), 
                                index->settings->max_total_buffer_size+DISK_BUFFER_SIZE*(PROGRESS_CALCULATE_THREAD_NUMBER-1), index);
    first_buffer_layer *fbltran;
    // set the thread on decided cpu
    int indexbulkloadingroundchose=0;
    bool firstflush=true;
    for(i=0; i<LOCK_SIZE ;i++)
    {
        pthread_mutex_init(&lockcbl[i], NULL);
    }
    int *bufferpresize=malloc(sizeof(int)*((parallel_dfirst_buffer_layer*)index->fbl)->number_of_buffers*(calculate_thread/2+propotion_number)*2);
    for (long int i = 0; i < ((parallel_dfirst_buffer_layer*)index->fbl)->number_of_buffers*(calculate_thread/2+propotion_number)*2; i++)
    {
        bufferpresize[i]=0;
    }

    trans_fbl_input constructioninput_data;
    pthread_t threadconstructionid[calculate_thread/2-propotion_number-1];
    pthread_mutex_t lock_fbl_conter=PTHREAD_MUTEX_INITIALIZER;
    constructioninput_data.index=index;
    constructioninput_data.lock_index=&lock_index;
    constructioninput_data.lock_fbl_conter=&lock_fbl_conter;
    constructioninput_data.conternumber=0;
    constructioninput_data.stop_number=((parallel_dfirst_buffer_layer*)index->fbl)->number_of_buffers;
    constructioninput_data.preworkernumber=calculate_thread/2+propotion_number;
    constructioninput_data.fbloffset=calculate_thread/2+propotion_number;
    constructioninput_data.lock_barrier1=&lock_barrier1;
    constructioninput_data.lock_barrier2=&lock_barrier2;
    constructioninput_data.buffersize=bufferpresize;
    constructioninput_data.finished=false;


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


    ts_type * ts     = malloc(sizeof(ts_type) * index->settings->timeseries_size*read_block_length*(calculate_thread/2+propotion_number));
    ts_type * ts1    = malloc(sizeof(ts_type) * index->settings->timeseries_size*read_block_length*(calculate_thread/2+propotion_number));
    ts_type * ts2;
    sax_type * saxv  = malloc(sizeof(sax_type) * index->settings->paa_segments*read_block_length*(calculate_thread/2+propotion_number));
    sax_type * saxv1 = malloc(sizeof(sax_type) * index->settings->paa_segments*read_block_length*(calculate_thread/2+propotion_number));
    sax_type * saxv2;
    
    for (i = 0; i < (calculate_thread/2+propotion_number); i++)
    {
        input_data[i].index=index;
        input_data[i].lock_fbl=&lockfbl;
        input_data[i].lock_cbl=lockcbl;
        input_data[i].lock_record=&lock_record;
        input_data[i].bufferpresize=bufferpresize;
        input_data[i].lock_firstnode =&lock_firstnode;
        input_data[i].lock_index=&lock_index;
        input_data[i].blocid=i*read_block_length;
        input_data[i].lock_disk=&lock_disk;
        input_data[i].workernumber=i;
        input_data[i].total_workernumber=(calculate_thread/2+propotion_number)*2;
        input_data[i].lock_barrier1=&lock_barrier1;
        input_data[i].lock_barrier2=&lock_barrier2;
        input_data[i].saxv =saxv;
        input_data[i].finished=false;
    }
    index->settings->raw_filename = malloc(256);
    strcpy(index->settings->raw_filename, ifilename);

    #ifdef BENCHMARK    
    int percentage = (int) (ts_num / (file_position_type) 100);
    #endif

    *pos = ftell(ifile);

    if(ts_num>read_block_length*(calculate_thread/2+propotion_number))
    {
        fread(ts1, sizeof(ts_type), index->settings->timeseries_size*read_block_length*(calculate_thread/2+propotion_number), ifile);
        ts2   =ts;
        ts    =ts1;  
        ts1   =ts2;
        for ( j = 0; j < (calculate_thread/2+propotion_number); j++)
        {   
            input_data[j].pos        =*pos+index->settings->timeseries_size*sizeof(ts_type)*j*read_block_length;
            input_data[j].ts         =&(ts[index->settings->timeseries_size*j*read_block_length]);
            input_data[j].saxv       =saxv;
            input_data[j].fin_number =read_block_length;
        }
        for (j = 0; j < (calculate_thread/2+propotion_number); j++)
        {
            pthread_create(&(threadid[j]),NULL,indexbulkloadingworker_2RecBuf,(void*)&(input_data[j]));
        }
        for (int k = 0; k < calculate_thread/2-propotion_number-1; k++)
        {
            pthread_create(&(threadconstructionid[k]),NULL,indexconstructionworker_2nRecBuf,(void*)&(constructioninput_data));
        }

        for (i = read_block_length*(calculate_thread/2+propotion_number); i < ts_num; i+=read_block_length*(calculate_thread/2+propotion_number))
        {   
            *pos = ftell(ifile);
            //read the data of next round
            fread(ts1, sizeof(ts_type), index->settings->timeseries_size*read_block_length*(calculate_thread/2+propotion_number), ifile);
            //write the sax in disk (last round)
            if (sax_fist_time_check)
            {
                //printf("the sax0 is %d\n",saxv[0] );
                COUNT_OUTPUT_TIME_START
                fwrite(saxv1, index->settings->sax_byte_size, read_block_length*(calculate_thread/2+propotion_number), index->sax_file);
                COUNT_OUTPUT_TIME_END
            }
            else
            { 
                sax_fist_time_check=true;
            }
            
            //pthread_barrier_wait(&lock_barrier1);
            //pthread_barrier_wait(&lock_barrier3);
            //wait for the finish of other threads
            ts2   =ts;
            ts    =ts1;
            ts1   =ts2;
            saxv2 =saxv;
            saxv  =saxv1;
            saxv1 =saxv2;
            now_flush_time=i/(index->settings->max_total_buffer_size);
            __sync_fetch_and_add(&(index->total_records),read_block_length*(calculate_thread/2+propotion_number));
            __sync_fetch_and_add(&(index->fbl->current_record_index),read_block_length*(calculate_thread/2+propotion_number));
            if(indexbulkloadingroundchose==0)
            indexbulkloadingroundchose=calculate_thread/2+propotion_number;
            else
            indexbulkloadingroundchose=0;
            
            for ( j = 0; j < calculate_thread/2+propotion_number; j++)
            {   
                input_data[j].pos        =*pos+index->settings->timeseries_size*sizeof(ts_type)*j*read_block_length;
                input_data[j].ts         =&(ts[index->settings->timeseries_size*j*read_block_length]);
                input_data[j].saxv       =saxv;
                input_data[j].fin_number =read_block_length;
                input_data[j].workernumber=j+indexbulkloadingroundchose;

            }
            if(constructioninput_data.fbloffset==0)
                constructioninput_data.fbloffset=calculate_thread/2+propotion_number;
            else
                constructioninput_data.fbloffset=0;

            constructioninput_data.conternumber=0;
            pthread_barrier_wait(&lock_barrier2);
           // pthread_barrier_wait(&lock_barrier4);
        }

        *pos = ftell(ifile);
        fread(ts1, sizeof(ts_type), index->settings->timeseries_size*(ts_num%(read_block_length*(calculate_thread/2+propotion_number))), ifile);
        if (sax_fist_time_check)
        {
            //printf("the sax0 is %d\n",saxv[0] );
            COUNT_OUTPUT_TIME_START
            fwrite(saxv1, index->settings->sax_byte_size, read_block_length*(calculate_thread/2+propotion_number), index->sax_file);
            COUNT_OUTPUT_TIME_END
        }
        else
        { 
            sax_fist_time_check=true;
        }
        //pthread_barrier_wait(&lock_barrier1);
       // pthread_barrier_wait(&lock_barrier3);
        ts2   =ts;
        ts    =ts1;
        ts1   =ts2;
        saxv2 =saxv;
        saxv  =saxv1;
        saxv1 =saxv2;
        int conter_ts_number=ts_num%(read_block_length*(calculate_thread/2+propotion_number));
        sax_save_number=conter_ts_number;
        int threadnumber=0;
        pthread_barrier_t lock_barrier1_bas,lock_barrier2_bas;


        for ( j = 0; j < calculate_thread/2+propotion_number; j++)
        {   
            input_data[j].pos = *pos+index->settings->timeseries_size*sizeof(ts_type)*j*read_block_length;
            //conter++;
            input_data[j].ts=&(ts[index->settings->timeseries_size*j*read_block_length]);
            input_data[j].fin_number=min(conter_ts_number,read_block_length);
            conter_ts_number=conter_ts_number-read_block_length;

            input_data[j].workernumber=j+indexbulkloadingroundchose;

            //if((conter_ts_number+read_block_length)<=0)
                input_data[j].finished=true;
           // else
              //  threadnumber=j+2;
        }
        //pthread_barrier_init(&lock_barrier1_bas, NULL, threadnumber);
        //pthread_barrier_init(&lock_barrier2_bas, NULL, threadnumber);
        for ( j = 0; j < calculate_thread/2+propotion_number; j++)
        {  
           // input_data[j].lock_barrier1=&lock_barrier1_bas;
            //input_data[j].lock_barrier2=&lock_barrier2_bas;
        }

        if(constructioninput_data.fbloffset==0)
            constructioninput_data.fbloffset=calculate_thread/2+propotion_number;
        else
            constructioninput_data.fbloffset=0;
        constructioninput_data.conternumber=0;
        constructioninput_data.finished=1;
        pthread_barrier_wait(&lock_barrier2);
        //pthread_barrier_wait(&lock_barrier4);
        
        if (sax_fist_time_check)
        {
            //printf("the sax0 is %d\n",saxv[0] );
            COUNT_OUTPUT_TIME_START
            fwrite(saxv, index->settings->sax_byte_size, ts_num%read_block_length*(calculate_thread/2+propotion_number), index->sax_file);
            COUNT_OUTPUT_TIME_END
        }
        else
        { 
            sax_fist_time_check=true;
        }
        //pthread_barrier_wait(&lock_barrier1_bas);
        //pthread_barrier_wait(&lock_barrier3);
        for ( j = 0; j < calculate_thread/2+propotion_number; j++)
        {
        //input_data[j].finished=true;
        }
        if(input_data[0].workernumber==0)
        constructioninput_data.fbloffset=calculate_thread/2+propotion_number;
        else
        constructioninput_data.fbloffset=0;
        constructioninput_data.conternumber=0;
        constructioninput_data.finished=1;

        //pthread_barrier_wait(&lock_barrier2_bas);
        //pthread_barrier_wait(&lock_barrier4);


            for (int k = 0; k < calculate_thread/2+propotion_number; k++)
            {
                pthread_join(threadid[k],NULL);
            } 
            for (int k = 0; k < calculate_thread/2-propotion_number-1; k++)
            {
                pthread_join(threadconstructionid[k],NULL);
            } 

    }

    //indexpRecBufflush(index->fbl, index,&lock_index,&lock_disk,calculate_thread,calculate_thread/2+propotion_number);
    free(ts);
    free(ts1);
    free(input_data);
    free(saxv);
    free(saxv1);
    //free(sax);
    free(pos);
    pthread_barrier_destroy(&lock_barrier1);
    pthread_barrier_destroy(&lock_barrier2);
    pthread_barrier_destroy(&lock_barrier3);
    pthread_barrier_destroy(&lock_barrier4);
    free(bufferpresize);
    COUNT_INPUT2_TIME_START
    fclose(ifile);
    COUNT_INPUT2_TIME_END
}    
    

void* indexbulkloadingworker(void *transferdata)
{
    sax_type * sax = malloc(sizeof(sax_type) * ((index_buffer_data*)transferdata)->index->settings->paa_segments);

    int fin_number=((index_buffer_data*)transferdata)->fin_number;
    file_position_type *pos = malloc(sizeof(file_position_type));
    sax_type * saxv;
    int offset_saxv=((index_buffer_data*)transferdata)->blocid*((index_buffer_data*)transferdata)->index->settings->paa_segments;
    int paa_segments=((index_buffer_data*)transferdata)->index->settings->paa_segments;
    isax_index *index= ((index_buffer_data*)transferdata)->index;
    int i=0;
    pthread_barrier_t *lock_barrier1, *lock_barrier2;
    
    while(!((index_buffer_data*)transferdata)->finished)
    {   
        saxv=(((index_buffer_data*)transferdata)->saxv);
        lock_barrier1=((index_buffer_data*)transferdata)->lock_barrier1;
        lock_barrier2=((index_buffer_data*)transferdata)->lock_barrier2;
        for (i=0;i<fin_number;i++)
        {
            if(sax_from_ts(((ts_type*)(((index_buffer_data*)transferdata)->ts)+i*index->settings->timeseries_size), sax, index->settings->ts_values_per_paa_segment,
                       index->settings->paa_segments, index->settings->sax_alphabet_cardinality,
                       index->settings->sax_bit_cardinality) == SUCCESS)
            {
                *pos= ((index_buffer_data*)transferdata)->pos+index->settings->timeseries_size*sizeof(ts_type)*i;
                memcpy(&(saxv[offset_saxv+i*paa_segments]), sax, sizeof(sax_type) * paa_segments);
                isax_fbl_index_insert_m(index, sax, pos, ((index_buffer_data*)transferdata)->lock_record, ((index_buffer_data*)transferdata)->lock_fbl,
                                    ((index_buffer_data*)transferdata)->lock_cbl,((index_buffer_data*)transferdata)->lock_firstnode,((index_buffer_data*)transferdata)->lock_index,((index_buffer_data*)transferdata)->lock_disk);
            }
            else
            {
                fprintf(stderr, "error: cannot insert record in index, since sax representation\
                    failed to be created");
            }

        }
        pthread_barrier_wait(lock_barrier1);
        pthread_barrier_wait(lock_barrier2);
    }
    free(pos);
    free(sax);
}
void* indexbulkloadingworker_new2(void *transferdata)
{
    sax_type * sax = malloc(sizeof(sax_type) * ((index_buffer_data*)transferdata)->index->settings->paa_segments);

    int fin_number=((index_buffer_data*)transferdata)->fin_number;
    file_position_type *pos = malloc(sizeof(file_position_type));
    sax_type * saxv;
    int offset_saxv=((index_buffer_data*)transferdata)->blocid*((index_buffer_data*)transferdata)->index->settings->paa_segments;
    int paa_segments=((index_buffer_data*)transferdata)->index->settings->paa_segments;
    isax_index *index= ((index_buffer_data*)transferdata)->index;
    int i=0;
    pthread_barrier_t *lock_barrier1=((index_buffer_data*)transferdata)->lock_barrier1;
    pthread_barrier_t *lock_barrier2=((index_buffer_data*)transferdata)->lock_barrier2;
    
    while(!((index_buffer_data*)transferdata)->finished)
    {   
        saxv=(((index_buffer_data*)transferdata)->saxv);

        for (i=0;i<fin_number;i++)
        {
            if(sax_from_ts(((ts_type*)(((index_buffer_data*)transferdata)->ts)+i*index->settings->timeseries_size), sax, index->settings->ts_values_per_paa_segment,
                       index->settings->paa_segments, index->settings->sax_alphabet_cardinality,
                       index->settings->sax_bit_cardinality) == SUCCESS)
            {
                *pos= ((index_buffer_data*)transferdata)->pos+index->settings->timeseries_size*sizeof(ts_type)*i;
                memcpy(&(saxv[offset_saxv+i*paa_segments]), sax, sizeof(sax_type) * paa_segments);
                isax_fbl_index_insert_m_new(index, sax, pos, ((index_buffer_data*)transferdata)->lock_record, ((index_buffer_data*)transferdata)->lock_fbl,
                                    ((index_buffer_data*)transferdata)->lock_cbl,((index_buffer_data*)transferdata)->lock_firstnode,((index_buffer_data*)transferdata)->lock_index,((index_buffer_data*)transferdata)->lock_disk);
            }
            else
            {
                fprintf(stderr, "error: cannot insert record in index, since sax representation\
                    failed to be created");
            }

        }
        pthread_barrier_wait(lock_barrier1);
        pthread_barrier_wait(lock_barrier2);
    }
    free(pos);
    free(sax);
}

void* indexbulkloadingworker_new(void *transferdata)
{
    sax_type * sax = malloc(sizeof(sax_type) * ((index_buffer_data*)transferdata)->index->settings->paa_segments);

    int fin_number=((index_buffer_data*)transferdata)->fin_number;
    file_position_type *pos = malloc(sizeof(file_position_type));
    sax_type * saxv;
    int offset_saxv=((index_buffer_data*)transferdata)->blocid*((index_buffer_data*)transferdata)->index->settings->paa_segments;
    int paa_segments=((index_buffer_data*)transferdata)->index->settings->paa_segments;
    isax_index *index= ((index_buffer_data*)transferdata)->index;
    int i=0,j;
    pthread_barrier_t *lock_barrier1=((index_buffer_data*)transferdata)->lock_barrier1;
    pthread_barrier_t *lock_barrier2=((index_buffer_data*)transferdata)->lock_barrier2;
    pthread_barrier_t *lock_barrier3=((index_buffer_data*)transferdata)->lock_barrier3;
    isax_node_record *r = malloc(sizeof(isax_node_record));
    while(!((index_buffer_data*)transferdata)->finished)
    {   
        saxv=(((index_buffer_data*)transferdata)->saxv);

        for (i=0;i<fin_number;i++)
        {
            if(sax_from_ts(((ts_type*)(((index_buffer_data*)transferdata)->ts)+i*index->settings->timeseries_size), sax, index->settings->ts_values_per_paa_segment,
                       index->settings->paa_segments, index->settings->sax_alphabet_cardinality,
                       index->settings->sax_bit_cardinality) == SUCCESS)
            {
                *pos= ((index_buffer_data*)transferdata)->pos+index->settings->timeseries_size*sizeof(ts_type)*i;
                memcpy(&(saxv[offset_saxv+i*paa_segments]), sax, sizeof(sax_type) * paa_segments);
                isax_fbl_index_insert_m(index, sax, pos, ((index_buffer_data*)transferdata)->lock_record, ((index_buffer_data*)transferdata)->lock_fbl,
                                    ((index_buffer_data*)transferdata)->lock_cbl,((index_buffer_data*)transferdata)->lock_firstnode,((index_buffer_data*)transferdata)->lock_index,((index_buffer_data*)transferdata)->lock_disk);
            }
            else
            {
                fprintf(stderr, "error: cannot insert record in index, since sax representation\
                    failed to be created");
            }

        }
        pthread_barrier_wait(lock_barrier3);
        while(1)
        {
            j=__sync_fetch_and_add(((index_buffer_data*)transferdata)->nodecounter,1);
            if(j>=index->fbl->number_of_buffers)
            {
                break;
            }
            fbl_soft_buffer *current_fbl_node = &index->fbl->soft_buffers[j];
            if (!current_fbl_node->initialized) {
                continue;
            }
            for (i=((index_buffer_data*)transferdata)->bufferpresize[j]; i<current_fbl_node->buffer_size; i++) 
            {
                r->sax = (sax_type *) current_fbl_node->sax_records[i];
                r->position = (file_position_type *) current_fbl_node->pos_records[i];
                r->insertion_mode = NO_TMP | PARTIAL;
                
                // Add record to index
                add_record_to_node(index, current_fbl_node->node, r, 1);
                
            }
            ((index_buffer_data*)transferdata)->bufferpresize[j]=current_fbl_node->buffer_size;
        }
        
        pthread_barrier_wait(lock_barrier1);
        pthread_barrier_wait(lock_barrier2);
    }
    free(pos);
    free(sax);
}
/*void* indexbulkloadingworker_pRecBuf_old(void *transferdata)
{
    sax_type * sax = malloc(sizeof(sax_type) * ((index_buffer_data*)transferdata)->index->settings->paa_segments);

    int fin_number=((index_buffer_data*)transferdata)->fin_number;
    file_position_type *pos = malloc(sizeof(file_position_type));
    sax_type * saxv=(((index_buffer_data*)transferdata)->saxv);
    int offset_saxv=((index_buffer_data*)transferdata)->blocid*((index_buffer_data*)transferdata)->index->settings->paa_segments;
    int paa_segments=((index_buffer_data*)transferdata)->index->settings->paa_segments;
    isax_index *index= ((index_buffer_data*)transferdata)->index;
    int i=0;
    for (i=0;i<fin_number;i++)
    {
    #ifndef DEBUG
    #if VERBOSE_LEVEL == 2
        printf("\r\x1b[32mLoading: \x1b[36m%d\x1b[0m",(i + 1));
    #endif
    #endif

        if(sax_from_ts(((ts_type*)(((index_buffer_data*)transferdata)->ts)+i*index->settings->timeseries_size), sax, index->settings->ts_values_per_paa_segment,
                       index->settings->paa_segments, index->settings->sax_alphabet_cardinality,
                       index->settings->sax_bit_cardinality) == SUCCESS)
        {
            *pos= ((index_buffer_data*)transferdata)->pos+index->settings->timeseries_size*sizeof(ts_type)*i;
            
            //for(j=0;j<((index_buffer_data*)transferdata)->index->settings->paa_segments;j++)
            //{
                //saxv[offset_saxv+i*paa_segments+j]=sax[j];
                //printf("the sax [%d ]%d\n",i,(int)saxv[offset_saxv+i*paa_segments+j]);
                 //printf("the sax [%d ]%d\n",i,(int)sax[j]);
            //}
                        
            memcpy(&(saxv[offset_saxv+i*paa_segments]), sax, sizeof(sax_type) * paa_segments);
        
            #ifdef CLUSTERED
            root_mask_type first_bit_mask = 0x00;
            * pos=(index_buffer_data)transferdata->pos+index->settings->timeseries_size*sizeof(ts_type)*i;
            CREATE_MASK(first_bit_mask, index, sax);
            char* pfilename = malloc(255);
            snprintf(pfilename, 255, "%s.%llu",index->settings->raw_filename,first_bit_mask);
            FILE *pfile = fopen(pfilename, "a+");
            * pos=(index_buffer_data)transferdata->pos+index->settings->timeseries_size*sizeof(ts_type)*i;
            //
            fwrite(ts, sizeof(ts_type), index->settings->timeseries_size, pfile);
            //
            fclose(pfile);
            free(pfilename);
            #endif
            
            //printf("the pos is %lld\n",*pos);
            //isax_fbl_index_insert(index, sax, pos);
            isax_pRecBuf_index_insert(index, sax, pos, ((index_buffer_data*)transferdata)->lock_record,((index_buffer_data*)transferdata)->lock_firstnode,((index_buffer_data*)transferdata)->workernumber,((index_buffer_data*)transferdata)->total_workernumber);

        }
        else
        {
            fprintf(stderr, "error: cannot insert record in index, since sax representation\
                    failed to be created");
        }


    }
    free(pos);
    free(sax);

}*/
void* indexbulkloadingworker_pRecBuf(void *transferdata)
{
    sax_type * sax = malloc(sizeof(sax_type) * ((index_buffer_data*)transferdata)->index->settings->paa_segments);

    int fin_number=((index_buffer_data*)transferdata)->fin_number;

    file_position_type *pos = malloc(sizeof(file_position_type));
    sax_type * saxv;

    int offset_saxv=((index_buffer_data*)transferdata)->blocid*((index_buffer_data*)transferdata)->index->settings->paa_segments;
    int paa_segments=((index_buffer_data*)transferdata)->index->settings->paa_segments;
    isax_index *index= ((index_buffer_data*)transferdata)->index;
    int i=0;
    pthread_barrier_t *lock_barrier1=((index_buffer_data*)transferdata)->lock_barrier1;

    pthread_barrier_t *lock_barrier2=((index_buffer_data*)transferdata)->lock_barrier2;

    while(!((index_buffer_data*)transferdata)->finished)
    {   
        saxv=(((index_buffer_data*)transferdata)->saxv);
        printf("this is the bulkloading worker");
        for (i=0;i<fin_number;i++)
        {
            if(sax_from_ts(((ts_type*)(((index_buffer_data*)transferdata)->ts)+i*index->settings->timeseries_size), sax, index->settings->ts_values_per_paa_segment,
                       index->settings->paa_segments, index->settings->sax_alphabet_cardinality,
                       index->settings->sax_bit_cardinality) == SUCCESS)
            {
                *pos= ((index_buffer_data*)transferdata)->pos+index->settings->timeseries_size*sizeof(ts_type)*i;
                memcpy(&(saxv[offset_saxv+i*paa_segments]), sax, sizeof(sax_type) * paa_segments);
                isax_pRecBuf_index_insert(index, sax, pos, ((index_buffer_data*)transferdata)->lock_record,((index_buffer_data*)transferdata)->lock_firstnode,((index_buffer_data*)transferdata)->workernumber,((index_buffer_data*)transferdata)->total_workernumber);

            }
            else
            {
                fprintf(stderr, "error: cannot insert record in index, since sax representation\
                    failed to be created");
            }

        }

        pthread_barrier_wait(lock_barrier1);
        //printf("the work ID is %d\n",((index_buffer_data*)transferdata)->workernumber );  
        pthread_barrier_wait(lock_barrier2);
    }
    free(pos);
    free(sax);
}
void* indexbulkloadingworker_2RecBuf(void *transferdata)
{
    sax_type * sax = malloc(sizeof(sax_type) * ((index_buffer_data*)transferdata)->index->settings->paa_segments);

    int fin_number=((index_buffer_data*)transferdata)->fin_number;

    file_position_type *pos = malloc(sizeof(file_position_type));
    sax_type * saxv;
    file_position_type readpos;
    int offset_saxv=((index_buffer_data*)transferdata)->blocid*((index_buffer_data*)transferdata)->index->settings->paa_segments;
    int paa_segments=((index_buffer_data*)transferdata)->index->settings->paa_segments;
    isax_index *index= ((index_buffer_data*)transferdata)->index;
            int initialworkernumber=((index_buffer_data*)transferdata)->workernumber;
            int workernumber=initialworkernumber;
        int total_workernumber=((index_buffer_data*)transferdata)->total_workernumber;
    int i=0,offsetvalue=0;
    pthread_barrier_t *lock_barrier1=((index_buffer_data*)transferdata)->lock_barrier1;
    pthread_barrier_t *lock_barrier2=((index_buffer_data*)transferdata)->lock_barrier2;
    ts_type *ts;
    while(!((index_buffer_data*)transferdata)->finished)
    {   
        lock_barrier1=((index_buffer_data*)transferdata)->lock_barrier1;
        lock_barrier2=((index_buffer_data*)transferdata)->lock_barrier2;
        ts=(ts_type*)(((index_buffer_data*)transferdata)->ts);
        saxv=(((index_buffer_data*)transferdata)->saxv);
        readpos=((index_buffer_data*)transferdata)->pos;

        for (i=0;i<fin_number;i++)
        {
            if(sax_from_ts((ts+i*index->settings->timeseries_size), sax, index->settings->ts_values_per_paa_segment,
                       index->settings->paa_segments, index->settings->sax_alphabet_cardinality,
                       index->settings->sax_bit_cardinality) == SUCCESS)
            {
                *pos=readpos +index->settings->timeseries_size*sizeof(ts_type)*i;
                memcpy(&(saxv[offset_saxv+i*paa_segments]), sax, sizeof(sax_type) * paa_segments);
                isax_2pRecBuf_index_insert(index, sax, pos, ((index_buffer_data*)transferdata)->lock_record,((index_buffer_data*)transferdata)->lock_firstnode,workernumber,total_workernumber);
            }
            else
            {
                fprintf(stderr, "error: cannot insert record in index, since sax representation\
                    failed to be created");
            }

        }

        //spthread_barrier_wait(lock_barrier1);
        //printf("the work ID is %d\n",((index_buffer_data*)transferdata)->workernumber );  
        pthread_barrier_wait(lock_barrier2);
        offsetvalue=1-offsetvalue;
        workernumber=initialworkernumber+offsetvalue*total_workernumber/2;
    }
    
    free(pos);
    free(sax);
}

isax_node * insert_to_fbl_m(first_buffer_layer *fbl, sax_type *sax,
                          file_position_type *pos,root_mask_type mask, 
                          isax_index *index, pthread_mutex_t *lock_firstnode,pthread_mutex_t *lockfbl) 
{
    //pthread_rwlock_wrlock(lockfbl);

    fbl_soft_buffer *current_buffer = &fbl->soft_buffers[(int) mask];
    char * cd_s,*cd_p;
    // Check if this buffer is initialized
    if (!current_buffer->initialized) {
    #ifdef DEBUG
        printf("*** Creating new FBL node. ***\n\n");
    #endif
        current_buffer->initialized = 1;
        current_buffer->max_buffer_size = 0;
        current_buffer->buffer_size = 0;

        current_buffer->node = isax_root_node_init(mask, 
                                                   index->settings->initial_leaf_buffer_size);

        current_buffer->node->is_leaf = 1;
        pthread_mutex_lock(lock_firstnode);

        index->root_nodes++;//counter

        
        if(index->first_node == NULL)
        {
            index->first_node = current_buffer->node;
            pthread_mutex_unlock(lock_firstnode);
            current_buffer->node->next = NULL;
            current_buffer->node->previous = NULL;
            
        }
        else
        {
            isax_node * prev_first = index->first_node;
            index->first_node = current_buffer->node;
            index->first_node->next = prev_first;
            
            prev_first->previous = current_buffer->node;
            pthread_mutex_unlock(lock_firstnode);
        }
    }  

    // Check if this buffer is not full!
    if (current_buffer->buffer_size >= current_buffer->max_buffer_size) {
        
        if(current_buffer->max_buffer_size == 0) {
            current_buffer->max_buffer_size = fbl->initial_buffer_size;
            
            current_buffer->sax_records = malloc(sizeof(sax_type *) * 
                                                 current_buffer->max_buffer_size);
            current_buffer->pos_records = malloc(sizeof(file_position_type *) * 
                                                 current_buffer->max_buffer_size);
        }
        else {
            current_buffer->max_buffer_size *= BUFFER_REALLOCATION_RATE;
            
            current_buffer->sax_records = realloc(current_buffer->sax_records,
                                           sizeof(sax_type *) * 
                                           current_buffer->max_buffer_size);
            current_buffer->pos_records = realloc(current_buffer->pos_records,
                                           sizeof(file_position_type *) * 
                                           current_buffer->max_buffer_size);
            
        }
    }
    if (current_buffer->sax_records == NULL || current_buffer->pos_records == NULL) {
        fprintf(stderr, "error: Could not allocate memory in FBL.");
        return OUT_OF_MEMORY_FAILURE;
    }

    // Copy data to hard buffer and make current buffer point to the hard one
    //pthread_mutex_lock(lockfbl);
    //COUNT_CAL_TIME_START
    //fbl->current_record_index++;
    //cd_s=fbl->current_record;
    //fbl->current_record += index->settings->sax_byte_size;
    cd_s= __sync_fetch_and_add(&(fbl->current_record),index->settings->sax_byte_size);

    //cd_p=fbl->current_record;
    //fbl->current_record += index->settings->position_byte_size;
    cd_p= __sync_fetch_and_add(&(fbl->current_record),index->settings->position_byte_size);
    //pthread_mutex_unlock(lockfbl);
    
    current_buffer->sax_records[current_buffer->buffer_size] = (sax_type*) cd_s;    
    memcpy((void *) cd_s, (void *) sax, index->settings->sax_byte_size);
    
    
    current_buffer->pos_records[current_buffer->buffer_size] = (file_position_type*) cd_p;    
    memcpy((void *) cd_p, (void *) pos, index->settings->position_byte_size);


                

     
    #ifdef DEBUG
    printf("*** Added to node ***\n\n");
    #ifdef TOY
    sax_print(sax, index->settings->paa_segments, 
              index->settings->sax_bit_cardinality);
    #endif
    #endif
    
    
    //COUNT_CAL_TIME_END
    
    current_buffer->buffer_size++;
    return current_buffer->node;
}
isax_node * insert_to_fbl_m_new(first_buffer_layer *fbl, sax_type *sax,
                          file_position_type *pos,root_mask_type mask, 
                          isax_index *index, pthread_mutex_t *lock_firstnode,pthread_mutex_t *lockfbl) 
{
    //pthread_rwlock_wrlock(lockfbl);

    fbl_soft_buffer *current_buffer = &fbl->soft_buffers[(int) mask];
    char * cd_s,*cd_p;
    // Check if this buffer is initialized
    if (!current_buffer->initialized) {
    #ifdef DEBUG
        printf("*** Creating new FBL node. ***\n\n");
    #endif
        current_buffer->initialized = 1;
        current_buffer->max_buffer_size = 0;
        current_buffer->buffer_size = 0;

        current_buffer->node = isax_root_node_init(mask, 
                                                   index->settings->initial_leaf_buffer_size);

        current_buffer->node->is_leaf = 1;
        pthread_mutex_lock(lock_firstnode);

        index->root_nodes++;//counter

        
        if(index->first_node == NULL)
        {
            index->first_node = current_buffer->node;
            pthread_mutex_unlock(lock_firstnode);
            current_buffer->node->next = NULL;
            current_buffer->node->previous = NULL;
            
        }
        else
        {
            isax_node * prev_first = index->first_node;
            index->first_node = current_buffer->node;
            index->first_node->next = prev_first;
            
            prev_first->previous = current_buffer->node;
            pthread_mutex_unlock(lock_firstnode);
        }
    }  

    // Check if this buffer is not full!
    if (current_buffer->buffer_size >= current_buffer->max_buffer_size) {
        
        if(current_buffer->max_buffer_size == 0) {
            current_buffer->max_buffer_size = fbl->initial_buffer_size;
            
            current_buffer->sax_records = malloc(sizeof(sax_type *) * 
                                                 current_buffer->max_buffer_size);
            current_buffer->pos_records = malloc(sizeof(file_position_type *) * 
                                                 current_buffer->max_buffer_size);
        }
        else {
            current_buffer->max_buffer_size *= BUFFER_REALLOCATION_RATE;
            
            current_buffer->sax_records = realloc(current_buffer->sax_records,
                                           sizeof(sax_type *) * 
                                           current_buffer->max_buffer_size);
            current_buffer->pos_records = realloc(current_buffer->pos_records,
                                           sizeof(file_position_type *) * 
                                           current_buffer->max_buffer_size);
            
        }
    }
    if (current_buffer->sax_records == NULL || current_buffer->pos_records == NULL) {
        fprintf(stderr, "error: Could not allocate memory in FBL.");
        return OUT_OF_MEMORY_FAILURE;
    }

    // Copy data to hard buffer and make current buffer point to the hard one
    //pthread_mutex_lock(lockfbl);
    //COUNT_CAL_TIME_START
    //fbl->current_record_index++;
    //cd_s=fbl->current_record;
    //fbl->current_record += index->settings->sax_byte_size;
    cd_s= __sync_fetch_and_add(&(fbl->current_record),index->settings->sax_byte_size);

    //cd_p=fbl->current_record;
    //fbl->current_record += index->settings->position_byte_size;
    cd_p= __sync_fetch_and_add(&(fbl->current_record),index->settings->position_byte_size);
    //pthread_mutex_unlock(lockfbl);
    
    current_buffer->sax_records[current_buffer->buffer_size] = (sax_type*) cd_s;    
    memcpy((void *) cd_s, (void *) sax, index->settings->sax_byte_size);
    
    
    current_buffer->pos_records[current_buffer->buffer_size] = (file_position_type*) cd_p;    
    memcpy((void *) cd_p, (void *) pos, index->settings->position_byte_size);
    isax_node_record *r = malloc(sizeof(isax_node_record));
    r->sax = (sax_type*) cd_s;
    r->position = (file_position_type*) cd_p; 
    r->insertion_mode = NO_TMP | PARTIAL;
                // Add record to index
    add_record_to_node(index, current_buffer->node, r, 1);
                

    free(r);
    #ifdef DEBUG
    printf("*** Added to node ***\n\n");
    #ifdef TOY
    sax_print(sax, index->settings->paa_segments, 
              index->settings->sax_bit_cardinality);
    #endif
    #endif
    
    
    //COUNT_CAL_TIME_END
    
    current_buffer->buffer_size++;
    return current_buffer->node;
}


isax_node * insert_to_pRecBuf(parallel_first_buffer_layer *fbl, sax_type *sax,
                          file_position_type *pos,root_mask_type mask, 
                          isax_index *index, pthread_mutex_t *lock_firstnode, int workernumber,int total_workernumber) 
{
    //pthread_rwlock_wrlock(lockfbl);
    parallel_fbl_soft_buffer *current_buffer = &fbl->soft_buffers[(int) mask];

    file_position_type *filepointer;
    sax_type *saxpointer;

    int current_buffer_number;
    char * cd_s,*cd_p;
    // Check if this buffer is initialized


    if (!current_buffer->initialized) 
    {
        pthread_mutex_lock(lock_firstnode);
        if (!current_buffer->initialized) 
        {
            
            current_buffer->max_buffer_size = malloc(sizeof(int)*total_workernumber);
            current_buffer->buffer_size = malloc(sizeof(int)*total_workernumber);
            current_buffer->sax_records=malloc(sizeof(sax_type *)*total_workernumber);
            current_buffer->pos_records=malloc(sizeof(file_position_type *)*total_workernumber);
            for (int i = 0; i < total_workernumber; i++)
            {
                current_buffer->max_buffer_size[i]=0;
                current_buffer->buffer_size[i]=0;
                current_buffer->pos_records[i]=NULL;
                current_buffer->sax_records[i]=NULL;
            }
            current_buffer->node = isax_root_node_init(mask,index->settings->initial_leaf_buffer_size);
            current_buffer->node->is_leaf = 1;
            //current_buffer->finished=1;
            current_buffer->initialized = 1;
            //__sync_synchronize();
            if(index->first_node == NULL)
            {
                index->first_node = current_buffer->node;
                pthread_mutex_unlock(lock_firstnode);
                current_buffer->node->next = NULL;
                current_buffer->node->previous = NULL;
                
            }
            else
            {
                isax_node * prev_first = index->first_node;
                index->first_node = current_buffer->node;
                index->first_node->next = prev_first;
                prev_first->previous = current_buffer->node;
                pthread_mutex_unlock(lock_firstnode);
            }
            __sync_fetch_and_add(&(index->root_nodes),1);
        }
        else
        {
           pthread_mutex_unlock(lock_firstnode); 
        }
    }  
    
    // Check if this buffer is not full!
    if (current_buffer->buffer_size[workernumber] >= current_buffer->max_buffer_size[workernumber]) {
        if(current_buffer->max_buffer_size[workernumber] == 0) {
            current_buffer->max_buffer_size[workernumber] = fbl->initial_buffer_size;
            current_buffer->sax_records[workernumber] = malloc(index->settings->sax_byte_size * 
                                                 current_buffer->max_buffer_size[workernumber]);
            current_buffer->pos_records[workernumber] = malloc(index->settings->position_byte_size* 
                                                 current_buffer->max_buffer_size[workernumber]);
        }
        else {
            current_buffer->max_buffer_size[workernumber] *= BUFFER_REALLOCATION_RATE;
            
            current_buffer->sax_records[workernumber] = realloc(current_buffer->sax_records[workernumber],
                                           index->settings->sax_byte_size * 
                                           current_buffer->max_buffer_size[workernumber]);
            current_buffer->pos_records[workernumber] = realloc(current_buffer->pos_records[workernumber],
                                           index->settings->position_byte_size *
                                           current_buffer->max_buffer_size[workernumber]);
            
        }
    }

    if (current_buffer->sax_records[workernumber] == NULL || current_buffer->pos_records[workernumber] == NULL) {
        fprintf(stderr, "error: Could not allocate memory in FBL.");
        return OUT_OF_MEMORY_FAILURE;
    }
    // Copy data to hard buffer and make current buffer point to the hard one
    //pthread_mutex_lock(lockfbl);
    //COUNT_CAL_TIME_START
    //fbl->current_record_index++;
    
    //cd_s=fbl->current_record;
    //fbl->current_record += index->settings->sax_byte_size;
    //cd_s= __sync_fetch_and_add(&(fbl->current_record),index->settings->sax_byte_size+index->settings->position_byte_size);
    //cd_p=fbl->current_record;
    //fbl->current_record += index->settings->position_byte_size;
    //cd_p= cd_s+index->settings->sax_byte_size;
    //pthread_mutex_unlock(lockfbl);
    current_buffer_number=current_buffer->buffer_size[workernumber];
    filepointer=(file_position_type *)current_buffer->pos_records[workernumber];
    saxpointer=(sax_type *)current_buffer->sax_records[workernumber];
    //printf("the work number is %d sax is  %d \n",workernumber,saxpointer[current_buffer_number*index->settings->paa_segments]);
    memcpy((void *) (&saxpointer[current_buffer_number*index->settings->paa_segments]), (void *) sax, index->settings->sax_byte_size);
    memcpy((void *) (&filepointer[current_buffer_number]), (void *) pos, index->settings->position_byte_size);
    
    #ifdef DEBUG
    printf("*** Added to node ***\n\n");
    #ifdef TOY
    sax_print(sax, index->settings->paa_segments, 
              index->settings->sax_bit_cardinality);
    #endif
    #endif
    //printf("this is befor the checke \n");
    //__sync_fetch_and_add(&((current_buffer->buffer_size[workernumber])),1);
    //printf("this is after  the checke \n");
    (current_buffer->buffer_size[workernumber])++;

    return current_buffer->node;
}
isax_node * insert_to_2pRecBuf(parallel_dfirst_buffer_layer *fbl, sax_type *sax,
                          file_position_type *pos,root_mask_type mask, 
                          isax_index *index, pthread_mutex_t *lock_firstnode, int workernumber,int total_workernumber) 
{
    //pthread_rwlock_wrlock(lockfbl);
    parallel_dfbl_soft_buffer *current_buffer = &fbl->soft_buffers[(int) mask];

    file_position_type **filepointer;
    sax_type **saxpointer;

    int current_buffer_number;
    char * cd_s,*cd_p;
    // Check if this buffer is initialized


    if (!current_buffer->initialized) 
    {
        pthread_mutex_lock(lock_firstnode);
        if (!current_buffer->initialized) 
        {
            
            current_buffer->max_buffer_size = malloc(sizeof(int)*total_workernumber);
            current_buffer->buffer_size = malloc(sizeof(int)*total_workernumber);
            current_buffer->sax_records=malloc(sizeof(sax_type **)*total_workernumber);
            current_buffer->pos_records=malloc(sizeof(file_position_type **)*total_workernumber);
            for (int i = 0; i < total_workernumber; i++)
            {
                current_buffer->max_buffer_size[i]=0;
                current_buffer->buffer_size[i]=0;
                current_buffer->pos_records[i]=NULL;
                current_buffer->sax_records[i]=NULL;
            }
            current_buffer->node = isax_root_node_init(mask,index->settings->initial_leaf_buffer_size);
            current_buffer->node->is_leaf = 1;
            //current_buffer->finished=1;
            current_buffer->initialized = 1;
            //__sync_synchronize();
            if(index->first_node == NULL)
            {
                index->first_node = current_buffer->node;
                pthread_mutex_unlock(lock_firstnode);
                current_buffer->node->next = NULL;
                current_buffer->node->previous = NULL;
                
            }
            else
            {
                isax_node * prev_first = index->first_node;
                index->first_node = current_buffer->node;
                index->first_node->next = prev_first;
                prev_first->previous = current_buffer->node;
                pthread_mutex_unlock(lock_firstnode);
            }
            __sync_fetch_and_add(&(index->root_nodes),1);
        }
        else
        {
           pthread_mutex_unlock(lock_firstnode); 
        }
    }  
    
    // Check if this buffer is not full!
    if (current_buffer->buffer_size[workernumber] >= current_buffer->max_buffer_size[workernumber]) {
        if(current_buffer->max_buffer_size[workernumber] == 0) {
            current_buffer->max_buffer_size[workernumber] = fbl->initial_buffer_size;
            current_buffer->sax_records[workernumber] = malloc(sizeof(sax_type *) * 
                                                 current_buffer->max_buffer_size[workernumber]);
            current_buffer->pos_records[workernumber] = malloc(sizeof(file_position_type *)* 
                                                 current_buffer->max_buffer_size[workernumber]);
        }
        else {
            current_buffer->max_buffer_size[workernumber] *= BUFFER_REALLOCATION_RATE;
            
            current_buffer->sax_records[workernumber] = realloc(current_buffer->sax_records[workernumber],
                                           sizeof(sax_type *) * 
                                           current_buffer->max_buffer_size[workernumber]);
            current_buffer->pos_records[workernumber] = realloc(current_buffer->pos_records[workernumber],
                                           sizeof(file_position_type *) *
                                           current_buffer->max_buffer_size[workernumber]);
            
        }
    }

    if (current_buffer->sax_records[workernumber] == NULL || current_buffer->pos_records[workernumber] == NULL) {
        fprintf(stderr, "error: Could not allocate memory in FBL.");
        return OUT_OF_MEMORY_FAILURE;
    }
    // Copy data to hard buffer and make current buffer point to the hard one
    //pthread_mutex_lock(lockfbl);
    //COUNT_CAL_TIME_START
    //fbl->current_record_index++;
    
    //cd_s=fbl->current_record;
    //fbl->current_record += index->settings->sax_byte_size;
    //cd_s= __sync_fetch_and_add(&(fbl->current_record),index->settings->sax_byte_size+index->settings->position_byte_size);
    //cd_p=fbl->current_record;
    //fbl->current_record += index->settings->position_byte_size;
    //cd_p= cd_s+index->settings->sax_byte_size;
    //pthread_mutex_unlock(lockfbl);
    current_buffer_number=current_buffer->buffer_size[workernumber];
    filepointer=(file_position_type **)current_buffer->pos_records[workernumber];
    saxpointer=(sax_type **)current_buffer->sax_records[workernumber];
    saxpointer[current_buffer_number]=((parallel_dfirst_buffer_layer*)index->fbl)->hard_buffer+*pos/sizeof(ts_type)/index->settings->timeseries_size*index->settings->sax_byte_size;
    filepointer[current_buffer_number]=(file_position_type *)(((parallel_dfirst_buffer_layer*)index->fbl)->hard_buffer+*pos/sizeof(ts_type)/index->settings->timeseries_size*index->settings->sax_byte_size+index->settings->sax_byte_size);
    //printf("the work number is %d sax is  %d \n",workernumber,saxpointer[current_buffer_number*index->settings->paa_segments]);
    //memcpy((void *) (&saxpointer[current_buffer_number]), (void *) ((parallel_dfirst_buffer_layer*)index->fbl)->hard_buffer+*pos/sizeof(ts_type)/index->settings->timeseries_size*index->settings->sax_byte_size , sizeof(sax_type *));
    //memcpy((void *) (&filepointer[current_buffer_number]), (void *) ((parallel_dfirst_buffer_layer*)index->fbl)->hard_buffer+*pos/sizeof(ts_type)/index->settings->timeseries_size*index->settings->sax_byte_size+index->settings->sax_byte_size, sizeof(file_position_type *));
    
    #ifdef DEBUG
    printf("*** Added to node ***\n\n");
    #ifdef TOY
    sax_print(sax, index->settings->paa_segments, 
              index->settings->sax_bit_cardinality);
    #endif
    #endif
    //printf("this is befor the checke \n");
    //__sync_fetch_and_add(&((current_buffer->buffer_size[workernumber])),1);
    //printf("this is after  the checke \n");
    (current_buffer->buffer_size[workernumber])++;

    return current_buffer->node;
}
enum response indexconstruction(first_buffer_layer *fbl, isax_index *index,pthread_mutex_t *lock_index,pthread_mutex_t *lock_disk,int calculate_thread) 
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
    trans_fbl_input input_data;
    pthread_t threadid[calculate_thread];
    pthread_mutex_t lock_fbl_conter=PTHREAD_MUTEX_INITIALIZER;
    input_data.index=index;
    input_data.lock_index=lock_index;
    input_data.lock_fbl_conter=&lock_fbl_conter;
    input_data.lock_write=lock_disk;
    input_data.conternumber=0;
    input_data.stop_number=fbl->number_of_buffers;
    //start the loop
    COUNT_QUEUE_TIME_START
    for (int k = 0; k < calculate_thread; k++)
    {
        pthread_create(&(threadid[k]),NULL,indexconstructionworker,(void*)&(input_data));
    }
    for (int k = 0; k < calculate_thread; k++)
    {
        pthread_join(threadid[k],NULL);
    }   
    
    fbl->current_record_index = 0;
    fbl->current_record = fbl->hard_buffer;
    #ifndef DEBUG
    #if VERBOSE_LEVEL == 2
    printf("\n");
    #endif
    #endif
    
    return SUCCESS;
}
enum response indexflush(first_buffer_layer *fbl, isax_index *index,pthread_mutex_t *lock_index,pthread_mutex_t *lock_disk,int calculate_thread) 
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
    trans_fbl_input input_data;
    pthread_t threadid[calculate_thread];
    pthread_mutex_t lock_fbl_conter=PTHREAD_MUTEX_INITIALIZER;
    input_data.index=index;
    input_data.lock_index=lock_index;
    input_data.lock_fbl_conter=&lock_fbl_conter;
    input_data.lock_write=lock_disk;
    input_data.conternumber=0;
    input_data.stop_number=fbl->number_of_buffers;
    //start the loop
    COUNT_QUEUE_TIME_START
    for (int k = 0; k < calculate_thread; k++)
    {
        pthread_create(&(threadid[k]),NULL,indexflushworker,(void*)&(input_data));
    }
    for (int k = 0; k < calculate_thread; k++)
    {
        pthread_join(threadid[k],NULL);
    }
    
    
    fbl->current_record_index = 0;
    fbl->current_record = fbl->hard_buffer;
    #ifndef DEBUG
    #if VERBOSE_LEVEL == 2
    printf("\n");
    #endif
    #endif
    
    return SUCCESS;
}
enum response indexpRecBufflush(parallel_first_buffer_layer *fbl, isax_index *index,pthread_mutex_t *lock_index,pthread_mutex_t *lock_disk,int calculate_thread,int preworkernumber) 
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
    trans_fbl_input input_data;
    pthread_t threadid[calculate_thread];
    pthread_mutex_t lock_fbl_conter=PTHREAD_MUTEX_INITIALIZER;
    input_data.index=index;
    input_data.lock_index=lock_index;
    input_data.lock_fbl_conter=&lock_fbl_conter;
    input_data.lock_write=lock_disk;
    input_data.conternumber=0;
    input_data.stop_number=fbl->number_of_buffers;
    input_data.preworkernumber=preworkernumber;
    //start the loop
    COUNT_QUEUE_TIME_START
    for (int k = 0; k < calculate_thread; k++)
    {
        pthread_create(&(threadid[k]),NULL,indexpRecBufflushworker,(void*)&(input_data));
    }
    for (int k = 0; k < calculate_thread; k++)
    {
        pthread_join(threadid[k],NULL);
    }
    
    
    fbl->current_record_index = 0;
    fbl->current_record = fbl->hard_buffer;
    #ifndef DEBUG
    #if VERBOSE_LEVEL == 2
    printf("\n");
    #endif
    #endif
    
    return SUCCESS;
}
enum response indexconstruction_pRecBuf(first_buffer_layer *fbl, isax_index *index,pthread_mutex_t *lock_index,int calculate_thread) 
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
    trans_fbl_input input_data;
    pthread_t threadid[maxreadthread];
    pthread_mutex_t lock_fbl_conter=PTHREAD_MUTEX_INITIALIZER;
    input_data.index=index;
    input_data.lock_index=lock_index;
    input_data.lock_fbl_conter=&lock_fbl_conter;
    input_data.conternumber=0;
    input_data.stop_number=fbl->number_of_buffers;
    input_data.preworkernumber=calculate_thread-1;
    //start the loop
    
    for (int k = 0; k < calculate_thread; k++)
    {
        pthread_create(&(threadid[k]),NULL,indexconstructionworker_pRecBuf,(void*)&(input_data));
    }

    for (int k = 0; k < calculate_thread; k++)
    {
        pthread_join(threadid[k],NULL);
    }   
    
    fbl->current_record_index = 0;
    fbl->current_record = fbl->hard_buffer;
    #ifndef DEBUG
    #if VERBOSE_LEVEL == 2
    printf("\n");
    #endif
    #endif
    
    return SUCCESS;
}
/*void* fonction3(void *input)
{
    isax_index *index=((trans_fbl_input*)input)->index;
    pthread_mutex_t *lock_index=((trans_fbl_input*)input)->lock_index;
    pthread_mutex_t *lock_write=((trans_fbl_input*)input)->lock_write;
    int j,c=1;
    isax_node_record *r = malloc(sizeof(isax_node_record));
    for (j=((trans_fbl_input*)input)->start_number; j<((trans_fbl_input*)input)->stop_number; j++) 
    {
        
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
            // For all records in this buffer 
            //COUNT_CAL_TIME_START
            //printf("current_fbl_node->buffer_size is %d\n",current_fbl_node->buffer_size );
            for (i=0; i<current_fbl_node->buffer_size; i++) {
                r->sax = (sax_type *) current_fbl_node->sax_records[i];
                r->position = (file_position_type *) current_fbl_node->pos_records[i];
                r->insertion_mode = NO_TMP | PARTIAL;
                // Add record to index
                
                add_record_to_node(index, current_fbl_node->node, r, 1);
                
            }

            // flush index node
            //COUNT_CAL_TIME_START
            flush_subtree_leaf_buffers_m(index, current_fbl_node->node,lock_index,lock_write);
            //COUNT_CAL_TIME_END
            // clear FBL records moved in LBL buffers
            free(current_fbl_node->sax_records);
            free(current_fbl_node->pos_records);
            // clear records read from files (free only prev sax buffers)

            isax_index_clear_node_buffers(index, current_fbl_node->node, 
                                          INCLUDE_CHILDREN,
                                          TMP_AND_TS_CLEAN);
            
            index->allocated_memory = 0;
            // Set to 0 in order to re-allocate original space for buffers.
            current_fbl_node->buffer_size = 0;
            current_fbl_node->max_buffer_size = 0;
        }
        
    }
}*/
void* indexconstructionworker(void *input)
{
    //struct timeval workertimestart;
    //struct timeval writetiemstart;
    //struct timeval workercurenttime;
    //struct timeval writecurenttime;
    //double worker_total_time,write_total_time;
    //gettimeofday(&workertimestart, NULL);



    isax_index *index=((trans_fbl_input*)input)->index;
    pthread_mutex_t *lock_index=((trans_fbl_input*)input)->lock_index;
    pthread_mutex_t *lock_write=((trans_fbl_input*)input)->lock_write;
    int j,c=1;
    isax_node_record *r = malloc(sizeof(isax_node_record));
    //for (j=((trans_fbl_input*)input)->start_number; j<((trans_fbl_input*)input)->stop_number; j++) 
    while(1)
    {
        pthread_mutex_lock(((trans_fbl_input*)input)->lock_fbl_conter);
        //COUNT_CAL_TIME_START
        j=((trans_fbl_input*)input)->conternumber;
        ((trans_fbl_input*)input)->conternumber++;
        //COUNT_CAL_TIME_END
        pthread_mutex_unlock(((trans_fbl_input*)input)->lock_fbl_conter);
        if(j>=((trans_fbl_input*)input)->stop_number)
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
        if (current_fbl_node->buffer_size > 0) 
        {
            // For all records in this buffer 
            //COUNT_CAL_TIME_START
            //printf("current_fbl_node->buffer_size is %d\n",current_fbl_node->buffer_size );
            for (i=0; i<current_fbl_node->buffer_size; i++) 
            {
                r->sax = (sax_type *) current_fbl_node->sax_records[i];
                r->position = (file_position_type *) current_fbl_node->pos_records[i];
                r->insertion_mode = NO_TMP | PARTIAL;
                // Add record to index
                add_record_to_node(index, current_fbl_node->node, r, 1);
                
            }

            // flush index node
            //COUNT_CAL_TIME_START
            //gettimeofday(&writetiemstart, NULL);
            flush_subtree_leaf_buffers_m(index, current_fbl_node->node,lock_index,lock_write);
            //gettimeofday(&writecurenttime, NULL); 
            //write_total_time += writecurenttime.tv_sec*1000000 + (writecurenttime.tv_usec)-writetiemstart.tv_sec*1000000 - (writetiemstart.tv_usec); 
            //COUNT_CAL_TIME_END
            // clear FBL records moved in LBL buffers
            free(current_fbl_node->sax_records);
            free(current_fbl_node->pos_records);
            // clear records read from files (free only prev sax buffers)

            isax_index_clear_node_buffers(index, current_fbl_node->node, 
                                          INCLUDE_CHILDREN,
                                          TMP_AND_TS_CLEAN);
            
            index->allocated_memory = 0;
            // Set to 0 in order to re-allocate original space for buffers.
            current_fbl_node->buffer_size = 0;
            current_fbl_node->max_buffer_size = 0;
        }
    }
    //gettimeofday(&workercurenttime, NULL); 
    //worker_total_time += workercurenttime.tv_sec*1000000 + (workercurenttime.tv_usec)-workertimestart.tv_sec*1000000 - (workertimestart.tv_usec); 
    //printf("the index construction worker time is %f \n",worker_total_time );
    //printf("the worker's write  time is %f \n",write_total_time );
    free(r);
    
}
void* indexflushworker(void *input)
{
    isax_index *index=((trans_fbl_input*)input)->index;
    pthread_mutex_t *lock_index=((trans_fbl_input*)input)->lock_index;
    pthread_mutex_t *lock_write=((trans_fbl_input*)input)->lock_write;
    int j,c=1;
    isax_node_record *r = malloc(sizeof(isax_node_record));
    //for (j=((trans_fbl_input*)input)->start_number; j<((trans_fbl_input*)input)->stop_number; j++) 
    while(1)
    {
        pthread_mutex_lock(((trans_fbl_input*)input)->lock_fbl_conter);
        //COUNT_CAL_TIME_START
        j=((trans_fbl_input*)input)->conternumber;
        ((trans_fbl_input*)input)->conternumber++;
        //COUNT_CAL_TIME_END
        pthread_mutex_unlock(((trans_fbl_input*)input)->lock_fbl_conter);
        if(j>=((trans_fbl_input*)input)->stop_number)
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
        if (current_fbl_node->buffer_size > 0) 
        {
            // For all records in this buffer 
            //COUNT_CAL_TIME_START
            //printf("current_fbl_node->buffer_size is %d\n",current_fbl_node->buffer_size );

            flush_subtree_leaf_buffers_m(index, current_fbl_node->node,lock_index,lock_write);

            free(current_fbl_node->sax_records);
            free(current_fbl_node->pos_records);
            // clear records read from files (free only prev sax buffers)

            isax_index_clear_node_buffers(index, current_fbl_node->node, 
                                          INCLUDE_CHILDREN,
                                          TMP_AND_TS_CLEAN);
            
            index->allocated_memory = 0;
            // Set to 0 in order to re-allocate original space for buffers.
            current_fbl_node->buffer_size = 0;
            current_fbl_node->max_buffer_size = 0;
        }
    }
    //gettimeofday(&workercurenttime, NULL); 
    //worker_total_time += workercurenttime.tv_sec*1000000 + (workercurenttime.tv_usec)-workertimestart.tv_sec*1000000 - (workertimestart.tv_usec); 
    //printf("the index construction worker time is %f \n",worker_total_time );
    //printf("the worker's write  time is %f \n",write_total_time );
    free(r);
    
}

void* indexpRecBufflushworker(void *input)
{
    isax_index *index=((trans_fbl_input*)input)->index;
    pthread_mutex_t *lock_index=((trans_fbl_input*)input)->lock_index;
    pthread_mutex_t *lock_write=((trans_fbl_input*)input)->lock_write;
    int preworkernumber=((trans_fbl_input*)input)->preworkernumber;
    int j,c=1;
    bool have_record=false;
    //for (j=((trans_fbl_input*)input)->start_number; j<((trans_fbl_input*)input)->stop_number; j++) 
    while(1)
    {
        pthread_mutex_lock(((trans_fbl_input*)input)->lock_fbl_conter);
        //COUNT_CAL_TIME_START
        j=((trans_fbl_input*)input)->conternumber;
        ((trans_fbl_input*)input)->conternumber++;
        //COUNT_CAL_TIME_END
        pthread_mutex_unlock(((trans_fbl_input*)input)->lock_fbl_conter);
        if(j>=((trans_fbl_input*)input)->stop_number)
        {
            break;
        }
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
            // For all records in this buffer 
            //COUNT_CAL_TIME_START
            if (current_fbl_node->buffer_size[k] > 0)
            have_record=true;
        }
        if (have_record) 
        {
            // flush index node
            //COUNT_CAL_TIME_START
            flush_subtree_leaf_buffers_m(index, current_fbl_node->node,lock_index,lock_write);
            //COUNT_CAL_TIME_END
            // clear FBL records moved in LBL buffers
            for (int k = 0; k < preworkernumber; k++)
            {   
                if(current_fbl_node->sax_records[k]!=NULL)
                {
                    free((current_fbl_node->sax_records[k]));
                    free((current_fbl_node->pos_records[k]));
                    current_fbl_node->sax_records[k]=NULL;
                    current_fbl_node->pos_records[k]=NULL;
                }
                current_fbl_node->buffer_size[k] = 0;
                current_fbl_node->max_buffer_size[k] = 0;
            }

            // clear records read from files (free only prev sax buffers)

            isax_index_clear_node_buffers(index, current_fbl_node->node, 
                                          INCLUDE_CHILDREN,
                                          TMP_AND_TS_CLEAN);
            
            index->allocated_memory = 0;
        }
    }
    //gettimeofday(&workercurenttime, NULL); 
    //worker_total_time += workercurenttime.tv_sec*1000000 + (workercurenttime.tv_usec)-workertimestart.tv_sec*1000000 - (workertimestart.tv_usec); 
    //printf("the index construction worker time is %f \n",worker_total_time );
    //printf("the worker's write  time is %f \n",write_total_time );
    
}

void* indexconstructionworker_pRecBuf(void *input)
{
    isax_index *index=((trans_fbl_input*)input)->index;
    pthread_mutex_t *lock_index=((trans_fbl_input*)input)->lock_index;
    pthread_mutex_t *lock_write=((trans_fbl_input*)input)->lock_write;
    int j=0,k=0,c=1;
    bool have_record=false;
    int preworkernumber=((trans_fbl_input*)input)->preworkernumber;
    isax_node_record *r = malloc(sizeof(isax_node_record));
    while(1)
    {

        j=__sync_fetch_and_add(&(((trans_fbl_input*)input)->conternumber),1);
        //pthread_mutex_unlock(((trans_fbl_input*)input)->lock_fbl_conter);
        if(j>=((trans_fbl_input*)input)->stop_number)
        {
            break;
        }
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
            // For all records in this buffer 
            //COUNT_CAL_TIME_START
            if (current_fbl_node->buffer_size[k] > 0)
            have_record=true;
            for (i=0; i<current_fbl_node->buffer_size[k]; i++) 
            {
                r->sax = (sax_type *) &(((current_fbl_node->sax_records[k]))[i*index->settings->paa_segments]);
                r->position = (file_position_type *) &((file_position_type *)(current_fbl_node->pos_records[k]))[i];
                r->insertion_mode = NO_TMP | PARTIAL;
                // Add record to index

                sax_type* saxpointer=(sax_type *)current_fbl_node->sax_records[k];

                add_record_to_node(index, current_fbl_node->node, r, 1);
            }
        }
        if (have_record) 
        {
            // flush index node
            //COUNT_CAL_TIME_START
            flush_subtree_leaf_buffers_m(index, current_fbl_node->node,lock_index,lock_write);
            //COUNT_CAL_TIME_END
            // clear FBL records moved in LBL buffers
            for (int k = 0; k < preworkernumber; k++)
            {   
                if(current_fbl_node->sax_records[k]!=NULL)
                {
                    free((current_fbl_node->sax_records[k]));
                    free((current_fbl_node->pos_records[k]));
                    current_fbl_node->sax_records[k]=NULL;
                    current_fbl_node->pos_records[k]=NULL;
                }
                current_fbl_node->buffer_size[k] = 0;
                current_fbl_node->max_buffer_size[k] = 0;
            }

            // clear records read from files (free only prev sax buffers)

            isax_index_clear_node_buffers(index, current_fbl_node->node, 
                                          INCLUDE_CHILDREN,
                                          TMP_AND_TS_CLEAN);
            
            index->allocated_memory = 0;
            // Set to 0 in order to re-allocate original space for buffers.

        }
        
    }
    COUNT_QUEUE_TIME_START
    free(r);
}
void* indexconstructionworker_pRecBuf_new(void *input)
{
    isax_index *index=((trans_fbl_input*)input)->index;
    pthread_mutex_t *lock_index=((trans_fbl_input*)input)->lock_index;
    pthread_mutex_t *lock_write=((trans_fbl_input*)input)->lock_write;
    int fbloffset=((trans_fbl_input*)input)->fbloffset;
    int j=0,c=1;
    bool have_record=false;
    int preworkernumber=((trans_fbl_input*)input)->preworkernumber;
    isax_node_record *r = malloc(sizeof(isax_node_record));
    while(1)
    {
        //printf("this is the construction worker");
        
        j=__sync_fetch_and_add(&(((trans_fbl_input*)input)->conternumber),1);
        //pthread_mutex_unlock(((trans_fbl_input*)input)->lock_fbl_conter);
        if(j>=((trans_fbl_input*)input)->stop_number)
        {
            break;
        }
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

        for (int k = fbloffset; k < fbloffset+preworkernumber; k++)
        {
            // For all records in this buffer 
            //COUNT_CAL_TIME_START
            if (current_fbl_node->buffer_size[k] > 0)
            have_record=true;
            for (i=0; i<current_fbl_node->buffer_size[k]; i++) 
            {
                r->sax = (sax_type *) &(((current_fbl_node->sax_records[k]))[i*index->settings->paa_segments]);
                r->position = (file_position_type *) &((file_position_type *)(current_fbl_node->pos_records[k]))[i];
                r->insertion_mode = NO_TMP | PARTIAL;
                // Add record to index

                sax_type* saxpointer=(sax_type *)current_fbl_node->sax_records[k];

                add_record_to_node(index, current_fbl_node->node, r, 1);
            }
        }

        if (have_record) 
        {
            // flush index node
            //COUNT_CAL_TIME_START
            flush_subtree_leaf_buffers_m(index, current_fbl_node->node,lock_index,lock_write);

            //COUNT_CAL_TIME_END
            // clear FBL records moved in LBL buffers
            for (int k = fbloffset; k < fbloffset+preworkernumber; k++)
            {   
                if(current_fbl_node->sax_records[k]!=NULL)
                {
                    free((current_fbl_node->sax_records[k]));
                    free((current_fbl_node->pos_records[k]));
                    current_fbl_node->sax_records[k]=NULL;
                    current_fbl_node->pos_records[k]=NULL;
                }
  
                current_fbl_node->buffer_size[k] = 0;
                current_fbl_node->max_buffer_size[k] = 0;
            }

            // clear records read from files (free only prev sax buffers)

            isax_index_clear_node_buffers(index, current_fbl_node->node, 
                                          INCLUDE_CHILDREN,
                                          TMP_AND_TS_CLEAN);

            index->allocated_memory = 0;
            // Set to 0 in order to re-allocate original space for buffers.

        }
        
    }
    COUNT_QUEUE_TIME_START
    free(r);
}
void* indexconstructionworker_2RecBuf(void *input)
{
    isax_index *index=((trans_fbl_input*)input)->index;
    pthread_mutex_t *lock_index=((trans_fbl_input*)input)->lock_index;
    pthread_mutex_t *lock_write=((trans_fbl_input*)input)->lock_write;
    int fbloffset=((trans_fbl_input*)input)->fbloffset;
    int j=0,c=1;
    bool have_record=false;

    int preworkernumber=((trans_fbl_input*)input)->preworkernumber;
    isax_node_record *r = malloc(sizeof(isax_node_record));
    while(!((trans_fbl_input*)input)->finished)
    { 
        pthread_barrier_wait(((trans_fbl_input*)input)->lock_barrier1);
        pthread_barrier_wait(((trans_fbl_input*)input)->lock_barrier2);
        while(1)
        {
        //printf("this is the construction worker");
        
            j=__sync_fetch_and_add(&(((trans_fbl_input*)input)->conternumber),1);
            //pthread_mutex_unlock(((trans_fbl_input*)input)->lock_fbl_conter);
            if(j>=index->fbl->number_of_buffers)
            break;
            parallel_fbl_soft_buffer *current_fbl_node = &((parallel_first_buffer_layer*)(index->fbl))->soft_buffers[j];
            if (!current_fbl_node->initialized)
            continue;
            int i;
            have_record=false;
        
        // For all records in this buffer 
        //COUNT_CAL_TIME_START
        if (current_fbl_node->buffer_size[fbloffset] > 0)
        {
        //for (i=((index_buffer_data*)transferdata)->bufferpresize[j]; i<current_fbl_node->buffer_size; i++) 
            for (i=((trans_fbl_input*)input)->buffersize[j+fbloffset*index->fbl->number_of_buffers]; i<current_fbl_node->buffer_size[fbloffset]; i++) 
            {
                r->sax = (sax_type *) &(((current_fbl_node->sax_records[fbloffset]))[i*index->settings->paa_segments]);
                r->position = (file_position_type *) &((file_position_type *)(current_fbl_node->pos_records[fbloffset]))[i];
                r->insertion_mode = NO_TMP | PARTIAL;
                // Add record to index
                add_record_to_node(index, current_fbl_node->node, r, 1);
            }
            ((trans_fbl_input*)input)->buffersize[j+fbloffset*index->fbl->number_of_buffers]=current_fbl_node->buffer_size[fbloffset];
        }
            //((index_buffer_data*)transferdata)->bufferpresize[j]=current_fbl_node->buffer_size;        
        }
    }
    COUNT_QUEUE_TIME_START
    free(r);
}

void* indexconstructionworker_2nRecBuf(void *input)
{
    isax_index *index=((trans_fbl_input*)input)->index;
    pthread_mutex_t *lock_index=((trans_fbl_input*)input)->lock_index;
    pthread_mutex_t *lock_write=((trans_fbl_input*)input)->lock_write;
    int fbloffset=0;
    int j=0,k=0,c=1;
    bool have_record=false;
    
    int preworkernumber=((trans_fbl_input*)input)->preworkernumber;
    isax_node_record *r = malloc(sizeof(isax_node_record));
    while(!((trans_fbl_input*)input)->finished)
    {   
        fbloffset=preworkernumber-fbloffset;
        //pthread_barrier_wait(((trans_fbl_input*)input)->lock_barrier1);
        pthread_barrier_wait(((trans_fbl_input*)input)->lock_barrier2);
        if(((trans_fbl_input*)input)->finished)
        break;
        
        while(1)
        {
        //printf("this is the construction worker");
        
            j=__sync_fetch_and_add(&(((trans_fbl_input*)input)->conternumber),1);
            //pthread_mutex_unlock(((trans_fbl_input*)input)->lock_fbl_conter);
            if(j>=index->fbl->number_of_buffers)
            break;

            parallel_dfbl_soft_buffer *current_fbl_node = &((parallel_dfirst_buffer_layer*)(index->fbl))->soft_buffers[j];
            if (!current_fbl_node->initialized)
            continue;

        
        // For all records in this buffer 
        //COUNT_CAL_TIME_START
        //for (i=((index_buffer_data*)transferdata)->bufferpresize[j]; i<current_fbl_node->buffer_size; i++) 

        for( k = fbloffset; k < fbloffset+preworkernumber; k++)
        {   
            for (int i=((trans_fbl_input*)input)->buffersize[j+k*index->fbl->number_of_buffers]; i<current_fbl_node->buffer_size[k]; i++) 
            {
                r->sax = (((current_fbl_node->sax_records[k]))[i]);
                r->position = (file_position_type *)(((current_fbl_node->pos_records[k]))[i]);
                r->insertion_mode = NO_TMP | PARTIAL;
                // Add record to index
                add_record_to_node(index, current_fbl_node->node, r, 1);
            }
            ((trans_fbl_input*)input)->buffersize[j+k*index->fbl->number_of_buffers]=current_fbl_node->buffer_size[k];
            
            }
        }

            //((index_buffer_data*)transferdata)->bufferpresize[j]=current_fbl_node->buffer_size;        
    }
    
    COUNT_QUEUE_TIME_START
    free(r);
}
root_mask_type isax_fbl_index_insert_m(isax_index *index, 
                                    sax_type * sax,
                                    file_position_type * pos, pthread_mutex_t *lock_record, pthread_mutex_t *lock_fbl, pthread_mutex_t *lock_cbl,pthread_mutex_t *lock_firstnode,pthread_mutex_t *lock_index,pthread_mutex_t *lock_disk)
{
    int i,t;
    int totalsize = index->settings->max_total_buffer_size;

    // Create mask for the first bit of the sax representation
    
    // Step 1: Check if there is a root node that represents the 
    //         current node's sax representation
    

    root_mask_type first_bit_mask = 0x00;       

    CREATE_MASK(first_bit_mask, index, sax);

    pthread_mutex_lock(&(lock_cbl[first_bit_mask%LOCK_SIZE])); 
    insert_to_fbl_m(index->fbl, sax, pos, first_bit_mask, index,lock_firstnode,lock_fbl);
    pthread_mutex_unlock(&(lock_cbl[first_bit_mask%LOCK_SIZE]));
    
    return first_bit_mask;
}
root_mask_type isax_fbl_index_insert_m_new(isax_index *index, 
                                    sax_type * sax,
                                    file_position_type * pos, pthread_mutex_t *lock_record, pthread_mutex_t *lock_fbl, pthread_mutex_t *lock_cbl,pthread_mutex_t *lock_firstnode,pthread_mutex_t *lock_index,pthread_mutex_t *lock_disk)
{
    int i,t;
    int totalsize = index->settings->max_total_buffer_size;

    // Create mask for the first bit of the sax representation
    
    // Step 1: Check if there is a root node that represents the 
    //         current node's sax representation
    

    root_mask_type first_bit_mask = 0x00;       

    CREATE_MASK(first_bit_mask, index, sax);

    pthread_mutex_lock(&(lock_cbl[first_bit_mask%LOCK_SIZE])); 
    insert_to_fbl_m_new(index->fbl, sax, pos, first_bit_mask, index,lock_firstnode,lock_fbl);
    pthread_mutex_unlock(&(lock_cbl[first_bit_mask%LOCK_SIZE]));
    
    return first_bit_mask;
}

root_mask_type isax_pRecBuf_index_insert(isax_index *index, 
                                    sax_type * sax,
                                    file_position_type * pos, pthread_mutex_t *lock_record,pthread_mutex_t *lock_firstnode,int workernumber,int total_workernumber)
{
    int i,t;
    int totalsize = index->settings->max_total_buffer_size;
    // Create mask for the first bit of the sax representation
    
    // Step 1: Check if there is a root node that represents the 
    //         current node's sax representation

    root_mask_type first_bit_mask = 0x00;       

    CREATE_MASK(first_bit_mask, index, sax);

    insert_to_pRecBuf((parallel_first_buffer_layer*)(index->fbl), sax, pos,first_bit_mask, index,lock_firstnode,workernumber,total_workernumber);


    return first_bit_mask;
}
root_mask_type isax_2pRecBuf_index_insert(isax_index *index, 
                                    sax_type * sax,
                                    file_position_type * pos, pthread_mutex_t *lock_record,pthread_mutex_t *lock_firstnode,int workernumber,int total_workernumber)
{
    int i,t;
    int totalsize = index->settings->max_total_buffer_size;
    // Create mask for the first bit of the sax representation
    
    // Step 1: Check if there is a root node that represents the 
    //         current node's sax representation

    root_mask_type first_bit_mask = 0x00;       

    CREATE_MASK(first_bit_mask, index, sax);
    //printf("the pos is %d\n",*pos);

    void* current_recode=((parallel_dfirst_buffer_layer*)(index->fbl))->hard_buffer;
    //if(current_recode==NULL),
    //memcpy(current_recode,sax,index->settings->sax_byte_size);
    //printf("the posewfewjewooifwjo %d\n",current_recode);
    memcpy(current_recode+*pos/sizeof(ts_type)/index->settings->timeseries_size*index->settings->sax_byte_size,sax,index->settings->sax_byte_size);
    memcpy(current_recode+*pos/sizeof(ts_type)/index->settings->timeseries_size*index->settings->sax_byte_size+index->settings->sax_byte_size,pos,index->settings->position_byte_size);
    insert_to_2pRecBuf((parallel_dfirst_buffer_layer*)(index->fbl), sax, pos,first_bit_mask, index,lock_firstnode,workernumber,total_workernumber);
    return first_bit_mask;
}

/*void isax_index_binary_file_pRecBuf_old(const char *ifilename, int ts_num, isax_index *index,int calculate_thread)
{
    fprintf(stderr, ">>> Indexing: %s\n", ifilename);
    FILE * ifile;
    
    ifile = fopen (ifilename,"rb");
    index_buffer_data *input_data=malloc(sizeof(index_buffer_data)*(calculate_thread-1));
    index_buffer_data transferdata;
    file_position_type *pos = malloc(sizeof(file_position_type));
    pthread_t threadid[calculate_thread-1];
    bool sax_fist_time_check = false;
    
    long long int i;
    int prev_flush_time=0,now_flush_time=0;
    //initial the locks
    pthread_mutex_t lock_record=PTHREAD_MUTEX_INITIALIZER,lockfbl=PTHREAD_MUTEX_INITIALIZER,lock_index=PTHREAD_MUTEX_INITIALIZER,
                    lock_firstnode=PTHREAD_MUTEX_INITIALIZER,lock_disk=PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_t *lockcbl;
    // set the thread on decided cpu
    destroy_fbl(index->fbl);
        index->fbl = initialize_pRecBuf(index->settings->initial_fbl_buffer_size,
                                pow(2, index->settings->paa_segments), 
                                index->settings->max_total_buffer_size+DISK_BUFFER_SIZE*(PROGRESS_CALCULATE_THREAD_NUMBER-1), index);


    

    for (i = 0; i < (calculate_thread-1); i++)
    {
        input_data[i].index=index;
        input_data[i].lock_fbl=&lockfbl;
        input_data[i].lock_record=&lock_record;
        input_data[i].lock_firstnode =&lock_firstnode;
        input_data[i].lock_index=&lock_index;
        input_data[i].blocid=i*read_block_length;
        input_data[i].lock_disk=&lock_disk;
        input_data[i].workernumber=i;
        input_data[i].total_workernumber=calculate_thread-1;
    }
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
    int j,conter=0;
    ts_type * ts     = malloc(sizeof(ts_type) * index->settings->timeseries_size*read_block_length*(calculate_thread-1));
    ts_type * ts1    = malloc(sizeof(ts_type) * index->settings->timeseries_size*read_block_length*(calculate_thread-1));
    ts_type * ts2;
    sax_type * saxv  =malloc(sizeof(sax_type) * index->settings->paa_segments*read_block_length*(calculate_thread-1));
    sax_type * saxv1 =malloc(sizeof(sax_type) * index->settings->paa_segments*read_block_length*(calculate_thread-1));
    sax_type * saxv2;
                


    index->settings->raw_filename = malloc(256);
    strcpy(index->settings->raw_filename, ifilename);

    #ifdef BENCHMARK    
    int percentage = (int) (ts_num / (file_position_type) 100);
    #endif
        
    *pos = ftell(ifile);
    COUNT_INPUT_TIME_START
    //pre read the data of first round
    fread(ts1, sizeof(ts_type), index->settings->timeseries_size*read_block_length*(calculate_thread-1), ifile);
    COUNT_INPUT_TIME_END
    if(ts_num>read_block_length*(calculate_thread-1))
    {
        for (i = read_block_length*(calculate_thread-1); i < ts_num; i+=read_block_length*(calculate_thread-1))
        {   
                //quan bu yunxing wan hou
            ts2   =ts;
            ts    =ts1;  
            ts1   =ts2;
            saxv2 =saxv;
            saxv  =saxv1;
            saxv1 =saxv2;
            //printf("pos is %lld\n",*pos);
            for ( j = 0; j < (calculate_thread-1); j++)
            {   
                input_data[j].pos        =*pos+index->settings->timeseries_size*sizeof(ts_type)*j*read_block_length;
                input_data[j].ts         =&(ts[index->settings->timeseries_size*j*read_block_length]);
                input_data[j].saxv       =saxv;
                input_data[j].fin_number =read_block_length;
            }
            //create multiple threads
            for (j = 0; j < (calculate_thread-1); j++)
            {
                pthread_create(&(threadid[j]),NULL,indexbulkloadingworker_pRecBuf,(void*)&(input_data[j]));
                
            }
            *pos = ftell(ifile);
            //pthread_mutex_lock(&lock_disk);
            COUNT_INPUT_TIME_START
            //read the data of next round
            fread(ts1, sizeof(ts_type), index->settings->timeseries_size*read_block_length*(calculate_thread-1), ifile);
            COUNT_INPUT_TIME_END
            //write the sax in disk (last round)
            if (sax_fist_time_check)
            {
                COUNT_OUTPUT_TIME_START
                fwrite(saxv1, index->settings->sax_byte_size, read_block_length*(calculate_thread-1), index->sax_file);
                COUNT_OUTPUT_TIME_END
            }
            else
            {
                sax_fist_time_check=true;
            }
            //wait for the finish of other threads
            for (j = 0; j < (calculate_thread-1); j++)
            {
                pthread_join(threadid[j],NULL);
            }
            now_flush_time=i/(index->settings->max_total_buffer_size);
            if(now_flush_time!=prev_flush_time)
            {
                COUNT_QUEUE_TIME_START
                indexconstruction_pRecBuf(index->fbl, index,&lock_index,calculate_thread);
                COUNT_QUEUE_TIME_END
            }
            prev_flush_time=now_flush_time;
        }
    }

    ts2   =ts;
    ts    =ts1;
    ts1   =ts2;
    saxv2 =saxv;
    saxv  =saxv1;
    saxv1 =saxv2;
        //handle the rest data
    int conter_ts_number=ts_num%(read_block_length*(calculate_thread-1));
    int sax_save_number=conter_ts_number;
    for ( j = 0; j < (calculate_thread-1); j++)
    {   
        input_data[j].pos = *pos+index->settings->timeseries_size*sizeof(ts_type)*j*read_block_length;
        conter++;
        input_data[j].ts=&(ts[index->settings->timeseries_size*j*read_block_length]);
        input_data[j].fin_number=min(conter_ts_number,read_block_length);
        conter_ts_number=conter_ts_number-read_block_length;
        pthread_create(&(threadid[j]),NULL,indexbulkloadingworker_pRecBuf,(void*)&(input_data[j]));
        if(conter_ts_number<0)
            break;
    }
    if (sax_fist_time_check)
    {
       // pthread_mutex_lock(&lock_disk);
        COUNT_OUTPUT_TIME_START
        //fwrite(saxv1, index->settings->sax_byte_size, read_block_length*(calculate_thread-1), index->sax_file);
        COUNT_OUTPUT_TIME_END
        //pthread_mutex_unlock(&lock_disk);
    }
    else
    {
        sax_fist_time_check=true;
    }
    for (j = 0; j < conter; j++)
    {
        pthread_join(threadid[j],NULL);
    }
    //pthread_mutex_lock(&lock_disk);
    COUNT_OUTPUT_TIME_START
    //fwrite(saxv, index->settings->sax_byte_size, sax_save_number, index->sax_file);
    COUNT_OUTPUT_TIME_END
    //pthread_mutex_unlock(&lock_disk);
    COUNT_QUEUE_TIME_START
    indexconstruction_pRecBuf(index->fbl, index,&lock_index,calculate_thread);
    COUNT_QUEUE_TIME_END
    free(ts);
    free(ts1);
    free(input_data);
    free(lockcbl);
    free(saxv);
    free(saxv1);
    //free(sax);
    free(pos);
    COUNT_INPUT2_TIME_START
    fclose(ifile);
    COUNT_INPUT2_TIME_END
}*/


/*void isax_index_binary_file_mold(const char *ifilename, int ts_num, isax_index *index,int calculate_thread)
{
    fprintf(stderr, ">>> Indexing: %s\n", ifilename);
    FILE * ifile;
    
    ifile = fopen (ifilename,"rb");
    index_buffer_data *input_data=malloc(sizeof(index_buffer_data)*(calculate_thread-1));
    index_buffer_data transferdata;
    file_position_type *pos = malloc(sizeof(file_position_type));
    pthread_t threadid[calculate_thread-1];
    bool sax_fist_time_check = false;

    long long int i;
    int prev_flush_time=0,now_flush_time=0;
    //initial the locks
    pthread_mutex_t lock_record=PTHREAD_MUTEX_INITIALIZER,lockfbl=PTHREAD_MUTEX_INITIALIZER,lock_index=PTHREAD_MUTEX_INITIALIZER,
                    lock_firstnode=PTHREAD_MUTEX_INITIALIZER,lock_disk=PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_t *lockcbl;
    // set the thread on decided cpu

    lockcbl= malloc(sizeof(pthread_mutex_t)*LOCK_SIZE);

    for(i=0; i<LOCK_SIZE ;i++)
    {
        pthread_mutex_init(&lockcbl[i], NULL);
    }

    

    for (i = 0; i < (calculate_thread-1); i++)
    {
        input_data[i].index=index;
        input_data[i].lock_fbl=&lockfbl;
        input_data[i].lock_record=&lock_record;
        input_data[i].lock_cbl=lockcbl;
        input_data[i].lock_firstnode =&lock_firstnode;
        input_data[i].lock_index=&lock_index;
        input_data[i].blocid=i*read_block_length;
        input_data[i].lock_disk=&lock_disk;
    }
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
    int j,conter=0;
    ts_type * ts     = malloc(sizeof(ts_type) * index->settings->timeseries_size*read_block_length*(calculate_thread-1));
    ts_type * ts1    = malloc(sizeof(ts_type) * index->settings->timeseries_size*read_block_length*(calculate_thread-1));
    ts_type * ts2;
    sax_type * saxv  =malloc(sizeof(sax_type) * index->settings->paa_segments*read_block_length*(calculate_thread-1));
    sax_type * saxv1 =malloc(sizeof(sax_type) * index->settings->paa_segments*read_block_length*(calculate_thread-1));
    sax_type * saxv2;
                


    index->settings->raw_filename = malloc(256);
    strcpy(index->settings->raw_filename, ifilename);

    #ifdef BENCHMARK    
    int percentage = (int) (ts_num / (file_position_type) 100);
    #endif
        
    *pos = ftell(ifile);
    COUNT_INPUT_TIME_START
    //pre read the data of first round
    
    COUNT_INPUT_TIME_END
    if(ts_num>read_block_length*(calculate_thread-1))
    {
        fread(ts1, sizeof(ts_type), index->settings->timeseries_size*read_block_length*(calculate_thread-1), ifile);
        for (i = read_block_length*(calculate_thread-1); i < ts_num; i+=read_block_length*(calculate_thread-1))
        {   
                //quan bu yunxing wan hou
            ts2   =ts;
            ts    =ts1;  
            ts1   =ts2;
            saxv2 =saxv;
            saxv  =saxv1;
            saxv1 =saxv2;
            //printf("pos is %lld\n",*pos);
            for ( j = 0; j < (calculate_thread-1); j++)
            {   
                input_data[j].pos        =*pos+index->settings->timeseries_size*sizeof(ts_type)*j*read_block_length;
                input_data[j].ts         =&(ts[index->settings->timeseries_size*j*read_block_length]);
                input_data[j].saxv       =saxv;
                input_data[j].fin_number =read_block_length;
            }

            //create multiple threads
            for (j = 0; j < (calculate_thread-1); j++)
            {
                pthread_create(&(threadid[j]),NULL,indexbulkloadingworker,(void*)&(input_data[j]));
            }
            *pos = ftell(ifile);
           // pthread_mutex_lock(&lock_disk);
            COUNT_INPUT_TIME_START
            //read the data of next round
            fread(ts1, sizeof(ts_type), index->settings->timeseries_size*read_block_length*(calculate_thread-1), ifile);
            COUNT_INPUT_TIME_END
            //write the sax in disk (last round)
            if (sax_fist_time_check)
            {
                COUNT_OUTPUT_TIME_START
                fwrite(saxv1, index->settings->sax_byte_size, read_block_length*(calculate_thread-1), index->sax_file);
                COUNT_OUTPUT_TIME_END
            }
            else
            {
                sax_fist_time_check=true;
            }
       
            //wait for the finish of other threads
            for (j = 0; j < (calculate_thread-1); j++)
            {
                pthread_join(threadid[j],NULL);
            }
            now_flush_time=i/(index->settings->max_total_buffer_size);
            if(now_flush_time!=prev_flush_time)
            {
                COUNT_QUEUE_TIME_START
                indexconstruction(index->fbl, index,&lock_index,&lock_disk,calculate_thread);
                COUNT_QUEUE_TIME_END
            }
            prev_flush_time=now_flush_time;


        }
    }
    else
    {
        fread(ts1, sizeof(ts_type), index->settings->timeseries_size*ts_num, ifile);
    }
    ts2   =ts;
    ts    =ts1;
    ts1   =ts2;
    saxv2 =saxv;
    saxv  =saxv1;
    saxv1 =saxv2;
        //handle the rest data
    int conter_ts_number=ts_num%(read_block_length*(calculate_thread-1));
    int sax_save_number=conter_ts_number;
    for ( j = 0; j < (calculate_thread-1); j++)
    {   
        input_data[j].pos = *pos+index->settings->timeseries_size*sizeof(ts_type)*j*read_block_length;
        conter++;
        input_data[j].ts=&(ts[index->settings->timeseries_size*j*read_block_length]);
        input_data[j].fin_number=min(conter_ts_number,read_block_length);
        conter_ts_number=conter_ts_number-read_block_length;
        pthread_create(&(threadid[j]),NULL,indexbulkloadingworker,(void*)&(input_data[j]));
        if(conter_ts_number<0)
            break;
    }
    if (sax_fist_time_check)
    {
        pthread_mutex_lock(&lock_disk);
        COUNT_OUTPUT_TIME_START
        fwrite(saxv1, index->settings->sax_byte_size, read_block_length*(calculate_thread-1), index->sax_file);
        COUNT_OUTPUT_TIME_END
        pthread_mutex_unlock(&lock_disk);
    }
    else
    {
        sax_fist_time_check=true;
    }
    for (j = 0; j < conter; j++)
    {
        pthread_join(threadid[j],NULL);
    }
    pthread_mutex_lock(&lock_disk);
    COUNT_OUTPUT_TIME_START
    fwrite(saxv, index->settings->sax_byte_size, sax_save_number, index->sax_file);
    COUNT_OUTPUT_TIME_END
    pthread_mutex_unlock(&lock_disk);
    COUNT_QUEUE_TIME_START
    indexconstruction(index->fbl, index,&lock_index,&lock_disk,calculate_thread);
    COUNT_QUEUE_TIME_END
    free(ts);
    free(ts1);
    free(input_data);
    free(lockcbl);
    free(saxv);
    free(saxv1);
    //free(sax);
    free(pos);
    COUNT_INPUT2_TIME_START
    fclose(ifile);
    COUNT_INPUT2_TIME_END
}*/



/*void* indexbulkloadingworker_old(void *transferdata)
    {
    sax_type * sax = malloc(sizeof(sax_type) * ((index_buffer_data*)transferdata)->index->settings->paa_segments);

    int fin_number=((index_buffer_data*)transferdata)->fin_number;
    file_position_type *pos = malloc(sizeof(file_position_type));
    sax_type * saxv=(((index_buffer_data*)transferdata)->saxv);
    int offset_saxv=((index_buffer_data*)transferdata)->blocid*((index_buffer_data*)transferdata)->index->settings->paa_segments;
    int paa_segments=((index_buffer_data*)transferdata)->index->settings->paa_segments;
    isax_index *index= ((index_buffer_data*)transferdata)->index;
    int i=0;
    for (i=0;i<fin_number;i++)
    {
    #ifndef DEBUG
    #if VERBOSE_LEVEL == 2
        printf("\r\x1b[32mLoading: \x1b[36m%d\x1b[0m",(i + 1));
    #endif
    #endif
        if(sax_from_ts(((ts_type*)(((index_buffer_data*)transferdata)->ts)+i*index->settings->timeseries_size), sax, index->settings->ts_values_per_paa_segment,
                       index->settings->paa_segments, index->settings->sax_alphabet_cardinality,
                       index->settings->sax_bit_cardinality) == SUCCESS)
        {
            *pos= ((index_buffer_data*)transferdata)->pos+index->settings->timeseries_size*sizeof(ts_type)*i;
            
            //for(j=0;j<((index_buffer_data*)transferdata)->index->settings->paa_segments;j++)
            //{
                //saxv[offset_saxv+i*paa_segments+j]=sax[j];
                //printf("the sax [%d ]%d\n",i,(int)saxv[offset_saxv+i*paa_segments+j]);
                 //printf("the sax [%d ]%d\n",i,(int)sax[j]);
            //}
                        
            memcpy(&(saxv[offset_saxv+i*paa_segments]), sax, sizeof(sax_type) * paa_segments);
        

            #ifdef CLUSTERED
            root_mask_type first_bit_mask = 0x00;
            * pos=(index_buffer_data)transferdata->pos+index->settings->timeseries_size*sizeof(ts_type)*i;
            CREATE_MASK(first_bit_mask, index, sax);
            char* pfilename = malloc(255);
            snprintf(pfilename, 255, "%s.%llu",index->settings->raw_filename,first_bit_mask);
            FILE *pfile = fopen(pfilename, "a+");
            * pos=(index_buffer_data)transferdata->pos+index->settings->timeseries_size*sizeof(ts_type)*i;
            //
            fwrite(ts, sizeof(ts_type), index->settings->timeseries_size, pfile);
            //
            fclose(pfile);
            free(pfilename);
            #endif
            
            //printf("the pos is %lld\n",*pos);
            //isax_fbl_index_insert(index, sax, pos);
            isax_fbl_index_insert_m(index, sax, pos, ((index_buffer_data*)transferdata)->lock_record, ((index_buffer_data*)transferdata)->lock_fbl,
                                    ((index_buffer_data*)transferdata)->lock_cbl,((index_buffer_data*)transferdata)->lock_firstnode,((index_buffer_data*)transferdata)->lock_index,((index_buffer_data*)transferdata)->lock_disk);
        }
        else
        {
            fprintf(stderr, "error: cannot insert record in index, since sax representation\
                    failed to be created");
        }


    }
    free(pos);
    free(sax);
}*/