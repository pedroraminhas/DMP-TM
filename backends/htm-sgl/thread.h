/* =============================================================================
 *
 * thread.h
 *
 * =============================================================================
 *
 * Copyright (C) Stanford University, 2006.  All Rights Reserved.
 * Author: Chi Cao Minh
 *
 * =============================================================================
 *
 * For the license of bayes/sort.h and bayes/sort.c, please see the header
 * of the files.
 * 
 * ------------------------------------------------------------------------
 * 
 * For the license of kmeans, please see kmeans/LICENSE.kmeans
 * 
 * ------------------------------------------------------------------------
 * 
 * For the license of ssca2, please see ssca2/COPYRIGHT
 * 
 * ------------------------------------------------------------------------
 * 
 * For the license of lib/mt19937ar.c and lib/mt19937ar.h, please see the
 * header of the files.
 * 
 * ------------------------------------------------------------------------
 * 
 * For the license of lib/rbtree.h and lib/rbtree.c, please see
 * lib/LEGALNOTICE.rbtree and lib/LICENSE.rbtree
 * 
 * ------------------------------------------------------------------------
 * 
 * Unless otherwise noted, the following license applies to STAMP files:
 * 
 * Copyright (c) 2007, Stanford University
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 * 
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 * 
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in
 *       the documentation and/or other materials provided with the
 *       distribution.
 * 
 *     * Neither the name of Stanford University nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY STANFORD UNIVERSITY ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL STANFORD UNIVERSITY BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 *
 * =============================================================================
 */


#ifndef THREAD_H
#define THREAD_H 1



#include <sys/mman.h>
#include <fcntl.h>    /* For O_RDWR */
#include <unistd.h>

// Constants
extern unsigned long MB;    
extern unsigned long PAGE_SIZE; 
extern unsigned long NUM_PAGES;
extern unsigned long NUM_ELEMENTS_PAGE;    
extern unsigned long NUM_ELEMENTS_ARRAY;
# define CACHE_LINE_SIZE 128

typedef struct padded_scalar {
    volatile unsigned long counter;
    char suffixPadding[CACHE_LINE_SIZE];
} __attribute__((aligned(CACHE_LINE_SIZE))) padded_scalar_t;

typedef struct padded_float {
    volatile unsigned long counter;
    char suffixPadding[CACHE_LINE_SIZE];
} __attribute__((aligned(CACHE_LINE_SIZE))) padded_float_t;

typedef struct padded_statistics {
    unsigned long htm_commits;
    unsigned long sgl_commits;
    unsigned long footprint;
    unsigned long other_abort;
    unsigned long conflict;
    unsigned long user_abort;
    unsigned long nontrans;
    unsigned long lookup_HTM;
    volatile unsigned long lookup_STM;
    unsigned long lookup_STM_small;
    unsigned long insert_HTM;
    unsigned long insert_STM;
    unsigned long insert_STM_small;
    char suffixPadding[CACHE_LINE_SIZE];
} __attribute__((aligned(CACHE_LINE_SIZE))) padded_statistics_t;

extern __attribute__((aligned(CACHE_LINE_SIZE))) padded_statistics_t statistics_array[];

extern __attribute__((aligned(CACHE_LINE_SIZE))) padded_scalar_t single_global_lock;

typedef struct padded_pointer {
            unsigned char* pointer;
                char suffixPadding[CACHE_LINE_SIZE];
} __attribute__((aligned(CACHE_LINE_SIZE))) padded_pointer_t;

//Metadata structure
typedef struct{
        int status_field;       //protection of pge
        int writer_count;       //how many STM writers
        int transition_count;   //how many times HTM reverted to normal R/W access
        int lock_bit;           //Indicates if a system call is being issued
}metadata;

extern __thread unsigned int local_exec_mode;

extern metadata *metadata_array;

extern __thread char* memory_requested;
extern __attribute__((aligned(CACHE_LINE_SIZE))) padded_pointer_t HTM_pointer[];
extern __attribute__((aligned(CACHE_LINE_SIZE))) padded_pointer_t STM_pointer[];
extern long round_robin_counter;
extern int number_threads;
extern long memory_per_thread;
extern unsigned long page_first_address;
extern unsigned long STM_page_first_address;
extern unsigned long offset_heaps;

//Memory is mapped to two heaps: one for HTM and another for STM
extern unsigned char* HTM_Heap;
extern unsigned char* STM_Heap;

//indicates wheter is an update hardware transaction or not 
extern __thread int update_transaction;
extern int total_commits;
extern __thread long threadID;
extern __thread unsigned int thread_id;

//extern __thread int tries;

//extern volatile __thread int mode;
//extern __attribute__((aligned(CACHE_LINE_SIZE))) padded_scalar_t capacity[];
////extern __attribute__((aligned(CACHE_LINE_SIZE))) padded_scalar_t revive_mode[];
extern int disjoint_partitions;
extern __attribute__((aligned(CACHE_LINE_SIZE))) padded_scalar_t mode[][80];
extern __attribute__((aligned(CACHE_LINE_SIZE))) padded_scalar_t mode2[];
extern __thread int num_tx;
#ifndef REDUCED_TM_API

#include <pthread.h>
#include <stdlib.h>
#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif


#define THREAD_T                            pthread_t
#define THREAD_ATTR_T                       pthread_attr_t

#define THREAD_ATTR_INIT(attr)              pthread_attr_init(&attr)
#define THREAD_JOIN(tid)                    pthread_join(tid, (void**)NULL)
#define THREAD_CREATE(tid, attr, fn, arg)   pthread_create(&(tid), \
                                                           &(attr), \
                                                           (void* (*)(void*))(fn), \
                                                           (void*)(arg))

#define THREAD_LOCAL_T                      pthread_key_t
#define THREAD_LOCAL_INIT(key)              pthread_key_create(&key, NULL)
#define THREAD_LOCAL_SET(key, val)          pthread_setspecific(key, (void*)(val))
#define THREAD_LOCAL_GET(key)               pthread_getspecific(key)

#define THREAD_MUTEX_T                      pthread_mutex_t
#define THREAD_MUTEX_INIT(lock)             pthread_mutex_init(&(lock), NULL)
#define THREAD_MUTEX_LOCK(lock)             pthread_mutex_lock(&(lock))
#define THREAD_MUTEX_UNLOCK(lock)           pthread_mutex_unlock(&(lock))

#define THREAD_COND_T                       pthread_cond_t
#define THREAD_COND_INIT(cond)              pthread_cond_init(&(cond), NULL)
#define THREAD_COND_SIGNAL(cond)            pthread_cond_signal(&(cond))
#define THREAD_COND_BROADCAST(cond)         pthread_cond_broadcast(&(cond))
#define THREAD_COND_WAIT(cond, lock)        pthread_cond_wait(&(cond), &(lock))

#  define THREAD_BARRIER_T                  barrier_t
#  define THREAD_BARRIER_ALLOC(N)           barrier_alloc()
#  define THREAD_BARRIER_INIT(bar, N)       barrier_init(bar, N)
#  define THREAD_BARRIER(bar, tid)          barrier_cross(bar)
#  define THREAD_BARRIER_FREE(bar)          barrier_free(bar)

typedef struct barrier {
    pthread_cond_t complete;
    pthread_mutex_t mutex;
    int count;
    int crossing;
} barrier_t;

barrier_t *barrier_alloc();

void barrier_free(barrier_t *b);

void barrier_init(barrier_t *b, int n);

void barrier_cross(barrier_t *b);

void thread_startup (long numThread);

void thread_start (void (*funcPtr)(void*), void* argPtr);

void thread_shutdown ();

void thread_barrier_wait();

long thread_getId();

long thread_getNumThread();

#ifdef __cplusplus
}
#endif

#endif

#endif /* THREAD_H */


/* =============================================================================
 *
 * End of thread.h
 *
 * =============================================================================
 */
