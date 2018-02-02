/*
 * Binghamton CS 451/551 Project "Memory manager".
 * This file needs to be turned in.	
 */

#ifndef __MEM_MNGR_H__
#define  __MEM_MNGR_H__

#include "common.h"

/* Data allocation alignment. */
#define MEM_ALIGNMENT_BOUNDARY 8

/* The number of memory slots in a memory batch. */
#define MEM_BATCH_SLOT_COUNT 8

/* An auxiliary macro to get the aligned size given the size requested */
#define SLOT_ALLINED_SIZE(size)  (((size + MEM_ALIGNMENT_BOUNDARY - 1)/MEM_ALIGNMENT_BOUNDARY) * MEM_ALIGNMENT_BOUNDARY)

/* Definition of a "memory batch" */
typedef struct _stru_mem_batch
{
    void * batch_mem; // the starting address of the memory chunk
    struct _stru_mem_batch * next_batch; // points to the next memory batch
} STRU_MEM_BATCH;

/* Definition of a "memory list" */
typedef struct _stru_mem_list
{
    int slot_size;    // the slot size of this list
    int batch_count;  // the number of memory batches in this list
    unsigned char * free_slots_bitmap; // the bitmap of free slots in this list
    int bitmap_size; // the size in bytes of the bitmap
    struct _stru_mem_batch * first_batch; // points to the first memory batch of this list
    struct _stru_mem_list * next_list; // points to the next memory list
} STRU_MEM_LIST;

void mem_mngr_init(void);
void mem_mngr_leave(void);
void mem_mngr_print_snapshot(void);
void * mem_mngr_alloc(size_t size);
void mem_mngr_free(void * ptr);

#endif //__MEM_MNGR_H__
