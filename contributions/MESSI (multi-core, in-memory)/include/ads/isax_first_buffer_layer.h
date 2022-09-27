//
//  first_buffer_layer.h
//  isaxlib
//
//  Created by Kostas Zoumpatianos on 3/20/12.
//  Copyright 2012 University of Trento. All rights reserved.
//

#ifndef isaxlib_first_buffer_layer_h
#define isaxlib_first_buffer_layer_h
#include "../../config.h"
#include "../../globals.h"
#include "isax_node.h"
#include "isax_index.h"

typedef struct fbl_soft_buffer {
    isax_node *node;
    sax_type ** sax_records;
    file_position_type ** pos_records;
    int initialized; 
    int max_buffer_size;
    int buffer_size;
} fbl_soft_buffer;

typedef struct fbl_soft_buffer2 {
    isax_node *node;
    sax_type * sax_records;
    file_position_type *pos_records;
    int initialized; 
    int max_buffer_size;
    int buffer_size;
} fbl_soft_buffer2;
typedef struct parallel_fbl_soft_buffer {
    isax_node *node;
    sax_type ** sax_records;
    file_position_type ** pos_records;
    int initialized; 
    int *max_buffer_size;
    int *buffer_size;
    int finished;
} parallel_fbl_soft_buffer;

typedef struct parallel_dfbl_soft_buffer {
    isax_node *node;
    sax_type *** sax_records;
    file_position_type *** pos_records;
    int initialized; 
    int *max_buffer_size;
    int *buffer_size;
    int finished;
} parallel_dfbl_soft_buffer;

typedef struct first_buffer_layer {
    int number_of_buffers;
    int initial_buffer_size;
    int max_total_size;
    int current_record_index;
    char *current_record;
    char *hard_buffer;
    fbl_soft_buffer *soft_buffers;
    
} first_buffer_layer;
typedef struct first_buffer_layer2 {
    int number_of_buffers;
    int initial_buffer_size;
    int max_total_size;
    int current_record_index;
    char *current_record;
    char *hard_buffer;
    fbl_soft_buffer2 *soft_buffers;
    
} first_buffer_layer2;
typedef struct parallel_first_buffer_layer {
    int number_of_buffers;
    int initial_buffer_size;
    int max_total_size;
    int current_record_index;
    char *current_record;
    char *hard_buffer;
    parallel_fbl_soft_buffer *soft_buffers;
    int total_worker_number;
} parallel_first_buffer_layer;

typedef struct parallel_dfirst_buffer_layer {
    int number_of_buffers;
    int initial_buffer_size;
    int max_total_size;
    int current_record_index;
    char *current_record;
    char *hard_buffer;
    parallel_dfbl_soft_buffer *soft_buffers;
    int total_worker_number;
} parallel_dfirst_buffer_layer;


typedef struct trans_fbl_input
{
    int start_number,stop_number,conternumber;
    isax_index *index;
    pthread_mutex_t *lock_index;
    pthread_mutex_t *lock_fbl_conter;
    pthread_mutex_t *lock_write;
    first_buffer_layer *fbl;
    int preworkernumber;
    int fbloffset;
    int *buffersize;
    pthread_barrier_t *lock_barrier1, *lock_barrier2, *lock_barrier3;
    bool finished;
}trans_fbl_input;

first_buffer_layer * initialize_fbl(int initial_buffer_size, int max_fbl_size, 
                                    int max_total_size, isax_index *index);

parallel_first_buffer_layer * initialize_pRecBuf(int initial_buffer_size, int max_fbl_size, 
                                    int max_total_size, isax_index *index);
first_buffer_layer2 * initialize_simrec(int initial_buffer_size, int number_of_buffers,
                                           int max_total_buffers_size, isax_index *index);
parallel_dfirst_buffer_layer * initialize_2pRecBuf(int initial_buffer_size, int max_fbl_size, 
                                    int max_total_size, isax_index *index);

void destroy_fbl(first_buffer_layer *fbl);
void destroy_fbl2(first_buffer_layer2 *fbl);
void destroy_pRecBuf(parallel_first_buffer_layer *fbl,int prewokernumber);
isax_node * insert_to_fbl(first_buffer_layer *fbl, sax_type *sax,
                          file_position_type * pos, root_mask_type mask, 
                          isax_index *index);

enum response flush_fbl(first_buffer_layer *fbl, isax_index *index);

#endif
