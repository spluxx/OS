#include <stdio.h>
#include "thread.h"

const int MAX_THREADS = 20;
int threads = MAX_THREADS;
bool barrier_down;

void barrier_thread(void) {
  thread_lock(1);
  while(threads > 0) thread_wait(1, 1);
  barrier_down = true;
  printf("Barrier: everyone reached the first goal\n");
  thread_broadcast(1, 1);
  thread_unlock(1);
}

void barriered_thread(int id) {
  thread_lock(1);
  if(--threads == 0) thread_broadcast(1, 1);
  printf("%d: first goal\n", id);
  while(!barrier_down) thread_wait(1, 1);
  printf("%d: second goal\n", id); 
  thread_unlock(1);
}

void initial_thread(void) {
  barrier_down = false;
  thread_create((thread_startfunc_t) barrier_thread, NULL);
  for(int i = 0 ; i < MAX_THREADS ; i ++)
    thread_create((thread_startfunc_t) barriered_thread, (void *)i);
}

int main() {
  thread_libinit((thread_startfunc_t) initial_thread, NULL);
  return 0;
} 
