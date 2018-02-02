/*
 * Binghamton CS 451/551 Project "Memory manager".
 * This file needs to be turned in.	
 */


#include "memory_manager.h"
#include<assert.h>

#define FREE_BIT 1
#define OCCUPIED_BIT 0


static STRU_MEM_LIST * mem_pool = NULL;

/*
 * Print out the current status of the memory manager.
 * Reading this function may help you understand how the memory manager organizes the memory.
 * Do not change the implementation of this function. It will be used to help the grading.
 */
void mem_mngr_print_snapshot(void)
{
    STRU_MEM_LIST * mem_list = NULL;

    printf("============== Memory snapshot ===============\n");

    mem_list = mem_pool; // Get the first memory list
    while(NULL != mem_list)
    {
        STRU_MEM_BATCH * mem_batch = mem_list->first_batch; // Get the first mem batch from the list 

        printf("mem_list %p slot_size %d batch_count %d free_slot_bitmap %p\n", 
                   mem_list, mem_list->slot_size, mem_list->batch_count, mem_list->free_slots_bitmap);
        bitmap_print_bitmap(mem_list->free_slots_bitmap, mem_list->bitmap_size);

        while (NULL != mem_batch)
        {
            printf("\t mem_batch %p batch_mem %p\n", mem_batch, mem_batch->batch_mem);
            mem_batch = mem_batch->next_batch; // get next mem batch
        }

        mem_list = mem_list->next_list;
    }

    printf("==============================================\n");
}

/*
 * Initialize the memory manager.
 * You may add your code related to initialization here if there is any.
 */
void mem_mngr_init(void)
{
    ;
}

/*
 * Clean up the memory manager (e.g., release all the memory allocated)
 */
void mem_mngr_leave(void)
{

    // If mem_pool is NULL then either memory manager initializer is not called.
    // or mem_mngr_leave has been called already.
    if(mem_pool == NULL){
        return;
    }
    
    STRU_MEM_LIST * mem_list = NULL;

    mem_list = mem_pool; // Get the first memory list

    while(NULL != mem_list)
    {
        STRU_MEM_BATCH * mem_batch = mem_list->first_batch; // Get the first mem batch from the list 
        
        while(NULL != mem_batch)
        {
            // delete curr batch first  
            free(mem_batch->batch_mem);
            mem_batch->batch_mem = NULL;

            STRU_MEM_BATCH *temp = mem_batch;
            mem_batch = mem_batch->next_batch;
            free(temp);
            temp = NULL;
        }
    
        STRU_MEM_LIST *temp2 = mem_list;
        mem_list = mem_list->next_list;
        free(temp2->free_slots_bitmap);
        temp2->free_slots_bitmap = NULL;
        free(temp2);
    }
    mem_pool = NULL;
}

struct _stru_mem_batch* allocateNewBatch(size_t batch_size){
    struct _stru_mem_batch* new_batch = malloc(sizeof(struct _stru_mem_batch));
    
    new_batch->batch_mem = malloc(batch_size*MEM_BATCH_SLOT_COUNT);
    new_batch->next_batch = NULL;
    
    return new_batch; 
}

/*
 * Allocate a chunk of memory 	
 */
void * mem_mngr_alloc(size_t size)
{

    // In vase size is zero or less than zero
    // return NULL. 
    if(size <= 0){
        return NULL;
    }

    if(mem_pool == NULL){

        // allocate memory to _stru_mem_list
        int i=0;

        mem_pool = malloc(sizeof(STRU_MEM_LIST));
        mem_pool->free_slots_bitmap = malloc(sizeof(char)*(MEM_BATCH_SLOT_COUNT/BIT_PER_BYTE)); 
        mem_pool->bitmap_size = sizeof(char)*(MEM_BATCH_SLOT_COUNT/BIT_PER_BYTE);
        memset(mem_pool->free_slots_bitmap,~0,mem_pool->bitmap_size);

        mem_pool->slot_size = SLOT_ALLINED_SIZE(size);
        mem_pool->next_list = NULL;
        size_t aligned_size = SLOT_ALLINED_SIZE(size);

        mem_pool->first_batch = malloc(sizeof(struct _stru_mem_batch));
        mem_pool->first_batch->batch_mem = malloc(aligned_size*MEM_BATCH_SLOT_COUNT);
        mem_pool->first_batch->next_batch = NULL;
        mem_pool->batch_count = 1;
    }

	// First round the requested size to upper multiple of MEM_ALIGNMENT_BOUNDARY
    size_t size_to_allocate = SLOT_ALLINED_SIZE(size);

    int i=0;
    STRU_MEM_LIST* temp = mem_pool;
    STRU_MEM_LIST* prevList = NULL;

    while(temp!= NULL){

        // Slot of relevant size found
        if(temp->slot_size == size_to_allocate){
            // We found a suitable size list.
            // Now we need to check if there is any
            // free memory slot in this batch or not.
            struct _stru_mem_batch* tempBatch = temp->first_batch;

            int pos = bitmap_find_first_bit(temp->free_slots_bitmap,
                    temp->bitmap_size,
                    FREE_BIT);

            assert(pos != BITMAP_OP_ERROR);


            int slot_in_batch = pos/MEM_BATCH_SLOT_COUNT;
            
            if(pos != BITMAP_OP_NOT_FOUND){
                int batchCount = 0;
                while(batchCount != slot_in_batch){
                    batchCount++;
                    tempBatch = tempBatch->next_batch;
                }

                int res = bitmap_clear_bit(temp->free_slots_bitmap, temp->bitmap_size, pos);
                assert(res == BITMAP_OP_SUCCEED);

                int num = pos%MEM_BATCH_SLOT_COUNT;
                return (void*)((char*)tempBatch->batch_mem + (temp->slot_size*num));
            }

            struct _stru_mem_batch* prevBatch = temp->first_batch;
            while(prevBatch->next_batch != NULL){
                prevBatch = prevBatch->next_batch;
            }
            
            // If flow is here it means empty batch is not found.
            // we will need to allocate one empty batch.
            struct _stru_mem_batch* new_batch = allocateNewBatch(size_to_allocate);
            prevBatch->next_batch = new_batch; 
            temp->batch_count += 1;
            
            // We will need to reallocate bitmap.
            size_t old_bitmap_size = temp->bitmap_size;
            temp->bitmap_size = sizeof(char)*(temp->batch_count)*(MEM_BATCH_SLOT_COUNT/BIT_PER_BYTE);


            temp->free_slots_bitmap = realloc(temp->free_slots_bitmap,temp->bitmap_size);
            memset(temp->free_slots_bitmap+old_bitmap_size,~0,(temp->bitmap_size-old_bitmap_size));
            int res = bitmap_clear_bit(temp->free_slots_bitmap, temp->bitmap_size, (old_bitmap_size*BIT_PER_BYTE));
            assert(res == BITMAP_OP_SUCCEED);

            return new_batch->batch_mem;;
        }
        prevList = temp;
        temp = temp->next_list;
    }

    // If flow is here it means that block 
    // of size size_to_allocate is not found
    // and we need to add one more list here.
    STRU_MEM_LIST* new_mem_list= malloc(sizeof(STRU_MEM_LIST)); 
    new_mem_list->slot_size = size_to_allocate;
    new_mem_list->free_slots_bitmap = malloc(sizeof(char)*(MEM_BATCH_SLOT_COUNT/BIT_PER_BYTE)); 
    new_mem_list->bitmap_size = sizeof(char)*(MEM_BATCH_SLOT_COUNT/BIT_PER_BYTE);
    memset(new_mem_list->free_slots_bitmap,~0,new_mem_list->bitmap_size);
    new_mem_list->next_list = NULL;    
    new_mem_list->first_batch = malloc(sizeof(struct _stru_mem_batch));
    new_mem_list->first_batch->batch_mem = malloc(size_to_allocate*MEM_BATCH_SLOT_COUNT);
    new_mem_list->first_batch->next_batch = NULL;
    new_mem_list->batch_count = 1;

    prevList->next_list = new_mem_list;

    // In this case first slot would be free for sure
    int res = bitmap_clear_bit(new_mem_list->free_slots_bitmap, new_mem_list->bitmap_size, 0);
    assert(res == BITMAP_OP_SUCCEED);
    return (void*)(new_mem_list->first_batch->batch_mem);
}

/*
 * Free a chunk of memory pointed by ptr
 */
void mem_mngr_free(void * ptr)
{

    if(NULL == ptr){
        return;
    }

    //Check which slot this pointer belongs to.
    STRU_MEM_LIST * mem_list = NULL;
    mem_list = mem_pool; // Get the first memory list
    while(NULL != mem_list)
    {
        int batchNum = 0;
        STRU_MEM_BATCH * mem_batch = mem_list->first_batch; // Get the first mem batch from the list 
        while (NULL != mem_batch)
        {
            // Calculate start and end of eac batch here
            void* start_address = mem_batch->batch_mem;
            void* end_address = (void*)((char*)start_address + (MEM_BATCH_SLOT_COUNT*mem_list->slot_size));

            if((ptr >= start_address) && (ptr < end_address)){
                // Address in question lies in this batch.
                ptrdiff_t diff = (char*)ptr - (char*)start_address;

                // If given address is not starting address
                if(diff % (mem_list->slot_size) != 0){
                    printf("Error! Invalid free.\n");
                    return;
                }

                int index = diff/mem_list->slot_size;

                int retRes = bitmap_bit_is_set(mem_list->free_slots_bitmap,mem_list->bitmap_size, (index + (batchNum*MEM_BATCH_SLOT_COUNT)));

                // Checking for double free
                if((retRes == BITMAP_OP_ERROR) || (retRes == FREE_BIT)){
                    printf("Error! Double free.\n");
                    return;
                }

                bitmap_set_bit(mem_list->free_slots_bitmap,mem_list->bitmap_size,(index+ (batchNum*MEM_BATCH_SLOT_COUNT)));
                return;
            }
            mem_batch = mem_batch->next_batch; // get next mem batch
            batchNum++;
        }
        mem_list = mem_list->next_list;
    }
    return;

}
