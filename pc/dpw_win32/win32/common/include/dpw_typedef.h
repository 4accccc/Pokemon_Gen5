/*! @file
	@brief	DPW‚ÌWIN32”Å‚ÅŽg—p‚·‚éŒ^‚Ì’è‹`
	
	@author	kitayama

	@version 1.00	initial release.
*/

#ifndef DPW_TYPEDEF_H_
#define DPW_TYPEDEF_H_


#ifdef __cplusplus
extern "C" {
#endif


typedef unsigned char u8;
typedef unsigned short int u16;
typedef unsigned long u32;
typedef unsigned __int64 u64;

typedef signed char s8;
typedef signed short int s16;
typedef signed long s32;
typedef signed __int64 s64;


typedef int BOOL;
#define TRUE                    1
#define FALSE                   0


#ifndef NULL
#ifdef  __cplusplus
#define NULL                    0
#else  // __cplusplus
#define NULL                ((void *)0)
#endif // __cplusplus
#endif // NULL



#ifdef __cplusplus
}
#endif

#endif /* DPW_TYPEDEF_H_ */
