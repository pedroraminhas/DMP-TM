#ifndef _GNU_SOURCE
#define _GNU_SOURCE 1
#endif

#include "thread.h"
#include "tm.h"


__attribute__((aligned(CACHE_LINE_SIZE))) padded_statistics_t statistics_array[80];

__attribute__((aligned(CACHE_LINE_SIZE))) padded_scalar_t single_global_lock;

__attribute__((aligned(CACHE_LINE_SIZE))) padded_scalar_t thread_lock[MAX_NUM_THREADS];

__attribute__((aligned(CACHE_LINE_SIZE))) padded_scalar_t sched_mode[5];
__thread __attribute__((aligned(CACHE_LINE_SIZE))) padded_scalar_t sched_capacity[5];
__thread __attribute__((aligned(CACHE_LINE_SIZE))) padded_scalar_t sched_htm_commits[5];


__attribute__((aligned(CACHE_LINE_SIZE))) padded_pointer_t STM_pointer[80];
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

long pages_read_heap[80][20000000];
long pages_written_heap[80][20000000];
__thread long counter_pages_read=0;
__thread long counter_pages_written=0;
//Global and local counter used to count how many increases of the transition count exists in the moment
volatile long global_transition_count=0;
__thread long local_transition_count=0;
__thread long* transition_count_pages_read;

volatile long writer_count[80][20000000];
//First address of the page that contains the HTM_Heap
//Usually the address is the same as the HTM_Heap
unsigned long page_first_address;
unsigned long STM_page_first_address;

int number_threads=0;
//__thread int mode=0;
//__thread int mode[20]={0};
//int mode2[20]={0};
//__thread float capacity[20];
__thread int num_tx=0;
__thread int update_transaction=0;
long memory_per_thread;
int sched_threshold = 90;
__thread long threadID;
volatile int htm_retries;
volatile int bailout = 0;
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

    transition_count_pages_read = (int *) calloc(NUM_PAGES,sizeof(int));


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
    single_global_lock.counter = 0;
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


inline void handler (int sig, siginfo_t *si, void *unused){
        long pageID = ((long int)si->si_addr - (long int)HTM_Heap) >> 16; 
        int result=0;
         if((pageID<0) || (pageID>NUM_PAGES)){
                printf("num_tx is %ld\n",num_tx);
                printf("something is wrong: %ld\n",pageID);
                exit(0);
        }else{  
           AV_HANDLER(pageID);
        }   
}

void start_monitoring(){
    int syscall_duration = 9;
    int syscall_threshold = 500*0.4; 
    int i,exitFlag=0;
    int w;
    double ratio_system_calls=0;
    double ratio_system_calls_old=0;
    long stm_system_calls_old=0; long htm_system_calls_old=0;
    int begin_loop_counter;
    cpu_set_t my_set;
    CPU_ZERO(&my_set);
    CPU_SET(79, &my_set);
    sched_setaffinity(0, sizeof(cpu_set_t), &my_set);
    long array_htm[10];
    long array_stm[10];
    long stm_system_calls,htm_system_calls,total_system_calls,htm_commits,stm_commits,sgl_commits,current_htm_commits,current_stm_commits;
    htm_commits = 0;
    stm_commits = 0;
	sgl_commits = 0; 
	current_htm_commits = 0;
	current_stm_commits = 0;
    long system_calls_new=0;
    long system_calls_old=0;
	long commits_new = 0;
	long commits_old = 0;
    int k=0;
    while(!global_doShutdown && !exitFlag){
        htm_system_calls=0;
        stm_system_calls=0;
        for(i=0;i<global_numThread;i++){
            htm_system_calls+=statistics_array[i].htm_system_calls;
            stm_system_calls+=statistics_array[i].system_calls;
        }
        system_calls_new= htm_system_calls+stm_system_calls;
        if(system_calls_old!=0){
                if((system_calls_new-system_calls_old)* syscall_duration > syscall_threshold ){
                    htm_retries = 10;
                    bailout = 1;
                    asm volatile ("sync" ::: "memory");
                    printf("bailout\n");
                    for(i=0;i<5;i++){
                        sched_mode[i].counter=0;
                    }
					/*do{
            			long w=1;
            			for (begin_loop_counter=0;begin_loop_counter<number_threads;begin_loop_counter++){
                    		if(thread_lock[begin_loop_counter].counter){
                                    printf(""); 
                                    w = 0;
                                    break;
                            }
            			}
        			}while(!w);*/
    				for(i=0;i<global_numThread;i++){
				        htm_commits+=statistics_array[i].htm_commits;
				        sgl_commits+=statistics_array[i].sgl_commits;
				    }
				    commits_old = htm_commits + sgl_commits;
                    htm_commits = 0;
                    sgl_commits = 0;
				    usleep(10000);
				    for(i=0;i<global_numThread;i++){
				        htm_commits+=statistics_array[i].htm_commits;
				        sgl_commits+=statistics_array[i].sgl_commits;
				    }
				    commits_new = htm_commits + sgl_commits;
				    current_htm_commits = commits_new - commits_old;
                    single_global_lock.counter = 1;
                    asm volatile ("sync" ::: "memory");
				    for(i=0;i<5;i++){
				        sched_mode[i].counter=2;
				    }
				    single_global_lock.counter = 0;
				    asm volatile ("sync" ::: "memory");
				    for(i=0;i<global_numThread;i++)
				        stm_commits+=statistics_array[i].stm_commits;
				    commits_old = stm_commits;
                    stm_commits = 0;
				    usleep(10000);
				    for(i=0;i<global_numThread;i++)
				        stm_commits+=statistics_array[i].stm_commits;
				    current_stm_commits = stm_commits - commits_old;
                    if(current_stm_commits < current_htm_commits){
						for(i=0;i<5;i++){
                        	sched_mode[i].counter=0;
                    	}
                    }
					break;
                }
        }
        system_calls_old = system_calls_new;
        ratio_system_calls_old=ratio_system_calls;
        usleep(1000);
    }
    pthread_exit(NULL);
}

void start_bt(){
    pthread_t t1;
    pthread_create(&t1, NULL, start_monitoring, NULL);
}

#endif
