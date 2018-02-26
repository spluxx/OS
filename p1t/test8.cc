#include <stdio.h>
#include "thread.h"

const int NTHREADS = 100000;

void threads() { }

void init() {
  for(int i = 0 ; i < NTHREADS ; i ++) {
    if(i % 50000 == 0) thread_yield();
    thread_create((thread_startfunc_t) threads, NULL);
  }
}

int main() {
  thread_libinit((thread_startfunc_t) init, NULL);
  return 0;
}
