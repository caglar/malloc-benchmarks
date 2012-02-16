/*
 *  malloc-test
 *  This code is based on the one at: http://www.citi.umich.edu/projects/linux-scalability/reports/malloc.html
 *  Syntax:
 *  malloc-test [ size [ iterations [ thread count ]]]
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>

#include <pthread.h>

#define USECSPERSEC 1000000
#define pthread_attr_default NULL
#define MAX_THREADS 50

static unsigned size = 512;
static unsigned iteration_count = 1000000;

void * dummy(unsigned i)
{
    return NULL;
}

void run_test (void)
{
    unsigned int i;
    unsigned request_size = size;
    unsigned total_iterations = iteration_count;
    struct timeval start, end, null, elapsed, adjusted;

    /*
     * Time a null loop.  We'll subtract this from the final
     * malloc loop results to get a more accurate value.
     */
    gettimeofday(&start, NULL);

    for (i = 0; i < total_iterations; i++) {
        void * buf;
        buf = dummy(i);
        buf = dummy(i);
    }

    gettimeofday(&end, NULL);

    null.tv_sec = end.tv_sec - start.tv_sec;
    null.tv_usec = end.tv_usec - start.tv_usec;
    if (null.tv_usec < 0) {
        null.tv_sec--;
        null.tv_usec += USECSPERSEC;
    }

    /*
     * Run the real malloc test
     */
    gettimeofday(&start, NULL);

    for (i = 0; i < total_iterations; i++) {
        void * buf;
        buf = malloc(request_size);
        buf = realloc(buf, request_size*2);
        free(buf);
    }

    gettimeofday(&end, NULL);

    elapsed.tv_sec = end.tv_sec - start.tv_sec;
    elapsed.tv_usec = end.tv_usec - start.tv_usec;
    if (elapsed.tv_usec < 0) {
        elapsed.tv_sec--;
        elapsed.tv_usec += USECSPERSEC;
    }

    /*
     * Adjust elapsed time by null loop time
     */
    adjusted.tv_sec = elapsed.tv_sec - null.tv_sec;
    adjusted.tv_usec = elapsed.tv_usec - null.tv_usec;
    if (adjusted.tv_usec < 0) {
        adjusted.tv_sec--;
        adjusted.tv_usec += USECSPERSEC;
    }
    printf("Thread %lo adjusted timing: %ld.%06ld seconds for %d requests"
        " of %d bytes.\n", pthread_self(),
        adjusted.tv_sec, adjusted.tv_usec, 
        total_iterations,
        (int) request_size);

    pthread_exit(NULL);
}



int main (int argc, char *argv[])
{
    unsigned i;
    unsigned thread_count = 1;
    pthread_t thread[MAX_THREADS];

    /*
     * Parse our arguments
     */
    switch (argc) {
    case 4:
        /* size, iteration count, and thread count were specified */
        thread_count = atoi(argv[3]);
        if (thread_count > MAX_THREADS) thread_count = MAX_THREADS;
    case 3:
        /* size and iteration count were specified; others default */
        iteration_count = atoi(argv[2]);
    case 2:
        /* size was specified; others default */
        size = atoi(argv[1]);
    case 1:
        /* use default values */
        break;
    default:
        printf("Unrecognized arguments.\n");
        exit(1);
    }

    /*
     * Invoke the tests
     */
    printf("Starting test...\n");
    for (i=1; i<=thread_count; i++)
        if (pthread_create(&(thread[i]), pthread_attr_default,
                    (void *) &run_test, NULL))
            printf("failed.\n");

    /*
     * Wait for tests to finish
     */
    for (i=1; i<=thread_count; i++)
        pthread_join(thread[i], NULL);

    exit(0);
}
