#include <stdio.h>
#include <algorithm>
#include "thread.h"
using namespace std;

// tests arbitrary yield to its limit

const int NTHREADS = 5000;
const int MAX_ID = 19;

int check[MAX_ID];

void some_work(int lc) {
  pair<int, int> lock_cv = make_pair(lc/30,lc%30);
  thread_lock(lock_cv.first);
  thread_signal(lock_cv.first, lock_cv.second);
  for(int i = 1 ; i <= 10 ; i ++) {
    thread_yield();
    check[((lock_cv.first << (i%4) + lock_cv.second*i)+i)%MAX_ID] ++;
    thread_yield();
    if(check[((lock_cv.first << (i%4) + lock_cv.second*i)+i)%MAX_ID] % 5 == 0)
      printf("%d %d\n", lock_cv.first, lock_cv.second);
    thread_yield();
    thread_wait(lock_cv.first, lock_cv.second);
  } 
  thread_unlock(lock_cv.first); 
}

void initial_thread() {
  for(int i = 0 ; i < MAX_ID ; i ++) check[i] ++;

  int id = 13;
  int cv = 7;
  for(int i = 0 ; i < NTHREADS ; i ++) {
    id = (id * 13) % MAX_ID; 
    cv = (cv * 7) % MAX_ID;
    thread_create((thread_startfunc_t) some_work, (void *) (id*30+cv));
  }
}

int main() {
  thread_libinit((thread_startfunc_t) initial_thread, NULL);
  return 0;
}
