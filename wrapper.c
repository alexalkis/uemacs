#include "usage.h"
#include <stdlib.h>

/* Function copyright: git */
int xmkstemp(char *template) {
  int fd;

#ifdef AMIGA
  fd = mkstemp(template);
#else
  fd = mkstemp(template);
#endif
  if (fd < 0)
    die("Unable to create temporary file");
  return fd;
}

#include <stdio.h>
void *xmalloc(size_t size) {
  static int c = 0;
  void *ret = malloc(size);
  if (!ret) {
#ifdef AMIGA
    printf("Memory allocation failed for %d bytes.\n", size);
#endif
    die("Out of memory");
  }
  //fprintf(stderr, "Got mem %d, req# %d\n", size, ++c);
  return ret;
}
