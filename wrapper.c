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

void *xmalloc(size_t size) {
  void *ret = malloc(size);
  if (!ret)
    die("Out of memory");
  return ret;
}
