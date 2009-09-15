/*---------------------------------------------------------------------------*
  Project:  TwlSDK - WM - libraries
  File:     wm_private.h

  Copyright 2003-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-18#$
  $Rev: 8573 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/

#ifndef LIBRARIES_WM_PRIVATE_H_
#define LIBRARIES_WM_PRIVATE_H_

#ifdef  __cplusplus
extern "C" {
#endif

/*===========================================================================*/


#include <nitro.h>

#ifdef SDK_ARM7
#include <nitro_wl/ARM7/WlLib.h>
#endif

/*---------------------------------------------------------------------------*
    定数定義
 *---------------------------------------------------------------------------*/

//#define WM_DEBUG                // WM のデバッグフラグ(主にARM7)
//#define WM_DEBUG_DATASHARING    // DataSharing 関係のデバッグメッセージフラグ(ARM9)
//#define WM_DEBUG_CALLBACK       // ARM9 の無線用 PXI 割り込みのデバッグ表示フラグ(ARM9)
//#define WM_DEBUG_AIDBITMAP      // ARM9 の AID BITMAP 関係のデバッグ表示フラグ(ARM9)

/*---------------------------------------------------------------------------*
    マクロ定義
 *---------------------------------------------------------------------------*/

#ifdef SDK_DEBUG
#define WM_WARNING(...) \
    ( (void) ( WMi_Warning(__FILE__, __LINE__, __VA_ARGS__) ) )
#else
#define WM_WARNING(...)    ((void) 0)
#endif

#ifdef WM_DEBUG
#define WM_ASSERT(exp) \
    ( (void) ( (exp) || (WMi_Warning(__FILE__, __LINE__, "Failed assertion " #exp), 0) ) )
#define WM_ASSERTMSG(exp, ...) \
    ( (void) ( (exp) || (WMi_Warning(__FILE__, __LINE__, __VA_ARGS__), 0) ) )
#define WM_DPRINTF WMi_Printf
#else  // WM_DEBUG
#define WM_ASSERT(exp)     ((void) 0)
#define WM_ASSERTMSG(...)     ((void) 0)
#define WM_DPRINTF(...)     ((void) 0)
#endif // WM_DEBUG

#ifndef SDK_FINALROM
#define WMi_Printf      OS_TPrintf
#define WMi_Warning     OSi_TWarning
#else
#define WMi_Printf(...)    ((void) 0)
#define WMi_Warning(...)   ((void) 0)
#endif


/*===========================================================================*/

#ifdef  __cplusplus
}       /* extern "C" */
#endif

#endif /* LIBRARIES_WM_PRIVATE_H_ */

/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/
