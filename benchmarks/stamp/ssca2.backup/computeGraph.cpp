/* =============================================================================
 *
 * computeGraph.c
 *
 * =============================================================================
 *
 * For the license of bayes/sort.h and bayes/sort.c, please see the header
 * of the files.
 *
 * ------------------------------------------------------------------------
 *
 * For the license of kmeans, please see kmeans/LICENSE.kmeans
 *
 * ------------------------------------------------------------------------
 *
 * For the license of ssca2, please see ssca2/COPYRIGHT
 *
 * ------------------------------------------------------------------------
 *
 * For the license of lib/mt19937ar.c and lib/mt19937ar.h, please see the
 * header of the files.
 *
 * ------------------------------------------------------------------------
 *
 * For the license of lib/rbtree.h and lib/rbtree.c, please see
 * lib/LEGALNOTICE.rbtree and lib/LICENSE.rbtree
 *
 * ------------------------------------------------------------------------
 *
 * Unless otherwise noted, the following license applies to STAMP files:
 *
 * Copyright (c) 2007, Stanford University
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in
 *       the documentation and/or other materials provided with the
 *       distribution.
 *
 *     * Neither the name of Stanford University nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY STANFORD UNIVERSITY ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL STANFORD UNIVERSITY BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 *
 * =============================================================================
 */


#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "computeGraph.h"
#include "createPartition.h"
#include "defs.h"
#include "globals.h"
#include "thread.h"
#include "utility.h"
#include "tm.h"

static ULONGINT_T*  global_p_a                 = NULL;
static ULONGINT_T   global_maxNumVertices_a    = 0;
static ULONGINT_T   global_outVertexListSize_a = 0;
static ULONGINT_T*  global_impliedEdgeList_a   = NULL;
static ULONGINT_T** global_auxArr_a            = NULL;
#ifdef DEP
static ULONGINT_T*  global_p_b                 = NULL;
static ULONGINT_T   global_maxNumVertices_b    = 0;
static ULONGINT_T   global_outVertexListSize_b = 0;
static ULONGINT_T*  global_impliedEdgeList_b   = NULL;
static ULONGINT_T** global_auxArr_b            = NULL;
#endif


/* =============================================================================
 * prefix_sums
 * =============================================================================
 */
static void
prefix_sums_a (ULONGINT_T* result, LONGINT_T* input, ULONGINT_T arraySize)
{
    long myId = local_app_id;
    long numThread = local_num_threads;
    ULONGINT_T* p = NULL;
    if (myId == 0) {
        p = (ULONGINT_T*)P_MALLOC(NOSHARE(numThread) * sizeof(ULONGINT_T));
        assert(p);
        global_p_a = p;
    }


		#ifdef DEP
    thread_barrier_wait_a();
    #else
    thread_barrier_wait();
    #endif


    p = global_p_a;

    long start;
    long end;

    long r = arraySize / numThread;
    start = myId * r + 1;
    end = (myId + 1) * r;
    if (myId == (numThread - 1)) {
        end = arraySize;
    }

    ULONGINT_T j;
    for (j = start; j < end; j++) {
        result[j] = input[j-1] + result[j-1];
    }

    p[NOSHARE(myId)] = result[end-1];

		#ifdef DEP
    thread_barrier_wait_a();
    #else
    thread_barrier_wait();
    #endif


    if (myId == 0) {
        for (j = 1; j < numThread; j++) {
            p[NOSHARE(j)] += p[NOSHARE(j-1)];
        }
    }

		#ifdef DEP
    thread_barrier_wait_a();
    #else
    thread_barrier_wait();
    #endif


    if (myId > 0) {
        ULONGINT_T add_value = p[NOSHARE(myId-1)];
        for (j = start-1; j < end; j++) {
            result[j] += add_value;
        }
    }


		#ifdef DEP
    thread_barrier_wait_a();
    #else
    thread_barrier_wait();
    #endif


    if (myId == 0) {
        P_FREE(p);
    }
}
#ifdef DEP
static void
prefix_sums_b (ULONGINT_T* result, LONGINT_T* input, ULONGINT_T arraySize)
{
    long myId = local_app_id;
    long numThread = local_num_threads;
    ULONGINT_T* p = NULL;
    if (myId == 0) {
        p = (ULONGINT_T*)P_MALLOC(NOSHARE(numThread) * sizeof(ULONGINT_T));
        assert(p);
        global_p_b = p;
    }


    thread_barrier_wait_b();


    p = global_p_b;

    long start;
    long end;

    long r = arraySize / numThread;
    start = myId * r + 1;
    end = (myId + 1) * r;
    if (myId == (numThread - 1)) {
        end = arraySize;
    }

    ULONGINT_T j;
    for (j = start; j < end; j++) {
        result[j] = input[j-1] + result[j-1];
    }

    p[NOSHARE(myId)] = result[end-1];

    thread_barrier_wait_b();


    if (myId == 0) {
        for (j = 1; j < numThread; j++) {
            p[NOSHARE(j)] += p[NOSHARE(j-1)];
        }
    }

    thread_barrier_wait_b();


    if (myId > 0) {
        ULONGINT_T add_value = p[NOSHARE(myId-1)];
        for (j = start-1; j < end; j++) {
            result[j] += add_value;
        }
    }


    thread_barrier_wait_b();


    if (myId == 0) {
        P_FREE(p);
    }
}
#endif

/* =============================================================================
 * computeGraph
 * =============================================================================
 */

void _computeGraph_a(graph* GPtr, graphSDG* SDGdataPtr){
	long numThread = local_num_threads;
    long myId = local_app_id;


    ULONGINT_T j;
    ULONGINT_T maxNumVertices = 0;
    ULONGINT_T numEdgesPlaced = SDGdataPtr->numEdgesPlaced;

    /*
     * First determine the number of vertices by scanning the tuple
     * startVertex list
     */

    long i;
    long i_start;
    long i_stop;
    createPartition(0, numEdgesPlaced, myId, numThread, &i_start, &i_stop);
    for (i = i_start; i < i_stop; i++) {
        if (SDGdataPtr->startVertex[i] > maxNumVertices) {
            maxNumVertices = SDGdataPtr->startVertex[i];
        }
    }

    int ro = 0;
    TM_BEGIN(0);
        long tmp_maxNumVertices = (long)FAST_PATH_SHARED_READ(global_maxNumVertices_a);
        long new_maxNumVertices = MAX(tmp_maxNumVertices, maxNumVertices) + 1;
        FAST_PATH_SHARED_WRITE(global_maxNumVertices_a, new_maxNumVertices);
    TM_END();

		#ifdef DEP
    thread_barrier_wait_a();
		#else
		thread_barrier_wait();
		#endif

    maxNumVertices = global_maxNumVertices_a;

    if (myId == 0) {

        GPtr->numVertices = maxNumVertices;
        GPtr->numEdges    = numEdgesPlaced;
        GPtr->intWeight   = SDGdataPtr->intWeight;
        GPtr->strWeight   = SDGdataPtr->strWeight;

        for (i = 0; i < numEdgesPlaced; i++) {
            if (GPtr->intWeight[numEdgesPlaced-i-1] < 0) {
                GPtr->numStrEdges = -(GPtr->intWeight[numEdgesPlaced-i-1]) + 1;
                GPtr->numIntEdges = numEdgesPlaced - GPtr->numStrEdges;
                break;
            }
        }

        GPtr->outDegree =
            (LONGINT_T*)P_MALLOC((GPtr->numVertices) * sizeof(LONGINT_T));
        assert(GPtr->outDegree);

        GPtr->outVertexIndex =
            (ULONGINT_T*)P_MALLOC((GPtr->numVertices) * sizeof(ULONGINT_T));
        assert(GPtr->outVertexIndex);
    }

		#ifdef DEP
    thread_barrier_wait_a();
    #else
    thread_barrier_wait();
    #endif


    createPartition(0, GPtr->numVertices, myId, numThread, &i_start, &i_stop);

    for (i = i_start; i < i_stop; i++) {
        GPtr->outDegree[i] = 0;
        GPtr->outVertexIndex[i] = 0;
    }

    ULONGINT_T outVertexListSize = 0;

		#ifdef DEP
    thread_barrier_wait_a();
    #else
    thread_barrier_wait();
    #endif


    ULONGINT_T i0 = -1UL;

    for (i = i_start; i < i_stop; i++) {

        ULONGINT_T k = i;
        if ((outVertexListSize == 0) && (k != 0)) {
            while (i0 == -1UL) {
                for (j = 0; j < numEdgesPlaced; j++) {
                    if (k == SDGdataPtr->startVertex[j]) {
                        i0 = j;
                        break;
                    }

                }
                k--;
            }
        }

        if ((outVertexListSize == 0) && (k == 0)) {
            i0 = 0;
        }

        for (j = i0; j < numEdgesPlaced; j++) {
            if (i == GPtr->numVertices-1) {
                break;
            }
            if ((i != SDGdataPtr->startVertex[j])) {
                if ((j > 0) && (i == SDGdataPtr->startVertex[j-1])) {
                    if (j-i0 >= 1) {
                        outVertexListSize++;
                        GPtr->outDegree[i]++;
                        ULONGINT_T t;
                        for (t = i0+1; t < j; t++) {
                            if (SDGdataPtr->endVertex[t] !=
                                SDGdataPtr->endVertex[t-1])
                            {
                                outVertexListSize++;
                                GPtr->outDegree[i] = GPtr->outDegree[i]+1;
                            }
                        }
                    }
                }
                i0 = j;
                break;
            }
        }

        if (i == GPtr->numVertices-1) {
            if (numEdgesPlaced-i0 >= 0) {
                outVertexListSize++;
                GPtr->outDegree[i]++;
                ULONGINT_T t;
                for (t = i0+1; t < numEdgesPlaced; t++) {
                    if (SDGdataPtr->endVertex[t] != SDGdataPtr->endVertex[t-1]) {
                        outVertexListSize++;
                        GPtr->outDegree[i]++;
                    }
                }
            }
        }

    } /* for i */

		#ifdef DEP
    thread_barrier_wait_a();
    #else
    thread_barrier_wait();
    #endif


    prefix_sums_a(GPtr->outVertexIndex, GPtr->outDegree, GPtr->numVertices);

  	#ifdef DEP
    thread_barrier_wait_a();
    #else
    thread_barrier_wait();
    #endif

	  TM_BEGIN(0);
				long temp = (long)FAST_PATH_SHARED_READ(global_outVertexListSize_a);
				temp += outVertexListSize;
        FAST_PATH_SHARED_WRITE(
            global_outVertexListSize_a,
            temp);
    TM_END();


		#ifdef DEP
    thread_barrier_wait_a();
    #else
    thread_barrier_wait();
    #endif

    outVertexListSize = global_outVertexListSize_a;

    if (myId == 0) {
        GPtr->numDirectedEdges = outVertexListSize;
        GPtr->outVertexList =
            (ULONGINT_T*)P_MALLOC(outVertexListSize * sizeof(ULONGINT_T));
        assert(GPtr->outVertexList);
        GPtr->paralEdgeIndex =
            (ULONGINT_T*)P_MALLOC(outVertexListSize * sizeof(ULONGINT_T));
        assert(GPtr->paralEdgeIndex);
        GPtr->outVertexList[0] = SDGdataPtr->endVertex[0];
    }


		#ifdef DEP
    thread_barrier_wait_a();
    #else
    thread_barrier_wait();
    #endif

    /*
     * Evaluate outVertexList
     */

    i0 = -1UL;

    for (i = i_start; i < i_stop; i++) {

        ULONGINT_T k = i;
        while ((i0 == -1UL) && (k != 0)) {
            for (j = 0; j < numEdgesPlaced; j++) {
                if (k == SDGdataPtr->startVertex[j]) {
                    i0 = j;
                    break;
                }
            }
            k--;
        }

        if ((i0 == -1) && (k == 0)) {
            i0 = 0;
        }

        for (j = i0; j < numEdgesPlaced; j++) {
            if (i == GPtr->numVertices-1) {
                break;
            }
            if (i != SDGdataPtr->startVertex[j]) {
                if ((j > 0) && (i == SDGdataPtr->startVertex[j-1])) {
                    if (j-i0 >= 1) {
                        long ii = GPtr->outVertexIndex[i];
                        ULONGINT_T r = 0;
                        GPtr->paralEdgeIndex[ii] = i0;
                        GPtr->outVertexList[ii] = SDGdataPtr->endVertex[i0];
                        r++;
                        ULONGINT_T t;
                        for (t = i0+1; t < j; t++) {
                            if (SDGdataPtr->endVertex[t] !=
                                SDGdataPtr->endVertex[t-1])
                            {
                                GPtr->paralEdgeIndex[ii+r] = t;
                                GPtr->outVertexList[ii+r] = SDGdataPtr->endVertex[t];
                                r++;
                            }
                        }

                    }
                }
                i0 = j;
                break;
            }
        } /* for j */

        if (i == GPtr->numVertices-1) {
            ULONGINT_T r = 0;
            if (numEdgesPlaced-i0 >= 0) {
                long ii = GPtr->outVertexIndex[i];
                GPtr->paralEdgeIndex[ii+r] = i0;
                GPtr->outVertexList[ii+r] = SDGdataPtr->endVertex[i0];
                r++;
                ULONGINT_T t;
                for (t = i0+1; t < numEdgesPlaced; t++) {
                    if (SDGdataPtr->endVertex[t] != SDGdataPtr->endVertex[t-1]) {
                        GPtr->paralEdgeIndex[ii+r] = t;
                        GPtr->outVertexList[ii+r] = SDGdataPtr->endVertex[t];
                        r++;
                    }
                }
            }
        }

    } /* for i */


		#ifdef DEP
    thread_barrier_wait_a();
    #else
    thread_barrier_wait();
    #endif

		printf("thread id is %d, %d\n",local_thread_id, local_app_id); 
    if (myId == 0) {
        P_FREE(SDGdataPtr->startVertex);
        P_FREE(SDGdataPtr->endVertex);
        GPtr->inDegree =
            (LONGINT_T*)P_MALLOC(GPtr->numVertices * sizeof(LONGINT_T));
        assert(GPtr->inDegree);
        GPtr->inVertexIndex =
            (ULONGINT_T*)P_MALLOC(GPtr->numVertices * sizeof(ULONGINT_T));
        assert(GPtr->inVertexIndex);
    }

		#ifdef DEP
    thread_barrier_wait_a();
    #else
    thread_barrier_wait();
    #endif
	

    for (i = i_start; i < i_stop; i++) {
        GPtr->inDegree[i] = 0;
        GPtr->inVertexIndex[i] = 0;
    }

    /* A temp. array to store the inplied edges */
    ULONGINT_T* impliedEdgeList;
    if (myId == 0) {
        impliedEdgeList = (ULONGINT_T*)P_MALLOC(GPtr->numVertices
                                                * MAX_CLUSTER_SIZE
                                                * sizeof(ULONGINT_T));
        global_impliedEdgeList_a = impliedEdgeList;
    }

		#ifdef DEP
    thread_barrier_wait_a();
    #else
    thread_barrier_wait();
    #endif


    impliedEdgeList = global_impliedEdgeList_a;

    createPartition(0,
                    (GPtr->numVertices * MAX_CLUSTER_SIZE),
                    myId,
                    numThread,
                    &i_start,
                    &i_stop);

    for (i = i_start; i < i_stop; i++) {
        impliedEdgeList[i] = 0;
    }

    /*
     * An auxiliary array to store implied edges, in case we overshoot
     * MAX_CLUSTER_SIZE
     */

    ULONGINT_T** auxArr;
    if (myId == 0) {
        auxArr = (ULONGINT_T**)P_MALLOC(GPtr->numVertices * sizeof(ULONGINT_T*));
        assert(auxArr);
        global_auxArr_a = auxArr;
    }

		#ifdef DEP
    thread_barrier_wait_a();
    #else
    thread_barrier_wait();
    #endif

    auxArr = global_auxArr_a;

    createPartition(0, GPtr->numVertices, myId, numThread, &i_start, &i_stop);

    for (i = i_start; i < i_stop; i++) {
        /* Inspect adjacency list of vertex i */
        for (j = GPtr->outVertexIndex[i];
             j < (GPtr->outVertexIndex[i] + GPtr->outDegree[i]);
             j++)
        {
            ULONGINT_T v = GPtr->outVertexList[j];
            ULONGINT_T k;
            for (k = GPtr->outVertexIndex[v];
                 k < (GPtr->outVertexIndex[v] + GPtr->outDegree[v]);
                 k++)
            {
                if (GPtr->outVertexList[k] == i) {
                    break;
                }
            }
            if (k == GPtr->outVertexIndex[v]+GPtr->outDegree[v]) {
            	int ro = 0;
                TM_BEGIN(0);
                	/* Add i to the impliedEdgeList of v */
                	long inDegree = (long)FAST_PATH_SHARED_READ(GPtr->inDegree[v]);
									long temp = inDegree + 1;\
                	FAST_PATH_SHARED_WRITE(GPtr->inDegree[v], temp);
                	if (inDegree < MAX_CLUSTER_SIZE) {
                		FAST_PATH_SHARED_WRITE(impliedEdgeList[v*MAX_CLUSTER_SIZE+inDegree],
                				i);
                	} else {
                		/* Use auxiliary array to store the implied edge */
                		/* Create an array if it's not present already */
                		ULONGINT_T* a = NULL;
                		if ((inDegree % MAX_CLUSTER_SIZE) == 0) {
                			a = (ULONGINT_T*)TM_MALLOC(MAX_CLUSTER_SIZE
                					* sizeof(ULONGINT_T));
                			assert(a);
                			FAST_PATH_SHARED_WRITE_P(auxArr[v], a);
                		} else {
                			a = auxArr[v];
                		}
                		FAST_PATH_SHARED_WRITE(a[inDegree % MAX_CLUSTER_SIZE], i);
                	}
								TM_END(); 
            }
        }
    } /* for i */

		#ifdef DEP
    thread_barrier_wait_a();
    #else
    thread_barrier_wait();
    #endif

    prefix_sums_a(GPtr->inVertexIndex, GPtr->inDegree, GPtr->numVertices);

    if (myId == 0) {
        GPtr->numUndirectedEdges = GPtr->inVertexIndex[GPtr->numVertices-1]
                                   + GPtr->inDegree[GPtr->numVertices-1];
        GPtr->inVertexList =
            (ULONGINT_T *)P_MALLOC(GPtr->numUndirectedEdges * sizeof(ULONGINT_T));
    }

		#ifdef DEP
    thread_barrier_wait_a();
    #else
    thread_barrier_wait();
    #endif

    /*
     * Create the inVertex List
     */

    for (i = i_start; i < i_stop; i++) {
        for (j = GPtr->inVertexIndex[i];
             j < (GPtr->inVertexIndex[i] + GPtr->inDegree[i]);
             j++)
        {
            if ((j - GPtr->inVertexIndex[i]) < MAX_CLUSTER_SIZE) {
                GPtr->inVertexList[j] =
                    impliedEdgeList[i*MAX_CLUSTER_SIZE+j-GPtr->inVertexIndex[i]];
            } else {
                GPtr->inVertexList[j] =
                    auxArr[i][(j-GPtr->inVertexIndex[i]) % MAX_CLUSTER_SIZE];
            }
        }
    }


		
		#ifdef DEP
    thread_barrier_wait_a();
    #else
    thread_barrier_wait();
    #endif

    if (myId == 0) {
        P_FREE(impliedEdgeList);
    }

    for (i = i_start; i < i_stop; i++) {
        if (GPtr->inDegree[i] > MAX_CLUSTER_SIZE) {
            P_FREE(auxArr[i]);
        }
    }

		#ifdef DEP
    thread_barrier_wait_a();
    #else
    thread_barrier_wait();
    #endif

    if (myId == 0) {
        P_FREE(auxArr);
    }

    TM_THREAD_EXIT();
}

#ifdef DEP
void _computeGraph_b(graph* GPtr, graphSDG* SDGdataPtr){
		long numThread = local_num_threads;
    long myId = local_app_id;


    ULONGINT_T j;
    ULONGINT_T maxNumVertices = 0;
    ULONGINT_T numEdgesPlaced = SDGdataPtr->numEdgesPlaced;

    /*
     * First determine the number of vertices by scanning the tuple
     * startVertex list
     */

    long i;
    long i_start;
    long i_stop;
    createPartition(0, numEdgesPlaced, myId, numThread, &i_start, &i_stop);
    for (i = i_start; i < i_stop; i++) {
        if (SDGdataPtr->startVertex[i] > maxNumVertices) {
            maxNumVertices = SDGdataPtr->startVertex[i];
        }
    }

    int ro = 0;
	local_exec_mode = 0;
    TM_BEGIN_B(0);
        long tmp_maxNumVertices = (long)FAST_PATH_SHARED_READ(global_maxNumVertices_b);
        long new_maxNumVertices = MAX(tmp_maxNumVertices, maxNumVertices) + 1;
        FAST_PATH_SHARED_WRITE(global_maxNumVertices_b, new_maxNumVertices);
    TM_END_B();


    thread_barrier_wait_b();

    maxNumVertices = global_maxNumVertices_b;

    if (myId == 0) {

        GPtr->numVertices = maxNumVertices;
        GPtr->numEdges    = numEdgesPlaced;
        GPtr->intWeight   = SDGdataPtr->intWeight;
        GPtr->strWeight   = SDGdataPtr->strWeight;

        for (i = 0; i < numEdgesPlaced; i++) {
            if (GPtr->intWeight[numEdgesPlaced-i-1] < 0) {
                GPtr->numStrEdges = -(GPtr->intWeight[numEdgesPlaced-i-1]) + 1;
                GPtr->numIntEdges = numEdgesPlaced - GPtr->numStrEdges;
                break;
            }
        }

        GPtr->outDegree =
            (LONGINT_T*)P_MALLOC((GPtr->numVertices) * sizeof(LONGINT_T));
        assert(GPtr->outDegree);

        GPtr->outVertexIndex =
            (ULONGINT_T*)P_MALLOC((GPtr->numVertices) * sizeof(ULONGINT_T));
        assert(GPtr->outVertexIndex);
    }


    thread_barrier_wait_b();

    createPartition(0, GPtr->numVertices, myId, numThread, &i_start, &i_stop);

    for (i = i_start; i < i_stop; i++) {
        GPtr->outDegree[i] = 0;
        GPtr->outVertexIndex[i] = 0;
    }

    ULONGINT_T outVertexListSize = 0;


    thread_barrier_wait_b();

    ULONGINT_T i0 = -1UL;

    for (i = i_start; i < i_stop; i++) {

        ULONGINT_T k = i;
        if ((outVertexListSize == 0) && (k != 0)) {
            while (i0 == -1UL) {
                for (j = 0; j < numEdgesPlaced; j++) {
                    if (k == SDGdataPtr->startVertex[j]) {
                        i0 = j;
                        break;
                    }

                }
                k--;
            }
        }

        if ((outVertexListSize == 0) && (k == 0)) {
            i0 = 0;
        }

        for (j = i0; j < numEdgesPlaced; j++) {
            if (i == GPtr->numVertices-1) {
                break;
            }
            if ((i != SDGdataPtr->startVertex[j])) {
                if ((j > 0) && (i == SDGdataPtr->startVertex[j-1])) {
                    if (j-i0 >= 1) {
                        outVertexListSize++;
                        GPtr->outDegree[i]++;
                        ULONGINT_T t;
                        for (t = i0+1; t < j; t++) {
                            if (SDGdataPtr->endVertex[t] !=
                                SDGdataPtr->endVertex[t-1])
                            {
                                outVertexListSize++;
                                GPtr->outDegree[i] = GPtr->outDegree[i]+1;
                            }
                        }
                    }
                }
                i0 = j;
                break;
            }
        }

        if (i == GPtr->numVertices-1) {
            if (numEdgesPlaced-i0 >= 0) {
                outVertexListSize++;
                GPtr->outDegree[i]++;
                ULONGINT_T t;
                for (t = i0+1; t < numEdgesPlaced; t++) {
                    if (SDGdataPtr->endVertex[t] != SDGdataPtr->endVertex[t-1]) {
                        outVertexListSize++;
                        GPtr->outDegree[i]++;
                    }
                }
            }
        }

    } /* for i */


    thread_barrier_wait_b();

    prefix_sums_b(GPtr->outVertexIndex, GPtr->outDegree, GPtr->numVertices);


    thread_barrier_wait_b();
	local_exec_mode = 0;
    TM_BEGIN_B(0);
				//long temp = (long)FAST_PATH_SHARED_READ(global_outVertexListSize);
				//temp += outVertexListSize; 
        FAST_PATH_SHARED_WRITE(
            global_outVertexListSize_b,
            ((long)FAST_PATH_SHARED_READ(global_outVertexListSize_b)+ outVertexListSize));
    TM_END_B();


    thread_barrier_wait_b();

    outVertexListSize = global_outVertexListSize_b;

    if (myId == 0) {
        GPtr->numDirectedEdges = outVertexListSize;
        GPtr->outVertexList =
            (ULONGINT_T*)P_MALLOC(outVertexListSize * sizeof(ULONGINT_T));
        assert(GPtr->outVertexList);
        GPtr->paralEdgeIndex =
            (ULONGINT_T*)P_MALLOC(outVertexListSize * sizeof(ULONGINT_T));
        assert(GPtr->paralEdgeIndex);
        GPtr->outVertexList[0] = SDGdataPtr->endVertex[0];
    }


    thread_barrier_wait_b();

    /*
     * Evaluate outVertexList
     */

    i0 = -1UL;

    for (i = i_start; i < i_stop; i++) {

        ULONGINT_T k = i;
        while ((i0 == -1UL) && (k != 0)) {
            for (j = 0; j < numEdgesPlaced; j++) {
                if (k == SDGdataPtr->startVertex[j]) {
                    i0 = j;
                    break;
                }
            }
            k--;
        }

        if ((i0 == -1) && (k == 0)) {
            i0 = 0;
        }

        for (j = i0; j < numEdgesPlaced; j++) {
            if (i == GPtr->numVertices-1) {
                break;
            }
            if (i != SDGdataPtr->startVertex[j]) {
                if ((j > 0) && (i == SDGdataPtr->startVertex[j-1])) {
                    if (j-i0 >= 1) {
                        long ii = GPtr->outVertexIndex[i];
                        ULONGINT_T r = 0;
                        GPtr->paralEdgeIndex[ii] = i0;
                        GPtr->outVertexList[ii] = SDGdataPtr->endVertex[i0];
                        r++;
                        ULONGINT_T t;
                        for (t = i0+1; t < j; t++) {
                            if (SDGdataPtr->endVertex[t] !=
                                SDGdataPtr->endVertex[t-1])
                            {
                                GPtr->paralEdgeIndex[ii+r] = t;
                                GPtr->outVertexList[ii+r] = SDGdataPtr->endVertex[t];
                                r++;
                            }
                        }

                    }
                }
                i0 = j;
                break;
            }
        } /* for j */

        if (i == GPtr->numVertices-1) {
            ULONGINT_T r = 0;
            if (numEdgesPlaced-i0 >= 0) {
                long ii = GPtr->outVertexIndex[i];
                GPtr->paralEdgeIndex[ii+r] = i0;
                GPtr->outVertexList[ii+r] = SDGdataPtr->endVertex[i0];
                r++;
                ULONGINT_T t;
                for (t = i0+1; t < numEdgesPlaced; t++) {
                    if (SDGdataPtr->endVertex[t] != SDGdataPtr->endVertex[t-1]) {
                        GPtr->paralEdgeIndex[ii+r] = t;
                        GPtr->outVertexList[ii+r] = SDGdataPtr->endVertex[t];
                        r++;
                    }
                }
            }
        }

    } /* for i */


    thread_barrier_wait_b();

    if (myId == 0) {
        P_FREE(SDGdataPtr->startVertex);
        P_FREE(SDGdataPtr->endVertex);
        GPtr->inDegree =
            (LONGINT_T*)P_MALLOC(GPtr->numVertices * sizeof(LONGINT_T));
        assert(GPtr->inDegree);
        GPtr->inVertexIndex =
            (ULONGINT_T*)P_MALLOC(GPtr->numVertices * sizeof(ULONGINT_T));
        assert(GPtr->inVertexIndex);
    }


    thread_barrier_wait_b();

    for (i = i_start; i < i_stop; i++) {
        GPtr->inDegree[i] = 0;
        GPtr->inVertexIndex[i] = 0;
    }

    /* A temp. array to store the inplied edges */
    ULONGINT_T* impliedEdgeList;
    if (myId == 0) {
        impliedEdgeList = (ULONGINT_T*)P_MALLOC(GPtr->numVertices
                                                * MAX_CLUSTER_SIZE
                                                * sizeof(ULONGINT_T));
        global_impliedEdgeList_b = impliedEdgeList;
    }


    thread_barrier_wait_b();

    impliedEdgeList = global_impliedEdgeList_b;

    createPartition(0,
                    (GPtr->numVertices * MAX_CLUSTER_SIZE),
                    myId,
                    numThread,
                    &i_start,
                    &i_stop);

    for (i = i_start; i < i_stop; i++) {
        impliedEdgeList[i] = 0;
    }

    /*
     * An auxiliary array to store implied edges, in case we overshoot
     * MAX_CLUSTER_SIZE
     */

    ULONGINT_T** auxArr;
    if (myId == 0) {
        auxArr = (ULONGINT_T**)P_MALLOC(GPtr->numVertices * sizeof(ULONGINT_T*));
        assert(auxArr);
        global_auxArr_b = auxArr;
    }


    thread_barrier_wait_b();

    auxArr = global_auxArr_b;

    createPartition(0, GPtr->numVertices, myId, numThread, &i_start, &i_stop);

    for (i = i_start; i < i_stop; i++) {
        /* Inspect adjacency list of vertex i */
        for (j = GPtr->outVertexIndex[i];
             j < (GPtr->outVertexIndex[i] + GPtr->outDegree[i]);
             j++)
        {
            ULONGINT_T v = GPtr->outVertexList[j];
            ULONGINT_T k;
            for (k = GPtr->outVertexIndex[v];
                 k < (GPtr->outVertexIndex[v] + GPtr->outDegree[v]);
                 k++)
            {
                if (GPtr->outVertexList[k] == i) {
                    break;
                }
            }
            if (k == GPtr->outVertexIndex[v]+GPtr->outDegree[v]) {
            	int ro = 0;
		local_exec_mode = 0;
                TM_BEGIN_B(0);
                	/* Add i to the impliedEdgeList of v */
                	long inDegree = (long)FAST_PATH_SHARED_READ(GPtr->inDegree[v]);
									long temp = inDegree + 1; 
                	FAST_PATH_SHARED_WRITE(GPtr->inDegree[v], temp);
                	if (inDegree < MAX_CLUSTER_SIZE) {
                		FAST_PATH_SHARED_WRITE(impliedEdgeList[v*MAX_CLUSTER_SIZE+inDegree],
                				i);
                	} else {
                		/* Use auxiliary array to store the implied edge */
                		/* Create an array if it's not present already */
                		ULONGINT_T* a = NULL;
                		if ((inDegree % MAX_CLUSTER_SIZE) == 0) {
                			a = (ULONGINT_T*)TM_MALLOC(MAX_CLUSTER_SIZE
                					* sizeof(ULONGINT_T));
                			assert(a);
                			FAST_PATH_SHARED_WRITE_P(auxArr[v], a);
                		} else {
                			a = auxArr[v];
                		}
                		FAST_PATH_SHARED_WRITE(a[inDegree % MAX_CLUSTER_SIZE], i);
                	}
                TM_END_B();
            }
        }
    } /* for i */


    thread_barrier_wait_b();

    prefix_sums_b(GPtr->inVertexIndex, GPtr->inDegree, GPtr->numVertices);

    if (myId == 0) {
        GPtr->numUndirectedEdges = GPtr->inVertexIndex[GPtr->numVertices-1]
                                   + GPtr->inDegree[GPtr->numVertices-1];
        GPtr->inVertexList =
            (ULONGINT_T *)P_MALLOC(GPtr->numUndirectedEdges * sizeof(ULONGINT_T));
    }


    thread_barrier_wait_b();

    /*
     * Create the inVertex List
     */

    for (i = i_start; i < i_stop; i++) {
        for (j = GPtr->inVertexIndex[i];
             j < (GPtr->inVertexIndex[i] + GPtr->inDegree[i]);
             j++)
        {
            if ((j - GPtr->inVertexIndex[i]) < MAX_CLUSTER_SIZE) {
                GPtr->inVertexList[j] =
                    impliedEdgeList[i*MAX_CLUSTER_SIZE+j-GPtr->inVertexIndex[i]];
            } else {
                GPtr->inVertexList[j] =
                    auxArr[i][(j-GPtr->inVertexIndex[i]) % MAX_CLUSTER_SIZE];
            }
        }
    }


    thread_barrier_wait_b();

    if (myId == 0) {
        P_FREE(impliedEdgeList);
    }

    for (i = i_start; i < i_stop; i++) {
        if (GPtr->inDegree[i] > MAX_CLUSTER_SIZE) {
            P_FREE(auxArr[i]);
        }
    }


    thread_barrier_wait_b();

    if (myId == 0) {
        P_FREE(auxArr);
    }

    TM_THREAD_EXIT();
}
#endif

void
computeGraph (void* argPtr)
{
    TM_THREAD_ENTER();

    graph*    GPtr;
    graphSDG* SDGdataPtr;
    if(local_replica){
		#ifdef DEP
      GPtr       = ((computeGraph_arg_t*)argPtr)->GPtr_b;
      SDGdataPtr = ((computeGraph_arg_t*)argPtr)->SDGdataPtr_b;
			_computeGraph_b(GPtr,SDGdataPtr);
		#endif
    }
    else{
      GPtr       = ((computeGraph_arg_t*)argPtr)->GPtr_a;
      SDGdataPtr = ((computeGraph_arg_t*)argPtr)->SDGdataPtr_a;
			_computeGraph_a(GPtr,SDGdataPtr);
    }
}


/* =============================================================================
 *
 * End of computeGraph.c
 *
 * =============================================================================
 */
