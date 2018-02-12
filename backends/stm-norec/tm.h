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

#  include <string.h>
#  include "stm_norec.h"
#  include "norec.h"
#ifndef REDUCED_TM_API
#  include "thread.h"
#endif

#ifdef REDUCED_TM_API
#    define norec_Self                        TM_ARG_ALONE
#    define TM_ARG_ALONE                get_thread()
#    define SPECIAL_THREAD_ID()         get_tid()
#    define SPECIAL_INIT_THREAD(id)     thread_desc[id] = (void*)TM_ARG_ALONE;
#    define TM_THREAD_ENTER()         norec_Thread* inited_thread = norec_STM_NEW_THREAD(); \
                                      norec_STM_INIT_THREAD(inited_thread, SPECIAL_THREAD_ID()); \
                                      threadID=SPECIAL_THREAD_ID(); \
                                      thread_desc[SPECIAL_THREAD_ID()] = (void*)inited_thread;
#else
#    define TM_ARG_ALONE                  norec_STM_SELF
#    define SPECIAL_THREAD_ID()         thread_getId()
#    define TM_ARGDECL                    norec_STM_THREAD_T* TM_ARG
#    define TM_ARGDECL_ALONE              norec_STM_THREAD_T* TM_ARG_ALONE
#    define TM_THREAD_ENTER()         TM_ARGDECL_ALONE = norec_STM_NEW_THREAD(); \
                                        threadID=SPECIAL_THREAD_ID(); \
                                      norec_STM_INIT_THREAD(TM_ARG_ALONE, SPECIAL_THREAD_ID());
#endif
#    define TM_CALLABLE                   /* nothing */
#    define TM_ARG                        TM_ARG_ALONE,
#    define TM_THREAD_EXIT()          norec_STM_FREE_THREAD(TM_ARG_ALONE)

#   define AL_LOCK(x)

# define SETUP_NUMBER_TASKS(p);
# define SETUP_NUMBER_THREADS(p);
# define PRINT_STATS()

#      define TM_STARTUP(numThread,useless)     norec_STM_STARTUP(); \
                                               number_threads=numThread; \
                        threadID = SPECIAL_THREAD_ID(); \ 
                        int loop_counter_startup=0; \
                        /*Calculate the offset between the two heaps*/ \
                        memory_per_thread=(NUM_ELEMENTS_ARRAY/numThread); \
                    for(loop_counter_startup=0;loop_counter_startup<numThread;loop_counter_startup++) {\
                        STM_pointer[loop_counter_startup].pointer=STM_Heap+(memory_per_thread*loop_counter_startup); \
                        /*printf("HTM_Heap is %ld and pointer is %ld\n",HTM_Heap,STM_pointer[loop_counter_startup].pointer); */\
                    }
 
#      define TM_SHUTDOWN() { \
        norec_STM_SHUTDOWN(); \
    unsigned long commits = 0; \
    unsigned long aborts = 0; \
    unsigned long lookup_HTM = 0; \
    unsigned long lookup_STM = 0; \
    unsigned long lookup_STM_small = 0; \
    unsigned long insert_HTM = 0; \ 
    unsigned long insert_STM = 0; \
    unsigned long insert_STM_small = 0; \
    int i = 0; \
    for (; i < 128; i++) { \
       /*if (statistics_array[i].commits == 0) { break; }*/ \
       commits += statistics_array[i].commits; \
       aborts += statistics_array[i].aborts; \
            lookup_HTM += statistics_array[i].lookup_HTM; \
            lookup_STM += statistics_array[i].lookup_STM; \
            lookup_STM_small += statistics_array[i].lookup_STM_small; \
            insert_STM += statistics_array[i].insert_STM; \
            insert_HTM += statistics_array[i].insert_HTM; \
            insert_STM_small += statistics_array[i].insert_STM_small; \
    } \
    total_commits=commits; \
    printf("Total lookup: %lu\n\tHTM lookup: %lu\n\tSTM lookup: %lu\n\tSTM small lookup %lu\nTotal inserts: %lu\n\tHTM insert: %lu\n\tSTM inser: %lu\n\tSTM small insert: %lu\nTotal commits: %lu\nNumber of aborts: %lu\n", lookup_HTM+lookup_STM+lookup_STM_small,lookup_HTM,lookup_STM,lookup_STM_small,insert_HTM+insert_STM+insert_STM_small,insert_HTM,insert_STM,insert_STM_small, commits, aborts); \
}
/*
#      define P_MALLOC(size)            malloc(size)
#      define P_FREE(ptr)               free(ptr)
#      define TM_MALLOC(size)           malloc(size)
#      define FAST_PATH_FREE(ptr)       free(ptr)
#      define SLOW_PATH_FREE(ptr)        FAST_PATH_FREE(ptr)
#      define TM_FREE(ptr)              free(ptr)
*/

#  define TM_MALLOC(size)       ({ \
                                                    if(threadID>80){ \
                                                       round_robin_counter++; \
                                                        threadID=round_robin_counter%number_threads; \
                                                    } \
                                                    memory_requested=STM_pointer[threadID].pointer; \
                                                    /*printf("memory requested is %ld\n",memory_requested); */\
                                                    STM_pointer[threadID].pointer+=(size); \
                                                    memory_requested; \
                                })



#      define P_FREE(ptr)               /*free(ptr)*/
#  define P_MALLOC(size)                TM_MALLOC(size)
#  define FAST_PATH_FREE(ptr)            P_FREE(ptr)
#  define SLOW_PATH_FREE(ptr)            P_FREE(ptr)
#  define TM_FREE(ptr)                   P_FREE(ptr)

# define TM_BEGIN(b,mode)     TM_BEGIN_EXT(b,0)

# define TM_BEGIN_EXT(b,ro)    { \
    local_exec_mode=1; \
    norec_STM_BEGIN(ro); \
    statistics_array[SPECIAL_THREAD_ID()].aborts++; \
}; 

#    define TM_END() {  \
        norec_STM_END(); \
    statistics_array[SPECIAL_THREAD_ID()].aborts--; \
    statistics_array[SPECIAL_THREAD_ID()].commits++; \
};

#define FAST_PATH_BEGIN(b,ro)     TM_BEGIN_EXT(b,ro) 
#define SLOW_PATH_BEGIN(b,ro)     TM_BEGIN_EXT(b,ro) 

#define FAST_PATH_END()     TM_END()
#define SLOW_PATH_END()     TM_END()

#    define FAST_PATH_RESTART()          norec_STM_RESTART()
#    define SLOW_PATH_RESTART()          FAST_PATH_RESTART()

#  define TM_EARLY_RELEASE(var)         /* nothing */

#  define FAST_PATH_SHARED_READ(var)     var
#  define FAST_PATH_SHARED_READ_P(var)   var
#  define FAST_PATH_SHARED_READ_D(var)   var
#  define FAST_PATH_SHARED_READ_F(var)   var
#  define FAST_PATH_SHARED_WRITE(var, val)     ({var = val; var;})
#  define FAST_PATH_SHARED_WRITE_P(var, val)   ({var = val; var;})
#  define FAST_PATH_SHARED_WRITE_D(var, val)   ({var = val; var;})
#  define FAST_PATH_SHARED_WRITE_F(var, val)   ({var = val; var;})

# define STM_SHARED_READ(var)             SLOW_PATH_SHARED_READ(var)
# define STM_SHARED_READ_P(var)           SLOW_PATH_SHARED_READ_P(var)
# define STM_SHARED_WRITE(var, val)       SLOW_PATH_SHARED_WRITE(var, val)
# define STM_SHARED_WRITE_P(var, val)     SLOW_PATH_SHARED_WRITE_P(var, val)
# define HTM_SHARED_READ(var)           FAST_PATH_SHARED_READ(var)
# define HTM_SHARED_WRITE(var, val)     FAST_PATH_SHARED_WRITE(var, val)

# define SLOW_PATH_SHARED_READ(var)             norec_STM_READ(var)
# define SLOW_PATH_SHARED_READ_P(var)         norec_STM_READ_P(var)
# define SLOW_PATH_SHARED_READ_F(var)         norec_STM_READ_F(var)
# define SLOW_PATH_SHARED_READ_D(var)         norec_STM_READ_D(var)
# define SLOW_PATH_SHARED_WRITE(var, val)     norec_STM_WRITE((var), val)
# define SLOW_PATH_SHARED_WRITE_P(var, val)   norec_STM_WRITE_P((var), val)
# define SLOW_PATH_SHARED_WRITE_D(var, val)   norec_STM_WRITE_D((var), val)
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

#  define TM_LOCAL_WRITE(var, val)      norec_STM_LOCAL_WRITE(var, val)
#  define TM_LOCAL_WRITE_P(var, val)    norec_STM_LOCAL_WRITE_P(var, val)
#  define TM_LOCAL_WRITE_D(var, val)    norec_STM_LOCAL_WRITE_D(var, val)

#endif /* TM_H */
