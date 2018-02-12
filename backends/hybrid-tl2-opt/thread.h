#ifndef THREAD_H
#define THREAD_H 1

#include <pthread.h>
#include <stdlib.h>
#include "types.h"

//#include <immintrin.h>
//#include <rtmintrin.h>

#include "stm_tl2.h"
#include "tl2.h"


# define CACHE_LINE_SIZE 128

extern __thread vwLock next_commit;

typedef struct padded_scalar {
    volatile unsigned long counter;
    char suffixPadding[CACHE_LINE_SIZE];
} __attribute__((aligned(CACHE_LINE_SIZE))) padded_scalar_t;

typedef struct padded_statistics {
    unsigned long htm_commits;
    unsigned long stm_commits;
    unsigned long stm_aborts;
    unsigned long conflicts;
    unsigned long nontrans;
    unsigned long capacity_aborts;
    unsigned long user_aborts;
    unsigned long other_aborts;
    unsigned long sgl_commits;
    unsigned long system_calls;
    unsigned long htm_system_calls;
    unsigned long fallback_to_STM;
    unsigned long lookup_HTM;
    unsigned long lookup_STM;
    unsigned long lookup_STM_small;
    unsigned long insert_HTM;
    unsigned long insert_STM;
    unsigned long insert_STM_small;
    char suffixPadding[CACHE_LINE_SIZE];
} __attribute__((aligned(CACHE_LINE_SIZE))) padded_statistics_t;

typedef struct padded_float {
            volatile float counter;
                char suffixPadding[CACHE_LINE_SIZE];
} __attribute__((aligned(CACHE_LINE_SIZE))) padded_float_t;

typedef struct padded_pointer {
    long* pointer;
    char suffixPadding[CACHE_LINE_SIZE];
} __attribute__((aligned(CACHE_LINE_SIZE))) padded_pointer_t;

extern __attribute__((aligned(CACHE_LINE_SIZE))) padded_statistics_t statistics_array[];

extern __attribute__((aligned(CACHE_LINE_SIZE))) padded_scalar_t fallback_in_use;

extern __attribute__((aligned(CACHE_LINE_SIZE))) padded_pointer_t HTM_pointer[];
extern __attribute__((aligned(CACHE_LINE_SIZE))) padded_pointer_t STM_pointer[];
extern unsigned long MB;    
extern unsigned long PAGE_SIZE; 
extern unsigned long NUM_PAGES;
extern unsigned long NUM_ELEMENTS_PAGE;    
extern unsigned long NUM_ELEMENTS_ARRAY;
extern __thread unsigned int local_exec_mode ;
extern long* STM_Heap;
extern long* HTM_Heap;
extern __attribute__((aligned(CACHE_LINE_SIZE))) padded_scalar_t mode2[];
extern __thread int num_tx;
extern long total_commits;
extern __thread int tries;

extern __thread long threadID;
extern long memory_per_thread;
#ifndef REDUCED_TM_API


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
#define THREAD_MUTEX_INIT(lock)             pthread_spin_init(&(lock), NULL)
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

//void* TM_MALLOC(int size);

#ifdef __cplusplus
}
#endif

#endif

#endif /* THREAD_H */
