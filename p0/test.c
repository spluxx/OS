#include <stdio.h>
#include <stdlib.h>

struct metadata {
  size_t size;
  metadata* next;
};

int main() {
  metadata* a = (metadata*) malloc(sizeof(metadata));
  a->size = 10;
  a->next = NULL;

  size_t b = 1UL << 42;
  printf("%llu\n", b);
  printf("%p\n", *((metadata**) (((char *) a) + sizeof(size_t))));
  return 0;
}
