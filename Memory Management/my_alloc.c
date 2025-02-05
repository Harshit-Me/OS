#include<stdio.h>
#include<errno.h>
#include <sys/mman.h>

//Header for allocated space.
typedef struct header
{
	int size;// Size of the available space.
	int magic;// Validation Marker
} header;
typedef struct ll_node// Free list node. We maintain a freelist as a singly Linked List
{
	int size;// Size of the free chunk. (This size involves size of ll_node also)
	struct ll_node *next;//Next Pointer
}ll_t;// Linked List Node;

//Globabl Variables.
ll_t *freelist; // Free list of my heap. Always contains chunks in increasing order of addresses. This decreases external fragmentation
void *start;// Start of the mapping address. Used for Unmapping
int num_allocated = 0;// Total number of blocks allocated.


int my_init()//Initialization of heap
{
	//mmap allocation of memory
	freelist = (ll_t*)mmap(NULL,4*1024,PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANONYMOUS,-1,0);// Allocating a memory from OS chosen physical address
	start = (void*)freelist;
	if (freelist == MAP_FAILED)// mmap has failed
	{
		return errno;
	}
	freelist->size = 4096;// Setting the header and size.
	freelist->next = NULL;
	return 0;
}
void printStats()// Stats of my freelist. Used for debugging purposes.
{
	ll_t*trav = freelist;
	int num_iter = 0;
	while(trav&& num_iter<10)
	{
		num_iter++;// To ensure we dont print infinitely incase of infinte loop.
		printf("Trav is %p and size is %d \n",trav, trav->size );
		trav = trav->next;
	}
	printf("     \n");
}
void my_free(void * x)// Free this space.
{

	if(!x)// Given address is NULL.
	{
		return;
	}

	header *addr = (header*)(x-sizeof(header));
	
	if(addr->magic!=5913235)// Address is not valid.
	{
		return;
	}
	addr->magic = 0;// This addres is now not sane and hence unallocated.
	num_allocated--;// Total number of allocated blocks has decreased.
	ll_t*trav= freelist;// Head of the freelist. Traversal.
	if((void*)addr<(void*)trav)// My address is smaller than freelist head. So i insert at the head.
	{

		if((void*)addr+addr->size+sizeof(header)==freelist) //This chunk is continuous with freelist head. Coaelescing will take place.
		{

			//Coalescing
			int size = addr->size;// Store the size of the (previously )allocated chunk
			ll_t* temp = (ll_t*)addr;// Typecaste.
			temp->next = freelist->next;// Set the next to freelist next
			temp->size = freelist->size+size+sizeof(header);// The size will be size of the freelist head +size of header +size of free space.
			freelist = temp;// Change the head.
			return;
		}
		else// Not continuius chunk. Just add at head.
		{
			
			int size = addr->size;
			ll_t* temp = (ll_t*)addr;
			temp->next = freelist;
			temp->size = size+sizeof(header);
			freelist = temp;
			return;
		}
	}
    // Not smaller than head. Either will be inserted in between two nodes or at tail.
	while(trav)
	{
		if (trav->next)
		{	
			if((void*)trav<(void*)addr && (void*)addr<(void*)(trav->next))// Gets inserted in between two nodes.
			{
				ll_t*next = trav->next;
				int size =addr->size;
				ll_t* temp = (ll_t*) addr;
				if((void*)addr+sizeof(header)+size == next)
				{
					//Coaelescing
					temp->next = next->next;
					temp->size = size+sizeof(header)+next->size;
					
				}
				else
				{
					temp->next = next;
					temp->size = size+sizeof(header);
				}
				if ((void*)trav+trav->size ==addr)// [B1]->[INB](Continuous)
				{
					//Coaelescig
				
					trav->next = temp->next;
					trav->size = trav->size+temp->size;
				}
				else
				{
					trav->next = temp; 
				}
				return;

			}
		}
		else// Gets inserted at tail.
		{
		
			int size =addr->size;
			ll_t* temp = (ll_t*) addr;
			if((void*)trav+trav->size == addr)
			{
				trav->next = NULL;
				trav->size = trav->size +sizeof(header)+size;
			}
			else
			{
				trav->next = temp;
				temp->next = NULL;
				temp->size = size +sizeof(header);
			}
			return;
		}
		trav = trav->next;
	}
}
void  my_clean()// Self explanatory.
{
	munmap(start,4*1024);
	num_allocated = 0;
	return;
}
void * my_alloc(int num)// Allocate space equal num. Return null in case it is not possible 
{
	if (num==0 || num%8!=0) //Only allot if given number is a multiple of 8.
	{
		return NULL;
	}
	int req_size = num+sizeof(header);
	ll_t *trav = freelist;
	ll_t*prev = NULL;
	while (trav)
	{
		printf("%d\n",trav->size);
		if (trav->size>=req_size+sizeof(ll_t)||trav->size == req_size)// Corner case handle
		{
			
            if(trav->size == req_size)// The whole freelist chunk can be assigned.
            {
                if (prev)
                {
                    prev->next = trav->next;
                    header *temp =(header *)trav;
                    temp->size= req_size-sizeof(header);
                    temp->magic = 5913235; //Assigning validation Number
                    num_allocated+=1;
                    return (void*)(temp+1);
                }
                else
                {
                    freelist = freelist->next;
                    header *temp =(header *)trav;
                    temp->size= req_size-sizeof(header);
                    temp->magic = 5913235;
                    num_allocated+=1;
                    return (void*)(temp+1);
                }
            }
			ll_t* next = trav->next;
			ll_t *addr = (ll_t*)((void*)trav+req_size);
			addr->size = trav->size - req_size;
			addr->next = next;
			if (prev)
			{
				prev->next = addr;
			}
			if(trav==freelist)
			{
				freelist = addr;
			}
			header *temp= (header*)trav;
			temp->size = num;
			temp->magic = 5913235;
			num_allocated+=1;
			return (void*)((void*)temp+sizeof(header));
		}
		prev = trav;
		trav = trav->next;
	}
	return NULL;
}
void my_heapinfo()// Self explanatory
{
	int free_chunks = 0;
	int occupied_chunks = num_allocated;
	int allocated_space = 0;
	int max_free = 0;
	int total_free = 0;
	int smallest_free = 4096;
	int max_size = 0;
	ll_t *ptr = freelist;
	while(ptr)
	{
	
		free_chunks++;
		total_free+=ptr->size-sizeof(ll_t);
		if (max_free<ptr->size-sizeof(ll_t))
		{
			max_free = ptr->size -sizeof(ll_t);
		}
		if(smallest_free>ptr->size -sizeof(ll_t))
		{
			smallest_free = ptr->size -sizeof(ll_t);
		}
		ptr = ptr->next;
	}
	allocated_space = 4096-total_free;
	max_size = 4096;
	if(smallest_free == 4096)
	{
		smallest_free =0;
	}

	printf("=== Heap Info ================\n");
	printf("Max Size: %d\n", max_size);
	printf("Current Size: %d\n", allocated_space );
 	printf("Free Memory: %d\n", total_free);
 	printf("Blocks allocated: %d\n", occupied_chunks);
 	printf("Smallest available chunk: %d\n", smallest_free);
 	printf("Largest available chunk: %d\n", max_free);
 	printf("==============================\n");
	return;
}

void main(){
	if(my_init() == 0){
		printf("Successfully Requested Memory from OS\n");
	}
	else{
		printf("mmap() failed to allocate memory!Exiting");
		return;
	}
	int flag = 0;
	int size = 0;
	void *p;
	do{
		printf("========MENU=======\n");
		printf("0. exit\n");
		printf("1. Allocate Memory\n");
		printf("2. Release Memory\n");
		printf("3. Heap Information\n");
		
		scanf("%d",&flag);
		switch (flag)
		{
		case 0:
			return;
		case 1:
			printf("Enter the amound of memory to allocate(Only Enter Multiple of 8):");
			scanf("%d",&size);
			p = my_alloc(size);
			if(p == NULL) {
				printf("Memory Allocation Failed!\n");
				return;
			}
			else printf("Memory Allocation Successful\n");
			break;
		case 2:
			my_free(p);
			printf("Memory Released\n");
			break;
		
		case 3:
			my_heapinfo();
			break;

		default:
			printf("Invalid Option!\n");
		}
	}
	while(flag);
	return;
}