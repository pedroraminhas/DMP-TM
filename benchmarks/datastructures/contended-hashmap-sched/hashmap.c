#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <malloc.h>
#include <stdlib.h>
#include <errno.h>
#include <math.h>
#include <assert.h>
//#include <htmintrin.h>
//#include <htmxlintrin.h>
#include "hashmap.h"
#include <pthread.h>
#include <sys/time.h>
#include "thread.h"


static int lock_initialising_accounts=1;
static int handler_lock=0;
static int sum_money=0;
int system_call_counter[16];
int transition_count_counter[16];

float sum_transition_count=0;
float sum_system_call=0;

static float num_printf=0;
static long p_both_threshold=0;
static long p_bias_threshold=0;
static long p_op_threshold=0;
static long p_sched_threshold=100;
unsigned int nb_threads=0;
unsigned long num_operations=0;
int current_bucket,previous_bucket=0;
unsigned int seed;
static int not_found=0;
static int found=0;



int main(int argc,char *argv[]){

    //shared data between HTM and STM
    int i=0;
    nb_threads = atoi(argv[1]);
    p_both_threshold = atoi(argv[2]);
    p_bias_threshold = atoi(argv[3]);
    p_op_threshold = atoi(argv[4]);
    num_operations = atol(argv[5]);
    p_sched_threshold = atol(argv[6]);
    int* pointer_to_physical;
    int fd=-1;
    int previous_value,value=0;

	MB = 1048576;
    PAGE_SIZE = getpagesize();
    NUM_PAGES = 64;
    NUM_ELEMENTS_PAGE = PAGE_SIZE/sizeof(long);
    NUM_ELEMENTS_ARRAY =  NUM_PAGES*NUM_ELEMENTS_PAGE;

    fd = shm_open("shared_array_pedro", O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
    if (fd == -1)
        printf("FAILURE CREATING SHARED MEMORY\n");

    if (ftruncate(fd, sizeof(long) * NUM_ELEMENTS_ARRAY) == -1)
        printf("FAILURE TRUNCATE\n");

	//Map chunk of 1MB memory to private address space
	HTM_Heap = mmap (NULL, sizeof(long) * NUM_ELEMENTS_ARRAY, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    STM_Heap = mmap (NULL, sizeof(long) * NUM_ELEMENTS_ARRAY, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    //Deal with failure on the mapping
    if(HTM_Heap == MAP_FAILED)
        printf("MAP_FAILED\n");

    if(STM_Heap == MAP_FAILED)
         printf("MAP_FAILED\n");

 //   printf("\nNumber of elements per bucket: %d\nNumber of buckets of the small set: %d\nNumber of elements in a page: %d\n",NUM_ELEMENTS_PER_BUCKET,SMALL_SET_NUM_BUCKETS,NUM_ELEMENTS_PAGE);
    seed = time(NULL);
    srand(seed);

   // for (i=0; i<NUM_ELEMENTS_ARRAY;i++){
     //   if(i<SHORT_STRUCTURE_LIMIT){
    for (i=0; i<LARGE_STRUCTURE_LIMIT;i++){
                HTM_Heap[i] = i;
                STM_Heap[i] = i;
    }

        TM_STARTUP(nb_threads,2000);
        thread_startup(nb_threads);

    float time=0;
    float throughput=0;
    struct timeval start, end;
        //mode2[0].counter=0;
        //mode2[2].counter=0;
        gettimeofday(&start, NULL);
        thread_start(test,NULL);
        gettimeofday(&end, NULL);

        time=((end.tv_sec * 1000000 + end.tv_usec) - (start.tv_sec * 1000000 + start.tv_usec))/1000000.0;
        throughput += num_operations/time;
        printf("Time Elapsed: %f\n",time);
        printf ("Throughput: %f\n",throughput);

    //printf ("SYSTEM CALLS ISSUED %d\n",sum_system_call);
    //printf ("transition count incremented %d times\n",sum_transition_count);
//    printf("Throughput: %f  \nNumber of System Calls: %f  \nNumber of transition counts: %f  ",throughput,sum_system_call, sum_transition_count);

    //Unmap memory chunk
    munmap (HTM_Heap,sizeof(long) * NUM_ELEMENTS_ARRAY);
	munmap (STM_Heap,sizeof(long) * NUM_ELEMENTS_ARRAY);

    //Close shared object
    shm_unlink("shared_array_pedro");

    TM_SHUTDOWN();
    thread_shutdown();

    return throughput;
    //return 0;
}

long get_short_structure(TM_ARGDECL long value){
    long i=0;
    long result=0;

    //printf("-----GET-------\n");

    //TM_BEGIN_EXT(0,0,1);
    FAST_PATH_BEGIN(1,1);

    if(local_exec_mode==1){
        for(i=value;i<value+NUM_ELEMENTS_PER_BUCKET_SMALL;i++){
			      result=SLOW_PATH_SHARED_READ(STM_Heap[i]);
            if(result==i*2)
                break;
        }
    } else if(local_exec_mode == 2){
        for(i=value;i<value+NUM_ELEMENTS_PER_BUCKET_SMALL;i++){
            #if USE_HTM_HEAP
                result=MIXED_PATH_SHARED_READ(HTM_Heap[i]);
            #else
                result=MIXED_PATH_SHARED_READ(STM_Heap[i]);
            #endif
            if(result==i*2)
                break;
        }
    } else if (local_exec_mode == 3){
        for(i=value;i<value+NUM_ELEMENTS_PER_BUCKET_SMALL;i++){
            result=BAILOUT_SHARED_READ(STM_Heap[i]);
            if(result==i*2)
                break;
        } 
	}else{ 
         for(i=value;i<value+NUM_ELEMENTS_PER_BUCKET_SMALL;i++){
            result=FAST_PATH_SHARED_READ(HTM_Heap[i]);
            if(result==i*2)
                break;
        }
    }

    //TM_END();
  /*  if(local_exec_mode == 1)
        FAST_PATH_END();
    if(local_exec_mode == 2)
        MIXED_END();
    if(local_exec_mode == 0)
        SLOW_PATH_END();
*/
    FAST_PATH_END();

    statistics_array[SPECIAL_THREAD_ID()].lookup_HTM++;
    return (result==value);
}

long get_large_structure(TM_ARGDECL long value){
    long i=0;
    long result=0;

    //printf("-----GET-------\n");

    //TM_BEGIN_EXT(0,0,1);
    SLOW_PATH_BEGIN(0,1);

    if(local_exec_mode == 1){
        for(i=value;i<value+NUM_ELEMENTS_PER_BUCKET_LARGE;i++){
			      result=SLOW_PATH_SHARED_READ(STM_Heap[i]);
            if(result==i*2)
                break;
        }
    } else if(local_exec_mode == 2){
        for(i=value;i<value+NUM_ELEMENTS_PER_BUCKET_LARGE;i++){
             #if USE_HTM_HEAP
                result=MIXED_PATH_SHARED_READ(HTM_Heap[i]);
             #else
                result=MIXED_PATH_SHARED_READ(STM_Heap[i]);
            #endif
            if(result==i*2)
                break;
        }
	} else if (local_exec_mode == 3){
    	    for(i=value;i<value+NUM_ELEMENTS_PER_BUCKET_LARGE;i++){
            result=BAILOUT_SHARED_READ(STM_Heap[i]);
            if(result==i*2)
                break;
        }
	} else{
         for(i=value;i<value+NUM_ELEMENTS_PER_BUCKET_LARGE;i++){
            result=FAST_PATH_SHARED_READ(HTM_Heap[i]);
            if(result==i*2)
                break;
        }
    }

    //TM_END();
  /*  if(local_exec_mode == 1)
        FAST_PATH_END();
    if(local_exec_mode == 2)
        MIXED_END();
    if(local_exec_mode == 0)
        SLOW_PATH_END();
*/
    SLOW_PATH_END();

    statistics_array[SPECIAL_THREAD_ID()].lookup_STM++;
    return (result==value);
}


long insert_value_short_structure(TM_ARGDECL long value_to_find){
    long i=0;
    long result=0;
    long value_to_insert;

    //printf("-------INSERT--------:%ld\n",value_to_find);

    //TM_BEGIN_EXT(1,1,0);
	FAST_PATH_BEGIN(1,0);

    if(local_exec_mode == 1){
        for(i=value_to_find;i<value_to_find+NUM_ELEMENTS_PER_BUCKET_SMALL;i++){
			       result=SLOW_PATH_SHARED_READ(STM_Heap[i]);
             if(result==i*2){
                value_to_insert = i/2;
                SLOW_PATH_SHARED_WRITE(STM_Heap[i],value_to_insert);
              }
        }
        value_to_insert = i*2;
        SLOW_PATH_SHARED_WRITE(STM_Heap[i-1],value_to_insert);
    }else if(local_exec_mode == 2){
        for(i=value_to_find;i<value_to_find+NUM_ELEMENTS_PER_BUCKET_SMALL;i++){
                #if USE_HTM_HEAP
			        result=MIXED_PATH_SHARED_READ(HTM_Heap[i]);
                #else
                    result=MIXED_PATH_SHARED_READ(STM_Heap[i]);
                #endif
             if(result==i*2){
                value_to_insert = i/2;
                #if USE_HTM_HEAP
                    MIXED_PATH_SHARED_WRITE(HTM_Heap[i],value_to_insert);
                #else
                    MIXED_PATH_SHARED_WRITE(STM_Heap[i],value_to_insert);
                #endif
              }
        }
        value_to_insert = i*2;
        #if USE_HTM_HEAP
            MIXED_PATH_SHARED_WRITE(HTM_Heap[i-1],value_to_insert);
        #else
            MIXED_PATH_SHARED_WRITE(STM_Heap[i-1],value_to_insert);
        #endif
	}else if(local_exec_mode == 3){
		for(i=value_to_find;i<value_to_find+NUM_ELEMENTS_PER_BUCKET_SMALL;i++){
            result=BAILOUT_SHARED_READ(STM_Heap[i]);
            if(result==i*2){
                value_to_insert = i/2;
                BAILOUT_SHARED_WRITE(STM_Heap[i],value_to_insert);
            }
        }
        value_to_insert = i*2;
        BAILOUT_SHARED_WRITE(STM_Heap[i-1],value_to_insert);	
    } else{ 
        for(i=value_to_find;i<value_to_find+NUM_ELEMENTS_PER_BUCKET_SMALL;i++){
		    result=FAST_PATH_SHARED_READ(HTM_Heap[i]);
            if(result==i*2){
                value_to_insert = i/2;
                FAST_PATH_SHARED_WRITE(HTM_Heap[i],value_to_insert);
            }
        }
        value_to_insert = i*2;
        FAST_PATH_SHARED_WRITE(HTM_Heap[i-1],value_to_insert);
    }

    //TM_END();
	/*if(local_exec_mode == 1)
        FAST_PATH_END();
    if(local_exec_mode == 2)
        MIXED_END();
    if(local_exec_mode == 0)
        SLOW_PATH_END();
*/
    FAST_PATH_END();

    statistics_array[SPECIAL_THREAD_ID()].insert_HTM++;
    return value_to_insert;
}


long insert_value_large_structure(TM_ARGDECL long value_to_find){
    long i=0;
    long result=0;
    long value_to_insert=0;
    //printf("-------INSERT--------\n");

    //TM_BEGIN_EXT(1,1,0);
	SLOW_PATH_BEGIN(0,0);

    if(local_exec_mode == 1){
        for(i=value_to_find;i<value_to_find+NUM_ELEMENTS_PER_BUCKET_LARGE;i++){
			       result=SLOW_PATH_SHARED_READ(STM_Heap[i]);
             if(result==i*2){
                value_to_insert = i/2;
                SLOW_PATH_SHARED_WRITE(STM_Heap[i],value_to_insert);
              }
        }
        value_to_insert = i*2;
        SLOW_PATH_SHARED_WRITE(STM_Heap[i-1],value_to_insert);
    }else if(local_exec_mode == 2){
        for(i=value_to_find;i<value_to_find+NUM_ELEMENTS_PER_BUCKET_LARGE;i++){
                 #if USE_HTM_HEAP
			       result=MIXED_PATH_SHARED_READ(HTM_Heap[i]);
                #else
                   result=MIXED_PATH_SHARED_READ(STM_Heap[i]);
                #endif
             if(result==i*2){
                value_to_insert = i/2;
                #if USE_HTM_HEAP
                    MIXED_PATH_SHARED_WRITE(HTM_Heap[i],value_to_insert);
                #else
                    MIXED_PATH_SHARED_WRITE(STM_Heap[i],value_to_insert);
                #endif
              }
        }
        value_to_insert = i*2;
        #if USE_HTM_HEAP
            MIXED_PATH_SHARED_WRITE(HTM_Heap[i-1],value_to_insert);
        #else
             MIXED_PATH_SHARED_WRITE(STM_Heap[i-1],value_to_insert);
        #endif
    }else if(local_exec_mode == 3){
		for(i=value_to_find;i<value_to_find+NUM_ELEMENTS_PER_BUCKET_LARGE;i++){
                   result=BAILOUT_SHARED_READ(STM_Heap[i]);
             if(result==i*2){
                value_to_insert = i/2;
                BAILOUT_SHARED_WRITE(STM_Heap[i],value_to_insert);
              }
        }
        value_to_insert = i*2;
        BAILOUT_SHARED_WRITE(STM_Heap[i-1],value_to_insert);
	} else{ 
        for(i=value_to_find;i<value_to_find+NUM_ELEMENTS_PER_BUCKET_LARGE;i++){
			       result=FAST_PATH_SHARED_READ(HTM_Heap[i]);
             if(result==i*2){
                value_to_insert = i/2;
                FAST_PATH_SHARED_WRITE(HTM_Heap[i],value_to_insert);
              }
        }
        value_to_insert = i*2;
        FAST_PATH_SHARED_WRITE(HTM_Heap[i-1],value_to_insert);
    }

    //TM_END();
	/*if(local_exec_mode == 1)
        FAST_PATH_END();
    if(local_exec_mode == 2)
        MIXED_END();
    if(local_exec_mode == 0)
        SLOW_PATH_END();
*/
    SLOW_PATH_END();

    statistics_array[SPECIAL_THREAD_ID()].insert_STM++;
    return value_to_insert;
}

//volatile int schd_mode = 1;
volatile int x = 0;


void* test(void* data){
    long numThread = thread_getId();
    volatile float j=0;
    //int mode;
    int use_large_datastructure=0;
    long long value_to_find,value_to_insert,index=0;
    long long value=0;
    long p_both=0;
    long p_op=0;
    long p_bias=0;
    long page,index_inside=0;
    unsigned int mySeed = seed + sched_getcpu();
    int var=0;
    int belonging_bucket=0;
    unsigned long myOps=num_operations/nb_threads;
    unsigned long longOps = myOps * p_bias_threshold / 100;
    unsigned long shortOps = myOps - longOps;
    //printf("operation threshold is %d\n", p_op_threshold);
    int p_both_threshold_local=p_both_threshold;
    int p_bias_threshold_local=p_bias_threshold;
    int p_op_threshold_local=p_op_threshold;
    long long  small_dataset_population=SMALL_DATASET_POPULATION;
    long long large_dataset_population=LARGE_DATASET_POPULATION;
    int large_structure_limit=LARGE_STRUCTURE_LIMIT;
    int large_structure_lower_limit=LARGE_STRUCTURE_LOWER_LIMIT;
    int short_structure_limit=SHORT_STRUCTURE_LIMIT;
    int num_buckets_small_dataset=NUM_BUCKETS_SMALL_DATASET;
    int num_buckets_converted=NUM_BUCKETS_CONVERTED;
    int num_elements_per_bucket_large=NUM_ELEMENTS_PER_BUCKET_LARGE;
    int num_elements_per_bucket_small=NUM_ELEMENTS_PER_BUCKET_SMALL;
    int num_buckets_large_dataset=NUM_BUCKETS_LARGE_DATASET;
    int pool;

    TM_THREAD_ENTER();

    if(numThread==(nb_threads-1))
          myOps+=((int)myOps%nb_threads);

    int schd_counter = 0;
    int schd_mode = 1;
    int round = 1;
    long schd_threshold = p_sched_threshold;//*shortOps/100;

    while(myOps) { //shortOps || longOps) {
    //printf("starting to execute the benchmark\n");

        //if p_bias is smaller than the p_bias_threshold than it will be used STM, otherwise is used HTM
            p_bias = rand_r(&mySeed) % 100;
            p_op = rand_r(&mySeed) %100;
            --myOps;

        //if(!numThread){
            /*if(shortOps && schd_mode){
                    p_bias = p_bias_threshold_local + 1;
                    --shortOps;
                    if(++schd_counter>schd_threshold){
                            __sync_add_and_fetch (&x,1);
                            while(x<nb_threads*round);
                            ++round;
                            schd_mode = 0;
                            schd_counter = 0;
                    }
            }else if(longOps && !schd_mode){
                    p_bias = p_bias_threshold_local - 1;
                    --longOps;
                    if(++schd_counter>schd_threshold){
                        __sync_add_and_fetch (&x,1);
                        while(x<nb_threads*round);
                        ++round;
                        schd_mode = 1;  
                        schd_counter = 0;
                    }
            }else{
                    __sync_add_and_fetch (&x,1);
                    while(x<nb_threads*round);
                    ++round;
                    if(schd_mode)
                            schd_mode = 0;
                    else
                            schd_mode = 1;
                    schd_threshold = 1000000000000;
            }*/
        /*}else{
			if(schd_mode){
                    p_bias = p_bias_threshold_local + 1;
            }else if(!schd_mode){
                    p_bias = p_bias_threshold_local - 1;
            }
        }*/


      //  printf("p_bias is %d p_op is %d and p_both is %d\n",p_bias,p_op,p_both);

                if(p_bias >= p_bias_threshold_local){
                        //printf("using: HTM\n");
                        pool=0;
						do{
                            value = rand_r(&mySeed)%LARGE_STRUCTURE_LIMIT;
                        }while(value>(LARGE_STRUCTURE_LIMIT-129));
                }else{
                        //printf("using: STM\n");
                        use_large_datastructure = 1;
                        pool=1;
                   		do{
        			        value = rand_r(&mySeed)%LARGE_STRUCTURE_LIMIT;
		                }while(value>(LARGE_STRUCTURE_LIMIT-1025)); 
                }

        //A lookup operation
        if(p_op >= p_op_threshold_local){

            //  update_transaction = 0;
            //  printf("-----Going to find value %d\n",value);
            //  printf("mode is %d\n",mode);


            //var = get(TM_ARG value, mode,use_large_datastructure);
              var = pool == 0 ? get_short_structure(TM_ARG value) : get_large_structure(TM_ARG value);


            if(var==-1){
                printf("-----VALUE: %d NOT FOUND------\n",value);
            }

        }else{
          //update_transaction = 1;

            var = pool == 0 ? insert_value_short_structure(TM_ARG value): insert_value_large_structure(TM_ARG value);



            if(var==-1){
                printf("VALUE INSERTED IS %d\n",var);
            }
        }
    }
//TM_THREAD_EXIT is only executed when there is a chance of execute STM transactions
//    if(p_bias_threshold>0){
        TM_THREAD_EXIT();
  //  }
        return NULL;

}
