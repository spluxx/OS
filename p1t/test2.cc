#include <stdio.h>
#include <algorithm>
#include "thread.h"
using namespace std;

// tests arbitrary yield to its limit

const int NTHREADS = 100;
const int MAX_ID = 19;

int check[MAX_ID];

void some_work(int lc) {
  thread_yield();
  pair<int, int> lock_cv = make_pair(lc/30,lc%30);
  thread_lock(lock_cv.first);
  thread_yield();
  thread_signal(lock_cv.first, lock_cv.second);
  thread_yield();
  for(int i = 1 ; i <= 100 ; i ++) {
    thread_yield();
    check[(((lock_cv.first << (i%4)) + lock_cv.second*i)+i)%MAX_ID] ++;
    thread_yield();
    thread_broadcast(lock_cv.first*7, lock_cv.second*13);
    if(check[(((lock_cv.first << (i%4)) + lock_cv.second*i)+i)%MAX_ID] % 5 == 0)
      printf("%d %d\n", lock_cv.first, lock_cv.second);
    thread_yield();
    thread_wait(lock_cv.first, lock_cv.second);
    thread_yield();
  } 
  thread_yield();
  thread_unlock(lock_cv.first); 
}

void initial_thread() {
  //start_preemptions(true, true, 20812058);
  for(int i = 0 ; i < MAX_ID ; i ++) check[i] ++;
  thread_yield();
  int id = 13;
  thread_yield();
  int cv = 7;
  thread_yield();
  for(int i = 0 ; i < NTHREADS ; i ++) {
    thread_yield();
    id = (id * 13) % MAX_ID; 
    thread_yield();
    cv = (cv * 7) % MAX_ID;
    thread_yield();
    thread_create((thread_startfunc_t) some_work, (void *) (id*30+cv));
    thread_yield();
  }
}

int main() {
  thread_libinit((thread_startfunc_t) initial_thread, NULL);
  return 0;
}
