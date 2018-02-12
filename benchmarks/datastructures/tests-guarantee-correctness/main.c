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
#include "main.h"
#include <pthread.h>


#define DEFAULT_NB_THREADS              2
#define SHORT_STRUCTURE_LIMIT      64
#define LARGE_STRUCTURE_LOWER_LIMIT     NUM_ELEMENTS_PAGE
#define LARGE_STRUCTURE_LIMIT          NUM_ELEMENTS_PAGE+NUM_ELEMENTS_PAGE*2
#define NUM_RUNS                        5
#define NUM_OPERATIONS                  10000.0


static int lock_initialising_accounts=1;
static int handler_lock=0;
static int sum_money=0;
int system_call_counter[16];
int transition_count_counter[16];

float sum_transition_count=0;
float sum_system_call=0;

static long p_both_threshold=0;
static long p_bias_threshold=0;
static long p_op_threshold=0;
unsigned int nb_threads=0;
static float num_operations=NUM_OPERATIONS;

unsigned int seed;


int main(int argc,char *argv[]){

    //shared data between HTM and STM
    struct sigaction sa; 
    int i=0;
    nb_threads = atoi(argv[1]); 
    p_both_threshold = atoi(argv[2]);
    p_bias_threshold = atoi(argv[3]);
    p_op_threshold = atoi(argv[4]);
    num_operations = atof(argv[5]);
    int* pointer_to_physical;
    int fd;

    fd = shm_open("/shared_array", O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
    if (fd == -1)
        printf("FAILURE CREATING SHARED MEMORY\n");

    if (ftruncate(fd, sizeof(long) * NUM_ELEMENTS_ARRAY) == -1)
        printf("FAILURE TRUNCATE\n");

	HTM_Heap = mmap (NULL, sizeof(long) * NUM_ELEMENTS_ARRAY, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    STM_Heap = mmap (NULL, sizeof(long) * NUM_ELEMENTS_ARRAY, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);  

    //Deal with failure on the mapping
    if(HTM_Heap == MAP_FAILED)
        printf("MAP_FAILED\n");

    if(STM_Heap == MAP_FAILED)
         printf("MAP_FAILED\n");
    

    seed = time(NULL);
    srand(seed);

    for (i=0; i<NUM_ELEMENTS_ARRAY;i++){
    	HTM_Heap[i]=(long)(rand() % 100);
		STM_Heap[i]=(long)(rand() % 100);
    }

        TM_STARTUP(nb_threads,42);
        thread_startup(nb_threads);

    float time=0;
    float throughput=0;
    struct timeval start, end;

    //for(i=0;i<NUM_RUNS;i++){
        //startEnergyIntel();
        gettimeofday(&start, NULL);
        thread_start(test_HTM_read_after_STM_write,NULL);
        gettimeofday(&end, NULL);

        //printf("\n\nI came out of test\n\n");
        time=((end.tv_sec * 1000000 + end.tv_usec) - (start.tv_sec * 1000000 + start.tv_usec))/1000000.0;
        printf("time elapsed: %f\n",time); 
        
        throughput += (num_operations * nb_threads)/time;
    //}

    //printf ("%f\n",time); 
    //printf ("Throughput: %f\n",throughput);    
   
    for(i=0;i<NUM_PAGES;i++){
        sum_transition_count+=transition_count_counter[i];
        sum_system_call+=system_call_counter[i];
    }

    //printf ("SYSTEM CALLS ISSUED %d\n",sum_system_call);
    //printf ("transition count incremented %d times\n",sum_transition_count);
    printf("Throughput: %f  \nNumber of System Calls: %f  \nNumber of transition counts: %f  ",throughput,sum_system_call, sum_transition_count);
    
    //Unmap memory chunk
    munmap (HTM_Heap,sizeof(long) * NUM_ELEMENTS_ARRAY);
	munmap (STM_Heap,sizeof(long) * NUM_ELEMENTS_ARRAY); 

	//Free memory
	free(metadata_array);
    
    //Close shared object
    shm_unlink("/shared_array");

    TM_SHUTDOWN();  
    thread_shutdown();
    

    return 0;
}

/*
 * Scenario where after a write of STM, HTM tries to read a value from the same page
 * Expected to release a signal
*/
 void* test_HTM_read_after_STM_write(void* data){    
   
    printf("THREAD_ID = %d\n",thread_getId());
    long mode = thread_getId();
   
   if(mode == 1){ 
        TM_THREAD_ENTER();
    }   

    long result=50;
    int i=0;
    
         if(mode == 1){ 
            printf("STM Start!\n");
         }else{
            for(i=0;i<150000;i++){
                __asm__ ("nop;");
            }
            printf("HTM Start!\n");
         }

    update_transaction = 0;
    TM_BEGIN(1,mode);    
            
        if(mode == 0){ 
            result = FAST_PATH_SHARED_READ(HTM_Heap[3]);
        }else{
            SLOW_PATH_SHARED_WRITE(STM_Heap[3],(long)555);
        }   

    TM_END();

    
    if(mode==1){
            printf("STM finished!\n");
    }else{
            printf("HTM finished!\n");
            printf("Value read is %d\n",result);
    }

   
   if(mode == 1)
        TM_THREAD_EXIT();    

   return NULL;
}

/*
 * Scenario where after a STM read, a HTM transaction reads
 * Expected to not throw any signal as both transactions can run concurrently
*/
 void* test_HTM_read_after_STM_read(void* data){
     
    printf("THREAD_ID = %d\n",thread_getId());
    long mode = thread_getId();

    if(mode == 1){
        TM_THREAD_ENTER();
    }

    int result=50;
    int i=0;

         if(mode == 1){
            printf("STM Start!\n");
         }else{
            printf("HTM Start!\n");
            for(i=0;i<15000;i++)
                __asm__ ("nop;");
         }
    
    update_transaction = 0;
    TM_BEGIN(1,mode);

        if(mode == 0){
            result = FAST_PATH_SHARED_READ(HTM_Heap[3]);
        }else{
            result = SLOW_PATH_SHARED_READ(STM_Heap[3]);
        }

    TM_END();


    if(mode==1){
            printf("STM finished!\n");
            printf("Value read by STM is %d\n",result);
    }else{
            printf("HTM finished!\n");
            printf("Value read by HTM is %d\n",result);
    }


    if(mode == 1)
        TM_THREAD_EXIT();

    return NULL;    
}

/*
 *  Scenario where after a STM write, a STM transaction issues a write and then another transaction issues a read
 *  Expected to not throw any signal as both transactions can run concurrently
 *  
 */
void* test_STM_read_after_STM_write_after_STM_write(void *data){
    
    printf("THREAD_ID = %d\n",thread_getId());
    long mode = 1;
    long numThread = thread_getId();

        TM_THREAD_ENTER();
    
    int result=50;
    int i=0;
    
        if(numThread == 0){
            printf("STM Start transaction 1!\n");
        }
        if(numThread == 1){
            printf("STM Start transaction 2!\n");
            for(i=0;i<150000;i++)
                __asm__ ("nop;");
        }
         if(numThread == 2){
            printf("STM Start transaction 3!\n");
            for(i=0;i<300000;i++)
                __asm__ ("nop;");
         }  
         
    TM_BEGIN(1,mode);
    
        if(numThread == 0){
            SLOW_PATH_SHARED_WRITE(STM_Heap[3],555); 
        }
        if(numThread == 1){
            SLOW_PATH_SHARED_WRITE(STM_Heap[3],111); 
        }   
        if(numThread == 2){
            result=SLOW_PATH_SHARED_READ(STM_Heap[3]); 
        }
    TM_END();
    
        if(numThread == 0){
            printf("STM transaction 1 finished\n");
        }
        if(numThread == 1){
            printf("STM transaction 2 finished\n");
        }
        if(numThread == 2){
            printf("STM transaction 3 finished\n");
        }
    
   printf("result is %d\n",result);
   
  TM_THREAD_EXIT();
        
    return NULL; 
    
}

/*
 *  Scenario where after a STM read, a STM transaction issues a read
 *  Expected to not throw any signal as both transactions can run concurrently
 *  
 */
void* test_STM_read_after_STM_read(void *data){
        
    printf("THREAD_ID = %d\n",thread_getId());
    long numThread = thread_getId();
    long mode = 1;

    TM_THREAD_ENTER();
    
    int result=50;
    int i=0;
    
        if(numThread == 0){ 
            printf("STM Start transaction 1!\n");
        }   
        if(numThread == 1){ 
            printf("STM Start transaction 2!\n");
            for(i=0;i<150000;i++);
        }   
  
    TM_BEGIN(1,mode);
    
        if(numThread == 0){ 
            SLOW_PATH_SHARED_READ(STM_Heap[3]); 
        }   
        if(numThread == 1){ 
            SLOW_PATH_SHARED_READ(STM_Heap[3]); 
        }   
    TM_END();
    
        if(numThread == 0){ 
            printf("STM transaction 1 finished\n");
        }   
        if(numThread == 1){ 
            printf("STM transaction 2 finished\n");
        }   
    
   printf("result is %d\n",result);
   
  TM_THREAD_EXIT();
    
   return NULL; 
    
}

/*
 *  Scenario where after a STM read, a HTM transaction issues a write
 *  Expected to not throw any signal as both transactions can run concurrently
 *  
 */
void* test_HTM_write_after_STM_read(void *data){
    
    printf("THREAD_ID = %d\n",thread_getId());
    long mode = thread_getId();

    if(mode == 1){
        TM_THREAD_ENTER();
    }   
    
    long result=50;
    int i=0;
    
         if(mode == 1){
            printf("STM Start!\n");
         }else{
            printf("HTM Start!\n");
            for(i=0;i<1000000;i++)
                   __asm__ ("nop;");
         }  

    update_transaction =1;   
    TM_BEGIN(1,mode);
    
        if(mode == 0){
            //Just do 150000 iterations
            //for(i=0;i<150000;i++);
            
            FAST_PATH_SHARED_WRITE(HTM_Heap[3],666);
        }else{
            result = SLOW_PATH_SHARED_READ(STM_Heap[3]);
        }   
        
    TM_END();
    

    if(mode==1){
            printf("STM finished!\n");
            printf("Value read by STM is %ld\n",result);
    }else{  
            printf("HTM finished!\n");
    }

    if(mode == 1)
        TM_THREAD_EXIT();

    return NULL;    
}

/*
 *  Scenario where after a STM read, a STM transaction issues a write and then a read
 *  Expected to not throw any signal as both transactions can run concurrently
 *  
 */
void* test_STM_read_after_STM_write_after_STM_read(void *data){
    
   printf("THREAD_ID = %d\n",thread_getId());
    long mode = 1;
    long numThread = thread_getId();

        TM_THREAD_ENTER();
    
    int result=50;
    int i=0;
    
        if(numThread == 0){ 
            printf("STM Start transaction 1!\n");
        }   
        if(numThread == 1){ 
            printf("STM Start transaction 2!\n");
            for(i=0;i<150000;i++);
        }   
         if(numThread == 2){ 
            printf("STM Start transaction 3!\n");
            for(i=0;i<300000;i++);
         }   

        TM_BEGIN(1,mode);
 
        if(numThread == 0){
            result = SLOW_PATH_SHARED_READ(STM_Heap[3]);
        }
        if(numThread == 1){
            SLOW_PATH_SHARED_WRITE(STM_Heap[3],111);
        }
        if(numThread == 2){
            result=SLOW_PATH_SHARED_READ(STM_Heap[3]);
        }
    TM_END();

        if(numThread == 0){
            printf("STM transaction 1 finished\n");
        }
        if(numThread == 1){
            printf("STM transaction 2 finished\n");
        }
        if(numThread == 2){
            printf("STM transaction 3 finished\n");
        }

   printf("result is %d\n",result);

  TM_THREAD_EXIT();

    return NULL;
    }
   

/*
 *  Scenario where after a STM read, a STM transaction issues a write and then a read
 *  Expected to not throw any signal as both transactions can run concurrently
 *  
 */
void* test_HTM_read_after_HTM_write(void *data){
  
    printf("THREAD_ID = %d\n",thread_getId());
    long numThread = thread_getId();
    long mode = 0;
    int result=50;
    int i=0;
    
        if(numThread == 0){ 
            printf("HTM Start transaction 1!\n");
        }   
        if(numThread == 1){ 
            printf("HTM Start transaction 2!\n");
            for(i=0;i<150000;i++)
                __asm__ ("nop;");
        }   
 
        update_transaction=1;
    TM_BEGIN(1,mode);
    
        if(numThread == 0){ 
            FAST_PATH_SHARED_WRITE(HTM_Heap[3],8); 
        }   
        if(numThread == 1){ 
            result = FAST_PATH_SHARED_READ(HTM_Heap[3]); 
        }   
    TM_END();
    
        if(numThread == 0){ 
            printf("HTM transaction 1 finished\n");
        }   
        if(numThread == 1){ 
            printf("HTM transaction 2 finished\n");
        }   
    
   printf("result is %d\n",result);
    
   return NULL;     
}

/*
 *  Scenario where after a STM write, a HTM transaction issues a write on a differentpage
 *  Expected to not throw any signal as both transactions can run concurrently
 *  
 */
void* test_HTM_write_after_STM_write_different_page(void *data){
 
    printf("THREAD_ID = %d\n",thread_getId());
    long mode = thread_getId();
    
    if(mode == 1){ 
        TM_THREAD_ENTER();
    }   
    
    int result=50;
    int i=0;
    
         if(mode == 1){ 
            printf("STM Start!\n");
         }else{
            printf("HTM Start!\n");
            for(i=0;i<150000;i++)
                __asm__ ("nop;");
         }   
    
    update_transaction =1;
    TM_BEGIN(1,mode);
    
        if(mode == 0){ 
            FAST_PATH_SHARED_WRITE(HTM_Heap[8193],9);
        }else{
            SLOW_PATH_SHARED_WRITE(STM_Heap[6],999);
        }   
    
    TM_END();
    

    if(mode==1){
            printf("STM finished!\n");
    }else{
            printf("HTM finished!\n");
    }

    if(mode == 1)
        TM_THREAD_EXIT();

    return NULL;
    }

void* test_STM_write_after_STM_write(void *data){
 
    printf("THREAD_ID = %d\n",thread_getId());
    long numThread = thread_getId();
    long mode = 1;

    TM_THREAD_ENTER();
    
    int result=50;
    int i=0;
    
        if(numThread == 0){
            printf("STM Start transaction 1!\n");
        }
        if(numThread == 1){
            printf("STM Start transaction 2!\n");
            for(i=0;i<150000;i++);
        }
  
    TM_BEGIN(1,mode);
    
        if(numThread == 0){
            SLOW_PATH_SHARED_WRITE(STM_Heap[3],555); 
        }
        if(numThread == 1){
            SLOW_PATH_SHARED_WRITE(STM_Heap[3],111); 
        }   
    TM_END();
    
        if(numThread == 0){
            printf("STM transaction 1 finished\n");
        }
        if(numThread == 1){
            printf("STM transaction 2 finished\n");
        }
    
   printf("result is %d\n",result);
   
  TM_THREAD_EXIT();
        
   return NULL;  
}
/*
 *  Scenario where after a STM read, a HTM transaction issues a write
 *  Expected to not throw any signal as both transactions can run concurrently
 *  
 */
void* test_STM_aborts_transition_count_changes(void *data){
    
    printf("THREAD_ID = %d\n",thread_getId());
    long mode = thread_getId();
    
    if(mode == 1){
        TM_THREAD_ENTER();
    }   
    
    int result=50;
    int i=0;
    
         if(mode == 1){
            printf("STM Start!\n");
         }else{
            printf("HTM Start!\n");
            for(i=0;i<150000;i++);
        }  
        update_transaction=1; 
    TM_BEGIN(1,mode);
    
        if(mode == 0){
            
            FAST_PATH_SHARED_WRITE(HTM_Heap[3],666);
        }else{
            result = SLOW_PATH_SHARED_READ(STM_Heap[3]);
            for(i=0;i<1500000;i++);
            printf("RETURN FROM FOR\n");
        }   
        
    TM_END();
    
    
    if(mode==1){
            printf("STM finished!\n");
    }else{  
            printf("HTM finished!\n");
    }

   printf("result is %d\n",result);

    if(mode == 1)
        TM_THREAD_EXIT();

    return NULL;    
}
/*
 *  Scenario where after a STM read, a HTM transaction issues a write
 *  Expected to not throw any signal as both transactions can run concurrently
 *  
 */
void* test_HTM_have_to_wait_for_writers_to_restore_protection(void *data){
    
    printf("THREAD_ID = %d\n",thread_getId());
    long mode = thread_getId();
    
    if(mode == 1){
        TM_THREAD_ENTER();
    }   
    
    int result=50;
    int i=0;
    
         if(mode == 1){
            printf("STM Start!\n");
         }else{
            printf("HTM Start!\n");
            for(i=0;i<150000;i++);
         }  
    update_transaction =1;         
    TM_BEGIN(1,mode);
    
        if(mode == 0){
            //Just do 150000 iterations
            //for(i=0;i<150000;i++);
            
            FAST_PATH_SHARED_WRITE(HTM_Heap[3],666);
        }else{
            SLOW_PATH_SHARED_WRITE(STM_Heap[3],666);        
            for(i=0;i<300000;i++);
         }   
        
    TM_END();
    

    if(mode==1){
            printf("STM finished!\n");
    }else{  
            printf("HTM finished!\n");
    }

   printf("result is %d\n",result);

    if(mode == 1)
        TM_THREAD_EXIT();

    return NULL;    
}

void* test_STM_waits_to_set_the_page_protection(void *data){
    
    printf("THREAD_ID = %d\n",thread_getId());
    long mode = 1;
    long numThread = thread_getId();

        TM_THREAD_ENTER();
    
    int result=50;
    int i=0;
    
        if(numThread == 0){
            printf("STM Start transaction 1!\n");
        }
        if(numThread == 1){
            printf("STM Start transaction 2!\n");
        }
         if(numThread == 2){
            printf("STM Start transaction 3!\n");
         }  
         
    TM_BEGIN(1,mode);
    
        if(numThread == 0){
            result=SLOW_PATH_SHARED_READ(STM_Heap[3]); 
        }
        if(numThread == 1){
            result=SLOW_PATH_SHARED_READ(STM_Heap[3]); 
        }   
        if(numThread == 2){
            result=SLOW_PATH_SHARED_READ(STM_Heap[3]); 
        }
    TM_END();
    
        if(numThread == 0){
            printf("STM transaction 1 finished\n");
        }
        if(numThread == 1){
            printf("STM transaction 2 finished\n");
        }
        if(numThread == 2){
            printf("STM transaction 3 finished\n");
        }
    
   printf("result is %d\n",result);
   
  TM_THREAD_EXIT();
        
    return NULL; 
    
}

/*
 * Scenario where after a STM read, a HTM transaction reads
 * Expected to not throw any signal as both transactions can run concurrently
*/
 void* test_STM_breaks_opacity(void* data){
     
    printf("THREAD_ID = %d\n",thread_getId());
    long mode = thread_getId();

    if(mode == 1){
        TM_THREAD_ENTER();
    }

    int result=50;
    int i=0;

         if(mode == 1){
            printf("STM Start!\n");
         }else{
            printf("HTM Start!\n");
            for(i=0;i<150000;i++);
         }

    update_transaction=1;
    TM_BEGIN(1,mode);

        if(mode == 0){
            //Just do 150000 iterations
            //for(i=0;i<150000;i++);

            FAST_PATH_SHARED_WRITE(HTM_Heap[3],2);
            FAST_PATH_SHARED_WRITE(HTM_Heap[8194],2);
            }else{
            result = SLOW_PATH_SHARED_READ(STM_Heap[3]);
            for(i=0;i<15000000;i++);
            printf("CAME BACK\n");
            result = SLOW_PATH_SHARED_READ(STM_Heap[8193]);
            
        }

    TM_END();


    if(mode==1){
            printf("STM finished!\n");
    }else{
            printf("HTM finished!\n");
    }

   printf("result is %d\n",result);

    if(mode == 1)
        TM_THREAD_EXIT();

    return NULL;    
}

/*
 * Scenario where after a STM read, a HTM transaction reads
 * Expected to not throw any signal as both transactions can run concurrently
*/

 void* test_STM_write_middle_htm_write(void* data){
   
    printf("THREAD_ID = %d\n",thread_getId());
    long mode = thread_getId();

    if(mode == 1)
    TM_THREAD_ENTER();

    int result=50;
    int i=0;

         if(mode == 1){ 
            printf("STM Start!\n");
         }else{
            printf("HTM Start!\n");
         }   

    update_transaction = 1;

    TM_BEGIN(1,mode);

        if(mode == 0){ 
            FAST_PATH_SHARED_WRITE(HTM_Heap[3],666);
            for(i=0;i<15000;i++);
        }else{
            SLOW_PATH_SHARED_WRITE(STM_Heap[3],1);
        }   

    TM_END();


    if(mode==1){
            printf("STM finished!\n");
    }else{
            printf("HTM finished!\n");
    }   

    printf("result is %d\n",STM_Heap[3]);

    if(mode == 1)
    TM_THREAD_EXIT();

    return NULL; 
}
/*
 * Scenario where STM first aborts and then it manages to read
 * Not suposed to throw any signal
 * Abort and restart are handled by STM itself
 */

void* test_STM_first_aborts_then_reads(void* data){
    printf("THREAD_ID = %d\n",thread_getId());
    long mode=1;
    int i=0;

    if(mode == 1){ 
        TM_THREAD_ENTER();
    }   

    int result=50;

         if(mode == 1){ 
            printf("STM Start!\n");
         }else{
            printf("HTM Start!\n");
            for(i=0;i<150000;i++);
         }   

    update_transaction = 0;
    TM_BEGIN(1,mode);

        if(mode == 0){ 
            result = FAST_PATH_SHARED_READ(HTM_Heap[3]);
        }else{
            if(i ==0){
                i++;
                printf("RESTARTED\n");
                SLOW_PATH_RESTART();
             }else{
                result = SLOW_PATH_SHARED_READ(STM_Heap[3]);
             }        
        }   

    TM_END();


    if(mode==1){
            printf("STM finished!\n");
    }else{
            printf("HTM finished!\n");
    }   

   printf("result is %d\n",result);

    if(mode == 1)
        TM_THREAD_EXIT();

    return NULL;
}



void* test_HTM_exhausts_transition_count_fallbacks_STM(void *data){
 
    printf("THREAD_ID = %d\n",thread_getId());
    long numThread = thread_getId();
    long mode = numThread;
    int i=0;    
    
    TM_THREAD_ENTER();
    
    metadata_array[0].transition_count=5;   
 
        if(mode == 0){
            printf("HTM Start transaction 1!\n");
            for(i=0;i<150000;i++);
        }
        if(mode == 1){
            printf("STM Start transaction 2!\n");
        }
  
    TM_BEGIN(1,mode);
    
        if(mode == 0){
            FAST_PATH_SHARED_WRITE(HTM_Heap[3],555); 
            FAST_PATH_SHARED_WRITE(HTM_Heap[3],2);
        }
        if(mode == 1){
            SLOW_PATH_SHARED_WRITE(STM_Heap[3],111); 
            SLOW_PATH_SHARED_WRITE(STM_Heap[3],1);
        }   
    TM_END();
    
        if(mode == 0){
            printf("HTM transaction 1 finished\n");
        }
        if(mode == 1){
            printf("STM transaction 2 finished\n");
        }
    
     printf("result is %d\n",STM_Heap[3]);
   
     TM_THREAD_EXIT();
        
   return NULL;  
}

// Test where STM first reads and then two HTM threads try to write and then get an AV
void* test_HTM_write_after_HTM_write_after_STM_read(void *data){

    printf("THREAD_ID = %d\n",thread_getId());
    long mode = (thread_getId() % 2);
    
    if(mode == 1){
        TM_THREAD_ENTER();
    }   
    
    int result=50;
    int i=0;
    
         if(mode == 1){
            printf("STM Start!\n");
         }else{
            printf("HTM Start THREAD ID %d!\n",thread_getId());
            for(i=0;i<300000;i++);
         }  
         
    update_transaction =1;
    TM_BEGIN(1,mode);
    
        if(mode == 0){
            //Just do 150000 iterations
            //for(i=0;i<150000;i++);
            
            FAST_PATH_SHARED_WRITE(HTM_Heap[3],666);
        }else{
            result = SLOW_PATH_SHARED_READ(STM_Heap[3]);
        }   
        
    TM_END();
    
    
    if(mode==1){
            printf("STM finished!\n");
    }else{  
        printf("HTM Finished THREAD ID %d!\n",thread_getId());
    }       
    
   printf("result is %d\n",STM_Heap[3]);
   
    if(mode == 1)
        TM_THREAD_EXIT();
        
    return NULL;
}

// Bank benchmark where exists 1000 accounts and one HTM xact transacts money from one account to another 
// While STM is sweeping entire accounts and assert that the total sum of money remains the same
void* single_transaction_bank_benchmark(void *data){
    
    printf("THREAD_ID = %d\n",thread_getId());
    long numThread = thread_getId();
    int j,i=0;    
    int result,do_next_transaction=1;
    int num_accounts=1000;
    int account_money=0;
    long increment_inside_loop=0;

    srand(time(NULL) * sched_getcpu() * numThread);
    int index = rand() % num_accounts;
    
    srand(time(NULL)+ 3.14159265359+numThread);
    int index2 = rand() % num_accounts;
    
    srand(time(NULL) + sched_getcpu() * sched_getcpu()+numThread);
    long mode = rand()% 2;
    
    if(numThread == 1){
        mode=1;
    }
    

    printf("###### MODE = %d THREAD NUM:%d #######\n ", mode,numThread);

    //Initializes all accounts with 200 euros and calculate sum of it
    if(numThread == 1){
        for (i=0; i<num_accounts;i++){
            HTM_Heap[i]=200;
            sum_money=sum_money+200;
        }
        printf("SUM of money is %d\n",sum_money);
        lock_initialising_accounts = 0;
    }else{
        while(lock_initialising_accounts)
            increment_inside_loop++;
    }

    //wait until all threads have initialized their accounts 
    //if(mode==0)
    //for(i=0;i<10000;i++);
    
    TM_THREAD_ENTER();
    
    if(numThread != 1){
        printf("Index to be withdraw money is %d\n",index);
        printf("Index to be deposited money is %d\n",index2);
    }

        if(mode == 0){
            printf("HTM Start transaction %d!\n",numThread);
        }    
        if((mode == 1) && (numThread==1)){
            printf("STM Start transaction %d!\n",numThread);
            TM_BEGIN(1,mode);
            
            result=0;
            printf("STM transaction has began\n");

            for(j=0;j<4;j++){           
                for (i=0; i<num_accounts;i++){    
                   //printf("READ\n");
                    account_money=SLOW_PATH_SHARED_READ(STM_Heap[i]);       
                    result += account_money;
                    printf("---%d->has:%d\n",i,account_money);
                }
                printf("TRANSACTION FINISHED ALL ACCOUNTS, RESULT IS %d and SUM_MONEY IS %d\n",result,sum_money);
                assert(result == 200000); 
                result=0;  
                do_next_transaction=0;
            }
            TM_END();
        }
        if((mode == 1) && (numThread != 1)){ 
            printf("STM Start Transaction %d!!\n",numThread);
        }


    update_transaction=1; 
    
    if(do_next_transaction){
    TM_BEGIN(1,mode);
    
        if(mode == 0){
            int first_account;
            int second_account;

            first_account= FAST_PATH_SHARED_READ(HTM_Heap[index]);
            FAST_PATH_SHARED_WRITE(HTM_Heap[index],first_account-100);
            second_account = FAST_PATH_SHARED_READ(HTM_Heap[index2]);
            FAST_PATH_SHARED_WRITE(HTM_Heap[index2],second_account+100); 
        }    
        if(mode == 1){
            int first_account;
            int second_account;
            printf("-----FIRST READ-----THREAD ID %d\n",numThread);            
            first_account= SLOW_PATH_SHARED_READ(STM_Heap[index]);
            SLOW_PATH_SHARED_WRITE(STM_Heap[index],first_account-100);
            printf("-----SECOND READ-----THREAD ID %d\n",numThread);  
            second_account = SLOW_PATH_SHARED_READ(STM_Heap[index2]);
            SLOW_PATH_SHARED_WRITE(STM_Heap[index2],second_account+100);
        }    
    TM_END();
    } 
        if(mode == 0){
            printf("HTM transaction %d finished\n", numThread);
        }    
        if(mode == 1){
            printf("STM transaction %d finished\n", numThread);
        }    
    
         TM_THREAD_EXIT();
             
   return NULL;      

}

// Bank benchmark where exists 1000 accounts and one HTM xact transacts money from one account to another three times
// While STM is sweeping entire accounts and assert that the total sum of money remains the same
void* triple_transaction_bank_benchmark(void *data){
    
    printf("THREAD_ID = %d\n",thread_getId());
    long numThread = thread_getId();
    int j,i=0;    
    int sum_money=0;
    int result,do_next_transaction=1;
    int num_accounts=1000;
    int account_money=0;
    int index[3],index2[3];
    int number_money_tx_per_xact=3;

    srand(time(NULL) * sched_getcpu() * numThread);
    for(j=0;j<number_money_tx_per_xact;j++){
        index[j] = rand() % num_accounts;
    }

    srand(time(NULL)+ 3.14159265359+numThread);
     for(j=0;j<number_money_tx_per_xact;j++){
        index2[j] = rand() % num_accounts;
     }

    srand(time(NULL) + sched_getcpu() * sched_getcpu()+numThread);
    long mode = rand()% 2;
    
    if(numThread == 1){
        mode=1;
    }
    
    printf("MODE = %d THREAD NUM:%d\n ", mode,numThread);

     //Initializes all accounts with 200 euros and calculate sum of it
    if(numThread == 1){
        for (i=0; i<num_accounts;i++){
            HTM_Heap[i]=200;
            sum_money=sum_money+200;
        }    
        printf("SUM of money is %d\n",sum_money);
        lock_initialising_accounts = 0; 
    }else{
        while(lock_initialising_accounts);
    }    
    
  
    //wait until all threads have initialized their accounts 
    //if(mode==0)
    //for(i=0;i<150000;i++);
    
    TM_THREAD_ENTER();
    
    if(numThread != 1){
        for(j=0;j<number_money_tx_per_xact;j++){
            printf("Index to be withdraw money is %d\n",index[j]);
            printf("Index to be deposited money is %d\n",index2[j]);
        }
    }

        if(mode == 0){
            printf("HTM Start transaction %d!\n",numThread);
        }    
        if((mode == 1) && (numThread==1)){
            printf("STM Start transaction %d!\n",numThread);
            TM_BEGIN(1,mode);
            
            result=0;
            printf("STM transaction has began\n");

            for(j=0;j<2;j++){           
                for (i=0; i<num_accounts;i++){    
                   //printf("READ\n");
                    account_money=SLOW_PATH_SHARED_READ(STM_Heap[i]);       
                    result += account_money;
                    printf("---%d->has:%d\n",i,account_money);
                }
                printf("TRANSACTION FINISHED ALL ACCOUNTS, RESULT IS %d\n",result);
                assert(result == sum_money); 
                result=0;  
                do_next_transaction=0;
            }
            TM_END();
        }
        if((mode == 1) && (numThread != 1)){ 
            printf("STM Start Transaction %d!!\n",numThread);
        }


    update_transaction=1; 
    
    if(do_next_transaction){
    TM_BEGIN(1,mode);
    
        if(mode == 0){
            int first_account;
            int second_account;
          
                first_account= FAST_PATH_SHARED_READ(HTM_Heap[index[0]]);
                FAST_PATH_SHARED_WRITE(HTM_Heap[index[0]],first_account-100);
                second_account = FAST_PATH_SHARED_READ(HTM_Heap[index2[0]]);
                FAST_PATH_SHARED_WRITE(HTM_Heap[index2[0]],second_account+100); 
        
                first_account= FAST_PATH_SHARED_READ(HTM_Heap[index[1]]);
                FAST_PATH_SHARED_WRITE(HTM_Heap[index[1]],first_account-100);
                second_account = FAST_PATH_SHARED_READ(HTM_Heap[index2[1]]);
                FAST_PATH_SHARED_WRITE(HTM_Heap[index2[1]],second_account+100);    
    
                first_account= FAST_PATH_SHARED_READ(HTM_Heap[index[2]]);
                FAST_PATH_SHARED_WRITE(HTM_Heap[index[2]],first_account-100);
                second_account = FAST_PATH_SHARED_READ(HTM_Heap[index2[2]]);
                FAST_PATH_SHARED_WRITE(HTM_Heap[index2[2]],second_account+100);
    }
        if(mode == 1){
            long first_account;
            long second_account;
                
                first_account= SLOW_PATH_SHARED_READ(STM_Heap[index[0]]);
                SLOW_PATH_SHARED_WRITE(STM_Heap[index[0]],first_account-100);
                second_account = SLOW_PATH_SHARED_READ(STM_Heap[index2[0]]);
                SLOW_PATH_SHARED_WRITE(STM_Heap[index2[0]],second_account+100);
     
                first_account= SLOW_PATH_SHARED_READ(STM_Heap[index[1]]);
                SLOW_PATH_SHARED_WRITE(STM_Heap[index[1]],first_account-100);
                second_account = SLOW_PATH_SHARED_READ(STM_Heap[index2[1]]);
                SLOW_PATH_SHARED_WRITE(STM_Heap[index2[1]],second_account+100);
                
                first_account= SLOW_PATH_SHARED_READ(STM_Heap[index[2]]);
                SLOW_PATH_SHARED_WRITE(STM_Heap[index[2]],first_account-100);
                second_account = SLOW_PATH_SHARED_READ(STM_Heap[index2[2]]);
                SLOW_PATH_SHARED_WRITE(STM_Heap[index2[2]],second_account+100);
    }
    TM_END();
    } 
        if(mode == 0){
            printf("HTM transaction %d finished\n", numThread);
        }    
        if(mode == 1){
            printf("STM transaction %d finished\n", numThread);
        }    
         
    TM_THREAD_EXIT();
             
   return NULL;      
}
