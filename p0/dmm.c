#include <stdio.h>  // needed for size_t
#include <unistd.h> // needed for sbrk
#include <assert.h> // needed for asserts
#include "dmm.h"

/* You can improve the below metadata structure using the concepts from Bryant
 * and OHallaron book (chapter 9).
 */

typedef struct metadata {
  /* size_t is the return type of the sizeof operator. Since the size of an
   * object depends on the architecture and its implementation, size_t is used
   * to represent the maximum size of any object in the particular
   * implementation. size contains the size of the data object or the number of
   * free bytes
   */
  size_t size;
  struct metadata* next;
  struct metadata* prev; 
} metadata_t;

/* freelist maintains all the blocks which are not in use; freelist is kept
 * sorted to improve coalescing efficiency 
 */

static metadata_t* freelist = NULL;

void createNode(void* ptr, size_t size_, metadata_t* next_, metadata_t* prev_) {
  if(size_ == -METADATA_T_ALIGNED) { // erase meaningless header
    if(next_ != NULL) next_->prev=prev_; 
    if(prev_ != NULL) prev_->next=next_;
    else freelist = next_;
  } else {
    metadata_t* ret = (metadata_t*) ptr; 
    ret -> size = size_;

    ret -> next = next_;
    if(ret -> next != NULL) 
      ret -> next -> prev = ret;
    
    ret -> prev = prev_;
    if(ret -> prev != NULL)
      ret -> prev -> next = ret;
    else freelist = ret;
  }
}

void addNode(metadata_t* prev, metadata_t* that, metadata_t* next) {
  if(prev != NULL) prev->next = that; 
  else freelist = that;
  that->prev = prev;
  if(next != NULL) next->prev = that;
  that->next = next;
}


void* dmalloc(size_t numbytes) {
  /* initialize through sbrk call first time */
  if(freelist == NULL) { 			
    if(!dmalloc_init())
      return NULL;
  }
  assert(numbytes > 0);

  DEBUG("BEFORE DMALLOC %d\n", numbytes);
  print_freelist();

  numbytes = ALIGN(numbytes);

  metadata_t* tmp = freelist;
  while(tmp != NULL && (tmp->size > MAX_HEAP_SIZE ? true : tmp->size < numbytes)) tmp = tmp->next;

  if(tmp != NULL) {
    createNode(
      (metadata_t*)((char*)tmp + numbytes + METADATA_T_ALIGNED), 
      tmp->size - numbytes - METADATA_T_ALIGNED,
      tmp->next, tmp->prev);
    tmp->size = numbytes;
    tmp = (metadata_t*)((char*)tmp + METADATA_T_ALIGNED);
  }

  DEBUG("AFTER DMALLOC\n");
  print_freelist();

  return (void*) tmp;
}

void dfree(void* ptr) {
  metadata_t* header = (metadata_t*)((char*) ptr - METADATA_T_ALIGNED);
  metadata_t* tmpPrev = NULL;
  metadata_t* tmp = freelist;

  DEBUG("BEFORE DFREE %p\n", ptr);
  print_freelist();
  
  if(header > tmp) {
    while(tmp != NULL && header >= tmp) {
      tmpPrev = tmp;
      tmp = tmp->next;
    }
  }

  addNode(tmpPrev, header, tmp);
  DEBUG("ADD NODE\n");
  print_freelist();

  // coalesce right
  if(tmp != NULL && (char*)header + METADATA_T_ALIGNED + header->size == (char*) tmp) {
    header->size += tmp->size + METADATA_T_ALIGNED;
    header->next = tmp->next;
    if(tmp->next != NULL) tmp->next->prev = header;
  }

  // coalesce left
  if(tmpPrev != NULL && (char*)header - METADATA_T_ALIGNED - tmpPrev->size == (char*) tmpPrev) {
    tmpPrev->size += header->size + METADATA_T_ALIGNED;
    tmpPrev->next = header->next;
    if(header->next != NULL) header->next->prev = tmpPrev;
  }

  DEBUG("AFTER DFREE\n");
  print_freelist();
}

bool dmalloc_init() {

  /* Two choices: 
   * 1. Append prologue and epilogue blocks to the start and the
   * end of the freelist 
   *
   * 2. Initialize freelist pointers to NULL
   *
   * Note: We provide the code for 2. Using 1 will help you to tackle the 
   * corner cases succinctly.
   */

  size_t max_bytes = ALIGN(MAX_HEAP_SIZE);
  /* returns heap_region, which is initialized to freelist */
  freelist = (metadata_t*) sbrk(max_bytes); 
  /* Q: Why casting is used? i.e., why (void*)-1? */
  if (freelist == (void *)-1)
    return false;
  freelist->next = NULL;
  freelist->prev = NULL;
  freelist->size = max_bytes-METADATA_T_ALIGNED;
  return true;
}

/* for debugging; can be turned off through -NDEBUG flag*/
void print_freelist() {
  metadata_t *freelist_head = freelist;
  while(freelist_head != NULL) {
    DEBUG("\tFreelist Size:%zd, Head:%p, Prev:%p, Next:%p\t",
	  freelist_head->size,
	  freelist_head,
	  freelist_head->prev,
	  freelist_head->next);
    freelist_head = freelist_head->next;
  }
  DEBUG("\n");
}
