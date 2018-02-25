#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <vector>
#include "thread.h"
using namespace std;

// Global Variables 
int numCash;
int livCash;
int boardSz;
int* corkBoard;
int* orderIn;
char** inputFiles;
bool isFull, isDone;

//-------- LOCKS ------------------
unsigned int CHEFL;
int abs(int a) { return a > 0 ? a : -a; }

void makeChef(char* sw[]) {
	thread_lock(CHEFL);
	int prevMade = -1;
	


start_preemptions(true, true, 1);
  order_complete = (int *) malloc(sizeof(int)*nCashier);
  for(int i = 2 ; i <= nCashier+1 ; i ++)
    thread_create((thread_startfunc_t) create_cashier, fopen(order_files[i], "r"));
  thread_create((thread_startfunc_t) create_maker, NULL);



	thread_unlock(CHEFL);
  
}

int main(int argc, char* argv[]) {
  numCash = argc-2;
  livCash = 0;
  boardSz = (argc-2<atoi(argv[1])? argc-2 : atoi(argv[1]));
  isFull = false;
  isDone = numCash <= 0;
  corkBoard = (int*) malloc(boardSz*sizeof(int));
  orderIn = (int*) malloc(boardSz*sizeof(int));
  // Initialize falues to -1
  for(int i=0; i<boardSz;i++){
  	corkBoard[i]=-1;
  	orderIn[i]=-1;  
  }
  thread_libinit((thread_startfunc_t) makeChef, argv);
  return 0;
}