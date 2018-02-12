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
#include <htmxlintrin.h>
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
unsigned int nb_threads=0;
static int num_operations=0;
int current_bucket,previous_bucket=0;
unsigned int seed;
static int not_found=0;
static int found=0;



int main(int argc,char *argv[]){

    //shared data between HTM and STM
    struct sigaction sa; 
    int i=0;
    nb_threads = atoi(argv[1]); 
    p_both_threshold = atoi(argv[2]);
    p_bias_threshold = atoi(argv[3]);
    p_op_threshold = atoi(argv[4]);
    num_operations = atoi(argv[5]);
    int* pointer_to_physical;
    int fd=-1;
    int previous_value,value=0;
    
    fd = shm_open("shared_array", O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
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
    for (i=0; i<SHORT_STRUCTURE_LIMIT;i++){
        while(1){        
                value=previous_value+(rand()%SMALL_DATASET_POPULATION);       
                current_bucket=(i/NUM_ELEMENTS_PER_BUCKET_SMALL);
                if(current_bucket!=previous_bucket){
                        previous_bucket=current_bucket;
                        previous_value=0;
                }
                if(((value%NUM_BUCKETS_SMALL_DATASET)==current_bucket) && (value>=previous_value)){
                    HTM_Heap[i]= value;
                    STM_Heap[i]=value;
                    previous_value=value;
                    printf("Position: %d BUCKET %d value %d\n",i,(i/NUM_ELEMENTS_PER_BUCKET_SMALL),HTM_Heap[i]);
                    break;
                }
            }
    }
    previous_value=0;
    current_bucket=0;
    previous_bucket=0;
    printf("out of the while loop\n");
    printf("i=%d\n",i);
    for(i=SHORT_STRUCTURE_LIMIT;i<NUM_ELEMENTS_ARRAY;i++){
            printf("here\n");
            printf("i=%d\n",i);
            while(1){    
                value=previous_value+(rand()%LARGE_DATASET_POPULATION);    
                current_bucket=(i/NUM_ELEMENTS_PER_BUCKET_LARGE);
                if(current_bucket!=previous_bucket){
                        previous_bucket=current_bucket;
                        previous_value=0;
                } 
                if(((value%NUM_BUCKETS_LARGE_DATASET)==current_bucket-NUM_BUCKETS_CONVERTED) && (value>=previous_value)){
                    HTM_Heap[i]= value;
                    STM_Heap[i]=value;
                    previous_value=value;
                    printf("Position: %d BUCKET %d value %d\n",i,(i/NUM_ELEMENTS_PER_BUCKET_LARGE)-NUM_BUCKETS_CONVERTED,HTM_Heap[i]);    
                    break;
                }
            }         

    }

        TM_STARTUP(nb_threads,42);
        thread_startup(nb_threads);

    float time=0;
    float throughput=0;
    struct timeval start, end;

    //for(i=0;i<NUM_RUNS;i++){
        //startEnergyIntel();
        gettimeofday(&start, NULL);
        thread_start(test,NULL);
        gettimeofday(&end, NULL);
   //    for (i=0; i<SHORT_STRUCTURE_LIMIT;i++){
//                    printf("Position: %d BUCKET %d value %d\n",i,(i/NUM_ELEMENTS_PER_BUCKET_SMALL),HTM_Heap[i]);
   // }   
    previous_value=0;
    current_bucket=0;
    previous_bucket=0;
    //    for(i=SHORT_STRUCTURE_LIMIT;i<NUM_ELEMENTS_ARRAY;i++){
  //           printf("Position: %d BUCKET %d value %d\n",i,(i/NUM_ELEMENTS_PER_BUCKET_LARGE)-NUM_BUCKETS_CONVERTED,STM_Heap[i]);    
  //      }  
        //printf("\n\nI came out of test\n\n");
        time=((end.tv_sec * 1000000 + end.tv_usec) - (start.tv_sec * 1000000 + start.tv_usec))/1000000.0;
        printf("time elapsed: %f\n",time); 
        
        throughput += (num_operations * nb_threads)/time;
    //}

    //printf ("%f\n",time); 
    printf ("Throughput: %f\n",throughput);    

    //printf ("SYSTEM CALLS ISSUED %d\n",sum_system_call);
    //printf ("transition count incremented %d times\n",sum_transition_count);
//    printf("Throughput: %f  \nNumber of System Calls: %f  \nNumber of transition counts: %f  ",throughput,sum_system_call, sum_transition_count);
    
    //Unmap memory chunk
    munmap (HTM_Heap,sizeof(long) * NUM_ELEMENTS_ARRAY);
	munmap (STM_Heap,sizeof(long) * NUM_ELEMENTS_ARRAY); 

    //Close shared object
    shm_unlink("/shared_array");

    TM_SHUTDOWN();  
    thread_shutdown();
    
    return throughput;
    //return 0;
}


int get_short_structure(TM_ARGDECL int value,int num_buckets_small_dataset,int num_elements_per_bucket_small){
    int i=0;
    int belonging_bucket=value%num_buckets_small_dataset;
    //printf("Belonging bucket: %d",belonging_bucket);
    int result=0;    
        for(i=belonging_bucket*num_elements_per_bucket_small;i<belonging_bucket*num_elements_per_bucket_small+num_elements_per_bucket_small;i++){
            result = FAST_PATH_SHARED_READ(HTM_Heap[i]);
            if(result>=value)
                break;
    }
    statistics_array[SPECIAL_THREAD_ID()].lookup_HTM++;
    return (result==value);
}


int get_short_structure_STM(TM_ARGDECL long value,int num_buckets_small_dataset,int num_elements_per_bucket_small){
    int i=0;
    int belonging_bucket=value%num_buckets_small_dataset;
    int result=0;    
       for(i=belonging_bucket*num_elements_per_bucket_small;i<belonging_bucket*num_elements_per_bucket_small+num_elements_per_bucket_small;i++){
            result=SLOW_PATH_SHARED_READ(STM_Heap[i]); 
            if(result>=value)
                break;
    }   
    statistics_array[SPECIAL_THREAD_ID()].lookup_STM_small++;
    return (result==value);
}


int get_large_structure(TM_ARGDECL long value,int num_buckets_converted,int num_elements_per_bucket_large,int num_buckets_large_dataset){
    int i=0;
    int belonging_bucket=value%num_buckets_large_dataset;
    int result=0;  
    //printf("BELONGING BUCKET %d value %d\n",belonging_bucket,result);

      for(i=(num_buckets_converted+belonging_bucket)*num_elements_per_bucket_large;i<(num_buckets_converted+belonging_bucket)*num_elements_per_bucket_large+num_elements_per_bucket_large;i++){
            result=SLOW_PATH_SHARED_READ(STM_Heap[i]);
            //printf("----%d----\n",i);
   //         result=HTM_Heap[i];
            if(result>=value)
                break;
    }   
    statistics_array[SPECIAL_THREAD_ID()].lookup_STM++;
    return (result==value);
}    

int insert_value_short_structure(TM_ARGDECL long value_to_find, unsigned int mySeed,int small_dataset_population,int num_buckets_small_dataset,int num_elements_per_bucket_small){
    int i=0;
    int belonging_bucket=value_to_find%num_buckets_small_dataset;
    int diff=0;
    int result=0;
    int value_to_add=0;

        for(i=belonging_bucket*num_elements_per_bucket_small;i<belonging_bucket*num_elements_per_bucket_small+num_elements_per_bucket_small;i++){
            result = FAST_PATH_SHARED_READ(HTM_Heap[i]);
            if(result>=value_to_find){
                if(i==(belonging_bucket*num_elements_per_bucket_small+num_elements_per_bucket_small)-1){
                    diff=small_dataset_population-result;
                }else{
                    diff=FAST_PATH_SHARED_READ(HTM_Heap[i+1])-result;
                }
                while(1){
                        value_to_add = rand_r(&mySeed)%diff;
                        if(((result+value_to_add)%num_buckets_small_dataset)==belonging_bucket)
                                break;
                }
                FAST_PATH_SHARED_WRITE(HTM_Heap[i],result+value_to_add);
                statistics_array[SPECIAL_THREAD_ID()].insert_HTM++;
                return result+value_to_add;
             }
        } 
}

int insert_value_short_structure_STM(TM_ARGDECL int value_to_find,unsigned int mySeed,int small_dataset_population,int num_buckets_small_dataset,int num_elements_per_bucket_small){
    int i=0;
    int belonging_bucket=value_to_find%num_buckets_small_dataset;
    int result=0;
    int diff=0;
    int value_to_add=0;

    for(i=belonging_bucket*num_elements_per_bucket_small;i<belonging_bucket*num_elements_per_bucket_small+num_elements_per_bucket_small;i++){
            result = SLOW_PATH_SHARED_READ(STM_Heap[i]);
            if(result>=value_to_find){
                if(i==(belonging_bucket*num_elements_per_bucket_small+num_elements_per_bucket_small)-1){
                    diff=small_dataset_population-result;
                }else{
                    diff=SLOW_PATH_SHARED_READ(STM_Heap[i+1])-result;
                }
                while(1){
                    value_to_add=(rand_r(&mySeed)%diff);
                    if(((result+value_to_add)%num_buckets_small_dataset)==belonging_bucket)
                            break;
                }
                SLOW_PATH_SHARED_WRITE(STM_Heap[i],result+value_to_add);
                statistics_array[SPECIAL_THREAD_ID()].insert_STM_small++;
                return result+value_to_add;
             }
        }
}


int insert_value_large_structure(TM_ARGDECL int value_to_find,unsigned int mySeed,int num_buckets_converted,int num_elements_per_bucket_large,int large_dataset_population,int num_buckets_large_dataset){
    int i=0;
    int belonging_bucket=value_to_find%num_buckets_large_dataset;
    int result=0;
    int diff=0;
    int value_to_add=0;

        for(i=(num_buckets_converted+belonging_bucket)*num_elements_per_bucket_large;i<(num_buckets_converted+belonging_bucket)*num_elements_per_bucket_large+num_elements_per_bucket_large;i++){
            result = SLOW_PATH_SHARED_READ(STM_Heap[i]);
            if(result>=value_to_find){
                if(i==((num_buckets_converted+belonging_bucket)*num_elements_per_bucket_large+num_elements_per_bucket_large)-1){
                    diff=large_dataset_population-result;
                }else{
                    diff=SLOW_PATH_SHARED_READ(STM_Heap[i+1])-result;
                    while(1){
                        value_to_add=(rand_r(&mySeed)%diff);
                        if(((result+value_to_add)%num_buckets_large_dataset)==belonging_bucket)     
                            break;
                    }
                    SLOW_PATH_SHARED_WRITE(STM_Heap[i],result+value_to_add);
                    statistics_array[SPECIAL_THREAD_ID()].insert_STM++;
                    return result+value_to_add;
             }
        }
    }
}



void* test(void* data){
    long numThread = thread_getId();
    volatile float j=0;
    int mode;
    int use_large_datastructure=0;
    unsigned value_to_find,value_to_insert,index=0;
    unsigned long value=0;
    long p_both=0;
    long p_op=0;
    long p_bias=0;
    long page,index_inside=0;
    unsigned int mySeed = seed + sched_getcpu();
    int var=0;
    int belonging_bucket=0;   
    int myOps=(float)num_operations/nb_threads;
    //printf("operation threshold is %d\n", p_op_threshold);
    int p_both_threshold_local=p_both_threshold;
    int p_bias_threshold_local=p_bias_threshold;
    int p_op_threshold_local=p_op_threshold;
    int small_dataset_population=SMALL_DATASET_POPULATION;
    int large_dataset_population=LARGE_DATASET_POPULATION;
    int large_structure_limit=LARGE_STRUCTURE_LIMIT;
    int large_structure_lower_limit=LARGE_STRUCTURE_LOWER_LIMIT;
    int short_structure_limit=SHORT_STRUCTURE_LIMIT;
    int num_buckets_small_dataset=NUM_BUCKETS_SMALL_DATASET;
    int num_buckets_converted=NUM_BUCKETS_CONVERTED;
    int num_elements_per_bucket_large=NUM_ELEMENTS_PER_BUCKET_LARGE;
    int num_elements_per_bucket_small=NUM_ELEMENTS_PER_BUCKET_SMALL;
    int num_buckets_large_dataset=NUM_BUCKETS_LARGE_DATASET;

    TM_THREAD_ENTER();
    
    if(numThread==(nb_threads-1))
          myOps+=((int)myOps%nb_threads);
    
    printf("starting to execute the benchmark\n");
     //if p_both is smaller than the p_both_threshold than it will be used both datastructures, otherwise it will be used just one datastructure
   while(1){
        p_both = rand_r(&mySeed)% 100;
        if((p_both >0) && (p_both<100))
            break;
    }

    for(j=0;j<myOps;j++){

   //     printf("operation number %f\n",j);

        //if p_bias is smaller than the p_bias_threshold than it will be used STM, otherwise is used HTM
        while(1){
            p_bias = rand_r(&mySeed) % 100;
            p_op = rand_r(&mySeed) %100;
            if((p_bias >0) && (p_bias<100) && (p_op > 0) && (p_op<100))
                break;
        }

      //  printf("p_bias is %d p_op is %d and p_both is %d\n",p_bias,p_op,p_both);

        if(p_both > p_both_threshold_local){
                if(p_bias > p_bias_threshold_local){
            //            printf("using: HTM\n");
                        mode=0;
                }else{
              //          printf("using: STM\n");
                        use_large_datastructure = 1;
                        mode =1;
                }
        }else{
            use_large_datastructure = !use_large_datastructure;
            mode = 1;
        }
    
        //A lookup operation
        if(p_op > p_op_threshold_local){
            if(mode){
         //       printf("lookup on the bigger hashmap\n");
                value = rand_r(&mySeed)%large_dataset_population;
                //printf("value is %d\n",value);
            }else{
           //     printf("lookup on the smaller hashmap\n");
                value= rand_r(&mySeed)%small_dataset_population;
                //printf("value is %d\n",value);
            }
     //        update_transaction = 0;
//             printf("-----Going to find value %d\n",value);   
             //printf("mode is %d\n",mode);
             TM_BEGIN_EXT(1,mode,1);

            //var = get(TM_ARG value, mode,use_large_datastructure); 
              var = mode == 0 ? get_short_structure(TM_ARG value,num_buckets_small_dataset,num_elements_per_bucket_small) : (use_large_datastructure==1 ? get_large_structure(TM_ARG value,num_buckets_converted,num_elements_per_bucket_large,num_buckets_large_dataset) : get_short_structure_STM(TM_ARG value,num_buckets_small_dataset,num_elements_per_bucket_small));

/*                if(mode==0){
                    var = lookup_short_structure(value);
                }
                if(mode==1 && (use_large_datastructure==1)){
                    var = lookup_large_structure(value);
                //    printf("lookup on large structure ended\n");
                }
                if(mode==1 && (use_large_datastructure==0)){
                    var = lookup_short_structure_STM(value);
              //      printf("lookup on short structure ended\n");
                } */
           TM_END();
            if(var==-1){
                printf("-----VALUE: %d NOT FOUND------\n",value);
            }

        }else{
           if(mode){
                value_to_find = rand_r(&mySeed)%large_dataset_population;
     //           belonging_bucket = value_to_find % NUM_BUCKETS_LARGE_DATASET;
             //   printf("WE ARE GOING TO DO AN INSERT IN THE BIG DATASET\n");
              //  printf("value to find is %d\n",value_to_find);
           }else{
                value_to_find= rand_r(&mySeed)%small_dataset_population;
       //         belonging_bucket = value_to_find%NUM_BUCKETS_SMALL_DATASET;
             //   printf("WE ARE GOING TO DO AN INSERT IN THE SMALL DATASET\n");
             //   printf("value to find is %d\n",value_to_find);
           }  
            //value=10;
  //          printf("-----Going to find value %d\n",value_to_find);
          //  printf("insert and mode is %d and index %d\n",mode,index);
            //update_transaction = 1;
            TM_BEGIN_EXT(1,mode,0);

            var = mode == 0 ? insert_value_short_structure(TM_ARG value_to_find,mySeed,small_dataset_population,num_buckets_small_dataset,num_elements_per_bucket_small): (use_large_datastructure==1 ? insert_value_large_structure(TM_ARG value_to_find,mySeed,num_buckets_converted,num_elements_per_bucket_large,large_dataset_population,num_buckets_large_dataset) : insert_value_short_structure_STM(TM_ARG value_to_find,mySeed,small_dataset_population,num_buckets_small_dataset,num_elements_per_bucket_small));           


           TM_END();
        
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

