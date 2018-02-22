#include <stdlib.h>
#include <iostream>
#include <ucontext.h>
#include <queue>
#include <map>
#include <algorithm>
#include "thread.h"
using namespace std;

typedef unsigned int lock_t;
typedef pair<unsigned int, unsigned int> lock_cv_t;

map<ucontext_t *, bool> thread_complete; // true if complete

// lock and condition variable owners
// if available, owner = null
map<lock_t, ucontext_t *> lock_owner;
map<lock_cv_t, ucontext_t *> cv_owner;

// ucontext storages
ucontext_t *running, *tmp; 
deque<ucontext_t *> readyQ;
map<lock_t, deque<ucontext_t *> > wait_lockQ;
map<lock_cv_t, deque<ucontext_t *> > wait_cvQ;

//------------------------helper functions-------------------------------//

void free_ucontext(void *ucp) {
  free(((ucontext_t *)ucp)->uc_stack.ss_sp);
  free(ucp);
}

void collect_garbage(bool done) {
  map<ucontext_t *, bool>::iterator it = thread_complete.begin();
  if(done) free(tmp);
  while(it != thread_complete.end()) {
    // if the program's about to exit OR if the thread is completed
    if((it->first != running) && (done || it->second)) {
      free_ucontext(it->first); // free
      thread_complete.erase(it++); // erase from thread_complete
    } else it ++;
  }
}

// function wrapper, forcing the given function to
// 1. run the function with given arguments
// 2. report completion for GC;
// 3. yield CPU to next ready thread.
void func_extend(void *ucp, thread_startfunc_t func, void *arg) {
  func(arg);
  thread_complete[(ucontext_t *)ucp] = true;
  thread_yield(); // after freeing whatever, yield the CPU
}

void exit_lib() {
  // we should probably try one more time to wake threads up
  collect_garbage(true);
  // can't really free the last thread... fuck
  cout << "Thread library exiting.\n";
  exit(0);
}

//------------------------library functions-------------------------------//

int thread_libinit(thread_startfunc_t func, void *arg) { 
  tmp = (ucontext_t *) malloc(sizeof(ucontext_t));
  if(tmp == NULL) return -1;
  running = NULL; // initialize

  thread_create(func, arg); // create initial thread
  thread_yield(); // start the initial thread
  return 0; // NOT EXECUTED
}

int thread_create(thread_startfunc_t func, void *arg) {
  void *stk_ptr = malloc(STACK_SIZE); 
  ucontext_t *ucp = (ucontext_t *) malloc(sizeof(ucontext_t));

  if(stk_ptr == NULL || ucp == NULL) {
    free(stk_ptr);
    free(ucp);
    return -1;
  }

  // setup context for the new thread
  getcontext(ucp);
  ucp->uc_stack.ss_sp = stk_ptr;
  ucp->uc_stack.ss_size = STACK_SIZE;
  ucp->uc_stack.ss_flags = 0;
  ucp->uc_link = NULL; 

  makecontext(ucp, (void (*)()) func_extend, 3, ucp, func, arg); 

  thread_complete[ucp] = false;
  readyQ.push_back(ucp);

  return 0;
}

int thread_yield(void) { 
  if(readyQ.size() == 0) {
    if(thread_complete[running]) exit_lib();
    else return 0;
  } 
  
  if(running != NULL && !thread_complete[running]) // if running thread is neither complete nor nil
    readyQ.push_back(running); // put running thread into ready queue

  running = readyQ.front(); 
  readyQ.pop_front(); // pop one thread to run from ready queue
  return swapcontext(readyQ.size() == 0 ? tmp : readyQ.back(), running);
}

int thread_lock(lock_t lock) {
  // if lock doesn't even exist within our knowledge, initialize it to NULL
  if(lock_owner.find(lock) == lock_owner.end()) lock_owner[lock] = NULL;
  // if the thread wants a lock when it already holds it - fuck you
  if(lock_owner[lock] == running) return -1;
  // if the lock is unavailable, wait
  if(lock_owner[lock] != NULL) {
    wait_lockQ[lock].push_back(running);
    if(readyQ.size() == 0) exit_lib();
    running = readyQ.front();
    readyQ.pop_front();
    return swapcontext(wait_lockQ[lock].back(), running);
  // if the lock is available, yyaaaas!
  } else lock_owner[lock] = running;
  return 0;
}

int thread_unlock(lock_t lock) {
  // if lock doesn't even exist within our knowledge, initialize it to NULL
  if(lock_owner.find(lock) == lock_owner.end()) lock_owner[lock] = NULL;
  // if the thread tries to return a lock it doesn't even own - fuck you
  if(lock_owner[lock] != running) return -1;
  // alright this thread actually has a lock - unlock it;
  if(wait_lockQ[lock].size() > 0) {
    lock_owner[lock] = wait_lockQ[lock].front();
    readyQ.push_back(wait_lockQ[lock].front());
    wait_lockQ[lock].pop_front();
  } else lock_owner[lock] = NULL;
  return 0;
}
