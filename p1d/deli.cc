#include <stdio.h>
#include <vector>
#include "thread.h"
using namespace std;

// Locks
const int BOARD_LOCK = 1;
const int NUMBER_OF_CASHIER_LOCK = 2;
// Condition Variables
const int ORDER_COMPLETE = 1;

struct order_info {
  int cashier_id;
  int sandwich_number;
};

int max_orders, nCashier;
vector<order_info> board;
int num_posts;

int *order_complete;

void create_cashier(FILE* order_file) {
  int myID = -1;
  order_info* info = (order_info*) malloc(sizeof(order_info));

  thread_lock(NUMBER_OF_CASHIER_LOCK);
  myID = nCashier ++;
  thread_unlock(NUMBER_OF_CASHIER_LOCK);

  info->cashier_id = myID;

  while(fscanf(order_file, "%d", &info->sandwich_number) != EOF) {
    thread_lock(BOARD_LOCK); // ------------------USING BOARD---------------- //

    if(board.size() < max_orders)  // if there's space on the board post order
      board.push_back(info);

    order_complete[myID] = 0; // order is not completed

    while(order_complete[myID] == 0) // keep waiting until we get signal(order complete) from the cooker 
      thread_wait(BOARD_LOCK, ORDER_COMPLETE);

    thread_unlock(BOARD_LOCK);
  }

  thread_lock(NUMBER_OF_CASHIER_LOCK);
  nCashier --;
  thread_unlock(NUMBER_OF_CASHIER_LOCK);

  fclose(order_file); 
}

void initialize(char* order_files[]) {
  for(int i = 0 ; i < nChashier ; i ++)
    thread_create(create_cashier, fopen(order_file[i], "r"));
}

int main(int argc, char* argv[]) {
  nCashier = argc-2;
  order_complete = (int *) malloc(sizeof(int)*nCashier);
  sscanf(argv[1], "%d", &max_orders);
  thread_libinit(initialize, argv);
  free(order_complete);
  return 0;
}
