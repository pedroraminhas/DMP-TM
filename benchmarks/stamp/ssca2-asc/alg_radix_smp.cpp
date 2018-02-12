/* =============================================================================
 *
 * alg_radix_smp.c
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
#include "alg_radix_smp.h"
#include "createPartition.h"
#include "thread.h"
#include "tm.h"


#define BITS(x, k, j)  ((x>>k) & ~(~0<<j))

static long*          global_myHisto_a = NULL;
static long*          global_psHisto_a = NULL;
static unsigned long* global_lTemp_a   = NULL;
static unsigned long* global_lTemp2_a  = NULL;

static long*          global_myHisto_b = NULL;
static long*          global_psHisto_b = NULL;
static unsigned long* global_lTemp_b   = NULL;
static unsigned long* global_lTemp2_b  = NULL;

/* =============================================================================
 * all_countsort_node
 *
 * R (range)      must be a multiple of NODES
 * q (elems/proc) must be a multiple of NODES
 * =============================================================================
 */
void
all_countsort_node (long q,
                    unsigned long* lKey,
                    unsigned long* lSorted,
                    long R,
                    long bitOff,
                    long m)
{
    long myId = local_app_id;
    long numThread = local_num_threads;

    long* myHisto = NULL;
    long* psHisto = NULL;

    if (myId == 0) {
        myHisto = (long*)P_MALLOC(numThread * R * sizeof(long));
        assert(myHisto);
				#ifdef DEP
				if(local_replica)
					global_myHisto_b = myHisto;
				else
				#endif
        global_myHisto_a = myHisto;
        psHisto = (long*)P_MALLOC(numThread * R * sizeof(long));
        assert(psHisto);
				#ifdef DEP
        if(local_replica)
					global_psHisto_b = psHisto;
				else
				#endif
        global_psHisto_a = psHisto;
    }

		#ifdef DEP
      if(local_replica)
		    thread_barrier_wait_b();
			else
				thread_barrier_wait_a();
		#else
		thread_barrier_wait(); 
		#endif

		#ifdef DEP
    	if(local_replica){
				myHisto = global_myHisto_b;
		    psHisto = global_psHisto_b;
			}else{
		#endif
    myHisto = global_myHisto_a;
    psHisto = global_psHisto_a;
		#ifdef DEP
		}
		#endif


    long* mhp = myHisto + myId * R;

    long k;
    for (k = 0; k < R; k++) {
        mhp[k] = 0;
    }

    long k_start;
    long k_stop;
    createPartition(0, q, myId, numThread, &k_start, &k_stop);

    for (k = k_start; k < k_stop; k++) {
        mhp[BITS(lKey[k],bitOff,m)]++;
    }

		#ifdef DEP
      if(local_replica)
        thread_barrier_wait_b();
      else
        thread_barrier_wait_a();
    #else
    thread_barrier_wait();
    #endif

    createPartition(0, R, myId, numThread, &k_start, &k_stop);

    long last;
    long j;
    for (k = k_start; k < k_stop; k++) {
        last = psHisto[k] = myHisto[k];
        for (j = 1; j < numThread; j++) {
            long temp = psHisto[j*R + k] = last + myHisto[j*R + k];
            last = temp;
        }
    }

    long* allHisto = psHisto + (numThread - 1) * R;

		#ifdef DEP
      if(local_replica)
        thread_barrier_wait_b();
      else
        thread_barrier_wait_a();
    #else
    thread_barrier_wait();
    #endif    

		long offset = 0;

    long* mps = psHisto + (myId * R);
    for (k = 0; k < R; k++) {
        mhp[k]  = (mps[k] - mhp[k]) + offset;
        offset += allHisto[k];
    }

		#ifdef DEP
      if(local_replica)
        thread_barrier_wait_b();
      else
        thread_barrier_wait_a();
    #else
    thread_barrier_wait();
    #endif

    createPartition(0, q, myId, numThread, &k_start, &k_stop);

    for (k = k_start; k < k_stop; k++) {
        j = BITS(lKey[k],bitOff,m);
        lSorted[mhp[j]] = lKey[k];
        mhp[j]++;
    }

		#ifdef DEP
      if(local_replica)
        thread_barrier_wait_b();
      else
        thread_barrier_wait_a();
    #else
    thread_barrier_wait();
    #endif

    if (myId == 0) {
        P_FREE(psHisto);
        P_FREE(myHisto);
    }
}


/* =============================================================================
 * all_countsort_node_aux_seq
 *
 * R (range)      must be a multiple of NODES
 * q (elems/proc) must be a multiple of NODES
 * =============================================================================
 */
void
all_countsort_node_aux_seq (long q,
                            unsigned long* lKey,
                            unsigned long* lSorted,
                            unsigned long* auxKey,
                            unsigned long* auxSorted,
                            long R,
                            long bitOff,
                            long m)
{
    long* myHisto = (long*)malloc(R * sizeof(long));
    assert(myHisto);
    long* psHisto = (long*)malloc(R * sizeof(long));
    assert(psHisto);

    long* mhp = myHisto;

    long k;
    for (k = 0; k < R; k++) {
        mhp[k] = 0;
    }

    for (k = 0; k < q; k++) {
        mhp[BITS(lKey[k],bitOff,m)]++;
    }

    long last;
    long j;
    for (k = 0; k < R; k++) {
        last = psHisto[k] = myHisto[k];
    }

    long* allHisto = psHisto;

    long offset = 0;

    long* mps = psHisto;
    for (k = 0; k < R; k++) {
        mhp[k] = (mps[k] - mhp[k]) + offset;
        offset += allHisto[k];
    }

    for (k = 0; k < q; k++) {
        j = BITS(lKey[k], bitOff, m);
        lSorted[mhp[j]] = lKey[k];
        auxSorted[mhp[j]] = auxKey[k];
        mhp[j]++;
    }

    free(psHisto);
    free(myHisto);
}


/* =============================================================================
 * all_countsort_node_aux
 *
 * R (range)      must be a multiple of NODES
 * q (elems/proc) must be a multiple of NODES
 * =============================================================================
 */
void
all_countsort_node_aux (long q,
                        unsigned long* lKey,
                        unsigned long* lSorted,
                        unsigned long* auxKey,
                        unsigned long* auxSorted,
                        long R,
                        long bitOff,
                        long m)
{

    long myId = local_app_id;
    long numThread = local_num_threads;

    long* myHisto = NULL;
    long* psHisto = NULL;

    if (myId == 0) {
        myHisto = (long*)P_MALLOC(numThread * R * sizeof(long));
        assert(myHisto);
				#ifdef DEP
        if(local_replica)
					global_myHisto_b = myHisto;
				else
				#endif 
        global_myHisto_a = myHisto;
        psHisto = (long*)P_MALLOC(numThread * R * sizeof(long));
        assert(psHisto);
				#ifdef DEP
        if(local_replica)
					global_psHisto_b = psHisto;
				else
				#endif
        global_psHisto_b = psHisto;
    }

		#ifdef DEP
      if(local_replica)
        thread_barrier_wait_b();
      else
        thread_barrier_wait_a();
    #else
    thread_barrier_wait();
    #endif

		#ifdef DEP
      if(local_replica){
        myHisto = global_myHisto_b;
        psHisto = global_psHisto_b;
      }else{
    #endif
    myHisto = global_myHisto_a;
    psHisto = global_psHisto_a;
    #ifdef DEP
    }
    #endif

    long* mhp = myHisto + myId * R;

    long k;
    for (k = 0; k < R; k++) {
        mhp[k] = 0;
    }

    long k_start;
    long k_stop;
    createPartition(0, q, myId, numThread, &k_start, &k_stop);

    for (k = k_start; k < k_stop; k++) {
        mhp[BITS(lKey[k],bitOff,m)]++;
    }
		
		#ifdef DEP
      if(local_replica)
        thread_barrier_wait_b();
      else
        thread_barrier_wait_a();
    #else
    thread_barrier_wait();
    #endif
    
		createPartition(0, R, myId, numThread, &k_start, &k_stop);

    long last;
    long j;
    for (k = k_start; k < k_stop; k++) {
        last = psHisto[k] = myHisto[k];
        for (j = 1; j < numThread; j++) {
            long temp = psHisto[j*R + k] = last + myHisto[j*R + k];
            last = temp;
        }
    }

    long* allHisto = psHisto + (numThread - 1) * R;


		#ifdef DEP
      if(local_replica)
        thread_barrier_wait_b();
      else
        thread_barrier_wait_a();
    #else
    thread_barrier_wait();
    #endif

    long offset = 0;

    long* mps = psHisto + (myId * R);
    for (k = 0; k < R; k++) {
        mhp[k] = (mps[k] - mhp[k]) + offset;
        offset += allHisto[k];
    }

		#ifdef DEP
      if(local_replica)
        thread_barrier_wait_b();
      else
        thread_barrier_wait_a();
    #else
    thread_barrier_wait();
    #endif


    createPartition(0, q, myId, numThread, &k_start, &k_stop);

    for (k = k_start; k < k_stop; k++) {
        j = BITS(lKey[k], bitOff, m);
        lSorted[mhp[j]] = lKey[k];
        auxSorted[mhp[j]] = auxKey[k];
        mhp[j]++;
    }

		#ifdef DEP
      if(local_replica)
        thread_barrier_wait_b();
      else
        thread_barrier_wait_a();
    #else
    thread_barrier_wait();
    #endif


    if (myId == 0) {
        P_FREE(psHisto);
        P_FREE(myHisto);
    }
}


/* =============================================================================
 * all_radixsort_node_s3
 *
 * q (elems/proc) must be a multiple of NODES
 * =============================================================================
 */
void
all_radixsort_node_s3 (long q,
                       unsigned long* lKeys,
                       unsigned long* lSorted)
{

    long myId = local_app_id;

    unsigned long* lTemp = NULL;

    if (myId == 0) {
        lTemp = (unsigned long*)P_MALLOC(q * sizeof(unsigned long));
        assert(lTemp);
				#ifdef DEP
	      if(local_replica)
					global_lTemp_b = lTemp;
				else
				#endif
  	      global_lTemp_a = lTemp;
    }


		#ifdef DEP
      if(local_replica)
        thread_barrier_wait_b();
      else
        thread_barrier_wait_a();
    #else
    thread_barrier_wait();
    #endif

		#ifdef DEP
      if(local_replica)
				lTemp = global_lTemp_b;
			else
		#endif
    lTemp = global_lTemp_a;

    all_countsort_node(q, lKeys,   lSorted, (1<<11),  0, 11);
    all_countsort_node(q, lSorted, lTemp,   (1<<11), 11, 11);
    all_countsort_node(q, lTemp,   lSorted, (1<<10), 22, 10);


		#ifdef DEP
      if(local_replica)
        thread_barrier_wait_b();
      else
        thread_barrier_wait_a();
    #else
    thread_barrier_wait();
    #endif

    if (myId == 0) {
        P_FREE(lTemp);
    }
}


/* =============================================================================
 * all_radixsort_node_s2
 *
 * q (elems/proc) must be a multiple of NODES
 * =============================================================================
 */
void
all_radixsort_node_s2 (long q,
                       unsigned long* lKeys,
                       unsigned long* lSorted)
{


    long myId = local_app_id;

    unsigned long* lTemp = NULL;

    if (myId == 0) {
        lTemp = (unsigned long*)P_MALLOC(q * sizeof(unsigned long));
        assert(lTemp);
				#ifdef DEP
        if(local_replica)
          global_lTemp_b = lTemp;
        else
        #endif
          global_lTemp_a = lTemp;
    }


		#ifdef DEP
      if(local_replica)
        thread_barrier_wait_b();
      else
        thread_barrier_wait_a();
    #else
    thread_barrier_wait();
    #endif

		#ifdef DEP
      if(local_replica)
        lTemp = global_lTemp_b;
      else
    #endif
    lTemp = global_lTemp_a;

    all_countsort_node(q, lKeys, lTemp,   (1<<16),  0, 16);
    all_countsort_node(q, lTemp, lSorted, (1<<16), 16, 16);


		#ifdef DEP
      if(local_replica)
        thread_barrier_wait_b();
      else
        thread_barrier_wait_a();
    #else
    thread_barrier_wait();
    #endif

    if (myId == 0) {
        P_FREE(lTemp);
    }
}


/* =============================================================================
 * all_radixsort_node_aux_s3_seq
 *
 * q (elems/proc) must be a multiple of NODES
 * =============================================================================
 */
void
all_radixsort_node_aux_s3_seq (long q,
                               unsigned long* lKeys,
                               unsigned long* lSorted,
                               unsigned long* auxKey,
                               unsigned long* auxSorted)
{

    unsigned long* lTemp  = NULL;
    unsigned long* lTemp2 = NULL;

    lTemp = (unsigned long*)malloc(q * sizeof(unsigned long));
    assert(lTemp);
    lTemp2 = (unsigned long*)malloc(q * sizeof(unsigned long));
    assert(lTemp2);

    all_countsort_node_aux_seq(q, lKeys, lSorted, auxKey, auxSorted, (1<<11),  0, 11);
    all_countsort_node_aux_seq(q, lSorted, lTemp, auxSorted, lTemp2, (1<<11), 11, 11);
    all_countsort_node_aux_seq(q, lTemp, lSorted, lTemp2, auxSorted, (1<<10), 22, 10);

    free(lTemp);
    free(lTemp2);
}


/* =============================================================================
 * all_radixsort_node_aux_s3
 *
 * q (elems/proc) must be a multiple of NODES
 * =============================================================================
 */
void
all_radixsort_node_aux_s3 (long q,
                           unsigned long* lKeys,
                           unsigned long* lSorted,
                           unsigned long* auxKey,
                           unsigned long* auxSorted)
{
    long myId = local_app_id;

    unsigned long* lTemp  = NULL;
    unsigned long* lTemp2 = NULL;

    if (myId == 0) {
        lTemp = (unsigned long*)P_MALLOC(q * sizeof(unsigned long));
        assert(lTemp);
				#ifdef DEP
		      if(local_replica)
						global_lTemp_b = lTemp;
					else
				#endif
        global_lTemp_a = lTemp;
        lTemp2 = (unsigned long*)P_MALLOC(q * sizeof(unsigned long));
        assert(lTemp2);
				#ifdef DEP
          if(local_replica)
						global_lTemp2_b = lTemp2;
					else
				#endif
        global_lTemp2_a = lTemp2;
    }


		#ifdef DEP
      if(local_replica)
        thread_barrier_wait_b();
      else
        thread_barrier_wait_a();
    #else
    thread_barrier_wait();
    #endif

		#ifdef DEP
      if(local_replica){
				lTemp  = global_lTemp_b;
		    lTemp2 = global_lTemp2_b;
			} else{
		#endif
    lTemp  = global_lTemp_a;
    lTemp2 = global_lTemp2_a;
		#ifdef DEP
		}
		#endif

    all_countsort_node_aux(q, lKeys, lSorted, auxKey, auxSorted, (1<<11),  0, 11);
    all_countsort_node_aux(q, lSorted, lTemp, auxSorted, lTemp2, (1<<11), 11, 11);
    all_countsort_node_aux(q, lTemp, lSorted, lTemp2, auxSorted, (1<<10), 22, 10);


		#ifdef DEP
      if(local_replica)
        thread_barrier_wait_b();
      else
        thread_barrier_wait_a();
    #else
    thread_barrier_wait();
    #endif	

    if (myId == 0) {
        P_FREE(lTemp);
        P_FREE(lTemp2);
    }
}


/* =============================================================================
 *
 * End of alg_radix_smp.c
 *
 * =============================================================================
 */

