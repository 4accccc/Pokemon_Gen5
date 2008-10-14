/*---------------------------------------------------------------------------*
  Project:  Pokemon DP WiFi
  File:     dpw_typedef.h

  Copyright 2006 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $NoKeywords: $
 *---------------------------------------------------------------------------*/

#ifndef DPW_TYPEDEF_H_
#define DPW_TYPEDEF_H_


#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------*
					DSのプログラムで使用される型
 *-----------------------------------------------------------------------*/

typedef unsigned char u8;
typedef unsigned short int u16;
typedef unsigned long u32;
#ifdef _WIN32
typedef unsigned __int64 u64;
#else
typedef unsigned long long u64;
#endif

typedef signed char s8;
typedef signed short int s16;
typedef signed long s32;
#ifdef _WIN32
typedef signed __int64 s64;
#else
typedef signed long long s64;
#endif


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
