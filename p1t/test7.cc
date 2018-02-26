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
  if(thread_libinit((thread_startfunc_t) does_weird_things, NULL) == -1)
    printf("CALLED LIBINIT TWICE\n");
  thread_create((thread_startfunc_t) does_weird_things, NULL);
}

int main() {
  if(thread_create((thread_startfunc_t) does_weird_things, NULL) == -1) 
    printf("CREATE CALLED BEFORE CALLING LIBINIT\n");
  if(thread_yield() == -1) 
    printf("YIELD CALLED BEFORE CALLING LIBINIT\n");
  if(thread_lock(1) == -1) 
    printf("LOCK CALLED BEFORE CALLING LIBINIT\n");
  if(thread_unlock(1) == -1) 
    printf("UNLOCK CALLED BEFORE CALLING LIBINIT\n");
  if(thread_wait(1, 1) == -1) 
    printf("WAIT CALLED BEFORE CALLING LIBINIT\n");
  if(thread_signal(1, 1) == -1) 
    printf("SIGNAL CALLED BEFORE CALLING LIBINIT\n");
  if(thread_broadcast(1, 1) == -1) 
    printf("BROADCAST CALLED BEFORE CALLING LIBINIT\n");
  thread_libinit((thread_startfunc_t) initial_thread, NULL);
  return 0;
}
