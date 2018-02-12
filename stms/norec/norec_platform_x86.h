/* =============================================================================
 *
 * platform_x86.h
 *
 * x86-specific bindings
 *
 * =============================================================================
 */


#ifndef norec_PLATFORM_X86_H
#define norec_PLATFORM_X86_H 1


#include <stdint.h>
#include "norec_common.h"

/* =============================================================================
 * Compare-and-swap
 *
 * CCM: Notes for implementing CAS on x86:
 *
 * /usr/include/asm-x86_64/system.h
 * http://www-128.ibm.com/developerworks/linux/library/l-solar/
 * http://gcc.gnu.org/onlinedocs/gcc-4.1.0/gcc/Atomic-Builtins.html#Atomic-Builtins
 *
 * In C, CAS would be:
 *
 * static __inline__ intptr_t cas(intptr_t newv, intptr_t old, intptr_t* ptr) {
 *     intptr_t prev;
 *     pthread_mutex_lock(&lock);
 *     prev = *ptr;
 *     if (prev == old) {
 *         *ptr = newv;
 *     }
 *     pthread_mutex_unlock(&lock);
 *     return prev;
 * =============================================================================
 */
/*norec___INLINE__ intptr_t
cas (intptr_t newVal, intptr_t oldVal, volatile intptr_t* ptr)
{
    intptr_t prevVal;

//    __asm__ __volatile__ (
//        "lock \n"
//#ifdef __LP64__
//        "cmpxchgq %1,%2 \n"
//#else
//        "cmpxchgl %k1,%2 \n"
//#endif
//        : "=a" (prevVal)
//        : "q"(newVal), "m"(*ptr), "0" (oldVal)
//        : "memory"
//    );

	//int fail;
        __asm__ __volatile__ (
           "0: lwarx %0, 0, %1\n\t"
                 "      xor. %0, %3, %0\n\t"
              " bne 1f\n\t"
            " stwcx. %2, 0, %1\n\t"
                 "      bne- 0b\n\t"
            " isync\n\t"
        "1: "
        : "=&r"(prevVal)
        : "r"(ptr), "r"(newVal), "r"(oldVal)
        : "cr0");

    return prevVal;
}
pthread_mutex_t cas_mutex;
norec___INLINE__ intptr_t cas(intptr_t newv, intptr_t old, intptr_t* ptr) {
     intptr_t prev;
     pthread_mutex_lock(&cas_mutex);
     prev = *ptr;
     if (prev == old) {
          *ptr = newv;
     }
     pthread_mutex_unlock(&cas_mutex);
     return prev;
}*/

pthread_mutex_t cas_mutex;
norec___INLINE__ intptr_t cas(intptr_t newval, intptr_t old, intptr_t* p) {
		int result = 0;
		intptr_t oldval;
		/*__asm__ __volatile__ (
                "0: lwarx %0, 0, %1\n\t"
                "      xor. %0, %3, %0\n\t"
                " bne 1f\n\t"
                " stwcx. %2, 0, %1\n\t"
                "      bne- 0b\n\t"
                " isync\n\t"
                "1: "
                : "=&r"(fail)
                : "r"(p), "r"(newval), "r"(oldval)
                : "cr0");*/
  		__asm__ __volatile__(
               "1:ldarx %0,0,%2\n"   /* load and reserve              */
               "cmpd %0, %4\n"      /* if load is not equal to  */
               "bne 2f\n"            /*   old, fail                     */
               "stdcx. %3,0,%2\n"    /* else store conditional         */
               "bne- 1b\n"           /* retry if lost reservation      */
               "li %1,1\n"           /* result = 1;                     */
               "2:\n"
              : "=&r"(oldval), "=&r"(result)
              : "r"(p), "r"(newval), "r"(old), "1"(result)
              : "memory", "cr0");

		return oldval;
}


/* =============================================================================
 * Memory Barriers
 *
 * http://mail.nl.linux.org/kernelnewbies/2002-11/msg00127.html
 * =============================================================================
 */
#define norec_MEMBARLDLD()                    /* nothing */
#define norec_MEMBARSTST()                    /* nothing */
#define norec_MEMBARSTLD()                    __asm__ __volatile__ ("sync" : : :"memory")


/* =============================================================================
 * Prefetching
 *
 * We use PREFETCHW in LD...CAS and LD...ST circumstances to force the $line
 * directly into M-state, avoiding RTS->RTO upgrade txns.
 * =============================================================================
 */
#ifndef ARCH_HAS_PREFETCHW
norec___INLINE__ void
prefetchw (volatile void* x)
{
    /* nothing */
}
#endif


/* =============================================================================
 * Non-faulting load
 * =============================================================================
 */
#define norec_LDNF(a)                         (*(a)) /* CCM: not yet implemented */


/* =============================================================================
 * MP-polite spinning
 *
 * Ideally we would like to drop the priority of our CMT strand.
 * =============================================================================
 */
#define norec_PAUSE()                         /* nothing */


/* =============================================================================
 * Timer functions
 * =============================================================================
 */
/* CCM: low overhead timer; also works with simulator */
/*#define norec_TL2_TIMER_READ() ({ \
    unsigned int lo; \
    unsigned int hi; \
    __asm__ __volatile__ ("rdtsc" : "=a" (lo), "=d" (hi)); \
    ((TL2_TIMER_T)hi) << 32 | lo; \
})*/

/*
Version with clock_gettime:
*/
#define norec_TL2_TIMER_READ() ({ \
      struct timespec time; \
      clock_gettime(CLOCK_MONOTONIC, &time); \
      (long)time.tv_sec * 1000000000L + (long)time.tv_nsec; \
})


/*
Version with gettimeofday:

#define TL2_TIMER_READ() ({ \

    struct timeval time; \

    gettimeofday(&time, NULL); \

    (long)time.tv_sec * 1000000L + (long)time.tv_usec; \

})
*/


#endif /* PLATFORM_X86_H */


/* =============================================================================
 *
 * End of platform_x86.h
 *
 * =============================================================================
 */
