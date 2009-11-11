/*---------------------------------------------------------------------------*
  Project:  TwlWiFi - NHTTP - libraries
  File:     NHTTP_stdlib_NTR.h

  Copyright 2007-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-18#$
  $Rev: 1024 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/
#ifndef __NHTTP_STDLIB_NTR_H__
#define __NHTTP_STDLIB_NTR_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <nitro/os/common/printf.h>

#define         NHTTPi_printf(...)              OS_TPrintf(__VA_ARGS__)

/*---------------------------------------------------------------------------*
 * Function Prototypes
 *---------------------------------------------------------------------------*/
extern int      NHTTPi_encodeUrlChar            ( char* p_p, char c );
extern int      NHTTPi_getUrlEncodedSize        ( const char* p_p );
extern int      NHTTPi_getUrlEncodedSize2       ( const char* p_p, int len );
extern int      NHTTPi_strToHex                 ( const char* p_p, int len );
extern int      NHTTPi_strToInt                 ( const char* p_p, int len );
extern int      NHTTPi_intToStr                 ( char* p_p, u32 n );
extern int      NHTTPi_compareToken             ( const char* p1_p, const char* p2_p );
extern int      NHTTPi_memfind                  ( const char* p_p, int size, const char* pattern_p, int patternsize );
extern int      NHTTPi_strtonum                 ( const char* p_p, int size );
extern int      NHTTPi_strcmp                   ( const char *str1_p, const char *str2_p );
extern int      NHTTPi_strncmp                  ( const char* p1_p, const char* p2_p, int size );
extern int      NHTTPi_strnicmp                 ( const char* p1_p, const char* p2_p, int size );
extern int      NHTTPi_strlen                   ( const char *str_p );
extern int      NHTTPi_strnlen                  ( const char *str_p, int len );
extern void     NHTTPi_memclr                   ( void* p_p, int size );
extern void     NHTTPi_memcpy                   ( void* dst_p, const void* src_p, int size );
// 2006/08/04’Ç‰Á    
extern int      NHTTPi_Base64Encode             ( char* dst_p, const char* src_p );

#ifdef __cplusplus
}
#endif

#endif // __NHTTP_STDLIB_NTR_H__
