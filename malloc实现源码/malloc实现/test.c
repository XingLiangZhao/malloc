#include <stdio.h>
#include "malloc.h"

void main(void)
{
	char testbuf[0x10000];
	unsigned long base = (unsigned long)&testbuf[0];
	printf("%lx\n",base);
	mem_malloc_init(base,0x8000);
	char *buf=malloc(100);
	sprintf(buf,"hello\n");
	printf("%s",buf);
}
