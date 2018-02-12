#ifndef _GNU_SOURCE
#define _GNU_SOURCE 1
#endif

#include "thread.h"

unsigned long MB;    
unsigned long PAGE_SIZE; 
unsigned long NUM_PAGES;
unsigned long NUM_ELEMENTS_PAGE;    
unsigned long NUM_ELEMENTS_ARRAY;

__attribute__((aligned(CACHE_LINE_SIZE))) padded_scalar_t fallback_in_use;

__attribute__((aligned(CACHE_LINE_SIZE))) padded_scalar_t exists_sw;

__attribute__((aligned(CACHE_LINE_SIZE))) __thread unsigned short read_only_htm;

__attribute__((aligned(CACHE_LINE_SIZE))) padded_statistics_t statistics_array[128];

//Array of metadata of each page
metadata *metadata_array;

//__thread int tries;
//Memory is mapped to two heaps: one for HTM and another for STM
unsigned long* HTM_Heap;
unsigned long* STM_Heap;

//indicates wheter is an update hardware transaction or not 
__thread int update_transaction = 0;
long total_commits;
//__thread int mode;
__attribute__((aligned(CACHE_LINE_SIZE))) padded_scalar_t mode[20][80];
__attribute__((aligned(CACHE_LINE_SIZE))) padded_scalar_t mode2[20];
__thread int num_tx;
int disjoint_partitions=0;
__thread unsigned int local_exec_mode ;
__attribute__((aligned(CACHE_LINE_SIZE))) padded_float_t balances[100];
#ifndef REDUCED_TM_API

#include <assert.h>
#include <stdlib.h>
#include <sched.h>
#include "types.h"
//#include "rapl.h"
//#include <random.h>

unsigned long STM_page_first_address;
__thread long* memory_requested;
__attribute__((aligned(CACHE_LINE_SIZE))) padded_pointer_t HTM_pointer[];
__attribute__((aligned(CACHE_LINE_SIZE))) padded_pointer_t STM_pointer[];
long round_robin_counter;
int number_threads;
long memory_per_thread;
unsigned long page_first_address;
unsigned long STM_page_first_address;
unsigned long offset_heaps;
__thread unsigned short thread_id;
__thread long threadID;
#include <math.h>

static THREAD_LOCAL_T    global_threadId;
static long              global_numThread       = 1;
static THREAD_BARRIER_T* global_barrierPtr      = NULL;
static long*             global_threadIds       = NULL;
static THREAD_ATTR_T     global_threadAttr;
static THREAD_T*         global_threads         = NULL;
static void            (*global_funcPtr)(void*) = NULL;
static void*             global_argPtr          = NULL;
static volatile bool_t   global_doShutdown      = FALSE;

#if defined(__powerpc__) || defined(__ppc__) || defined(__PPC__)
void bindThread(long threadId) {
    cpu_set_t my_set;
    CPU_ZERO(&my_set);
    int offset = threadId / 10; 
    CPU_SET((threadId % 10)*8+offset, &my_set);
    sched_setaffinity(0, sizeof(cpu_set_t), &my_set);
}
#endif

static void threadWait (void* argPtr)
{
    long threadId = *(long*)argPtr;
    
    THREAD_LOCAL_SET(global_threadId, (long)threadId);

    bindThread(threadId);

    while (1) {
        THREAD_BARRIER(global_barrierPtr, threadId); /* wait for start parallel */
        if (global_doShutdown) {
            break;
        }
        global_funcPtr(global_argPtr);
        THREAD_BARRIER(global_barrierPtr, threadId); /* wait for end parallel */
        if (threadId == 0) {
            break;
        }
    }
}

void thread_startup (long numThread)
{
    long i;

    global_numThread = numThread;
	if(numThread==80)
        global_numThread = 79;
    global_doShutdown = FALSE;

    /* Set up barrier */
    assert(global_barrierPtr == NULL);
    global_barrierPtr = THREAD_BARRIER_ALLOC(numThread);
    assert(global_barrierPtr);
    THREAD_BARRIER_INIT(global_barrierPtr, numThread);

    /* Set up ids */
    THREAD_LOCAL_INIT(global_threadId);
    assert(global_threadIds == NULL);
    global_threadIds = (long*)malloc(numThread * sizeof(long));
    assert(global_threadIds);
    for (i = 0; i < numThread; i++) {
        global_threadIds[i] = i;
    }

    /* Set up thread list */
    assert(global_threads == NULL);
    global_threads = (THREAD_T*)malloc(numThread * sizeof(THREAD_T));
    assert(global_threads);

    exists_sw.counter = 0;
    fallback_in_use.counter = 0;

	//pthread_mutex_init(&cas_mutex, NULL);

    /* Set up pool */
    THREAD_ATTR_INIT(global_threadAttr);
    for (i = 1; i < numThread; i++) {
        THREAD_CREATE(global_threads[i],
                      global_threadAttr,
                      &threadWait,
                      &global_threadIds[i]);
    }
}

void thread_start (void (*funcPtr)(void*), void* argPtr)
{
    global_funcPtr = funcPtr;
    global_argPtr = argPtr;

    long threadId = 0; /* primary */
    threadWait((void*)&threadId);
}


void thread_shutdown ()
{
    /* Make secondary threads exit wait() */
    global_doShutdown = TRUE;
    THREAD_BARRIER(global_barrierPtr, 0);

    long numThread = global_numThread;

    long i;
    for (i = 1; i < numThread; i++) {
        THREAD_JOIN(global_threads[i]);
    }

    THREAD_BARRIER_FREE(global_barrierPtr);
    global_barrierPtr = NULL;

    free(global_threadIds);
    global_threadIds = NULL;

    free(global_threads);
    global_threads = NULL;

    global_numThread = 1;
}

barrier_t *barrier_alloc() {
    return (barrier_t *)malloc(sizeof(barrier_t));
}

void barrier_free(barrier_t *b) {
    free(b);
}

void barrier_init(barrier_t *b, int n) {
    pthread_cond_init(&b->complete, NULL);
    pthread_mutex_init(&b->mutex, NULL);
    b->count = n;
    b->crossing = 0;
}

void barrier_cross(barrier_t *b) {
    pthread_mutex_lock(&b->mutex);
    /* One more thread through */
    b->crossing++;
    /* If not all here, wait */
    if (b->crossing < b->count) {
        pthread_cond_wait(&b->complete, &b->mutex);
    } else {
        /* Reset for next time */
        b->crossing = 0;
        pthread_cond_broadcast(&b->complete);
    }
    pthread_mutex_unlock(&b->mutex);
}

void thread_barrier_wait()
{
    long threadId = thread_getId();
    THREAD_BARRIER(global_barrierPtr, threadId);
}


long thread_getId()
{
    return (long)THREAD_LOCAL_GET(global_threadId);
}


long thread_getNumThread()
{
    return global_numThread;
}

#endif
