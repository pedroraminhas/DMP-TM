#ifndef THREAD_H
#define THREAD_H 1

//#  include "stm_tinystm.h"
#include <signal.h>
#include <sys/mman.h>
#include <setjmp.h>

// Constants
# define CACHE_LINE_SIZE 128
//# define NUM_PAGES NUM_ELEMENTS_ARRAY/NUM_ELEMENTS_PAGE
# define CHANGE_PROT_HTM_READ 0
# define CHANGE_PROT_HTM_READ_WRITE 1
# define PAGE_IS_FOR_STM 2
# define MAX_NUM_TRANSITIONS 5


// Constants
extern unsigned long MB;    
extern unsigned long PAGE_SIZE; 
extern unsigned long NUM_PAGES;
extern unsigned long NUM_ELEMENTS_PAGE;    
extern unsigned long NUM_ELEMENTS_ARRAY;
#define rmb()           asm volatile ("sync" ::: "memory")
# define LOCKED(lock)        *((volatile long*)(&lock)) != 0

# define MAX_NUM_THREADS 80

extern __thread sigjmp_buf point;

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

typedef struct padded_scalar {
    volatile unsigned long counter;
    char suffixPadding[CACHE_LINE_SIZE];
} __attribute__((aligned(CACHE_LINE_SIZE))) padded_scalar_t;

typedef struct padded_pointer {
    char* pointer;
    char suffixPadding[CACHE_LINE_SIZE];
} __attribute__((aligned(CACHE_LINE_SIZE))) padded_pointer_t;

extern __attribute__((aligned(CACHE_LINE_SIZE))) padded_statistics_t statistics_array[];

extern __attribute__((aligned(CACHE_LINE_SIZE))) padded_scalar_t single_global_lock;
extern char* STM_Heap;
extern char* HTM_Heap;
extern __attribute__((aligned(CACHE_LINE_SIZE))) padded_scalar_t read_set[];
extern __attribute__((aligned(CACHE_LINE_SIZE))) padded_scalar_t commits;

extern __attribute__((aligned(CACHE_LINE_SIZE))) padded_scalar_t write_set[];

extern __attribute__((aligned(CACHE_LINE_SIZE))) padded_scalar_t single_global_lock;
extern __attribute__((aligned(CACHE_LINE_SIZE))) padded_scalar_t thread_lock[];

extern __attribute__((aligned(CACHE_LINE_SIZE))) padded_pointer_t STM_pointer[];

//extern __attribute__((aligned(CACHE_LINE_SIZE))) padded_scalar_t capacity[];
//extern __attribute__((aligned(CACHE_LINE_SIZE))) padded_scalar_t revive_mode[];
extern __attribute__((aligned(CACHE_LINE_SIZE))) padded_scalar_t mode2[];

extern __attribute__((aligned(CACHE_LINE_SIZE))) padded_scalar_t mode[][80];

extern __attribute__((aligned(CACHE_LINE_SIZE))) padded_scalar_t capacity[][80];

extern __thread int incremented_lock_flag;

extern __thread unsigned short thread_id;

//Indicates that transactions that use HTM, now have to revert to STM
extern __thread int use_STM;
//indicates wheter is an update hardware transaction or not
extern __thread int update_transaction;
extern int disjoint_partitions;
//Counters of the benchmark in order to count the number of system calls and transitions counts
extern int system_call_counter[16];
extern int transition_count_counter[16];

//First address of the page that contains the HTM_Heap
//Usually the address is the same as the HTM_Heap
extern unsigned long page_first_address;
extern unsigned long STM_page_first_address;

extern int global_transition_count;
extern int writer_count[80][20000000];

//Metadata structure
typedef struct{
        long status_field;       //protection of pge
        long writer_count;       //how many STM writers
        long transition_count;   //how many times HTM reverted to normal R/W access
        long lock_bit;           //Indicates if a system call is being issued
}metadata;

extern metadata *metadata_array;

typedef struct padded_float {
            volatile float counter;
                char suffixPadding[CACHE_LINE_SIZE];
} __attribute__((aligned(CACHE_LINE_SIZE))) padded_float_t;

//Distance between the begginning of both heaps
extern unsigned long offset_heaps;


//Set of pages read and written by STM transactions
//extern __thread unsigned long pages_read;
//extern __thread unsigned long pages_written;
extern __thread long pages_read;
extern __thread long pages_written;

extern int pages_read_heap[80][20000000];
extern int pages_written_heap[80][20000000];
extern __thread long counter_pages_read;
extern __thread long counter_pages_written;

//Global and local counter used to count how many increases of the transition count exists in the moment
extern int global_transition_count;
extern  __thread int local_transition_count;
extern int transition_count_pages_read[80][20000000];
extern long round_robin_counter;
extern __thread char* memory_requested;

extern int total_commits;
extern int number_threads;
//extern __thread int mode;
//extern __thread int mode[20];
//extern int mode2[20];
extern __thread int num_tx;
extern __thread int revive_mode[20];
extern __thread long commit_threshold;
extern long memory_per_thread;
extern __thread long threadID;
extern __thread unsigned int local_exec_mode ;
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

extern inline int ask_oracle(int pageID,metadata* metadata_array,int update_transaction);
extern inline void change_protection_HTM(int protection,int pageID,long* HTM_Heap,metadata* metadata_array);

# define ASK_ORACLE(pageID) ({ int return_ask_oracle=0; \
                                   if(metadata_array[pageID].transition_count >= MAX_NUM_TRANSITIONS) \
                                        return_ask_oracle=PAGE_IS_FOR_STM; \
                                   if(update_transaction) \
                                        return_ask_oracle=CHANGE_PROT_HTM_READ_WRITE;   \
                                    if(update_transaction == 0) \
                                        return_ask_oracle=CHANGE_PROT_HTM_READ; \
                                    return_ask_oracle; \
                            })

# define CHANGE_PROTECTION_HTM_READ(pageID) ({ \
    long increment_inside_loop=0; \
    long increment_page_loop=0; \
    long result; \
    BEGIN_HANDLER_READ:while(1){ \
                        while(LOCKED(metadata_array[pageID].lock_bit)){ \
                            __asm__ ("nop;"); \
                        } \
                        if(__sync_val_compare_and_swap(&metadata_array[pageID].lock_bit, 0, 1) == 0) { \
                            break; \
                        } \
                    } \
        result=0; \
        for (increment_inside_loop=0;increment_inside_loop<number_threads;increment_inside_loop++){ \
                if(writer_count[increment_inside_loop][pageID]){ result = 1; break;} \
        } \
    if(result == 0){ \
       if(metadata_array[pageID].status_field== (PROT_NONE)){ \
            mprotect((void *)HTM_Heap+(PAGE_SIZE*pageID),PAGE_SIZE,PROT_READ); \
            statistics_array[thread_getId()].htm_system_calls++; \
            metadata_array[pageID].status_field = PROT_READ; \
            metadata_array[pageID].lock_bit=0; \
        }else{ \
            metadata_array[pageID].lock_bit=0; \
        }   \
    }else{ \
        metadata_array[pageID].lock_bit=0;  \
        do{ \
        result=0; \
        for (increment_inside_loop=0;increment_inside_loop<number_threads;increment_inside_loop++){ \
                if(writer_count[increment_inside_loop][pageID]){ result = 1; break;} \
        } \
        }while(result!=0); \
        goto BEGIN_HANDLER_READ;  \
        }   \
})


# define CHANGE_PROTECTION_HTM_READ_WRITE(pageID) ({ \
    long increment_inside_loop=0; \
    long increment_page_loop=0; \
    long result; \
    BEGIN_HANDLER:while(1){ \
                        while(LOCKED(metadata_array[pageID].lock_bit)){ \
                            __asm__ ("nop;"); \
                        } \
                        if(__sync_val_compare_and_swap(&metadata_array[pageID].lock_bit, 0, 1) == 0) { \
                            break; \
                        } \
                    } \
        result=0; \
        for (increment_inside_loop=0;increment_inside_loop<number_threads;increment_inside_loop++){ \
                if(writer_count[increment_inside_loop][pageID]){ result = 1; break;} \
       } \
    if(result ==0){ \
    if(metadata_array[pageID].status_field== (PROT_READ) || metadata_array[pageID].status_field== (PROT_NONE)){ \
            mprotect((void *)HTM_Heap+(PAGE_SIZE*pageID),PAGE_SIZE,(PROT_READ| PROT_WRITE)); \
            statistics_array[thread_getId()].htm_system_calls++; \
            metadata_array[pageID].status_field = (PROT_READ| PROT_WRITE); \
            metadata_array[pageID].transition_count++;\
            __sync_add_and_fetch(&global_transition_count,1); \
        metadata_array[pageID].lock_bit=0; \
        }else{ \
            metadata_array[pageID].lock_bit=0; \
        }   \
    }else{ \
        metadata_array[pageID].lock_bit=0;  \
        do{ \
        result=0; \
        for (increment_inside_loop=0;increment_inside_loop<number_threads;increment_inside_loop++){ \
                if(writer_count[increment_inside_loop][pageID]){ result = 1; break;} \
        } \
         asm volatile ("" ::: "memory"); \
        }while(result!=0); \
        goto BEGIN_HANDLER;  \
        }   \
})


# define AV_HANDLER(pageID)({/*int action= ask_oracle(pageID,metadata_array,update_transaction);*/\
                             int action = ASK_ORACLE((pageID)); \
                             int return_value; \
                                    switch(action) {  \
                                    case CHANGE_PROT_HTM_READ: \
                                         /*change_protection_HTM(PROT_READ,pageID,HTM_Heap,metadata_array); */\
                                        CHANGE_PROTECTION_HTM_READ((pageID)); \
                                        return_value=0;  \
                                         break; \
                                    case CHANGE_PROT_HTM_READ_WRITE: \
                                        /*change_protection_HTM(PROT_READ | PROT_WRITE,pageID,HTM_Heap,metadata_array);  */\
                                        CHANGE_PROTECTION_HTM_READ_WRITE((pageID)); \
                                        return_value=0;  \
                                         break; \
                                    case PAGE_IS_FOR_STM: \
                                         return_value=1;  \
                                         break; \
                                    } \
                                return_value; \
                        })

//Handles an access violation and returns proper handling
int AV_handler (long* address, long* HTM_Heap, metadata* metadata_array,unsigned long offset_heaps,int pageID,int update_transaction);

void handler (int sig, siginfo_t *si, void *unused);
#ifdef __cplusplus
}
#endif

#endif


#endif /* THREAD_H */
