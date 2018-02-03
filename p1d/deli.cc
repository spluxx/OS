#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <vector>
#include "thread.h"
using namespace std;

int abs(int a) { return a > 0 ? a : -a; }

// ------- Locks -------------------------
const int BOARD_LOCK = 1; //		  |
const int NUMBER_OF_CASHIER_LOCK = 2; //  |
const int MONITOR_LOCK = 3;
// ------- Condition Variables -----------
const int BOARD_FULL = 4; //		  |
const int BOARD_NOT_FULL = 5; //	  |
// ---------------------------------------


struct order_info {
  int cashier_id;
  int sandwich_id;
  order_info(){}
  order_info(int c, int s) { 
    cashier_id = c;
    sandwich_id = s;
  }
};

int max_orders, nCashier; 
bool full, done;

// GOVERNED BY NUMBER_OF_CASHIER_LOCK --------------
int liveCashier;			    //      |
// -------------------------------------------------

// GOVERNED BY BOARD_LOCK ---------------------------
vector<order_info> board; //			     | 
int *order_complete; // 0 (incomplete) 1 (complete)  |
// --------------------------------------------------

void create_maker() {
  int made_last = -1;

  while(true) {
    int mi = 0;
    int mn = 5000;
    order_info toCook;

    thread_lock(BOARD_LOCK);
    // wait for board_full signal
    while(!full) thread_wait(BOARD_LOCK, BOARD_FULL);	
    if(done) break;

    // find closest sandwich from made_last
    for(int i = 0 ; i < board.size() ; i ++) {		
      int diff = abs(board[i].sandwich_id - made_last);
      if(diff < mn) {
        mi = i;
        mn = diff;
        toCook = board[i];
      }
    }

    board.erase(board.begin()+mi); // remove from board
    made_last = toCook.sandwich_id; // update latest sandwich made
    order_complete[toCook.cashier_id] = 1; // cooking completed!

    thread_lock(MONITOR_LOCK);
    cout << "READY: cashier " << toCook.cashier_id << " sandwich " << toCook.sandwich_id << endl; 
    thread_unlock(MONITOR_LOCK);

    full = false;
    thread_broadcast(BOARD_LOCK, BOARD_NOT_FULL); // broadcast board_not_full signal

    thread_unlock(BOARD_LOCK);
  }

  free(order_complete);
}

void create_cashier(FILE* order_file) {
  int myID = -1;

  thread_lock(NUMBER_OF_CASHIER_LOCK); 
  myID = liveCashier++;		       
  thread_unlock(NUMBER_OF_CASHIER_LOCK);
  
  int sandwich = -1;
  order_complete[myID] = 1;

  // read order one by one
  while(fscanf(order_file, "%d", &sandwich) != EOF) {
    thread_lock(BOARD_LOCK); 							

    // the previous order must have been completed
    // to wait for board_not_full broadcast 
    while(full || order_complete[myID] == 0) thread_wait(BOARD_LOCK, BOARD_NOT_FULL);

    board.push_back(order_info(myID, sandwich));	  // post an order
    order_complete[myID] = 0;

    thread_lock(MONITOR_LOCK);
    cout << "POSTED: cashier " << myID << " sandwich " << sandwich << endl; 
    thread_unlock(MONITOR_LOCK);

    if(board.size() == max_orders) {
      full = true; 
      // send board_full signal
      thread_signal(BOARD_LOCK, BOARD_FULL);
    }

    thread_unlock(BOARD_LOCK);
  }

  // wait for final order
  thread_lock(BOARD_LOCK);
  while(order_complete[myID] == 0) thread_wait(BOARD_LOCK, BOARD_NOT_FULL);
  thread_unlock(BOARD_LOCK);

  // cashier can go home now
  thread_lock(NUMBER_OF_CASHIER_LOCK); 
  if(--liveCashier < max_orders) {
    max_orders = liveCashier;  
    if(liveCashier == 0) done = true;

    thread_lock(BOARD_LOCK);
    full = true;
    thread_signal(BOARD_LOCK, BOARD_FULL);
    thread_unlock(BOARD_LOCK);
  }
  thread_unlock(NUMBER_OF_CASHIER_LOCK);

  fclose(order_file);
}

void initialize(char* order_files[]) {
  start_preemptions(true, true, 1);
  order_complete = (int *) malloc(sizeof(int)*nCashier);
  for(int i = 2 ; i <= nCashier+1 ; i ++)
    thread_create((thread_startfunc_t) create_cashier, fopen(order_files[i], "r"));
  thread_create((thread_startfunc_t) create_maker, NULL);
}

int main(int argc, char* argv[]) {
  nCashier = argc-2;
  liveCashier = 0;
  full = false;
  sscanf(argv[1], "%d", &max_orders);
  done = nCashier <= 0;
  max_orders = max_orders < nCashier ? max_orders : nCashier;

  thread_libinit((thread_startfunc_t) initialize, argv);
  return 0;
}
