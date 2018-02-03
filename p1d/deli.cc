#include <stdio.h>
#include <stdlib.h>
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

vector<vector<int> > orders;

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
    printf("NUM_CASH_LOCK MAKER\n");
    breakFlag |= done;
    thread_unlock(NUMBER_OF_CASHIER_LOCK);
    printf("NUM_CASH_UNLOCK MAKER\n");
    if(breakFlag) break;

    int mi = 0;
    int mn = 5000;
    order_info toCook;

    thread_lock(BOARD_LOCK);
    printf("BOARD LOCK MAKER\n");
    while(!full) thread_wait(BOARD_LOCK, WAITING_FULL_BOARD);
    printf("RECEIVED SIGNAL(MAKER)\n");

    for(int i = 0 ; i < board.size() ; i ++) {
      int diff = abs(board[i].sandwich_id - made_last);
      if(diff < mn) {
        mi = i;
        mn = diff;
        toCook = board[i];
      }
    }

    printf("BOARD:\n");
    for(int i = 0 ; i < board.size() ; i ++) {
      printf("CASHIER:%d SANDWICH ID:%d\n", board[i].cashier_id, board[i].sandwich_id); 
    }

    board.erase(board.begin()+mi); // remove from board
    full = false;
    made_last = toCook.sandwich_id; // update latest sandwich made
    order_complete[toCook.cashier_id] = 1; // cooking completed!
    printf("SIGNAL %d %d\n", toCook.cashier_id, order_complete[toCook.cashier_id]);
    thread_signal(BOARD_LOCK, toCook.cashier_id); // signal the cashier

    printf("ORDER_COMPLETE:\n");
    for(int i = 0 ; i < nCashier ; i ++) {
      printf("ORDER FOR CASHIER %d: %d\n", i, order_complete[i]); 
    }

    thread_unlock(BOARD_LOCK);
    printf("BOARD_UNLOCK MAKER\n");
  }

  printf("PEACE!\n");
}

void create_cashier(void* order_file) {
  int myID = -1;
  int current_order = 0;

  // -----------------USING nCashier--------
  thread_lock(NUMBER_OF_CASHIER_LOCK); //   |
  printf("NUM_CASH_LOCK CASHIER\n");
  myID = liveCashier++;		       //   |
  thread_unlock(NUMBER_OF_CASHIER_LOCK); // |
  printf("NUM_CASH_LOCK CASHIER %d\n", myID);
  // ---------------------------------------
  
  while(current_order < orders[myID].size()) {
    thread_lock(BOARD_LOCK); 							
    printf("BOARD_LOCK %d\n", myID);

    if(board.size() < max_orders) { // if there's space on the board post order 
      printf("PUT SANDWICH %d\n", orders[myID][current_order]);
      board.push_back(order_info(myID, orders[myID][current_order]));
      order_complete[myID] = 0; // order is not completed			

      if(board.size() == max_orders) {
	full = true;
	thread_signal(BOARD_LOCK, WAITING_FULL_BOARD); 
	printf("SIGNAL BOARD_FULL %d\n", myID);
      }

      printf("BOARD_WAIT %d\n", myID);
      while(order_complete[myID] == 0) { // keep waiting until we get signal(order complete) from the cooker {
	thread_wait(BOARD_LOCK, myID); // release lock, wait until signal with this ID is sent
      }
      printf("RECEIVED_SIGNAL %d\n", myID);
      current_order ++;
    }

    thread_unlock(BOARD_LOCK);
    printf("BOARD_UNLOCK %d\n", myID);
  }

  // -----------------USING nCashier------------------------
  thread_lock(NUMBER_OF_CASHIER_LOCK); //		  //|
  printf("CHECK EXIT %d < %d\n", liveCashier, max_orders);
  if(liveCashier < max_orders) {
    max_orders = liveCashier;  //|
    printf("UG...\n");

    thread_lock(BOARD_LOCK);
    full = true;
    thread_signal(BOARD_LOCK, WAITING_FULL_BOARD);
    printf("SIGNAL FULL BEFORE EXIT %d\n", myID);
    thread_unlock(BOARD_LOCK);
  }
  if(--liveCashier == 0) done = 1;	//		    |
  thread_unlock(NUMBER_OF_CASHIER_LOCK); //		    |
  // -------------------------------------------------------

  printf("CASHIER %d EXIT\n", myID);
}

void initialize(char* order_files[]) {
  start_preemptions(true, true, 1);
  order_complete = (int *) malloc(sizeof(int)*nCashier);

  for(int i = 2 ; i <= nCashier+1 ; i ++) {
    vector<int> orderForI;
    FILE* fileForI = fopen(order_files[i], "r");
    int sandwich_id = -1;
    while(fscanf(fileForI, "%d", &sandwich_id) != EOF) {
      orderForI.push_back(sandwich_id);
    } orders.push_back(orderForI);
    fclose(fileForI);
  }

  for(int i = 2 ; i <= nCashier+1 ; i ++) 
    thread_create((thread_startfunc_t) create_cashier, fopen(order_files[i], "r"));
  thread_create((thread_startfunc_t) create_maker, NULL);

  free(order_complete);
}

int main(int argc, char* argv[]) {
  nCashier = argc-2;
  liveCashier = 0;
  full = false;
  done = nCashier > 0 ? 0 : 1;
  sscanf(argv[1], "%d", &max_orders);
  thread_libinit((thread_startfunc_t) initialize, argv);
  return 0;
}
