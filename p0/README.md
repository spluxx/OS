``Metadata`` <br>
The `metadata` structure is similar to one given - except for the fact that first bit of `size` was used to denote free/allocate state of this node, and second bit of `size` was used to denote free/allocate state of the previous node. <br>

``Constant time dmalloc - Segregated List`` <br>
We keep free blocks in a segregated list - linked list separated by size. Keeping track of array of 64 linked lists, containing free blocks with sizes in range of `[2^i*ALIGN, 2^{i+1}*ALIGN)`. This way, we can save time searching for right free block to use for `dmalloc`, since we can skip to the list that probably has the right-sized free block within few iterations.

``Constant time dfree - Footer`` <br>
Instead of keeping only the `headers`, keeping `footers` help when coalescing, since then there's no need to search through the freelist. We can reduce the space overhead by not placing footers on allocated blocks, as they're not gonna be coalesced anyways.

``Putting it together`` <br>
It's not so bad, as long as we don't forget to put footers on new free blocks and take care of minor edge cases in segregated list. Just had to keep in mind that `prev` in the context of `Segregated Lists` is not the same `prev` in the context of `Footers`. <br> 
It took me about 4 hours to complete the assignment without extra credit, and 6 more hours to complete the assignment with extra credit.
