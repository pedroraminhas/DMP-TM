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

static ULONGINT_T*  global_p_b                 = NULL;
static ULONGINT_T   global_maxNumVertices_b    = 0;
static ULONGINT_T   global_outVertexListSize_b = 0;
static ULONGINT_T*  global_impliedEdgeList_b   = NULL;
static ULONGINT_T** global_auxArr_b            = NULL;


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

    thread_barrier_wait();


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

    thread_barrier_wait();


    if (myId == 0) {
        for (j = 1; j < numThread; j++) {
            p[NOSHARE(j)] += p[NOSHARE(j-1)];
        }
    }


    thread_barrier_wait();


    if (myId > 0) {
        ULONGINT_T add_value = p[NOSHARE(myId-1)];
        for (j = start-1; j < end; j++) {
            result[j] += add_value;
        }
    }


    thread_barrier_wait();


    if (myId == 0) {
        P_FREE(p);
    }
}
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


    thread_barrier_wait();


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

    thread_barrier_wait();


    if (myId == 0) {
        for (j = 1; j < numThread; j++) {
            p[NOSHARE(j)] += p[NOSHARE(j-1)];
        }
    }

    thread_barrier_wait();


    if (myId > 0) {
        ULONGINT_T add_value = p[NOSHARE(myId-1)];
        for (j = start-1; j < end; j++) {
            result[j] += add_value;
        }
    }


    thread_barrier_wait();


    if (myId == 0) {
        P_FREE(p);
    }
}

/* =============================================================================
 * computeGraph
 * =============================================================================
 */

void _computeGraph_a(graph* GPtr_a, graphSDG* SDGdataPtr_a,graph* GPtr_b, graphSDG* SDGdataPtr_b){
	  long numThread = local_num_threads;
    long myId = local_app_id;


    ULONGINT_T j;
    ULONGINT_T maxNumVertices_a = 0;
    ULONGINT_T numEdgesPlaced_a = SDGdataPtr_a->numEdgesPlaced;

    /*
     * First determine the number of vertices by scanning the tuple
     * startVertex list
     */

    long i;
    long i_start_a;
    long i_stop_a;
    createPartition(0, numEdgesPlaced_a, myId, numThread, &i_start_a, &i_stop_a);
    for (i = i_start_a; i < i_stop_a; i++) {
        if (SDGdataPtr_a->startVertex[i] > maxNumVertices_a) {
            maxNumVertices_a = SDGdataPtr_a->startVertex[i];
        }
    }

    ULONGINT_T maxNumVertices_b = 0;
    ULONGINT_T numEdgesPlaced_b = SDGdataPtr_b->numEdgesPlaced;

    /*
     * First determine the number of vertices by scanning the tuple
     * startVertex list
     */
    long i_start_b;
    long i_stop_b;
    createPartition(0, numEdgesPlaced_b, myId, numThread, &i_start_b, &i_stop_b);
    for (i = i_start_b; i < i_stop_b; i++) {
        if (SDGdataPtr_b->startVertex[i] > maxNumVertices_b) {
            maxNumVertices_b = SDGdataPtr_b->startVertex[i];
        }
    }

    int ro = 0;
    TM_BEGIN(0);
        long tmp_maxNumVertices_a = (long)FAST_PATH_SHARED_READ(global_maxNumVertices_a);
        long new_maxNumVertices_a = MAX(tmp_maxNumVertices_a, maxNumVertices_a) + 1;
        FAST_PATH_SHARED_WRITE(global_maxNumVertices_a, new_maxNumVertices_a);

        long tmp_maxNumVertices_b = (long)SLOW_PATH_SHARED_READ(global_maxNumVertices_b);
        long new_maxNumVertices_b = MAX(tmp_maxNumVertices_b, maxNumVertices_b) + 1;
        SLOW_PATH_SHARED_WRITE(global_maxNumVertices_b, new_maxNumVertices_b);
    TM_END();

		thread_barrier_wait();

    maxNumVertices_a = global_maxNumVertices_a;
    maxNumVertices_b = global_maxNumVertices_b;

    if (myId == 0) {

        GPtr_a->numVertices = maxNumVertices_a;
        GPtr_a->numEdges    = numEdgesPlaced_a;
        GPtr_a->intWeight   = SDGdataPtr_a->intWeight;
        GPtr_a->strWeight   = SDGdataPtr_a->strWeight;

        for (i = 0; i < numEdgesPlaced_a; i++) {
            if (GPtr_a->intWeight[numEdgesPlaced_a-i-1] < 0) {
                GPtr_a->numStrEdges = -(GPtr_a->intWeight[numEdgesPlaced_a-i-1]) + 1;
                GPtr_a->numIntEdges = numEdgesPlaced_a - GPtr_a->numStrEdges;
                break;
            }
        }

        GPtr_a->outDegree =
            (LONGINT_T*)P_MALLOC((GPtr_a->numVertices) * sizeof(LONGINT_T));
        assert(GPtr_a->outDegree);

        GPtr_a->outVertexIndex =
            (ULONGINT_T*)P_MALLOC((GPtr_a->numVertices) * sizeof(ULONGINT_T));
        assert(GPtr_a->outVertexIndex);

        GPtr_b->numVertices = maxNumVertices_b;
        GPtr_b->numEdges    = numEdgesPlaced_b;
        GPtr_b->intWeight   = SDGdataPtr_b->intWeight;
        GPtr_b->strWeight   = SDGdataPtr_b->strWeight;

        for (i = 0; i < numEdgesPlaced_b; i++) {
            if (GPtr_b->intWeight[numEdgesPlaced_b-i-1] < 0) {
                GPtr_b->numStrEdges = -(GPtr_b->intWeight[numEdgesPlaced_b-i-1]) + 1;
                GPtr_b->numIntEdges = numEdgesPlaced_b - GPtr_b->numStrEdges;
                break;
            }
        }

        GPtr_b->outDegree =
            (LONGINT_T*)P_MALLOC((GPtr_b->numVertices) * sizeof(LONGINT_T));
        assert(GPtr_b->outDegree);

        GPtr_b->outVertexIndex =
            (ULONGINT_T*)P_MALLOC((GPtr_b->numVertices) * sizeof(ULONGINT_T));
        assert(GPtr_b->outVertexIndex);
    }


    thread_barrier_wait();


    createPartition(0, GPtr_a->numVertices, myId, numThread, &i_start_a, &i_stop_a);

    for (i = i_start_a; i < i_stop_a; i++) {
        GPtr_a->outDegree[i] = 0;
        GPtr_a->outVertexIndex[i] = 0;
    }

    ULONGINT_T outVertexListSize_a = 0;

    createPartition(0, GPtr_b->numVertices, myId, numThread, &i_start_b, &i_stop_b);

    for (i = i_start_b; i < i_stop_b; i++) {
        GPtr_b->outDegree[i] = 0;
        GPtr_b->outVertexIndex[i] = 0;
    }

    ULONGINT_T outVertexListSize_b = 0;

    thread_barrier_wait();


    ULONGINT_T i0_a = -1UL;

    for (i = i_start_a; i < i_stop_a; i++) {

        ULONGINT_T k = i;
        if ((outVertexListSize_a == 0) && (k != 0)) {
            while (i0_a == -1UL) {
                for (j = 0; j < numEdgesPlaced_a; j++) {
                    if (k == SDGdataPtr_a->startVertex[j]) {
                        i0_a = j;
                        break;
                    }

                }
                k--;
            }
        }

        if ((outVertexListSize_a == 0) && (k == 0)) {
            i0_a = 0;
        }

        for (j = i0_a; j < numEdgesPlaced_a; j++) {
            if (i == GPtr_a->numVertices-1) {
                break;
            }
            if ((i != SDGdataPtr_a->startVertex[j])) {
                if ((j > 0) && (i == SDGdataPtr_a->startVertex[j-1])) {
                    if (j-i0_a >= 1) {
                        outVertexListSize_a++;
                        GPtr_a->outDegree[i]++;
                        ULONGINT_T t;
                        for (t = i0_a+1; t < j; t++) {
                            if (SDGdataPtr_a->endVertex[t] !=
                                SDGdataPtr_a->endVertex[t-1])
                            {
                                outVertexListSize_a++;
                                GPtr_a->outDegree[i] = GPtr_a->outDegree[i]+1;
                            }
                        }
                    }
                }
                i0_a = j;
                break;
            }
        }

        if (i == GPtr_a->numVertices-1) {
            if (numEdgesPlaced_a-i0_a >= 0) {
                outVertexListSize_a++;
                GPtr_a->outDegree[i]++;
                ULONGINT_T t;
                for (t = i0_a+1; t < numEdgesPlaced_a; t++) {
                    if (SDGdataPtr_a->endVertex[t] != SDGdataPtr_a->endVertex[t-1]) {
                        outVertexListSize_a++;
                        GPtr_a->outDegree[i]++;
                    }
                }
            }
        }

    } /* for i */

    /* second execution */

    ULONGINT_T i0_b = -1UL;

    for (i = i_start_b; i < i_stop_b; i++) {

        ULONGINT_T k = i;
        if ((outVertexListSize_b == 0) && (k != 0)) {
            while (i0_b == -1UL) {
                for (j = 0; j < numEdgesPlaced_b; j++) {
                    if (k == SDGdataPtr_b->startVertex[j]) {
                        i0_b = j;
                        break;
                    }

                }
                k--;
            }
        }

        if ((outVertexListSize_b == 0) && (k == 0)) {
            i0_b = 0;
        }

        for (j = i0_b; j < numEdgesPlaced_b; j++) {
            if (i == GPtr_b->numVertices-1) {
                break;
            }
            if ((i != SDGdataPtr_b->startVertex[j])) {
                if ((j > 0) && (i == SDGdataPtr_b->startVertex[j-1])) {
                    if (j-i0_b >= 1) {
                        outVertexListSize_b++;
                        GPtr_b->outDegree[i]++;
                        ULONGINT_T t;
                        for (t = i0_b+1; t < j; t++) {
                            if (SDGdataPtr_b->endVertex[t] !=
                                SDGdataPtr_b->endVertex[t-1])
                            {
                                outVertexListSize_b++;
                                GPtr_b->outDegree[i] = GPtr_b->outDegree[i]+1;
                            }
                        }
                    }
                }
                i0_b = j;
                break;
            }
        }

        if (i == GPtr_b->numVertices-1) {
            if (numEdgesPlaced_b-i0_b >= 0) {
                outVertexListSize_b++;
                GPtr_b->outDegree[i]++;
                ULONGINT_T t;
                for (t = i0_b+1; t < numEdgesPlaced_b; t++) {
                    if (SDGdataPtr_b->endVertex[t] != SDGdataPtr_b->endVertex[t-1]) {
                        outVertexListSize_b++;
                        GPtr_b->outDegree[i]++;
                    }
                }
            }
        }

    } /* for i */



    thread_barrier_wait();


    prefix_sums_a(GPtr_a->outVertexIndex, GPtr_a->outDegree, GPtr_a->numVertices);
    prefix_sums_b(GPtr_b->outVertexIndex, GPtr_b->outDegree, GPtr_b->numVertices);

    thread_barrier_wait();

	  TM_BEGIN(0);
				long temp_a = (long)FAST_PATH_SHARED_READ(global_outVertexListSize_a);
				temp_a += outVertexListSize_a;
        FAST_PATH_SHARED_WRITE(
            global_outVertexListSize_a,
            temp_a);

        long temp_b = (long)SLOW_PATH_SHARED_READ(global_outVertexListSize_b);
				temp_b += outVertexListSize_b;
        SLOW_PATH_SHARED_WRITE(
            global_outVertexListSize_b,
            temp_b);
    TM_END();



    thread_barrier_wait();

    outVertexListSize_a = global_outVertexListSize_a;

    if (myId == 0) {
        GPtr_a->numDirectedEdges = outVertexListSize_a;
        GPtr_a->outVertexList =
            (ULONGINT_T*)P_MALLOC(outVertexListSize_a * sizeof(ULONGINT_T));
        assert(GPtr_a->outVertexList);
        GPtr_a->paralEdgeIndex =
            (ULONGINT_T*)P_MALLOC(outVertexListSize_a * sizeof(ULONGINT_T));
        assert(GPtr_a->paralEdgeIndex);
        GPtr_a->outVertexList[0] = SDGdataPtr_a->endVertex[0];
    }

    outVertexListSize_b = global_outVertexListSize_b;

    if (myId == 0) {
        GPtr_b->numDirectedEdges = outVertexListSize_b;
        GPtr_b->outVertexList =
            (ULONGINT_T*)P_MALLOC(outVertexListSize_b * sizeof(ULONGINT_T));
        assert(GPtr_b->outVertexList);
        GPtr_b->paralEdgeIndex =
            (ULONGINT_T*)P_MALLOC(outVertexListSize_b * sizeof(ULONGINT_T));
        assert(GPtr_b->paralEdgeIndex);
        GPtr_b->outVertexList[0] = SDGdataPtr_b->endVertex[0];
    }



    thread_barrier_wait();

    /*
     * Evaluate outVertexList
     */

    i0_a = -1UL;

    for (i = i_start_a; i < i_stop_a; i++) {

        ULONGINT_T k = i;
        while ((i0_a == -1UL) && (k != 0)) {
            for (j = 0; j < numEdgesPlaced_a; j++) {
                if (k == SDGdataPtr_a->startVertex[j]) {
                    i0_a = j;
                    break;
                }
            }
            k--;
        }

        if ((i0_a == -1) && (k == 0)) {
            i0_a = 0;
        }

        for (j = i0_a; j < numEdgesPlaced_a; j++) {
            if (i == GPtr_a->numVertices-1) {
                break;
            }
            if (i != SDGdataPtr_a->startVertex[j]) {
                if ((j > 0) && (i == SDGdataPtr_a->startVertex[j-1])) {
                    if (j-i0_a >= 1) {
                        long ii = GPtr_a->outVertexIndex[i];
                        ULONGINT_T r = 0;
                        GPtr_a->paralEdgeIndex[ii] = i0_a;
                        GPtr_a->outVertexList[ii] = SDGdataPtr_a->endVertex[i0_a];
                        r++;
                        ULONGINT_T t;
                        for (t = i0_a+1; t < j; t++) {
                            if (SDGdataPtr_a->endVertex[t] !=
                                SDGdataPtr_a->endVertex[t-1])
                            {
                                GPtr_a->paralEdgeIndex[ii+r] = t;
                                GPtr_a->outVertexList[ii+r] = SDGdataPtr_a->endVertex[t];
                                r++;
                            }
                        }

                    }
                }
                i0_a = j;
                break;
            }
        } /* for j */

        if (i == GPtr_a->numVertices-1) {
            ULONGINT_T r = 0;
            if (numEdgesPlaced_a-i0_a >= 0) {
                long ii = GPtr_a->outVertexIndex[i];
                GPtr_a->paralEdgeIndex[ii+r] = i0_a;
                GPtr_a->outVertexList[ii+r] = SDGdataPtr_a->endVertex[i0_a];
                r++;
                ULONGINT_T t;
                for (t = i0_a+1; t < numEdgesPlaced_a; t++) {
                    if (SDGdataPtr_a->endVertex[t] != SDGdataPtr_a->endVertex[t-1]) {
                        GPtr_a->paralEdgeIndex[ii+r] = t;
                        GPtr_a->outVertexList[ii+r] = SDGdataPtr_a->endVertex[t];
                        r++;
                    }
                }
            }
        }

    } /* for i */

    /* second execution */

    i0_b = -1UL;

    for (i = i_start_b; i < i_stop_b; i++) {

        ULONGINT_T k = i;
        while ((i0_b == -1UL) && (k != 0)) {
            for (j = 0; j < numEdgesPlaced_b; j++) {
                if (k == SDGdataPtr_b->startVertex[j]) {
                    i0_b = j;
                    break;
                }
            }
            k--;
        }

        if ((i0_b == -1) && (k == 0)) {
            i0_b = 0;
        }

        for (j = i0_b; j < numEdgesPlaced_b; j++) {
            if (i == GPtr_b->numVertices-1) {
                break;
            }
            if (i != SDGdataPtr_b->startVertex[j]) {
                if ((j > 0) && (i == SDGdataPtr_b->startVertex[j-1])) {
                    if (j-i0_b >= 1) {
                        long ii = GPtr_b->outVertexIndex[i];
                        ULONGINT_T r = 0;
                        GPtr_b->paralEdgeIndex[ii] = i0_b;
                        GPtr_b->outVertexList[ii] = SDGdataPtr_b->endVertex[i0_b];
                        r++;
                        ULONGINT_T t;
                        for (t = i0_b+1; t < j; t++) {
                            if (SDGdataPtr_b->endVertex[t] !=
                                SDGdataPtr_b->endVertex[t-1])
                            {
                                GPtr_b->paralEdgeIndex[ii+r] = t;
                                GPtr_b->outVertexList[ii+r] = SDGdataPtr_b->endVertex[t];
                                r++;
                            }
                        }

                    }
                }
                i0_b = j;
                break;
            }
        } /* for j */

        if (i == GPtr_b->numVertices-1) {
            ULONGINT_T r = 0;
            if (numEdgesPlaced_b-i0_b >= 0) {
                long ii = GPtr_b->outVertexIndex[i];
                GPtr_b->paralEdgeIndex[ii+r] = i0_b;
                GPtr_b->outVertexList[ii+r] = SDGdataPtr_b->endVertex[i0_b];
                r++;
                ULONGINT_T t;
                for (t = i0_b+1; t < numEdgesPlaced_b; t++) {
                    if (SDGdataPtr_b->endVertex[t] != SDGdataPtr_b->endVertex[t-1]) {
                        GPtr_b->paralEdgeIndex[ii+r] = t;
                        GPtr_b->outVertexList[ii+r] = SDGdataPtr_b->endVertex[t];
                        r++;
                    }
                }
            }
        }

    } /* for i */

    thread_barrier_wait();

    if (myId == 0) {
        P_FREE(SDGdataPtr_a->startVertex);
        P_FREE(SDGdataPtr_a->endVertex);
        GPtr_a->inDegree =
            (LONGINT_T*)P_MALLOC(GPtr_a->numVertices * sizeof(LONGINT_T));
        assert(GPtr_a->inDegree);
        GPtr_a->inVertexIndex =
            (ULONGINT_T*)P_MALLOC(GPtr_a->numVertices * sizeof(ULONGINT_T));
        assert(GPtr_a->inVertexIndex);
    }

    if (myId == 0) {
        P_FREE(SDGdataPtr_b->startVertex);
        P_FREE(SDGdataPtr_b->endVertex);
        GPtr_b->inDegree =
            (LONGINT_T*)P_MALLOC(GPtr_b->numVertices * sizeof(LONGINT_T));
        assert(GPtr_b->inDegree);
        GPtr_b->inVertexIndex =
            (ULONGINT_T*)P_MALLOC(GPtr_b->numVertices * sizeof(ULONGINT_T));
        assert(GPtr_b->inVertexIndex);
    }


    thread_barrier_wait();


    for (i = i_start_a; i < i_stop_a; i++) {
        GPtr_a->inDegree[i] = 0;
        GPtr_a->inVertexIndex[i] = 0;
    }

    /* A temp. array to store the inplied edges */
    ULONGINT_T* impliedEdgeList_a;
    if (myId == 0) {
        impliedEdgeList_a = (ULONGINT_T*)P_MALLOC(GPtr_a->numVertices
                                                * MAX_CLUSTER_SIZE
                                                * sizeof(ULONGINT_T));
        global_impliedEdgeList_a = impliedEdgeList_a;
    }



    for (i = i_start_b; i < i_stop_b; i++) {
        GPtr_b->inDegree[i] = 0;
        GPtr_b->inVertexIndex[i] = 0;
    }

    /* A temp. array to store the inplied edges */
    ULONGINT_T* impliedEdgeList_b;
    if (myId == 0) {
        impliedEdgeList_b = (ULONGINT_T*)P_MALLOC(GPtr_b->numVertices
                                                * MAX_CLUSTER_SIZE
                                                * sizeof(ULONGINT_T));
        global_impliedEdgeList_b = impliedEdgeList_b;
    }


    thread_barrier_wait();


    impliedEdgeList_a = global_impliedEdgeList_a;

    createPartition(0,
                    (GPtr_a->numVertices * MAX_CLUSTER_SIZE),
                    myId,
                    numThread,
                    &i_start_a,
                    &i_stop_a);

    for (i = i_start_a; i < i_stop_a; i++) {
        impliedEdgeList_a[i] = 0;
    }

    /*
     * An auxiliary array to store implied edges, in case we overshoot
     * MAX_CLUSTER_SIZE
     */

    ULONGINT_T** auxArr_a;
    if (myId == 0) {
        auxArr_a = (ULONGINT_T**)P_MALLOC(GPtr_a->numVertices * sizeof(ULONGINT_T*));
        assert(auxArr_a);
        global_auxArr_a = auxArr_a;
    }

    /*second execution*/

    impliedEdgeList_b = global_impliedEdgeList_b;

    createPartition(0,
                    (GPtr_b->numVertices * MAX_CLUSTER_SIZE),
                    myId,
                    numThread,
                    &i_start_b,
                    &i_stop_b);

    for (i = i_start_b; i < i_stop_b; i++) {
        impliedEdgeList_b[i] = 0;
    }

    /*
     * An auxiliary array to store implied edges, in case we overshoot
     * MAX_CLUSTER_SIZE
     */

    ULONGINT_T** auxArr_b;
    if (myId == 0) {
        auxArr_b = (ULONGINT_T**)P_MALLOC(GPtr_b->numVertices * sizeof(ULONGINT_T*));
        assert(auxArr_b);
        global_auxArr_b = auxArr_b;
    }


    thread_barrier_wait();


    /*second execution*/
    auxArr_a = global_auxArr_a;
    auxArr_b = global_auxArr_b;

    createPartition(0, GPtr_a->numVertices, myId, numThread, &i_start_a, &i_stop_a);
    createPartition(0, GPtr_b->numVertices, myId, numThread, &i_start_b, &i_stop_b);

    for (i = i_start_b; i < i_stop_b; i++) {
        /* Inspect adjacency list of vertex i */
        for (j = GPtr_b->outVertexIndex[i];
             j < (GPtr_b->outVertexIndex[i] + GPtr_b->outDegree[i]);
             j++)
        {
            ULONGINT_T v = GPtr_b->outVertexList[j];
            ULONGINT_T k;
            for (k = GPtr_b->outVertexIndex[v];
                 k < (GPtr_b->outVertexIndex[v] + GPtr_b->outDegree[v]);
                 k++)
            {
                if (GPtr_b->outVertexList[k] == i) {
                    break;
                }
            }
            if (k == GPtr_b->outVertexIndex[v]+GPtr_b->outDegree[v]) {
                	/* Add i to the impliedEdgeList of v */
                  TM_BEGIN(0);

                  long inDegree_a = (long)FAST_PATH_SHARED_READ(GPtr_a->inDegree[v]);
									long temp_a = inDegree_a + 1;\
                	FAST_PATH_SHARED_WRITE(GPtr_a->inDegree[v], temp_a);
                	if (inDegree_a < MAX_CLUSTER_SIZE) {
                		FAST_PATH_SHARED_WRITE(impliedEdgeList_a[v*MAX_CLUSTER_SIZE+inDegree_a],
                				i);
                	} else {
                		/* Use auxiliary array to store the implied edge */
                		/* Create an array if it's not present already */
                		ULONGINT_T* a = NULL;
                		if ((inDegree_a % MAX_CLUSTER_SIZE) == 0) {
                			a = (ULONGINT_T*)TM_MALLOC(MAX_CLUSTER_SIZE
                					* sizeof(ULONGINT_T));
                			assert(a);
                			FAST_PATH_SHARED_WRITE_P(auxArr_a[v], a);
                		} else {
                			a = auxArr_a[v];
                		}
                		FAST_PATH_SHARED_WRITE(a[inDegree_a % MAX_CLUSTER_SIZE], i);
                	}


                	long inDegree_b = (long)SLOW_PATH_SHARED_READ(GPtr_b->inDegree[v]);
									long temp_b = inDegree_b + 1;\
                	SLOW_PATH_SHARED_WRITE(GPtr_b->inDegree[v], temp_b);
                	if (inDegree_b < MAX_CLUSTER_SIZE) {
                		SLOW_PATH_SHARED_WRITE(impliedEdgeList_b[v*MAX_CLUSTER_SIZE+inDegree_b],
                				i);
                	} else {
                		/* Use auxiliary array to store the implied edge */
                		/* Create an array if it's not present already */
                		ULONGINT_T* a = NULL;
                		if ((inDegree_b % MAX_CLUSTER_SIZE) == 0) {
                			a = (ULONGINT_T*)TM_MALLOC(MAX_CLUSTER_SIZE
                					* sizeof(ULONGINT_T));
                			assert(a);
                			SLOW_PATH_SHARED_WRITE_P(auxArr_b[v], a);
                		} else {
                			a = auxArr_b[v];
                		}
                		SLOW_PATH_SHARED_WRITE(a[inDegree_b % MAX_CLUSTER_SIZE], i);
                	}
                  TM_END();
            }
        }
    } /* for i */

    thread_barrier_wait();

    prefix_sums_a(GPtr_a->inVertexIndex, GPtr_a->inDegree, GPtr_a->numVertices);

    if (myId == 0) {
        GPtr_a->numUndirectedEdges = GPtr_a->inVertexIndex[GPtr_a->numVertices-1]
                                   + GPtr_a->inDegree[GPtr_a->numVertices-1];
        GPtr_a->inVertexList =
            (ULONGINT_T *)P_MALLOC(GPtr_a->numUndirectedEdges * sizeof(ULONGINT_T));
    }

    prefix_sums_b(GPtr_b->inVertexIndex, GPtr_b->inDegree, GPtr_b->numVertices);

    if (myId == 0) {
        GPtr_b->numUndirectedEdges = GPtr_b->inVertexIndex[GPtr_b->numVertices-1]
                                   + GPtr_b->inDegree[GPtr_b->numVertices-1];
        GPtr_b->inVertexList =
            (ULONGINT_T *)P_MALLOC(GPtr_b->numUndirectedEdges * sizeof(ULONGINT_T));
    }


    thread_barrier_wait();

    /*
     * Create the inVertex List
     */

    for (i = i_start_a; i < i_stop_a; i++) {
        for (j = GPtr_a->inVertexIndex[i];
             j < (GPtr_a->inVertexIndex[i] + GPtr_a->inDegree[i]);
             j++)
        {
            if ((j - GPtr_a->inVertexIndex[i]) < MAX_CLUSTER_SIZE) {
                GPtr_a->inVertexList[j] =
                    impliedEdgeList_a[i*MAX_CLUSTER_SIZE+j-GPtr_a->inVertexIndex[i]];
            } else {
                GPtr_a->inVertexList[j] =
                    auxArr_a[i][(j-GPtr_a->inVertexIndex[i]) % MAX_CLUSTER_SIZE];
            }
        }
    }


    for (i = i_start_b; i < i_stop_b; i++) {
        for (j = GPtr_b->inVertexIndex[i];
             j < (GPtr_b->inVertexIndex[i] + GPtr_b->inDegree[i]);
             j++)
        {
            if ((j - GPtr_b->inVertexIndex[i]) < MAX_CLUSTER_SIZE) {
                GPtr_b->inVertexList[j] =
                    impliedEdgeList_b[i*MAX_CLUSTER_SIZE+j-GPtr_b->inVertexIndex[i]];
            } else {
                GPtr_b->inVertexList[j] =
                    auxArr_b[i][(j-GPtr_b->inVertexIndex[i]) % MAX_CLUSTER_SIZE];
            }
        }
    }




    thread_barrier_wait();

    if (myId == 0) {
        P_FREE(impliedEdgeList_a);
    }

    for (i = i_start_a; i < i_stop_a; i++) {
        if (GPtr_a->inDegree[i] > MAX_CLUSTER_SIZE) {
            P_FREE(auxArr_a[i]);
        }
    }

    if (myId == 0) {
        P_FREE(impliedEdgeList_b);
    }

    for (i = i_start_b; i < i_stop_b; i++) {
        if (GPtr_b->inDegree[i] > MAX_CLUSTER_SIZE) {
            P_FREE(auxArr_b[i]);
        }
    }

    thread_barrier_wait();

    if (myId == 0) {
        P_FREE(auxArr_a);
    }

    if (myId == 0) {
        P_FREE(auxArr_b);
    }

    TM_THREAD_EXIT();
}


void
computeGraph (void* argPtr)
{
    TM_THREAD_ENTER();

    graph*    GPtr_a;
    graphSDG* SDGdataPtr_a;
    graph*    GPtr_b;
    graphSDG* SDGdataPtr_b;
    GPtr_b       = ((computeGraph_arg_t*)argPtr)->GPtr_b;
    SDGdataPtr_b = ((computeGraph_arg_t*)argPtr)->SDGdataPtr_b;
    GPtr_a       = ((computeGraph_arg_t*)argPtr)->GPtr_a;
    SDGdataPtr_a = ((computeGraph_arg_t*)argPtr)->SDGdataPtr_a;
		_computeGraph_a(GPtr_a,SDGdataPtr_a,GPtr_b,SDGdataPtr_b);
}


/* =============================================================================
 *
 * End of computeGraph.c
 *
 * =============================================================================
 */
