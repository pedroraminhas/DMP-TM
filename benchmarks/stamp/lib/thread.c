#ifndef _GNU_SOURCE
#define _GNU_SOURCE 1
#endif

#include "thread.h"
#include "tm.h"

__thread sigjmp_buf point;

__attribute__((aligned(CACHE_LINE_SIZE))) padded_statistics_t statistics_array[80];
//__attribute__((aligned(CACHE_LINE_SIZE))) padded_scalar_t capacity[20];

__attribute__((aligned(CACHE_LINE_SIZE))) padded_scalar_t single_global_lock;

__attribute__((aligned(CACHE_LINE_SIZE))) padded_scalar_t thread_lock[MAX_NUM_THREADS];
__thread unsigned short thread_id;

//__attribute__((aligned(CACHE_LINE_SIZE))) padded_scalar_t revive_mode[20];

__attribute__((aligned(CACHE_LINE_SIZE))) padded_scalar_t mode2[20];

__attribute__((aligned(CACHE_LINE_SIZE))) padded_scalar_t mode[20][80];

__thread __attribute__((aligned(CACHE_LINE_SIZE))) padded_scalar_t capacity[5];
__thread __attribute__((aligned(CACHE_LINE_SIZE))) padded_scalar_t htm_commits[5];


__thread int use_STM=0;

__attribute__((aligned(CACHE_LINE_SIZE))) padded_scalar_t read_set[80];

__attribute__((aligned(CACHE_LINE_SIZE))) padded_scalar_t commits;

__attribute__((aligned(CACHE_LINE_SIZE))) padded_scalar_t write_set[80];

__attribute__((aligned(CACHE_LINE_SIZE))) padded_pointer_t STM_pointer[80];
__thread int incremented_lock_flag=0;
__thread unsigned int local_exec_mode = 0;
//Array of metadata of each page
metadata *metadata_array;

//Distance between the begginning of both heaps
unsigned long offset_heaps;

char* STM_Heap;
char* HTM_Heap;

// Constants
 unsigned long MB;    
 unsigned long PAGE_SIZE; 
 unsigned long NUM_PAGES;
 unsigned long NUM_ELEMENTS_PAGE;    
 unsigned long NUM_ELEMENTS_ARRAY;

//Memory is mapped to two heaps: one for HTM and another for STM


//Set of pages read and written by STM transactions
//__thread unsigned long pages_read;
//__thread unsigned long pages_written;
__thread long pages_read;
__thread long pages_written;

int pages_read_heap[80][20000000];
int pages_written_heap[80][20000000];
__thread long counter_pages_read=0;
__thread long counter_pages_written=0;
int disjoint_partitions=0;
//Global and local counter used to count how many increases of the transition count exists in the moment
int global_transition_count=0;
__thread int local_transition_count=0;
int transition_count_pages_read[80][20000000];

int writer_count[80][20000000];
//First address of the page that contains the HTM_Heap
//Usually the address is the same as the HTM_Heap
unsigned long page_first_address;
unsigned long STM_page_first_address;

int total_commits=0;
int number_threads=0;
//__thread int mode=0;
//__thread int mode[20]={0};
//int mode2[20]={0};
//__thread float capacity[20];
__thread int num_tx=0;
__thread int update_transaction=0;
long round_robin_counter=0;
__thread char* memory_requested;
long memory_per_thread;
__thread int revive_mode[20]={0};
__thread long commit_threshold=1000;
__thread long threadID;
#ifndef REDUCED_TM_API

#include <assert.h>
#include <stdlib.h>
#include <sched.h>
#include "types.h"
#include "random.h"
//#include "rapl.h"

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
            //endEnergy();
            break;
        }
    }
}

void thread_startup (long numThread)
{
    long i;

    global_numThread = numThread;
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

    //startEnergy();

    /* Set up pool */
    THREAD_ATTR_INIT(global_threadAttr);
    for (i = 1; i < numThread; i++) {
        THREAD_CREATE(global_threads[i],
                      global_threadAttr,
                      &threadWait,
                      &global_threadIds[i]);
    }

    /*
     * Wait for primary thread to call thread_start
     */
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

/*
 * Functions used by Hybrid-Partition
 */

inline int ask_oracle(int pageID,metadata* metadata_array,int update_transaction){

    if(metadata_array[pageID].transition_count >= MAX_NUM_TRANSITIONS)
        return PAGE_IS_FOR_STM;

    if(update_transaction)
        return CHANGE_PROT_HTM_READ_WRITE;

    if(update_transaction == 0)
        return CHANGE_PROT_HTM_READ;
}

inline void change_protection_HTM(int protection,int pageID,long* HTM_Heap,metadata* metadata_array){
    long increment_inside_loop=0;

    BEGIN:while(__sync_val_compare_and_swap(&metadata_array[pageID].lock_bit,0,1)==0){
            increment_inside_loop++;
            //__asm__ ("nop;");  
          }

  //  printf("Acquire lock_bit by THREAD ID = %d\n", thread_getId());

    if(metadata_array[pageID].writer_count == 0){
    // printf("writer count equal 0\n");
    // printf("status field is %d and protection is %d\n",metadata_array[pageID].status_field, protection); 

       if(((metadata_array[pageID].status_field== (PROT_READ) || metadata_array[pageID].status_field== (PROT_NONE)) && (protection == (PROT_READ|PROT_WRITE))) || (metadata_array[pageID].status_field== (PROT_NONE) && (protection == PROT_READ))){
            mprotect((void *)HTM_Heap+(PAGE_SIZE*pageID),PAGE_SIZE,protection);
            metadata_array[pageID].status_field = protection;

            //system_call_counter[pageID]++;
           // printf("Protection changed to %d by THREAD ID %d\n",protection,thread_getId());

            if(protection ==(PROT_READ| PROT_WRITE)){
                __sync_add_and_fetch(&metadata_array[pageID].transition_count,1);
                __sync_add_and_fetch(&global_transition_count,1);

                printf("global counter was incremented, now its value is %d\n",global_transition_count);

         //       transition_count_counter[pageID]++;
         //       printf("TRANSITION_COUNT incremented, now is %d\n",metadata_array[pageID].transition_count);
            }
     //   printf("Release lock bit after change protection\n");
        metadata_array[pageID].lock_bit=0;
        }else{
       //     printf("Release lock bit because protection is already changed by another thread\n");
            metadata_array[pageID].lock_bit=0;
        }
    }else{
       // printf("Release lock bit because there are writers active\n");
        metadata_array[pageID].lock_bit=0;
        while(metadata_array[pageID].writer_count!=0){
          increment_inside_loop++;
            //__asm__ ("nop;");
        }
       // printf("done waiting for writer_count\n");
        goto BEGIN;
        }
}

/*
 * Defines an action to be done upon receiving the acess violation
 * Action variable could represent several responses:
 *
 * 0 - Change protection of HTM to #Read
 * 1 - Change protection of HTM to #Read/Write
 * 2 - Page is from STM
 */

inline int AV_handler (long* address, long* HTM_Heap, metadata* metadata_array,unsigned long offset_heaps,int pageID,int update_transaction){

    int action= ask_oracle(pageID,metadata_array,update_transaction);

    switch(action) {
            //change protection of HTM to #Read
            case CHANGE_PROT_HTM_READ:
     //               printf("Change protection to READ\n");
                    change_protection_HTM(PROT_READ,pageID,HTM_Heap,metadata_array);
                    return 0;

            case CHANGE_PROT_HTM_READ_WRITE:
    //                printf("Change protection to READ/WRITE\n");
                    change_protection_HTM(PROT_READ | PROT_WRITE,pageID,HTM_Heap,metadata_array);
                    return 0;

            case PAGE_IS_FOR_STM:
      //              printf("Give page to STM\n");
                    return 1;
    }
}

inline int get_pageID_of_address(long* address){

 //       printf("main.c address=%lx page_first_address=%lx\n",address,page_first_address);
    return ((long int)address - (long int)page_first_address)>>16;
}

inline void handler (int sig, siginfo_t *si, void *unused){
        //int pageID = get_pageID_of_address((long)si->si_addr);
        long pageID = ((long int)si->si_addr - (long int)HTM_Heap) >> 16; 
        //rmb();
        //printf("\nAccess Violation at PAGE %ld (%ld) with THREAD ID = %d with mode %d (%d)\n",pageID,NUM_PAGES,thread_getId(),mode[num_tx],num_tx);
        int result=0;
        //printf("%d: %ld\n",threadID,num_tx);
         if((pageID<0) || (pageID>NUM_PAGES)){
                printf("num_tx is %ld\n",num_tx);
                printf("something is wrong: %ld\n",pageID);
                exit(0);
                longjmp(point, 1); 
                disjoint_partitions=1;
                //SLOW_PATH_RESTART();
                //mprotect((void *)HTM_Heap+(PAGE_SIZE*NUM_PAGES),PAGE_SIZE,PROT_READ|PROT_WRITE);
                 //sigsetjmp(*_e, 0);
                //exit(0);
        }else{  
           result=AV_HANDLER(pageID);
            if(result){
                use_STM=1;
            }   
        }   
}

#endif
