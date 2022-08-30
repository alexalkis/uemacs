#include "usage.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

void ttclose(void);

static void report(const char *prefix, const char *err, va_list params) {
#ifdef AMIGA
  char msg[1024];
#else
  char msg[4096];
#endif
  vsnprintf(msg, sizeof(msg), err, params);
  fprintf(stderr, "%s%s\n", prefix, msg);
}

void die(const char *err, ...) {
  va_list params;

  va_start(params, err);
  report("fatal: ", err, params);
  va_end(params);
  ttclose();
  exit(128);
}
