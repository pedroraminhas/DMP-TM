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
#include <math.h>             // Needed for log()


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
float timeout_change=0;
unsigned long num_operations[80];
int current_bucket,previous_bucket=0;
unsigned int seed;
static int not_found=0;
static int found=0;
struct timeval start, end;
float timeouts[NUM_PAGES][100000];
float time_threshold;

//----- Function prototypes -------------------------------------------------
double expon(double x);       // Returns an exponential random variable
double rand_val(int seed);    // Jain's RNG

//Generate all the timeouts exponentially
void populate_timeouts(void){
    int page=0;
    int position=0;
    double value_returned;
    float sum=0;
    
    for(page=0;page<NUM_PAGES;page++){
        rand_val(1+page);  //Seed of the exponential random generator
        //printf(" PAGE NUMBER: %d --->",page);
        for(position=0;position<10;position++){
            //populate the timeouts table with the exponential values
            if(position==0){
                timeouts[page][position]= expon(timeout_change/62);
            }else{
                timeouts[page][position]= timeouts[page][position-1] + expon(timeout_change/62);
           }
                //printf("%f ", timeouts[page][position]);
        }
        //sum=timeouts[page][position]/2000;
        //printf("%f ", sum);
        //printf("\n");
	}
}
    

//Generate all the timeouts deterministically
void populate_fixed_timeouts(void){
    int page=0;
    int position=0;
    double value_returned;
     
        
    for(page=0;page<NUM_PAGES;page++){
        for(position=0;position<100000;position++){
            timeouts[page][position]= timeout_change;
        }   
    }   
}


int main(int argc,char *argv[]){

    //shared data between HTM and STM
    long i=0;
    nb_threads = atoi(argv[1]); 
    p_bias_threshold = atoi(argv[2]);
    timeout_change = atof(argv[3]);
    time_threshold = atof(argv[4]);
    p_op_threshold  = atol(argv[5]);
    int* pointer_to_physical;
    int fd=-1;
    int previous_value,value=0;
 
    if(timeout_change == 6)
            timeout_change=6.4;
	if(timeout_change == 3)
            timeout_change=3.2;
	if(timeout_change == 2)
            timeout_change=0.96;
	if(timeout_change == 1)
            timeout_change=0.448;
	if(timeout_change == 0)
            timeout_change=0.1;

    fd = shm_open("shared_array_pedro", O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
    if (fd == -1)
        printf("FAILURE CREATING SHARED MEMORY\n");

    if (ftruncate(fd, sizeof(long) * NUM_ELEMENTS_ARRAY) == -1)
        printf("FAILURE TRUNCATE\n");

	//Map chunk of memory to private address space
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
    
	for (i=0; i<SHORT_STRUCTURE_LIMIT;i++){
                current_bucket=(i/NUM_ELEMENTS_PER_BUCKET_SMALL);
                value = (rand()%SMALL_DATASET_POPULATION)%current_bucket;
                HTM_Heap[i] = value;
                STM_Heap[i] = value;
    }   
    for(i=SHORT_STRUCTURE_LIMIT;i<LARGE_STRUCTURE_LIMIT;i++){
                current_bucket=((i/NUM_ELEMENTS_PER_BUCKET_LARGE)-NUM_BUCKETS_CONVERTED);
                value = (rand()%LARGE_DATASET_POPULATION)%current_bucket;
                HTM_Heap[i] = value;
                STM_Heap[i] = value;
    }

    //Populate timeouts  
    populate_timeouts();      //Exponential Distribution
    //populate_fixed_timeouts(); //Fixed values

    TM_STARTUP(nb_threads,42);
    thread_startup(nb_threads);

   	float time=0;
    float throughput=0;
   	mode2[0].counter=0;
    mode2[2].counter=0; 
        
    gettimeofday(&start, NULL);
    thread_start(test,NULL);
    gettimeofday(&end, NULL);
        
    time=((end.tv_sec * 1000000 + end.tv_usec) - (start.tv_sec * 1000000 + start.tv_usec))/1000000.0;
        
    //calculate the throughput of the system by summing all the operations and dividing by time
	for(i=0;i<80;i++){
		throughput += num_operations[i];
	}

	throughput= throughput/time;
    
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

//===========================================================================
//=  Function to generate exponentially distributed random variables        =
//=    - Input:  Mean value of distribution                                 =
//=    - Output: Returns with exponentially distributed random variable     =
//===========================================================================
double expon(double x)
{
  double z;                     // Uniform random number (0 < z < 1)
  double exp_value;             // Computed exponential value to be returned

  // Pull a uniform random number (0 < z < 1)
  do
  {
    z = rand_val(0);
  }
  while ((z == 0) || (z == 1));

  // Compute exponential random variable using inversion method
  exp_value = -x * log(z);

  return(exp_value);
}

//=========================================================================
//= Multiplicative LCG for generating uniform(0.0, 1.0) random numbers    =
//=   - x_n = 7^5*x_(n-1)mod(2^31 - 1)                                    =
//=   - With x seeded to 1 the 10000th x value should be 1043618065       =
//=   - From R. Jain, "The Art of Computer Systems Performance Analysis," =
//=     John Wiley & Sons, 1991. (Page 443, Figure 26.2)                  =
//=========================================================================
double rand_val(int seed)
{
  const long  a =      16807;  // Multiplier
  const long  m = 2147483647;  // Modulus
  const long  q =     127773;  // m div a
  const long  r =       2836;  // m mod a
  static long x;               // Random int value
  long        x_div_q;         // x divided by q
  long        x_mod_q;         // x modulo q
  long        x_new;           // New x value

  // Set the seed if argument is non-zero and then return zero
  if (seed > 0)
  {
    x = seed;
    return(0.0);
  }

  // RNG using integer arithmetic
  x_div_q = x / q;
  x_mod_q = x % q;
  x_new = (a * x_mod_q) - (r * x_div_q);
  if (x_new > 0)
    x = x_new;
  else
    x = x_new + m;

  // Return a random value between 0.0 and 1.0
  return((double) x / m);
}


int get(TM_ARGDECL int value,int belonging_bucket,int num_elements_per_bucket){
    int i=0;
    long result=0;    

    //printf("GET HTM\n");
    
    FAST_PATH_BEGIN(0,0,1); 
   
    if(!local_exec_mode){
        for(i=belonging_bucket*num_elements_per_bucket;i<belonging_bucket*num_elements_per_bucket+num_elements_per_bucket;i++){
            result = SLOW_PATH_SHARED_READ(STM_Heap[i]);
            if(result==value)
                break;
        }
    }else if(local_exec_mode==2){
       for(i=belonging_bucket*num_elements_per_bucket;i<belonging_bucket*num_elements_per_bucket+num_elements_per_bucket;i++){
            result=MIXED_PATH_SHARED_READ(STM_Heap[i]);
            if(result==value)
                break;
        }
	}else{
		for(i=belonging_bucket*num_elements_per_bucket;i<belonging_bucket*num_elements_per_bucket+num_elements_per_bucket;i++){
            result=FAST_PATH_SHARED_READ(HTM_Heap[i]);
            if(result==value)
                break;
        } 
	}
    if(result==-1){
        printf("-----VALUE: %d NOT FOUND------\n",value);
    }

	FAST_PATH_END();
    statistics_array[SPECIAL_THREAD_ID()].lookup_HTM++;
    
    return (result==value);
}


int get_stm(TM_ARGDECL int value,int belonging_bucket,int num_elements_per_bucket){
    int i=0;
    long result=0;    

    //printf("GET STM\n");
    SLOW_PATH_BEGIN(0,0,1);


    if(!local_exec_mode){
        for(i=belonging_bucket*num_elements_per_bucket;i<belonging_bucket*num_elements_per_bucket+num_elements_per_bucket;i++){
            result = SLOW_PATH_SHARED_READ(STM_Heap[i]);
            if(result==value)
                break;
        }
    }else if(local_exec_mode==2){
		for(i=belonging_bucket*num_elements_per_bucket;i<belonging_bucket*num_elements_per_bucket+num_elements_per_bucket;i++){
            result = MIXED_PATH_SHARED_READ(STM_Heap[i]);
            if(result==value)
                break;
        } 
	}else{
		for(i=belonging_bucket*num_elements_per_bucket;i<belonging_bucket*num_elements_per_bucket+num_elements_per_bucket;i++){
            result = FAST_PATH_SHARED_READ(HTM_Heap[i]);
            if(result==value)
                break;
        }
	}

    if(mode==1 && result==-1){
        printf("-----VALUE: %d NOT FOUND------\n",value);
    }
	
	SLOW_PATH_END();
    
	statistics_array[SPECIAL_THREAD_ID()].lookup_STM++;
    return (result==value);
}


int insert(TM_ARGDECL long value_to_find, unsigned int mySeed,int dataset_population,int belonging_bucket,int num_elements_per_bucket,int num_buckets){
    int i=0;
    int diff=0;
    long result=0;
    long value_to_insert=0;
    int position_to_insert=0;

    //printf("insert HTM\n");
    FAST_PATH_BEGIN(1,1,0);

     if(!local_exec_mode){
        for(i=belonging_bucket*num_elements_per_bucket;i<belonging_bucket*num_elements_per_bucket+num_elements_per_bucket;i++){
                result = SLOW_PATH_SHARED_READ(STM_Heap[i]);
                if(result==value_to_find){
                    value_to_insert = rand_r(&mySeed)%dataset_population;
                    SLOW_PATH_SHARED_WRITE(STM_Heap[i-1],value_to_insert);
                    break;
                }
        }
		value_to_insert = rand_r(&mySeed)%dataset_population;
        SLOW_PATH_SHARED_WRITE(STM_Heap[i-1],value_to_insert);
		}else if(local_exec_mode==2){
			for(i=belonging_bucket*num_elements_per_bucket;i<belonging_bucket*num_elements_per_bucket+num_elements_per_bucket;i++){
                result = MIXED_PATH_SHARED_READ(STM_Heap[i]);
                if(result==value_to_find){
                    value_to_insert = rand_r(&mySeed)%dataset_population;
                    MIXED_PATH_SHARED_WRITE(STM_Heap[i-1],value_to_insert);
                    break;
                }   
        }       
        value_to_insert = rand_r(&mySeed)%dataset_population;
        MIXED_PATH_SHARED_WRITE(STM_Heap[i-1],value_to_insert);
		}else{
			for(i=belonging_bucket*num_elements_per_bucket;i<belonging_bucket*num_elements_per_bucket+num_elements_per_bucket;i++){
                result = FAST_PATH_SHARED_READ(HTM_Heap[i]);
                if(result==value_to_find){
                    value_to_insert = rand_r(&mySeed)%dataset_population;
                    FAST_PATH_SHARED_WRITE(HTM_Heap[i-1],value_to_insert);
                    break;
                }
        }
        value_to_insert = rand_r(&mySeed)%dataset_population;
        FAST_PATH_SHARED_WRITE(HTM_Heap[i-1],value_to_insert);
		}
		
		FAST_PATH_END();

		statistics_array[SPECIAL_THREAD_ID()].insert_HTM++;
		return value_to_insert;
        } 
        

int insert_stm(TM_ARGDECL long value_to_find, unsigned int mySeed,int dataset_population,int belonging_bucket,int num_elements_per_bucket,int num_buckets){
    int i=0;
    int diff=0;
    long result=0;
    long value_to_insert=0;
    int position_to_insert=0;

    //printf("insert STM\n");

    SLOW_PATH_BEGIN(1,1,0);

     if(!local_exec_mode){
        for(i=belonging_bucket*num_elements_per_bucket;i<belonging_bucket*num_elements_per_bucket+num_elements_per_bucket;i++){
                result = SLOW_PATH_SHARED_READ(STM_Heap[i]);
                if(result==value_to_find){
                    value_to_insert = rand_r(&mySeed)%dataset_population;
                    SLOW_PATH_SHARED_WRITE(STM_Heap[i-1],value_to_insert);
                    break;
                }
        }
        value_to_insert = rand_r(&mySeed)%dataset_population;
        SLOW_PATH_SHARED_WRITE(STM_Heap[i-1],value_to_insert);
        }else if(local_exec_mode==2){
            for(i=belonging_bucket*num_elements_per_bucket;i<belonging_bucket*num_elements_per_bucket+num_elements_per_bucket;i++){
                result = MIXED_PATH_SHARED_READ(STM_Heap[i]);
                if(result==value_to_find){
                    value_to_insert = rand_r(&mySeed)%dataset_population;
                    MIXED_PATH_SHARED_WRITE(STM_Heap[i-1],value_to_insert);
                    break;
                }
        }
        value_to_insert = rand_r(&mySeed)%dataset_population;
        MIXED_PATH_SHARED_WRITE(STM_Heap[i-1],value_to_insert);
        }else{
            for(i=belonging_bucket*num_elements_per_bucket;i<belonging_bucket*num_elements_per_bucket+num_elements_per_bucket;i++){
                result = FAST_PATH_SHARED_READ(HTM_Heap[i]);
                if(result==value_to_find){
                    value_to_insert = rand_r(&mySeed)%dataset_population;
                    FAST_PATH_SHARED_WRITE(HTM_Heap[i-1],value_to_insert);
                    break;
                }
        }
        value_to_insert = rand_r(&mySeed)%dataset_population;
        FAST_PATH_SHARED_WRITE(HTM_Heap[i-1],value_to_insert);
        }

        SLOW_PATH_END();
		
		statistics_array[SPECIAL_THREAD_ID()].insert_STM++;
        return value_to_insert;		
}


void* test(void* data){
    volatile long numThread = thread_getId();
    volatile float j=0;
    int use_large_datastructure=0;
    long long value_to_find,value_to_insert,index=0;
    long long value=0;
    long p_both=0;
    long p_op=0;
    long p_bias=0;
    long page,index_inside=0;
    unsigned int mySeed = seed + sched_getcpu(); //seed is different for each thread
    volatile int var=0;
    int belonging_bucket=0;   
    //printf("operation threshold is %d\n", p_op_threshold);
    int p_op_threshold_local=p_op_threshold;
    long long dataset_population= DATASET_POPULATION;
    int large_structure_lower_limit=LARGE_STRUCTURE_LOWER_LIMIT;
    int num_buckets_small_dataset=NUM_BUCKETS_SMALL_DATASET;
    int num_elements_per_bucket_large=NUM_ELEMENTS_PER_BUCKET_LARGE;
    int num_elements_per_bucket_small=NUM_ELEMENTS_PER_BUCKET_SMALL;
    int num_buckets_large_dataset=NUM_BUCKETS_LARGE_DATASET;
    int page_beginning;
    int page_to_find=0;
    int position_to_access;
    int bucket;
    struct timeval current_time,last_time_clock_read, start_time;
    float time_difference,time_since_beginning;
    int p_bias_threshold_local=p_bias_threshold;
	int pool;	
    int i;

    int index_page[NUM_PAGES]= { 0 };
    //printf("NUM PAGES is %d",NUM_PAGES); 
 
    TM_THREAD_ENTER();
	gettimeofday(&last_time_clock_read,NULL); 
   
    while(time_difference<time_threshold){        
            //printf("\n-------\ntime difference is %f\n", time_difference);
        while(1){
          p_bias = rand_r(&mySeed)% 100;
          if((p_bias >0) && (p_bias<100))
          	break;
          }

          if(p_bias > p_bias_threshold_local){
			pool=0;
			num_tx=0;
          }else{ 
          	pool=1;
			num_tx=2;
         }        
             
		gettimeofday(&current_time,NULL);
        time_difference =((current_time.tv_sec * 1000000 + current_time.tv_usec)- (last_time_clock_read.tv_sec * 1000000 + last_time_clock_read.tv_usec))/1000000.0; 
            
        FIND_PAGE:page_to_find=rand_r(&mySeed)%NUM_PAGES;

         if(time_difference> timeouts[page_to_find][index_page[page_to_find]]){	
            __sync_add_and_fetch(&index_page[page_to_find],1); 
         //   printf("page  pool changed to %ld \n", index_page[page_to_find]);
         }	
        //for(i=0;i<NUM_PAGES;i++)
            //printf("PAGE %d has index %ld\n", i,index_page[i]);
        if(page_to_find<39){

            if((((int)index_page[page_to_find]) % 2) != pool){ 
                    //printf("Did not found any page %d with index %d in pool %ld\n", page_to_find, index_page[page_to_find],pool);
                    goto FIND_PAGE;
            }
        }else{
            if((((int)index_page[page_to_find]+1) % 2) != pool){
                    //printf("Did not found any page %d with index %d in pool %ld\n", page_to_find, index_page[page_to_find]+1,pool);
                    goto FIND_PAGE;
            }
        }
        
         //printf("time_difference is %f\n",time_difference);
			//index_page[page_to_find]= (index_page[page_to_find]+1)%TOTAL_NUMBER_POSITIONS;

    //        printf("Page to be found is %ld and mode is %d\n",page_to_find,mode);
             
            //mode= (page_to_find%2 + (int)(time_difference/timeout_change)) % 2;
            
            /*if((time_difference> timeouts[page_to_find][index_page[page_to_find]]/3) && (time_difference< timeouts[page_to_find][index_page[page_to_find]]))
                    disjoint_partitions=1;
            
            if(time_difference> timeouts[page_to_find][index_page[page_to_find]]){
                disjoint_partitions=0;
                index_page[page_to_find]= (index_page[page_to_find]+1)%2000;
                printf("mode of page %d changed to %d \n",page_to_find,index_page[page_to_find]);
                last_time_clock_read = current_time;
            } */          
            

    //        printf("time difference %f timeout change %f\n",time_difference,timeout_change);
            //printf("mode of page %d is %d\n",page_to_find,mode);
            while(1){
                p_op = rand_r(&mySeed) %100;
                if((p_op > 0) && (p_op<100))
                    break;
            }
            
            if(pool){
                page_beginning = (page_to_find*NUM_ELEMENTS_PAGE)/num_elements_per_bucket_large;
                bucket=(rand_r(&mySeed)%(num_buckets_large_dataset/NUM_PAGES))+page_beginning;
            }else{
                page_beginning = (page_to_find*NUM_ELEMENTS_PAGE)/num_elements_per_bucket_small;
                bucket=(rand_r(&mySeed)%(num_buckets_small_dataset/NUM_PAGES))+page_beginning;
            }

        //printf("page %d with mode %d is in the bucket %d num buckets large dataset %d\n",page_to_find,mode,bucket,num_buckets_large_dataset);
        //A lookup operation
        if(p_op > p_op_threshold_local){
            value = rand_r(&mySeed)%dataset_population;
                //printf("I'm doing a read\n");
            //TM_BEGIN_EXT(1,mode,1);

              var = pool == 0 ? get(TM_ARG value,bucket,num_elements_per_bucket_small) : get_stm(TM_ARG value,bucket,num_elements_per_bucket_large);

           //TM_END();
            
      //      if(mode)
        //        printf("Just ended");

           if(var==-1){
                printf("-----VALUE: %d NOT FOUND------\n",value);
            }
        //A write operation
        }else{
                value_to_find = rand_r(&mySeed)%dataset_population;
         
        //TM_BEGIN_EXT(1,mode,0);
                var = pool == 0 ? insert(TM_ARG value_to_find,mySeed,dataset_population,bucket,num_elements_per_bucket_small,num_buckets_small_dataset) : insert_stm(TM_ARG value_to_find,mySeed,dataset_population,bucket,num_elements_per_bucket_large,num_buckets_large_dataset);
                //            var = mode == 0 ? insert_value_short_structure(TM_ARG value_to_find,mySeed,small_dataset_population,num_buckets_small_dataset,num_elements_per_bucket_small): (use_large_datastructure==1 ? insert_value_large_structure(TM_ARG value_to_find,mySeed,num_buckets_converted,num_elements_per_bucket_large,large_dataset_population,num_buckets_large_dataset) : insert_value_short_structure_STM(TM_ARG value_to_find,mySeed,small_dataset_population,num_buckets_small_dataset,num_elements_per_bucket_small));           

          // TM_END();
        } 
            if(var==-1){
                printf("VALUE INSERTED IS %d\n",var);
            }
        //num_operations[SPECIAL_THREAD_ID()]++;
	}

        TM_THREAD_EXIT();
        return NULL;

}

