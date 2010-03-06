/*---------------------------------------------------------------------------*
  Project:  TwlDWC

  Copyright 2005-2010 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.
 *---------------------------------------------------------------------------*/

#ifndef DWC_INIT_H_
#define DWC_INIT_H_

#include <common/dwc_memfunc.h>

#ifdef __cplusplus
extern "C"
{
#endif


    /* -------------------------------------------------------------------------
            define
       ------------------------------------------------------------------------- */

// 初期化関数ワークメモリサイズ
#ifdef SDK_TWL
#define DWC_INIT_WORK_SIZE 0x1000
#define DWC_BM_INIT_WORK_SIZE 0x1000
#else
#define DWC_INIT_WORK_SIZE 0x700
#define DWC_BM_INIT_WORK_SIZE 0x700
#endif


    /* -------------------------------------------------------------------------
            enum
       ------------------------------------------------------------------------- */

    // DWC_Init関数の返り値を示す列挙子
    enum
    {
        DWC_INIT_RESULT_NOERROR = 0,            // 初期化成功。
        DWC_INIT_RESULT_CREATE_USERID,          // DS本体に仮のユーザIDを生成しました。
        DWC_INIT_RESULT_DESTROY_USERID,         // DS本体のユーザIDが破壊されていた可能性があるので、仮のユーザIDを生成しました。
        DWC_INIT_RESULT_DESTROY_OTHER_SETTING,  // DS本体の接続先設定が破壊された可能性があります。
        DWC_INIT_RESULT_MEMORY_FULL,            // ワークメモリの確保に失敗しました。(HYBRID ROM / LIMITED ROM で0x1000バイト、NITRO ROMで0x700バイト)
        DWC_INIT_RESULT_LAST
    };


    /* -------------------------------------------------------------------------
            function - external
       ------------------------------------------------------------------------- */

    int     DWC_InitForDevelopment( const char* gameName, u32 gameCode, DWCAllocEx alloc, DWCFreeEx free );
    int     DWC_InitForProduction( const char* gameName, u32 gameCode, DWCAllocEx alloc, DWCFreeEx free );

    u64     DWC_GetAuthenticatedUserId( void );

    void    DWC_Debug_DWCInitError( void* work, int dwc_init_error );


    /* -------------------------------------------------------------------------
            function - internal
       ------------------------------------------------------------------------- */
    u32    DWCi_GetGamecode         ( void );



#ifdef __cplusplus
}
#endif

#endif // DWC_INIT_H_
