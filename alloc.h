
#ifndef _ALLOC_H_
#define	_ALLOC_H_


#ifdef	__cplusplus
extern "C" {
#endif

#include "stm32f4xx_hal.h"
#include "main.h"
    
#include "buff_management.h"


#define __USE_MY_MALLOC__  1


#define __use_user_point__ 1




  


// ÄÚ´æ¹ÜÀí
#ifdef __USE_MY_MALLOC__

    void minit(); 
    int mmpt_size(void *pm);
    void mfree(void *pm);
    void mfreeX(void *ptr, int * total);
    void *maloc(void *pu, int size);
    void *malocX(void *pu, int size, int * total);
    
    int mmcmp(const void *s1, const void *s2, unsigned  int n);
    void *mmcpy(void * s1, const void * s2, unsigned  int n);
    void *mmset(void *s, int c, unsigned  int n);
    void _mtest(); 
	ub2 get_free_size();
#else 
    #define  minit()    
    #define  maloc          malloc
    #define  mfree          free
    #define  _mtest()   
#endif



	int mmcmp(const void *s1, const void *s2, unsigned  int n);
	void *mmcpy(void * s1, const void * s2, unsigned  int n);
	void *mmset(void *s, int c, unsigned  int n);

////////////////////////////////////////////////////////////
#ifdef	__cplusplus
}
#endif

#endif	/* _PMEM_H_ */


/****************** End of file ***************************/