// Copyright 2008,2009,2010 Massachusetts Institute of Technology.
// All rights reserved. Use of this source code is governed by a
// BSD-style license that can be found in the LICENSE file.

#define __STDC_FORMAT_MACROS
#include <climits>
#include <cstdio>
#include <inttypes.h>
#include <getopt.h>
#include <pthread.h>

#include "clock.h"
#include "randomgenerator.h"
#include "tpccclient.h"
#include "tpccgenerator.h"
#include "tpcctables.h"
#include "tm.h"
#include "random.h"

#include <sys/mman.h>
#include <fcntl.h>    /* For O_RDWR */
#include <unistd.h> 

#define DEFAULT_STOCK_LEVEL_TXS_RATIO       4
#define DEFAULT_DELIVERY_TXS_RATIO          4
#define DEFAULT_ORDER_STATUS_TXS_RATIO      4
#define DEFAULT_PAYMENT_TXS_RATIO           43
#define DEFAULT_NEW_ORDER_TXS_RATIO         45
#define DEFAULT_NUMBER_WAREHOUSES           1
#define DEFAULT_TIME_SECONDS                10
#define DEFAULT_NUM_CLIENTS					1

int duration_secs;

__attribute__((aligned(CACHE_LINE_SIZE))) padded_float_t* balances;
__attribute__((aligned(CACHE_LINE_SIZE))) padded_float_t* client_balances;
__attribute__((aligned(CACHE_LINE_SIZE))) padded_scalar_t* client_balances_cnt;
__attribute__((aligned(CACHE_LINE_SIZE))) padded_scalar_t* client_delivery_cnt;
__attribute__((aligned(CACHE_LINE_SIZE))) padded_float_t* district_balances;
__attribute__((aligned(CACHE_LINE_SIZE))) padded_float_t* customer_balances;

void* client(void *data) {
	TM_THREAD_ENTER();
    // FIXME(nmld): you may call some code to init the worker TM thread here
    //

    TPCCClient* client = (TPCCClient*)((TPCCClient**) data)[threadID];
    SystemClock* clock = new SystemClock();
    int64_t begin = clock->getMicroseconds();


    do {
        client->doOne(TM_ARG_ALONE);
	//printf("Tx executed");
    } while (((clock->getMicroseconds() - begin) / 1000000) < duration_secs);
	TM_THREAD_EXIT(); 
}



int main(int argc, char** argv) {



    if (argc < 9) {
        printf("Please provide all the minimum parameters\n");
        exit(1);
    }

    struct option long_options[] = {
      // These options don't set a flag
      {"stockLevel transactions ratio",     required_argument, NULL, 's'},
      {"delivery transactions ratio",       required_argument, NULL, 'd'},
      {"order status transactions ratio",   required_argument, NULL, 'o'},
      {"payment transactions ratio",        required_argument, NULL, 'p'},
      {"new order txs ratio",               required_argument, NULL, 'r'},
      {"number warehouses",                 required_argument, NULL, 'w'},
      {"duration in seconds",               required_argument, NULL, 't'},
      {"number of clients",		            required_argument, NULL, 'n'},
      {"workload change",                   required_argument, NULL, 'c'},
      {"maximum number of warehouses",      required_argument, NULL, 'm'},
      {NULL, 0, NULL, 0}
    };

    global_stock_level_txs_ratio = DEFAULT_STOCK_LEVEL_TXS_RATIO;
    global_delivery_txs_ratio = DEFAULT_DELIVERY_TXS_RATIO;
    global_order_status_txs_ratio = DEFAULT_ORDER_STATUS_TXS_RATIO;
    global_payment_txs_ratio = DEFAULT_PAYMENT_TXS_RATIO;
    global_new_order_ratio = DEFAULT_NEW_ORDER_TXS_RATIO;
    int num_warehouses = DEFAULT_NUMBER_WAREHOUSES;
    duration_secs = DEFAULT_TIME_SECONDS;
    int num_clients = DEFAULT_NUM_CLIENTS;
	int fd=-1;	
    
    
    MB = 1048576;
    PAGE_SIZE = getpagesize();
    NUM_PAGES = 20000000;
    NUM_ELEMENTS_PAGE = PAGE_SIZE/sizeof(long);
    NUM_ELEMENTS_ARRAY =  NUM_PAGES*NUM_ELEMENTS_PAGE;
    
    fd = shm_open("/shared_arraypedro", O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
    if (fd == -1) 
        printf("FAILURE CREATING SHARED MEMORY\n");

    if (ftruncate(fd, sizeof(long) * NUM_ELEMENTS_ARRAY) == -1) 
        printf("FAILURE TRUNCATE\n");

    //Map chunk of 1MB memory to private address space
    HTM_Heap = (char*)mmap (NULL, sizeof(long) * NUM_ELEMENTS_ARRAY, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0); 
    STM_Heap = (char*)mmap (NULL, sizeof(long) * NUM_ELEMENTS_ARRAY, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0); 

    //printf("MMAP issued\n");
    //Deal with failure on the mapping
    if(HTM_Heap == MAP_FAILED)
        printf("MAP_FAILED\n");

    if(STM_Heap == MAP_FAILED)
         printf("MAP_FAILED\n");


    // If "-c" is found, then we start parsing the parameters into the workload_changes.
    // The argument of each "-c" is the number of seconds that it lasts.
    std::vector<int> workload_changes;
    int adapt_workload = 0;

    int i, c;
    while(1) {
        i = 0;
        c = getopt_long(argc, argv, "s:d:o:p:r:w:t:n:c:m:", long_options, &i);
        if(c == -1)
          break;

        if(c == 0 && long_options[i].flag == 0)
          c = long_options[i].val;

        switch(c) {
         case 'c':
            adapt_workload = 1;
            workload_changes.push_back(atoi(optarg));
          break;
         case 's':
             workload_changes.push_back(atoi(optarg));
           break;
         case 'd':
             workload_changes.push_back(atoi(optarg));
           break;
         case 'o':
             workload_changes.push_back(atoi(optarg));
           break;
         case 'p':
             workload_changes.push_back(atoi(optarg));
           break;
         case 'r':
             workload_changes.push_back(atoi(optarg));
           break;
         case 'w':
             workload_changes.push_back(atoi(optarg));
           break;
         case 'm':
             num_warehouses = atoi(optarg);
             break;
         case 't':
             duration_secs = atoi(optarg);
           break;
         case 'n':
        	 num_clients = atoi(optarg);
        	 break;
         default:
           printf("Incorrect argument! :(\n");
           exit(1);
        }
      }




    SystemClock* clock = new SystemClock();

    // Create a generator for filling the database.
    tpcc::RealRandomGenerator* random = new tpcc::RealRandomGenerator();
    tpcc::NURandC cLoad = tpcc::NURandC::makeRandom(random);
    random->setC(cLoad);


	SIM_GET_NUM_CPU(num_clients);
        TM_STARTUP(num_clients,42);
        P_MEMORY_STARTUP(num_clients);
        thread_startup(num_clients);

	TM_THREAD_ENTER();


    TPCCTables* tables2 = (TPCCTables*)TM_MALLOC(sizeof(TPCCTables));
    TPCCTables* tables = new(tables2) TPCCTables();

    printf("page size is: %ld\n",getpagesize());

    TM_MALLOC(getpagesize());
    balances = (padded_float_t*) TM_MALLOC(sizeof(padded_float_t)*10);
    TM_MALLOC(getpagesize());
    client_balances = (padded_float_t*) TM_MALLOC(sizeof(padded_float_t)*300000);
    TM_MALLOC(getpagesize());
    client_balances_cnt = (padded_scalar_t*) TM_MALLOC(sizeof(padded_scalar_t) * 300000);
    TM_MALLOC(getpagesize());
    client_delivery_cnt = (padded_scalar_t*) TM_MALLOC(sizeof(padded_scalar_t) * 300000);
    TM_MALLOC(getpagesize());
    district_balances = (padded_float_t*) TM_MALLOC(sizeof(padded_float_t)*100);
    TM_MALLOC(getpagesize());
    customer_balances = (padded_float_t*) TM_MALLOC(sizeof(padded_float_t)*300000);
    TM_MALLOC(getpagesize());


    // Generate the data
    printf("Loading %ld warehouses... ", num_warehouses);
    fflush(stdout);
    char now[Clock::DATETIME_SIZE+1];
    clock->getDateTimestamp(now);
	printf("num items: %d", Item::NUM_ITEMS);
    int64_t begin = clock->getMicroseconds();
    int ro = 1;
    htm_retries = 0;
    local_exec_mode = 2;
   	if(local_exec_mode==1 || local_exec_mode==3){
    	SLOW_PATH_BEGIN(0,ro);
  	}else{
        FAST_PATH_BEGIN(0,ro);
    }
    TPCCGenerator generator(random, now, Item::NUM_ITEMS, District::NUM_PER_WAREHOUSE,
            Customer::NUM_PER_DISTRICT, NewOrder::INITIAL_NUM_PER_DISTRICT);
    generator.makeItemsTable(TM_ARG tables);
    for (int i = 0; i < num_warehouses; ++i) {
        generator.makeWarehouse(TM_ARG tables, i+1);
    }

	if(local_exec_mode==1){
        SLOW_PATH_END();
    }else{
        FAST_PATH_END();
    }
    htm_retries = 1000;
    int64_t end = clock->getMicroseconds();
    printf("%ld ms\n", (end - begin + 500)/1000);
    
    //warehouse_balances = (Warehouse_balance)TM_MALLOC(sizeof(Warehouse_balance)* num_warehouses);

    // Client owns all the parameters
    TPCCClient** clients = (TPCCClient**) TM_MALLOC(num_clients * sizeof(TPCCClient*));
    pthread_t* threads = (pthread_t*) TM_MALLOC(num_clients * sizeof(pthread_t));
    for (c = 0; c < num_clients; c++) {
	 // Change the constants for run
	 random = new tpcc::RealRandomGenerator();
	 random->setC(tpcc::NURandC::makeRandomForRun(random, cLoad));
        TPCCClient* client_aux = (TPCCClient*)TM_MALLOC(sizeof(TPCCClient));
        clients[c] = new(client_aux) TPCCClient(clock, random, tables, Item::NUM_ITEMS, static_cast<int>(num_warehouses),
                District::NUM_PER_WAREHOUSE, Customer::NUM_PER_DISTRICT);

    }

    int64_t next_workload_secs;
    uint64_t pos_vec = 0;
    if (adapt_workload) {
        next_workload_secs = workload_changes[pos_vec++];
    } else {
        next_workload_secs = duration_secs;
    }

    global_num_warehouses = workload_changes[pos_vec++];
    global_stock_level_txs_ratio = workload_changes[pos_vec++];
    global_delivery_txs_ratio = workload_changes[pos_vec++];
    global_order_status_txs_ratio = workload_changes[pos_vec++];
    global_payment_txs_ratio = workload_changes[pos_vec++];
    global_new_order_ratio = workload_changes[pos_vec++];

    printf("Running with the following parameters for %ld secs: (max warehouses %d)\n", next_workload_secs, num_warehouses);
    printf("\tWarehouses     (-w): %d\n", global_num_warehouses);
    printf("\tStockLevel ratio   (-s): %d\n", global_stock_level_txs_ratio);
    printf("\tDelivery ratio     (-d): %d\n", global_delivery_txs_ratio);
    printf("\tOrder Status ratio (-o): %d\n", global_order_status_txs_ratio);
    printf("\tPayment ratio      (-p): %d\n", global_payment_txs_ratio);
    printf("\tNewOrder ratio     (-r): %d\n", global_new_order_ratio);

    int sum = global_stock_level_txs_ratio + global_delivery_txs_ratio + global_order_status_txs_ratio
            + global_payment_txs_ratio + global_new_order_ratio;
    if (sum != 100) {
        printf("==== ERROR: the sum of the ratios of tx types does not match 100: %d\n", sum);
        exit(1);
    }
    if (global_num_warehouses > num_warehouses) {
        printf("==== ERROR: the number of warehouses is too large\n");
        exit(1);
    }

    //TM_STARTUP(num_clients,42);
/*
	TM_THREAD_EXIT();
P_MEMORY_SHUTDOWN();
  GOTO_SIM();
  thread_shutdown();
  */
//TM_SHUTDOWN();


/*
SIM_GET_NUM_CPU(num_clients);
        TM_STARTUP(num_clients,42);
        P_MEMORY_STARTUP(num_clients);
        thread_startup(num_clients);
*/
    printf("Running... ");
    fflush(stdout);
    begin = clock->getMicroseconds();
    //for (c = 0; c < num_clients; c++) {
    //	pthread_create(&threads[c], NULL, client, clients[c]);
    //}
    thread_start(client, clients);



/*    for (c = 0; c < num_clients; c++) {
    	pthread_join(threads[c], NULL);
    }*/


    end = clock->getMicroseconds();
    int64_t microseconds = end - begin;


    unsigned long executed_stock_level_txs = 0;
    unsigned long executed_delivery_txs = 0;
    unsigned long executed_order_status_txs = 0;
    unsigned long executed_payment_txs = 0;
    unsigned long executed_new_order_txs = 0;

    for (c = 0; c < num_clients; c++) {
        executed_stock_level_txs += clients[c]->executed_stock_level_txs_;
        executed_delivery_txs += clients[c]->executed_delivery_txs_;
        executed_order_status_txs += clients[c]->executed_order_status_txs_;
        executed_payment_txs += clients[c]->executed_payment_txs_;
        executed_new_order_txs += clients[c]->executed_new_order_txs_;
    }

    double sum_txs_exec = executed_stock_level_txs + executed_delivery_txs + executed_order_status_txs
            + executed_payment_txs + executed_new_order_txs;
    printf("\nExecuted the following txs types:\n");
    printf("\tStockLevel : %.2f\t%lu\n", (executed_stock_level_txs / sum_txs_exec), executed_stock_level_txs);
    printf("\tDelivery   : %.2f\t%lu\n", (executed_delivery_txs / sum_txs_exec), executed_delivery_txs);
    printf("\tOrderStatus: %.2f\t%lu\n", (executed_order_status_txs / sum_txs_exec), executed_order_status_txs);
    printf("\tPayment    : %.2f\t%lu\n", (executed_payment_txs / sum_txs_exec), executed_payment_txs);
    printf("\tNewOrder   : %.2f\t%lu\n", (executed_new_order_txs / sum_txs_exec), executed_new_order_txs);

    printf("%ld transactions in %ld ms = %.2f txns/s\n", (long)sum_txs_exec,
            (microseconds + 500)/1000, sum_txs_exec / (double) microseconds * 1000000.0);
    printf("Time Elapsed: %.3f\n", (microseconds / 1000000.0));
    printf ("Throughput: %.2f\n",sum_txs_exec / (double) microseconds * 1000000.0);

    printf("Txs: %ld\n", (long)sum_txs_exec);

	//Unmap memory chunk
    munmap (HTM_Heap,sizeof(long) * NUM_ELEMENTS_ARRAY);
    munmap (STM_Heap,sizeof(long) * NUM_ELEMENTS_ARRAY); 

    //Close shared object
    shm_unlink("/shared_arraypedro");

 P_MEMORY_SHUTDOWN();
  GOTO_SIM();
  thread_shutdown();
	TM_SHUTDOWN();

    return 0;
}
