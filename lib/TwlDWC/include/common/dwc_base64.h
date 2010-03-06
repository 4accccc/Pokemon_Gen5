/*---------------------------------------------------------------------------*
  Project:  TwlDWC

  Copyright 2005-2010 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.
 *---------------------------------------------------------------------------*/

#ifndef DWC_BASE64_H_
#define DWC_BASE64_H_

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus


    /* -------------------------------------------------------------------------
            function - external
       ------------------------------------------------------------------------- */

    int DWC_Base64Encode(const char *src, const unsigned long srclen, char *dst, const unsigned long dstlen);
    int DWC_Base64Decode(const char *src, const unsigned long srclen, char *dst, const unsigned long dstlen);


#ifdef __cplusplus
}
#endif // __cplusplus


#endif // DWC_BASE64_H_
