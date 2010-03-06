/*---------------------------------------------------------------------------*
  Project:  DWC

  Copyright 2005-2010 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.
 *---------------------------------------------------------------------------*/

#ifndef DWC_COMMON_H_
#define DWC_COMMON_H_

#ifdef __cplusplus
extern "C"
{
#endif


//----------------------------------------------------------------------------
// define
//----------------------------------------------------------------------------

/// îƒópkey/valueï∂éöóÒÇÃÇªÇÍÇºÇÍÇÃç≈ëÂï∂éöóÒí∑
#define DWC_COMMONSTR_MAX_KEY_VALUE_LEN 4096

//----------------------------------------------------------------------------
// enum
//----------------------------------------------------------------------------


//----------------------------------------------------------------------------
// typedef - function
//----------------------------------------------------------------------------


//----------------------------------------------------------------------------
// struct
//----------------------------------------------------------------------------


//----------------------------------------------------------------------------
// function - external
//----------------------------------------------------------------------------

    int  DWC_SetCommonKeyValueString    ( const char* key, const char* value, char* string, char separator );
    int  DWC_AddCommonKeyValueString    ( const char* key, const char* value, char* string, char separator );
    int  DWC_GetCommonValueString       ( const char* key, char* value, const char* string, char separator );


#ifdef __cplusplus
}
#endif


#endif // DWC_COMMON_H_
