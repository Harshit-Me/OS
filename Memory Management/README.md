Implementation of malloc() and free() function in C, using mmap() and munmap() system call. \
Overview: \
my_init(): This will be the initialization function for the memory allocator. It will use the mmap system call to request a page of 4KB from the memory.
The 4KB page is the space that the memory allocator will be working with. \

my_alloc(int): Implementation of malloc(int). Any space asked for should be a multiple of 8 bytes, otherwise the function returns NULL, which is also does in case
of any other error. As expected, just like malloc(int), it would return a pointer to the address assigned in case of successful completion. \

my_free(void *): Takes a pointer to a previously allocated chunk and frees up that chunk, just like free(char *) does. \

my_clean(): This would be done at the end to return the memory back to the OS using munmap system call. \

my_heapinfo(): This would give some useful information about the current state of heap. This must include the maximum size allowed for the heap,
the size of the current heap, total size of the free memory in the heap, number of blocks allocated, size of a smallest available chunk,
and the size of a largest available chunk.
