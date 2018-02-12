#ifndef HASHMAP_H
#define HASHMAP_H

// Constants
#define NUM_BUCKETS 128
#define NUM_ELEMENTS_PER_BUCKET 4*MB/(sizeof(long)*NUM_BUCKETS)
#define SMALL_SET_NUM_BUCKETS   NUM_ELEMENTS_PAGE/NUM_ELEMENTS_PER_BUCKET

//#define SMALL_DATASET_POPULATION            4096
#define LARGE_STRUCTURE_LOWER_LIMIT     NUM_ELEMENTS_PAGE
#define SHORT_STRUCTURE_LIMIT           NUM_ELEMENTS_PAGE*40
#define LARGE_STRUCTURE_LIMIT           SHORT_STRUCTURE_LIMIT+NUM_ELEMENTS_PAGE*22

//#define SMALL_DATASET_POPULATION            1000000
//#define LARGE_DATASET_POPULATION            200000000 
#define SMALL_DATASET_POPULATION            70000*4
#define LARGE_DATASET_POPULATION            240000
#define NUM_ELEMENTS_PER_BUCKET_LARGE       1024
#define NUM_ELEMENTS_PER_BUCKET_SMALL       32
#define NUM_BUCKETS_SMALL_DATASET           SHORT_STRUCTURE_LIMIT/NUM_ELEMENTS_PER_BUCKET_SMALL
#define NUM_BUCKETS_LARGE_DATASET           (LARGE_STRUCTURE_LIMIT-SHORT_STRUCTURE_LIMIT)/NUM_ELEMENTS_PER_BUCKET_LARGE
#define NUM_BUCKETS_CONVERTED           SHORT_STRUCTURE_LIMIT/NUM_ELEMENTS_PER_BUCKET_LARGE
#define NUM_PAGES 62
#define DATASET_POPULATION  280000
#define rmb()           asm volatile ("sync" ::: "memory")
#define THREAD_ID_HELPER 1

#define TOTAL_NUMBER_POSITIONS 100000
#include "tm.h"

//Metadata structure
typedef struct{
        int page_id;                    
        float time_to_move;       
        long num_threads_using;
        int lock_bit;
}page_metadata;

/* Test functions prototypes */
void* test(void* data);

#endif
