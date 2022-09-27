#ifndef parallel_parallel_index_engine_h
#define parallel_parallel_index_engine_h
#include "../../config.h"
#include "../../globals.h"
#include "sax/ts.h"
#include "sax/sax.h"	
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <math.h>
#include "isax_index.h"
#include "isax_query_engine.h"

void isax_index_binary_file_m(const char *ifilename, int ts_num, isax_index *index,int calculate_thread);
void isax_index_binary_file_m_new(const char *ifilename, int ts_num, isax_index *index,int calculate_thread);
void isax_index_binary_file_pRecBuf(const char *ifilename, int ts_num, isax_index *index,int calculate_thread);
void isax_index_binary_file_2RecBuf(const char *ifilename, int ts_num, isax_index *index,int calculate_thread);
void isax_index_binary_file_2nRecBuf(const char *ifilename, int ts_num, isax_index *index,int calculate_thread);
void* indexbulkloadingworker(void* transferdata);
void* indexbulkloadingworker_new(void* transferdata);
void* indexbulkloadingworker_new2(void *transferdata);
void* indexbulkloadingworker_pRecBuf(void *transferdata);
void* indexbulkloadingworker_2RecBuf(void *transferdata);
isax_node * insert_to_fbl_m(first_buffer_layer *fbl, 
							sax_type *sax,
							file_position_type *pos,
							root_mask_type mask, 
							isax_index *index, 
							pthread_mutex_t *locknode,
							pthread_mutex_t *lockfbl);
isax_node * insert_to_fbl_m_new(first_buffer_layer *fbl, 
							sax_type *sax,
							file_position_type *pos,
							root_mask_type mask, 
							isax_index *index, 
							pthread_mutex_t *locknode,
							pthread_mutex_t *lockfbl);

isax_node * insert_to_pRecBuf(parallel_first_buffer_layer *fbl,
									sax_type *sax,
									file_position_type *pos,
									root_mask_type mask, 
									isax_index *index, 
									pthread_mutex_t *lock_firstnode,
									int workernumber,
									int total_workernumber);
isax_node * insert_to_2pRecBuf(parallel_dfirst_buffer_layer *fbl,
									sax_type *sax,
									file_position_type *pos,
									root_mask_type mask, 
									isax_index *index, 
									pthread_mutex_t *lock_firstnode,
									int workernumber,
									int total_workernumber);


enum response indexconstruction(first_buffer_layer *fbl,
								isax_index *index,
								pthread_mutex_t *lock_index,
								pthread_mutex_t *lock_disk,
								int calculate_thread); 
enum response indexflush(first_buffer_layer *fbl,
								isax_index *index,
								pthread_mutex_t *lock_index,
								pthread_mutex_t *lock_disk,
								int calculate_thread); 
enum response indexpRecBufflush(parallel_first_buffer_layer *fbl,
								isax_index *index,
								pthread_mutex_t *lock_index,
								pthread_mutex_t *lock_disk,
								int calculate_thread,
								int preworkernumber); 
enum response indexconstruction_pRecBuf(first_buffer_layer *fbl,
								isax_index *index,
								pthread_mutex_t *lock_index,
								int calculate_thread); 
void* indexconstructionworker(void *input);
void* indexflushworker(void *input);
void* indexpRecBufflushworker(void *input);

void* indexconstructionworker_pRecBuf(void *input);
void* indexconstructionworker_pRecBuf_new(void *input);
void* indexconstructionworker_2RecBuf(void *input);

void* indexconstructionworker_2nRecBuf(void *input);
root_mask_type isax_fbl_index_insert_m(isax_index *index, 
									sax_type * sax,
                                    file_position_type * pos, 
                                    pthread_mutex_t *lock_record, 
                                    pthread_mutex_t *lock_fbl,
                                    pthread_mutex_t *lock_cbl,
                                    pthread_mutex_t *lock_firstnode,
                                    pthread_mutex_t *lock_index,
                                    pthread_mutex_t *locdisk);
root_mask_type isax_fbl_index_insert_m_new(isax_index *index, 
									sax_type * sax,
                                    file_position_type * pos, 
                                    pthread_mutex_t *lock_record, 
                                    pthread_mutex_t *lock_fbl,
                                    pthread_mutex_t *lock_cbl,
                                    pthread_mutex_t *lock_firstnode,
                                    pthread_mutex_t *lock_index,
                                    pthread_mutex_t *locdisk);

root_mask_type isax_pRecBuf_index_insert(isax_index *index, 
                                    sax_type * sax,
                                    file_position_type * pos, 
                                    pthread_mutex_t *lock_record,
                                    pthread_mutex_t *lock_firstnode,
                                    int workernumber,
                                    int total_workernumber);
root_mask_type isax_2pRecBuf_index_insert(isax_index *index, 
                                    sax_type * sax,
                                    file_position_type * pos, 
                                    pthread_mutex_t *lock_record,
                                    pthread_mutex_t *lock_firstnode,
                                    int workernumber,
                                    int total_workernumber);
typedef struct index_buffer_data
{
	//int ts_num,ts_loaded;
	file_position_type pos;
	isax_index *index;
	ts_type * ts;
	sax_type *saxv;
	int fin_number,blocid;
	pthread_mutex_t *lock_record;
	pthread_mutex_t *lock_fbl;
	pthread_mutex_t *lock_index;
	pthread_mutex_t *lock_cbl;
	pthread_mutex_t *lock_firstnode;
	pthread_mutex_t *lock_nodeconter;
	pthread_mutex_t *lock_disk;
	file_position_type *fbl;
	int *bufferpresize;
	int workernumber;
	int total_workernumber;
	int *nodecounter;
	pthread_barrier_t *lock_barrier1, *lock_barrier2, *lock_barrier3;
	bool finished;
}index_buffer_data;


int read_block_length;	
#endif