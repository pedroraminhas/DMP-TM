#ifndef TM_H
#define TM_H 1

#  include <stdio.h>
#  include <unistd.h>


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
# define LOCKED(lock)        *((volatile long*)(&lock)) != 0


#  include <string.h>
#include <htmxlintrin.h>
#ifndef REDUCED_TM_API
#  include "thread.h"
#endif

#    define TM_ARG                        /* nothing */
#    define TM_ARG_ALONE                  /* nothing */
#    define TM_ARGDECL                    /* nothing */
#    define TM_ARGDECL_ALONE              /* nothing */
#    define TM_CALLABLE                   /* nothing */

#      include <mod_mem.h>
#      include <mod_stats.h>

#ifdef REDUCED_TM_API
#    define SPECIAL_THREAD_ID()         get_tid()
#else
#    define SPECIAL_THREAD_ID()         thread_getId()
#endif

#define DISJOINT_PARTITIONS 2000
#define MICROBENCHMARK 0
#define REVIVE_MODE_THRESHOLD 5
#define THRESHOLD_CAPACITY 0.8
extern __inline long
__attribute__ ((__gnu_inline__, __always_inline__, __artificial__))
__TM_is_nontrans_conflict(void* const TM_buff)
{
texasr_t texasr = __builtin_get_texasr ();
return _TEXASR_NON_TRANSACTIONAL_CONFLICT (texasr);
}

#define AL_LOCK(idx)


# define SETUP_NUMBER_TASKS(p);
# define SETUP_NUMBER_THREADS(p);
# define PRINT_STATS()

#define INIT_HEAP_read(threadID,limit) \
                            int init_heap_loop=0; \
                            for(init_heap_loop=0;init_heap_loop<limit;init_heap_loop++){ \
                                    pages_read_heap[threadID][init_heap_loop]=-1; \
                            }


#define INIT_HEAP_write(threadID,limit) \
                            int init_heap_write_loop=0; \
                            for(init_heap_write_loop=0;init_heap_write_loop<limit;init_heap_write_loop++){ \
                                    pages_written_heap[threadID][init_heap_write_loop]=-1; \
                            }

#define INIT_METADATA()   \
                        int init_metadata_loop_counter=0; \
                        for (init_metadata_loop_counter=0;init_metadata_loop_counter<NUM_PAGES;init_metadata_loop_counter++){ \
                            metadata_array[init_metadata_loop_counter].status_field = PROT_READ|PROT_WRITE; \
                            metadata_array[init_metadata_loop_counter].writer_count = 0; \
                            metadata_array[init_metadata_loop_counter].transition_count = 0; \
                            metadata_array[init_metadata_loop_counter].lock_bit = 0; \
                        }


#define INIT_METADATA_SMART()   \
                        int init_metadata_loop_counter=0; \
                        for (init_metadata_loop_counter=0;init_metadata_loop_counter<NUM_PAGES;init_metadata_loop_counter++){ \
                           if((init_metadata_loop_counter%3)%2 == 1){ \
                                metadata_array[init_metadata_loop_counter].status_field = PROT_NONE; \
                                /*mprotect((void *)page_first_address+(PAGE_SIZE*init_metadata_loop_counter),PAGE_SIZE,0);*/ \
                            }else{ \
                                metadata_array[init_metadata_loop_counter].status_field = PROT_READ|PROT_WRITE; \
                            } \
                            metadata_array[init_metadata_loop_counter].writer_count = 0; \
                            metadata_array[init_metadata_loop_counter].transition_count = 0; \
                            metadata_array[init_metadata_loop_counter].lock_bit = 0; \
                        }


#define INIT_METADATA_MICROBENCHMARK()   \
                        int init_metadata_loop_counter=0; \
                        for (init_metadata_loop_counter=0;init_metadata_loop_counter<NUM_PAGES;init_metadata_loop_counter++){ \
                                metadata_array[init_metadata_loop_counter].status_field = PROT_NONE; \
                                /*mprotect((void *)page_first_address+(PAGE_SIZE*init_metadata_loop_counter),PAGE_SIZE,0);*/ \
                            metadata_array[init_metadata_loop_counter].writer_count = 0; \
                            metadata_array[init_metadata_loop_counter].transition_count = 0; \
                            metadata_array[init_metadata_loop_counter].lock_bit = 0; \
                        }

#define handle_error(msg) \
    do { perror(msg); exit(EXIT_FAILURE); } while (0)

#define TM_STARTUP(numThread,useless)     \
                                            stm_init(); \
                                            mod_mem_init(0); \
                                            if (getenv("STM_STATS") != NULL) { \
                                                mod_stats_init(); \
                                            } \
                        threadID = 0; \
                        int loop_counter_startup=0; \
                        number_threads = numThread; \
                        /*Initialization of metadata */ \
                        metadata_array = (metadata*) malloc(sizeof(metadata) * NUM_PAGES); \
                        /*SIGSEGV Handler initialization*/ \
                        struct sigaction sa; \
                        sa.sa_flags = SA_SIGINFO; \
                        sigemptyset(&sa.sa_mask); \
                        sa.sa_sigaction = handler; \
                        if (sigaction(SIGSEGV, &sa, NULL) == -1) \
                            handle_error("sigaction"); \
                        /*Calculate the offset between the two heaps*/ \
                            offset_heaps= HTM_Heap - STM_Heap; \
                        /*Calculate the first address of the page that contains the begginning of HTM Heap*/ \
                        page_first_address= ((long int) HTM_Heap) & ~(PAGE_SIZE-1); \
                        STM_page_first_address = ((long int) STM_Heap) & ~(PAGE_SIZE-1); \
                        memory_per_thread=(NUM_ELEMENTS_ARRAY/numThread); \
                        for(loop_counter_startup=0;loop_counter_startup<numThread;loop_counter_startup++) {\
                            STM_pointer[loop_counter_startup].pointer=STM_Heap+(memory_per_thread*loop_counter_startup); \
                        } \
                         /*Init metadata of each page*/ \
                        INIT_METADATA();


#      define TM_SHUTDOWN()             \
                    /*Free memory*/ \
                    free(metadata_array); \
                                            if (getenv("STM_STATS") != NULL) { \
                                                unsigned long u; \
                                                if (stm_get_global_stats("global_nb_commits", &u) != 0) \
                                                    printf("#commits    : %lu\n", u); \
                                                if (stm_get_global_stats("global_nb_aborts", &u) != 0) \
                                                    printf("#aborts     : %lu\n", u); \
                                                if (stm_get_global_stats("global_max_retries", &u) != 0) \
                                                     printf("Max retries : %lu\n", u); \
                                            } \
                                        stm_exit(); \
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



#      define TM_THREAD_ENTER()         ({  threadID=SPECIAL_THREAD_ID(); \
                                            stm_init_thread();})

#      define TM_THREAD_EXIT()          ({  stm_exit_thread(); \
                                        })

# define SPEND_BUDGET(b)    if(RETRY_POLICY == 0) (*b)=0; else if (RETRY_POLICY == 2) (*b)=(*b)/2; else (*b)=--(*b);

//#      define P_MALLOC(size)            malloc(size)
//#      define P_FREE(ptr)               free(ptr)
//#      define TM_MALLOC(size)           stm_malloc(size)
//#      define FAST_PATH_FREE(ptr)       free(ptr)
//#      define SLOW_PATH_FREE(ptr)       stm_free(ptr, sizeof(stm_word_t))
//#       define TM_FREE(ptr)             free(ptr)
//#      define P_MALLOC(size)            malloc(size)

#      define P_FREE(ptr)               /*free(ptr)*/
//#      define TM_MALLOC(size)           stm_malloc(size)
#  define TM_MALLOC(size)       ({ \
                                    /*printf("threadID is %ld\n",threadID);*/\
                                    /*printf("in P_MALLOC with size %ld\n",size);*/ \
                                    /*printf("passed here\n");*/ \
                                                                        /*if(threadID>80){ \
                                       round_robin_counter++; \
                                        threadID=round_robin_counter%number_threads; \
                                    } */\
                                    void* memory_requested=(void*)STM_pointer[threadID].pointer; \
                                    STM_pointer[threadID].pointer+=size; \
                                                                        /*if(STM_pointer[threadID].pointer > HTM_Heap+(memory_per_thread*(threadID+1)))  \
                                                                                printf("%d: %ld\t%ld\n",threadID,STM_pointer[threadID].pointer, HTM_Heap+memory_per_thread*threadID); */\
                                    /*printf("memory previously requested is %ld\n",memory_requested); */\
                                    memory_requested; })

#      define P_MALLOC(size)            TM_MALLOC(size)
#       define TM_FREE                  /* Do nothing, our allocator does the freeing in the end */
#      define FAST_PATH_FREE(ptr)       /*free(ptr)*/
#      define SLOW_PATH_FREE(ptr)       /*stm_free(ptr, sizeof(stm_word_t)) */





#define FAST_PATH_BEGIN(b,ro){ \
        int tries = HTM_RETRIES; \
        num_tx=b; \
        update_transaction=!ro; \
		local_exec_mode = 0; \
       	while(1){ \
						while (LOCKED(single_global_lock.counter)) { \
							asm volatile ("nop"); \
                        } \
                        TM_buff_type TM_buff; \
                        unsigned char status = __TM_begin(&TM_buff); \
                        if (status == _HTM_TBEGIN_STARTED) { \
							if (LOCKED(single_global_lock.counter)) { \
                                __TM_abort(); \
                            } \
                            break; \
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
                                if(tries<=0){ \
                		            int begin_loop_counter=0; \
									local_exec_mode = 2; \
									while(1){ \
                                    	  while(LOCKED(single_global_lock.counter)){ \
                                        	__asm__ ("nop;"); \
                                      	} \
                                      	if( __sync_val_compare_and_swap(&single_global_lock.counter, 0, 1) == 0) { \
                                        	break; \
                                      	} \
                                    } \
                                    for (begin_loop_counter=0;begin_loop_counter<number_threads;begin_loop_counter++){ \
                                      while(1){ \
                                        while(LOCKED(thread_lock[begin_loop_counter].counter)){ \
                                          __asm__ ("nop;"); \
                                        } \
                                        if(__sync_val_compare_and_swap(&thread_lock[begin_loop_counter].counter,0,1)==0){ \
                                          break; \
                                        } \
                                      } \
                                    } \
                                    break; \
                                  } \
                                }  \
                              };

# define FAST_PATH_END() { \
		if (local_exec_mode==0) { \
            __TM_end(); \
            statistics_array[SPECIAL_THREAD_ID()].htm_commits++; \
         }else{ \
       int loop_counter=0; \
       statistics_array[SPECIAL_THREAD_ID()].sgl_commits++; \
       single_global_lock.counter = 0; \
       for (loop_counter=0;loop_counter<number_threads;loop_counter++){ \
         thread_lock[loop_counter].counter=0; \
       } \
     } \
};


#define SLOW_PATH_BEGIN(b,ro) { \
                        num_tx=b; \
						local_exec_mode = 1; \
                        while(1){ \
                            while(LOCKED(thread_lock[SPECIAL_THREAD_ID()].counter)){ \
                                __asm__ ("nop;"); \
                            } \
                            if(__sync_val_compare_and_swap(&thread_lock[SPECIAL_THREAD_ID()].counter, 0, 1) == 0) { \
                                break; \
                            } \
                        } \
                        int loop_counter=0; \
                        int pageID=0; \
                        sigjmp_buf buf; \
                        sigsetjmp(buf, 0); \
                        stm_tx_attr_t _a = {}; \
                        _a.read_only = ro; \
                        stm_start(_a, &buf); \
                        sigsetjmp(buf, 0); \
                        statistics_array[SPECIAL_THREAD_ID()].stm_aborts++; \
                        local_transition_count=global_transition_count; \
};	

#define UNS(a) ((unsigned long)a)
#define FILTERHASH(a)                   ((UNS(a) >> 2) ^ (UNS(a) >> 5))
#define FILTERBITS(a)                   (1 << (FILTERHASH(a) & 0x1F))

#define SLOW_PATH_END(){ \ 
	    int pageID; \
        int loop_counter=0; \
        if(!counter_pages_written){ \
        if(local_transition_count != global_transition_count){ \
            for(loop_counter=0; loop_counter< counter_pages_read; loop_counter++){ \
                pageID=pages_read_heap[SPECIAL_THREAD_ID()][loop_counter]; \
                if(metadata_array[pageID].transition_count != transition_count_pages_read[threadID][pageID]){ \
                    SLOW_PATH_RESTART(); \
                    transition_count_pages_read[threadID][pageID] = 0; \
                }    \
            }   \
        }   \
        } \
        stm_commit(); \
        for(loop_counter=0; loop_counter < counter_pages_written; loop_counter++){ \
                pageID=pages_written_heap[SPECIAL_THREAD_ID()][loop_counter]; \
                writer_count[threadID][pageID]=0; \
            }   \
        statistics_array[SPECIAL_THREAD_ID()].stm_aborts--; \
        statistics_array[SPECIAL_THREAD_ID()].stm_commits++; \
		counter_pages_read=0; \
        counter_pages_written=0; \
        pages_read = 0; \
		pages_written = 0; \
        thread_lock[SPECIAL_THREAD_ID()].counter=0; \
};


#    define FAST_PATH_RESTART()                __TM_abort();
/*#    define SLOW_PATH_RESTART()                ({ \
                                                int loop_counter_abort; \
                                                int pageID; \
                                                for(loop_counter_abort=0;loop_counter_abort<counter_pages_written;loop_counter_abort++){ \
                                                    pageID=pages_written_heap[SPECIAL_THREAD_ID()][loop_counter_abort]; \
                                                    __sync_sub_and_fetch(&metadata_array[pageID].writer_count,1); \
                                                 } \
                                                 free(transition_count_pages_read); \
                                                 free(pages_written); \
                                                 free(pages_read); \
                                                counter_pages_read=0; \
                                                counter_pages_written=0; \
                                                stm_abort(0);})*/

#	define SLOW_PATH_RESTART()		stm_abort(0);



#    define TM_EARLY_RELEASE(var)       /* nothing */

#  include <wrappers.h>
//#  define FAST_PATH_SHARED_READ(var) *(long*)(((long)(&(var)))+offset_heaps)
//#  define FAST_PATH_SHARED_READ_F(var) (*(float*)(((long)(&(var)))+offset_heaps))
//#  define FAST_PATH_SHARED_READ_D(var) *(double*)(((long)(&(var)))+offset_heaps)
//#  define FAST_PATH_SHARED_READ_P(var) *(long*)(((long)(&(var)))+offset_heaps)
//#   define FAST_PATH_SHARED_READ_P(var) (var)        
        
//#  define FAST_PATH_SHARED_WRITE(var, val)  ({ *((long *)(((long)(&(var)))+offset_heaps)) = val;})
//#  define FAST_PATH_SHARED_WRITE_F(var, val)  ({ *((float *)(((long)(&(var)))+offset_heaps)) = val;})
//#  define FAST_PATH_SHARED_WRITE_D(var, val)  ({ *((double *)(((long)(&(var)))+offset_heaps)) = val;})
//#  define FAST_PATH_SHARED_WRITE_P(var, val)  ({ *((long*)(((long)(&(var)))+offset_heaps)) = val;})
//#  define FAST_PATH_SHARED_WRITE_P(var, val) ({var = val; var;})
/*
#  define STM_SHARED_READ(var)              stm_load((volatile stm_word_t *)(void *)&(var))
#  define STM_SHARED_WRITE(var, val)        stm_store((volatile stm_word_t *)(void *)&(var), (stm_word_t)val)
#  define STM_SHARED_READ_P(var)            stm_load_ptr((volatile void **)(void *)&(var))
#  define STM_SHARED_WRITE_P(var, val)      stm_store_ptr((volatile void **)(void *)&(var), val) 
# define HTM_PATH_SHARED_READ(var)           (var)
# define HTM_PATH_SHARED_READ_P(var)           (var)
# define HTM_PATH_SHARED_WRITE(var, val)     ({var = val; var;})
# define HTM_PATH_SHARED_WRITE_P(var, val)     ({var = val; var;})
*/
#  define FAST_PATH_SHARED_READ(var)    (var)
#  define FAST_PATH_SHARED_READ_P(var) (var)
#  define FAST_PATH_SHARED_READ_D(var)   (var)
#  define FAST_PATH_SHARED_READ_F(var)   (var)
#  define FAST_PATH_SHARED_WRITE(var, val)     ({var = val; var;})
#  define FAST_PATH_SHARED_WRITE_P(var, val)  ({var = val; var;})
#  define FAST_PATH_SHARED_WRITE_F(var, val)  ({var = val; var;})
#  define FAST_PATH_SHARED_WRITE_D(var, val)  ({var = val; var;})

# define MIXED_PATH_SHARED_READ(var)           FAST_PATH_SHARED_READ(var)
# define MIXED_PATH_SHARED_READ_P(var)         FAST_PATH_SHARED_READ_P(var)
# define MIXED_PATH_SHARED_READ_F(var)         FAST_PATH_SHARED_READ_D(var)
# define MIXED_PATH_SHARED_READ_D(var)         FAST_PATH_SHARED_READ_D(var)
# define MIXED_PATH_SHARED_WRITE(var, val)     FAST_PATH_SHARED_WRITE(var, val)
# define MIXED_PATH_SHARED_WRITE_P(var, val)   FAST_PATH_SHARED_WRITE_P(var, val)
# define MIXED_PATH_SHARED_WRITE_D(var, val)   FAST_PATH_SHARED_WRITE_D(var, val)

//# define SLOW_PATH_SHARED_READ(var)            stm_read(&var,metadata_array,page_first_address,STM_page_first_address, HTM_Heap,offset_heaps,pages_read,pages_written,transition_count_pages_read,local_transition_count,global_transition_count)

#define VALIDATE_READSET(){ \
	long temp_gtc = global_transition_count; \
    if(local_transition_count!=temp_gtc){ \
        for(loop_counter_read=0;loop_counter_read<counter_pages_read;loop_counter_read++){ \
            pageID_aux=pages_read_heap[SPECIAL_THREAD_ID()][loop_counter_read]; \
            long msk = FILTERBITS(pageID_aux); \
            if(((pages_read & msk) == msk) && (transition_count_pages_read[threadID][pageID_aux] != metadata_array[pageID_aux].transition_count)){ \
                    SLOW_PATH_RESTART(); \
            } \
        } \
        local_transition_count=temp_gtc; \
    } \
}

# define SLOW_PATH_SHARED_READ(var) ({    long value_read=0; \
    long pageID; \
    long transition_count=0; \
    int loop_counter_read; \
    long increment_inside_loop=0; \
    pageID = ((long int) &(var) - (long int)STM_page_first_address)>>16; \
    int first_time_page_read; \
    int pageID_aux; \
	VALIDATE_READSET(); \
    if(metadata_array[pageID].status_field == 3){ \
        while(1){ \
                        while(LOCKED(metadata_array[pageID].lock_bit)){ \
                            __asm__ ("nop;"); \
                        } \
                        if(__sync_val_compare_and_swap(&metadata_array[pageID].lock_bit, 0, 1) == 0) { \
                            break; \
                        } \
                    } \
		if(metadata_array[pageID].status_field == 3){ \
            mprotect((void *)page_first_address+(PAGE_SIZE*pageID),PAGE_SIZE,1); \
            statistics_array[SPECIAL_THREAD_ID()].system_calls++; \
            metadata_array[pageID].status_field = 1;    \
        } \
    	metadata_array[pageID].lock_bit=0; \
    } \
    long msk = FILTERBITS(pageID); \
    if(!((pages_read & msk) == msk)){ \
        pages_read |= msk; \
        pages_read_heap[SPECIAL_THREAD_ID()][counter_pages_read]=pageID; \
        transition_count_pages_read[threadID][pageID] = metadata_array[pageID].transition_count; \
        counter_pages_read++; \
    } \
    value_read= stm_load((volatile stm_word_t *)(void *)&(var)); \
    /*if(transition_count_pages_read[threadID][pageID_aux] != metadata_array[pageID_aux].transition_count){ \
            SLOW_PATH_RESTART(); \
    } */\
    if(local_transition_count!=global_transition_count){ \
        for(loop_counter_read=0;loop_counter_read<counter_pages_read;loop_counter_read++){ \
            pageID_aux=pages_read_heap[SPECIAL_THREAD_ID()][loop_counter_read]; \
            long msk = FILTERBITS(pageID_aux); \
            if(((pages_read & msk) == msk) && (transition_count_pages_read[threadID][pageID_aux] != metadata_array[pageID_aux].transition_count)){ \
                    SLOW_PATH_RESTART(); \
            } \
        /*if(pageID_aux==pageID &&  (transitiodddn_count_pages_read[pageID_aux] != metadata_array[pageID_aux].transition_count)) \
			transition_count=metadata_array[pageID_aux].transition_count; \
		} \
        local_transition_count=global_transition_count; \
        transition_count_pages_read[pageID] = transition_count; */\
    } \
    } \
    /*transition_count_pages_read[pageID] = local_transition_count;*/ \
    value_read;  \
})

# define SLOW_PATH_SHARED_READ_D(var) ({    double value_read=0; \
    /*rmb();*/ \
    /*printf("STM write with %ld mode", mode[num_tx]); */\
    long pageID; \
    long transition_count=0; \
    int loop_counter_read; \
    long increment_inside_loop=0; \
    pageID = ((long int) &(var) - (long int)STM_page_first_address)>>16; \
    int first_time_page_read; \
    int pageID_aux; \
	VALIDATE_READSET(); \
    /*if(!(pages_read[pageID])){*/ \
     /*   local_transition_count=global_transition_count;*/ \
      /*  first_time_page_read=1;*/  \
    /*}  */ \
    if(metadata_array[pageID].status_field == 3){ \
        while(1){ \
                        while(LOCKED(metadata_array[pageID].lock_bit)){ \
                            __asm__ ("nop;"); \
                        } \
                        if(__sync_val_compare_and_swap(&metadata_array[pageID].lock_bit, 0, 1) == 0) { \
                            break; \
                        } \
                    } \
		if(metadata_array[pageID].status_field == 3){ \
            mprotect((void *)page_first_address+(PAGE_SIZE*pageID),PAGE_SIZE,1); \
            statistics_array[SPECIAL_THREAD_ID()].system_calls++; \
            metadata_array[pageID].status_field = 1;    \
        } \
    	metadata_array[pageID].lock_bit=0; \
    } \
    long msk = FILTERBITS(pageID); \
    if(((pages_read & msk) == msk)){ \
        pages_read |= msk; \
        pages_read_heap[SPECIAL_THREAD_ID()][counter_pages_read]=pageID; \
        counter_pages_read++; \
        transition_count_pages_read[threadID][pageID] = metadata_array[pageID].transition_count; \
        /*transition_count = metadata_array[pageID].transition_count;*/ \
    } \
    value_read=stm_load_double((volatile double *)(void *)&(var)); \
	/*if(transition_count_pages_read[threadID][pageID_aux] != metadata_array[pageID_aux].transition_count){ \
            SLOW_PATH_RESTART(); \
    } */\
    if(local_transition_count!=global_transition_count){ \
        for(loop_counter_read=0;loop_counter_read<counter_pages_read;loop_counter_read++){ \
            pageID_aux=pages_read_heap[SPECIAL_THREAD_ID()][loop_counter_read]; \
            long msk = FILTERBITS(pageID_aux); \
            if(((pages_read & msk) == msk) && (transition_count_pages_read[threadID][pageID_aux] != metadata_array[pageID_aux].transition_count)){ \
                    SLOW_PATH_RESTART(); \
            } \
        /*if(pageID_aux==pageID &&  (transition_count_pages_read[pageID_aux] != metadata_array[pageID_aux].transition_count)) \
			transition_count=metadata_array[pageID_aux].transition_count; \
		} \
        local_transition_count=global_transition_count; \
        transition_count_pages_read[pageID] = transition_count; */\
    } \
    }\
    /*transition_count_pages_read[pageID] = local_transition_count;*/ \
    value_read;  \
})



# define SLOW_PATH_SHARED_READ_F(var) ({    float value_read=0; \
    long pageID; \
    long transition_count=0; \
    int loop_counter_read; \
    long increment_inside_loop=0; \
    pageID = ((long int) &(var) - (long int)STM_page_first_address)>>16; \
    int first_time_page_read; \
    int pageID_aux; \
	VALIDATE_READSET(); \
    /*if(!(pages_read[pageID])){*/ \
     /*   local_transition_count=global_transition_count;*/ \
      /*  first_time_page_read=1;*/  \
    /*}  */ \
    if(metadata_array[pageID].status_field == 3){ \
        while(1){ \
                        while(LOCKED(metadata_array[pageID].lock_bit)){ \
                            __asm__ ("nop;"); \
                        } \
                        if(__sync_val_compare_and_swap(&metadata_array[pageID].lock_bit, 0, 1) == 0) { \
                            break; \
                        } \
                    } \
		if(metadata_array[pageID].status_field == 3){ \
            mprotect((void *)page_first_address+(PAGE_SIZE*pageID),PAGE_SIZE,1); \
            statistics_array[SPECIAL_THREAD_ID()].system_calls++; \
            metadata_array[pageID].status_field = 1;    \
        } \
    	metadata_array[pageID].lock_bit=0; \
    } \
    long msk = FILTERBITS(pageID); \
    if(((pages_read & msk) == msk)){ \
        pages_read |= msk; \
        pages_read_heap[SPECIAL_THREAD_ID()][counter_pages_read]=pageID; \
        counter_pages_read++; \
        transition_count_pages_read[threadID][pageID] = metadata_array[pageID].transition_count; \
    } \
    value_read=stm_load_float((volatile float *)(void *)&(var)); \
	/*if(transition_count_pages_read[threadID][pageID_aux] != metadata_array[pageID_aux].transition_count){ \
            SLOW_PATH_RESTART(); \
    } */\
    if(local_transition_count!=global_transition_count){ \
        for(loop_counter_read=0;loop_counter_read<counter_pages_read;loop_counter_read++){ \
            pageID_aux=pages_read_heap[SPECIAL_THREAD_ID()][loop_counter_read]; \
            long msk = FILTERBITS(pageID_aux); \
            if(((pages_read & msk) == msk) && (transition_count_pages_read[threadID][pageID_aux] != metadata_array[pageID_aux].transition_count)){ \
                    SLOW_PATH_RESTART(); \
            } \
        /*if(pageID_aux==pageID &&  (transition_count_pages_read[pageID_aux] != metadata_array[pageID_aux].transition_count)) \
			transition_count=metadata_array[pageID_aux].transition_count; \
		} \
        local_transition_count=global_transition_count; \
        transition_count_pages_read[pageID] = transition_count; */\
    } \
    }\ 
    value_read;  \
})


# define SLOW_PATH_SHARED_READ_P(var) ({    \
    void* value_read=0; \
    long pageID; \
    long transition_count=0; \
    int loop_counter_read; \
    long increment_inside_loop=0; \
    pageID = ((long)(&var) - (long)STM_page_first_address)>>16; \
    int first_time_page_read; \
    long pageID_aux; \
	VALIDATE_READSET(); \
    /*if(!(pages_read[pageID])){*/ \
     /*   local_transition_count=global_transition_count;*/ \
      /*  first_time_page_read=1;*/  \
    /*}  */ \
    /*if (pageID<0) pageID=0; */\
    if(metadata_array[pageID].status_field == 3){ \
        while(1){ \
                        while(LOCKED(metadata_array[pageID].lock_bit)){ \
                            __asm__ ("nop;"); \
                        } \
                        if(__sync_val_compare_and_swap(&metadata_array[pageID].lock_bit, 0, 1) == 0) { \
                            break; \
                        } \
                    } \
		if(metadata_array[pageID].status_field == 3){ \
            mprotect((void *)page_first_address+(PAGE_SIZE*pageID),PAGE_SIZE,1); \
            statistics_array[SPECIAL_THREAD_ID()].system_calls++; \
            metadata_array[pageID].status_field = 1;    \
        } \
    	metadata_array[pageID].lock_bit=0; \
    } \
    long msk = FILTERBITS(pageID); \
    if(((pages_read & msk) == msk)){ \
        pages_read |= msk; \
        pages_read_heap[SPECIAL_THREAD_ID()][counter_pages_read]=pageID; \
        counter_pages_read++; \
        transition_count_pages_read[threadID][pageID] = metadata_array[pageID].transition_count; \
    } \
    value_read = stm_load_ptr((volatile void **)(void *)&(var)); \
	/*if(transition_count_pages_read[threadID][pageID_aux] != metadata_array[pageID_aux].transition_count){ \
            SLOW_PATH_RESTART(); \
    } */\
    if(local_transition_count!=global_transition_count){ \
        for(loop_counter_read=0;loop_counter_read<counter_pages_read;loop_counter_read++){ \
            pageID_aux=pages_read_heap[SPECIAL_THREAD_ID()][loop_counter_read]; \
            long msk = FILTERBITS(pageID_aux); \
            if(((pages_read & msk) == msk) && (transition_count_pages_read[threadID][pageID_aux] != metadata_array[pageID_aux].transition_count)){ \
                    SLOW_PATH_RESTART(); \
            } \
        /*if(pageID_aux==pageID &&  (transition_count_pages_read[pageID_aux] != metadata_array[pageID_aux].transition_count)) \
			transition_count=metadata_array[pageID_aux].transition_count; \
		} \
        local_transition_count=global_transition_count; \
        transition_count_pages_read[pageID] = transition_count; */\
    } \
    } \
    value_read;  \
})




//# define SLOW_PATH_SHARED_READ_F(var)         stm_load_double((volatile double *)(void *)&(var))

//# define SLOW_PATH_SHARED_READ_D(var)         stm_load_float((volatile float *)(void *)&(var))
//# define SLOW_PATH_SHARED_WRITE(var, val)     stm_write(&var,metadata_array,page_first_address,HTM_Heap,offset_heaps, pages_written,STM_page_first_address, val)

# define SLOW_PATH_SHARED_WRITE(var, val)   ({    \
    long pageID = ((long int) &(var) - (long int)STM_page_first_address)>>16; \
    long increment_inside_loop=0; \
            long msk = FILTERBITS(pageID); \
                    if(!((pages_written & msk) == msk)){ \
                        pages_written |= FILTERBITS(pageID); \
                        writer_count[threadID][pageID]=1; \
                        pages_written_heap[SPECIAL_THREAD_ID()][counter_pages_written]=pageID; \
                        counter_pages_written++; \
                    }   \
            if((metadata_array[pageID].status_field == 3) || metadata_array[pageID].status_field == 1){ \
               while(1){ \
                        while(LOCKED(metadata_array[pageID].lock_bit)){ \
                            __asm__ ("nop;"); \
                        } \
                        if(__sync_val_compare_and_swap(&metadata_array[pageID].lock_bit, 0, 1) == 0) { \
                            break; \
                        } \
                    } \
					if((metadata_array[pageID].status_field == 3) || metadata_array[pageID].status_field == 1){ \
                        mprotect((void *)page_first_address+(PAGE_SIZE*pageID),PAGE_SIZE,0); \
                        statistics_array[SPECIAL_THREAD_ID()].system_calls++; \
                        metadata_array[pageID].status_field = 0; \
                    }  \
                metadata_array[pageID].lock_bit = 0; \
            }   \
    stm_store((volatile stm_word_t *)(void *)&(var), (stm_word_t)(val)); \
})

# define SLOW_PATH_SHARED_WRITE_P(var, val)   ({    \
    long pageID = ((long int) &(var) - (long int)STM_page_first_address)>>16; \
    long increment_inside_loop=0; \
            long msk = FILTERBITS(pageID); \
                    if(!((pages_written & msk) == msk)){ \
                        pages_written |= FILTERBITS(pageID); \
                        writer_count[threadID][pageID]=1; \
                        pages_written_heap[SPECIAL_THREAD_ID()][counter_pages_written]=pageID; \
                        counter_pages_written++; \
                    }   \
            if((metadata_array[pageID].status_field == 3) || metadata_array[pageID].status_field == 1){ \
               while(1){ \
                        while(LOCKED(metadata_array[pageID].lock_bit)){ \
                            __asm__ ("nop;"); \
                        } \
                        if(__sync_val_compare_and_swap(&metadata_array[pageID].lock_bit, 0, 1) == 0) { \
                            break; \
                        } \
                    } \
					if((metadata_array[pageID].status_field == 3) || metadata_array[pageID].status_field == 1){ \
                        mprotect((void *)page_first_address+(PAGE_SIZE*pageID),PAGE_SIZE,0); \
                        statistics_array[SPECIAL_THREAD_ID()].system_calls++; \
                        metadata_array[pageID].status_field = 0; \
                    }  \
                metadata_array[pageID].lock_bit = 0; \
            }   \
     stm_store_ptr((volatile void **)(void *)&(var), val); \
})


//# define SLOW_PATH_SHARED_WRITE_D(var, val)   stm_store_double((volatile double *)(void *)&(var), val)

# define SLOW_PATH_SHARED_WRITE_D(var, val)   ({    \
    long pageID = ((long int) &(var) - (long int)STM_page_first_address)>>16; \
    long increment_inside_loop=0; \
        long msk = FILTERBITS(pageID); \
                    if(!((pages_written & msk) == msk)){ \
                        pages_written |= FILTERBITS(pageID); \
                        writer_count[threadID][pageID]=1; \
                        pages_written_heap[SPECIAL_THREAD_ID()][counter_pages_written]=pageID; \
                        counter_pages_written++; \
                    }   \
            if((metadata_array[pageID].status_field == 3) || metadata_array[pageID].status_field == 1){ \
               while(1){ \
                        while(LOCKED(metadata_array[pageID].lock_bit)){ \
                            __asm__ ("nop;"); \
                        } \
                        if(__sync_val_compare_and_swap(&metadata_array[pageID].lock_bit, 0, 1) == 0) { \
                            break; \
                        } \
                    } \
                    if((metadata_array[pageID].status_field == 3) || metadata_array[pageID].status_field == 1){ \
                        mprotect((void *)page_first_address+(PAGE_SIZE*pageID),PAGE_SIZE,0); \
                        statistics_array[SPECIAL_THREAD_ID()].system_calls++; \
                        metadata_array[pageID].status_field = 0; \
                    }  \
                metadata_array[pageID].lock_bit = 0; \
            }   \
     stm_store_double((volatile double *)(void *)&(var), val); \
})

# define SLOW_PATH_SHARED_WRITE_F(var, val)   ({    \
    long pageID = ((long int) &(var) - (long int)STM_page_first_address)>>16; \
    long increment_inside_loop=0; \
    long msk = FILTERBITS(pageID); \
                    if(!((pages_written & msk) == msk)){ \
                        pages_written |= FILTERBITS(pageID); \
                        writer_count[threadID][pageID]=1; \
                        pages_written_heap[SPECIAL_THREAD_ID()][counter_pages_written]=pageID; \
                        counter_pages_written++; \
                    }   \
            if((metadata_array[pageID].status_field == 3) || metadata_array[pageID].status_field == 1){ \
               while(1){ \
                        while(LOCKED(metadata_array[pageID].lock_bit)){ \
                            __asm__ ("nop;"); \
                        } \
                        if(__sync_val_compare_and_swap(&metadata_array[pageID].lock_bit, 0, 1) == 0) { \
                            break; \
                        } \
                    } \
                    if((metadata_array[pageID].status_field == 3) || metadata_array[pageID].status_field == 1){ \
                        mprotect((void *)page_first_address+(PAGE_SIZE*pageID),PAGE_SIZE,0); \
                        statistics_array[SPECIAL_THREAD_ID()].system_calls++; \
                        metadata_array[pageID].status_field = 0; \
                    }  \
                metadata_array[pageID].lock_bit = 0; \
            }   \
    stm_store_float((volatile float *)(void *)&(var), val); \
})

#  define TM_LOCAL_WRITE(var, val)      ({var = val; var;})
#  define TM_LOCAL_WRITE_P(var, val)    ({var = val; var;})
#  define TM_LOCAL_WRITE_D(var, val)    ({var = val; var;})

/*
#  define FAST_PATH_SHARED_READ(var) *(long*)(((long)(&(var)))+offset_heaps)
#  define FAST_PATH_SHARED_READ_F(var) (*(float*)(((long)(&(var)))+offset_heaps))
#  define FAST_PATH_SHARED_READ_D(var) *(double*)(((long)(&(var)))+offset_heaps)*/
//#  define FAST_PATH_SHARED_READ_P(var) var//*((long*)(((long)(&(var)))-offset_heaps))*/
# define BAILOUT_SHARED_READ(var)   FAST_PATH_SHARED_READ(var)
# define BAILOUT_SHARED_WRITE(var,val)   FAST_PATH_SHARED_WRITE(var,val)
/*
#  define FAST_PATH_SHARED_WRITE(var, val)  ({ *((long *)(((long)(&(var)))+offset_heaps)) = val;})
#  define FAST_PATH_SHARED_WRITE_F(var, val)  ({ *((float *)(((long)(&(var)))+offset_heaps)) = val;})
#  define FAST_PATH_SHARED_WRITE_D(var, val)  ({ *((double *)(((long)(&(var)))+offset_heaps)) = val;})
//#  define FAST_PATH_SHARED_WRITE_P(var, val)   ({ *((long*)(((long)(&(var)))+offset_heaps)) = val;})
#  define FAST_PATH_SHARED_WRITE_P(var, val) ({var = val; var;})
*/
//#  define SLOW_PATH_SHARED_READ_P(var)   stm_load_ptr((volatile void **)(void *)&(var))
# define HTM_SHARED_READ(var)       (var)
# define HTM_SHARED_READ_P(var)      (var)
# define HTM_SHARED_WRITE(var, val)     ({var = val; var;})

#endif /* TM_H */
