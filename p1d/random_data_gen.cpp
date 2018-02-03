#include <stdio.h>
#include <cstdlib>
#include <ctime>

int main() {
  srand(time(NULL)); 
  for(int i = 0 ; i < 1000 ; i ++) {
    printf("%d\n", rand()%1000);
  }
  return 0;
}
