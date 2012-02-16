/* Copyright (C) 1997 DJ Delorie, see COPYING.DJ for details */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/times.h>

#define MAXP    400
#define SUBP    20
#define NPASS   25
#define NLOOP   12

double metrics[NLOOP];

int lrand()
{
  static unsigned long long next = 0;
  next = next * 0x5deece66dLL + 11;
  return (int)((next >> 16) & 0x7fffffff);
}

char *pointers[MAXP];
int size[MAXP];

int rsize()
{
  int rv = 8 << (lrand() % 15);
  rv = lrand() & (rv-1);
  return rv;
}

#ifdef TESTMALLOC
#define malloc test_malloc
#define free test_free
#define realloc test_realloc
#endif

int main(int argc, char **argv)
{
  int i, r, loop, pass, subpass;
  int start_time, end_time;
  char *start_mem, *end_mem;
  struct tms tms;
  double elapsed;
  int absmax=0, curmax=0;
  FILE *f;
  int realloc_mask = -1;
  double iscale = 1.5;
  char *metrics_name = "test0.mtr";
  char *ideal_name = "ideal.out";

  if (argc > 1 && strcmp(argv[1], "-r") == 0)
  {
    realloc_mask = 48;
    iscale = 4;
    metrics_name = "test0.rtr";
    ideal_name = "ideal.rout";
  }

  setbuf(stdout, 0);
  memset(pointers, 0, MAXP*sizeof(pointers[0]));

#ifndef TEST0
  f = fopen(metrics_name, "rb");
  if (f)
  {
    setbuf(f, 0);
    fread(metrics, 1, sizeof(metrics), f);
    fclose(f);
  }
  else
    memset(metrics, 0, sizeof(metrics));
#endif

  times(&tms);
  start_time = tms.tms_utime;
  start_mem = sbrk(0);

  for (loop=0; loop<NLOOP; loop++)
  {
    for (pass=0; pass<NPASS; pass++)
    {
      for (subpass=0; subpass<SUBP; subpass++)
      {
    for (i=0; i<MAXP; i++)
    {
      int rno = rand();
      if (rno & 8)
      {
        if (pointers[i])
        {
          if (!(rno & realloc_mask))
          {
        r = rsize();
        curmax -= size[i];
        curmax += r;
        pointers[i] = realloc(pointers[i], rsize());
        size[i] = r;
        if (absmax < curmax) absmax = curmax;
          }
          else
          {
        curmax -= size[i];
        free(pointers[i]);
        pointers[i] = 0;
          }
        }
        else
        {
          r = rsize();
          curmax += r;
          pointers[i] = malloc(r);
          size[i] = r;
          if (absmax < curmax) absmax = curmax;
        }
      }
    }
      }
    }

    times(&tms);
    end_time = tms.tms_utime;
    end_mem = sbrk(0);

    elapsed = ((double)end_time - (double)start_time) / CLOCKS_PER_SEC;
#ifdef TEST0
    printf("%7.2f ", elapsed * iscale);
    printf("%10d\n", curmax/1024);
#else
    printf("%7.2f ", elapsed - metrics[loop]);
    printf("%10d\n", (end_mem-start_mem)/1024);
#endif
    metrics[loop] = elapsed;
  }
  printf("# absmax = %d\n", absmax/1024);

  for (i=0; i<MAXP; i++)
    if (pointers[i])
      free(pointers[i]);

#ifdef TEST0
  f = fopen(ideal_name, "w");
  setbuf(f, 0);
  fprintf(f, "0 %d\n30 %d\n", absmax/1024, absmax/1024);
  fclose(f);

  f = fopen(metrics_name, "wb");
  setbuf(f, 0);
  fwrite(metrics, 1, sizeof(metrics), f);
  fclose(f);
#endif
  return 0;
}

