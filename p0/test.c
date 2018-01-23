#include <stdio.h>
#include <stdlib.h>

int main() {
  size_t b = 2;
  size_t a = 1;
  printf("%s\n", (a-b) < 0 ? "neg":"nonneg");
  return 0;
}
