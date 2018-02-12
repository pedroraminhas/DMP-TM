#ifndef MAIN_H
#define MAIN_H

// Constants
#define MB 1048576                                  /* Number of bytes in 1MB*/
#define PAGE_SIZE getpagesize()                      /* Size of a page */
#define NUM_ELEMENTS_PAGE PAGE_SIZE/sizeof(long)      /* Number of longs in a page */
#define NUM_ELEMENTS_ARRAY MB/sizeof(long)           /* Number of longs in 1MB    */
#define NUM_PAGES MB/PAGE_SIZE                      /* Number of pages in 1MB */

extern int system_call_counter[16];
extern int transition_count_counter[16];


#include "tm.h"

/* Test functions prototypes */
void* test_HTM_read_after_STM_write(void* data);
void* test_HTM_read_after_STM_read(void* data);
void* test_STM_read_after_STM_read(void *data);
void* test_STM_read_after_STM_write_after_STM_write(void *data);
void* test_HTM_write_after_STM_read(void *data);
void* test_STM_read_after_STM_write_after_STM_read(void *data);
void* test_HTM_read_after_HTM_write(void *data);
void* test_HTM_write_after_STM_write_different_page(void *data);
void* test_STM_write_after_STM_write(void *data);
void* test_STM_aborts_transition_count_changes(void *data);
void* test_HTM_have_to_wait_for_writers_to_restore_protection(void *data);
void* test_STM_waits_to_set_the_page_protection(void *data);
void* test_STM_breaks_opacity(void* data);
void* test_STM_write_middle_htm_write(void* data);
void* test_STM_first_aborts_then_reads(void* data);
void* test_HTM_exhausts_transition_count_fallbacks_STM(void *data);
void* single_transaction_bank_benchmark(void *data);
void* triple_transaction_bank_benchmark(void *data);
void* test_HTM_write_after_HTM_write_after_STM_read(void *data);

#endif
