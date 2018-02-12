//#include "rapl.h"
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "computeGraph.h"
#include "defs.h"
#include "findSubGraphs.h"
#include "genScalData.h"
#include "getUserParameters.h"
#include "globals.h"
#include "timer.h"
#include "thread.h"
#include "tm.h"
#include "random.h"



MAIN(argc, argv)
{
    GOTO_REAL();

    SETUP_NUMBER_TASKS(10);

    /*
     * Tuple for Scalable Data Generation
     * stores startVertex, endVertex, long weight and other info
     */
    graphSDG* SDGdata_a;
		#ifdef DEP
    graphSDG* SDGdata_b;
		#endif

    /*
     * The graph data structure for this benchmark - see defs.h
     */
    graph* G_a;
		#ifdef DEP
    graph* G_b;
		#endif


    double totalTime = 0.0;

    /* -------------------------------------------------------------------------
     * Preamble
     * -------------------------------------------------------------------------
     */

    /*
     * User Interface: Configurable parameters, and global program control
     */

    getUserParameters(argc, (char** const) argv);

		#ifdef DEP
    SIM_GET_NUM_CPU(THREADS*2);
    TM_STARTUP(THREADS*2, SSCA2_ID);
    P_MEMORY_STARTUP(THREADS*2);
    SETUP_NUMBER_THREADS(THREADS*2);
    thread_startup(THREADS*2);
		#else
		SIM_GET_NUM_CPU(THREADS);
    TM_STARTUP(THREADS, SSCA2_ID);
    P_MEMORY_STARTUP(THREADS);
    SETUP_NUMBER_THREADS(THREADS);
    thread_startup(THREADS);
		#endif

double time_total = 0.0;
//int repeat = REPEATS;

//for (; repeat > 0; --repeat) {


		

    SDGdata_a = (graphSDG*)malloc(sizeof(graphSDG));
    assert(SDGdata_a);
    genScalData_seq(SDGdata_a);
    #ifdef DEP
    SDGdata_b = (graphSDG*)malloc(sizeof(graphSDG));
    assert(SDGdata_b);
		genScalData_seq(SDGdata_b);
    #endif

    G_a = (graph*)malloc(sizeof(graph));
    assert(G_a);
    computeGraph_arg_t computeGraphArgs;
    computeGraphArgs.GPtr_a       = G_a;
    computeGraphArgs.SDGdataPtr_a = SDGdata_a;
    #ifdef DEP
    G_b = (graph*)malloc(sizeof(graph));
    assert(G_b);
    computeGraphArgs.GPtr_b       = G_b;
    computeGraphArgs.SDGdataPtr_b = SDGdata_b;
    #endif


TIMER_T start;
    TIMER_READ(start);

    GOTO_SIM();
    thread_start(computeGraph, (void*)&computeGraphArgs);
    GOTO_REAL();
TIMER_T stop;
    TIMER_READ(stop);
double time_tmp = TIMER_DIFF_SECONDS(start, stop);
//printf("summary\t%lf\t%lf\n", time_tmp);
PRINT_STATS();
//time_total += time_tmp;

//}

//totalTime += time_total;

    printf("\nTime taken for all is %9.6f sec.\n\n", time_tmp);

    /* -------------------------------------------------------------------------
     * Cleanup
     * -------------------------------------------------------------------------
     */

    P_FREE(G_a->outDegree);
    P_FREE(G_a->outVertexIndex);
    P_FREE(G_a->outVertexList);
    P_FREE(G_a->paralEdgeIndex);
    P_FREE(G_a->inDegree);
    P_FREE(G_a->inVertexIndex);
    P_FREE(G_a->inVertexList);
    P_FREE(G_a->intWeight);
    P_FREE(G_a->strWeight);

    P_FREE(SOUGHT_STRING_a);
    P_FREE(G_a);
    P_FREE(SDGdata_a);

		#ifdef DEP
    P_FREE(G_b->outDegree);
    P_FREE(G_b->outVertexIndex);
    P_FREE(G_b->outVertexList);
    P_FREE(G_b->paralEdgeIndex);
    P_FREE(G_b->inDegree);
    P_FREE(G_b->inVertexIndex);
    P_FREE(G_b->inVertexList);
    P_FREE(G_b->intWeight);
    P_FREE(G_b->strWeight);

    P_FREE(SOUGHT_STRING_b);
    P_FREE(G_b);
    P_FREE(SDGdata_b);
		#endif

    TM_SHUTDOWN();
    P_MEMORY_SHUTDOWN();

    GOTO_SIM();

    thread_shutdown();

    MAIN_RETURN(0);
}
