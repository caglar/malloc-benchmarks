#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <glib.h>

#define N_ELEMENTS      100000
#define N_SAMPLES       10
#define MAX_SIZE        1500

static gpointer mem [N_ELEMENTS];

static gdouble
difftimeval (struct timeval *old, struct timeval *new)
{
    return (new->tv_sec - old->tv_sec) * 1000. + (new->tv_usec - old->tv_usec) / 1000;
}


static void
test_gslice (gint       n_elements,
         gint   block_size,
         gdouble        *elapsed_alloc,
         gdouble        *elapsed_free)
{
    struct rusage old_usage;
    struct rusage new_usage;
    gint i;
    
    getrusage (RUSAGE_SELF, &old_usage);
    
    for (i = 0; i < n_elements; i++)
        mem [i] = g_slice_alloc (block_size);

    getrusage (RUSAGE_SELF, &new_usage);
    
    *elapsed_alloc = difftimeval (&old_usage.ru_utime, &new_usage.ru_utime) +
             difftimeval (&old_usage.ru_stime, &new_usage.ru_stime);

    getrusage (RUSAGE_SELF, &old_usage);
    
    for (i = 0; i < n_elements; i++)
        g_slice_free1 (block_size, mem [i]);
    
    getrusage (RUSAGE_SELF, &new_usage);
    
    *elapsed_free = difftimeval (&old_usage.ru_utime, &new_usage.ru_utime) +
            difftimeval (&old_usage.ru_stime, &new_usage.ru_stime);
}


static void
test_malloc (gint       n_elements,
         gint   block_size,
         gdouble        *elapsed_alloc,
         gdouble        *elapsed_free)
{
    struct rusage old_usage;
    struct rusage new_usage;
    gint i;
    
    getrusage (RUSAGE_SELF, &old_usage);
    
    for (i = 0; i < n_elements; i++)
        mem [i] = malloc (block_size);

    getrusage (RUSAGE_SELF, &new_usage);
    
    *elapsed_alloc = difftimeval (&old_usage.ru_utime, &new_usage.ru_utime) +
             difftimeval (&old_usage.ru_stime, &new_usage.ru_stime);
    
    getrusage (RUSAGE_SELF, &old_usage);
    
    for (i = 0; i < n_elements; i++)
        free (mem [i]);
    
    getrusage (RUSAGE_SELF, &new_usage);
    
    *elapsed_free = difftimeval (&old_usage.ru_utime, &new_usage.ru_utime) +
            difftimeval (&old_usage.ru_stime, &new_usage.ru_stime);
}


gint
main (gint argc, gchar **argv)
{
    gint i, j;
    gdouble gslice_alloc, gslice_free, malloc_alloc, malloc_free,
        total_gslice_alloc, total_gslice_free, total_malloc_alloc,
        total_malloc_free;
    
    g_print ("Block Size\tGSlice Alloc\tGSlice Free\tMalloc Alloc\tMalloc Free\n");
    
    for (i = 1; i <= MAX_SIZE; i++) {
        total_gslice_alloc = 0;
        total_gslice_free = 0;
        total_malloc_alloc = 0;
        total_malloc_free = 0;
        
        for (j = 0; j < N_SAMPLES; j++) {
            test_gslice (N_ELEMENTS, i, &gslice_alloc, &gslice_free);
            test_malloc (N_ELEMENTS, i, &malloc_alloc, &malloc_free);
            
            total_gslice_alloc += gslice_alloc;
            total_malloc_alloc += malloc_alloc;
            total_gslice_free += gslice_free;
            total_malloc_free += malloc_free;
        }
        
        g_print ("%d\t\t%g\t\t%g\t\t%g\t\t%g\n",
             i,
             (total_gslice_alloc / N_SAMPLES),
             (total_gslice_free / N_SAMPLES),
             (total_malloc_alloc / N_SAMPLES),
             (total_malloc_free / N_SAMPLES));
    }
    
    return 0;
}

