#include "malloc.h"

static unsigned char has_initialized;
static unsigned int managed_memory_start;
static unsigned int managed_memory_end;
static unsigned int managed_memory_size;

/* heap buffer size */
#define HEAPSIZE	500

/* heap start addrress */
#define malloc_addr heap
/* heap size */ 
#define malloc_size HEAPSIZE

/* heap buffer */
unsigned char heap[HEAPSIZE]; 

/**
  * @brief Initializes malloc's gloabl varialbe and head of heap memory    
  * @param None 
  * @retval None
  */
void malloc_init(void)
{
	mem_control_block * mcb = NULL;
	
	/* confirm heap's start address */
	managed_memory_start = (unsigned int)malloc_addr;
	/* confirm heap's size */
	managed_memory_size = malloc_size;
	/*confirm heap's end address */
	managed_memory_end = managed_memory_start + managed_memory_size;
	
	/* make mcb point to heap's start address */ 
	mcb = (mem_control_block *)managed_memory_start;
	/*the first blcok is avaialbe */
	mcb->is_available = 1;
	/*there is no block before the first block */
	mcb->prior_blocksize = 0;
	/*the first block's block size is difference of between heap's size and control block */ 
	mcb->current_blocksize = managed_memory_size - sizeof(mem_control_block);
	/* Initialize done */
	has_initialized = 1;
}

/**
  * @brief Dynamic distribute memory function
  * @param numbytes: what size you need   
  * @retval a void pointer to the distribute first address
  */ 
void * malloc(unsigned int numbytes)
{
	unsigned int current_location,otherbck_location;
	/* This is the same as current_location, but cast to a memory_control_block */
	mem_control_block * current_location_mcb = NULL,* other_location_mcb = NULL;
	/* varialbe for saving return value and be set to 0 until we find something suitable */
	void * memory_location = NULL;
	/* current dividing block size */
	unsigned int process_blocksize;
	
	/* Initialize if we haven't already done so */
	if(! has_initialized) {
		malloc_init();
	}
	
	/* Begin searching at the start of managed memory */
	current_location = managed_memory_start;
	/* Keep going until we have searched all allocated space */
	while(current_location != managed_memory_end){
		/* current_location and current_location_mcb point to the same address.  However, 
		 * current_location_mcb is of the correct type, so we can use it as a struct. current_location 
		 * is a void pointer so we can use it to calculate addresses.
		 */
		current_location_mcb = (mem_control_block *)current_location;
		/* judge whether current block is avaiable */
		if(current_location_mcb->is_available){
			/* judge whether current block size exactly fit for the need */
			if((current_location_mcb->current_blocksize == numbytes)){
				/* It is no longer available */ 
				current_location_mcb->is_available = 0;			
			    /* We own it */
				memory_location = (void *)(current_location + sizeof(mem_control_block));
				/* Leave the loop */
				break;
			/* judge whether current block size is enough for dividing a new block */
			}else if(current_location_mcb->current_blocksize >= numbytes + sizeof(mem_control_block)){
				/* It is no longer available */ 
				current_location_mcb->is_available = 0;
				/* because we will divide current blcok,before we changed current block size,we should
				 * save the integral size.
				 */
				process_blocksize = current_location_mcb->current_blocksize;
				/* Now blcok size could be changed */
				current_location_mcb->current_blocksize = numbytes;
				
				/* find the memory_control_block's head of remaining block and set parameter,block of no
				 * parameter can't be managed. 
				 */
				other_location_mcb = (mem_control_block *)(current_location + numbytes \
												+ sizeof(mem_control_block));
				/* the remaining block is still avaiable */
				other_location_mcb->is_available = 1;
				/* of course,its prior block size is numbytes */
				other_location_mcb->prior_blocksize = numbytes;
				/* its size should get small */
				other_location_mcb->current_blocksize = process_blocksize - numbytes \
												- sizeof(mem_control_block);
				
				/* find the memory_control_block's head of block after current block and \
				 * set parameter--prior_blocksize. 
				 */
				otherbck_location = current_location + process_blocksize \
											+ sizeof(mem_control_block);				
				/* We need check wehter this block is on the edge of managed memeory! */
				if(otherbck_location != managed_memory_end){
					other_location_mcb = (mem_control_block *)(otherbck_location);
					/*  its prior block size has changed! */
					other_location_mcb->prior_blocksize = process_blocksize\
						- numbytes - sizeof(mem_control_block);
				}
				/*We own the occupied block ,not remaining block */ 
				memory_location = (void *)(current_location + sizeof(mem_control_block));
				/* Leave the loop */
				break;
			} 
		}
		/* current block is unavaiable or block size is too small and move to next block*/
		current_location += current_location_mcb->current_blocksize \
									+ sizeof(mem_control_block);
	}
	/* if we still don't have a valid location,we'll have to return NULL */
	if(memory_location == NULL)	{
		return NULL;
	}
	/* return the pointer */
	return memory_location;	
}

/**
  * @brief  free your unused block 
  * @param  firstbyte: a pointer to first address of your unused block
  * @retval None
  */ 
void free(void *firstbyte) 
{
	unsigned int current_location,otherbck_location;
	mem_control_block * current_mcb = NULL,* next_mcb = NULL,* prior_mcb \
								= NULL,* other_mcb = NULL;
	/* Backup from the given pointer to find the current block */
	current_location = (unsigned int)firstbyte - sizeof(mem_control_block);
	current_mcb = (mem_control_block *)current_location;
	/* Mark the block as being avaiable */
	current_mcb->is_available = 1;
	
	/* find next block location */
	otherbck_location = current_location + sizeof(mem_control_block) \
									+ current_mcb->current_blocksize;
	/* We need check wehter this block is on the edge of managed memeory! */
	if(otherbck_location != managed_memory_end){
		/* point to next block */
		next_mcb = (mem_control_block *)otherbck_location;
		/* We need check whether its next block is avaiable */ 
		if(next_mcb->is_available){
			/* Because its next block is also avaiable,we should merge blocks */
			current_mcb->current_blocksize = current_mcb->current_blocksize \
				+ sizeof(mem_control_block) + next_mcb->current_blocksize;
			
			/* We have merge two blocks,so we need change prior_blocksize of
			 * block after the two blocks,just find next block location. 
			 */
			otherbck_location = current_location + sizeof(mem_control_block) \
									+ current_mcb->current_blocksize;
			/* We need check wehter this block is on the edge of managed memeory! */
			if(otherbck_location != managed_memory_end){
				other_mcb = (mem_control_block *)otherbck_location;
				/*  its prior block size has changed! */
				other_mcb->prior_blocksize = current_mcb->current_blocksize;
			}
		}
	}
	
	/* We need check wehter this block is on the edge of managed memeory! */
	if(current_location != managed_memory_start){
		/* point to prior block */
		prior_mcb = (mem_control_block *)(current_location - sizeof(mem_control_block)\
											- current_mcb->prior_blocksize);
		/* We need check whether its prior block is avaiable */ 
		if(prior_mcb->is_available){
			/* Because its prior block is also avaiable,we should merge blocks */
			prior_mcb->current_blocksize = prior_mcb->current_blocksize \
				+ sizeof(mem_control_block) + current_mcb->current_blocksize;
			
			/* We have merge two blocks,so we need change prior_blocksize of
			 * block after the two blocks,just find next block location. 
			 */
			otherbck_location = current_location + sizeof(mem_control_block) \
									+ current_mcb->current_blocksize;
			/* We need check wehter this block is on the edge of managed memeory! */
			if(otherbck_location != managed_memory_end){
				other_mcb = (mem_control_block *)otherbck_location;
				/*  its prior block size has changed! */
				other_mcb->prior_blocksize = prior_mcb->current_blocksize;
			}
		}
	}
}