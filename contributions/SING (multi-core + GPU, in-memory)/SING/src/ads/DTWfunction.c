       //     lower_upper_lemire(new_record->ts,full_time_series_size,warWind,lowerLemire,upperLemire);
                    
                    
         //   paa_from_ts(new_record->ts, paa, numberOFSegmentQuery,
                  //      index->settings->ts_values_per_paa_segment);
 
           /// paa_from_ts(upperLemire, paaUpperLemQuery, numberOFSegmentQuery,
                //        index->settings->ts_values_per_paa_segment);
            //paa_from_ts(lowerLemire, paaLowerLemQuery, numberOFSegmentQuery,
              //          index->settings->ts_values_per_paa_segment);
       /*     ts_type * paaUpperLemQuery = malloc(sizeof(ts_type) * index->settings->paa_segments);
            ts_type * paaLowerLemQuery = malloc(sizeof(ts_type) * index->settings->paa_segments);
            
            ts_type * upperLemire = malloc(sizeof(ts_type) * full_time_series_size);
            ts_type * lowerLemire = malloc(sizeof(ts_type) * full_time_series_size);
            
            lower_upper_lemire(new_record->ts,full_time_series_size,warWind,lowerLemire,upperLemire);
                    
                    
            paa_from_ts(new_record->ts, paa, numberOFSegmentQuery,
                        index->settings->ts_values_per_paa_segment);
 
            paa_from_ts(upperLemire, paaUpperLemQuery, numberOFSegmentQuery,
                        index->settings->ts_values_per_paa_segment);
            paa_from_ts(lowerLemire, paaLowerLemQuery, numberOFSegmentQuery,
                        index->settings->ts_values_per_paa_segment);*/
#include "../../config.h"
#include "../../globals.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "immintrin.h"
#ifdef VALUES
	#include <values.h>
#endif

#include "ads/sax/sax.h"
#include "ads/sax/ts.h"
#include "ads/sax/sax_breakpoints.h"
#include "ads/isax_index.h"
#include "ads/inmemory_index_engine.h"
#include "ads/DTWfunction.h"
#define max(x,y) ((x)>(y)?(x):(y))
#define dist(x,y) ((x-y)*(x-y))
void init(deque *d, int capacity)
{
    d->capacity = capacity;
    d->size = 0;
    d->dq = (int *) malloc(sizeof(int)*d->capacity);
    d->f = 0;
    d->r = d->capacity-1;
}

/// Insert to the queue at the back
void push_back(struct deque *d, int v)
{
    d->dq[d->r] = v;
    d->r--;
    if (d->r < 0)
        d->r = d->capacity-1;
    d->size++;
}

/// Delete the current (front) element from queue
void pop_front(struct deque *d)
{
    d->f--;
    if (d->f < 0)
        d->f = d->capacity-1;
    d->size--;
}

/// Delete the last element from queue
void pop_back(struct deque *d)
{
    d->r = (d->r+1)%d->capacity;
    d->size--;
}

/// Get the value at the current position of the circular queue
int front(struct deque *d)
{
    int aux = d->f - 1;

    if (aux < 0)
        aux = d->capacity-1;
    return d->dq[aux];
}

/// Get the value at the last position of the circular queueint back(struct deque *d)
int back(struct deque *d)
{
    int aux = (d->r+1)%d->capacity;
    return d->dq[aux];
}

/// Check whether or not the queue is empty
int empty(struct deque *d)
{
    return d->size == 0;
}


/// Destroy the queue
void destroy(deque *d)
{
    free(d->dq);
}



void lower_upper_lemire(float *t, int len, int r, float *l, float *u)
{
    struct deque du, dl;

    init(&du, 2*r+2);
    init(&dl, 2*r+2);

    push_back(&du, 0);
    push_back(&dl, 0);
    int i;
    
    for (i = 1; i < len; i++)
    {
        if (i > r)
        {
            u[i-r-1] = t[front(&du)];
            l[i-r-1] = t[front(&dl)];
        }
        if (t[i] > t[i-1])
        {
            pop_back(&du);
            while (!empty(&du) && t[i] > t[back(&du)])
                pop_back(&du);
        }
        else
        {
            pop_back(&dl);
            while (!empty(&dl) && t[i] < t[back(&dl)])
                pop_back(&dl);
        }
        push_back(&du, i);
        push_back(&dl, i);
        if (i == 2 * r + 1 + front(&du))
            pop_front(&du);
        else if (i == 2 * r + 1 + front(&dl))
            pop_front(&dl);
    }
    for (i= len; i < len+r+1; i++)
    {
        u[i-r-1] = t[front(&du)];
        l[i-r-1] = t[front(&dl)];
        if (i-front(&du) >= 2 * r + 1)
            pop_front(&du);
        if (i-front(&dl) >= 2 * r + 1)
            pop_front(&dl);
    }
    destroy(&du);
    destroy(&dl);


}




float minidist_paa_to_isax_DTW(float *paaU, float *paaL , sax_type *sax, 
                           sax_type *sax_cardinalities,
                           sax_type max_bit_cardinality,
                           int max_cardinality,
                           int number_of_segments,
                           int min_val,
                           int max_val,
                           float ratio_sqrt) 
{
   
    float distance = 0;
    // TODO: Store offset in index settings. and pass index settings as parameter.
    
    int offset = ((max_cardinality - 1) * (max_cardinality - 2)) / 2;
    
    // For each sax record find the break point
    int i;
    for (i=0; i<number_of_segments; i++) {

        sax_type c_c = sax_cardinalities[i];

        sax_type c_m = max_bit_cardinality;
        sax_type v = sax[i];
        //sax_print(&v, 1, c_m);
        
        
        sax_type region_lower = (v <<  (c_m - c_c));
        sax_type region_upper = (~((int)MAXFLOAT << (c_m - c_c)) | region_lower);
		//printf("[%d, %d] %d -- %d\n", sax[i], c_c, region_lower, region_upper);
        float breakpoint_lower = 0; // <-- TODO: calculate breakpoints.
        float breakpoint_upper = 0; // <-- - || -
        
        
        if (region_lower == 0) {
            breakpoint_lower = min_val;
        }
        else
        {
            breakpoint_lower = sax_breakpoints[offset + region_lower - 1];
        }
        if (region_upper == max_cardinality - 1) {
            breakpoint_upper = max_val;
        }
        else
        {
            breakpoint_upper = sax_breakpoints[offset + region_upper];
        }
        //printf("\n%d.%d is from %d to %d, %lf - %lf\n", v, c_c, region_lower, region_upper,
        //       breakpoint_lower, breakpoint_upper);

        //printf("FROM: \n");
        //sax_print(&region_lower, 1, c_m);
        //printf("TO: \n");
        //sax_print(&region_upper, 1, c_m);
		
		//printf ("\n---------\n");
        
        if (breakpoint_lower > paaU[i]) {
            distance += pow(breakpoint_lower - paaU[i], 2);
        }
        else if(breakpoint_upper < paaL[i]) {
            distance += pow(breakpoint_upper - paaL[i], 2);
        }
    //        else {
    //            printf("%lf is between: %lf and %lf\n", paa[i], breakpoint_lower, breakpoint_upper);
    //        }
    }
    
    //distance = ratio_sqrt * sqrtf(distance);
    distance = ratio_sqrt * distance;
    return distance;
}



float   minidist_paa_to_isax_raw_DTW(float *paaU, float *paaL , sax_type *sax, 
                           sax_type *sax_cardinalities,
                           sax_type max_bit_cardinality,
                           int max_cardinality,
                           int number_of_segments,
                           int min_val,
                           int max_val,
                           float ratio_sqrt)
{
   
    float distance = 0;
    // TODO: Store offset in index settings. and pass index settings as parameter.
    
    int offset = ((max_cardinality - 1) * (max_cardinality - 2)) / 2;

    // For each sax record find the break point
    int i;
    for (i=0; i<number_of_segments; i++) {
        
        sax_type c_c = sax_cardinalities[i];
        sax_type c_m = max_bit_cardinality;
        sax_type v = sax[i];
        //sax_print(&v, 1, c_m);

        sax_type region_lower = (v >> (c_m - c_c)) <<  (c_m - c_c);//shift operation 
        sax_type region_upper = (~((int)MAXFLOAT << (c_m - c_c)) | region_lower);
		//printf("[%d, %d] %d -- %d\n", sax[i], c_c, region_lower, region_upper);

        float breakpoint_lower = 0; // <-- TODO: calculate breakpoints.
        float breakpoint_upper = 0; // <-- - || -
        
        
        if (region_lower == 0) {
            breakpoint_lower = min_val;
        }
        else
        {
            breakpoint_lower = sax_breakpoints[offset + region_lower - 1];
        }
        if (region_upper == max_cardinality - 1) {
            breakpoint_upper = max_val;
        }
        else
        {
            breakpoint_upper = sax_breakpoints[offset + region_upper];//search in a list(why?)
        }

        //printf("\n%d.%d is from %d to %d, %lf - %lf\n", v, c_c, region_lower, region_upper,
        //       breakpoint_lower, breakpoint_upper);

        //printf("FROM: \n");
        //sax_print(&region_lower, 1, c_m);
        //printf("TO: \n");
        //sax_print(&region_upper, 1, c_m);
		
		//printf ("\n---------\n");
        
        if (breakpoint_lower > paaU[i] ) {
            distance += pow(breakpoint_lower - paaU[i], 2);
        }
        else if(breakpoint_upper < paaL[i]) {
            distance += pow(breakpoint_upper - paaL[i], 2);
        }


//        else {
//            printf("%lf is between: %lf and %lf\n", paa[i], breakpoint_lower, breakpoint_upper);
//        }
    }
    
    //distance = ratio_sqrt * sqrtf(distance);
    distance = ratio_sqrt * distance;
    return distance;
}




void isax_DTWquery_binary_file_traditional(const char *ifilename, int q_num, isax_index *index,
                            float minimum_distance, int min_checked_leaves,int warpWind)
                            //)query_result (*search_function)(ts_type*, ts_type*, isax_index*,node_list*, float, int)) 
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
            ts_type * paaUpperLemQuery = malloc(sizeof(ts_type) * index->settings->paa_segments);
            ts_type * paaLowerLemQuery = malloc(sizeof(ts_type) * index->settings->paa_segments);
            
            ts_type * upperLemire = malloc(sizeof(ts_type) * index->settings->timeseries_size);
            ts_type * lowerLemire = malloc(sizeof(ts_type) * index->settings->timeseries_size);
            

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

            lower_upper_lemire(ts,index->settings->timeseries_size,warpWind,lowerLemire,upperLemire);
                    
                
            paa_from_ts(upperLemire, paaUpperLemQuery, index->settings->paa_segments,
                        index->settings->ts_values_per_paa_segment);
            paa_from_ts(lowerLemire, paaLowerLemQuery, index->settings->paa_segments,
                        index->settings->ts_values_per_paa_segment);


        //printf("Querying for: %d\n", index->settings->ts_byte_size * q_loaded);
        // Parse ts and make PAA representation
        paa_from_ts(ts, paa, index->settings->paa_segments,
                    index->settings->ts_values_per_paa_segment);
        COUNT_TOTAL_TIME_START
                query_result result = exact_DTW_serial_ParIS_inmemory(ts, paa,paaUpperLemQuery,paaLowerLemQuery, index, minimum_distance, min_checked_leaves,warpWind);

        //query_result result = exact_DTW_serial_ParIS_openmp_inmemory(ts, paa,paaUpperLemQuery,paaLowerLemQuery, index, minimum_distance, min_checked_leaves,warpWind);
       // query_result result = exact_DTW_MESSI_inmemory_hybrid(ts, paa,paaUpperLemQuery,paaLowerLemQuery, index,&nodelist, minimum_distance, min_checked_leaves,warpWind);

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
    free(nodelist.nlist);
    free(paa);
    free(ts);
    fclose(ifile);
    free(paaUpperLemQuery);
    free(paaLowerLemQuery);
    free(lowerLemire);
    free(upperLemire);
    fprintf(stderr, ">>> Finished querying.\n");

}


void isax_DTWknn_query_binary_file(const char *ifilename,const char *labelfilename, int q_num, isax_index *index,
                            float minimum_distance, int min_checked_leaves,int warpWind,int k,long int classlength,
                            pqueue_bsf (*search_function)(ts_type*,ts_type*, ts_type*,ts_type*, isax_index*, float, int,int, int)) 
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
                ts_type * paaUpperLemQuery = malloc(sizeof(ts_type) * index->settings->paa_segments);
            ts_type * paaLowerLemQuery = malloc(sizeof(ts_type) * index->settings->paa_segments);
            
            ts_type * upperLemire = malloc(sizeof(ts_type) * index->settings->timeseries_size);
            ts_type * lowerLemire = malloc(sizeof(ts_type) * index->settings->timeseries_size);
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
            lower_upper_lemire(ts,index->settings->timeseries_size,warpWind,lowerLemire,upperLemire);
                    
                
            paa_from_ts(upperLemire, paaUpperLemQuery, index->settings->paa_segments,
                        index->settings->ts_values_per_paa_segment);
            paa_from_ts(lowerLemire, paaLowerLemQuery, index->settings->paa_segments,
                        index->settings->ts_values_per_paa_segment);
        COUNT_TOTAL_TIME_START
        COUNT_OUTPUT2_TIME_START
        pqueue_bsf result = search_function(ts, paa,paaUpperLemQuery,paaLowerLemQuery, index, minimum_distance, min_checked_leaves,warpWind,k);
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


void isax_DTWknn_query_binary_file_traditional(const char *ifilename,const char *labelfilename, int q_num, isax_index *index,
                            float minimum_distance, int min_checked_leaves,int warpWind ,int k,long int classlength,
                            pqueue_bsf (*search_function)(ts_type*, ts_type*, ts_type*, ts_type*, isax_index*,node_list*, float,int, int, int)) 
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
            ts_type * paaUpperLemQuery = malloc(sizeof(ts_type) * index->settings->paa_segments);
            ts_type * paaLowerLemQuery = malloc(sizeof(ts_type) * index->settings->paa_segments);
            
            ts_type * upperLemire = malloc(sizeof(ts_type) * index->settings->timeseries_size);
            ts_type * lowerLemire = malloc(sizeof(ts_type) * index->settings->timeseries_size);
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
            lower_upper_lemire(ts,index->settings->timeseries_size,warpWind,lowerLemire,upperLemire);  
            paa_from_ts(upperLemire, paaUpperLemQuery, index->settings->paa_segments,
                        index->settings->ts_values_per_paa_segment);
            paa_from_ts(lowerLemire, paaLowerLemQuery, index->settings->paa_segments,
                        index->settings->ts_values_per_paa_segment);

        COUNT_TOTAL_TIME_START
        //COUNT_OUTPUT2_TIME_START
        pqueue_bsf result = search_function(ts,  paa,paaUpperLemQuery,paaLowerLemQuery, index,&nodelist, minimum_distance, min_checked_leaves,warpWind,k);
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
       // printf(" the [%d] query's label is %ld \n",q_loaded,classeposition);

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







float dtw(float* A, float* B,float *cb,int m, int r,  float bsf)
{

    float *cost;
    float *cost_prev;
    float *cost_tmp;
    int i,j,k;
    float x,y,z,min_cost;

    /// Instead of using matrix of size O(m^2) or O(mr), we will reuse two array of size O(r).
    cost = (float*)malloc(sizeof(float)*(2*r+1));
    for(k=0; k<2*r+1; k++)    cost[k]=FLT_MAX;

    cost_prev = (float*)malloc(sizeof(float)*(2*r+1));
    for(k=0; k<2*r+1; k++)    cost_prev[k]=FLT_MAX;

    for (i=0; i<m; i++)
    {
        k = max(0,r-i);
        min_cost = FLT_MAX;

        for(j=max(0,i-r); j<=min(m-1,i+r); j++, k++)
        {
            /// Initialize all row and column
            if ((i==0)&&(j==0))
            {
                cost[k]=dist(A[0],B[0]);
                min_cost = cost[k];
                continue;
            }

            if ((j-1<0)||(k-1<0))     y = FLT_MAX;
            else                      y = cost[k-1];
            if ((i-1<0)||(k+1>2*r))   x = FLT_MAX;
            else                      x = cost_prev[k+1];
            if ((i-1<0)||(j-1<0))     z = FLT_MAX;
            else                      z = cost_prev[k];

            /// Classic DTW calculation
            cost[k] = min( min( x, y) , z) + dist(A[i],B[j]);

            /// Find minimum cost in row for early abandoning (possibly to use column instead of row).
            if (cost[k] < min_cost)
            {   min_cost = cost[k];
            }
        }

        /// We can abandon early if the current cummulative distace with lower bound together are larger than bsf
        if (i+r < m-1 && min_cost + cb[i+r+1] >= bsf)
        {   free(cost);
            free(cost_prev);
            return min_cost + cb[i+r+1];
        }
        /// Move current array to previous array.
        cost_tmp = cost;
        cost = cost_prev;
        cost_prev = cost_tmp;
    }
    k--;

    /// the DTW distance is in the last cell in the matrix of size O(m^2) or at the middle of our array.
    float final_dtw = cost_prev[k];
    free(cost);
    free(cost_prev);
    return final_dtw;
}
float dtwsimd(float* A, float* B, float *cb, int m, int r, float bsf, float* tSum, float* pCost, float* rDist)
{
	int length = 2*r + 1;
	int start, end, k, ij, i;
	float minCost = 0.0f;
	for(k=0; k<=r; k++)
	{
		rDist[k] = dist(A[0],B[k]);
	}	
	tSum[0] = rDist[0];
	for(ij=1; ij<=r; ij++)
	{
		tSum[ij] = tSum[ij-1] + rDist[ij];
	}
	pCost[0] = tSum[0];
	for(ij=1; ij<=r; ij++)
		pCost[ij] = min(tSum[ij-1],tSum[ij]);
	pCost[r+1] = tSum[r];

	for(i=1; i<m-1; i++)
	{
		start = max(0,i-r);
		end = min(m-1,i+r);

		for(k=start; k<=end; k++)
		{
			rDist[k-start] = dist(A[i],B[k]);
			tSum[k-start] = pCost[k-start] + rDist[k-start];
		}
		minCost = tSum[0];
		for(k=start+1;k<=end;k++)
		{
			if(tSum[k-1-start]<pCost[k-start])
				tSum[k-start] = tSum[k-1-start] + rDist[k-start];
			if(tSum[k-start] < minCost)
				minCost = tSum[k-start];
		}	
		if(i+r<m-1&&minCost+cb[i+r+1] >= bsf)
		{
			return minCost + cb[i+r+1];
		}
		if((end-start+1)<length&&start==0)
		{
			pCost[start-start] = tSum[start-start];
			for(ij = start+1; ij<=end; ij++)
				pCost[ij-start] = min(tSum[ij-1-start],tSum[ij-start]);
			pCost[end+1-start] = tSum[end-start];
		}
		else
		{
			for(ij=start+1;ij<end;ij++)
				pCost[ij-1-start] = min(tSum[ij-1-start],tSum[ij-1-start]);
			pCost[end-start] = tSum[end-start];
		}
	}	


	for(int k=start; k<=end; k++)
	{
		rDist[k-start] = dist(A[m-1],B[k]);
		tSum[k-start] = pCost[k-start] + rDist[k-start];
	}
	for(k=start+1;k<=end;k++)
	{
		if(tSum[k-1-start] < pCost[k-start])
			tSum[k-start] = tSum[k-1-start] + rDist[k-start];
	}
	float ret = tSum[r];
	return ret;
}
float dtwsimdPruned(float* A, float* B,float* cb, int m, int r, float bsf, float* tSum, float* pCost, float* rDist)
{
	int length = 2*r + 1;
	// SIMD register
	//__m256 a256, b256;	

	int start, end;
	float minCost = 0.0f;
	// the first line
	for(int k=0; k<=r; k++)
	{
		rDist[k] = (A[0]-B[k])*(A[0]-B[k]);
	}

	tSum[0] = rDist[0];
	for(int ij=1; ij<=r; ij++)
		tSum[ij] = tSum[ij-1] + rDist[ij];
	

	pCost[0] = tSum[0];
	for(int ij= 1; ij<=r; ij++)
	{
		pCost[ij] = min(tSum[ij-1],tSum[ij]);
	}
	pCost[r+1] = tSum[r];

	for(int i=1; i<m-1; i++)
	{
		start = max(0,i-r);
		end = min(m-1,i+r);

		for(int k=start; k<=end; k++)
		{
			rDist[k-start] = (A[i]-B[k])*(A[i]-B[k]);
		}

		for(int k=start; k<=end; k++)
		{
			tSum[k-start] =  pCost[k-start] + rDist[k-start];
		}

		minCost = tSum[0];
		for(int k=start+1; k<=end; k++)
		{
			if(tSum[k-1-start]<pCost[k-start])
			{
				tSum[k-start] = tSum[k-1-start] + rDist[k-start];
			}
			if(tSum[k-start] < minCost)
				minCost = tSum[k-start];
		}	
		if(i+r<m-1&&minCost+cb[i+r+1] >= bsf)
		{
			return minCost + cb[i+r+1];
		}

		if((end-start+1) < length && start == 0)
		{
			pCost[start-start] = tSum[start-start];
			for(int ij= start+1; ij<=end; ij++)
			{
				pCost[ij-start] = min(tSum[ij-1-start],tSum[ij-start]);
			}
			pCost[end+1-start] = tSum[end-start];
		}
		else
		{
			for(int ij= start+1; ij<=end; ij++)
			{
				pCost[ij-1-start] = min(tSum[ij-1-start],tSum[ij-start]);
			}
			pCost[end-start] = tSum[end-start];
		}
	}
	
	// the last line
	start = m-1-r;
	end = m-1;

	for(int k=start; k<=end; k++)
	{
		rDist[k-start] = (A[m-1]-B[k])*(A[m-1]-B[k]);
	}
	
	for(int k=start; k<=end; k++)
	{
		tSum[k-start] =  pCost[k-start] + rDist[k-start];
	}
	for(int k=start+1; k<=end; k++)
	{
		if(tSum[k-1-start]<pCost[k-start])
		{
			tSum[k-start] = tSum[k-1-start] + rDist[k-start];
		}
	}
	float ret = tSum[r];
	return ret;
}




query_result exact_DTW_serial_ParIS_openmp_inmemory(ts_type *ts,ts_type *paa, ts_type *paaU, ts_type *paaL,isax_index *index, float minimum_distance, int min_checked_leaves,int warpWind) 
{

    RESET_BYTES_ACCESSED
    //LBDcalculationnumber=0;
    //RDcalculationnumber=0;

    pthread_t threadid[maxquerythread];
    COUNT_INPUT_TIME_START
    query_result approximate_result = approximate_DTW_inmemory_pRecBuf(ts, paa, index,warpWind);
    ts_type *ts_buffer = malloc(index->settings->ts_byte_size);
    query_result bsf_result = approximate_result;

    int sum_of_lab=0;
    int tight_bound = index->settings->tight_bound;
    int aggressive_check = index->settings->aggressive_check;
    float bsf_distance;
    unsigned long j;
    float* cb=(float *)calloc(index->settings->timeseries_size,sizeof(float));
    omp_lock_t bsflock;
    //omp_init_lock(&bsflock);
    // Early termination...
    if (approximate_result.distance == 0) {
        return approximate_result;
    }
    
    if(approximate_result.distance == FLT_MAX || min_checked_leaves > 1) {
        //approximate_result = refine_answer_inmemory(ts, paa, index, approximate_result, minimum_distance, min_checked_leaves);
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
    #pragma omp parallel for num_threads(maxquerythread) reduction(min : bsf_distance)
    for(unsigned long  j=0; j<index->sax_cache_size; j++) {
        sax_type *sax = &index->sax_cache[j * index->settings->paa_segments];
        if(minidist_paa_to_isax_raw_DTW_SIMD(paaU,paaL, sax, index->settings->max_sax_cardinalities,
                                                         index->settings->sax_bit_cardinality,
                                                         index->settings->sax_alphabet_cardinality,
                                                         index->settings->paa_segments, MINVAL, MAXVAL,
                                                         index->settings->mindist_sqrt) <= bsf_distance) {
            ts_buffer=&rawfile[j*index->settings->timeseries_size];

            float dist = dtw(ts, ts_buffer,cb, index->settings->timeseries_size, warpWind, bsf_distance);
            //ts_euclidean_distance_SIMD(ts, ts_buffer, index->settings->timeseries_size, bsf_distance);
            //__sync_fetch_and_add(&RDcalculationnumber,1);
            if(dist < bsf_distance) {
                //omp_set_lock(&bsflock);
                bsf_distance = dist;
            //omp_unset_lock(&bsflock);
            }
        }
    }
    approximate_result.distance=bsf_distance;
    COUNT_CAL_TIME_END
    free(cb);
    //        printf("the number of LB distance calculation is %ld\t\t and the Real distance calculation is %ld\n ",LBDcalculationnumber,RDcalculationnumber);

    return approximate_result;
}
query_result exact_DTW_serial_ParIS_inmemory(ts_type *ts,ts_type *paa, ts_type *paaU, ts_type *paaL,isax_index *index, float minimum_distance, int min_checked_leaves,int warpWind)
{

    RESET_BYTES_ACCESSED


    pthread_t threadid[maxquerythread];
    COUNT_INPUT_TIME_START
    query_result approximate_result = approximate_DTW_inmemory_pRecBuf(ts, paa, index,warpWind);
    ts_type *ts_buffer = malloc(index->settings->ts_byte_size);
    query_result bsf_result = approximate_result;
    
    float *lowerLemire= malloc(sizeof(float)*index->settings->timeseries_size);
    float *upperLemire= malloc(sizeof(float)*index->settings->timeseries_size);
    lower_upper_lemire(ts,index->settings->timeseries_size,warpWind,lowerLemire,upperLemire);
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
        //approximate_result = refine_answer_inmemory(ts, paa, index, approximate_result, minimum_distance, min_checked_leaves);
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
        essdata[i].paaU=paaU;
        essdata[i].paaL=paaL;
        essdata[i].ts=ts;
        essdata[i].bsfdistance=approximate_result.distance;
        essdata[i].sum_of_lab=0;
    }
    essdata[maxquerythread-1].index=index;
    essdata[maxquerythread-1].lock_bsf=&lock_bsf;
    essdata[maxquerythread-1].start_number=(maxquerythread-1)*(index->sax_cache_size/maxquerythread);
    essdata[maxquerythread-1].stop_number=index->sax_cache_size;
    essdata[maxquerythread-1].paa=paa;
    essdata[maxquerythread-1].paaU=paaU;
    essdata[maxquerythread-1].paaL=paaL;
    essdata[maxquerythread-1].ts=ts;
    essdata[maxquerythread-1].bsfdistance=approximate_result.distance;
    essdata[maxquerythread-1].sum_of_lab=0;

    for(i=0; i<maxquerythread; i++) 
    {
        pthread_create(&(threadid[i]),NULL,mindtwdistance_worker_inmemory,(void*)&(essdata[i]));
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
        //printf("sum is %ld\n",index->sax_cache_size);
    }
    pthread_t readthread[maxquerythread*maxreadthread];
    ParIS_read_worker_data readpointer;
    unsigned long readcounter=0;
    float bsfdistance=(approximate_result.distance);
    
        
    readpointer.ts=ts;
    readpointer.tsU=upperLemire;
    readpointer.tsL=lowerLemire;
    readpointer.index=index;
    readpointer.counter=&readcounter;
    readpointer.bsf=approximate_result.distance;
    readpointer.load_point=label_number;
    readpointer.lock_bsf=&lock_bsf;
    readpointer.bsf2=&bsfdistance;
    readpointer.minidisvector=minidisvector;
    readpointer.sum_of_lab=sum_of_lab;
    readpointer.rawfile=rawfile;
    readpointer.warpWind=warpWind;
    for (i = 0; i < maxquerythread*maxreadthread; i++)
    {   
        pthread_create(&(readthread[i]),NULL,dtwreadworker_inmemory,(void*)&(readpointer));
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
    free(lowerLemire);
    free(upperLemire);
    //printf("I need to check: %2.2lf%% of the data.\n", (double)tocheck*100/(double)index->sax_cache_size);
    /*bit_array_free(bitarray);*/
            //printf("the new distance is: %f \n",approximate_result.distance);
                //.sax_type *sax = &index->sax_cache[1 * index->settings->paa_segments];
    return approximate_result;
}

pqueue_bsf exact_DTWknn_serial_ParIS_inmemory(ts_type *ts,ts_type *paa, ts_type *paaU, ts_type *paaL,isax_index *index, float minimum_distance, int min_checked_leaves,int warpWind,int k)
{

    RESET_BYTES_ACCESSED

    pqueue_bsf *pq_bsf= pqueue_bsf_init(k);
    pthread_t threadid[maxquerythread];
    COUNT_INPUT_TIME_START
    approximate_DTWtopk_inmemory (ts, paa, index,warpWind,pq_bsf) ;
    //query_result approximate_result = approximate_DTW_inmemory_pRecBuf(ts, paa, index,warpWind);
    ts_type *ts_buffer = malloc(index->settings->ts_byte_size);
    //query_result bsf_result = approximate_result;
    float *lowerLemire= malloc(sizeof(float)*index->settings->timeseries_size);
    float *upperLemire= malloc(sizeof(float)*index->settings->timeseries_size);
    lower_upper_lemire(ts,index->settings->timeseries_size,warpWind,lowerLemire,upperLemire);
    int sum_of_lab=0;
    int tight_bound = index->settings->tight_bound;
    int aggressive_check = index->settings->aggressive_check;
    float bsf_distance;
    unsigned long j;

    // Early termination...
    //if (approximate_result.distance == 0) {
    //    return approximate_result;
   // }
    
    //if(approximate_result.distance == FLT_MAX || min_checked_leaves > 1) {
        //approximate_result = refine_answer_inmemory(ts, paa, index, approximate_result, minimum_distance, min_checked_leaves);
        //approximate_result = refine_answer_m(ts, paa, index, approximate_result2, minimum_distance, min_checked_leaves);
   // }
    
    COUNT_INPUT_TIME_END
    /*BIT_ARRAY* bitarray = bit_array_create(index->sax_cache_size);*/
    //printf("the old distance is: %f \n",approximate_result.distance);
    unsigned long i;

    #ifdef AUTO_TUNE
    float *mindists = malloc(sizeof(float) * index->sax_cache_size);
    #endif
    
    //SET_APPROXIMATE(approximate_result.distance);
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
        essdata[i].paaU=paaU;
        essdata[i].paaL=paaL;
        essdata[i].ts=ts;
        essdata[i].bsfdistance=pq_bsf->knn[pq_bsf->k-1];
        essdata[i].sum_of_lab=0;
    }
    essdata[maxquerythread-1].index=index;
    essdata[maxquerythread-1].lock_bsf=&lock_bsf;
    essdata[maxquerythread-1].start_number=(maxquerythread-1)*(index->sax_cache_size/maxquerythread);
    essdata[maxquerythread-1].stop_number=index->sax_cache_size;
    essdata[maxquerythread-1].paa=paa;
    essdata[maxquerythread-1].paaU=paaU;
    essdata[maxquerythread-1].paaL=paaL;
    essdata[maxquerythread-1].ts=ts;
    essdata[maxquerythread-1].bsfdistance=pq_bsf->knn[pq_bsf->k-1];
    essdata[maxquerythread-1].sum_of_lab=0;

    for(i=0; i<maxquerythread; i++) 
    {
        pthread_create(&(threadid[i]),NULL,mindtwdistance_worker_inmemory,(void*)&(essdata[i]));
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
        //printf("sum is %ld\n",index->sax_cache_size);
    }
    pthread_t readthread[maxquerythread*maxreadthread];
    ParIS_read_worker_data readpointer;
    unsigned long readcounter=0;
    
        
    readpointer.ts=ts;
    readpointer.tsU=upperLemire;
    readpointer.tsL=lowerLemire;
    readpointer.index=index;
    readpointer.counter=&readcounter;
    readpointer.bsf=pq_bsf->knn[pq_bsf->k-1];
    readpointer.load_point=label_number;
    readpointer.lock_bsf=&lock_bsf;
    //readpointer.bsf2=pq_bsf->knn[pq_bsf->k-1];
    readpointer.minidisvector=minidisvector;
    readpointer.sum_of_lab=sum_of_lab;
    readpointer.rawfile=rawfile;
    readpointer.warpWind=warpWind;
    readpointer.pq_bsf=pq_bsf;
    for (i = 0; i < maxquerythread*maxreadthread; i++)
    {   
        pthread_create(&(readthread[i]),NULL,dtwknnreadworker_inmemory,(void*)&(readpointer));
    }

    for (i = 0; i < maxquerythread*maxreadthread; i++)
    {   
        pthread_join(readthread[i],NULL);
    }
    COUNT_OUTPUT_TIME_END
    //approximate_result.distance=bsfdistance;
    free(essdata);
    free(minidisvector);
    free(label_number);
    free(lowerLemire);
    free(upperLemire);
    //printf("I need to check: %2.2lf%% of the data.\n", (double)tocheck*100/(double)index->sax_cache_size);
    /*bit_array_free(bitarray);*/
            //printf("the new distance is: %f \n",approximate_result.distance);
                //.sax_type *sax = &index->sax_cache[1 * index->settings->paa_segments];
    return *pq_bsf;
}




void* mindtwdistance_worker_inmemory(void *essdata)
{    
    
    unsigned long i;
    float bsfdistance,mindist;
    isax_index *index=((ParIS_LDCW_data*)essdata)->index;
    unsigned long start_number=((ParIS_LDCW_data*)essdata)->start_number;
    unsigned long stop_number=((ParIS_LDCW_data*)essdata)->stop_number;
    ts_type *paa=((ParIS_LDCW_data*)essdata)->paa;
    ts_type *paaU=((ParIS_LDCW_data*)essdata)->paaU;
    ts_type *paaL=((ParIS_LDCW_data*)essdata)->paaL;
    ts_type *ts=((ParIS_LDCW_data*)essdata)->ts;
    ((ParIS_LDCW_data*)essdata)->label_number=malloc(sizeof(unsigned long)*10000);
    ((ParIS_LDCW_data*)essdata)->minidisvector=malloc(sizeof(float)*10000);

    unsigned long max_number=10000;

    for(i=start_number;i<stop_number;i++)
    {

        sax_type *sax = &index->sax_cache[i * index->settings->paa_segments];

        mindist = minidist_paa_to_isax_raw_DTW_SIMD(paaU,paaL, sax, index->settings->max_sax_cardinalities,
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

void* dtwreadworker_inmemory(void *read_pointer)
{
    isax_index *index=((ParIS_read_worker_data*)read_pointer)->index;
    float bsf,dist,dist2;
    float* cb=calloc(index->settings->timeseries_size,sizeof(float));
        float* cb1=calloc(index->settings->timeseries_size,sizeof(float));  
    ts_type *ts_buffer;
    ts_type *ts =((ParIS_read_worker_data*)read_pointer)->ts;
    unsigned long t=0,p;
    unsigned long sum_of_lab=((ParIS_read_worker_data*)read_pointer)->sum_of_lab;
    int warpWind=((ParIS_read_worker_data*)read_pointer)->warpWind;
    float *minidisvector=((ParIS_read_worker_data*)read_pointer)->minidisvector;
    bsf= *(((ParIS_read_worker_data*)read_pointer)->bsf2);
    float *lowerLemire= ((ParIS_read_worker_data*)read_pointer)->tsL;
    float *upperLemire= ((ParIS_read_worker_data*)read_pointer)->tsU;
    int length = 2*warpWind+1;
				float* tSum = (float*) malloc(sizeof(float)*length);
				// pre_cost
				float* pCost = (float*) malloc(sizeof(float)*length);
				// raw distance
				float* rDist = (float*) malloc(sizeof(float)*length);
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
            
            dist2=lb_keogh_data_bound(ts_buffer, upperLemire,lowerLemire,cb1, index->settings->timeseries_size,bsf);


            if(dist2<bsf)
            {
                cb[index->settings->timeseries_size-1] = cb1[index->settings->timeseries_size-1];
                for(int k=index->settings->timeseries_size-2;k>=0; k--)
                    cb[k] = cb[k+1] + cb1[k];
                dist =dtwsimdPruned(ts,ts_buffer,cb, index->settings->timeseries_size, warpWind,bsf, tSum, pCost, rDist);
                //dist = ts_euclidean_distance_SIMD(ts, ts_buffer, index->settings->timeseries_size, bsf); 
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
    free(tSum);
    free(pCost);
        free(cb);
    free(cb1);
    free(rDist);
}

void* dtwknnreadworker_inmemory(void *read_pointer)
{
    isax_index *index=((ParIS_read_worker_data*)read_pointer)->index;
    float bsf,dist,dist2;
    float* cb=calloc(index->settings->timeseries_size,sizeof(float));
    float* cb1=calloc(index->settings->timeseries_size,sizeof(float));  
    ts_type *ts_buffer;
    ts_type *ts =((ParIS_read_worker_data*)read_pointer)->ts;
    unsigned long t=0,p;
    unsigned long sum_of_lab=((ParIS_read_worker_data*)read_pointer)->sum_of_lab;
    int warpWind=((ParIS_read_worker_data*)read_pointer)->warpWind;
    float *minidisvector=((ParIS_read_worker_data*)read_pointer)->minidisvector;
    float *lowerLemire= ((ParIS_read_worker_data*)read_pointer)->tsL;
    float *upperLemire= ((ParIS_read_worker_data*)read_pointer)->tsU;
    pqueue_bsf *pq_bsf= ((ParIS_read_worker_data*)read_pointer)->pq_bsf;
    int length = 2*warpWind+1;
				float* tSum = (float*) malloc(sizeof(float)*length);
				// pre_cost
				float* pCost = (float*) malloc(sizeof(float)*length);
				// raw distance
				float* rDist = (float*) malloc(sizeof(float)*length);
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
            ts_buffer= &((ParIS_read_worker_data*)read_pointer)->rawfile[p*index->settings->timeseries_size];
            
            dist2=lb_keogh_data_bound(ts_buffer, upperLemire,lowerLemire,cb1, index->settings->timeseries_size,bsf);


            if(dist2<pq_bsf->knn[pq_bsf->k-1])
            {
                cb[index->settings->timeseries_size-1] = cb1[index->settings->timeseries_size-1];
                for(int k=index->settings->timeseries_size-2;k>=0; k--)
                    cb[k] = cb[k+1] + cb1[k];

                dist =dtwsimdPruned(ts,ts_buffer,cb, index->settings->timeseries_size, warpWind,pq_bsf->knn[pq_bsf->k-1], tSum, pCost, rDist);
                //dist = ts_euclidean_distance_SIMD(ts, ts_buffer, index->settings->timeseries_size, bsf); 
                if(dist < pq_bsf->knn[pq_bsf->k-1])  
                {  
                    pthread_rwlock_wrlock(((ParIS_read_worker_data*)read_pointer)->lock_bsf); 
                    pqueue_bsf_insert(pq_bsf,dist,p,NULL);
                    pthread_rwlock_unlock(((ParIS_read_worker_data*)read_pointer)->lock_bsf); 
                }            

            }    
        } 
    }
    free(tSum);
    free(pCost);
        free(cb);
    free(cb1);
    free(rDist);
}


query_result exact_DTW_MESSI_inmemory_hybrid (ts_type *ts,ts_type *paa, ts_type *paaU, ts_type *paaL, isax_index *index,node_list *nodelist,
                           float minimum_distance, int min_checked_leaves,int warpWind) 
{   
     //   RDcalculationnumber=0;
    //LBDcalculationnumber=0;
    query_result approximate_result = approximate_DTW_inmemory_pRecBuf(ts, paa, index,warpWind);
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
        //approximate_result = refine_answer_inmemory_m(ts, paa, index, approximate_result, minimum_distance, min_checked_leaves);
    }
    pqueue_t **allpq=malloc(sizeof(pqueue_t*)*N_PQUEUE);
    
    ts_type * upperLemire = malloc(sizeof(ts_type) * index->settings->timeseries_size);
    ts_type * lowerLemire = malloc(sizeof(ts_type) * index->settings->timeseries_size);

    lower_upper_lemire(ts,index->settings->timeseries_size,warpWind,lowerLemire,upperLemire);


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
        workerdata[i].paaU=paaU;
        workerdata[i].paaL=paaL;
        workerdata[i].ts=ts;
        workerdata[i].uo=upperLemire;
        workerdata[i].lo=lowerLemire;
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
        workerdata[i].warpWind=warpWind;
    }
    
    for (int i = 0; i < maxquerythread; i++)
    {
        pthread_create(&(threadid[i]),NULL,exact_DTW_worker_inmemory_hybridpqueue,(void*)&(workerdata[i]));
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
     //       printf("the number of LB distance calculation is %ld\t\t and the Real distance calculation is %ld\n ",LBDcalculationnumber,RDcalculationnumber);
    return bsf_result;

    // Free the nodes that where not popped.

}

pqueue_bsf exact_DTWknn_MESSI_inmemory_hybrid (ts_type *ts,ts_type *paa, ts_type *paaU, ts_type *paaL, isax_index *index,node_list *nodelist,
                           float minimum_distance, int min_checked_leaves,int warpWind,int k) 
{   
        RDcalculationnumber=0;
    LBDcalculationnumber=0;
    pqueue_bsf *pq_bsf= pqueue_bsf_init(k);
      approximate_DTWtopk_inmemory (ts, paa, index,warpWind,pq_bsf);

    //query_result approximate_result = approximate_search_inmemory(ts, paa, index);
    int tight_bound = index->settings->tight_bound;
    int aggressive_check = index->settings->aggressive_check;
    int node_counter=0;
    // Early termination...

    //if(approximate_result.distance == FLT_MAX || min_checked_leaves > 1) {
        //approximate_result = refine_answer_inmemory_m(ts, paa, index, approximate_result, minimum_distance, min_checked_leaves);
    //}
    pqueue_t **allpq=malloc(sizeof(pqueue_t*)*N_PQUEUE);
    
    ts_type * upperLemire = malloc(sizeof(ts_type) * index->settings->timeseries_size);
    ts_type * lowerLemire = malloc(sizeof(ts_type) * index->settings->timeseries_size);

    lower_upper_lemire(ts,index->settings->timeseries_size,warpWind,lowerLemire,upperLemire);


    pthread_mutex_t ququelock[N_PQUEUE];
    int queuelabel[N_PQUEUE];

    //query_result *do_not_remove = &approximate_result;

    //SET_APPROXIMATE(approximate_result.distance);


    //if(approximate_result.node != NULL) {
        // Insert approximate result in heap.
        //pqueue_insert(pq, &approximate_result);
        //GOOD: if(approximate_result.node->filename != NULL)
        //GOOD: printf("POPS: %.5lf\t", approximate_result.distance);
    //}
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
        workerdata[i].paaU=paaU;
        workerdata[i].paaL=paaL;
        workerdata[i].ts=ts;
        workerdata[i].uo=upperLemire;
        workerdata[i].lo=lowerLemire;
        workerdata[i].lock_queue=&lock_queue;
        workerdata[i].lock_current_root_node=&lock_current_root_node;
        workerdata[i].lock_bsf=&lock_bsf;
        workerdata[i].nodelist=nodelist->nlist;
        workerdata[i].amountnode=nodelist->node_amount;
        workerdata[i].index=index;
        workerdata[i].minimum_distance=minimum_distance;
        workerdata[i].node_counter=&node_counter;
        workerdata[i].pq=allpq[i];
        //workerdata[i].bsf_result = &bsf_result;
        workerdata[i].lock_barrier=&lock_barrier;
        workerdata[i].alllock=ququelock;
        workerdata[i].allqueuelabel=queuelabel;
        workerdata[i].allpq=allpq;
        workerdata[i].startqueuenumber=i%N_PQUEUE;
        workerdata[i].warpWind=warpWind;
        workerdata[i].pq_bsf=pq_bsf;

    }
    
    for (int i = 0; i < maxquerythread; i++)
    {
        pthread_create(&(threadid[i]),NULL,exact_DTWknn_worker_inmemory_hybridpqueue,(void*)&(workerdata[i]));
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
            printf(" and the bsf update time is \t %ld\n ",LBDcalculationnumber);
    return *pq_bsf;

    // Free the nodes that where not popped.

}


void* exact_DTW_worker_inmemory_hybridpqueue(void *rfdata)
{   

    isax_node *current_root_node;
    query_result *n;
    isax_index *index=((MESSI_workerdata*)rfdata)->index;
    ts_type *paa=((MESSI_workerdata*)rfdata)->paa;
    ts_type *paaU=((MESSI_workerdata*)rfdata)->paaU;
    ts_type *paaL=((MESSI_workerdata*)rfdata)->paaL;
    int warpWind=((MESSI_workerdata*)rfdata)->warpWind;
    ts_type *ts=((MESSI_workerdata*)rfdata)->ts;
    ts_type *uo=((MESSI_workerdata*)rfdata)->uo;
    ts_type *lo=((MESSI_workerdata*)rfdata)->lo;
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
            //insert_tree_node_m_hybridpqueue(paaU,current_root_node,index,bsfdisntance,((MESSI_workerdata*)rfdata)->allpq,((MESSI_workerdata*)rfdata)->alllock,&tnumber);
            insert_tree_node_m_hybridpqueue_DTW(paaU,paaL,current_root_node,index,bsfdisntance,((MESSI_workerdata*)rfdata)->allpq,((MESSI_workerdata*)rfdata)->alllock,&tnumber);
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

                float distance = calculate_node_DTW2_inmemory(index, n->node, ts,uo,lo,paa,paaU,paaL, bsfdisntance,warpWind);
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
                            float distance = calculate_node_DTW2_inmemory(index, n->node, ts,uo,lo,paa,paaU,paaL, bsfdisntance,warpWind);
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



void* exact_DTWknn_worker_inmemory_hybridpqueue(void *rfdata)
{   

    isax_node *current_root_node;
    query_result *n;
    isax_index *index=((MESSI_workerdata*)rfdata)->index;
    ts_type *paa=((MESSI_workerdata*)rfdata)->paa;
    ts_type *paaU=((MESSI_workerdata*)rfdata)->paaU;
    ts_type *paaL=((MESSI_workerdata*)rfdata)->paaL;
    int warpWind=((MESSI_workerdata*)rfdata)->warpWind;
    ts_type *ts=((MESSI_workerdata*)rfdata)->ts;
    ts_type *uo=((MESSI_workerdata*)rfdata)->uo;
    ts_type *lo=((MESSI_workerdata*)rfdata)->lo;
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
    pqueue_bsf *pq_bsf=((MESSI_workerdata*)rfdata)->pq_bsf;
    float bsfdisntance=pq_bsf->knn[pq_bsf->k-1];
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
            //insert_tree_node_m_hybridpqueue(paaU,current_root_node,index,bsfdisntance,((MESSI_workerdata*)rfdata)->allpq,((MESSI_workerdata*)rfdata)->alllock,&tnumber);
            insert_tree_node_m_hybridpqueue_DTW(paaU,paaL,current_root_node,index,bsfdisntance,((MESSI_workerdata*)rfdata)->allpq,((MESSI_workerdata*)rfdata)->alllock,&tnumber);
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

                calculate_node_DTW2knn_inmemory(index, n->node, ts,uo,lo,paa,paaU,paaL, bsfdisntance,warpWind,pq_bsf,((MESSI_workerdata*)rfdata)->lock_bsf);

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
                            bsfdisntance=pq_bsf->knn[pq_bsf->k-1];
                            checks++;
                            calculate_node_DTW2knn_inmemory(index, n->node, ts,uo,lo,paa,paaU,paaL, bsfdisntance,warpWind,pq_bsf,((MESSI_workerdata*)rfdata)->lock_bsf);


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




void insert_tree_node_m_hybridpqueue_DTW(float *paaU,float *paaL,isax_node *node,isax_index *index,float bsf,pqueue_t **pq,pthread_mutex_t *lock_queue,int *tnumber)
{   
    //COUNT_CAL_TIME_START
    float distance =  minidist_paa_to_isax_DTW(paaU,paaL, node->isax_values,
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
            if (node->left_child->isax_cardinalities != NULL)
            {
                insert_tree_node_m_hybridpqueue_DTW(paaU,paaL,node->left_child,index, bsf,pq,lock_queue,tnumber);
            }
            if (node->right_child->isax_cardinalities != NULL)
            {
                insert_tree_node_m_hybridpqueue_DTW(paaU,paaL,node->right_child,index,bsf,pq,lock_queue,tnumber);
            }
        }
    }
}

query_result  approximate_DTW_inmemory_messi (ts_type *ts, ts_type *paa, isax_index *index) 
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


float calculate_node_DTW_inmemory (isax_index *index, isax_node *node, ts_type *query, float bsf,int warpWind) 
{
    COUNT_CHECKED_NODE()
    // If node has buffered data
    
    if (node->buffer != NULL) 
    {   
        float* cb=(float *)calloc(index->settings->timeseries_size,sizeof(float));
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

            float dist =  dtw(query, &(rawfile[*node->buffer->partial_position_buffer[i]]),cb, index->settings->timeseries_size, warpWind, bsf);

           // ts_euclidean_distance_SIMD(query, &(rawfile[*node->buffer->partial_position_buffer[i]]), 
            //                                   index->settings->timeseries_size, bsf);

            if (dist < bsf) {
                bsf = dist;

            }
        }
        free(cb);
    }
    
    return bsf;
}


void calculate_node_DTWknn_inmemory (isax_index *index, isax_node *node, ts_type *query,int warpWind, pqueue_bsf *pq_bsf) 
{
    COUNT_CHECKED_NODE()
    // If node has buffered data

    if (node->buffer != NULL) 
    {   
        float* cb=(float *)calloc(index->settings->timeseries_size,sizeof(float));
        int i;
       // RDcalculationnumber=RDcalculationnumber+node->buffer->partial_buffer_size;
        for (i=0; i<node->buffer->partial_buffer_size; i++) {

            float dist =  dtw(query, &(rawfile[*node->buffer->partial_position_buffer[i]]),cb, index->settings->timeseries_size, warpWind, FLT_MAX);

           // ts_euclidean_distance_SIMD(query, &(rawfile[*node->buffer->partial_position_buffer[i]]), 
            //                                   index->settings->timeseries_size, bsf);

            pqueue_bsf_insert(pq_bsf,dist,*node->buffer->partial_position_buffer[i]/index->settings->timeseries_size,node);

        }
        free(cb);
    }    
}


float calculate_node_DTW2_inmemory (isax_index *index, isax_node *node, ts_type *query,float *uo, float *lo,ts_type *paa,ts_type *paaU,ts_type *paaL, float bsf,int warpWind) 
{
    //COUNT_CHECKED_NODE()
    float distmin;
    int k;
    float* cb = (float *)malloc(sizeof(float)*index->settings->timeseries_size);
    float* cb1 = (float *)malloc(sizeof(float)*index->settings->timeseries_size);
    //float* cb2 = (float *)malloc(sizeof(float)*index->settings->timeseries_size);
    //float* datauper = (float *)malloc(sizeof(float)*index->settings->timeseries_size);
    //float* datalower = (float *)malloc(sizeof(float)*index->settings->timeseries_size);
    				// tempral sum
                    int length = 2*warpWind+1;
				float* tSum = (float*) malloc(sizeof(float)*length);
				// pre_cost
				float* pCost = (float*) malloc(sizeof(float)*length);
				// raw distance
				float* rDist = (float*) malloc(sizeof(float)*length);
                
        // If node has buffered data
    if (node->buffer != NULL) 
    {
        //#pragma omp parallel for num_threads(2) reduction(min : bsf)
        
       // __sync_fetch_and_add(&LBDcalculationnumber,node->buffer->partial_buffer_size);
        for (int i=0; i<node->buffer->partial_buffer_size; i++) 
        {

            distmin=minidist_paa_to_isax_raw_DTW_SIMD(paaU,paaL, node->buffer->partial_sax_buffer[i],
                                               index->settings->max_sax_cardinalities,
                                               index->settings->sax_bit_cardinality,
                                               index->settings->sax_alphabet_cardinality,
                                               index->settings->paa_segments, MINVAL, MAXVAL,
                                               index->settings->mindist_sqrt);

            if (distmin<bsf)
            {
                distmin=lb_keogh_data_bound(&(rawfile[*node->buffer->partial_position_buffer[i]]), uo,lo,cb1, index->settings->timeseries_size,bsf);
                //if (distmin1<bsf)
                {   
                    //lower_upper_lemire(&(rawfile[*node->buffer->partial_position_buffer[i]]),index->settings->timeseries_size,warpWind,datalower,datauper);
                    //distmin2=lb_keogh_data_bound(query, datauper,datalower,cb2, index->settings->timeseries_size,bsf);
                    
                    //if (distmin2<bsf)
                    {
                        //printf("the dist is %f !!!!\n",distmin2);
                   //     if(distmin1> distmin2)
                     //   {
                            cb[index->settings->timeseries_size-1] = cb1[index->settings->timeseries_size-1];
                            for(k=index->settings->timeseries_size-2;k>=0; k--)
                            cb[k] = cb[k+1] + cb1[k];
                        //}
                       // else
                        //{
                           // cb[index->settings->timeseries_size-1] = cb2[index->settings->timeseries_size-1];
                            //for(k=index->settings->timeseries_size-2;k>=0; k--)
                            //cb[k] = cb[k+1] + cb2[k];
                        //}
                        
                        float dist =dtwsimdPruned(query, &(rawfile[*node->buffer->partial_position_buffer[i]]),cb, index->settings->timeseries_size, warpWind,bsf, tSum, pCost, rDist);
                    //float dist =  dtw(query, &(rawfile[*node->buffer->partial_position_buffer[i]]),cb, index->settings->timeseries_size, warpWind, bsf);
                    //float dist = ts_euclidean_distance_SIMD(query, &(rawfile[*node->buffer->partial_position_buffer[i]]), 
                                             //  index->settings->timeseries_size, bsf);
                    //__sync_fetch_and_add(&RDcalculationnumber,1);
                        if (dist < bsf) {
                            bsf = dist;
                        }
                    }
                }
            }
        }
    }
    free(tSum);
	free(pCost);
	free(rDist);
    free(cb);
    free(cb1);
//    free(cb2);
  //  free(datauper);
    //free(datalower);
    return bsf;
}

void calculate_node_DTW2knn_inmemory (isax_index *index, isax_node *node, ts_type *query,float *uo, float *lo,ts_type *paa,ts_type *paaU,ts_type *paaL, float bsf,int warpWind, pqueue_bsf *pq_bsf, pthread_rwlock_t *lock_queue) 
{
    //COUNT_CHECKED_NODE()
    float distmin;
    int k;
       float* cb = (float *)malloc(sizeof(float)*index->settings->timeseries_size);
    float* cb1 = (float *)malloc(sizeof(float)*index->settings->timeseries_size);
    //float* cb2 = (float *)malloc(sizeof(float)*index->settings->timeseries_size);
    //float* datauper = (float *)malloc(sizeof(float)*index->settings->timeseries_size);
    //float* datalower = (float *)malloc(sizeof(float)*index->settings->timeseries_size);
    				// tempral sum
                    int length = 2*warpWind+1;
				float* tSum = (float*) malloc(sizeof(float)*length);
				// pre_cost
				float* pCost = (float*) malloc(sizeof(float)*length);
				// raw distance
				float* rDist = (float*) malloc(sizeof(float)*length);
                
        // If node has buffered data
    if (node->buffer != NULL) 
    {
        //#pragma omp parallel for num_threads(2) reduction(min : bsf)
        
       // __sync_fetch_and_add(&LBDcalculationnumber,node->buffer->partial_buffer_size);
        for (int i=0; i<node->buffer->partial_buffer_size; i++) 
        {

            distmin=minidist_paa_to_isax_raw_DTW_SIMD(paaU,paaL, node->buffer->partial_sax_buffer[i],
                                               index->settings->max_sax_cardinalities,
                                               index->settings->sax_bit_cardinality,
                                               index->settings->sax_alphabet_cardinality,
                                               index->settings->paa_segments, MINVAL, MAXVAL,
                                               index->settings->mindist_sqrt);
                    __sync_fetch_and_add(&LBDcalculationnumber,1);

            if (distmin<bsf)
            {
                distmin=lb_keogh_data_bound(&(rawfile[*node->buffer->partial_position_buffer[i]]), uo,lo,cb1, index->settings->timeseries_size,bsf);
                //if (distmin1<bsf)
                {   
                    //lower_upper_lemire(&(rawfile[*node->buffer->partial_position_buffer[i]]),index->settings->timeseries_size,warpWind,datalower,datauper);
                    //distmin2=lb_keogh_data_bound(query, datauper,datalower,cb2, index->settings->timeseries_size,bsf);
                    
                    //if (distmin2<bsf)
                    {
                        //printf("the dist is %f !!!!\n",distmin2);
                   //     if(distmin1> distmin2)
                     //   {
                            cb[index->settings->timeseries_size-1] = cb1[index->settings->timeseries_size-1];
                            for(k=index->settings->timeseries_size-2;k>=0; k--)
                            cb[k] = cb[k+1] + cb1[k];
                        //}
                       // else
                        //{
                           // cb[index->settings->timeseries_size-1] = cb2[index->settings->timeseries_size-1];
                            //for(k=index->settings->timeseries_size-2;k>=0; k--)
                            //cb[k] = cb[k+1] + cb2[k];
                        //}
                        
                        float dist =dtwsimdPruned(query, &(rawfile[*node->buffer->partial_position_buffer[i]]),cb, index->settings->timeseries_size, warpWind,bsf, tSum, pCost, rDist);
                    //float dist =  dtw(query, &(rawfile[*node->buffer->partial_position_buffer[i]]),cb, index->settings->timeseries_size, warpWind, bsf);
                    //float dist = ts_euclidean_distance_SIMD(query, &(rawfile[*node->buffer->partial_position_buffer[i]]), 
                                             //  index->settings->timeseries_size, bsf);
                    //__sync_fetch_and_add(&RDcalculationnumber,1);
            if (dist <= pq_bsf->knn[pq_bsf->k-1]) {
                pthread_rwlock_wrlock(lock_queue);
                __sync_fetch_and_add(&RDcalculationnumber,1);
                COUNT_QUEUE_TIME_START
                pqueue_bsf_insert(pq_bsf,dist,*node->buffer->partial_position_buffer[i]/index->settings->timeseries_size,node);
                COUNT_QUEUE_TIME_END
                pthread_rwlock_unlock(lock_queue);
            }
                    }
                }
            }
        }
    }
    free(tSum);
	free(pCost);
	free(rDist);
    free(cb);
    free(cb1);
//    free(cb2);
  //  free(datauper);
    //free(datalower);
}

query_result  approximate_DTW_inmemory_pRecBuf (ts_type *ts, ts_type *paa, isax_index *index,int warpWind) 
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
        result.distance = calculate_node_DTW_inmemory(index, node, ts, FLT_MAX,warpWind);
        result.node = node;
    }
    else {
        result.node = NULL;
        result.distance = FLT_MAX;
    }

    free(sax);

    return result;
}

void  approximate_DTWtopk_inmemory (ts_type *ts, ts_type *paa, isax_index *index,int warpWind,pqueue_bsf *pq_bsf) 
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

        calculate_node_DTWknn_inmemory(index, node, ts,warpWind, pq_bsf);
    }
    else {

    }
    for (int i = 0; i < pq_bsf->k-1; ++i)
    {
        pq_bsf->knn[i]=pq_bsf->knn[pq_bsf->k-1];
    }
    free(sax);
}



float   minidist_paa_to_isax_raw_DTW_SIMD(float *paaU,float *paaL, sax_type *sax, 
                           sax_type *sax_cardinalities,
                           sax_type max_bit_cardinality,
                           int max_cardinality,
                           int number_of_segments,
                           int min_val,
                           int max_val,
                           float ratio_sqrt)
{
   
   int region_upper[16],region_lower[16];
    float distancef[16];
    int offset = ((max_cardinality - 1) * (max_cardinality - 2)) / 2;

    __m256i vectorsignbit = _mm256_set1_epi32 (0xffffffff);
    __m256i vloweroffset = _mm256_set1_epi32 (offset-1);
    __m256i vupperoffset = _mm256_set1_epi32 (offset);

        //__m256i c_cv_0 = _mm256_set_epi32 ( sax_cardinalities[7] , sax_cardinalities[6] ,sax_cardinalities[5] ,sax_cardinalities[4] , sax_cardinalities[3] ,sax_cardinalities[2] ,sax_cardinalities[1],sax_cardinalities[0]);  
        //__m256i c_cv_1 = _mm256_set_epi32 ( sax_cardinalities[15], sax_cardinalities[14],sax_cardinalities[13],sax_cardinalities[12], sax_cardinalities[11],sax_cardinalities[10],sax_cardinalities[9],sax_cardinalities[8]);
        __m128i sax_cardinalitiesv8 = _mm_lddqu_si128 ((const void*)sax_cardinalities);
        __m256i sax_cardinalitiesv16= _mm256_cvtepu8_epi16 (sax_cardinalitiesv8);
        __m128i sax_cardinalitiesv16_0=_mm256_extractf128_si256 (sax_cardinalitiesv16,0);
        __m128i sax_cardinalitiesv16_1=_mm256_extractf128_si256 (sax_cardinalitiesv16,1);
        __m256i c_cv_0 = _mm256_cvtepu16_epi32 (sax_cardinalitiesv16_0);
        __m256i c_cv_1 = _mm256_cvtepu16_epi32 (sax_cardinalitiesv16_1);

        //__m256i v_0    = _mm256_set_epi32 (sax[7],sax[6],sax[5],sax[4],sax[3],sax[2],sax[1],sax[0]);
        //__m256i v_1    = _mm256_set_epi32 (sax[15],sax[14],sax[13],sax[12],sax[11],sax[10],sax[9],sax[8]);
        __m128i saxv8= _mm_lddqu_si128 ((const void*)sax);
        __m256i saxv16= _mm256_cvtepu8_epi16 (saxv8);
        __m128i saxv16_0 =_mm256_extractf128_si256 (saxv16,0);
        __m128i saxv16_1=_mm256_extractf128_si256 (saxv16,1);
        __m256i v_0= _mm256_cvtepu16_epi32 (saxv16_0);
        __m256i v_1 = _mm256_cvtepu16_epi32 (saxv16_1);


        __m256i c_m    = _mm256_set1_epi32 (max_bit_cardinality);
        __m256i cm_ccv_0 = _mm256_sub_epi32 (c_m, c_cv_0);
        __m256i cm_ccv_1 = _mm256_sub_epi32 (c_m, c_cv_1);

        //__m256i _mm256_set_epi32 (int e7, int e6, int e5, int e4, int e3, int e2, int e1, int e0)    
        //  __m256i _mm256_set1_epi32 (int a)
        __m256i region_lowerv_0 = _mm256_srlv_epi32 (v_0, cm_ccv_0);
        __m256i region_lowerv_1 = _mm256_srlv_epi32 (v_1, cm_ccv_1);
        region_lowerv_0 =  _mm256_sllv_epi32 (region_lowerv_0, cm_ccv_0);
        region_lowerv_1 =  _mm256_sllv_epi32 (region_lowerv_1, cm_ccv_1);

        
        __m256i v1= _mm256_andnot_si256 (_mm256_setzero_si256 (), vectorsignbit);

        __m256i region_upperv_0 = _mm256_sllv_epi32 (v1,cm_ccv_0);
        __m256i region_upperv_1 = _mm256_sllv_epi32 (v1,cm_ccv_1);
        region_upperv_0 = _mm256_andnot_si256 (region_upperv_0, vectorsignbit);
        region_upperv_1 = _mm256_andnot_si256 (region_upperv_1, vectorsignbit);

        region_upperv_0 = _mm256_or_si256 (region_upperv_0, region_lowerv_0);

        region_upperv_1 = _mm256_or_si256 (region_upperv_1, region_lowerv_1);

        region_lowerv_0 = _mm256_add_epi32 (region_lowerv_0, vloweroffset);
        region_lowerv_1 = _mm256_add_epi32 (region_lowerv_1, vloweroffset);
        region_upperv_0 = _mm256_add_epi32 (region_upperv_0, vupperoffset);
        region_upperv_1 = _mm256_add_epi32 (region_upperv_1, vupperoffset);
        _mm256_storeu_si256 ((void*)&(region_lower[0]),region_lowerv_0);
        _mm256_storeu_si256 ((void*)&(region_lower[8]),region_lowerv_1);
        _mm256_storeu_si256 ((void*)&(region_upper[0]),region_upperv_0);
        _mm256_storeu_si256 ((void*)&(region_upper[8]),region_upperv_1);

        
    //lower

        __m256i lower_juge_zerov_0 = _mm256_cmpeq_epi32 (region_lowerv_0, _mm256_setzero_si256 ());
        __m256i lower_juge_zerov_1 = _mm256_cmpeq_epi32 (region_lowerv_1, _mm256_setzero_si256 ());

        __m256i lower_juge_nzerov_0 = _mm256_andnot_si256 (lower_juge_zerov_0, vectorsignbit);
        __m256i lower_juge_nzerov_1 = _mm256_andnot_si256 (lower_juge_zerov_1, vectorsignbit);

        __m256 minvalv = _mm256_set1_ps (min_val);



        
        __m256 lsax_breakpoints_shiftv_0 =_mm256_i32gather_ps (sax_breakpoints,region_lowerv_0, 4);
        //__m256 lsax_breakpoints_shiftv_0= _mm256_set_ps (sax_breakpoints[region_lower[7]],
                                                        //sax_breakpoints[region_lower[6]], 
                                                        //sax_breakpoints[region_lower[5]],
                                                        //sax_breakpoints[region_lower[4]],
                                                        //sax_breakpoints[region_lower[3]],
                                                        //sax_breakpoints[region_lower[2]],
                                                        //sax_breakpoints[region_lower[1]],
                                                        //sax_breakpoints[region_lower[0]]);
        __m256 lsax_breakpoints_shiftv_1 =_mm256_i32gather_ps (sax_breakpoints,region_lowerv_1, 4);
        //__m256 lsax_breakpoints_shiftv_1= _mm256_set_ps (sax_breakpoints[region_lower[15]],
                                                        //sax_breakpoints[region_lower[14]], 
                                                        //sax_breakpoints[region_lower[13]],
                                                        //sax_breakpoints[region_lower[12]],
                                                        //sax_breakpoints[region_lower[11]],
                                                        //sax_breakpoints[region_lower[10]],
                                                        //sax_breakpoints[region_lower[9]],
                                                        //sax_breakpoints[region_lower[8]]);


        __m256 breakpoint_lowerv_0 = (__m256)_mm256_or_si256 (_mm256_and_si256(lower_juge_zerov_0,(__m256i)minvalv),_mm256_and_si256(lower_juge_nzerov_0,(__m256i)lsax_breakpoints_shiftv_0));
        __m256 breakpoint_lowerv_1 = (__m256)_mm256_or_si256 (_mm256_and_si256(lower_juge_zerov_1,(__m256i)minvalv),_mm256_and_si256(lower_juge_nzerov_1,(__m256i)lsax_breakpoints_shiftv_1));

    //uper
        __m256 usax_breakpoints_shiftv_0 =_mm256_i32gather_ps (sax_breakpoints,region_upperv_0, 4);
        //__m256 usax_breakpoints_shiftv_0= _mm256_set_ps (sax_breakpoints[region_upper[7]],
                                                        //sax_breakpoints[region_upper[6]], 
                                                        //sax_breakpoints[region_upper[5]],
                                                        //sax_breakpoints[region_upper[4]],
                                                        //sax_breakpoints[region_upper[3]],
                                                        //sax_breakpoints[region_upper[2]],
                                                        //sax_breakpoints[region_upper[1]],
                                                        //sax_breakpoints[region_upper[0]]);
        __m256 usax_breakpoints_shiftv_1 =_mm256_i32gather_ps (sax_breakpoints,region_upperv_1, 4);
        //__m256 usax_breakpoints_shiftv_1= _mm256_set_ps (sax_breakpoints[region_upper[15]],
                                                        //sax_breakpoints[region_upper[14]], 
                                                        //sax_breakpoints[region_upper[13]],
                                                        //sax_breakpoints[region_upper[12]],
                                                        //sax_breakpoints[region_upper[11]],
                                                        //sax_breakpoints[region_upper[10]],
                                                        //sax_breakpoints[region_upper[9]],
                                                        //sax_breakpoints[region_upper[8]]);


        __m256i upper_juge_maxv_0 = _mm256_cmpeq_epi32 (region_upperv_0,  _mm256_set1_epi32 (max_cardinality - 1));
        __m256i upper_juge_maxv_1 = _mm256_cmpeq_epi32 (region_upperv_1,  _mm256_set1_epi32 (max_cardinality - 1));

        __m256i upper_juge_nmaxv_0 = _mm256_andnot_si256 (upper_juge_maxv_0, vectorsignbit);
        __m256i upper_juge_nmaxv_1 = _mm256_andnot_si256 (upper_juge_maxv_1, vectorsignbit);

        __m256 breakpoint_upperv_0 = (__m256)_mm256_or_si256 (_mm256_and_si256(upper_juge_maxv_0,(__m256i)_mm256_set1_ps (max_val)),_mm256_and_si256(upper_juge_nmaxv_0,(__m256i)usax_breakpoints_shiftv_0));
        __m256 breakpoint_upperv_1 = (__m256)_mm256_or_si256 (_mm256_and_si256(upper_juge_maxv_1,(__m256i)_mm256_set1_ps (max_val)),_mm256_and_si256(upper_juge_nmaxv_1,(__m256i)usax_breakpoints_shiftv_1));







    //dis
            __m256 paaUv_0,paaUv_1,paaLv_0,paaLv_1;



            ///paav_0 =_mm256_loadu_ps (paa);
           // paav_1 =_mm256_loadu_ps (&(paa[8]));
            paaUv_0 =_mm256_loadu_ps (paaU);
            paaUv_1 =_mm256_loadu_ps (&(paaU[8]));            
            paaLv_0 =_mm256_loadu_ps (paaL);
            paaLv_1 =_mm256_loadu_ps (&(paaL[8]));


            __m256 dis_juge_upv_0=_mm256_cmp_ps (breakpoint_lowerv_0, paaUv_0, _CMP_GT_OS);
            __m256 dis_juge_upv_1=_mm256_cmp_ps (breakpoint_lowerv_1, paaUv_1, _CMP_GT_OS);

            __m256 dis_juge_lov_0=_mm256_cmp_ps (breakpoint_upperv_0, paaLv_0, _CMP_LT_OS);
            __m256 dis_juge_lov_1=_mm256_cmp_ps (breakpoint_upperv_1, paaLv_1, _CMP_LT_OS);



            //__m256 dis_juge_lov_0=(__m256)_mm256_and_si256 ((__m256i)_mm256_cmp_ps (breakpoint_lowerv_0, paaUv_0, _CMP_NGT_US),(__m256i)_mm256_cmp_ps (breakpoint_upperv_0, paav_0, _CMP_LT_OS))  ;
            //__m256 dis_juge_lov_1=(__m256)_mm256_and_si256 ((__m256i)_mm256_cmp_ps (breakpoint_lowerv_1, paaUv_1, _CMP_NGT_US),(__m256i)_mm256_cmp_ps (breakpoint_upperv_1, paav_1, _CMP_LT_OS));

            __m256 dis_juge_elv_0=(__m256)_mm256_andnot_si256 (_mm256_or_si256 ((__m256i)dis_juge_upv_0, (__m256i)dis_juge_lov_0),vectorsignbit);
            __m256 dis_juge_elv_1=(__m256)_mm256_andnot_si256 (_mm256_or_si256 ((__m256i)dis_juge_upv_1, (__m256i)dis_juge_lov_1),vectorsignbit);

            __m256 dis_lowv_0 =_mm256_sub_ps (breakpoint_lowerv_0, paaUv_0);
            __m256 dis_lowv_1 =_mm256_sub_ps (breakpoint_lowerv_1, paaUv_1);
            __m256 dis_uppv_0 =_mm256_sub_ps (breakpoint_upperv_0, paaLv_0);
            __m256 dis_uppv_1 =_mm256_sub_ps (breakpoint_upperv_1, paaLv_1);


            __m256 distancev_0=(__m256)_mm256_or_si256(_mm256_or_si256(_mm256_and_si256((__m256i)dis_juge_upv_0,(__m256i)dis_lowv_0),_mm256_and_si256((__m256i)dis_juge_lov_0,(__m256i)dis_uppv_0)),_mm256_and_si256((__m256i)dis_juge_elv_0,(__m256i)_mm256_set1_ps (0.0)));
            __m256 distancev_1=(__m256)_mm256_or_si256(_mm256_or_si256(_mm256_and_si256((__m256i)dis_juge_upv_1,(__m256i)dis_lowv_1),_mm256_and_si256((__m256i)dis_juge_lov_1,(__m256i)dis_uppv_1)),_mm256_and_si256((__m256i)dis_juge_elv_1,(__m256i)_mm256_set1_ps (0.0)));


            __m256 distancesum_0 = _mm256_dp_ps (distancev_0, distancev_0, 0xff);
            __m256 distancesum_1 = _mm256_dp_ps (distancev_1, distancev_1, 0xff);
            __m256 distancevf = _mm256_add_ps (distancesum_0, distancesum_1);
            //__m256 distancev2 = _mm256_hadd_ps (distancev, distancev);
            //__m256 distancevf = _mm256_hadd_ps (distancev2, distancev2);
            //__m256 _mm256_dp_ps (__m256 a, __m256 b, const int imm8);

            _mm256_storeu_ps (distancef ,distancevf);
        //_mm256_storeu_ps (&checkvalue[8] ,distancev_1);


            return (distancef[0]+distancef[4])*ratio_sqrt ;
}




// lb_ keogh, given a query series (ordered) with its lower - upper lemire envelope
// it computes the lb of the dtw. Moreover it keeps the comulative bound, which is then used for early abandoning the computation 

float lb_keogh_cumulative_norm(float *qs, float *uo, float *lo, float *cb,int len, float mean, float std, float best_so_far)
{
    float lb = 0;
    float x, d;
    int i;
    for (i = 0; i < len && lb < best_so_far; i++)
    {
        x = qs[(i)];
        d = 0;
        if (x > uo[i])
            d = dist(x,uo[i]);
        else if(x < lo[i])
            d = dist(x,lo[i]);
        lb += d;
        cb[i] = d;
    }
    return lb;
}

float lb_keogh_data_bound( float* qo,float* tu,  float* tl, float* cb, int len, float bsf)
{
	float lb = 0;
	float uu=0,ll=0,d=0;
	int i=0;

	int len1 = (len/8)*8;
	__m256 tu256, tl256, cb256, Q, calc1, calc2;
	__m128 temp1, temp2;
	float *cbtmp = malloc(sizeof(float)*8);
	
	for(i=0; i<len1&&lb<bsf; i+=8)
	{
		Q = _mm256_loadu_ps(&qo[i]);
		tu256 = _mm256_loadu_ps(&tu[i]);
		tl256 = _mm256_loadu_ps(&tl[i]);
		//tu256 = _mm_setr_ps(tu[order[i]],tu[order[i+1]],tu[order[i+2]],tu[order[i+3]]);
		//tl256 = _mm_setr_ps(tl[order[i]],tl[order[i+1]],tl[order[i+2]],tl[order[i+3]]);
		calc1 = _mm256_min_ps(Q,tu256);
		calc1 = _mm256_sub_ps(Q,calc1);

		calc2 = _mm256_max_ps(Q,tl256);
		calc2 = _mm256_sub_ps(calc2,Q);
		calc1 = _mm256_add_ps(calc1,calc2);

		calc1 = _mm256_mul_ps(calc1,calc1);

		_mm256_storeu_ps(cbtmp,calc1);

		calc1 = _mm256_hadd_ps(calc1,calc1);
		calc1 = _mm256_hadd_ps(calc1,calc1);
		temp1 = _mm256_extractf128_ps(calc1,1);
		temp2 = _mm_add_ss(_mm256_castps256_ps128(calc1),temp1);
		lb += _mm_cvtss_f32(temp2);
		
		cb[i]=cbtmp[0];cb[i+1] = cbtmp[1];cb[i+2]=cbtmp[2];cb[i+3]=cbtmp[3];
		cb[i+4]=cbtmp[4];cb[i+5] = cbtmp[5];cb[i+6]=cbtmp[6];cb[i+7]=cbtmp[7];
	}

	for(;i<len&&lb<bsf;i++)
	{
		uu = tu[i];
		ll = tl[i];
		d= 0;
		if(qo[i] > uu)
		{
			d = dist(qo[i],uu);
		}
		else if(qo[i] < ll)
		{
			d = dist(qo[i],ll);
		}
		lb += d;
		cb[i] = d;
	}

	free( cbtmp);
	return lb;
}
