#ifndef TM_H
#define TM_H 1

#  include <stdio.h>
//#  include "stm_src.h"
#include <unistd.h>


#ifndef REDUCED_TM_API

#  define MAIN(argc, argv)              int main (int argc, char** argv)
#  define MAIN_RETURN(val)              return val

#  define GOTO_SIM()                    /* nothing */
#  define GOTO_REAL()                   /* nothing */
#  define IS_IN_SIM()                   (0)

#  define SIM_GET_NUM_CPU(var)          /* nothing */

#  define TM_PRINTF                     printf
#  define TM_PRINT0                     printf
#  define TM_PRINT1                     printf
#  define TM_PRINT2                     printf
#  define TM_PRINT3                     printf

#  define P_MEMORY_STARTUP(numThread)   /* nothing */
#  define P_MEMORY_SHUTDOWN()           /* nothing */

#  include <assert.h>
#  include "memory.h"
#  include "thread.h"
#  include "types.h"
#  include "thread.h"
#  include <math.h>


#  define TM_ARG                        /* nothing */
#  define TM_ARG_ALONE                  /* nothing */
#  define TM_ARGDECL                    /* nothing */
#  define TM_ARGDECL_ALONE              /* nothing */
#  define TM_CALLABLE                   /* nothing */

#  define TM_BEGIN_WAIVER()
#  define TM_END_WAIVER()

#  define TM_MALLOC(size)       ({ \
                                    /*printf("threadID is %ld",threadID);*/ \
                                    /*printf("in P_MALLOC with size %ld\n",size);*/ \
                                    if(threadID>80){ \
                                       round_robin_counter++; \
                                        threadID=round_robin_counter%number_threads; \
                                    } \
                                    /*printf("passed here\n");*/ \
                                    memory_requested=STM_pointer[threadID].pointer; \
                                    STM_pointer[threadID].pointer+=((size)/sizeof(long)); \
                                    /*printf("memory previously requested is %ld\n",memory_requested); */\
                                    memory_requested; \
                })

#      define P_FREE(ptr)               /*free(ptr)*/

#  define P_MALLOC(size)                 TM_MALLOC(size)
#  define FAST_PATH_FREE(ptr)            P_FREE(ptr)
#  define SLOW_PATH_FREE(ptr)           P_FREE(ptr)
#  define TM_FREE(ptr)                  P_FREE(ptr)



# define SETUP_NUMBER_TASKS(n)
# define SETUP_NUMBER_THREADS(n)
# define PRINT_STATS()
# define AL_LOCK(idx)

#endif

#ifdef REDUCED_TM_API
#    define SPECIAL_THREAD_ID()         get_tid()
#else
#    define SPECIAL_THREAD_ID()         thread_getId()
#endif

//#  include <immintrin.h>
//#  include <rtmintrin.h>
#include <htmxlintrin.h>

extern __inline long
__attribute__ ((__gnu_inline__, __always_inline__, __artificial__))
__TM_is_nontrans_conflict(void* const TM_buff)
{
texasr_t texasr = __builtin_get_texasr (); 
return _TEXASR_NON_TRANSACTIONAL_CONFLICT (texasr);
}

#  define TM_STARTUP(numThread, bId) \
				                        number_threads=numThread; \
                        threadID = SPECIAL_THREAD_ID(); \ 
                        int loop_counter_startup=0; \
                        metadata_array = (metadata*) malloc(sizeof(metadata) * NUM_PAGES); \
                        /*Calculate the offset between the two heaps*/ \
                            offset_heaps= HTM_Heap - STM_Heap; \
                        /*Calculate the first address of the page that contains the begginning of HTM Heap*/ \
                        page_first_address= ((long int) HTM_Heap) & ~(PAGE_SIZE-1); \
                        STM_page_first_address = ((long int) STM_Heap) & ~(PAGE_SIZE-1); \
                        memory_per_thread=(NUM_ELEMENTS_ARRAY/numThread); \  
                    for(loop_counter_startup=0;loop_counter_startup<numThread;loop_counter_startup++) {\
                        STM_pointer[loop_counter_startup].pointer=HTM_Heap+(memory_per_thread*loop_counter_startup); \
                        /*printf("HTM_Heap is %ld and pointer is %ld\n",HTM_Heap,STM_pointer[loop_counter_startup].pointer);*/ \
                    }      

#  define TM_SHUTDOWN() { \
    unsigned long htm_commits = 0; \
    unsigned long sgl_commits = 0; \
    unsigned long footprint = 0; \
    unsigned long other_abort = 0; \
    unsigned long conflict = 0; \
    unsigned long user_abort = 0; \
    unsigned long lookup_HTM = 0; \
    unsigned long lookup_STM = 0; \
    unsigned long lookup_STM_small = 0; \
    unsigned long insert_HTM = 0; \ 
    unsigned long insert_STM = 0; \
    unsigned long insert_STM_small = 0; \
    int i = 0; \
    unsigned long nontrans = 0; \ 
    for (; i < 80; i++) { \
       if (!statistics_array[i].htm_commits && !statistics_array[i].sgl_commits) { break; } \
       htm_commits += statistics_array[i].htm_commits; \
       sgl_commits += statistics_array[i].sgl_commits;  \
       total_commits += statistics_array[i].htm_commits +statistics_array[i].sgl_commits; \
       lookup_HTM += statistics_array[i].lookup_HTM; \
       /*printf("----lookupSTM -----= %d", statistics_array[i].lookup_STM);*/ \
       lookup_STM += statistics_array[i].lookup_STM; \
       lookup_STM_small += statistics_array[i].lookup_STM_small; \
       insert_STM += statistics_array[i].insert_STM; \
       insert_HTM += statistics_array[i].insert_HTM; \
       insert_STM_small += statistics_array[i].insert_STM_small; \
        nontrans += statistics_array[i].nontrans; \
        footprint += statistics_array[i].footprint; \
	conflict += statistics_array[i].conflict; \
	user_abort += statistics_array[i].user_abort; \
	other_abort += statistics_array[i].other_abort; \
    } \
    printf("Total lookup: %lu\n\tHTM lookup: %lu\n\tSTM lookup: %lu\n\tSTM small lookup %lu\nTotal inserts: %lu\n\tHTM insert: %lu\n\tSTM inser: %lu\n\tSTM small insert: %lu\nTotal commits: %lu\n\tHTM commits: %lu\n\tSGL commits: %lu\nTotal aborts: %lu\n\tCapacity aborts: %d\n\tConflicts: %d\n\tUser aborts: %d\n\tNon-transactional aborts: %d\n\tOther aborts: %d\n", lookup_HTM+lookup_STM+lookup_STM_small,lookup_HTM,lookup_STM,lookup_STM_small,insert_HTM+insert_STM+insert_STM_small,insert_HTM,insert_STM,insert_STM_small,htm_commits+sgl_commits, htm_commits, sgl_commits, footprint+conflict+user_abort+other_abort,footprint,conflict,user_abort,nontrans,other_abort); \
}

#  define TM_THREAD_ENTER() ({threadID =  SPECIAL_THREAD_ID();})
#  define TM_THREAD_EXIT()

# define IS_LOCKED(lock)        *((volatile long*)(&lock)) != 0

//# define TM_BEGIN(mode,ro) TM_BEGIN_EXT(b,mode,ro)
//# define TM_BEGIN(b,mode) TM_BEGIN_EXT(b,mode,0)
# define SPEND_BUDGET(b)	if(RETRY_POLICY == 0) (*b)=0; else if (RETRY_POLICY == 2) (*b)=(*b)/2; else (*b)=--(*b);
#define cpu_relax()     asm volatile ("" ::: "memory");

# define FAST_PATH_BEGIN(b,ro) { \
        int tries = HTM_RETRIES; \
        local_exec_mode = 0; \
        while (1) { \
            while(IS_LOCKED(single_global_lock.counter)) { \
                 __asm__ ("nop;"); \
            } \
	        TM_buff_type TM_buff; \
            unsigned char status = __TM_begin(&TM_buff); \
            if (status == _HTM_TBEGIN_STARTED) { \
            	if (IS_LOCKED(single_global_lock.counter)) { \
            		__TM_abort(); \
                } \
            	break; \
            } \
            else if (__TM_is_footprint_exceeded(&TM_buff)) { \
               SPEND_BUDGET(&tries); \
                statistics_array[SPECIAL_THREAD_ID()].footprint++; \
            } \
            else if(__TM_is_conflict(&TM_buff)){ \
		        tries--; \
                statistics_array[SPECIAL_THREAD_ID()].conflict++; \    
                if(__TM_is_nontrans_conflict(&TM_buff)) statistics_array[SPECIAL_THREAD_ID()].nontrans++; \ 
            } \
            else if (__TM_is_user_abort(&TM_buff)) { \
		        tries--; \
                statistics_array[SPECIAL_THREAD_ID()].user_abort++; \
            } \
            else { \
		        statistics_array[SPECIAL_THREAD_ID()].other_abort++; \
                tries--; \
            } \
            if(tries <= 0){ \
                local_exec_mode = 2; \
				while(1){ \
                	while(IS_LOCKED(single_global_lock.counter)){ \
                    	__asm__ ("nop;"); \
                    } \
                    if( __sync_val_compare_and_swap(&single_global_lock.counter, 0, 1) == 0) { \
                    	break; \
                    } \
                } \
                break; \
            } \
            } \
   }; 
  
#define SLOW_PATH_BEGIN(b,ro) FAST_PATH_BEGIN(b,ro) 

#define FAST_PATH_END(){ \
    if(local_exec_mode == 0){ \
        __TM_end(); \
        statistics_array[SPECIAL_THREAD_ID()].htm_commits++; \
    }else{ \
        statistics_array[SPECIAL_THREAD_ID()].sgl_commits++; \
   		single_global_lock.counter = 0; \ 
	} \
};


#define SLOW_PATH_END() 	FAST_PATH_END() 

#    define TM_BEGIN_RO()                 TM_BEGIN(0)
#    define TM_RESTART()                  __TM_abort();
#    define TM_EARLY_RELEASE(var)

# define FAST_PATH_RESTART() __TM_abort();
# define FAST_PATH_SHARED_READ(var) (var)
# define FAST_PATH_SHARED_READ_P(var) (var)
# define FAST_PATH_SHARED_READ_D(var) (var)
# define FAST_PATH_SHARED_READ_F(var) (var)
# define FAST_PATH_SHARED_WRITE(var, val) ({var = val; var;})
# define FAST_PATH_SHARED_WRITE_P(var, val) ({var = val; var;})
# define FAST_PATH_SHARED_WRITE_D(var, val) ({var = val; var;})
# define FAST_PATH_SHARED_WRITE_F(var, val) ({var = val; var;})

# define BAILOUT_RESTART() FAST_PATH_RESTART()
# define BAILOUT_SHARED_READ(var)           FAST_PATH_SHARED_READ(var)
# define BAILOUT_SHARED_READ_P(var)         FAST_PATH_SHARED_READ_P(var)
# define BAILOUT_SHARED_READ_F(var)         FAST_PATH_SHARED_READ_D(var)
# define BAILOUT_SHARED_READ_D(var)         FAST_PATH_SHARED_READ_D(var)
# define BAILOUT_SHARED_WRITE(var, val)     FAST_PATH_SHARED_WRITE(var, val)
# define BAILOUT_SHARED_WRITE_P(var, val)   FAST_PATH_SHARED_WRITE_P(var, val)
# define BAILOUT_SHARED_WRITE_D(var, val)   FAST_PATH_SHARED_WRITE_D(var, val)

# define SLOW_PATH_RESTART() FAST_PATH_RESTART()
# define SLOW_PATH_SHARED_READ(var)           FAST_PATH_SHARED_READ(var)
# define SLOW_PATH_SHARED_READ_P(var)         FAST_PATH_SHARED_READ_P(var)
# define SLOW_PATH_SHARED_READ_F(var)         FAST_PATH_SHARED_READ_F(var)
# define SLOW_PATH_SHARED_READ_D(var)         FAST_PATH_SHARED_READ_D(var)
# define SLOW_PATH_SHARED_WRITE(var, val)     FAST_PATH_SHARED_WRITE(var, val)
# define SLOW_PATH_SHARED_WRITE_P(var, val)   FAST_PATH_SHARED_WRITE_P(var, val)
# define SLOW_PATH_SHARED_WRITE_D(var, val)   FAST_PATH_SHARED_WRITE_D(var, val)
# define SLOW_PATH_SHARED_WRITE_F(var, val)   FAST_PATH_SHARED_WRITE_F(var, val)

# define STM_SHARED_READ(var)           FAST_PATH_SHARED_READ(var)
# define STM_SHARED_READ_P(var)           FAST_PATH_SHARED_READ(var)
# define STM_SHARED_WRITE(var, val)     FAST_PATH_SHARED_WRITE(var, val)
# define STM_SHARED_WRITE_P(var, val)     FAST_PATH_SHARED_WRITE(var, val)
# define HTM_SHARED_READ(var)           FAST_PATH_SHARED_READ(var)
# define HTM_SHARED_WRITE(var, val)     FAST_PATH_SHARED_WRITE(var, val)

# define MIXED_PATH_RESTART() FAST_PATH_RESTART()
# define MIXED_PATH_SHARED_READ(var)           FAST_PATH_SHARED_READ(var)
# define MIXED_PATH_SHARED_READ_P(var)         FAST_PATH_SHARED_READ_P(var)
# define MIXED_PATH_SHARED_READ_F(var)         FAST_PATH_SHARED_READ_D(var)
# define MIXED_PATH_SHARED_READ_D(var)         FAST_PATH_SHARED_READ_D(var)
# define MIXED_PATH_SHARED_WRITE(var, val)     FAST_PATH_SHARED_WRITE(var, val)
# define MIXED_PATH_SHARED_WRITE_P(var, val)   FAST_PATH_SHARED_WRITE_P(var, val)
# define MIXED_PATH_SHARED_WRITE_D(var, val)   FAST_PATH_SHARED_WRITE_D(var, val)


#  define TM_LOCAL_WRITE(var, val)      ({var = val; var;})
#  define TM_LOCAL_WRITE_P(var, val)    ({var = val; var;})
#  define TM_LOCAL_WRITE_D(var, val)    ({var = val; var;})


#endif
