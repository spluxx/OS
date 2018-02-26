#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <ucontext.h>
#include <queue>
#include <map>
#include <algorithm>
#include "thread.h"
#include "interrupt.h"
using namespace std;

#ifdef DEBUG
  #define DEBUG(M, ...) fprintf(stderr, "[DEBUG] %s:%d: " M "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#else
  #define DEBUG(M, ...)
#endif

typedef unsigned int lock_t;
typedef pair<unsigned int, unsigned int> lock_cv_t;

static int complete_threads;
static map<ucontext_t *, bool> thread_complete; // true if complete

// lock owners
// if available, owner = null
static map<lock_t, ucontext_t *> lock_owner;

// ucontext storages
static ucontext_t *running, *tmp; 
static deque<ucontext_t *> readyQ;
static map<lock_t, deque<ucontext_t *> > wait_lockQ;
static map<lock_cv_t, deque<ucontext_t *> > wait_cvQ;

//------------------------helper functions-------------------------------//

int interrupt_enable(int ret) { interrupt_enable(); return ret; }
int interrupt_disable(int ret) { interrupt_disable(); return ret; }

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
  complete_threads = 0;
}

// function wrapper, forcing the given function to
// 1. run the function with given arguments
// 2. report completion for GC;
// 3. yield CPU to next ready thread.
void func_extend(void *ucp, thread_startfunc_t func, void *arg) {
  interrupt_enable();
  func(arg);
  thread_complete[(ucontext_t *)ucp] = true;
  complete_threads ++;
  thread_yield(); // after freeing whatever, yield the CPU
}

int exit_lib() {
  collect_garbage(true);
  cout << "Thread library exiting.\n";
  exit(0);
  return 0;
}

//------------------------library functions-------------------------------//

int thread_libinit(thread_startfunc_t func, void *arg) { 
  if(tmp != NULL) return -1;
  running = tmp = NULL;
  complete_threads = 0;
  tmp = (ucontext_t *) malloc(sizeof(ucontext_t));
  if(tmp == NULL) return -1;
  if(thread_create(func, arg) == -1) return -1; // create initial thread
  if(thread_yield() == -1) return -1; // start the initial thread
  return -1; // NOT EXECUTED
}

int thread_create(thread_startfunc_t func, void *arg) {
  interrupt_disable();
  if(tmp == NULL) return interrupt_enable(-1);
  void *stk_ptr = malloc(STACK_SIZE); 
  ucontext_t *ucp = (ucontext_t *) malloc(sizeof(ucontext_t));

  if(stk_ptr == NULL || ucp == NULL) {
    free(stk_ptr);
    free(ucp);
    return interrupt_enable(-1);
  }

  // setup context for the new thread
  if(getcontext(ucp) == -1) return -1;
  ucp->uc_stack.ss_sp = stk_ptr;
  ucp->uc_stack.ss_size = STACK_SIZE;
  ucp->uc_stack.ss_flags = 0;
  ucp->uc_link = NULL; 

  makecontext(ucp, (void (*)()) func_extend, 3, ucp, func, arg); 

  thread_complete[ucp] = false;
  readyQ.push_back(ucp);
  return interrupt_enable(0);
}

int thread_yield(void) { 
  interrupt_disable();
  if(tmp == NULL) return interrupt_enable(-1);
  
  if(readyQ.size() == 0) {
    if(thread_complete[running]) exit_lib();
    else return interrupt_enable(0);
  } 

  if(complete_threads > 10) collect_garbage(false);
  
  bool init_thread = running == NULL;
  // DO NOT put back into ready queue 
  // if the current running thread is
  // 1. the initial thread (running = NULL)
  // 2. done running
  bool noswap = init_thread || thread_complete[running];
  if(!noswap) readyQ.push_back(running); 

  // pop a thread to run, setting it as running
  running = readyQ.front(); 
  readyQ.pop_front(); 
  if(swapcontext(noswap ? tmp : readyQ.back(), running) == -1) return -1;
  if(!init_thread) {
    interrupt_enable();
  } return 0;
}

int thread_lock(lock_t lock) {
  interrupt_disable();
  if(tmp == NULL) return interrupt_enable(-1);

  // if lock doesn't even exist within our knowledge, initialize it to NULL
  if(lock_owner.find(lock) == lock_owner.end()) lock_owner[lock] = NULL;
  // if the thread wants a lock when it already holds it - fuck you
  if(lock_owner[lock] == running) return interrupt_enable(-1);
  // if the lock is unavailable, wait
  if(lock_owner[lock] != NULL) {
    wait_lockQ[lock].push_back(running);
    if(readyQ.size() == 0) { interrupt_enable(); exit_lib(); }
    running = readyQ.front();
    readyQ.pop_front();
    if(swapcontext(wait_lockQ[lock].back(), running) == -1) return -1;
  } else lock_owner[lock] = running;
  return interrupt_enable(0);
}

int thread_unlock(lock_t lock) {
  interrupt_disable();
  if(tmp == NULL) return interrupt_enable(-1);
  // if lock doesn't even exist within our knowledge, initialize it to NULL
  if(lock_owner.find(lock) == lock_owner.end()) lock_owner[lock] = NULL;
  // if the thread tries to return a lock it doesn't even own - fuck you
  if(lock_owner[lock] != running) return interrupt_enable(-1);
  // alright this thread actually has a lock - unlock it;
  if(wait_lockQ[lock].size() > 0) {
    lock_owner[lock] = wait_lockQ[lock].front();
    readyQ.push_back(wait_lockQ[lock].front());
    wait_lockQ[lock].pop_front();
  } else lock_owner[lock] = NULL;
  return interrupt_enable(0);
}

int thread_wait(lock_t lock, lock_t cv) {
  interrupt_disable();
  if(tmp == NULL) return interrupt_enable(-1);

  lock_cv_t lock_cv = make_pair(lock, cv);
  // if lock doesn't even exist within our knowledge, initialize it to NULL
  if(lock_owner.find(lock) == lock_owner.end()) lock_owner[lock] = NULL;
  // if the thread doesn't have a lock to begin with - fuck you
  if(lock_owner[lock] != running) return interrupt_enable(-1);
  
  // drop the lock
  if(wait_lockQ[lock].size() > 0) {
    lock_owner[lock] = wait_lockQ[lock].front();
    readyQ.push_back(wait_lockQ[lock].front());
    wait_lockQ[lock].pop_front();
  } else lock_owner[lock] = NULL;

  // and wait
  wait_cvQ[lock_cv].push_back(running);

  // yield
  if(readyQ.size() == 0) { interrupt_enable(); exit_lib(); } 
  running = readyQ.front();
  readyQ.pop_front();
  if(swapcontext(wait_cvQ[lock_cv].back(), running) == -1) return -1;
  interrupt_enable();

  // when it wakes up, pick up the lock
  return thread_lock(lock);
}

int thread_signal(lock_t lock, lock_t cv) {
  interrupt_disable();
  if(tmp == NULL) return interrupt_enable(-1);
  lock_cv_t lock_cv = make_pair(lock, cv);

  // if lock doesn't even exist within our knowledge, initialize it to NULL
  if(lock_owner.find(lock) == lock_owner.end()) lock_owner[lock] = NULL;
  
  // wake a waiting thread up
  if(wait_cvQ[lock_cv].size() == 0) return interrupt_enable(0);
  readyQ.push_back(wait_cvQ[lock_cv].front());
  wait_cvQ[lock_cv].pop_front();
  
  return interrupt_enable(0);
}

int thread_broadcast(lock_t lock, lock_t cv) {
  interrupt_disable();  
  if(tmp == NULL) return interrupt_enable(-1);
  lock_cv_t lock_cv = make_pair(lock, cv);

  // if lock doesn't even exist within our knowledge, initialize it to NULL
  if(lock_owner.find(lock) == lock_owner.end()) lock_owner[lock] = NULL;
  
  // wake every waiting thread up
  int size = wait_cvQ[lock_cv].size();
  for(int i = 0 ; i < size; i ++) {
    readyQ.push_back(wait_cvQ[lock_cv].front());
    wait_cvQ[lock_cv].pop_front();
  }
  
  return interrupt_enable(0);
}
