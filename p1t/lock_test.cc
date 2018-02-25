#include <stdio.h>
#include <iostream>
#include <vector>
#include "thread.h"
using namespace std;

#define LOCK 1

const int NTHREADS = 100;
vector<int> mark;

void mini(int i) {
  thread_lock(LOCK);
  mark.push_back(i);
  printf("%d %lu\n", i, mark.size());
  thread_unlock(LOCK);
}

void initial_thread() {
  for(int i = 0 ; i < NTHREADS ; i ++)
    thread_create((thread_startfunc_t) mini, (void*) i);
}

int main() {
  thread_libinit((thread_startfunc_t) initial_thread, NULL);
  return 0;
}
