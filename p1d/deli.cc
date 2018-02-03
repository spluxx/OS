#include <stdio.h>
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

int max_orders; 

// GOVERNED BY NUMBER_OF_CASHIER_LOCK --------------
int nCashier, done;	    //			    |
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
    breakFlag |= done;
    thread_unlock(NUMBER_OF_CASHIER_LOCK);
    if(breakFlag) break;

    thread_lock(BOARD_LOCK);
    if(board.size() == max_orders) {
      int mi = 0;
      int mn = 5000;
      order_info* toCook = NULL;
      for(int i = 0 ; i < board.size() ; i ++) {
	int diff = abs(board[i]->sandwich_id-made_last);
	if(diff < mn) {
	  mi = i;
	  toCook = board[i];
	}
      }
      board.erase(board.begin()+mi); // remove from board
      made_last = toCook->sandwich_id; // update latest sandwich made
      order_complete[toCook->cashier_id] = 1; // cooking completed!
      thread_signal(NUMBER_OF_CASHIER_LOCK, toCook->cashier_id); // signal the cashier
    }
    thread_unlock(BOARD_LOCK);
  }
}

void create_cashier(void* order_file) {
  int myID = -1;
  order_info* info = (order_info*) malloc(sizeof(order_info));

  // -----------------USING nCashier--------
  thread_lock(NUMBER_OF_CASHIER_LOCK); //   |
  myID = nCashier ++;		       //   |
  thread_unlock(NUMBER_OF_CASHIER_LOCK); // |
  // ---------------------------------------

  info->cashier_id = myID;

  while(fscanf((FILE*) order_file, "%d", &info->sandwich_id) != EOF) {
    thread_lock(BOARD_LOCK); 							
    if(board.size() < max_orders) { // if there's space on the board post order 
      board.push_back(info); //							
      order_complete[myID] = 0; // order is not completed			
      while(order_complete[myID] == 0) // keep waiting until we get signal(order complete) from the cooker 
	thread_wait(BOARD_LOCK, myID); // release lock, wait until signal with this ID is sent
    }
    thread_unlock(BOARD_LOCK);
  }

  // -----------------USING nCashier--------
  thread_lock(NUMBER_OF_CASHIER_LOCK); //   |
  if(--nCashier == 0) done = 1;		//  |
  thread_unlock(NUMBER_OF_CASHIER_LOCK); // |
  // ---------------------------------------

  free(info);
  fclose((FILE*) order_file); 
}

void initialize(char* order_files[]) {
  order_complete = (int *) malloc(sizeof(int)*nCashier);
  for(int i = 0 ; i < nCashier ; i ++)
    thread_create(create_cashier, fopen((char*) order_files[i], "r"));
  free(order_complete);
}

int main(int argc, char* argv[]) {
  nCashier = argc-2;
  done = nCashier > 0 ? 0 : 1;
  sscanf(argv[1], "%d", &max_orders);

  thread_libinit((thread_startfunc_t) initialize, argv);
  return 0;
}
