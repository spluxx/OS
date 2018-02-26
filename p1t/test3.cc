#include <stdio.h>
#include "thread.h"

const int PINGPONGS = 10;
const int LOOPS = 10;

void ping_pong(int cv) {
  thread_lock(1);
  for(int i = 0 ; i < LOOPS ; i ++) {
    thread_signal(1, (cv+1)%PINGPONGS);
    printf("%d\n", cv);
    thread_wait(1, cv);
  }
  thread_unlock(1);
}

void init_thread() {
  //start_preemptions(true, true, 235203);
  for(int i = 0 ; i < PINGPONGS*10 ; i ++)
    thread_create((thread_startfunc_t) ping_pong, (void *) (i%PINGPONGS));
}

int main() {
  thread_libinit((thread_startfunc_t) init_thread, NULL); 
  return 0;
}
