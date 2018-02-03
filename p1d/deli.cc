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

struct order_info {
  int cashier_id;
  int sandwich_id;
};

vector<vector<int> > orders;

int max_orders, nCashier; 

// GOVERNED BY NUMBER_OF_CASHIER_LOCK --------------
int liveCashier, done;	    //      |
// -------------------------------------------------

// GOVERNED BY BOARD_LOCK ---------------------------
vector<order_info*> board; //			     | 
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

    printf("BOARD_LOCK MAKER\n");
    thread_lock(BOARD_LOCK);
    if(board.size() == max_orders) {
      int mi = 0;
      int mn = 5000;
      order_info* toCook = NULL;

      printf("BOARD: ");
      for(int i = 0 ; i < board.size() ; i ++) {
	printf("cashier=%d sandwich=%d\n", board[i]->cashier_id, board[i]->sandwich_id);
	int diff = abs(board[i]->sandwich_id-made_last);
	if(diff < mn) {
	  mi = i;
	  mn = diff;
	  toCook = board[i];
	}
      }
      printf("\n");

      board.erase(board.begin()+mi); // remove from board
      made_last = toCook->sandwich_id; // update latest sandwich made
      order_complete[toCook->cashier_id] = 1; // cooking completed!
      thread_signal(BOARD_LOCK, toCook->cashier_id); // signal the cashier
      printf("SIGNAL %d\n", toCook->cashier_id);
    }
    thread_unlock(BOARD_LOCK);
    printf("BOARD_UNLOCK MAKER\n");
  }
}

void create_cashier(void* order_file) {
  int myID = -1;
  int current_order = 0;
  order_info* info = (order_info*) malloc(sizeof(order_info));

  // -----------------USING nCashier--------
  thread_lock(NUMBER_OF_CASHIER_LOCK); //   |
  printf("NUM_CASH_LOCK CASHIER\n");
  myID = liveCashier++;		       //   |
  thread_unlock(NUMBER_OF_CASHIER_LOCK); // |
  printf("NUM_CASH_LOCK CASHIER %d\n", myID);
  // ---------------------------------------

  info->cashier_id = myID;
  while(current_order < orders[myID].size()) {
    info->sandwich_id = orders[myID][current_order];

    thread_lock(BOARD_LOCK); 							
    printf("BOARD_LOCK %d\n", myID);

    if(board.size() < max_orders) { // if there's space on the board post order 
      printf("PUT SANDWICH %d\n", info->sandwich_id);
      board.push_back(info);
      order_complete[myID] = 0; // order is not completed			

      printf("BOARD_WAIT %d\n", myID);
      while(order_complete[myID] == 0) // keep waiting until we get signal(order complete) from the cooker 
	thread_wait(BOARD_LOCK, myID); // release lock, wait until signal with this ID is sent
      printf("BOARD_SIGNALED %d\n", myID);
      current_order ++;
    }

    thread_unlock(BOARD_LOCK);
    printf("BOARD_UNLOCK %d\n", myID);
  }

  // -----------------USING nCashier------------------------
  thread_lock(NUMBER_OF_CASHIER_LOCK); //		  //|
  if(liveCashier < max_orders) max_orders = liveCashier;  //|
  if(--liveCashier == 0) done = 1;	//		    |
  thread_unlock(NUMBER_OF_CASHIER_LOCK); //		    |
  // -------------------------------------------------------

  free(info);
}

void initialize(char* order_files[]) {
  // start_preemptions(true, true, 1);
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
  done = nCashier > 0 ? 0 : 1;
  sscanf(argv[1], "%d", &max_orders);
  thread_libinit((thread_startfunc_t) initialize, argv);
  return 0;
}
