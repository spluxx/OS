#include <stdio.h>
#include "thread.h"

// Calling libinit twice

void initial_thread2(void) { }

void initial_thread(void) { 
  if(thread_libinit((thread_startfunc_t) initial_thread2, NULL) == -1) {
    printf("LIBINIT TWICE\n");
  }
}

int main() {
  thread_libinit((thread_startfunc_t) initial_thread, NULL);
  return 0;
}
