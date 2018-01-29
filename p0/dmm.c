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

#define LL		    long long
#define MAX_SEGRAGATION	    (8*WORD_SIZE)
#define PACK(s, pa, a)	    ((s) | (pa) | (a))
#define BP(mdp)		    ((char *)(mdp) + METADATA_T_ALIGNED)
#define GET_SIZE(mdp)	    ((*((size_t *) mdp)) & ~(ALIGNMENT-1)) // get portion of size within a size_t
#define GET_ALLOC(mdp)	    ((*((size_t *) mdp)) & 0x1)	// 0 if free // 1 if alloc
#define GET_PREV_ALLOC(mdp) ((*((size_t *) mdp)) & 0x2)	// 0 if free // 2 if alloc
#define HDRP(bp)	    ((char *)(bp) - METADATA_T_ALIGNED)
#define FTRP(bp)	    ((char *)(bp) + GET_SIZE(HDRP(bp)) - METADATA_T_ALIGNED)

/* freelist maintains all the blocks which are not in use; freelist is kept
 * sorted to improve coalescing efficiency */

static metadata_t* freelist[MAX_SEGRAGATION]; // freelist[i] -> blocks of size [2^i * ALIGNMENT, 2^(i+1) * ALIGNMENT)
					      // freelist[-1] -> blocks of size [2^(MAX_SEGRAGATION-1) * ALIGNMENT, inf)
static metadata_t* start_of_world = NULL; // initial
static metadata_t* end_of_world; // brk

bool check_init() {
  bool flag = true;
  for(int i = 0 ; i < MAX_SEGRAGATION ; i ++) {
    if(freelist[i] != NULL) { flag = false; break; }
  } if(flag) if(!dmalloc_init()) return false;
  return true;
}

int find_freelist(size_t numbytes) {
  int whichList = -1;
  for(int i = 0 ; i < MAX_SEGRAGATION ; i ++) {  
    LL ub = (1UL << (i+1)) * ALIGNMENT - 1;
    if(numbytes <= ub && freelist[i] != NULL) { whichList = i; break; } // find min i that has free block for numbytes 
  } return whichList;
}

int where_to_put(size_t numbytes) {
  int whichList = -1;
  for(int i = 0 ; i < MAX_SEGRAGATION ; i ++) {
    LL ub = (1UL << (i+1)) * ALIGNMENT - 1;
    if(numbytes <= ub) { whichList = i; break; } // find min i that has free block for numbytes 
  } return whichList;
}

bool put_free_block(void* ptr) {
  metadata_t* tmp = (metadata_t*) ptr;
  int whichList = where_to_put(GET_SIZE(ptr));
  if(whichList == -1) return false;

  if(freelist[whichList] == NULL) {
    freelist[whichList] = tmp;
    tmp->next = tmp->prev = NULL;
  } else {
    tmp->next = freelist[whichList];
    tmp->prev = NULL;
    freelist[whichList]->prev = tmp;
    freelist[whichList] = tmp;
  } return true;
}

void* dmalloc(size_t numbytes) {
  if(!check_init()) return NULL; // more memory only when it's not caused by bad fragmentation

  assert(numbytes > 0);

  print_all();
  print_freelist();
   
  numbytes = ALIGN(numbytes);
  numbytes = numbytes < METADATA_T_ALIGNED+WORD_SIZE ? METADATA_T_ALIGNED+WORD_SIZE : numbytes;

  int whichList = find_freelist(numbytes);
  if(whichList == -1) return NULL; // if none is available... fail...

  void* ret = NULL;
  metadata_t* tmp = freelist[whichList];
  size_t residue = GET_SIZE(tmp) - numbytes - 2*METADATA_T_ALIGNED;
  if((LL)residue > 0) { // split
    tmp->size = PACK(numbytes, GET_PREV_ALLOC(tmp), 0x1);     
    metadata_t* nextBlk = (metadata_t *)(BP(tmp) + GET_SIZE(tmp));
    metadata_t* nextFtr = (metadata_t *)((char *) nextBlk + residue + METADATA_T_ALIGNED);
    nextBlk->size = nextFtr->size = PACK(residue+METADATA_T_ALIGNED, 0x2, 0x0);
    nextBlk->next = nextFtr->next = tmp->next;
    nextBlk->prev = nextFtr->prev = NULL;
    if(nextBlk->next != NULL) nextBlk->next->prev = nextBlk;
    freelist[whichList] = freelist[whichList]->next;
    if(freelist[whichList] != NULL) freelist[whichList]->prev = NULL;
    put_free_block(nextBlk);
    ret = (void *) BP(tmp);
  } else { // just
    tmp->size = PACK(GET_SIZE(tmp), GET_PREV_ALLOC(tmp), 0x1);
    metadata_t* nextHeader = (metadata_t*) ((char*) tmp + GET_SIZE(tmp) + METADATA_T_ALIGNED);
    if(nextHeader < end_of_world) nextHeader->size |= 0x2;
    freelist[whichList] = freelist[whichList]->next;
    if(freelist[whichList] != NULL) freelist[whichList]->prev = NULL;
    if(tmp->next != NULL) tmp->next->prev = tmp->prev;
    ret = (void *) BP(tmp);
  }
  
  print_all();
  print_freelist();

  return ret;
}

void dfree(void* ptr) {
  DEBUG("FREE %p\n", ptr);

  print_all();
  print_freelist();

  metadata_t* header = (metadata_t*) ((char*) ptr - METADATA_T_ALIGNED);
  metadata_t* footer = (metadata_t*) ((char*) ptr + GET_SIZE(header) - METADATA_T_ALIGNED);
  metadata_t* nextHeader = (metadata_t*) ((char*) ptr + GET_SIZE(header));

  header->size &= (~0x1); // set alloc to free
  footer->size = header->size; // copy header into footer

  bool prevFree = GET_PREV_ALLOC(header) == 0x0;
  bool nextFree = (nextHeader < end_of_world) && (GET_ALLOC(nextHeader) == 0x0);

  metadata_t* prevHeader = NULL;
  if(prevFree) { // if prev is free then it must have a footer
    prevHeader = (metadata_t*) ((char *) header - GET_SIZE(((char *) header - METADATA_T_ALIGNED)) - METADATA_T_ALIGNED);
    // remove prevHeader from freelist
    int whichList = where_to_put(GET_SIZE(prevHeader));
    if(freelist[whichList] == prevHeader) { 
      freelist[whichList] = freelist[whichList]->next;
      if(freelist[whichList] != NULL) freelist[whichList]->prev = NULL;
    } else {
      prevHeader->prev->next = prevHeader->next;
      if(prevHeader->next != NULL) prevHeader->next->prev = prevHeader->prev;
    }
  }

  if(nextHeader < end_of_world) {
    nextHeader->size &= (~0x2); // set prev_alloc of the next block to free
    if(nextFree) {
      // remove nextHeader from freelist
      int whichList = where_to_put(GET_SIZE(nextHeader));
      if(freelist[whichList] == nextHeader) { 
        freelist[whichList] = freelist[whichList]->next;
        if(freelist[whichList] != NULL) freelist[whichList]->prev = NULL;
      } else {
        nextHeader->prev->next = nextHeader->next;
        if(nextHeader->next != NULL) nextHeader->next->prev = nextHeader->prev;
      }
    }
  }

  void* toPut = NULL;
  if(!prevFree && !nextFree) {
    DEBUG("NO COALESCE\n");
    toPut = (void *) header; // no coalesce
  } else if(prevFree && !nextFree) { // coalesce to the left
    DEBUG("COALESCE LEFT\n");
    prevHeader->size = 
      PACK(GET_SIZE(prevHeader) + METADATA_T_ALIGNED + GET_SIZE(header), GET_PREV_ALLOC(prevHeader), GET_ALLOC(prevHeader));
    ((metadata_t *) ((char*) prevHeader + GET_SIZE(prevHeader)))->size = prevHeader->size;
    toPut = (void *) prevHeader;
  } else if(!prevFree && nextFree) { // coalesce to the right
    DEBUG("COALESCE RIGHT\n");
    header->size = 
      PACK(GET_SIZE(header) + METADATA_T_ALIGNED + GET_SIZE(nextHeader), GET_PREV_ALLOC(header), GET_ALLOC(header));
    ((metadata_t *) ((char*) header + GET_SIZE(header)))->size = header->size;
    toPut = (void *) header;
  } else { // coalesce both left and right
    DEBUG("COALESCE BOTH\n");
    prevHeader->size =
      PACK(GET_SIZE(prevHeader) + METADATA_T_ALIGNED + GET_SIZE(header) + METADATA_T_ALIGNED + GET_SIZE(nextHeader),
	  GET_PREV_ALLOC(prevHeader), GET_ALLOC(prevHeader));
    ((metadata_t *) ((char*) prevHeader + GET_SIZE(prevHeader)))->size = prevHeader->size;
    toPut = (void *) prevHeader;
  }

  put_free_block(toPut); // then put into corresponding freelist

  print_all();
  print_freelist();
}

bool dmalloc_init() {
  size_t max_bytes = ALIGN(MAX_HEAP_SIZE);
  int whichList = where_to_put(max_bytes-METADATA_T_ALIGNED);
  if(whichList == -1) return false; // if none is available... fail...

  freelist[whichList] = (metadata_t*) sbrk(max_bytes); 
  end_of_world = (metadata_t*) ((char*) freelist[whichList] + max_bytes); // update end_of_world
  if(start_of_world == NULL) start_of_world = (metadata_t*) freelist[whichList]; // update start_of_world

  if (freelist[whichList] == (void *)-1) return false;

  metadata_t* footer = (metadata_t *) ((char *) freelist[whichList] + max_bytes - METADATA_T_ALIGNED);
  freelist[whichList]->next = footer->next = NULL;
  freelist[whichList]->prev = footer->prev = NULL;
  freelist[whichList]->size = footer->size = PACK(max_bytes-METADATA_T_ALIGNED, 0x2, 0x0);
  return true;
}

/* for debugging; can be turned off through -NDEBUG flag*/
void print_freelist() {
  for(int i = 0 ; i < MAX_SEGRAGATION ; i ++) {
    metadata_t *freelist_head = freelist[i];
    while(freelist_head != NULL) {
      DEBUG("\t%d'th segragation: Freelist Size:%zd, Head:%p, Next:%p, Prev:%p\t",
	    i,
            GET_SIZE(freelist_head),
            freelist_head,
            freelist_head->next,
	    freelist_head->prev
	    );
      freelist_head = freelist_head->next;
    }
  }
  DEBUG("\n");
}

void print_all() {
  metadata_t* tmp = start_of_world;
  while(tmp < end_of_world) {
      DEBUG("%s, Size:%zd\tHead:%p\tNext:%p\tPrev:%p(%s)\t",
	    GET_ALLOC(tmp) == 0 ? "Free" : "ALLOC",
            GET_SIZE(tmp),
            tmp,
            tmp->next,
	    tmp->prev,
	    GET_PREV_ALLOC(tmp) == 0 ? "Free" : "ALLOC"
	    );
      metadata_t* footer = (metadata_t*) ((char*) tmp + GET_SIZE(tmp));
      if(GET_ALLOC(tmp) == 0) 
	DEBUG("FOOTER Size:%zd\tHead:%p\tNext:%p\tPrev:%p(%s)\t",
            GET_SIZE(footer),
            footer,
            footer->next,
	    footer->prev,
	    GET_PREV_ALLOC(footer) == 0 ? "Free" : "ALLOC"
	    );
    tmp = (metadata_t*) ((char *) tmp + GET_SIZE(tmp) + METADATA_T_ALIGNED);
  }
  DEBUG("\n");
}
