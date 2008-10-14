/*! @file
	@brief	DPWのWIN32版用assertマクロ
	
	@author	kitayama

	@version 1.00	initial release.
*/


#ifndef DPWI_ASSERT_H_
#define DPWI_ASSERT_H_


#ifdef __cplusplus
extern "C" {
#endif

#include "dpw_typedef.h"

/*-----------------------------------------------------------------------*
					型・定数宣言
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
#define DPW_TASSERTMSG(exp, ...)    ((void) 0)
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

#define OS_TPrintf(...)		printf(__VA_ARGS__)


#ifdef __cplusplus
}
#endif


#endif // DPWI_ASSERT_H_
