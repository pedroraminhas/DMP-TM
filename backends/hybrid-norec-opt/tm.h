#ifndef TM_H
#define TM_H 1

#  include <stdio.h>

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

# define AL_LOCK(b)
# define PRINT_STATS()
# define SETUP_NUMBER_TASKS(b)
# define SETUP_NUMBER_THREADS(b)

#  include <assert.h>
#ifndef REDUCED_TM_API
#  include "memory.h"
#  include "thread.h"
#  include "types.h"
#endif

//#include <immintrin.h>
//#include <rtmintrin.h>
#include <htmxlintrin.h>

extern __inline long
__attribute__ ((__gnu_inline__, __always_inline__, __artificial__))
__TM_is_self_conflict(void* const TM_buff)
{
  texasr_t texasr = __builtin_get_texasr ();
  return _TEXASR_SELF_INDUCED_CONFLICT (texasr);
}

extern __inline long
__attribute__ ((__gnu_inline__, __always_inline__, __artificial__))
__TM_is_trans_conflict(void* const TM_buff)
{
  texasr_t texasr = __builtin_get_texasr ();
  return _TEXASR_TRANSACTION_CONFLICT (texasr);
}

extern __inline long
__attribute__ ((__gnu_inline__, __always_inline__, __artificial__))
__TM_is_nontrans_conflict(void* const TM_buff)
{
  texasr_t texasr = __builtin_get_texasr ();
  return _TEXASR_NON_TRANSACTIONAL_CONFLICT (texasr);
}

#include "stm.h"
#include "norec.h"

# define INIT_FUN_PTRS                  /*abortFunPtr = &abortHTM; \
                                        sharedReadFunPtr = &sharedReadHTM;  \
                                        sharedWriteFunPtr = &sharedWriteHTM; \
                                        freeFunPtr = &freeHTM;*/

#ifdef REDUCED_TM_API
#    define Self                        TM_ARG_ALONE
#    define TM_ARG_ALONE                get_thread()
#    define SPECIAL_THREAD_ID()         get_tid()
#    define SPECIAL_INIT_THREAD(id)     thread_desc[id] = (void*)TM_ARG_ALONE;
#    define TM_THREAD_ENTER()         Thread* inited_thread = STM_NEW_THREAD(); \
                                      STM_INIT_THREAD(inited_thread, SPECIAL_THREAD_ID()); \
                                      threadID = SPECIAL_THREAD_ID(); \
                                      statistics_array[SPECIAL_THREAD_ID()].htm_commits=0; \
                                      thread_desc[SPECIAL_THREAD_ID()] = (void*)inited_thread; \
                                      INIT_FUN_PTRS
#else
#    define TM_ARG_ALONE                  STM_SELF
#    define SPECIAL_THREAD_ID()         thread_getId()
#    define TM_ARGDECL                    STM_THREAD_T* TM_ARG
#    define TM_ARGDECL_ALONE              STM_THREAD_T* TM_ARG_ALONE
#    define TM_THREAD_ENTER()         TM_ARGDECL_ALONE = STM_NEW_THREAD(); \
                                      threadID = SPECIAL_THREAD_ID(); \
                                        statistics_array[SPECIAL_THREAD_ID()].htm_commits=0; \
                                      STM_INIT_THREAD(TM_ARG_ALONE, SPECIAL_THREAD_ID()); \
                                      INIT_FUN_PTRS
#endif





#    define TM_CALLABLE                   /* nothing */
#    define TM_ARG                        TM_ARG_ALONE,
#    define TM_THREAD_EXIT()          STM_FREE_THREAD(TM_ARG_ALONE)

#      define TM_STARTUP(numThread,u)({ \
																	     STM_STARTUP(); \
                        number_threads=numThread; \
                        int loop_counter_startup=0; \
                        /*Calculate the offset between the two heaps*/ \
                            offset_heaps= HTM_Heap - STM_Heap; \
                        /*Calculate the first address of the page that contains the begginning of HTM Heap*/ \
                        page_first_address= ((long int) HTM_Heap) & ~(PAGE_SIZE-1); \
                        STM_page_first_address = ((long int) STM_Heap) & ~(PAGE_SIZE-1); \
                        memory_per_thread=(NUM_ELEMENTS_ARRAY/numThread); \
                    for(loop_counter_startup=0;loop_counter_startup<numThread;loop_counter_startup++) {\
                        STM_pointer[loop_counter_startup].pointer=STM_Heap+(memory_per_thread*loop_counter_startup); \
                        /*printf("HTM_Heap is %ld and pointer is %ld\n",HTM_Heap,STM_pointer[loop_counter_startup].pointer); */\
										}						 \
})

#      define TM_SHUTDOWN()             \
    STM_SHUTDOWN(); \
    long htm_commits = 0; \
    unsigned long stm_commits = 0; \
    unsigned long stm_aborts = 0; \
    unsigned long conflicts = 0; \
    unsigned long capacity_aborts = 0; \
    unsigned long user_aborts = 0; \
    unsigned long nontrans = 0; \
    unsigned long other_aborts = 0; \
    unsigned long system_calls=0; \
    unsigned long handler_system_calls=0; \
    unsigned long fallback_to_STM=0; \
    unsigned long lookup_HTM = 0; \
    unsigned long lookup_STM = 0; \
    unsigned long lookup_STM_small = 0; \
    unsigned long insert_HTM = 0; \
    unsigned long insert_STM = 0; \
    unsigned long insert_STM_small = 0; \
    unsigned long sgl_commits = 0; \
    unsigned long total_commits=0; \
    int ik = 0; \
    for (ik=0; ik < 128; ik++) { \
       /*if (!statistics_array[ik].htm_commits && !statistics_array[ik].stm_commits ) { break; } */\
            htm_commits += statistics_array[ik].htm_commits; \
            printf("htm_commits: %ld",statistics_array[ik].htm_commits);\
            stm_commits += statistics_array[ik].stm_commits; \
            total_commits += statistics_array[ik].stm_commits+statistics_array[ik].htm_commits+statistics_array[ik].sgl_commits; \
            stm_aborts += statistics_array[ik].stm_aborts; \
            conflicts += statistics_array[ik].conflicts; \
            capacity_aborts += statistics_array[ik].capacity_aborts; \
            user_aborts += statistics_array[ik].user_aborts; \
            nontrans += statistics_array[ik].nontrans; \
            other_aborts += statistics_array[ik].other_aborts; \
            system_calls += statistics_array[ik].system_calls; \
            fallback_to_STM += statistics_array[ik].fallback_to_STM; \
            lookup_HTM += statistics_array[ik].lookup_HTM; \
            lookup_STM += statistics_array[ik].lookup_STM; \
            lookup_STM_small += statistics_array[ik].lookup_STM_small; \
            insert_STM += statistics_array[ik].insert_STM; \
            insert_HTM += statistics_array[ik].insert_HTM; \
            insert_STM_small += statistics_array[ik].insert_STM_small; \
            sgl_commits += statistics_array[ik].sgl_commits; \
            handler_system_calls += statistics_array[ik].handler_system_calls; \
    } \
    printf("Total lookup: %lu\n\tHTM lookup: %lu\n\tSTM lookup: %lu\n\tSTM small lookup %lu\nTotal inserts: %lu\n\tHTM insert: %lu\n\tSTM inser: %lu\n\tSTM small insert: %lu\nTotal commits: %ld \n\tHTM commits: %ld \n\tSGL Commits: %lu\n\tSTM commits: %ld \nTotal aborts: %ld\n\tHTM conficts: %lu\n\tCapacity aborts: %lu\n\tUser aborts: %lu\n\tNon-transactional aborts: %lu\n\tOther aborts: %lu\n\tSTM aborts: %lu\nSystem Calls: %ld\nSTM System Calls: %ld\n", lookup_HTM+lookup_STM+lookup_STM_small,lookup_HTM,lookup_STM,lookup_STM_small,insert_HTM+insert_STM+insert_STM_small,insert_HTM,insert_STM,insert_STM_small,(stm_commits+sgl_commits),0,sgl_commits,stm_commits, stm_aborts+conflicts+capacity_aborts+user_aborts+other_aborts,conflicts,capacity_aborts,user_aborts,nontrans,other_aborts,stm_aborts,system_calls,handler_system_calls); \

#  define TM_BEGIN_WAIVER()
#  define TM_END_WAIVER()

# define TM_BEGIN(ro)     TM_BEGIN_EXT(0, ro)
# define SPEND_BUDGET(b)	if(RETRY_POLICY == 0) (*b)=0; else if (RETRY_POLICY == 2) (*b)=(*b)/2; else (*b)=--(*b);

# define RETRY_POLICY 1

# define FAST_PATH_BEGIN(b,ro) TM_BEGIN_EXT(b,ro)
# define SLOW_PATH_BEGIN(b,ro) TM_BEGIN_EXT(b,ro)
#    define TM_BEGIN_EXT(b,ro)    \
    { \
        int tle_budget = HTM_RETRIES; \
	local_exec_mode = 0; \
	local_thread_id = SPECIAL_THREAD_ID(); \ 
        while (1) { \
            if (tle_budget > 0) { \
                while (fallback_in_use != 0) { __asm volatile ("nop;"); } \
		TM_buff_type TM_buff; \
                unsigned char status = __TM_begin(&TM_buff);    \
                if (status == _HTM_TBEGIN_STARTED) { \
                    if (fallback_in_use != 0) { __TM_abort(); } \
                    break;  \
                } \
                else {\
                if(__TM_is_failure_persistent(&TM_buff)){ \
                         SPEND_BUDGET(&tle_budget); \
                } \
                if(__TM_is_conflict(&TM_buff)){ \
                        statistics_array[local_thread_id].conflicts++; \
                        if(__TM_is_trans_conflict(&TM_buff)) statistics_array[local_thread_id].conflicts++; \
                        if(__TM_is_nontrans_conflict(&TM_buff)) statistics_array[local_thread_id].nontrans++; \
                        tle_budget--; \
                } \
                else if (__TM_is_user_abort(&TM_buff)) { \
                        statistics_array[local_thread_id].user_aborts++; \
                        tle_budget--; \
                } \
                else if(__TM_is_footprint_exceeded(&TM_buff)){ \
                        statistics_array[local_thread_id].capacity_aborts++; \
                        tle_budget--; \
                } \
                else{ \
                        statistics_array[local_thread_id].other_aborts++; \
                        tle_budget--; \
                } \
                } \
            } else {  \
		local_exec_mode = 1; \
                __sync_add_and_fetch(&exists_sw.counter,1); \
                STM_BEGIN(ro);   \
                statistics_array[local_thread_id].stm_aborts++; \
                break;  \
            } \
        } \
}

# define FAST_PATH_END()	TM_END()
# define SLOW_PATH_END()	TM_END()
#    define TM_END(){  \
        if (local_exec_mode == 0) {    \
            if (!ro && exists_sw.counter) { \
                HTM_INC_CLOCK(); \
            } \
            __TM_end();    \
            statistics_array[SPECIAL_THREAD_ID()].htm_commits++; \
        } else {    \
            __sync_add_and_fetch(&fallback_in_use,1);   \
            int ret = HYBRID_STM_END();  \
            __sync_sub_and_fetch(&fallback_in_use,1);    \
            if (ret == 0) { \
                STM_RESTART(); \
            } \
            __sync_sub_and_fetch(&exists_sw.counter,1); \
            statistics_array[local_thread_id].stm_aborts--; \
	    statistics_array[local_thread_id].stm_commits++; \
        } \
    };



#    define TM_EARLY_RELEASE(var)         


#      define P_MALLOC(size)            TM_MALLOC(size)
#      define P_FREE(ptr)               //free(ptr)
#      define SEQ_MALLOC(size)          TM_MALLOC(size)
#      define SEQ_FREE(ptr)             //free(ptr)

#  define TM_MALLOC(size)       ({ \
                                                    memory_requested=STM_pointer[local_thread_id].pointer; \
                                                    /*printf("memory requested is %ld\n",memory_requested); */\
                                                    STM_pointer[local_thread_id].pointer+=size; \
                                                    memory_requested; \
                                })
#      define FAST_PATH_FREE(ptr)       //free(ptr) //(*freeFunPtr)((void*)ptr)
#      define SLOW_PATH_FREE(ptr)       //(*freeFunPtr)((void*)ptr)
#				define TM_FREE(ptr)		FAST_PATH_FREE(ptr)

# define FAST_PATH_RESTART() __TM_abort()
# define FAST_PATH_SHARED_READ(var) var
# define FAST_PATH_SHARED_READ_P(var) var
# define FAST_PATH_SHARED_READ_D(var) var
# define FAST_PATH_SHARED_READ_F(var) var
# define FAST_PATH_SHARED_WRITE(var, val) ({var = val; var;})
# define FAST_PATH_SHARED_WRITE_P(var, val) ({var = val; var;})
# define FAST_PATH_SHARED_WRITE_D(var, val) ({var = val; var;})
# define FAST_PATH_SHARED_WRITE_F(var, val) ({var = val; var;})

#  define SLOW_PATH_RESTART()                  STM_RESTART()
#  define SLOW_PATH_SHARED_READ(var)           STM_READ(var)
#  define SLOW_PATH_SHARED_READ_P(var)         STM_READ_P(var)
#  define SLOW_PATH_SHARED_READ_D(var)         STM_READ_F(var)
#  define SLOW_PATH_SHARED_READ_F(var)         STM_READ_F(var)
#  define SLOW_PATH_SHARED_WRITE(var, val)     STM_WRITE((var), val)
#  define SLOW_PATH_SHARED_WRITE_P(var, val)   STM_WRITE_P((var), val)
#  define SLOW_PATH_SHARED_WRITE_D(var, val)   STM_WRITE_F((var), val)
# define SLOW_PATH_SHARED_WRITE_F(var,val)     STM_WRITE_F((var), val)

# define STM_SHARED_READ(var)    STM_READ(var)
# define STM_SHARED_READ_P(var)  STM_READ_P(var)
# define STM_SHARED_WRITE(var,val) STM_WRITE((var), val)
 # define STM_SHARED_WRITE_P(var,val)   STM_WRITE_P((var), val)

#  define TM_LOCAL_WRITE(var, val)      ({var = val; var;})
#  define TM_LOCAL_WRITE_P(var, val)    ({var = val; var;})
#  define TM_LOCAL_WRITE_D(var, val)    ({var = val; var;})

#endif /* TM_H */
