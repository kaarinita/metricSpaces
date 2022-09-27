//
//  isax_file_loaders.c
//  isax
//
//  Created by Kostas Zoumpatianos on 4/7/12.
//  Copyright 2012 University of Trento. All rights reserved.
//


#include "../../config.h"
#include "../../globals.h"
#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <inttypes.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdbool.h>
#include <float.h>
#include <unistd.h>
#include <math.h>

#include "ads/isax_node.h"
#include "ads/isax_index.h"
#include "ads/isax_query_engine.h"
#include "ads/isax_node_record.h"
#include "ads/isax_file_loaders.h"
#include "ads/isax_first_buffer_layer.h"
#include "ads/inmemory_index_engine.h"
void isax_query_binary_file(const char *ifilename, int q_num, isax_index *index,
                            float minimum_distance, int min_checked_leaves,
                            query_result (*search_function)(ts_type*, ts_type*, isax_index*, float, int)) 
{
	fprintf(stderr, ">>> Performing queries in file: %s\n", ifilename);
	FILE * ifile;
	ifile = fopen (ifilename,"rb");
    if (ifile == NULL) {
        fprintf(stderr, "File %s not found!\n", ifilename);
        exit(-1);
    }

    fseek(ifile, 0L, SEEK_END);
    file_position_type sz = (file_position_type) ftell(ifile);
    file_position_type total_records = sz/index->settings->ts_byte_size;
    fseek(ifile, 0L, SEEK_SET);
    if (total_records < q_num) {
        fprintf(stderr, "File %s has only %llu records!\n", ifilename, total_records);
        exit(-1);
    }

	int q_loaded = 0;
    ts_type * ts = malloc(sizeof(ts_type) * index->settings->timeseries_size);
    ts_type * paa = malloc(sizeof(ts_type) * index->settings->paa_segments);
    //sax_type * sax = malloc(sizeof(sax_type) * index->settings->paa_segments);
    while (q_loaded < q_num)
    {
        COUNT_INPUT_TIME_START
        fread(ts, sizeof(ts_type),index->settings->timeseries_size,ifile);
        COUNT_INPUT_TIME_END
        //printf("Querying for: %d\n", index->settings->ts_byte_size * q_loaded);
        // Parse ts and make PAA representation
        paa_from_ts(ts, paa, index->settings->paa_segments,
                    index->settings->ts_values_per_paa_segment);
        COUNT_TOTAL_TIME_START
        //COUNT_OUTPUT2_TIME_START
        query_result result = search_function(ts, paa, index, minimum_distance, min_checked_leaves);
        //COUNT_OUTPUT2_TIME_END
        COUNT_TOTAL_TIME_END
        PRINT_STATS(result.distance)
        
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
    free(paa);
    free(ts);
	fclose(ifile);
	fprintf(stderr, ">>> Finished querying.\n");

}
void isax_query_binary_file_traditional(const char *ifilename, int q_num, isax_index *index,
                            float minimum_distance, int min_checked_leaves,
                            query_result (*search_function)(ts_type*, ts_type*, isax_index*,node_list*, float, int)) 
{
    fprintf(stderr, ">>> Performing queries in file: %s\n", ifilename);
    FILE * ifile;
    ifile = fopen (ifilename,"rb");
    if (ifile == NULL) {
        fprintf(stderr, "File %s not found!\n", ifilename);
        exit(-1);
    }

    fseek(ifile, 0L, SEEK_END);
    file_position_type sz = (file_position_type) ftell(ifile);
    file_position_type total_records = sz/index->settings->ts_byte_size;
    fseek(ifile, 0L, SEEK_SET);
    if (total_records < q_num) {
        fprintf(stderr, "File %s has only %llu records!\n", ifilename, total_records);
        exit(-1);
    }

    int q_loaded = 0; 
    ts_type * ts = malloc(sizeof(ts_type) * index->settings->timeseries_size);
    ts_type * paa = malloc(sizeof(ts_type) * index->settings->paa_segments);
    //sax_type * sax = malloc(sizeof(sax_type) * index->settings->paa_segments);

    node_list nodelist;
    nodelist.nlist=malloc(sizeof(isax_node*)*pow(2, index->settings->paa_segments));
    nodelist.node_amount=0;
    isax_node *current_root_node = index->first_node;
    while(1)
    {
        if (current_root_node!=NULL)
        {
            nodelist.nlist[nodelist.node_amount]=current_root_node;
            current_root_node=current_root_node->next;
            nodelist.node_amount++;
        }
        else
        {
            break;
        }
                    
    }
    //printf("the node node_amount is %d\n",nodelist.node_amount );
                
    while (q_loaded < q_num)
    {
        COUNT_INPUT_TIME_START
        fread(ts, sizeof(ts_type),index->settings->timeseries_size,ifile);
        COUNT_INPUT_TIME_END
        //printf("Querying for: %d\n", index->settings->ts_byte_size * q_loaded);
        // Parse ts and make PAA representation
        paa_from_ts(ts, paa, index->settings->paa_segments,
                    index->settings->ts_values_per_paa_segment);
        COUNT_TOTAL_TIME_START
        query_result result = search_function(ts, paa, index,&nodelist, minimum_distance, min_checked_leaves);
        COUNT_TOTAL_TIME_END
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
    free(nodelist.nlist);
    free(paa);
    free(ts);
    fclose(ifile);
    fprintf(stderr, ">>> Finished querying.\n");

}
void isax_query_binary_fixbsf_file(const char *ifilename, int q_num, isax_index *index,
                            float minimum_distance, int min_checked_leaves,
                            query_result (*search_function)(ts_type*, ts_type*, isax_index*, float, int,float)) 
{
    fprintf(stderr, ">>> Performing queries in file: %s\n", ifilename);

    FILE * ifile;
    ifile = fopen (ifilename,"rb");
    if (ifile == NULL) {
        fprintf(stderr, "File %s not found!\n", ifilename);
        exit(-1);
    }

    fseek(ifile, 0L, SEEK_END);
    file_position_type sz = (file_position_type) ftell(ifile);
    file_position_type total_records = sz/index->settings->ts_byte_size;
    fseek(ifile, 0L, SEEK_SET);
    if (total_records < q_num) {
        fprintf(stderr, "File %s has only %llu records!\n", ifilename, total_records);
        exit(-1);
    }

    int q_loaded = 0;
    ts_type * ts = malloc(sizeof(ts_type) * index->settings->timeseries_size);
    ts_type * paa = malloc(sizeof(ts_type) * index->settings->paa_segments);
    //sax_type * sax = malloc(sizeof(sax_type) * index->settings->paa_segments);

    while (q_loaded < q_num)
    {
        COUNT_INPUT_TIME_START
        fread(ts, sizeof(ts_type),index->settings->timeseries_size,ifile);
        COUNT_INPUT_TIME_END
        //printf("Querying for: %d\n", index->settings->ts_byte_size * q_loaded);
        // Parse ts and make PAA representation
        paa_from_ts(ts, paa, index->settings->paa_segments,
                    index->settings->ts_values_per_paa_segment);
        query_result bsf=search_function(ts, paa, index, minimum_distance, min_checked_leaves,FLT_MAX);
        COUNT_TOTAL_TIME_START
        query_result result = search_function(ts, paa, index, minimum_distance, min_checked_leaves,bsf.distance);
        COUNT_TOTAL_TIME_END
        PRINT_STATS(result.distance)
        
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
    free(paa);
    free(ts);
    fclose(ifile);
    fprintf(stderr, ">>> Finished querying.\n");

}
void isax_topk_query_binary_file(const char *ifilename, int q_num, isax_index *index,
                            float minimum_distance, int min_checked_leaves,int k,
                            pqueue_bsf (*search_function)(ts_type*, ts_type*, isax_index*, float, int, int)) 
{
    fprintf(stderr, ">>> Performing queries in file: %s\n", ifilename);

    FILE * ifile;
    ifile = fopen (ifilename,"rb");
    if (ifile == NULL) {
        fprintf(stderr, "File %s not found!\n", ifilename);
        exit(-1);
    }
    fseek(ifile, 0L, SEEK_END);
    file_position_type sz = (file_position_type) ftell(ifile);
    file_position_type total_records = sz/index->settings->ts_byte_size;
    fseek(ifile, 0L, SEEK_SET);
    if (total_records < q_num) {
        fprintf(stderr, "File %s has only %llu records!\n", ifilename, total_records);
        exit(-1);
    }

    int q_loaded = 0;
    ts_type * ts = malloc(sizeof(ts_type) * index->settings->timeseries_size);
    ts_type * paa = malloc(sizeof(ts_type) * index->settings->paa_segments);
    //sax_type * sax = malloc(sizeof(sax_type) * index->settings->paa_segments);

    while (q_loaded < q_num)
    {
        COUNT_INPUT_TIME_START
        fread(ts, sizeof(ts_type),index->settings->timeseries_size,ifile);
        COUNT_INPUT_TIME_END
        //printf("Querying for: %d\n", index->settings->ts_byte_size * q_loaded);
        // Parse ts and make PAA representation
        paa_from_ts(ts, paa, index->settings->paa_segments,
                    index->settings->ts_values_per_paa_segment);
        COUNT_TOTAL_TIME_START
        COUNT_OUTPUT2_TIME_START
        pqueue_bsf result = search_function(ts, paa, index, minimum_distance, min_checked_leaves,k);
        COUNT_OUTPUT2_TIME_END
        COUNT_TOTAL_TIME_END
        for (int i = 0; i < result.k; i++)
        {
            printf(" the [%d] query [%d] NN is %f at %ld\n",q_loaded,i,result.knn[i],result.position[i]);
        }
        PRINT_STATS(result.knn[result.k-1])
        fflush(stdout);

        //sax_from_paa(paa, sax, index->settings->paa_segments, index->settings->sax_alphabet_cardinality, index->settings->sax_bit_cardinality);
        //if (index->settings->timeseries_size * sizeof(ts_type) * q_loaded == 1024) {
        //    sax_print(sax, index->settings->paa_segments, index->settings->sax_bit_cardinality);
        //}
        q_loaded++;
    }
    free(paa);
    free(ts);
    fclose(ifile);
    fprintf(stderr, ">>> Finished querying.\n");

}
void isax_knn_query_binary_file(const char *ifilename,const char *labelfilename, int q_num, isax_index *index,
                            float minimum_distance, int min_checked_leaves,int k,long int classlength,
                            pqueue_bsf (*search_function)(ts_type*, ts_type*, isax_index*, float, int, int)) 
{
    fprintf(stderr, ">>> Performing queries in file: %s and label in file %s\n", ifilename,labelfilename);

    FILE *ifile ,*lfile ;
    ifile = fopen (ifilename,"rb");
    lfile = fopen (labelfilename,"rb");
    long int *datalabel;
    if (ifile == NULL) {
        fprintf(stderr, "File %s not found!\n", ifilename);
        exit(-1);
    }
    if (lfile == NULL) {
        fprintf(stderr, "File %s not found!\n", labelfilename);
        exit(-1);
    }

    fseek(ifile, 0L, SEEK_END);
    file_position_type sz = (file_position_type) ftell(ifile);
    file_position_type total_records = sz/index->settings->ts_byte_size;
    fseek(ifile, 0L, SEEK_SET);

    if (total_records < q_num) {
        fprintf(stderr, "File %s has only %llu records!\n", ifilename, total_records);
        exit(-1);
    }
    datalabel=malloc(sizeof(long int) * index->sax_cache_size);
    fread(datalabel, sizeof(long int),index->sax_cache_size,lfile);


    int q_loaded = 0;
    ts_type * ts = malloc(sizeof(ts_type) * index->settings->timeseries_size);
    ts_type * paa = malloc(sizeof(ts_type) * index->settings->paa_segments);
    //sax_type * sax = malloc(sizeof(sax_type) * index->settings->paa_segments);

    while (q_loaded < q_num)
    {
        COUNT_INPUT_TIME_START
        fread(ts, sizeof(ts_type),index->settings->timeseries_size,ifile);
        COUNT_INPUT_TIME_END
        //printf("Querying for: %d\n", index->settings->ts_byte_size * q_loaded);
        // Parse ts and make PAA representation
        paa_from_ts(ts, paa, index->settings->paa_segments,
                    index->settings->ts_values_per_paa_segment);
        COUNT_TOTAL_TIME_START
        COUNT_OUTPUT2_TIME_START
        pqueue_bsf result = search_function(ts, paa, index, minimum_distance, min_checked_leaves,k);
        COUNT_OUTPUT2_TIME_END
        COUNT_TOTAL_TIME_END
        long int *classcounter=malloc(sizeof(long int) * classlength);
        long int classeposition=0,classtmp=0;
        for (int i = 0; i < classlength; i++)
        {
            classcounter[i]=0;
        }
        for (int i = 0; i < result.k; i++)
        {
            //printf("datalabel[result.position[i]] is %ld\n",datalabel[result.position[i]] );
            classcounter[datalabel[result.position[i]]]++;
            //printf("the class conter is %ld\n",classcounter[datalabel[result.position[i]]] );
            if(classtmp<classcounter[datalabel[result.position[i]]])
            {
                classtmp=classcounter[datalabel[result.position[i]]];
                classeposition=datalabel[result.position[i]];
 
            }
            //printf(" the [%d] query [%d] NN is %f at %ld label is %ld and now classeposition is %ld\n",q_loaded,i,result.knn[i],result.position[i],datalabel[result.position[i]],classeposition);
        }
        printf(" the [%d] query's label is %ld \n",q_loaded,classeposition);

        //PRINT_STATS(result.knn[result.k-1])
        fflush(stdout);

        //sax_from_paa(paa, sax, index->settings->paa_segments, index->settings->sax_alphabet_cardinality, index->settings->sax_bit_cardinality);
        //if (index->settings->timeseries_size * sizeof(ts_type) * q_loaded == 1024) {
        //    sax_print(sax, index->settings->paa_segments, index->settings->sax_bit_cardinality);
        //}
        q_loaded++;
        free(classcounter);
    }
    free(paa);
    free(ts);
    fclose(ifile);
    fprintf(stderr, ">>> Finished querying.\n");

}


void isax_knn_query_binary_file_traditional(const char *ifilename,const char *labelfilename, int q_num, isax_index *index,
                            float minimum_distance, int min_checked_leaves,int k,long int classlength,
                            pqueue_bsf (*search_function)(ts_type*, ts_type*, isax_index*,node_list*, float, int, int)) 
{
    fprintf(stderr, ">>> Performing queries in file: %s and label in file %s\n", ifilename,labelfilename);

    FILE *ifile ,*lfile ;
    ifile = fopen (ifilename,"rb");
    lfile = fopen (labelfilename,"rb");
    long int *datalabel;
    if (ifile == NULL) {
        fprintf(stderr, "File %s not found!\n", ifilename);
        exit(-1);
    }
    if (lfile == NULL) {
        fprintf(stderr, "File %s not found!\n", labelfilename);
        exit(-1);
    }

    fseek(ifile, 0L, SEEK_END);
    file_position_type sz = (file_position_type) ftell(ifile);
    file_position_type total_records = sz/index->settings->ts_byte_size;
    fseek(ifile, 0L, SEEK_SET);

    if (total_records < q_num) {
        fprintf(stderr, "File %s has only %llu records!\n", ifilename, total_records);
        exit(-1);
    }
    datalabel=malloc(sizeof(long int) * index->sax_cache_size);
    fread(datalabel, sizeof(long int),index->sax_cache_size,lfile);


    int q_loaded = 0;
    ts_type * ts = malloc(sizeof(ts_type) * index->settings->timeseries_size);
    ts_type * paa = malloc(sizeof(ts_type) * index->settings->paa_segments);
    //sax_type * sax = malloc(sizeof(sax_type) * index->settings->paa_segments);
    node_list nodelist;
    nodelist.nlist=malloc(sizeof(isax_node*)*pow(2, index->settings->paa_segments));
    nodelist.node_amount=0;
    isax_node *current_root_node = index->first_node;
    while(1)
    {
        if (current_root_node!=NULL)
        {
            nodelist.nlist[nodelist.node_amount]=current_root_node;
            current_root_node=current_root_node->next;
            nodelist.node_amount++;
        }
        else
        {
            break;
        }
                    
    }
    
    while (q_loaded < q_num)
    {
        COUNT_INPUT_TIME_START
        fread(ts, sizeof(ts_type),index->settings->timeseries_size,ifile);
        COUNT_INPUT_TIME_END
        //printf("Querying for: %d\n", index->settings->ts_byte_size * q_loaded);
        // Parse ts and make PAA representation
        paa_from_ts(ts, paa, index->settings->paa_segments,
                    index->settings->ts_values_per_paa_segment);
        COUNT_TOTAL_TIME_START
        //COUNT_OUTPUT2_TIME_START
        pqueue_bsf result = search_function(ts, paa, index,&nodelist, minimum_distance, min_checked_leaves,k);
        //COUNT_OUTPUT2_TIME_END
        COUNT_TOTAL_TIME_END
        long int *classcounter=malloc(sizeof(long int) * classlength);
        long int classeposition=0,classtmp=0;
        for (int i = 0; i < classlength; i++)
        {
            classcounter[i]=0;
        }
        for (int i = 0; i < result.k; i++)
        {
            printf("datalabel[result.position[i]] is %ld\n",datalabel[result.position[i]] );
            classcounter[datalabel[result.position[i]]]++;
            //printf("the class conter is %ld\n",classcounter[datalabel[result.position[i]]] );
            if(classtmp<classcounter[datalabel[result.position[i]]])
            {
                classtmp=classcounter[datalabel[result.position[i]]];
                classeposition=datalabel[result.position[i]];
 
            }
            //printf(" the [%d] query [%d] NN is %f at %ld label is %ld and now classeposition is %ld\n",q_loaded,i,result.knn[i],result.position[i],datalabel[result.position[i]],classeposition);
        }
        printf(" the [%d] query's label is %ld \n",q_loaded,classeposition);

        //PRINT_STATS(result.knn[result.k-1])
        fflush(stdout);

        //sax_from_paa(paa, sax, index->settings->paa_segments, index->settings->sax_alphabet_cardinality, index->settings->sax_bit_cardinality);
        //if (index->settings->timeseries_size * sizeof(ts_type) * q_loaded == 1024) {
        //    sax_print(sax, index->settings->paa_segments, index->settings->sax_bit_cardinality);
        //}
        q_loaded++;
        free(classcounter);
    }
    free(paa);
    free(ts);
    fclose(ifile);
    fprintf(stderr, ">>> Finished querying.\n");

}



void isax_query_binary_file_batch(const char *ifilename, int q_num, isax_index *index,
                            float minimum_distance, int min_checked_leaves,
                            void (*search_function)(ts_type*, ts_type*, isax_index*, float, int,int)) 
{
    fprintf(stderr, ">>> Performing queries in file: %s\n", ifilename);

    FILE * ifile;
    ifile = fopen (ifilename,"rb");
    if (ifile == NULL) {
        fprintf(stderr, "File %s not found!\n", ifilename);
        exit(-1);
    }

    fseek(ifile, 0L, SEEK_END);
    file_position_type sz = (file_position_type) ftell(ifile);
    file_position_type total_records = sz/index->settings->ts_byte_size;
    fseek(ifile, 0L, SEEK_SET);
    if (total_records < q_num) {
        fprintf(stderr, "File %s has only %llu records!\n", ifilename, total_records);
        exit(-1);
    }

    int q_loaded = 0;
    ts_type * ts = malloc(sizeof(ts_type) * index->settings->timeseries_size*q_num);
    ts_type * paa = malloc(sizeof(ts_type) * index->settings->paa_segments*q_num);
    //sax_type * sax = malloc(sizeof(sax_type) * index->settings->paa_segments);
    fread(ts, sizeof(ts_type),index->settings->timeseries_size*q_num,ifile);
    while (q_loaded < q_num)
    {
        //printf("Querying for: %d\n", index->settings->ts_byte_size * q_loaded);
        // Parse ts and make PAA representation
        paa_from_ts(&(ts[index->settings->timeseries_size*q_loaded]), &(paa[index->settings->paa_segments*q_loaded]), index->settings->paa_segments,
                    index->settings->ts_values_per_paa_segment);

        
        
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

    COUNT_TOTAL_TIME_START
    COUNT_OUTPUT2_TIME_START
    search_function(ts, paa, index, minimum_distance, min_checked_leaves,q_num);
    COUNT_OUTPUT2_TIME_END
    COUNT_TOTAL_TIME_END
    free(paa);
    free(ts);
    fclose(ifile);
    fprintf(stderr, ">>> Finished querying.\n");

}



void isax_index_binary_file(const char *ifilename, int ts_num, isax_index *index)
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


    int ts_loaded = 0;

    ts_type * ts = malloc(sizeof(ts_type) * index->settings->timeseries_size);
    sax_type * sax = malloc(sizeof(sax_type) * index->settings->paa_segments);
    file_position_type * pos = malloc(sizeof(file_position_type));

    index->settings->raw_filename = malloc(256);
    strcpy(index->settings->raw_filename, ifilename);

#ifdef BENCHMARK
    int percentage = (int) (ts_num / (file_position_type) 100);
#endif

    while (ts_loaded<ts_num)
    {
#ifndef DEBUG
#if VERBOSE_LEVEL == 2
        printf("\r\x1b[32mLoading: \x1b[36m%d\x1b[0m",(ts_loaded + 1));
#endif
#endif
        *pos = ftell(ifile);
        COUNT_INPUT_TIME_START
        int read_number=fread(ts, sizeof(ts_type), index->settings->timeseries_size, ifile);
        //printf("the pos is %lld\n",*pos);
        COUNT_INPUT_TIME_END
        
        if(sax_from_ts(ts, sax, index->settings->ts_values_per_paa_segment,
                       index->settings->paa_segments, index->settings->sax_alphabet_cardinality,
                       index->settings->sax_bit_cardinality) == SUCCESS)
        {
    #ifdef CLUSTERED
		    root_mask_type first_bit_mask = 0x00;
		    CREATE_MASK(first_bit_mask, index, sax);
            char* pfilename = malloc(255);
            snprintf(pfilename, 255, "%s.%llu",index->settings->raw_filename,first_bit_mask);
            FILE *pfile = fopen(pfilename, "a+");
            *pos = ftell(pfile);
            fwrite(ts, sizeof(ts_type), index->settings->timeseries_size, pfile);
            fclose(pfile);
            free(pfilename);
    #endif
            isax_fbl_index_insert(index, sax, pos);
            ts_loaded++;

            /*if(percentage == 0) {
			    float distance = 0;
		    	printf("%d ", ts_loaded);
			    PRINT_STATS(distance);
                fflush(stdout);
		    }
		    else if(ts_loaded % percentage == 0)
		    {
		    	float distance = 0;
		    	printf("%d ", ts_loaded);
                PRINT_STATS(distance)
                
                fflush(stdout);
		    }*/
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
    COUNT_INPUT_TIME_START
	fclose(ifile);
    COUNT_INPUT_TIME_END
    fprintf(stderr, ">>> Finished indexing\n");

}

/*
 *
** Using documented GCC type unsigned __int128 instead of undocumented
** obsolescent typedef name __uint128_t.  Works with GCC 4.7.1 but not
** GCC 4.1.2 (but __uint128_t works with GCC 4.1.2) on Mac OS X 10.7.4.
*/
typedef unsigned __int128 uint128_t;

/*      UINT64_MAX 18446744073709551615ULL */
#define P10_UINT64 10000000000000000000ULL   /* 19 zeroes */
#define E10_UINT64 19

#define STRINGIZER(x)   # x
#define TO_STRING(x)    STRINGIZER(x)

static int print_u128_u(uint128_t u128)
{
    int rc;
    if (u128 > UINT64_MAX)
    {
        uint128_t leading  = u128 / P10_UINT64;
        uint64_t  trailing = u128 % P10_UINT64;
        rc = print_u128_u(leading);
        rc += printf("%." TO_STRING(E10_UINT64) PRIu64, trailing);
    }
    else
    {
        uint64_t u64 = u128;
        rc = printf("%" PRIu64, u64);
    }
    return rc;
}

typedef struct
{
    uint64_t hi;
    uint64_t lo;
} my_uint128_t;

typedef struct {
	my_uint128_t sax;
	file_position_type pos;
} sax_vector;

/** Return returns 1 if a>b, -1 if b>a, 0 if a == b */
int
uint128_compare (const void * a_or, const void *b_or)
{
	my_uint128_t *a = (my_uint128_t*) &((sax_vector*)a_or)->sax;
	my_uint128_t *b = (my_uint128_t*) &((sax_vector*)b_or)->sax;

    if (a->hi > b->hi) return -1;
    if (a->hi < b->hi) return 1;
    if (a->lo > b->lo) return -1;
    if (a->lo < b->lo) return 1;
    return 0;
}


void isax_sorted_index_binary_file(const char *ifilename, int ts_num, isax_index *index)
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


    int ts_loaded = 0;

    ts_type * ts = malloc(sizeof(ts_type) * index->settings->timeseries_size);
    sax_vector * sax_vectors = malloc(sizeof(sax_vector) * ts_num);

    index->settings->raw_filename = malloc(256);
    strcpy(index->settings->raw_filename, ifilename);

#ifdef BENCHMARK
    int percentage = (int) (ts_num / (file_position_type) 100);
#endif

    while (ts_loaded<ts_num)
    {
#ifndef DEBUG
#if VERBOSE_LEVEL == 2
        printf("\r\x1b[32mLoading: \x1b[36m%d\x1b[0m",(ts_loaded + 1));
#endif
#endif
        file_position_type pos = ftell(ifile);
        COUNT_INPUT_TIME_START
        fread(ts, sizeof(ts_type), index->settings->timeseries_size, ifile);
        COUNT_INPUT_TIME_END

        if(sax_from_ts(ts, (sax_type*)&(sax_vectors[ts_loaded].sax), index->settings->ts_values_per_paa_segment,
                       index->settings->paa_segments, index->settings->sax_alphabet_cardinality,
                       index->settings->sax_bit_cardinality) == SUCCESS)
        {
#ifdef CLUSTERED
		    root_mask_type first_bit_mask = 0x00;
		    CREATE_MASK(first_bit_mask, index, sax);
            char* pfilename = malloc(255);
            snprintf(pfilename, 255, "%s.%llu",index->settings->raw_filename,first_bit_mask);
            FILE *pfile = fopen(pfilename, "a+");
            *pos = ftell(pfile);
            fwrite(ts, sizeof(ts_type), index->settings->timeseries_size, pfile);
            fclose(pfile);
            free(pfilename);
#endif

            sax_vectors[ts_loaded].pos = pos;

            ts_loaded++;

            if(percentage == 0) {
			    float distance = 0;
			    PRINT_STATS(distance);
		    }
		    else if(ts_loaded % percentage == 0)
		    {
		    	float distance = 0;
		    	PRINT_STATS(distance)
		    }
        }
        else
        {
            fprintf(stderr, "error: cannot insert record in index, since sax representation\
                    failed to be created");
        }
	}

    qsort(sax_vectors, sizeof(sax_vector), ts_num, uint128_compare);

    int i;
    for(i=0; i<ts_num; i++) {
    	isax_fbl_index_insert(index, (sax_type *) &(sax_vectors[i].sax), &(sax_vectors[i].pos));
    }

    free(ts);
    free(sax_vectors);
    COUNT_INPUT_TIME_START
	fclose(ifile);
    COUNT_INPUT_TIME_END
    fprintf(stderr, ">>> Finished indexing\n");

}


void isax_merge_sorted_index_binary_file(const char *ifilename, int ts_num, isax_index *index)
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


    int ts_loaded = 0;

    ts_type * ts = malloc(sizeof(ts_type) * index->settings->timeseries_size);
    sax_vector * sax_vectors = malloc(sizeof(sax_vector) * ts_num);

    index->settings->raw_filename = malloc(256);
    strcpy(index->settings->raw_filename, ifilename);

#ifdef BENCHMARK
    int percentage = (int) (ts_num / (file_position_type) 100);
#endif

    while (ts_loaded<ts_num)
    {
#ifndef DEBUG
#if VERBOSE_LEVEL == 2
        printf("\r\x1b[32mLoading: \x1b[36m%d\x1b[0m",(ts_loaded + 1));
#endif
#endif
        file_position_type pos = ftell(ifile);
        COUNT_INPUT_TIME_START
        fread(ts, sizeof(ts_type), index->settings->timeseries_size, ifile);
        COUNT_INPUT_TIME_END

        if(sax_from_ts(ts, (sax_type*)&(sax_vectors[ts_loaded].sax), index->settings->ts_values_per_paa_segment,
                       index->settings->paa_segments, index->settings->sax_alphabet_cardinality,
                       index->settings->sax_bit_cardinality) == SUCCESS)
        {
#ifdef CLUSTERED
		    root_mask_type first_bit_mask = 0x00;
		    CREATE_MASK(first_bit_mask, index, sax);
            char* pfilename = malloc(255);
            snprintf(pfilename, 255, "%s.%llu",index->settings->raw_filename,first_bit_mask);
            FILE *pfile = fopen(pfilename, "a+");
            *pos = ftell(pfile);
            fwrite(ts, sizeof(ts_type), index->settings->timeseries_size, pfile);
            fclose(pfile);
            free(pfilename);
#endif

            sax_vectors[ts_loaded].pos = pos;

            ts_loaded++;

            if(percentage == 0) {
			    float distance = 0;
			    PRINT_STATS(distance);
		    }
		    else if(ts_loaded % percentage == 0)
		    {
		    	float distance = 0;
		    	PRINT_STATS(distance)
		    }
        }
        else
        {
            fprintf(stderr, "error: cannot insert record in index, since sax representation\
                    failed to be created");
        }
	}

    //qsort(sax_vectors, sizeof(sax_vector), ts_num, uint128_compare);

    int j;
	unsigned long i;
	int segment;
	sax_type *removals = malloc(sizeof(sax_type) * index->settings->paa_segments);
	for(i=0; i<index->settings->paa_segments; i++) {
		removals[i] = 0;
	}


	int prev_best_segment = -1;
	float current_utilization = 0;
	sax_vector *new_dataset = malloc(sizeof(sax_vector) * ts_num);

	do {
		// If I remove 1 bit from a segment what is the average page utilization?
		int best_segment = prev_best_segment+1;
		if(best_segment >= index->settings->paa_segments) {
			best_segment = 0;
		}
		prev_best_segment = best_segment;
		int best_segment_pages = ts_num;
		float best_segment_average_utilization = 0;
		float best_segment_min_page_utilization = 0;
		char updated = 0;

		//printf("Starting with: %d\n", best_segment);
		for(segment=0; segment<index->settings->paa_segments; segment++) {

			for(i=0; i<ts_num; i++) {
				memcpy(&new_dataset[i], &sax_vectors[i], sizeof(sax_vector));
				sax_type *sax_cpy = (sax_type *) &(new_dataset[i].sax);
				int k;
				for(k=0; k<index->settings->paa_segments; k++) {
					if(k == segment) {
						sax_cpy[k] = (sax_cpy[k] >> (removals[k]+1));
					}
					else {
						sax_cpy[k] = (sax_cpy[k] >> removals[k]);
					}
				}
			}

			qsort (new_dataset, ts_num, sizeof(sax_vector), uint128_compare);

			sax_vector prev = new_dataset[0];
			int counter = 1;
			int pid = 1;
			float page_utilization = 0;
			float min_page_utilization = 10000000000;
			for(i=1; i<ts_num; i++) {
				uint128_t *prev_sax = (uint128_t *)&(prev.sax);
				uint128_t *this_sax = (uint128_t *)&(new_dataset[i].sax);
				if(*prev_sax != *this_sax) {
					float d = (float)(counter * index->settings->ts_byte_size) / 4096;
					page_utilization += d;
					counter = 1;
					pid++;
					if(page_utilization < min_page_utilization) {
						min_page_utilization = page_utilization;
					}
				}
				else {
					counter++;
				}
				prev = new_dataset[i];
			}

			float d = (float)(counter * index->settings->ts_byte_size) / (float) 4096;
			page_utilization += d;

			if(pid < best_segment_pages || (pid == best_segment_pages && segment == prev_best_segment))
			{
				updated = 1;
				best_segment = segment;
				best_segment_pages = pid;
				best_segment_average_utilization = page_utilization / (float) pid;
				best_segment_min_page_utilization = min_page_utilization;
			}


			if(!updated) {
				best_segment_pages = pid;
				best_segment_average_utilization = page_utilization / (float) pid;
				best_segment_min_page_utilization = min_page_utilization;
			}

			//printf("%d has %d\n", segment, pid);
		}

		//printf("Chosing: %d (%d pages %lf utilized on average, min: %lf)\n", best_segment, best_segment_pages, best_segment_average_utilization, best_segment_min_page_utilization);
		current_utilization = best_segment_average_utilization;
		removals[best_segment]++;
	} while(current_utilization < 1.2);



    for(i=0; i<ts_num; i++) {
        j = new_dataset[i].pos / index->settings->ts_byte_size;
    	isax_fbl_index_insert(index, (sax_type *) &(sax_vectors[j].sax), &(sax_vectors[j].pos));
    }


    free(new_dataset);
    free(removals);

    free(ts);
    free(sax_vectors);
    COUNT_INPUT_TIME_START
	fclose(ifile);
    COUNT_INPUT_TIME_END
    fprintf(stderr, ">>> Finished indexing\n");

}
