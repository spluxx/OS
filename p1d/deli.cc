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
// ---------------------------------------
const int PROCESS = 3;
const int WAITING_FULL_BOARD = 4;

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
bool full;

// GOVERNED BY NUMBER_OF_CASHIER_LOCK --------------
int liveCashier, done;	    //      |
// -------------------------------------------------

// GOVERNED BY BOARD_LOCK ---------------------------
vector<order_info> board; //			     | 
int *order_complete; // 0 (incomplete) 1 (complete)  |
// --------------------------------------------------

void create_maker() {
  int made_last = -1;
  bool breakFlag = false;

  while(true) {
    thread_lock(NUMBER_OF_CASHIER_LOCK);
    breakFlag |= done;
    thread_unlock(NUMBER_OF_CASHIER_LOCK);
    if(breakFlag) break;

    int mi = 0;
    int mn = 5000;
    order_info toCook;

    thread_lock(BOARD_LOCK);
    while(!full) thread_wait(BOARD_LOCK, WAITING_FULL_BOARD);

    for(int i = 0 ; i < board.size() ; i ++) {
      int diff = abs(board[i].sandwich_id - made_last);
      if(diff < mn) {
        mi = i;
        mn = diff;
        toCook = board[i];
      }
    }

    board.erase(board.begin()+mi); // remove from board
    full = false;
    made_last = toCook.sandwich_id; // update latest sandwich made
    order_complete[toCook.cashier_id] = 1; // cooking completed!
    thread_signal(BOARD_LOCK, toCook.cashier_id); // signal the cashier
    cout << "READY: cashier " << toCook.cashier_id << " sandwich " << toCook.sandwich_id << endl; 


    thread_unlock(BOARD_LOCK);
  }

  free(order_complete);
}

void create_cashier(FILE* order_file) {
  int myID = -1;
  int current_order = 0;

  // -----------------USING nCashier--------
  thread_lock(NUMBER_OF_CASHIER_LOCK); //   |
  myID = liveCashier++;		       //   |
  thread_unlock(NUMBER_OF_CASHIER_LOCK); // |
  // ---------------------------------------
  
  int sandwich = -1;
  while(fscanf(order_file, "%d", &sandwich) != EOF) {
    thread_lock(BOARD_LOCK); 							

    if(board.size() < max_orders) { // if there's space on the board post order 
      board.push_back(order_info(myID, sandwich));
      order_complete[myID] = 0; // order is not completed			
      cout << "POSTED: cashier " << myID << " sandwich " << sandwich << endl; 

      if(board.size() == max_orders) {
	full = true;
	thread_signal(BOARD_LOCK, WAITING_FULL_BOARD); 
      }

      while(order_complete[myID] == 0) { // keep waiting until we get signal(order complete) from the cooker {
	thread_wait(BOARD_LOCK, myID); // release lock, wait until signal with this ID is sent
      }
      current_order ++;
    }

    thread_unlock(BOARD_LOCK);
  }

  // -----------------USING nCashier------------------------
  thread_lock(NUMBER_OF_CASHIER_LOCK); //		  //|
  if(--liveCashier == 0) done = 1;	//		    |
  else if(liveCashier < max_orders) {
    max_orders = liveCashier;  //|

    thread_lock(BOARD_LOCK);
    full = true;
    thread_signal(BOARD_LOCK, WAITING_FULL_BOARD);
    thread_unlock(BOARD_LOCK);
  }
  thread_unlock(NUMBER_OF_CASHIER_LOCK); //		    |
  // -------------------------------------------------------

  fclose(order_file);
}

void initialize(char* order_files[]) {
  //start_preemptions(true, true, 1);
  order_complete = (int *) malloc(sizeof(int)*nCashier);

  for(int i = 2 ; i <= nCashier+1 ; i ++) {
    thread_create((thread_startfunc_t) create_cashier, fopen(order_files[i], "r"));
  }
  thread_create((thread_startfunc_t) create_maker, NULL);
}

int main(int argc, char* argv[]) {
  nCashier = argc-2;
  liveCashier = 0;
  full = false;
  done = nCashier > 0 ? 0 : 1;
  sscanf(argv[1], "%d", &max_orders);
  max_orders = max_orders < nCashier ? max_orders : nCashier;
  thread_libinit((thread_startfunc_t) initialize, argv);
  return 0;
}
