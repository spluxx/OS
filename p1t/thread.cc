#include <ucontext.h>
#include <list>
#include <queue>
#include <map>
using namespace std;

/*
#define STACK_SIZE 262144	size of each thread's stack

typedef void (*thread_startfunc_t) (void *);

extern int thread_libinit(thread_startfunc_t func, void *arg);
extern int thread_create(thread_startfunc_t func, void *arg);
extern int thread_yield(void);
*/

map<void*, bool> func_complete; // 0 incomplete 1 complete
queue<uncontext_t *, deque<uncontext_t *> > readyQ, runningQ, sleepingQ;

thread_startfun_t func_extend(void* ucp_ptr, thread_startfun_t func, void* arg) {
  func(arg);
  func_complete[stk_ptr] = 1;
  free(((ucontext_t *)ucp_ptr)->uc_stack->ss_sp);
  free(((ucontext_t *)ucp_ptr)->uc_stack);
  free(ucp_ptr);
}

int thread_create(thread_startfun_t func, void *arg) {
  void* stk_ptr = malloc(STACK_SIZE); 
  ucontext_t *ucp = (ucontext_t *) malloc(sizeof(ucontext_t));
  stack_t *stp = (stack_t *) malloc(sizeof(stack_t));

  if(stk_ptr == NULL || ucp == NULL || stp == NULL) {
    free(stk_ptr);
    free(ucp);
    free(stp);
    return -1;
  }

  stp->ss_sp = stk_ptr;
  stp->ss_flags = 0;
  stp->ss_size = STACK_SIZE;
  ucp->uc_stack = stp;

  makecontext(ucp, func_extend(ucp, func, arg));
  readyQ.push(ucp);

  return 0;
}
