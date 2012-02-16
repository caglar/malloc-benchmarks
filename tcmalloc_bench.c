/*
 *  malloc-test
 *  This code is based on the one at: http://www.citi.umich.edu/projects/linux-scalability/reports/malloc.html
 *  Syntax:
 *  malloc-test [ size [ iterations [ thread count ]]]
 *
 */

#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdlib.h>

#include <pthread.h>

#define USECSPERSEC 1000000
#define pthread_attr_default NULL
#define MAX_THREADS 50

static unsigned size = 512;
static unsigned iteration_count = 1000000;

/**
 * this function is for computing the time difference between timeval x and y
 * the result is stored in result
 */
int timeval_subtract (struct timeval *result, struct timeval *x, struct timeval *y)
{
    /* Perform the carry for the later subtraction by updating y. */
    if (x->tv_usec < y->tv_usec) {
        int nsec = (y->tv_usec - x->tv_usec) / 1000000 + 1;
        y->tv_usec -= 1000000 * nsec;
        y->tv_sec += nsec;
    }
    if (x->tv_usec - y->tv_usec > 1000000) {
        int nsec = (x->tv_usec - y->tv_usec) / 1000000;
        y->tv_usec += 1000000 * nsec;
        y->tv_sec -= nsec;
    }

    /* Compute the time remaining to wait.
    tv_usec is certainly positive. */
    result->tv_sec = x->tv_sec - y->tv_sec;
    result->tv_usec = x->tv_usec - y->tv_usec;

    /* Return 1 if result is negative. */
    return x->tv_sec < y->tv_sec;
}

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
        buf = dummy(i);
    }

    gettimeofday(&end, NULL);
    
    timeval_subtract(&null, &end, &start);

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

    timeval_subtract(&elapsed, &end, &start);

    /*
     * Adjust elapsed time by null loop time
     */
    timeval_subtract(&adjusted, &elapsed, &null);
    printf("Thread %lo adjusted timing: %ld.%06ld seconds for %d requests"
        " of %d bytes.\n", pthread_self(),
        elapsed.tv_sec, elapsed.tv_usec, 
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

    return 0;
}
