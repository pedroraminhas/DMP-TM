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

# define AL_LOCK(b)
# define PRINT_STATS()
# define SETUP_NUMBER_TASKS(b)
# define SETUP_NUMBER_THREADS(b)

//#include <immintrin.h>
//#include <rtmintrin.h>
#include <htmxlintrin.h>

#include "stm_norec.h"
#include "norec.h"

#ifdef REDUCED_TM_API
#    define norec_Self                        TM_ARG_ALONE
#    define TM_ARG_ALONE                get_thread()
#    define SPECIAL_THREAD_ID()         get_tid()
#    define SPECIAL_INIT_THREAD(id)     thread_desc[id] = (void*)TM_ARG_ALONE;
#    define TM_THREAD_ENTER()         norec_Thread* inited_thread = norec_STM_NEW_THREAD(); \
                                      norec_STM_INIT_THREAD(inited_thread, SPECIAL_THREAD_ID()); \
                                      threadID =SPECIAL_THREAD_ID(); \
                                      thread_desc[SPECIAL_THREAD_ID()] = (void*)inited_thread;
#else
#    define TM_ARG_ALONE                  norec_STM_SELF
#    define SPECIAL_THREAD_ID()         thread_getId()
#    define TM_ARGDECL                    norec_STM_THREAD_T* TM_ARG
#    define TM_ARGDECL_ALONE              norec_STM_THREAD_T* TM_ARG_ALONE
#    define TM_THREAD_ENTER()         TM_ARGDECL_ALONE = norec_STM_NEW_THREAD(); \
                                      threadID = SPECIAL_THREAD_ID(); \
                                      norec_STM_INIT_THREAD(TM_ARG_ALONE, SPECIAL_THREAD_ID());
#endif
#    define TM_CALLABLE                   /* nothing */
#    define TM_ARG                        TM_ARG_ALONE,
#    define TM_THREAD_EXIT()          norec_STM_FREE_THREAD(TM_ARG_ALONE)

extern __inline long
__attribute__ ((__gnu_inline__, __always_inline__, __artificial__))
        __TM_is_self_conflict(void* const TM_buff)
{
          texasr_t texasr = __builtin_get_texasr ();
            return _TEXASR_SELF_INDUCED_CONFLICT (texasr);
}


#      define TM_STARTUP(numThread, useless) { \
    norec_STM_STARTUP(); \
	number_threads=numThread; \
                        threadID = 0; \ 
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
} \
}
 #define RETRY_POLICY 1

#  define TM_SHUTDOWN() { \
        norec_STM_SHUTDOWN(); \
    unsigned long htm_commits = 0; \
    unsigned long stm_commits = 0; \
    unsigned long conflicts = 0; \
    unsigned long capacity_aborts = 0; \
    unsigned long user_aborts = 0; \
    unsigned long other_aborts = 0; \
    unsigned long stm_aborts = 0; \
    unsigned long lookup_HTM = 0; \
    unsigned long lookup_STM = 0; \
    unsigned long lookup_STM_small = 0; \
    unsigned long insert_HTM = 0; \ 
    unsigned long insert_STM = 0; \
    unsigned long insert_STM_small = 0; \
    int i = 0; \
    for (; i < 128; i++) { \
       	    /*if ((statistics_array[i].htm_commits == 0) && (statistics_array[i].stm_commits == 0)) { break; } */\
            htm_commits += statistics_array[i].htm_commits; \
            stm_commits += statistics_array[i].stm_commits; \
            conflicts += statistics_array[i].conflicts; \
            capacity_aborts += statistics_array[i].capacity_aborts; \
            user_aborts += statistics_array[i].user_aborts; \
            other_aborts += statistics_array[i].other_aborts; \
            stm_aborts += statistics_array[i].stm_aborts; \
            lookup_HTM += statistics_array[i].lookup_HTM; \
            lookup_STM += statistics_array[i].lookup_STM; \
            lookup_STM_small += statistics_array[i].lookup_STM_small; \
            insert_STM += statistics_array[i].insert_STM; \
            insert_HTM += statistics_array[i].insert_HTM; \
            insert_STM_small += statistics_array[i].insert_STM_small; \
    } \
    total_commits += htm_commits + stm_commits; \
    printf("Total lookup: %lu\n\tHTM lookup: %lu\n\tSTM lookup: %lu\n\tSTM small lookup %lu\nTotal inserts: %lu\n\tHTM insert: %lu\n\tSTM inser: %lu\n\tSTM small insert: %lu\nTotal commits: %lu\n\tHTM commits: %lu\n\tSGL commits: %lu\n\tSTM commits: %lu\nTotal Aborts: %lu\n\tHTM conflicts: %lu\n\tHTM capacity aborts: %lu\n\tHTM user aborts: %lu\n\tHTM other aborts: %lu\n\tSTM Aborts: %lu\n", lookup_HTM+lookup_STM+lookup_STM_small,lookup_HTM,lookup_STM,lookup_STM_small,insert_HTM+insert_STM+insert_STM_small,insert_HTM,insert_STM,insert_STM_small, htm_commits+stm_commits,htm_commits, 0,stm_commits, stm_aborts+conflicts+capacity_aborts+user_aborts+other_aborts,conflicts,capacity_aborts,user_aborts,other_aborts,stm_aborts); \
}

#  define TM_BEGIN_WAIVER()
#  define TM_END_WAIVER()

# define TM_BEGIN(b,aux)     TM_BEGIN_EXT(b,0)
# define SPEND_BUDGET(b)    if(RETRY_POLICY == 0) (*b)=0; else if (RETRY_POLICY == 2) (*b)=(*b)/2; else (*b)=--(*b);

# define TM_BEGIN_EXT(b,ro) \
    { \
        /*num_tx=b;*/ \
        read_only_htm = 1; \
        int tries = HTM_RETRIES; \
        local_exec_mode=0; \
    	while (1) { \
    		if (tries > 0) { \
                	while (fallback_in_use.counter != 0) { __asm__ ( "nop;" ); } \
			TM_buff_type TM_buff; \
    			unsigned char status = __TM_begin(&TM_buff);	\
    			if (status == _HTM_TBEGIN_STARTED) { \
    				if (exists_sw.counter != 0) { __TM_abort(); } \
    				break;	\
    			} \
			if(__TM_is_conflict(&TM_buff)){ \
				tries--; \
		                statistics_array[SPECIAL_THREAD_ID()].conflicts++; \    
	                } \
		        else if (__TM_is_user_abort(&TM_buff)) { \
				tries--; \
		                statistics_array[SPECIAL_THREAD_ID()].user_aborts++; \
            		} \
    	      		else if (__TM_is_footprint_exceeded(&TM_buff)) { \
				statistics_array[SPECIAL_THREAD_ID()].capacity_aborts++; \
               			SPEND_BUDGET(&tries); \
            		} \
            		else { \
               			tries--; \
				statistics_array[SPECIAL_THREAD_ID()].other_aborts++; \
            		} \
    		} else {  \
                    local_exec_mode=1; \
                	__sync_add_and_fetch(&exists_sw.counter,1); \
                	norec_STM_BEGIN(ro);   \
        			statistics_array[SPECIAL_THREAD_ID()].stm_aborts++; \
    			break;  \
    		} \
    	} \
    };

#    define TM_END() { \
		if (local_exec_mode==0) {	\
			if (read_only_htm == 0 && exists_sw.counter != 0) { \
                __TM_suspend(); \
				norec_ATOMIC_INC_CLOCK(); \
                __TM_resume(); \
			} \
			__TM_end();	\
			statistics_array[SPECIAL_THREAD_ID()].htm_commits++; \
		} else {	\
    		__sync_add_and_fetch(&fallback_in_use.counter,1);   \
			int ret = norec_HYBRID_STM_END();  \
             __sync_sub_and_fetch(&fallback_in_use.counter,1);    \
			if (ret == 0) { \
				norec_STM_RESTART(); \
			} \
                __sync_sub_and_fetch(&exists_sw.counter,1); \
		        statistics_array[SPECIAL_THREAD_ID()].stm_commits++; \
			    statistics_array[SPECIAL_THREAD_ID()].stm_aborts--; \
		} \
    };

# define FAST_PATH_BEGIN(b,ro)    TM_BEGIN_EXT(b,ro) 
# define SLOW_PATH_BEGIN(b,ro)    TM_BEGIN_EXT(b,ro)
# define FAST_PATH_END()            TM_END()
# define SLOW_PATH_END()            TM_END()


#    define TM_EARLY_RELEASE(var)         


/*#      define P_MALLOC(size)            malloc(size)
#      define P_FREE(ptr)               free(ptr)
#      define SEQ_MALLOC(size)          malloc(size)
#      define SEQ_FREE(ptr)             free(ptr)

#      define TM_MALLOC(size)           malloc(size)
#      define FAST_PATH_FREE(ptr)        free(ptr)
#      define SLOW_PATH_FREE(ptr)       FAST_PATH_FREE(ptr)
#       define TM_FREE(ptr)             FAST_PATH_FREE(ptr)
*/

#  define TM_MALLOC(size)       ({ \
                                                    void* memory_requested=(void*)STM_pointer[threadID].pointer; \
                                                    /*printf("memory requested is %ld\n",memory_requested); */\
                                                    STM_pointer[threadID].pointer+=size; \
                                                    memory_requested; \
                                })


#      define P_FREE(ptr)               /*free(ptr)*/
//#  define P_MALLOC(size)                malloc(size)
#  define P_MALLOC(size)     TM_MALLOC(size)
#  define FAST_PATH_FREE(ptr)            P_FREE(ptr)
#  define SLOW_PATH_FREE(ptr)            P_FREE(ptr)
#  define TM_FREE(ptr)                   P_FREE(ptr)

# define FAST_PATH_RESTART() __TM_abort();
# define FAST_PATH_SHARED_READ(var) (var)
# define FAST_PATH_SHARED_READ_P(var) (var)
# define FAST_PATH_SHARED_READ_D(var) (var)
# define FAST_PATH_SHARED_READ_F(var) (var)
# define FAST_PATH_SHARED_WRITE(var, val) ({var = val; read_only_htm = 0; var;})
# define FAST_PATH_SHARED_WRITE_P(var, val) ({var = val; read_only_htm = 0; var;})
# define FAST_PATH_SHARED_WRITE_D(var, val) ({var = val; read_only_htm = 0; var;})
# define FAST_PATH_SHARED_WRITE_F(var, val) ({var = val; read_only_htm = 0; var;})

# define STM_SHARED_READ(var)           norec_STM_READ(var)
# define STM_SHARED_WRITE(var, val)     norec_STM_WRITE((var), val)
# define HTM_SHARED_READ(var)           FAST_PATH_SHARED_READ(var)
# define HTM_SHARED_READ_P(var)           FAST_PATH_SHARED_READ_P(var)
# define HTM_SHARED_WRITE(var, val)     FAST_PATH_SHARED_WRITE(var, val)

# define SLOW_PATH_RESTART() norec_STM_RESTART()
# define SLOW_PATH_SHARED_READ(var)           norec_STM_READ(var)
# define SLOW_PATH_SHARED_READ_P(var)         norec_STM_READ_P(var)
# define SLOW_PATH_SHARED_READ_F(var)         norec_STM_READ(var)
# define SLOW_PATH_SHARED_READ_D(var)         norec_STM_READ_F(var)
# define SLOW_PATH_SHARED_WRITE(var, val)     norec_STM_WRITE((var), val)
# define SLOW_PATH_SHARED_WRITE_P(var, val)   norec_STM_WRITE_P((var), val)
# define SLOW_PATH_SHARED_WRITE_D(var, val)   norec_STM_WRITE_F((var), val)
# define SLOW_PATH_SHARED_WRITE_F(var, val)  norec_STM_WRITE_F((var), val)

# define BAILOUT_SHARED_READ(var)           norec_STM_READ(var)
# define BAILOUT_SHARED_WRITE(var, val)     norec_STM_WRITE((var), val)
# define BAILOUT_SHARED_READ_P(var)      norec_STM_READ_P(var)
# define BAILOUT_SHARED_WRITE_P(var,val)      norec_STM_WRITE_P((var), val)
# define BAILOUT_RESTART()          norec_STM_RESTART()

# define MIXED_PATH_RESTART() FAST_PATH_RESTART()
# define MIXED_PATH_SHARED_READ(var)           FAST_PATH_SHARED_READ(var)
# define MIXED_PATH_SHARED_READ_P(var)         FAST_PATH_SHARED_READ_P(var)
# define MIXED_PATH_SHARED_READ_F(var)         FAST_PATH_SHARED_READ_D(var)
# define MIXED_PATH_SHARED_READ_D(var)         FAST_PATH_SHARED_READ_D(var)
# define MIXED_PATH_SHARED_WRITE(var, val)     FAST_PATH_SHARED_WRITE(var, val)
# define MIXED_PATH_SHARED_WRITE_P(var, val)   FAST_PATH_SHARED_WRITE_P(var, val)
# define MIXED_PATH_SHARED_WRITE_D(var, val)   FAST_PATH_SHARED_WRITE_D(var, val)

# define STM_SHARED_READ_P(var) norec_STM_READ_P(var)
# define  STM_SHARED_WRITE_P(var, val) norec_STM_WRITE_P((var), val)

#  define TM_LOCAL_WRITE(var, val)      ({var = val; var;})
#  define TM_LOCAL_WRITE_P(var, val)    ({var = val; var;})
#  define TM_LOCAL_WRITE_D(var, val)    ({var = val; var;})

#endif /* TM_H */
