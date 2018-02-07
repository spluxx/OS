#include <stdlib.h>
#include <ucontext.h>
#include <signal.h>
#include <list>
#include <queue>
#include <map>
using namespace std;

#include "thread.h"

map<void*, bool> thread_complete; // 0 incomplete 1 complete
map<int, bool> lock_available;
queue<ucontext_t *, deque<ucontext_t *> > readyQ, runningQ, sleepingQ;

void func_extend(void* ucp, thread_startfunc_t func, void* arg) {
  func(arg);
  thread_complete[ucp] = 1;
  free(((ucontext_t *)ucp)->uc_stack.ss_sp);
  free(ucp);
}

int thread_libinit(thread_startfunc_t func, void *arg) { thread_create(func, arg); }

int thread_create(thread_startfunc_t func, void *arg) {
  void* stk_ptr = malloc(STACK_SIZE); 
  ucontext_t *ucp = (ucontext_t *) malloc(sizeof(ucontext_t));

  if(stk_ptr == NULL || ucp == NULL) {
    free(stk_ptr);
    free(ucp);
    return -1;
  }

  getcontext(ucp);

  ucp->uc_stack.ss_sp = stk_ptr;
  ucp->uc_stack.ss_size = STACK_SIZE;
  ucp->uc_stack.ss_flags = 0;
  ucp->uc_link = NULL;

  makecontext(ucp, (void (*)()) func_extend, 3, ucp, func, arg);
  readyQ.push(ucp);

  return 0;
}

int thread_yield(void) {
  waitingQ.push(
}
