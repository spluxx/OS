#include <stdio.h>
#include "thread.h"

void does_weird_things(void) {
  thread_lock(1);
  if(thread_lock(1) == -1) printf("CALLED LOCK TWICE\n");
  thread_unlock(1);
  if(thread_unlock(1) == -1) printf("CALLED UNLOCK WITHOUT A LOCK\n"); 
  thread_unlock(1);
  if(thread_unlock(1) == -1) printf("CALLED UNLOCK WITHOUT A LOCK\n"); 
  if(thread_wait(1, 1) == -1) printf("CALLED WAIT WITHOUT A LOCK\n");
}

void initial_thread(void) {
  thread_create((thread_startfunc_t) does_weird_things, NULL);
}

int main() {
  thread_libinit((thread_startfunc_t) initial_thread, NULL);
  return 0;
}
