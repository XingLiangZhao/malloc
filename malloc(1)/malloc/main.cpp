
#include "malloc.h"
#include <stdio.h>
#include <string.h>

#define HEAPSIZE	500
extern unsigned char heap[HEAPSIZE];

/**
  * @brief Test malloc function     
  * @param None 
  * @retval int
  */
int main(void)
{
	int * p1;
	int * p2;
	int * p3;
	int * p4;

	memset((void *)heap,0,HEAPSIZE);
	malloc_init();
	p1 = (int *)malloc(sizeof(int));
	if(p1 == NULL){
		printf("malloc failed!\n");
	}
	*p1 = 60;
	printf("p1 = %d\n",*p1);

	p2 = (int *)malloc(20);
	if(p2 == NULL){
		printf("malloc failed!\n");
	}
	*p2 = 70;
	printf("p2 = %d\n",*p2);	

	p3 = (int *)malloc(sizeof(int));
	if(p3 == NULL){
		printf("malloc failed!\n");
	}
	*p3 = 80;
	printf("p3 = %d\n",*p3);

	free((void *)p2 );

	p4 = (int *)malloc(sizeof(int));
	if(p4 == NULL){
		printf("malloc failed!\n");
	}else{
		*p4 = 90;
		printf("p4 = %d\n",*p4);
	}

	free((void *)p1);
	free((void *)p3);
	free((void *)p4);

	while(1);
	return 0;
}