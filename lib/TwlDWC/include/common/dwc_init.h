/*---------------------------------------------------------------------------*
  Project:  TwlDWC

  Copyright 2005-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.
 *---------------------------------------------------------------------------*/

#ifndef DWC_INIT_H_
#define DWC_INIT_H_

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

    enum
    {
        DWC_INIT_RESULT_NOERROR,
        DWC_INIT_RESULT_CREATE_USERID,
        DWC_INIT_RESULT_DESTROY_USERID,
        DWC_INIT_RESULT_DESTROY_OTHER_SETTING,
        DWC_INIT_RESULT_LAST,

        DWC_INIT_RESULT_DESTORY_USERID = DWC_INIT_RESULT_DESTROY_USERID,
        DWC_INIT_RESULT_DESTORY_OTHER_SETTING = DWC_INIT_RESULT_DESTROY_OTHER_SETTING
    };



    /* -------------------------------------------------------------------------
            function - external
       ------------------------------------------------------------------------- */

    int     DWC_Init( void* work );

    u64     DWC_GetAuthenticatedUserId( void );

    void    DWC_Debug_DWCInitError( void* work, int dwc_init_error );



#ifdef __cplusplus
}
#endif

#endif // DWC_INIT_H_
