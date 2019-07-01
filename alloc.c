 
#include <stdio.h>
#include "alloc.h"


 
/////////////////////////////////////////////////////////////////


#define MAX_ALIGN 4         // - 16: // 内存溢出, 预留1行, end位于末行起始4字节, 调试好行对齐, 便于查看数据

#define MALOC_SIZEX         (MALOC_SIZE-MAX_ALIGN)   // 232 KB - 4 byte

#define MALOC_Base         (0x20006000) 

//#define  __zxl_test_small_ram__  1

#ifdef __zxl_test_small_ram__
#define MALOC_SIZE         (16*3+(64+8)*5) //16*3预留表头，64*5可以分配空间 //为了测试链表被破坏后指针重指向（内存总是先分完在找释放的）
#else 
#define MALOC_SIZE          ( (256-24) * 1024 - 4)      // = 232 KBytes -4Bytes//zxl
#endif



#define ub1      unsigned char       
#define ub2      unsigned short      
#define ub4      unsigned int        
#define ub8      unsigned long long  

#define MB8_SIZE   8  


typedef union st_Mit {
	struct {            // 用于管理链表
		union st_Mit  * pend;     // 下一内存块地址 
		
#ifdef __use_user_point__
		char          * upt;      // 指向此内存的用户指针: 指向pUserMm
#endif
		
		union st_Mit  * prev, *next;
	};
	struct {            // 用于具体内存块
		char * _end;	// 下一内存块地址 
#ifdef __use_user_point__
		char * _res;	// 指向此内存的用户指针: 指向pUserMm
#endif
		
		char   pm[8];
	};
}_Mit, *pMit;

// 内存管理  

typedef struct {	 // 12 Bytes __mymem_node_st
	ub4   alloc_max;    // 分配累计  cumulative

	ub4   alloc_cnt;    // 分配
	ub4   free_cnt;     // 释放
	ub4   merge_cnt;    // 合并
	ub4   debris_cnt;   // 碎片数   及空闲内存片链表长度,   空闲可以尝试优化[需要Mit加入用户指针]

	ub4   free_bytes;   // 空闲数  // ub4   alloc_bytes;  // 分配数
						// 24 
	pMit  phead;
	pMit  psec;  // 32 
	pMit  pend;//未用到
} DYN_MM;


typedef struct {	 // 12 Bytes __mymem_node_st 
	union {
		char  mm[MALOC_SIZE];//直接开内存zxl
		struct {
			_Mit  head;
			_Mit  nsec;  // 32
			_Mit  n3;    // 32
		};
	};
	ub4   end;
} MEM;
 
__no_init MEM mem @ (MALOC_Base);//zxl
__no_init DYN_MM  m;
 


//
//typedef struct {
//	ub4   _end;
//	ub4   f1;
//	ub4   f2;
//	ub4   f3;
//} MEM_magic;
 
//__no_init MEM_magic mgc @ (0x2003FFF0);


#define _node_size(pNode)    ((ub4)(pNode->pend) - (ub4)pNode)
 

void minit()
{
	ub4  ie, i3;
	ub4 * pu4 = (ub4 *)&m;
	for (int i = 0; i<6; i++) *pu4++ = 0;
	 

	m.phead = &mem.head;
	m.psec = &mem.nsec;

	m.phead->prev = m.phead->next = m.psec;    m.phead->pend = m.psec;  
	m.psec->prev = m.psec->next = m.phead;    m.psec->pend = (pMit)&(mem.end); 
	
#ifdef __use_user_point__
	 m.phead->upt = 0;
	 m.psec->upt = 0;

#endif

	
	
	ie = (ub4)m.psec->pend;        // 0x2003FFFC
	i3 = (ub4)&mem.n3;             // 0x20006020
	m.free_bytes = ie - i3;
}

static int _align_size(int size)//补充字节 4的倍数zxl
{
	return (size + (MAX_ALIGN - 1)) & ~(MAX_ALIGN - 1);
}

static int _alloc_size(int size)// 最小12 byte（本次程序不能小于16不然链表建立不了）
{
#define MIN_NODE_SIZE  8	
	size = _align_size(size) + MB8_SIZE;// _align_size(sizeof(void *));// 结束地址及用户指针[8字节]
	return size >= MIN_NODE_SIZE ? size : MIN_NODE_SIZE;
}


static pMit _find(_Mit *head, int size)
{
	pMit pNode;
	int n;

	for (pNode = head->next; pNode != head; pNode = pNode->next)
	{
		 n = _node_size(pNode);   // 内存溢出 
                
		
                
                if (n >= size) 
			{
                  
                  if((n-size)<16)
				 {
					if(pNode->prev==pNode->next) return 0;//zxl ,防止第一个mem.nsec被释放掉
					pNode->prev->next=pNode->next;
					pNode->next->prev=pNode->prev;//链表被破坏，重新指向zxl(为避免链表破坏后剩余的内存碎片，建议分配字节最小单位为8字节，实际上会扩展为16字节)
		        }
                
                return pNode;
          } 
		
	}
	return NULL;// min;
}
 

void *maloc(void *pu, int size)
{
    #define RESERVE_SIZE   12 // By Duckweed: 20181123      32
	pMit pNode = NULL;
	pMit pend = 0;

	if (size<4) return NULL;  

	size = _alloc_size(size);    // 加 MB8_SIZE 字节
								 // taskENTER_CRITICAL();        // = vPortEnterCritical();  #include "cmsis_os.h"
	{
		pNode = _find(m.phead, size);
		if (!pNode) {                // taskEXIT_CRITICAL();
			return NULL;
		} 
		pend = pNode->pend;                  

		if (_node_size(pNode)<size) {                       // taskEXIT_CRITICAL();
			return NULL;             // 剩余空间不足, 不分配（理论不用这次判断。因为在_find(m.phead, size)中做过判断zxl）
		}
		 
		pNode->_end -= size;          
		m.free_bytes -= size;          m.alloc_cnt++;

		pNode = pNode->pend;
		pNode->pend = pend;       
	}                                     // taskEXIT_CRITICAL(); // = vPortExitCritical(); #include "cmsis_os.h"
#ifdef __use_user_point__
		pNode->upt = pu;						 
#endif								 
	
	return pNode->pm;
} 

int mmpt_size(void *pm)
{
	if (!pm) return 0;
	pMit pNode = (pMit)((char *)pm - MB8_SIZE);
	return pNode->pend - pNode;
}


static void _insert(pMit head, pMit pCur)
{
	pMit it;       
	for (it = head->next; it < pCur && it != head; it = it->next);
	pCur->next = it;           
	pCur->prev = it->prev;     
	it->prev->next = pCur;     
	it->prev = pCur;           
}

static void _merge(pMit head, pMit pCur)    // 当前[4]     // 小号为内存高地址
{
	pMit prv = pCur->prev; 
	pMit nxt = pCur->next; 

	if (nxt == pCur->pend)          
	{
		m.merge_cnt++;              
		pCur->pend = nxt->pend;     
		pCur->next = nxt->next;     
		nxt->next->prev = nxt->prev; 
	}
        
        nxt = pCur->next;//zxl注意合并后的nxt值发生了变化
	if (pCur == pCur->prev->pend && pCur->prev != head)// 向下合并[小地址]
	{
		m.merge_cnt++;

		prv->pend = pCur->pend; 
		prv->next = pCur->next;  
		nxt->prev = pCur->prev;  
	}
}

void mfree(void *pm)
{
	pMit pNode;
	int len;

	if (!pm) return;
	//taskENTER_CRITICAL();
	{
		pNode = (pMit)((char *)pm - MB8_SIZE); 

		len = _node_size(pNode); // pNode->pend - pNode;
		m.free_bytes += len;        m.free_cnt++;

		_insert(m.phead, pNode);
		_merge(m.phead, pNode); 
	}
	//taskEXIT_CRITICAL();
}

void *malocX(void *pu, int size, int * total)   
{
	void *mm = 0;
	mm = maloc(pu, size);
	if (mm) (*total) += mmpt_size(mm);
	return mm;
}

void mfreeX(void *ptr, int * total)
{
	int size = mmpt_size(ptr);
	if ((*total) > size) (*total) -= size; else (*total) = 0;
	mfree(ptr);
}
 



/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////

// 内存比较, 直到固定长度或者不等( s1 < s2 ? -1 : +1 )
int mmcmp(const void *s1, const void *s2, unsigned  int n)
{       /* compare uchar s1[n], s2[n] */
	const char *su1 = (const char *)s1;
	const char *su2 = (const char *)s2; 

	for (; n; n--, su1++, su2++) {
		if (*su1 != *su2) return (*su1 < *su2 ? -1 : +1);
	}
	return (0);
}

void *mmcpy(void * s1, const void * s2, unsigned  int n)
{       /* copy char s2[n] to s1[n] in any order */
	char *su1 = (char *)s1;
	const char *su2 = (const char *)s2;

	for (; 0 < n; su1++, su2++, n--)
		(*su1) = (*su2);
	// for (; 0 < n;  *su1++ = *su2++, n--) ;
	return (s1);
}

void *mmset(void *s, int c, unsigned  int n)
{       /* store c throughout uchar s[n] */
	const char uc = (char)c;
	char *su = (char *)s;

	for (; 0 < n; ++su, --n)
		*su = uc;
	return (s);
}


////////////////////////////////////////////////////////////////////


void _mtest()
{
	int i, l;
	
	char *pb = 0;
        
        
#ifdef __zxl_test_small_ram__
void *arr[5];
for (i = 0; i < 5; i++)
	{
		arr[i] = maloc(&arr[i], 64);      		//printf("arr[%d] = %p\r\n",i,arr[i]);
		if (arr[i] == NULL) break;
		pb = (char *)(arr[i]);
		mmset(pb, 0, 64);

		for (l = 0; l < 64; l++)
			pb[l] = '0' + i;
	}

	mmset(arr[0], 'X', 64);
	mfree(arr[0]);
	//arr[0] = 0;
 

	mmset(arr[2], 0xff, 64);
	mfree(arr[2]);
	

	arr[4] = maloc(&arr[4], 64);
	mmset(arr[4], '$', 64);

	//printf("arr[15] = %p\r\n",arr[15]);
#else 
void *arr[16];
for (i = 0; i < 16; i++)
	{
		arr[i] = maloc(&arr[i], 64);      		//printf("arr[%d] = %p\r\n",i,arr[i]);
		if (arr[i] == NULL) break;
		pb = (char *)(arr[i]);
		mmset(pb, 0, 64);

		for (l = 0; l < 64; l++)
			pb[l] = '0' + i;
	}

	mmset(arr[0], 'X', 64);
	mfree(arr[0]);
	//arr[0] = 0;

	mmset(arr[15], 0xff, 64);
	mfree(arr[15]);
	//arr[15] = 0;

	mmset(arr[3], 0xff, 64);
	mfree(arr[3]);
	//arr[3] = 0;

	mmset(arr[5], 0xff, 64);
	mfree(arr[5]);
	//arr[5] = 0;

	mmset(arr[4], 0xff, 64);
	mfree(arr[4]);
	//arr[4] = 0;

	arr[15] = maloc(&arr[15], 80);
	mmset(arr[15], '$', 80);

	//printf("arr[15] = %p\r\n",arr[15]);
#endif

}

ub2 get_free_size()
{
    
    if(m.free_bytes/MALOC_SIZE) return 100;
    if(m.free_bytes%MALOC_SIZE)
    {
    return ((m.free_bytes/MALOC_SIZE)*100);
    }
    return 0;

}

