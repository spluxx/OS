#include <stdio.h>
#include <stdlib.h>
#include "thread.h"

void threadB() {
  thread_lock(2);
  printf("B\n");
  if(thread_unlock(1) == -1) printf("FUCK B\n");
  thread_unlock(2);
}

void threadA() {
  thread_lock(1);
  printf("A\n");
  if(thread_unlock(2) == -1) printf("FUCK A\n");
  thread_unlock(1);
}

void initial_thread(void) { 
  thread_yield();
  thread_create((thread_startfunc_t) threadA, NULL);
  thread_yield();
  thread_yield();
  thread_yield();
  thread_yield();
  thread_yield();
  thread_yield();
  thread_create((thread_startfunc_t) threadB, NULL);
  thread_yield();
  thread_yield();
  thread_yield();
  thread_yield();
  thread_yield();
  thread_yield();
}

int main() {
  if(thread_create((thread_startfunc_t) threadA, NULL) == -1) printf("WHAT\n");
  thread_libinit((thread_startfunc_t) initial_thread, NULL);
  return 0;
}
