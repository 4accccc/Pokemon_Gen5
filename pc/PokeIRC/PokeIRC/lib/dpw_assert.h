/*---------------------------------------------------------------------------*
  Project:  Pokemon DP WiFi
  File:     dpw_assert.h

  Copyright 2006 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $NoKeywords: $
 *---------------------------------------------------------------------------*/

#ifndef DPW_ASSERT_H_
#define DPW_ASSERT_H_


#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <stdio.h>
#include "dpw_typedef.h"

/*-----------------------------------------------------------------------*
					å^ÅEíËêîêÈåæ
 *-----------------------------------------------------------------------*/

#ifdef	_DEBUG
#define DPW_TASSERTMSG(exp, ...) do {\
	if (!(exp)) {	\
		fprintf(stderr, "%s:%d Panic: ", __FILE__, __LINE__);	\
		fprintf(stderr, __VA_ARGS__);	\
		abort();	\
	}	\
} while (0)
#else  // _DEBUG
#ifdef _WIN32
	#define DPW_TASSERTMSG
#else
	#define DPW_TASSERTMSG(exp, ...)    ((void) 0)
#endif
#endif // _DEBUG


#ifdef	_DEBUG
#define DPW_NULL_TASSERT(exp) do {\
	if ((exp) == NULL) {	\
		fprintf(stderr, "%s:%d Panic: ", __FILE__, __LINE__);	\
		fprintf(stderr, "Pointer must not be NULL ("#exp")");	\
		abort();	\
	}	\
} while (0)
#else  // SDK_FINALROM
#define DPW_NULL_TASSERT(exp)           ((void) 0)
#endif // SDK_FINALROM


#ifdef	_DEBUG
#define DPW_MINMAX_TASSERT(exp, min, max) do {\
	if ((exp) < (min) || (exp) > (max)) {	\
		fprintf(stderr, "%s:%d Panic: ", __FILE__, __LINE__);	\
		fprintf(stderr, #exp " is out of bounds(%d)\n%d <= "#exp" <= %d not satisfied.", exp, min, max);	\
		abort();	\
	}	\
} while (0)
#else  // SDK_FINALROM
#define DPW_MINMAX_TASSERT(exp, min, max)           ((void) 0)
#endif // SDK_FINALROM

#ifdef _WIN32
	#define OS_TPrintf
#else
	#define OS_TPrintf(...)		printf(__VA_ARGS__)
#endif

#ifdef __cplusplus
}
#endif


#endif // DPW_ASSERT_H_
