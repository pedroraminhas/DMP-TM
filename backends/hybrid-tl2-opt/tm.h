#ifndef TM_H
#define TM_H 1

#  include <stdio.h>
#  include <unistd.h>
#include <signal.h>
#include <sys/mman.h>
#include <setjmp.h>

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
#ifndef REDUCED_TM_API
#  include "memory.h"
#  include "thread.h"
#  include "types.h"
#endif

//#include <immintrin.h>
//#include <rtmintrin.h>
#include "stm_tl2.h"
//#include "tl2.h"
#include <htmxlintrin.h>

# define AL_LOCK(b)
# define PRINT_STATS()
# define SETUP_NUMBER_TASKS(b)
# define SETUP_NUMBER_THREADS(b)

#ifdef REDUCED_TM_API
#    define Self                        TM_ARG_ALONE
#    define TM_ARG_ALONE                get_thread()
#    define SPECIAL_THREAD_ID()         get_tid()
#    define SPECIAL_INIT_THREAD(id)     thread_desc[id] = (void*)TM_ARG_ALONE;
#    define TM_THREAD_ENTER()         Thread* inited_thread = STM_NEW_THREAD(); \
                                      STM_INIT_THREAD(inited_thread, SPECIAL_THREAD_ID()); \
                                      thread_desc[SPECIAL_THREAD_ID()] = (void*)inited_thread; \
                                      threadID = SPECIAL_THREAD_ID(); 
#else
#    define TM_ARG_ALONE                  STM_SELF
#    define SPECIAL_THREAD_ID()         thread_getId()
#    define TM_ARGDECL                    STM_THREAD_T* TM_ARG
#    define TM_ARGDECL_ALONE              STM_THREAD_T* TM_ARG_ALONE
#    define TM_THREAD_ENTER()         TM_ARGDECL_ALONE = STM_NEW_THREAD(); \
                                      STM_INIT_THREAD(TM_ARG_ALONE, SPECIAL_THREAD_ID()); \
                                    threadID = SPECIAL_THREAD_ID(); 
#endif
#    define TM_CALLABLE                   /* nothing */
#    define TM_ARG                        TM_ARG_ALONE,
#    define TM_THREAD_EXIT()          STM_FREE_THREAD(TM_ARG_ALONE)

#      define TM_STARTUP(numThread, useless)     \
                                    STM_STARTUP(); \
                                   threadID = 0; \
                        int loop_counter_startup=0; \
                        /*number_threads = numThread; */\
                        /*Initialization of metadata */ \
                        /*Calculate the first address of the page that contains the begginning of HTM Heap*/ \
                        memory_per_thread=( NUM_ELEMENTS_ARRAY/numThread); \
                        printf("memory per thread is %ld out of %ld for %ld threads\n",memory_per_thread,NUM_ELEMENTS_ARRAY,numThread); \
                        for(loop_counter_startup=0;loop_counter_startup<numThread;loop_counter_startup++) {\
                                STM_pointer[loop_counter_startup].pointer=STM_Heap+(memory_per_thread*loop_counter_startup); \
                        } \ 

#      define TM_SHUTDOWN() { \
    STM_SHUTDOWN(); \
	unsigned long htm_commits = 0; \
    unsigned long stm_commits = 0; \
    unsigned long stm_aborts = 0; \
    unsigned long conflicts = 0; \
    unsigned long capacity_aborts = 0; \
    unsigned long user_aborts = 0; \
    unsigned long nontrans = 0; \
    unsigned long other_aborts = 0; \
    unsigned long system_calls=0; \
    unsigned long htm_system_calls=0; \
    unsigned long fallback_to_STM=0; \
    unsigned long lookup_HTM = 0; \
    unsigned long lookup_STM = 0; \
    unsigned long lookup_STM_small = 0; \
    unsigned long insert_HTM = 0; \
    unsigned long insert_STM = 0; \
    unsigned long insert_STM_small = 0; \
    unsigned long sgl_commits = 0; \
    int ik = 0; \
    for (; ik < 80; ik++) { \
       /*if (!statistics_array[ik].htm_commits && !statistics_array[ik].stm_commits ) { break; } */\
            htm_commits += statistics_array[ik].htm_commits; \
            stm_commits += statistics_array[ik].stm_commits; \
            total_commits += statistics_array[ik].stm_commits+statistics_array[ik].htm_commits+statistics_array[ik].sgl_commits; \
            stm_aborts += statistics_array[ik].stm_aborts; \
            conflicts += statistics_array[ik].conflicts; \
            capacity_aborts += statistics_array[ik].capacity_aborts; \
            user_aborts += statistics_array[ik].user_aborts; \
            nontrans += statistics_array[ik].nontrans; \
            other_aborts += statistics_array[ik].other_aborts; \
            system_calls += statistics_array[ik].system_calls; \
            htm_system_calls += statistics_array[ik].htm_system_calls; \
            fallback_to_STM += statistics_array[ik].fallback_to_STM; \
            lookup_HTM += statistics_array[ik].lookup_HTM; \
            lookup_STM += statistics_array[ik].lookup_STM; \
            lookup_STM_small += statistics_array[ik].lookup_STM_small; \
            insert_STM += statistics_array[ik].insert_STM; \
            insert_HTM += statistics_array[ik].insert_HTM; \
            insert_STM_small += statistics_array[ik].insert_STM_small; \
            sgl_commits += statistics_array[ik].sgl_commits; \
    } \
 printf("Total lookup: %lu\n\tHTM lookup: %lu\n\tSTM lookup: %lu\n\tSTM small lookup %lu\nTotal inserts: %lu\n\tHTM insert: %lu\n\tSTM inser: %lu\n\tSTM small insert: %lu\nTotal commits: %ld \n\tHTM commits: %ld \n\tSGL Commits: %lu\n\tSTM commits: %ld \nTotal aborts: %ld\n\tHTM conficts: %lu\n\tCapacity aborts: %lu\n\tUser aborts: %lu\n\tNon-transactional aborts: %lu\n\tOther aborts: %lu\n\tSTM aborts: %lu\nSTM System Calls: %ld\nHTM System Calls: %ld\n", lookup_HTM+lookup_STM+lookup_STM_small,lookup_HTM,lookup_STM,lookup_STM_small,insert_HTM+insert_STM+insert_STM_small,insert_HTM,insert_STM,insert_STM_small,(htm_commits+sgl_commits+stm_commits),htm_commits,sgl_commits,stm_commits, stm_aborts+conflicts+capacity_aborts+user_aborts+other_aborts,conflicts,capacity_aborts,user_aborts,nontrans,other_aborts,stm_aborts,system_calls,htm_system_calls); \
}

extern __inline long
__attribute__ ((__gnu_inline__, __always_inline__, __artificial__))
        __TM_is_nontrans_conflict(void* const TM_buff)
{
        texasr_t texasr = __builtin_get_texasr (); 
        return _TEXASR_NON_TRANSACTIONAL_CONFLICT (texasr);
}

#  define TM_BEGIN_WAIVER()
#  define TM_END_WAIVER()
#define RETRY_POLICY 1

# define TM_BEGIN(b,mode)     TM_BEGIN_EXT(b,mode,0)
# define SPEND_BUDGET(b)	if(RETRY_POLICY == 0) (*b)=0; else if (RETRY_POLICY == 2) (*b)=(*b)/2; else (*b)=--(*b);

# define FAST_PATH_BEGIN(b,ro)  TM_BEGIN_EXT(b,ro)
# define SLOW_PATH_BEGIN(v,ro)  TM_BEGIN_EXT(b,ro)

#    define TM_BEGIN_EXT(b,ro) { \
        unsigned int counter_stm_executions = 0; \
        threadID = SPECIAL_THREAD_ID(); \
		int tries = HTM_RETRIES;	\
        local_exec_mode=0; \
		while (1) {	\
			if (tries > 0) { \
				while (fallback_in_use.counter != 0) { asm volatile ("nop"); } \
                   TM_buff_type TM_buff; \
                        unsigned char status = __TM_begin(&TM_buff); \
                        if (status == _HTM_TBEGIN_STARTED) { \ 
							if (fallback_in_use.counter != 0) { __TM_abort();  } \
                    		if (ro == 0) { next_commit = NEXT_CLOCK(); } \
								break;	\
						}else{  \
                                    if (__TM_is_footprint_exceeded(&TM_buff)) { \
                                        SPEND_BUDGET(&tries); \
                                    statistics_array[SPECIAL_THREAD_ID()].capacity_aborts++; \
                                } \
                                else if(__TM_is_conflict(&TM_buff)){ \
                                    tries--; \
                                    statistics_array[SPECIAL_THREAD_ID()].conflicts++; \
                                    if(__TM_is_nontrans_conflict(&TM_buff)) statistics_array[SPECIAL_THREAD_ID()].nontrans++; \
                                } \
                                    else if (__TM_is_user_abort(&TM_buff)) { \
                                        tries--; \
                                        statistics_array[SPECIAL_THREAD_ID()].user_aborts++; \
                                } \
                                else { \
                                    statistics_array[SPECIAL_THREAD_ID()].other_aborts++; \
                                    tries--; \
                                } \
                                } \ 
			} else {  \
				local_exec_mode = 1; \
				STM_BEGIN(ro); \
				statistics_array[SPECIAL_THREAD_ID()].stm_aborts++; \
				break;  \
			} \
	} \
};


#    define TM_END()	{	\
		if (local_exec_mode==0) {	\
			__TM_end(); \
            statistics_array[SPECIAL_THREAD_ID()].htm_commits++; \
		} else { \
            /*int result = 1; \
            do{ \
                long val = fallback_in_use.counter; \
                result = __sync_bool_compare_and_swap(&fallback_in_use.counter,val,val+1); \
            } \
            while(!result); */\
            __sync_fetch_and_add (&fallback_in_use.counter, 1); \
			int ret = HYBRID_STM_END();  \
            /*int result2 = 1; \
			do{ \
                long val = fallback_in_use.counter; \
                result2 = __sync_bool_compare_and_swap(&fallback_in_use.counter,val,val-1); \
            } \
            while(!result2); */\
            __sync_fetch_and_sub (&fallback_in_use.counter, 1); \
			if (ret == 0) { \
				STM_RESTART(); \
			} \
			AFTER_COMMIT(); \
			statistics_array[SPECIAL_THREAD_ID()].stm_aborts--; \
		    statistics_array[SPECIAL_THREAD_ID()].stm_commits++; \
		} \
};

#define SLOW_PATH_END() TM_END();
#define FAST_PATH_END() TM_END();

# define MIXED_PATH_SHARED_READ(var)           FAST_PATH_SHARED_READ(var)
# define MIXED_PATH_SHARED_READ_P(var)         FAST_PATH_SHARED_READ_P(var)
# define MIXED_PATH_SHARED_READ_F(var)         FAST_PATH_SHARED_READ_D(var)
# define MIXED_PATH_SHARED_READ_D(var)         FAST_PATH_SHARED_READ_D(var)
# define MIXED_PATH_SHARED_WRITE(var, val)     FAST_PATH_SHARED_WRITE(var, val)
# define MIXED_PATH_SHARED_WRITE_P(var, val)   FAST_PATH_SHARED_WRITE_P(var, val)
# define MIXED_PATH_SHARED_WRITE_D(var, val)   FAST_PATH_SHARED_WRITE_D(var, val)

#    define TM_EARLY_RELEASE(var)         
/*
#      define P_MALLOC(size)            malloc(size)
#      define P_FREE(ptr)               free(ptr)
#      define SEQ_MALLOC(size)          malloc(size)
#      define SEQ_FREE(ptr)             free(ptr)

#      define TM_MALLOC(size)           malloc(size)
#      define FAST_PATH_FREE(ptr)        free(ptr)
#      define SLOW_PATH_FREE(ptr)
*/

#      define P_FREE(ptr)               /*free(ptr)*/
//#      define TM_MALLOC(size)           stm_malloc(size)
#  define TM_MALLOC(size)       ({ \
                                    void* memory_requested=(void*)STM_pointer[threadID].pointer; \
                                    STM_pointer[threadID].pointer+=size; \
                                    memory_requested; }) 


#      define P_MALLOC(size)            TM_MALLOC(size)
#       define TM_FREE                  /* Do nothing, our allocator does the freeing in the end */
#      define FAST_PATH_FREE(ptr)       /*free(ptr)*/
#      define SLOW_PATH_FREE(ptr)       /*stm_free(ptr, sizeof(stm_word_t)) */

# define FAST_PATH_RESTART() __TM_abort();
# define FAST_PATH_SHARED_READ(var) (var)
# define FAST_PATH_SHARED_READ_P(var) (var)
# define FAST_PATH_SHARED_READ_D(var) (var)
# define FAST_PATH_SHARED_READ_F(var) (var)
# define FAST_PATH_SHARED_WRITE(var, val) ({HTM_WRITE((vintp*)(void*)&(var), (intptr_t)val, next_commit); var = val; var;})
# define FAST_PATH_SHARED_WRITE_P(var, val) ({HTM_WRITE((vintp*)(void*)&(var), VP2IP(val), next_commit); var = val; var;})
# define FAST_PATH_SHARED_WRITE_D(var, val)({HTM_WRITE((vintp*)DP2IPP(&(var)), D2IP(val), next_commit); var = val; var;})
# define FAST_PATH_SHARED_WRITE_F(var, val)({HTM_WRITE((vintp*)FP2IPP(&(var)), F2IP(val), next_commit); var = val; var;})
# define HTM_SHARED_WRITE(var, val)  FAST_PATH_SHARED_WRITE(var, val)
# define HTM_SHARED_READ(var) FAST_PATH_SHARED_READ(var)
# define HTM_SHARED_READ_P(var) FAST_PATH_SHARED_READ_P(var)

# define SLOW_PATH_RESTART() STM_RESTART();
# define SLOW_PATH_SHARED_READ(var)           STM_READ(var)
# define SLOW_PATH_SHARED_READ_P(var)         STM_READ_P(var)
# define SLOW_PATH_SHARED_READ_F(var)         STM_READ_F(var)
# define SLOW_PATH_SHARED_READ_D(var)         STM_READ_D(var)
# define SLOW_PATH_SHARED_WRITE(var, val)     STM_WRITE((var), val)
# define SLOW_PATH_SHARED_WRITE_P(var, val)   STM_WRITE_P((var), val)
# define SLOW_PATH_SHARED_WRITE_D(var, val)   STM_WRITE_D((var), val)
# define SLOW_PATH_SHARED_WRITE_F(var, val)   STM_WRITE_F((var), val)

# define BAILOUT_RESTART() FAST_PATH_RESTART()
# define BAILOUT_SHARED_READ(var)           FAST_PATH_SHARED_READ(var)
# define BAILOUT_SHARED_READ_P(var)         FAST_PATH_SHARED_READ_P(var)
# define BAILOUT_SHARED_READ_F(var)         FAST_PATH_SHARED_READ_D(var)
# define BAILOUT_SHARED_READ_D(var)         FAST_PATH_SHARED_READ_D(var)
# define BAILOUT_SHARED_WRITE(var, val)     FAST_PATH_SHARED_WRITE(var, val)
# define BAILOUT_SHARED_WRITE_P(var, val)   FAST_PATH_SHARED_WRITE_P(var, val)
# define BAILOUT_SHARED_WRITE_D(var, val)   FAST_PATH_SHARED_WRITE_D(var, val)

# define STM_SHARED_READ(var)       STM_READ(var)
# define STM_SHARED_READ_P(var)       STM_READ_P(var)
# define STM_SHARED_WRITE(var,val)  STM_WRITE((var), val)
# define STM_SHARED_WRITE_P(var,val)  STM_WRITE_P((var), val)

#  define TM_LOCAL_WRITE(var, val)      ({var = val; var;})
#  define TM_LOCAL_WRITE_P(var, val)    ({var = val; var;})
#  define TM_LOCAL_WRITE_D(var, val)    ({var = val; var;})

#endif /* TM_H */
