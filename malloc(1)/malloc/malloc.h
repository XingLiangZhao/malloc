#ifndef __MALLOC_H
#define __MALLOC_H

//#define malloc_addr 0x20000000
//#define malloc_size 0x100000
#define NULL 0

typedef struct{
unsigned char is_available;			/* whether blcok is avaiable */
unsigned int prior_blocksize;       /* size of prior block */
unsigned int current_blocksize;     /* block size */
}mem_control_block;

//#define malloc_init()	Rewrite_malloc_init()
//#define malloc(x)		Rewrite_malloc(x)
//#define free(x)			Rewrite_free(x)

void malloc_init(void);
void * malloc(unsigned int numbytes);
void free(void *firstbyte);

#endif